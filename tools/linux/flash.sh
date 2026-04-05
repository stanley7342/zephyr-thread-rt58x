#!/usr/bin/env bash
# RT582-EVB — WSL/Linux 燒錄腳本（CMSIS-DAP + OpenOCD）
#
# 前置步驟（Windows 端）：
#   1. winget install usbipd
#   2. .\scripts\windows\attach-usb.ps1
#
# 用法：
#   bash scripts/linux/flash.sh
#   bash scripts/linux/flash.sh --bin path/to/zephyr.bin
#   bash scripts/linux/flash.sh --setup-udev   # 首次：安裝 udev 規則

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
BIN="${PROJECT_DIR}/build/zephyr/zephyr.bin"
SETUP_UDEV=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --bin)        BIN="$2"; shift 2 ;;
        --setup-udev) SETUP_UDEV=1; shift ;;
        *) echo "未知參數：$1"; exit 1 ;;
    esac
done

# ── 顏色 ──────────────────────────────────────────────────────────────────────
step()  { echo -e "\n\033[36m==> $*\033[0m"; }
ok()    { echo -e "    \033[32m[OK]\033[0m $*"; }
err()   { echo -e "    \033[31m[!!]\033[0m $*" >&2; }

# ── 首次設定：udev 規則 ───────────────────────────────────────────────────────
setup_udev() {
    step "安裝 CMSIS-DAP udev 規則"
    RULES_FILE="/etc/udev/rules.d/99-cmsis-dap.rules"
    cat <<'EOF' | sudo tee "$RULES_FILE" > /dev/null
# CMSIS-DAP v1（HID）
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="0d28", ATTRS{idProduct}=="0204", MODE="0666", GROUP="plugdev"
# CMSIS-DAP v2（bulk USB）
SUBSYSTEM=="usb", ATTRS{idVendor}=="0d28", ATTRS{idProduct}=="0204", MODE="0666", GROUP="plugdev"
# Generic ARM DAPLink
SUBSYSTEM=="usb", ATTR{idVendor}=="0d28", MODE="0666", GROUP="plugdev"
EOF
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    # 確認目前使用者在 plugdev 群組
    if ! groups | grep -q plugdev; then
        sudo usermod -aG plugdev "$USER"
        echo ""
        echo -e "    \033[33m已加入 plugdev 群組，需重新登入 WSL 才能生效：\033[0m"
        echo "      wsl --terminate Ubuntu  （在 PowerShell 執行）"
        echo "      然後重新開啟 WSL"
    fi
    ok "udev 規則已安裝：$RULES_FILE"
}

if [[ $SETUP_UDEV -eq 1 ]]; then
    setup_udev
    exit 0
fi

# ── 確認 OpenOCD 已安裝 ───────────────────────────────────────────────────────
step "檢查 OpenOCD"

if ! command -v openocd &>/dev/null; then
    echo "    未找到 openocd，正在安裝..."
    sudo apt-get update -qq
    sudo apt-get install -y openocd
fi
ok "OpenOCD：$(openocd --version 2>&1 | head -1)"

# ── 找 Rafael SDK OpenOCD scripts（rt58x.cfg）────────────────────────────────
step "尋找 rt58x.cfg"

# 優先用 Rafael SDK（掛載自 Windows）
RAFAEL_OCD_SCRIPT=""
RAFAEL_CANDIDATES=(
    "/mnt/c/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD/script"
    "/mnt/c/Users/Stanley/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD/script"
)
for cand in "${RAFAEL_CANDIDATES[@]}"; do
    if [[ -f "$cand/target/rt58x.cfg" ]]; then
        RAFAEL_OCD_SCRIPT="$cand"
        break
    fi
done

if [[ -n "$RAFAEL_OCD_SCRIPT" ]]; then
    ok "使用 Rafael SDK scripts：$RAFAEL_OCD_SCRIPT"
    OCD_SCRIPT_DIR="$RAFAEL_OCD_SCRIPT"
else
    # Fallback：在本專案找
    LOCAL_CFG="$PROJECT_DIR/boards/arm/rt582_evb/support/rt58x.cfg"
    if [[ -f "$LOCAL_CFG" ]]; then
        OCD_SCRIPT_DIR="$(dirname "$LOCAL_CFG")"
        ok "使用本專案 cfg：$LOCAL_CFG"
    else
        err "找不到 rt58x.cfg"
        echo ""
        echo "請指定 Rafael SDK 路徑，或確認已掛載 /mnt/c/Rafael-IoT-SDK-Internal"
        echo "也可手動指定：OCD_SCRIPT_DIR=/path/to/openocd/script bash scripts/linux/flash.sh"
        exit 1
    fi
fi

# ── 確認 binary 存在 ──────────────────────────────────────────────────────────
step "確認 binary"

if [[ ! -f "$BIN" ]]; then
    err "找不到 binary：$BIN"
    echo "    請先執行：bash scripts/linux/build.sh"
    exit 1
fi
ok "Binary：$BIN（$(du -h "$BIN" | cut -f1)）"

# ── 確認 CMSIS-DAP 裝置可見 ───────────────────────────────────────────────────
step "確認 CMSIS-DAP 裝置"

if ! lsusb 2>/dev/null | grep -qi "0d28\|cmsis\|daplink\|mbed"; then
    err "找不到 CMSIS-DAP 裝置（lsusb）"
    echo ""
    echo "    請先在 Windows PowerShell 執行："
    echo "      .\\scripts\\windows\\attach-usb.ps1"
    echo ""
    echo "    若是 udev 權限問題，執行："
    echo "      bash scripts/linux/flash.sh --setup-udev"
    exit 1
fi
ok "CMSIS-DAP 已偵測到"

# ── 燒錄 ──────────────────────────────────────────────────────────────────────
step "燒錄 → 0x00000000"
echo "    Binary : $BIN"
echo "    Scripts: $OCD_SCRIPT_DIR"
echo ""

openocd \
    -s "$OCD_SCRIPT_DIR" \
    -f interface/cmsis-dap.cfg \
    -f target/rt58x.cfg \
    -c "init; halt; flash write_image erase \"$BIN\" 0x0; reset run; exit"

echo ""
ok "燒錄完成！請開啟序列終端機（115200 8N1）觀察輸出。"
