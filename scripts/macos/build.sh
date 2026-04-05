#!/usr/bin/env bash
# RT582-EVB Zephyr + OpenThread — macOS 編譯腳本
#
# 用法：
#   bash scripts/macos/build.sh              # Clean build（預設）
#   bash scripts/macos/build.sh --incremental

set -euo pipefail

PRISTINE="always"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --incremental|-i) PRISTINE="if_changed"; shift ;;
        *) echo "未知參數：$1"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
PROJECT_NAME="$(basename "$PROJECT_DIR")"
WORKSPACE="$(dirname "$PROJECT_DIR")"
ENV_SH="$WORKSPACE/env.sh"

if [[ ! -f "$ENV_SH" ]]; then
    echo "找不到 $ENV_SH，請先執行：" >&2
    echo "  bash scripts/macos/install.sh" >&2
    exit 1
fi

# shellcheck source=/dev/null
source "$ENV_SH"

MODE="$([ "$PRISTINE" = "always" ] && echo "Clean" || echo "增量")"

echo ""
echo -e "\033[36m==> west build（rt582_evb）\033[0m"
echo "    Mode    : $MODE"
echo "    BuildDir: $PROJECT_DIR/build"
echo ""

cd "$WORKSPACE"
west build -p "$PRISTINE" -b rt582_evb "$PROJECT_NAME" --build-dir "$PROJECT_NAME/build"

BIN="$PROJECT_DIR/build/zephyr/zephyr.bin"
if [[ -f "$BIN" ]]; then
    echo ""
    echo -e "    \033[32m[OK] 編譯成功：$BIN\033[0m"
fi
