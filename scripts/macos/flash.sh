#!/usr/bin/env bash
# RT583-EVB — macOS flash script (CMSIS-DAP + OpenOCD)
#
# Usage:
#   bash scripts/macos/flash.sh -p thread        # slot0 (0x10000)
#   bash scripts/macos/flash.sh -p bootloader    # 0x00000000
#   bash scripts/macos/flash.sh -p thread --addr 0x0   # override address
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
        *) echo "Usage: $0 -p <thread|bootloader> [--addr <hex>] [--bin <path>]" >&2; exit 1 ;;
    esac
done

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

# ── Locate openocd-rt58x ──────────────────────────────────────────────────────
step "Locating openocd-rt58x"

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
        ok "openocd-rt58x: $OCD_BIN"
        [[ -n "${OCD_SCRIPT_DIR:-}" ]] && ok "Scripts   : $OCD_SCRIPT_DIR"
        break
    fi
done

if [[ -z "$OCD_BIN" ]]; then
    SYS_OCD="$(command -v openocd 2>/dev/null || true)"
    if [[ -n "$SYS_OCD" ]]; then
        OCD_BIN="$SYS_OCD"
        OCD_SCRIPT_DIR="$TOOLS_LINUX/tcl"
        ok "Using system openocd: $OCD_BIN (with project tcl scripts)"
    fi
fi

if [[ -z "$OCD_BIN" ]]; then
    err "openocd not found"
    echo ""
    echo "    Install options:"
    echo ""
    echo "    1) Homebrew:"
    echo "       brew install openocd"
    echo ""
    echo "    2) Build openocd-rt58x from source:"
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
    err "OpenOCD tcl scripts directory not found"
    exit 1
fi

# ── Verify binary exists ──────────────────────────────────────────────────────
step "Verifying binary"

if [[ ! -f "$BIN" ]]; then
    err "Binary not found: $BIN"
    [[ -n "$TARGET" ]] && echo "    Build first: bash scripts/macos/build.sh -p $TARGET"
    exit 1
fi
ok "Binary: $BIN ($(du -h "$BIN" | awk '{print $1}'))"

# ── Verify CMSIS-DAP device is visible ───────────────────────────────────────
step "Checking CMSIS-DAP device"

USB_INFO="$(system_profiler SPUSBDataType 2>/dev/null || true)"
if ! echo "$USB_INFO" | grep -qi "cmsis.dap\|daplink\|mbed\|0d28"; then
    err "CMSIS-DAP device not found"
    echo ""
    echo "    Ensure the CMSIS-DAP debugger is connected via USB and powered on"
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
