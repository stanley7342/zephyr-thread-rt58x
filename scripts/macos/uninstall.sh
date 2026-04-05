#!/usr/bin/env bash
# RT582-EVB Zephyr + OpenThread — macOS 移除腳本
#
# 移除 install.sh 安裝的元件（SDK、.west、zephyr、Python venv、env.sh）。
# brew 安裝的套件不會自動移除。
#
# 用法：
#   bash scripts/macos/uninstall.sh
#   bash scripts/macos/uninstall.sh --sdk-dir ~/zephyr-sdk-1.0.1
#   bash scripts/macos/uninstall.sh --force

set -euo pipefail

SDK_DIR="${HOME}/zephyr-sdk-1.0.1"
FORCE=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --sdk-dir) SDK_DIR="$2"; shift 2 ;;
        --force|-f) FORCE=1; shift ;;
        *) echo "未知參數：$1"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
WORKSPACE="$(dirname "$PROJECT_DIR")"

C1=18
C2=45

yellow() { echo -e "\033[33m$*\033[0m"; }
gray()   { echo -e "\033[90m$*\033[0m"; }

print_row() {
    local name="$1" path="$2" status="$3" color="$4"
    printf "    %-${C1}s  %-${C2}s  " "$name" "$path"
    case "$color" in
        yellow) yellow "$status" ;;
        gray)   gray   "$status" ;;
    esac
}

VENV_DIR="$HOME/.zephyr-venv"

echo ""
echo -e "\033[33m移除 Zephyr 開發環境（macOS）\033[0m"
echo ""
printf "    %-${C1}s  %-${C2}s  %s\n" "項目" "路徑 / 套件" "狀態"
printf "    %-${C1}s  %-${C2}s  %s\n" "$(printf '%0.s-' $(seq 1 $C1))" "$(printf '%0.s-' $(seq 1 $C2))" "------"

declare -A DIR_ITEMS=(
    [".west"]="$WORKSPACE/.west"
    ["zephyr"]="$WORKSPACE/zephyr"
    ["env.sh"]="$WORKSPACE/env.sh"
    ["Zephyr SDK"]="$SDK_DIR"
    ["Python venv"]="$VENV_DIR"
)
DIR_ORDER=(".west" "zephyr" "env.sh" "Zephyr SDK" "Python venv")

for name in "${DIR_ORDER[@]}"; do
    path="${DIR_ITEMS[$name]}"
    if [[ -e "$path" ]]; then
        print_row "$name" "$path" "待移除" "yellow"
    else
        print_row "$name" "$path" "不存在" "gray"
    fi
done

# brew 套件僅顯示，不自動移除
BREW_PKGS=(cmake ninja python@3.12 xz)
for pkg in "${BREW_PKGS[@]}"; do
    if brew list --formula 2>/dev/null | grep -q "^${pkg%%@*}$"; then
        print_row "$pkg (brew)" "$pkg" "已安裝（不自動移除）" "gray"
    fi
done

echo ""

if [[ $FORCE -eq 0 ]]; then
    read -r -p "確認移除以上所有項目？(y/N) " confirm
    if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
        echo "已取消。"; exit 0
    fi
else
    echo -e "\033[90m（--force，自動確認）\033[0m"
fi

echo ""

for name in "${DIR_ORDER[@]}"; do
    path="${DIR_ITEMS[$name]}"
    if [[ -e "$path" ]]; then
        echo "  移除 $name ..."
        rm -rf "$path"
        echo -e "  \033[32m[OK] $name 已移除\033[0m"
    fi
done

echo ""
echo -e "\033[36m移除完成。\033[0m"
echo ""
echo "注意：brew 安裝的套件需手動移除："
echo "  brew uninstall cmake ninja python@3.12 xz"
