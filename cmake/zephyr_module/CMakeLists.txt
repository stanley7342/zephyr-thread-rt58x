# cmake/zephyr_module/CMakeLists.txt
#
# Zephyr module entry point — included via ZEPHYR_EXTRA_MODULES during
# Zephyr's module loop. Contributes out-of-tree drivers to Zephyr's
# driver library targets.

# ── UART driver (drivers__serial) ────────────────────────────────────────
if(CONFIG_UART_RT582)
    set(_RT582_PLATFORM ${CMAKE_SOURCE_DIR}/sdk/components/platform)

    target_sources(drivers__serial PRIVATE
        ${CMAKE_SOURCE_DIR}/drivers/serial/uart_rt582.c
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Src/hosal_uart.c
    )

    target_include_directories(drivers__serial PRIVATE
        ${_RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Inc
    )
endif()

# ── Flash driver (drivers__flash) ────────────────────────────────────────
if(CONFIG_FLASH_RT582)
    set(_RT582_PLATFORM ${CMAKE_SOURCE_DIR}/sdk/components/platform)

    target_sources(drivers__flash PRIVATE
        ${CMAKE_SOURCE_DIR}/drivers/flash/flash_rt582.c
    )

    target_include_directories(drivers__flash PRIVATE
        ${_RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
        ${_RT582_PLATFORM}/soc/rt582/rt582_system/Include
        ${_RT582_PLATFORM}/hosal/rt582_hosal/Inc
    )
endif()
