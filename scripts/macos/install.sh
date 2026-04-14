#!/usr/bin/env bash
# RT583-EVB Zephyr + OpenThread — macOS environment setup script
#
# Usage:
#   bash scripts/macos/install.sh
#   bash scripts/macos/install.sh --sdk-dir ~/zephyr-sdk-1.0.1

set -euo pipefail

# SDK version depends on architecture:
#   Apple Silicon (aarch64): v1.0.1
#   Intel (x86_64):          v0.17.0 (last version with x86_64 macOS support)
_HOST_ARCH="$(uname -m)"
if [[ "$_HOST_ARCH" == "arm64" || "$_HOST_ARCH" == "aarch64" ]]; then
    _SDK_VER="1.0.1"
else
    _SDK_VER="0.17.0"
fi
SDK_DIR="${HOME}/zephyr-sdk-${_SDK_VER}"

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

# ── Step 0: Homebrew ──────────────────────────────────────────────────────────
step "Checking Homebrew"
if ! command -v brew &>/dev/null; then
    echo "    Homebrew not found, installing..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    if [[ -f /opt/homebrew/bin/brew ]]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
    fi
fi
ok "Homebrew: $(brew --version | head -1)"

step "Updating Homebrew"
brew update 2>/dev/null || {
    warn "brew update failed, clearing cache and retrying..."
    rm -rf "$(brew --cache)" 2>/dev/null
    brew update
}
ok "Homebrew updated"

# ── Step 1: Required tools ────────────────────────────────────────────────────
step "Checking and installing required tools"

BREW_ORDER=(wget git cmake ninja "python@3.12" xz)
BREW_NAMES=("wget" "Git" "CMake" "Ninja" "Python 3.12" "xz")
TO_INSTALL=()

printf "    %-14s %-14s %-14s %s\n" "Package" "brew" "Version" "Status"
printf "    %-14s %-14s %-14s %s\n" "--------------" "--------------" "--------------" "--------"

for i in "${!BREW_ORDER[@]}"; do
    pkg="${BREW_ORDER[$i]}"
    name="${BREW_NAMES[$i]}"
    if brew list --formula 2>/dev/null | grep -q "^${pkg%%@*}$"; then
        ver="$(brew list --versions "${pkg%%@*}" 2>/dev/null | awk '{print $2}')"
        printf "    %-14s %-14s %-14s \033[32mOK\033[0m\n" "$name" "$pkg" "$ver"
    else
        printf "    %-14s %-14s %-14s \033[33mINSTALL\033[0m\n" "$name" "$pkg" "-"
        TO_INSTALL+=("$pkg")
    fi
done
echo ""

if [[ ${#TO_INSTALL[@]} -gt 0 ]]; then
    step "Installing missing tools (${#TO_INSTALL[@]} total)"
    for pkg in "${TO_INSTALL[@]}"; do
        echo "    Installing $pkg ..."
        brew install "$pkg" 2>&1 || {
            warn "$pkg installation failed, clearing cache and retrying..."
            rm -rf "$(brew --cache)" 2>/dev/null
            brew install "$pkg"
        }
    done
    ok "Tools installed"
else
    ok "All tools already installed, nothing to do"
fi

PYTHON3="$(brew --prefix python@3.12)/bin/python3.12"
[[ -f "$PYTHON3" ]] || PYTHON3="$(command -v python3.12 2>/dev/null || command -v python3)"
ok "Python: $PYTHON3 ($($PYTHON3 --version))"

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
step "Installing Zephyr SDK ${_SDK_VER} → $SDK_DIR"

SDK_SETUP="$SDK_DIR/setup.sh"

if [[ -f "$SDK_SETUP" ]]; then
    skip "Zephyr SDK"
else
    SDK_VER="$_SDK_VER"
    ARCH="$(uname -m)"
    [[ "$ARCH" == "arm64" ]] && ARCH="aarch64"
    # v1.0.1+ uses _gnu suffix; v0.17.0 does not
    if [[ "$SDK_VER" == "1.0.1" ]]; then
        SDK_FILE="zephyr-sdk-${SDK_VER}_macos-${ARCH}_gnu.tar.xz"
    else
        SDK_FILE="zephyr-sdk-${SDK_VER}_macos-${ARCH}.tar.xz"
    fi
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
        curl -L --progress-bar -o "$TMP" "$SDK_URL"
        if ! check_xz_magic "$TMP"; then
            rm -f "$TMP"; echo "Download failed: not a valid tar.xz file" >&2; exit 1
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
    if [[ "$SDK_VER" == "1.0.1" ]]; then
        bash "$SDK_SETUP"
    else
        # v0.17.0: -t <toolchain> to install, -c to register CMake package
        bash "$SDK_SETUP" -t arm-zephyr-eabi -h -c
    fi
    ok "Zephyr SDK ${SDK_VER} installed"
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
echo -e "  \033[33mbash scripts/macos/build.sh\033[0m"
