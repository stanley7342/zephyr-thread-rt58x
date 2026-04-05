#!/usr/bin/env bash
# RT582-EVB — 開啟 minicom 序列終端機
#
# 用法：
#   bash scripts/linux/minicom.sh              # 自動偵測序列埠
#   bash scripts/linux/minicom.sh /dev/ttyACM0 # 指定裝置

set -euo pipefail

PORT="${1:-}"
BAUD=115200

# ── 確認 minicom 已安裝 ───────────────────────────────────────────────────────
if ! command -v minicom &>/dev/null; then
    echo "安裝 minicom..."
    sudo apt-get install -y minicom
fi

# ── 自動偵測序列埠 ────────────────────────────────────────────────────────────
if [[ -z "$PORT" ]]; then
    # 依優先順序：ttyACM（DAPLink CDC）> ttyUSB
    CANDIDATES=( $(ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null) )

    if [[ ${#CANDIDATES[@]} -eq 0 ]]; then
        echo "找不到序列裝置（/dev/ttyACM* 或 /dev/ttyUSB*）" >&2
        echo "請確認 CMSIS-DAP 已透過 usbipd 掛載至 WSL：" >&2
        echo "  .\\tools\\windows\\attach-usb.ps1" >&2
        exit 1
    elif [[ ${#CANDIDATES[@]} -eq 1 ]]; then
        PORT="${CANDIDATES[0]}"
    else
        echo "找到多個序列裝置，請選擇："
        for i in "${!CANDIDATES[@]}"; do
            echo "  [$i] ${CANDIDATES[$i]}"
        done
        read -r -p "輸入編號：" sel
        PORT="${CANDIDATES[$sel]}"
    fi
fi

# ── 確認裝置存在 ──────────────────────────────────────────────────────────────
if [[ ! -c "$PORT" ]]; then
    echo "裝置不存在：$PORT" >&2
    exit 1
fi

# ── dialout 群組權限 ──────────────────────────────────────────────────────────
if ! groups | grep -q dialout; then
    echo "加入 dialout 群組（需重新登入 WSL 才能生效）..."
    sudo usermod -aG dialout "$USER"
    echo "請執行：wsl --terminate Ubuntu  然後重新開啟 WSL"
    echo "本次改用 sudo 開啟 minicom"
    SUDO=sudo
else
    SUDO=""
fi

# ── 啟動 minicom ──────────────────────────────────────────────────────────────
echo ""
echo "序列埠：$PORT  速率：$BAUD 8N1"
echo "離開：Ctrl-A X"
echo ""

$SUDO minicom -D "$PORT" -b "$BAUD" -8 --noinit
