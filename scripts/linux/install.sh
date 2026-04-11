#!/usr/bin/env bash
# RT583-EVB Zephyr + OpenThread — Linux/WSL environment setup script
#
# Usage:
#   bash scripts/linux/install.sh
#   bash scripts/linux/install.sh --sdk-dir ~/zephyr-sdk-1.0.1

set -euo pipefail

SDK_DIR="${HOME}/zephyr-sdk-1.0.1"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --sdk-dir) SDK_DIR="$2"; shift 2 ;;
        *) echo "Unknown argument: $1"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
WORKSPACE="$(dirname "$PROJECT_DIR")"
PROJECT_NAME="$(basename "$PROJECT_DIR")"

step()  { echo -e "\n\033[36m==> $*\033[0m"; }
ok()    { echo -e "    \033[32m[OK]\033[0m $*"; }
skip()  { echo -e "    \033[90m[--]\033[0m $* (already exists, skipping)"; }
warn()  { echo -e "    \033[33m[!!]\033[0m $*"; }

# ── Step 1: Required tools ────────────────────────────────────────────────────
step "Checking and installing required tools"

PKGS=()
declare -A PKG_NAMES=(
    [git]="Git"
    [cmake]="CMake"
    [ninja-build]="Ninja"
    [python3]="Python 3"
    [python3-pip]="pip"
    [python3-venv]="python3-venv"
    [wget]="wget"
    [xz-utils]="xz-utils"
)

printf "    %-20s  %-20s  %-20s  %s\n" "Package" "apt name" "Version" "Status"
printf "    %-20s  %-20s  %-20s  %s\n" "--------------------" "--------------------" "--------------------" "------"

for pkg in wget git cmake ninja-build python3 python3-pip python3-venv xz-utils; do
    name="${PKG_NAMES[$pkg]}"
    if dpkg -s "$pkg" 2>/dev/null | grep -q "^Status: install ok installed"; then
        ver="$(dpkg -s "$pkg" 2>/dev/null | grep '^Version:' | awk '{print $2}')"
        ver="${ver:0:20}"
        printf "    %-20s  %-20s  %-20s  \033[90minstalled\033[0m\n" "$name" "$pkg" "$ver"
    else
        printf "    %-20s  %-20s  %-20s  \033[33mpending\033[0m\n" "$name" "$pkg" "-"
        PKGS+=("$pkg")
    fi
done
echo ""

if [[ ${#PKGS[@]} -gt 0 ]]; then
    step "Installing missing tools (${#PKGS[@]} total)"
    sudo apt-get update -qq
    sudo apt-get install -y "${PKGS[@]}"
    ok "Tools installed"
else
    ok "All tools already installed, nothing to do"
fi

PYTHON3="$(command -v python3)"
ok "Python 3: $PYTHON3 ($($PYTHON3 --version))"

# ── Step 2: Create venv and install west ──────────────────────────────────────
step "Creating Python venv and installing west"

VENV_DIR="$HOME/.zephyr-venv"
if [[ ! -f "$VENV_DIR/bin/activate" ]]; then
    "$PYTHON3" -m venv "$VENV_DIR"
    ok "venv created at $VENV_DIR"
else
    skip "venv ($VENV_DIR)"
fi

source "$VENV_DIR/bin/activate"
PYTHON3="$VENV_DIR/bin/python3"
pip install --quiet --upgrade pip
pip install --quiet west
ok "west: $(west --version 2>/dev/null || echo installed)"

# ── Step 3: Zephyr SDK ────────────────────────────────────────────────────────
step "Installing Zephyr SDK 1.0.1 → $SDK_DIR"

SDK_SETUP="$SDK_DIR/setup.sh"

if [[ -f "$SDK_SETUP" ]]; then
    skip "Zephyr SDK"
else
    SDK_VER="1.0.1"
    # Bundle tarball includes hosttools + all toolchains; _gnu suffix is required in v1.0.1+
    SDK_FILE="zephyr-sdk-${SDK_VER}_linux-x86_64_gnu.tar.xz"
    SDK_URL="https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VER}/${SDK_FILE}"
    TMP="/tmp/${SDK_FILE}"

    check_xz_magic() {
        [[ -f "$1" ]] || return 1
        local magic
        magic=$(xxd -p -l 6 "$1" 2>/dev/null)
        [[ "$magic" == "fd377a585a00" ]]
    }

    if [[ -f "$TMP" ]] && ! check_xz_magic "$TMP"; then
        warn "Cached file $TMP is not a valid tar.xz — re-downloading..."
        rm -f "$TMP"
    fi

    if [[ ! -f "$TMP" ]]; then
        echo "    Downloading $SDK_FILE ..."
        echo "    URL: $SDK_URL"
        wget -q --show-progress -O "$TMP" "$SDK_URL"
        if ! check_xz_magic "$TMP"; then
            rm -f "$TMP"
            echo "Download failed: not a valid tar.xz file. Check the URL: $SDK_URL" >&2
            exit 1
        fi
    else
        skip "SDK archive ($TMP)"
    fi

    mkdir -p "$(dirname "$SDK_DIR")"
    echo "    Extracting SDK ..."
    tar -xvf "$TMP" -C "$(dirname "$SDK_DIR")" | while IFS= read -r line; do
        printf "\r    \033[90m%-72s\033[0m" "${line:0:72}"
    done
    echo ""
    echo "    Running setup.sh ..."
    bash "$SDK_SETUP"
    ok "Zephyr SDK installed"
fi

# ── Step 4: West workspace ────────────────────────────────────────────────────
step "Setting up west workspace → $WORKSPACE"

WEST_CONFIG="$WORKSPACE/.west/config"
ZEPHYR_DIR="$WORKSPACE/zephyr"
unset ZEPHYR_BASE

if [[ ! -f "$WEST_CONFIG" ]]; then
    echo "    west init ..."
    cd "$WORKSPACE"
    west init -l "$PROJECT_NAME"
    [[ -f "$WEST_CONFIG" ]] || { echo "west init succeeded but $WEST_CONFIG not found" >&2; exit 1; }
    ok "west init complete"
else
    skip "west init"
fi

REQ="$ZEPHYR_DIR/scripts/requirements-base.txt"
if [[ ! -f "$REQ" ]]; then
    echo "    west update (downloading Zephyr, ~500 MB)..."
    cd "$WORKSPACE"
    west update
    ok "west update complete"
else
    skip "zephyr (already downloaded)"
fi

[[ -f "$REQ" ]] || { echo "Still cannot find: $REQ after west update" >&2; exit 1; }

# ── Step 5: Python dependencies ───────────────────────────────────────────────
step "Installing Zephyr Python dependencies"
pip install --quiet -r "$REQ"
ok "Python dependencies installed"

# ── Step 6: Generate env.sh ───────────────────────────────────────────────────
step "Generating $WORKSPACE/env.sh"

ENV_SH="$WORKSPACE/env.sh"
cat > "$ENV_SH" <<EOF
# Zephyr environment variables — load in every new shell session: source $ENV_SH
source "$VENV_DIR/bin/activate"
export ZEPHYR_BASE="$ZEPHYR_DIR"
export ZEPHYR_TOOLCHAIN_VARIANT="zephyr"
export ZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"

echo "Zephyr env loaded (ZEPHYR_BASE=\$ZEPHYR_BASE)"
EOF
chmod +x "$ENV_SH"
ok "env.sh generated"
echo "    Load with: source $ENV_SH"

echo ""
echo -e "\033[36m======================================\033[0m"
echo -e "\033[36m  Environment setup complete!\033[0m"
echo -e "\033[36m======================================\033[0m"
echo ""
echo "Load the environment in every new shell session:"
echo -e "  \033[33msource $ENV_SH\033[0m"
echo ""
echo "Build with:"
echo -e "  \033[33mbash scripts/linux/build.sh\033[0m"
