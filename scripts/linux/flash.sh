#!/usr/bin/env bash
# RT583-EVB — WSL/Linux flash script (CMSIS-DAP + OpenOCD)
#
# Prerequisites (Windows side, if using WSL):
#   1. winget install usbipd
#   2. usbipd bind --busid <busid>; usbipd attach --wsl --busid <busid>
#
# Usage:
#   bash scripts/linux/flash.sh -p thread        # slot0 (0x10000)
#   bash scripts/linux/flash.sh -p bootloader    # 0x00000000
#   bash scripts/linux/flash.sh -p thread --addr 0x0   # override address
#   bash scripts/linux/flash.sh --bin path/to/custom.bin --addr 0x0
#   bash scripts/linux/flash.sh --setup-udev     # first-time: install udev rules

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
TOOLS_LINUX="$PROJECT_DIR/tools/linux"

TARGET=""
BIN=""
ADDR=""
SETUP_UDEV=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p)           TARGET="$2"; shift 2 ;;
        --bin)        BIN="$2"; shift 2 ;;
        --addr)       ADDR="$2"; shift 2 ;;
        --setup-udev) SETUP_UDEV=1; shift ;;
        *) echo "Usage: $0 -p <thread|bootloader> [--addr <hex>] [--bin <path>] [--setup-udev]" >&2; exit 1 ;;
    esac
done

# Determine default binary and flash address from target
if [[ -n "$TARGET" ]]; then
    if [[ "$TARGET" != "thread" && "$TARGET" != "bootloader" ]]; then
        echo "Error: unsupported target '$TARGET', use thread or bootloader" >&2
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
    echo "Error: specify -p <thread|bootloader> or --bin <path>" >&2
    exit 1
fi

[[ -z "$ADDR" ]] && ADDR="0x0"

# ── Colors ────────────────────────────────────────────────────────────────────
step()  { echo -e "\n\033[36m==> $*\033[0m"; }
ok()    { echo -e "    \033[32m[OK]\033[0m $*"; }
err()   { echo -e "    \033[31m[!!]\033[0m $*" >&2; }

# ── First-time setup: udev rules ──────────────────────────────────────────────
setup_udev() {
    step "Installing CMSIS-DAP udev rules"
    RULES_FILE="/etc/udev/rules.d/99-cmsis-dap.rules"
    cat <<'EOF' | sudo tee "$RULES_FILE" > /dev/null
# CMSIS-DAP v1 (HID)
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="0d28", ATTRS{idProduct}=="0204", MODE="0666", GROUP="plugdev"
# CMSIS-DAP v2 (bulk USB)
SUBSYSTEM=="usb", ATTRS{idVendor}=="0d28", ATTRS{idProduct}=="0204", MODE="0666", GROUP="plugdev"
# Generic ARM DAPLink
SUBSYSTEM=="usb", ATTR{idVendor}=="0d28", MODE="0666", GROUP="plugdev"
EOF
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    if ! groups | grep -q plugdev; then
        sudo usermod -aG plugdev "$USER"
        echo ""
        echo -e "    \033[33mAdded to plugdev group — WSL must be restarted for this to take effect:\033[0m"
        echo "      wsl --terminate Ubuntu  (run in PowerShell)"
        echo "      Then reopen WSL"
    fi
    ok "udev rules installed: $RULES_FILE"
}

if [[ $SETUP_UDEV -eq 1 ]]; then
    setup_udev
    exit 0
fi

# ── Locate openocd-rt58x (prefer tools/linux/ built-in) ──────────────────────
step "Locating openocd-rt58x"

OCD_BIN=""
OCD_SCRIPT_DIR=""
WORKSPACE="$(dirname "$PROJECT_DIR")"

OCD_SEARCH=(
    "$TOOLS_LINUX/openocd:::$TOOLS_LINUX/tcl"
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
        ok "openocd-rt58x: $OCD_BIN"
        [[ -n "$OCD_SCRIPT_DIR" ]] && ok "Scripts   : $OCD_SCRIPT_DIR"
        break
    fi
done

if [[ -z "$OCD_BIN" ]]; then
    err "No compiled openocd-rt58x found"
    echo ""
    echo "    Copy a compiled openocd binary to:"
    echo "      $PROJECT_DIR/tools/linux/openocd"
    echo ""
    echo "    Or build from source:"
    echo "      git clone <openocd-rt58x-repo> $HOME/openocd-rt58x"
    echo "      cd $HOME/openocd-rt58x"
    echo "      ./bootstrap && ./configure && make -j\$(nproc)"
    exit 1
fi

if [[ -z "$OCD_SCRIPT_DIR" ]]; then
    SYS_SHARE="/usr/share/openocd/scripts"
    [[ -d "$SYS_SHARE" ]] && OCD_SCRIPT_DIR="$SYS_SHARE"
fi

if [[ -z "$OCD_SCRIPT_DIR" ]]; then
    err "OpenOCD tcl scripts directory not found (expected at openocd-rt58x/tcl/)"
    exit 1
fi

# ── Verify binary exists ──────────────────────────────────────────────────────
step "Verifying binary"

if [[ ! -f "$BIN" ]]; then
    err "Binary not found: $BIN"
    [[ -n "$TARGET" ]] && echo "    Build first: bash scripts/linux/build.sh -p $TARGET"
    exit 1
fi
ok "Binary: $BIN ($(du -h "$BIN" | cut -f1))"

# ── Verify CMSIS-DAP device is visible ───────────────────────────────────────
step "Checking CMSIS-DAP device"

if ! lsusb 2>/dev/null | grep -qi "0d28\|cmsis\|daplink\|mbed"; then
    err "CMSIS-DAP device not found (lsusb)"
    echo ""
    echo "    Run in Windows PowerShell first:"
    echo "      .\\scripts\\windows\\attach-usb.ps1"
    echo ""
    echo "    If it is a udev permissions issue, run:"
    echo "      bash scripts/linux/flash.sh --setup-udev"
    exit 1
fi
ok "CMSIS-DAP detected"

# ── Flash ─────────────────────────────────────────────────────────────────────
step "Flashing → $ADDR"
echo "    Binary : $BIN"
echo "    Scripts: $OCD_SCRIPT_DIR"
echo ""

"$OCD_BIN" \
    -s "$OCD_SCRIPT_DIR" \
    -f interface/cmsis-dap.cfg \
    -f target/rt58x.cfg \
    -c "init; halt; flash write_image erase \"$BIN\" $ADDR; reset run; exit"

echo ""
echo -e "\n\033[32m[OK] Flash complete! Open a serial terminal (115200 8N1) to view output.\033[0m"
