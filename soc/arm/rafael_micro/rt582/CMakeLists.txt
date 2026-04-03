# Use Zephyr's standard Cortex-M linker script
set(SOC_LINKER_SCRIPT
    ${ZEPHYR_BASE}/include/zephyr/arch/arm/cortex_m/scripts/linker.ld
    CACHE INTERNAL "")

zephyr_sources(soc.c)

# Platform drivers are compiled from source under drivers/ (no pre-built .a needed).
zephyr_include_directories(
    ${CMAKE_SOURCE_DIR}/drivers/soc/rt582/rt582_driver/Inc
    ${CMAKE_SOURCE_DIR}/drivers/soc/rt582/rt582_system/Include
)
