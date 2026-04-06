if(CONFIG_UART_RT582)

zephyr_library()
zephyr_library_sources(uart_rt582.c)

get_filename_component(_RT582_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
zephyr_library_include_directories(
    ${_RT582_REPO_ROOT}/sdk/components/platform/soc/rt582/rt582_driver/Inc
    ${_RT582_REPO_ROOT}/sdk/components/platform/hosal/rt582_hosal/Inc
)

endif() # CONFIG_UART_RT582
