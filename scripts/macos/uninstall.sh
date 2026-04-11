#!/usr/bin/env bash
# RT583-EVB Zephyr + OpenThread — macOS uninstall script
#
# Removes components installed by install.sh (SDK, .west, zephyr, Python venv, env.sh).
# Homebrew packages are NOT removed automatically.
#
# Usage:
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
        *) echo "Unknown argument: $1"; exit 1 ;;
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
echo -e "\033[33mRemoving Zephyr development environment (macOS)\033[0m"
echo ""
printf "    %-${C1}s  %-${C2}s  %s\n" "Item" "Path / Package" "Status"
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
        print_row "$name" "$path" "pending" "yellow"
    else
        print_row "$name" "$path" "not found" "gray"
    fi
done

# brew packages: show only, do not remove automatically
BREW_PKGS=(cmake ninja python@3.12 xz)
for pkg in "${BREW_PKGS[@]}"; do
    if brew list --formula 2>/dev/null | grep -q "^${pkg%%@*}$"; then
        print_row "$pkg (brew)" "$pkg" "installed (not auto-removed)" "gray"
    fi
done

echo ""

if [[ $FORCE -eq 0 ]]; then
    read -r -p "Remove all items listed above? (y/N) " confirm
    if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
        echo "Cancelled."; exit 0
    fi
else
    echo -e "\033[90m(--force, auto-confirmed)\033[0m"
fi

echo ""

for name in "${DIR_ORDER[@]}"; do
    path="${DIR_ITEMS[$name]}"
    if [[ -e "$path" ]]; then
        echo "  Removing $name ..."
        rm -rf "$path"
        echo -e "  \033[32m[OK] $name removed\033[0m"
    fi
done

echo ""
echo -e "\033[36mUninstall complete.\033[0m"
echo ""
echo "Note: brew packages must be removed manually:"
echo "  brew uninstall cmake ninja python@3.12 xz"
