#!/usr/bin/env bash
# RT582-EVB Zephyr + OpenThread — Linux/WSL 環境建置腳本
#
# 用法：
#   bash scripts/linux/install.sh
#   bash scripts/linux/install.sh --sdk-dir ~/zephyr-sdk-1.0.1

set -euo pipefail

SDK_DIR="${HOME}/zephyr-sdk-1.0.1"

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

# ── 步驟 1：必要工具 ──────────────────────────────────────────────────────────
step "檢查並安裝必要工具"

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

printf "    %-20s  %-20s  %s\n" "套件" "apt 名稱" "狀態"
printf "    %-20s  %-20s  %s\n" "--------------------" "--------------------" "------"

for pkg in git cmake ninja-build python3 python3-pip python3-venv wget xz-utils; do
    name="${PKG_NAMES[$pkg]}"
    if dpkg -s "$pkg" &>/dev/null; then
        printf "    %-20s  %-20s  \033[90m已安裝\033[0m\n" "$name" "$pkg"
    else
        printf "    %-20s  %-20s  \033[33m待安裝\033[0m\n" "$name" "$pkg"
        PKGS+=("$pkg")
    fi
done
echo ""

if [[ ${#PKGS[@]} -gt 0 ]]; then
    step "安裝缺少的工具（共 ${#PKGS[@]} 項）"
    sudo apt-get update -qq
    sudo apt-get install -y "${PKGS[@]}"
    ok "工具安裝完成"
else
    ok "所有工具已安裝，略過"
fi

PYTHON3="$(command -v python3)"
ok "Python 3：$PYTHON3 ($($PYTHON3 --version))"

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
step "安裝 Zephyr SDK 1.0.1 → $SDK_DIR"

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
        warn "快取檔 $TMP 不是有效的 tar.xz，重新下載..."
        rm -f "$TMP"
    fi

    if [[ ! -f "$TMP" ]]; then
        echo "    下載 $SDK_FILE ..."
        echo "    URL: $SDK_URL"
        wget -q --show-progress -O "$TMP" "$SDK_URL"
        if ! check_xz_magic "$TMP"; then
            rm -f "$TMP"
            echo "下載失敗：非有效的 tar.xz 檔案。請確認 URL：$SDK_URL" >&2
            exit 1
        fi
    else
        skip "SDK 壓縮檔（$TMP）"
    fi

    mkdir -p "$(dirname "$SDK_DIR")"
    echo "    解壓縮 SDK ..."
    tar -xf "$TMP" -C "$(dirname "$SDK_DIR")"
    echo "    執行 setup.sh ..."
    bash "$SDK_SETUP"
    ok "Zephyr SDK 安裝完成"
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
echo -e "  \033[33mbash scripts/linux/build.sh\033[0m"
