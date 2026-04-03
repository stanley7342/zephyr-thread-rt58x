if(CONFIG_UART_RT582)

zephyr_library()
zephyr_library_sources(uart_rt582.c)

zephyr_library_include_directories(
    ${CMAKE_SOURCE_DIR}/drivers/soc/rt582/rt582_driver/Inc
    ${CMAKE_SOURCE_DIR}/drivers/hosal/rt582_hosal/Inc
)

endif() # CONFIG_UART_RT582
