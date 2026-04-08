if(CONFIG_UART_RT583)

zephyr_library()
zephyr_library_sources(uart_rt583.c)

get_filename_component(_RT583_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
zephyr_library_include_directories(
    ${_RT583_REPO_ROOT}/sdk/components/platform/soc/rt583/rt583_driver/Inc
    ${_RT583_REPO_ROOT}/sdk/components/platform/hosal/rt583_hosal/Inc
)

endif() # CONFIG_UART_RT583
