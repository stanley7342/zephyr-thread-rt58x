#!/usr/bin/env bash
# RT583-EVB — macOS 燒錄腳本（CMSIS-DAP + OpenOCD）
#
# 用法：
#   bash scripts/macos/flash.sh -p thread        # slot0 (0x10000)
#   bash scripts/macos/flash.sh -p bootloader    # 0x00000000
#   bash scripts/macos/flash.sh -p thread --addr 0x0   # 覆蓋位址
#   bash scripts/macos/flash.sh --bin path/to/custom.bin --addr 0x0

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
TOOLS_LINUX="$PROJECT_DIR/tools/linux"

TARGET=""
BIN=""
ADDR=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p)    TARGET="$2"; shift 2 ;;
        --bin) BIN="$2"; shift 2 ;;
        --addr) ADDR="$2"; shift 2 ;;
        *) echo "用法：$0 -p <thread|bootloader> [--addr <hex>] [--bin <path>]" >&2; exit 1 ;;
    esac
done

if [[ -n "$TARGET" ]]; then
    if [[ "$TARGET" != "thread" && "$TARGET" != "bootloader" ]]; then
        echo "錯誤：不支援的 target '$TARGET'，請使用 thread 或 bootloader" >&2
        exit 1
    fi
    if [[ -z "$BIN" ]]; then
        if [[ "$TARGET" == "thread" ]]; then
            BIN="$PROJECT_DIR/build/thread/zephyr/zephyr.signed.bin"
        else
            BIN="$PROJECT_DIR/build/$TARGET/${TARGET}_zephyr.bin"
        fi
    fi
    if [[ -z "$ADDR" ]]; then
        [[ "$TARGET" == "bootloader" ]] && ADDR="0x0" || ADDR="0x10000"
    fi
elif [[ -z "$BIN" ]]; then
    echo "錯誤：請指定 -p <thread|bootloader> 或 --bin <path>" >&2
    exit 1
fi

[[ -z "$ADDR" ]] && ADDR="0x0"

# ── 顏色 ──────────────────────────────────────────────────────────────────────
step()  { echo -e "\n\033[36m==> $*\033[0m"; }
ok()    { echo -e "    \033[32m[OK]\033[0m $*"; }
err()   { echo -e "    \033[31m[!!]\033[0m $*" >&2; }

# ── 尋找 openocd-rt58x ───────────────────────────────────────────────────────
step "尋找 openocd-rt58x"

OCD_BIN=""
OCD_SCRIPT_DIR=""
WORKSPACE="$(dirname "$PROJECT_DIR")"

OCD_SEARCH=(
    "$PROJECT_DIR/tools/macos/openocd:::$PROJECT_DIR/tools/macos/tcl"
    "$WORKSPACE/openocd-rt58x/src/openocd:::$WORKSPACE/openocd-rt58x/tcl"
    "$WORKSPACE/openocd-rt58x/build/src/openocd:::$WORKSPACE/openocd-rt58x/tcl"
    "$HOME/openocd-rt58x/src/openocd:::$HOME/openocd-rt58x/tcl"
    "$HOME/openocd-rt58x/build/src/openocd:::$HOME/openocd-rt58x/tcl"
    "/opt/openocd-rt58x/bin/openocd:::/opt/openocd-rt58x/share/openocd/scripts"
)

for entry in "${OCD_SEARCH[@]}"; do
    bin_path="${entry%%:::*}"
    tcl_path="${entry##*:::}"
    if [[ -x "$bin_path" ]]; then
        OCD_BIN="$bin_path"
        [[ -d "$tcl_path" ]] && OCD_SCRIPT_DIR="$tcl_path"
        ok "openocd-rt58x：$OCD_BIN"
        [[ -n "${OCD_SCRIPT_DIR:-}" ]] && ok "Scripts   ：$OCD_SCRIPT_DIR"
        break
    fi
done

if [[ -z "$OCD_BIN" ]]; then
    SYS_OCD="$(command -v openocd 2>/dev/null || true)"
    if [[ -n "$SYS_OCD" ]]; then
        OCD_BIN="$SYS_OCD"
        OCD_SCRIPT_DIR="$TOOLS_LINUX/tcl"
        ok "使用系統 openocd：$OCD_BIN（搭配本專案 tcl scripts）"
    fi
fi

if [[ -z "$OCD_BIN" ]]; then
    err "找不到 openocd"
    echo ""
    echo "    安裝方式（擇一）："
    echo ""
    echo "    1) Homebrew："
    echo "       brew install openocd"
    echo ""
    echo "    2) 從原始碼編譯 openocd-rt58x："
    echo "       git clone <openocd-rt58x-repo> \$HOME/openocd-rt58x"
    echo "       cd \$HOME/openocd-rt58x"
    echo "       ./bootstrap && ./configure && make -j\$(sysctl -n hw.logicalcpu)"
    exit 1
fi

if [[ -z "${OCD_SCRIPT_DIR:-}" ]]; then
    BREW_SHARE="$(brew --prefix 2>/dev/null)/share/openocd/scripts"
    [[ -d "$BREW_SHARE" ]] && OCD_SCRIPT_DIR="$BREW_SHARE"
fi

if [[ -z "${OCD_SCRIPT_DIR:-}" ]]; then
    err "找不到 OpenOCD tcl scripts 目錄"
    exit 1
fi

# ── 確認 binary 存在 ──────────────────────────────────────────────────────────
step "確認 binary"

if [[ ! -f "$BIN" ]]; then
    err "找不到 binary：$BIN"
    [[ -n "$TARGET" ]] && echo "    請先執行：bash scripts/macos/build.sh -p $TARGET"
    exit 1
fi
ok "Binary：$BIN（$(du -h "$BIN" | awk '{print $1}')）"

# ── 確認 CMSIS-DAP 裝置可見 ──────────────────────────────────────────────────
step "確認 CMSIS-DAP 裝置"

USB_INFO="$(system_profiler SPUSBDataType 2>/dev/null || true)"
if ! echo "$USB_INFO" | grep -qi "cmsis.dap\|daplink\|mbed\|0d28"; then
    err "找不到 CMSIS-DAP 裝置"
    echo ""
    echo "    請確認 CMSIS-DAP 調試器已透過 USB 連接至 Mac 並上電"
    exit 1
fi
ok "CMSIS-DAP 已偵測到"

# ── 燒錄 ──────────────────────────────────────────────────────────────────────
step "燒錄 → $ADDR"
echo "    Binary : $BIN"
echo "    Scripts: $OCD_SCRIPT_DIR"
echo ""

"$OCD_BIN" \
    -s "$OCD_SCRIPT_DIR" \
    -f interface/cmsis-dap.cfg \
    -f target/rt58x.cfg \
    -c "init; halt; flash write_image erase \"$BIN\" $ADDR; reset run; exit"

echo ""
echo -e "\n\033[32m[OK] 燒錄完成！請開啟序列終端機（115200 8N1）觀察輸出。\033[0m"
