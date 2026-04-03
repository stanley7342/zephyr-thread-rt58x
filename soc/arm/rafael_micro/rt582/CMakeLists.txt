# Use Zephyr's standard Cortex-M linker script
set(SOC_LINKER_SCRIPT
    ${ZEPHYR_BASE}/include/zephyr/arch/arm/cortex_m/scripts/linker.ld
    CACHE INTERNAL "")

zephyr_sources(soc.c)

# Locate the Rafael SDK.  CMAKE_SOURCE_DIR = application root (rt582-zephyr-helloworld/)
# The SDK lives one level above the application root.
set(RAFAEL_SDK $ENV{RAFAEL_SDK_BASE})
if(NOT RAFAEL_SDK)
    get_filename_component(RAFAEL_SDK
        "${CMAKE_SOURCE_DIR}/../Rafael-IoT-SDK-Internal"
        ABSOLUTE)
endif()

# soc.h includes mcu.h; expose the Rafael SDK include paths globally.
zephyr_include_directories(
    ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_driver/Inc
    ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_system/Include
)
