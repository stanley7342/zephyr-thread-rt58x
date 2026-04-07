# cmake/zephyr_module/CMakeLists.txt
#
# Zephyr module entry point — included via ZEPHYR_EXTRA_MODULES during
# Zephyr's module loop. Contributes out-of-tree drivers and RT583 platform
# sources to Zephyr's library targets.
#
# NOTE: CMAKE_SOURCE_DIR points to the app source dir (e.g. examples/thread/),
# not the module root. Use CMAKE_CURRENT_LIST_DIR to derive the module root.
get_filename_component(_RT583_MODULE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(_RT583_PLATFORM ${_RT583_MODULE_ROOT}/sdk/components/platform)
set(_RT583_DRV_SRC  ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Src)
set(_RT583_SYS_SRC  ${_RT583_PLATFORM}/soc/rt583/rt583_system/Src)

# ── RT583 platform sources (always required) ──────────────────────────────
# These provide SystemInit, enable_perclk, delay_us, pin_set_mode, etc.
# needed by soc.c, the UART driver, and the flash driver regardless of
# which application (thread FTD CLI or MCUboot) is being built.
zephyr_compile_definitions(
    SET_SYS_CLK=2
    CHIP_TYPE=3
)

zephyr_sources(
    ${_RT583_SYS_SRC}/system_mcu.c
    ${_RT583_SYS_SRC}/mp_sector.c
    ${_RT583_DRV_SRC}/sysctrl.c
    ${_RT583_DRV_SRC}/sysfun.c
    ${_RT583_DRV_SRC}/flashctl.c
    ${_RT583_DRV_SRC}/flash_bp_protect.c
    ${_RT583_DRV_SRC}/gpio.c
    ${_RT583_DRV_SRC}/dma.c
    ${_RT583_DRV_SRC}/rtc.c
    ${_RT583_DRV_SRC}/swi.c
    ${_RT583_DRV_SRC}/timer.c
    ${_RT583_DRV_SRC}/trng.c
    ${_RT583_DRV_SRC}/wdt.c
)

zephyr_include_directories(
    ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
    ${_RT583_PLATFORM}/soc/rt583/rt583_system/Include
    ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
)

# ── UART driver (drivers__serial) ────────────────────────────────────────
if(CONFIG_UART_RT583)
    target_sources(drivers__serial PRIVATE
        ${_RT583_MODULE_ROOT}/drivers/serial/uart_rt583.c
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Src/hosal_uart.c
    )

    target_include_directories(drivers__serial PRIVATE
        ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
    )
endif()

# ── Entropy driver (drivers__entropy) ────────────────────────────────────
# Provides CONFIG_ENTROPY_HAS_DRIVER=y so that CONFIG_CSPRNG_ENABLED is set
# and Zephyr's OT platform entropy.c compiles without error.
if(CONFIG_ENTROPY_RT583 AND TARGET drivers__entropy)
    target_sources(drivers__entropy PRIVATE
        ${_RT583_MODULE_ROOT}/drivers/entropy/entropy_rt583.c
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Src/hosal_trng.c
    )
    target_include_directories(drivers__entropy PRIVATE
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
        ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
    )
endif()

# ── IEEE 802.15.4 stub driver (drivers__ieee802154) ───────────────────────
# Provides DEVICE_DT_DEFINE for the rt583 ieee802154 DTS node so that
# Zephyr's openthread/platform/radio.c can resolve DT_CHOSEN(zephyr_ieee802154).
if(CONFIG_IEEE802154_RT583 AND TARGET drivers__ieee802154)
    target_sources(drivers__ieee802154 PRIVATE
        ${_RT583_MODULE_ROOT}/drivers/ieee802154/ieee802154_rt583.c
    )
    target_include_directories(drivers__ieee802154 PRIVATE
        ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
    )
endif()

# ── OpenThread public headers (for Matter GN build) ──────────────────────
# When CONFIG_OPENTHREAD_RT583=y, Zephyr's OT module does NOT add
# modules/lib/openthread/include to the global system include path
# (CONFIG_OPENTHREAD_SOURCES=n).  Add it here so that:
#  1. Zephyr's OT L2 layer (openthread.c) can find <openthread/instance.h>
#  2. Matter's chip-module picks it up via zephyr_get_compile_flags() and
#     writes it into the GN build's args.gn target_cflags — this must happen
#     during find_package(Zephyr) module processing (before chip-module runs).
# The SDK's own OT headers are added as PRIVATE -I in
# subsys/openthread/CMakeLists.txt (higher priority, not visible here).
if(CONFIG_OPENTHREAD_RT583)
    zephyr_system_include_directories(
        ${ZEPHYR_BASE}/../modules/lib/openthread/include
        ${ZEPHYR_BASE}/../modules/lib/openthread/examples/platforms
    )
endif()

# ── Flash driver (drivers__flash) ────────────────────────────────────────
# flash_rt583.c includes <zephyr/drivers/flash.h> (Zephyr flash API).
# flash_rt583_hal.c includes "flashctl.h" (RT583 HW API).
# They MUST be separate TUs because both headers define flash_erase().
if(CONFIG_FLASH_RT583 AND TARGET drivers__flash)
    target_sources(drivers__flash PRIVATE
        ${_RT583_MODULE_ROOT}/drivers/flash/flash_rt583.c
        ${_RT583_MODULE_ROOT}/drivers/flash/flash_rt583_hal.c
    )

    target_include_directories(drivers__flash PRIVATE
        ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
        ${_RT583_PLATFORM}/soc/rt583/rt583_system/Include
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
    )
endif()
