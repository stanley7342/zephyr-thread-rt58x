#!/usr/bin/env bash
# RT582-EVB Zephyr + OpenThread — Linux/WSL 移除腳本
#
# 移除 install.sh 安裝的元件（SDK、.west、zephyr、west pip、env.sh）。
# apt 安裝的套件不會自動移除。
#
# 用法：
#   bash scripts/linux/uninstall.sh
#   bash scripts/linux/uninstall.sh --sdk-dir ~/zephyr-sdk-1.0.1
#   bash scripts/linux/uninstall.sh --force

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

echo ""
echo -e "\033[33m移除 Zephyr 開發環境（Linux/WSL）\033[0m"
echo ""
printf "    %-${C1}s  %-${C2}s  %s\n" "項目" "路徑 / 套件" "狀態"
printf "    %-${C1}s  %-${C2}s  %s\n" "$(printf '%0.s-' $(seq 1 $C1))" "$(printf '%0.s-' $(seq 1 $C2))" "------"

declare -A DIR_ITEMS=(
    [".west"]="$WORKSPACE/.west"
    ["zephyr"]="$WORKSPACE/zephyr"
    ["env.sh"]="$WORKSPACE/env.sh"
    ["Zephyr SDK"]="$SDK_DIR"
)
DIR_ORDER=(".west" "zephyr" "env.sh" "Zephyr SDK")

for name in "${DIR_ORDER[@]}"; do
    path="${DIR_ITEMS[$name]}"
    if [[ -e "$path" ]]; then
        print_row "$name" "$path" "待移除" "yellow"
    else
        print_row "$name" "$path" "不存在" "gray"
    fi
done

WEST_INSTALLED=0
if python3 -m pip show west &>/dev/null 2>&1; then
    WEST_INSTALLED=1
    print_row "west (pip)" "pip uninstall west" "待移除" "yellow"
else
    print_row "west (pip)" "pip uninstall west" "未安裝" "gray"
fi

APT_PKGS=(git cmake ninja-build python3 python3-pip wget xz-utils)
for pkg in "${APT_PKGS[@]}"; do
    if dpkg -s "$pkg" &>/dev/null 2>&1; then
        print_row "$pkg (apt)" "$pkg" "已安裝（不自動移除）" "gray"
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

if [[ $WEST_INSTALLED -eq 1 ]]; then
    echo "  pip uninstall west ..."
    python3 -m pip uninstall west -y --quiet
    echo -e "  \033[32m[OK] west (pip) 已移除\033[0m"
fi

echo ""
echo -e "\033[36m移除完成。\033[0m"
echo ""
echo "注意：apt 安裝的套件（git、cmake、ninja-build 等）需手動移除："
echo "  sudo apt-get remove git cmake ninja-build python3-pip wget xz-utils"
