cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT   ${CMAKE_CURRENT_LIST_DIR})
set(SOC_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(DTS_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(KCONFIG_ROOT ${CMAKE_CURRENT_LIST_DIR}/Kconfig)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582_helloworld VERSION 1.0.0)

# Locate the Rafael IoT SDK
set(RAFAEL_SDK $ENV{RAFAEL_SDK_BASE})
if(NOT RAFAEL_SDK)
    set(RAFAEL_SDK ${CMAKE_CURRENT_LIST_DIR}/../Rafael-IoT-SDK-Internal)
endif()

# Application source
target_sources(app PRIVATE src/main.c)

# ── OpenThread subsystem ─────────────────────────────────────────────────────
add_subdirectory(subsys/openthread)

# ── RT582 UART driver ────────────────────────────────────────────────────────
# Add the driver directly to the 'app' library so it is included in the
# --whole-archive link section, which is required for Zephyr device
# registration macros (DEVICE_DT_INST_DEFINE) to work correctly.
if(CONFIG_UART_RT582)
    target_sources(app PRIVATE
        drivers/serial/uart_rt582.c
        # HOSAL UART implementation
        ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Src/hosal_uart.c
        # pin_set_mode(), enable_perclk(), delay_us() — required by hosal_uart.c
        ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_driver/Src/sysctrl.c
    )
    target_include_directories(app PRIVATE
        ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_driver/Inc
        ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Inc
    )
endif()
