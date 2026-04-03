# Use Zephyr's standard Cortex-M linker script
set(SOC_LINKER_SCRIPT
    ${ZEPHYR_BASE}/include/zephyr/arch/arm/cortex_m/scripts/linker.ld
    CACHE INTERNAL "")

zephyr_sources(soc.c)

# Rafael IoT SDK vendored into sdk/ inside the application root.
set(RAFAEL_SDK ${CMAKE_SOURCE_DIR}/sdk)

# soc.h includes mcu.h; expose the Rafael SDK include paths globally.
zephyr_include_directories(
    ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_driver/Inc
    ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_system/Include
)
