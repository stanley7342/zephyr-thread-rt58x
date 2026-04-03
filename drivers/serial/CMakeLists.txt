if(CONFIG_UART_RT582)

zephyr_library()
zephyr_library_sources(uart_rt582.c)

# Rafael SDK headers needed by the driver
set(RAFAEL_SDK $ENV{RAFAEL_SDK_BASE})
if(NOT RAFAEL_SDK)
    # drivers/serial/ → drivers/ → project_root → parent → SDK
    set(RAFAEL_SDK ${CMAKE_CURRENT_LIST_DIR}/../../../Rafael-IoT-SDK-Internal)
endif()

zephyr_library_include_directories(
    ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_driver/Inc
    ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Inc
)

# Build the Rafael HOSAL UART sources directly into this driver library
zephyr_library_sources(
    ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Src/hosal_uart.c
)

endif() # CONFIG_UART_RT582
