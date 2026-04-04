# cmake/zephyr_serial/CMakeLists.txt
#
# This cmake is included as a Zephyr module (via ZEPHYR_EXTRA_MODULES) during
# Zephyr's module loop at ZEPHYR_BASE/CMakeLists.txt:777-790 — AFTER
# add_subdirectory(drivers) creates the drivers__serial library target (line 774)
# and BEFORE Zephyr's empty-library check (line 1065). This is the only window
# where out-of-tree sources can be contributed to drivers__serial cleanly.

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
