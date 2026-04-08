#!/usr/bin/env bash
# RT583-EVB Zephyr + OpenThread — macOS 編譯腳本
#
# 用法：
#   bash scripts/macos/build.sh -p thread                  # Thread FTD CLI（Clean）
#   bash scripts/macos/build.sh -p thread -i               # Thread FTD CLI（增量）
#   bash scripts/macos/build.sh -p bootloader              # MCUboot（Clean）
#   bash scripts/macos/build.sh -p bootloader -i           # MCUboot（增量）
#
# 輸出：
#   thread     → build/thread/thread_zephyr.bin
#   bootloader → build/bootloader/bootloader_zephyr.bin

set -euo pipefail

TARGET=""
PRISTINE="always"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -p) shift; TARGET="${1:-}"; shift ;;
        --incremental|-i) PRISTINE="auto"; shift ;;
        *) echo "用法：$0 -p <thread|bootloader> [-i|--incremental]" >&2; exit 1 ;;
    esac
done

if [[ -z "$TARGET" ]]; then
    echo "錯誤：請指定 -p <thread|bootloader>" >&2
    echo "用法：$0 -p <thread|bootloader> [-i|--incremental]" >&2
    exit 1
fi

if [[ "$TARGET" != "thread" && "$TARGET" != "bootloader" ]]; then
    echo "錯誤：不支援的 target '$TARGET'，請使用 thread 或 bootloader" >&2
    exit 1
fi

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

# Skip ~/.zephyr-env — env.sh already sets the correct environment.
export ZEPHYR_NO_ENV=1

# Deactivate venv on exit (success or failure).
trap 'type deactivate &>/dev/null && deactivate' EXIT

MODE="$([ "$PRISTINE" = "always" ] && echo "Clean" || echo "增量")"
BUILD_DIR="$PROJECT_DIR/build/$TARGET"
OUT_BIN="$BUILD_DIR/${TARGET}_zephyr.bin"

if [[ "$TARGET" == "bootloader" ]]; then
    OVERLAY="$PROJECT_DIR/examples/bootloader/mcuboot.conf"

    echo ""
    echo -e "\033[36m==> west build（MCUboot / rt583_evb）\033[0m"
    echo "    Mode    : $MODE"
    echo "    BuildDir: $BUILD_DIR"
    echo "    Overlay : $OVERLAY"
    echo ""

    cd "$WORKSPACE"
    west build -p "$PRISTINE" -b rt583_evb \
        bootloader/mcuboot/boot/zephyr \
        --build-dir "$PROJECT_NAME/build/$TARGET" \
        -- -DOVERLAY_CONFIG="$OVERLAY" \
           -DBOARD_ROOT="$PROJECT_DIR" \
           -DSOC_ROOT="$PROJECT_DIR" \
           -DDTS_ROOT="$PROJECT_DIR"

else
    echo ""
    echo -e "\033[36m==> west build（Thread FTD CLI / rt583_evb）\033[0m"
    echo "    Mode    : $MODE"
    echo "    BuildDir: $BUILD_DIR"
    echo ""

    cd "$WORKSPACE"
    west build -p "$PRISTINE" -b rt583_evb \
        "$PROJECT_NAME/examples/thread" \
        --build-dir "$PROJECT_NAME/build/$TARGET"
fi

if [[ -f "$BUILD_DIR/zephyr/zephyr.bin" ]]; then
    cp "$BUILD_DIR/zephyr/zephyr.bin" "$OUT_BIN"
    echo ""
    echo -e "    \033[32m[OK] 編譯成功：$OUT_BIN\033[0m"
fi
