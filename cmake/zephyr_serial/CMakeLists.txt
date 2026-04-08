# cmake/zephyr_serial/CMakeLists.txt
#
# This cmake is included as a Zephyr module (via ZEPHYR_EXTRA_MODULES) during
# Zephyr's module loop at ZEPHYR_BASE/CMakeLists.txt:777-790 — AFTER
# add_subdirectory(drivers) creates the drivers__serial library target (line 774)
# and BEFORE Zephyr's empty-library check (line 1065). This is the only window
# where out-of-tree sources can be contributed to drivers__serial cleanly.

if(CONFIG_UART_RT583)
    get_filename_component(_RT583_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
    set(_RT583_PLATFORM ${_RT583_REPO_ROOT}/sdk/components/platform)

    target_sources(drivers__serial PRIVATE
        ${_RT583_REPO_ROOT}/drivers/serial/uart_rt583.c
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Src/hosal_uart.c
    )

    target_include_directories(drivers__serial PRIVATE
        ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
    )
endif()
