cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT   ${CMAKE_CURRENT_LIST_DIR})
set(SOC_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(DTS_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(KCONFIG_ROOT ${CMAKE_CURRENT_LIST_DIR}/Kconfig)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582_helloworld VERSION 1.0.0)

# Rafael IoT SDK vendored into sdk/.
set(RAFAEL_SDK ${CMAKE_CURRENT_LIST_DIR}/sdk)
set(RT582_PLATFORM ${RAFAEL_SDK}/components/platform)

# Application source
target_sources(app PRIVATE src/main.c)

# ── OpenThread ────────────────────────────────────────────────────────────────
if(CONFIG_OPENTHREAD_RT582)
    include(subsys/openthread/CMakeLists.txt)
endif()

# ── RT582 platform drivers (compiled from source) ────────────────────────────
# Replaces librt582_system.a + librt582_driver.a.
# Excludes uart_drv.c / uart_retarget.c (UART handled via hosal_uart.c + uart_rt582.c).
# Excludes dwt.c (may conflict with Zephyr cycle-counter support).
set(RT582_DRV_SRC ${RT582_PLATFORM}/soc/rt582/rt582_driver/Src)
set(RT582_SYS_SRC ${RT582_PLATFORM}/soc/rt582/rt582_system/Src)

# system_mcu.c uses SET_SYS_CLK and CHIP_TYPE at compile time.
# SET_SYS_CLK=SYS_CLK_64MHZ=2 → SystemInit configures BBPLL to 64 MHz.
# CHIP_TYPE=2 (RT582) — chip_define.h defaults to RT581=1, must override.
# CHIP_VERSION defaults to RT58X_MPB=2 in chip_define.h — correct for EVB.
target_compile_definitions(app PRIVATE
    SET_SYS_CLK=2
    CHIP_TYPE=2
)

target_sources(app PRIVATE
    # System init: SystemInit (BBPLL → 64 MHz), systemfrequencyupdate, mp-sector calibration
    ${RT582_SYS_SRC}/system_mcu.c
    ${RT582_SYS_SRC}/mp_sector.c
    # SOC peripheral drivers (only those needed for Thread FTD)
    # Excluded: i2c_master.c (uart_stdio.h missing), qspi.c (QSPI_AUX member mismatch),
    #           swt.c (swt.h missing), sadc.c (FreeRTOS dep), i2s/pwm/comparator/lpm (unused)
    ${RT582_DRV_SRC}/comm_subsystem_drv.c
    ${RT582_DRV_SRC}/dma.c
    ${RT582_DRV_SRC}/flash_bp_protect.c
    ${RT582_DRV_SRC}/flashctl.c
    ${RT582_DRV_SRC}/gpio.c
    ${RT582_DRV_SRC}/rtc.c
    ${RT582_DRV_SRC}/swi.c
    ${RT582_DRV_SRC}/sysctrl.c
    ${RT582_DRV_SRC}/sysfun.c
    ${RT582_DRV_SRC}/timer.c
    ${RT582_DRV_SRC}/trng.c
    ${RT582_DRV_SRC}/wdt.c
)

target_include_directories(app PRIVATE
    ${RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
    ${RT582_PLATFORM}/soc/rt582/rt582_system/Include
    ${RT582_PLATFORM}/hosal/rt582_hosal/Inc
)

# ── RT582 UART driver ─────────────────────────────────────────────────────────
# uart_rt582.c is compiled as part of 'app' (not the Zephyr 'drivers__serial'
# library) because Zephyr's empty-library check runs inside find_package() and
# marks drivers__serial as EXCLUDE_FROM_ALL before our CMakeLists can add
# sources to it. Setting ALLOW_EMPTY suppresses the resulting CMake warning.
if(CONFIG_UART_RT582)
    set_property(TARGET drivers__serial PROPERTY ALLOW_EMPTY 1)
    target_sources(app PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/drivers/serial/uart_rt582.c
        ${RT582_PLATFORM}/hosal/rt582_hosal/Src/hosal_uart.c
    )
endif()
