# Use Zephyr's standard Cortex-M linker script
set(SOC_LINKER_SCRIPT
    ${ZEPHYR_BASE}/include/zephyr/arch/arm/cortex_m/scripts/linker.ld
    CACHE INTERNAL "")

zephyr_sources(soc.c)

# Platform drivers are compiled from source under drivers/ (no pre-built .a needed).
# CMAKE_SOURCE_DIR points to the app source dir, not the repo root.
# Use CMAKE_CURRENT_LIST_DIR to derive the repo root (soc/arm/rafael_micro/rt583/ → ../../../../)
get_filename_component(_RT583_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)

zephyr_include_directories(
    ${_RT583_REPO_ROOT}/sdk/components/platform/soc/rt583/rt583_driver/Inc
    ${_RT583_REPO_ROOT}/sdk/components/platform/soc/rt583/rt583_system/Include
)
