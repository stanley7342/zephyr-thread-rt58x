# cmake/zephyr_module/CMakeLists.txt
#
# Zephyr module entry point — included via ZEPHYR_EXTRA_MODULES during
# Zephyr's module loop. Contributes out-of-tree drivers and RT582 platform
# sources to Zephyr's library targets.
#
# NOTE: CMAKE_SOURCE_DIR points to the app source dir (e.g. examples/thread/),
# not the module root. Use CMAKE_CURRENT_LIST_DIR to derive the module root.
get_filename_component(_RT582_MODULE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(_RT582_PLATFORM ${_RT582_MODULE_ROOT}/sdk/components/platform)
set(_RT582_DRV_SRC  ${_RT582_PLATFORM}/soc/rt582/rt582_driver/Src)
set(_RT582_SYS_SRC  ${_RT582_PLATFORM}/soc/rt582/rt582_system/Src)

# ── RT582 platform sources (always required) ──────────────────────────────
# These provide SystemInit, enable_perclk, delay_us, pin_set_mode, etc.
# needed by soc.c, the UART driver, and the flash driver regardless of
# which application (thread FTD CLI or MCUboot) is being built.
zephyr_compile_definitions(
    SET_SYS_CLK=2
    CHIP_TYPE=2
)

zephyr_sources(
    ${_RT582_SYS_SRC}/system_mcu.c
    ${_RT582_SYS_SRC}/mp_sector.c
    ${_RT582_DRV_SRC}/sysctrl.c
    ${_RT582_DRV_SRC}/sysfun.c
    ${_RT582_DRV_SRC}/flashctl.c
    ${_RT582_DRV_SRC}/flash_bp_protect.c
    ${_RT582_DRV_SRC}/gpio.c
    ${_RT582_DRV_SRC}/dma.c
    ${_RT582_DRV_SRC}/rtc.c
    ${_RT582_DRV_SRC}/swi.c
    ${_RT582_DRV_SRC}/timer.c
    ${_RT582_DRV_SRC}/trng.c
    ${_RT582_DRV_SRC}/wdt.c
)

zephyr_include_directories(
    ${_RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
    ${_RT582_PLATFORM}/soc/rt582/rt582_system/Include
    ${_RT582_PLATFORM}/hosal/rt582_hosal/Inc
)

# ── UART driver (drivers__serial) ────────────────────────────────────────
if(CONFIG_UART_RT582)
    target_sources(drivers__serial PRIVATE
        ${_RT582_MODULE_ROOT}/drivers/serial/uart_rt582.c
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Src/hosal_uart.c
    )

    target_include_directories(drivers__serial PRIVATE
        ${_RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Inc
    )
endif()

# ── BLE HCI driver (drivers__bluetooth) ─────────────────────────────────
if(CONFIG_BLE_RT582 AND TARGET drivers__bluetooth)
    target_sources(drivers__bluetooth PRIVATE
        ${_RT582_MODULE_ROOT}/drivers/bluetooth/hci_rt582.c
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Src/hosal_rf.c
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Src/hosal_lpm.c
    )

    target_include_directories(drivers__bluetooth PRIVATE
        ${_RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
        ${_RT582_PLATFORM}/soc/rt582/rt582_system/Include
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Inc
    )
endif()

# ── Flash driver (drivers__flash) ────────────────────────────────────────
# flash_rt582.c includes <zephyr/drivers/flash.h> (Zephyr flash API).
# flash_rt582_hal.c includes "flashctl.h" (RT582 HW API).
# They MUST be separate TUs because both headers define flash_erase().
if(CONFIG_FLASH_RT582 AND TARGET drivers__flash)
    target_sources(drivers__flash PRIVATE
        ${_RT582_MODULE_ROOT}/drivers/flash/flash_rt582.c
        ${_RT582_MODULE_ROOT}/drivers/flash/flash_rt582_hal.c
    )

    target_include_directories(drivers__flash PRIVATE
        ${_RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
        ${_RT582_PLATFORM}/soc/rt582/rt582_system/Include
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Inc
    )
endif()
