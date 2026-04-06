#!/usr/bin/env bash
# RT582-EVB Zephyr + OpenThread — macOS 環境建置腳本
#
# 用法：
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
        *) echo "未知參數：$1"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
WORKSPACE="$(dirname "$PROJECT_DIR")"
PROJECT_NAME="$(basename "$PROJECT_DIR")"

step()  { echo -e "\n\033[36m==> $*\033[0m"; }
ok()    { echo -e "    \033[32m[OK]\033[0m $*"; }
skip()  { echo -e "    \033[90m[--]\033[0m $* (已存在，略過)"; }
warn()  { echo -e "    \033[33m[!!]\033[0m $*"; }

# ── 步驟 0：Homebrew ──────────────────────────────────────────────────────────
step "檢查 Homebrew"
if ! command -v brew &>/dev/null; then
    echo "    未偵測到 Homebrew，正在安裝..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    if [[ -f /opt/homebrew/bin/brew ]]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
    fi
fi
ok "Homebrew：$(brew --version | head -1)"

step "更新 Homebrew"
brew update 2>/dev/null || {
    warn "brew update 失敗，嘗試清除快取後重試..."
    rm -rf "$(brew --cache)" 2>/dev/null
    brew update
}
ok "Homebrew 已更新"

# ── 步驟 1：必要工具 ──────────────────────────────────────────────────────────
step "檢查並安裝必要工具"

BREW_ORDER=(wget git cmake ninja "python@3.12" xz)
BREW_NAMES=("wget" "Git" "CMake" "Ninja" "Python 3.12" "xz")
TO_INSTALL=()

printf "    %-20s  %-20s  %-20s  %s\n" "Package" "brew name" "Version" "Status"
printf "    %-20s  %-20s  %-20s  %s\n" "--------------------" "--------------------" "--------------------" "------"

for i in "${!BREW_ORDER[@]}"; do
    pkg="${BREW_ORDER[$i]}"
    name="${BREW_NAMES[$i]}"
    if brew list --formula 2>/dev/null | grep -q "^${pkg%%@*}$"; then
        ver="$(brew list --versions "${pkg%%@*}" 2>/dev/null | awk '{print $2}')"
        printf "    %-20s  %-20s  %-16s  \033[90m已安裝\033[0m\n" "$name" "$pkg" "$ver"
    else
        printf "    %-20s  %-20s  %-16s  \033[33m待安裝\033[0m\n" "$name" "$pkg" "-"
        TO_INSTALL+=("$pkg")
    fi
done
echo ""

if [[ ${#TO_INSTALL[@]} -gt 0 ]]; then
    step "安裝缺少的工具（共 ${#TO_INSTALL[@]} 項）"
    for pkg in "${TO_INSTALL[@]}"; do
        echo "    安裝 $pkg ..."
        brew install "$pkg" 2>&1 || {
            warn "$pkg 安裝失敗，清除快取後重試..."
            rm -rf "$(brew --cache)" 2>/dev/null
            brew install "$pkg"
        }
    done
    ok "工具安裝完成"
else
    ok "所有工具已安裝，略過"
fi

PYTHON3="$(brew --prefix python@3.12)/bin/python3.12"
[[ -f "$PYTHON3" ]] || PYTHON3="$(command -v python3.12 2>/dev/null || command -v python3)"
ok "Python：$PYTHON3 ($($PYTHON3 --version))"

# ── 步驟 2：建立 venv 並安裝 west ────────────────────────────────────────────
step "建立 Python venv 並安裝 west"

VENV_DIR="$HOME/.zephyr-venv"
if [[ ! -f "$VENV_DIR/bin/activate" ]]; then
    "$PYTHON3" -m venv "$VENV_DIR"
    ok "venv 建立於 $VENV_DIR"
else
    skip "venv（$VENV_DIR）"
fi

source "$VENV_DIR/bin/activate"
PYTHON3="$VENV_DIR/bin/python3"
pip install --quiet --upgrade pip
pip install --quiet west
ok "west：$(west --version 2>/dev/null || echo 已安裝)"

# ── 步驟 3：Zephyr SDK ────────────────────────────────────────────────────────
step "安裝 Zephyr SDK ${_SDK_VER} → $SDK_DIR"

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
        warn "快取檔 $TMP 不是有效的 tar.xz，重新下載..."
        rm -f "$TMP"
    fi

    if [[ ! -f "$TMP" ]]; then
        echo "    下載 $SDK_FILE ..."
        echo "    URL: $SDK_URL"
        curl -L --progress-bar -o "$TMP" "$SDK_URL"
        if ! check_xz_magic "$TMP"; then
            rm -f "$TMP"; echo "下載失敗：非有效的 tar.xz 檔案" >&2; exit 1
        fi
    else
        skip "SDK 壓縮檔（$TMP）"
    fi

    mkdir -p "$(dirname "$SDK_DIR")"
    echo "    解壓縮 SDK ..."
    tar -xvf "$TMP" -C "$(dirname "$SDK_DIR")" | while IFS= read -r line; do
        printf "\r    \033[90m%-72s\033[0m" "${line:0:72}"
    done
    echo ""
    echo "    執行 setup.sh ..."
    if [[ "$SDK_VER" == "1.0.1" ]]; then
        bash "$SDK_SETUP"
    else
        # v0.17.0: register SDK and install only the ARM toolchain
        bash "$SDK_SETUP" -c arm-zephyr-eabi
    fi
    ok "Zephyr SDK ${SDK_VER} 安裝完成"
fi

# ── 步驟 4：West 工作區 ───────────────────────────────────────────────────────
step "建立 west 工作區 → $WORKSPACE"

WEST_CONFIG="$WORKSPACE/.west/config"
ZEPHYR_DIR="$WORKSPACE/zephyr"
unset ZEPHYR_BASE

if [[ ! -f "$WEST_CONFIG" ]]; then
    echo "    west init ..."
    cd "$WORKSPACE"
    west init -l "$PROJECT_NAME"
    [[ -f "$WEST_CONFIG" ]] || { echo "west init 成功但找不到 $WEST_CONFIG" >&2; exit 1; }
    ok "west init 完成"
else
    skip "west init"
fi

REQ="$ZEPHYR_DIR/scripts/requirements-base.txt"
if [[ ! -f "$REQ" ]]; then
    echo "    west update（下載 Zephyr，約 500 MB）..."
    cd "$WORKSPACE"
    west update
    ok "west update 完成"
else
    skip "zephyr（已下載）"
fi

[[ -f "$REQ" ]] || { echo "west update 後仍找不到：$REQ" >&2; exit 1; }

# ── 步驟 5：Python 依賴 ───────────────────────────────────────────────────────
step "安裝 Zephyr Python 依賴"
pip install --quiet -r "$REQ"
ok "Python 依賴安裝完成"

# ── 步驟 6：產生 env.sh ───────────────────────────────────────────────────────
step "產生 $WORKSPACE/env.sh"

ENV_SH="$WORKSPACE/env.sh"
cat > "$ENV_SH" <<EOF
# Zephyr 環境變數 — 每次開啟新 shell 執行: source $ENV_SH
source "$VENV_DIR/bin/activate"
export ZEPHYR_BASE="$ZEPHYR_DIR"
export ZEPHYR_TOOLCHAIN_VARIANT="zephyr"
export ZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"

echo "Zephyr 環境已載入（ZEPHYR_BASE=\$ZEPHYR_BASE）"
EOF
chmod +x "$ENV_SH"
ok "env.sh 產生完成"
echo "    載入方式：source $ENV_SH"

echo ""
echo -e "\033[36m======================================\033[0m"
echo -e "\033[36m  環境建置完成！\033[0m"
echo -e "\033[36m======================================\033[0m"
echo ""
echo "每次開啟新 shell 請先執行："
echo -e "  \033[33msource $ENV_SH\033[0m"
echo ""
echo "編譯請使用："
echo -e "  \033[33mbash scripts/macos/build.sh\033[0m"
