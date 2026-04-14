#!/usr/bin/env bash
# RT583-EVB — open minicom serial terminal
#
# Usage:
#   bash scripts/linux/minicom.sh              # auto-detect serial port
#   bash scripts/linux/minicom.sh /dev/ttyACM0 # specify device

set -euo pipefail

PORT="${1:-}"
BAUD=115200

# ── Verify minicom is installed ───────────────────────────────────────────────
if ! command -v minicom &>/dev/null; then
    echo "Installing minicom..."
    sudo apt-get install -y minicom
fi

# ── Auto-detect serial port ───────────────────────────────────────────────────
if [[ -z "$PORT" ]]; then
    # Priority order: ttyACM (DAPLink CDC) > ttyUSB
    CANDIDATES=( $(ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null) )

    if [[ ${#CANDIDATES[@]} -eq 0 ]]; then
        echo "No serial device found (/dev/ttyACM* or /dev/ttyUSB*)" >&2
        echo "Ensure the CMSIS-DAP is attached to WSL via usbipd:" >&2
        echo "  .\\tools\\windows\\attach-usb.ps1" >&2
        exit 1
    elif [[ ${#CANDIDATES[@]} -eq 1 ]]; then
        PORT="${CANDIDATES[0]}"
    else
        echo "Multiple serial devices found, select one:"
        for i in "${!CANDIDATES[@]}"; do
            echo "  [$i] ${CANDIDATES[$i]}"
        done
        read -r -p "Enter number: " sel
        PORT="${CANDIDATES[$sel]}"
    fi
fi

# ── Verify device exists ──────────────────────────────────────────────────────
if [[ ! -c "$PORT" ]]; then
    echo "Device not found: $PORT" >&2
    exit 1
fi

# ── dialout group permission ──────────────────────────────────────────────────
if ! groups | grep -q dialout; then
    echo "Adding to dialout group (WSL restart required for this to take effect)..."
    sudo usermod -aG dialout "$USER"
    echo "Run: wsl --terminate Ubuntu  then reopen WSL"
    echo "Using sudo for this session"
    SUDO=sudo
else
    SUDO=""
fi

# ── Start minicom ─────────────────────────────────────────────────────────────
echo ""
echo "Port: $PORT  Baud: $BAUD 8N1"
echo "Exit: Ctrl-A X"
echo ""

$SUDO minicom -D "$PORT" -b "$BAUD" -8 --noinit
