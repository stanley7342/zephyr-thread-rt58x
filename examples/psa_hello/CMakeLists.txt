cmake_minimum_required(VERSION 3.20.0)

get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOARD_ROOT   ${RT583_PROJECT_DIR})
set(SOC_ROOT     ${RT583_PROJECT_DIR})
set(DTS_ROOT     ${RT583_PROJECT_DIR})
set(KCONFIG_ROOT ${RT583_PROJECT_DIR}/Kconfig)

list(APPEND ZEPHYR_EXTRA_MODULES ${RT583_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt58x_psa_hello VERSION 1.0.0)

target_sources(app PRIVATE src/main.c)

# rt584-only: pull in vendor otp.c so we can call rt_otp_read_data(PUF_UID,...).
# Demonstrates SG.1 (Unique Identification) — die-unique ID from the rt584
# PUF/OTP block, hashed with PSA Crypto into a 32-byte implementation ID.
if(CONFIG_SOC_SERIES_RT584)
    set(_RT584_DRV ${RT583_PROJECT_DIR}/sdk/components/platform/soc/rt584/rt584_driver)
    target_sources(app PRIVATE
        ${_RT584_DRV}/Src/otp.c
    )
    target_include_directories(app PRIVATE
        ${_RT584_DRV}/Inc
        ${RT583_PROJECT_DIR}/sdk/components/platform/include
    )
    target_compile_definitions(app PRIVATE PSA_HELLO_HAS_UID=1)
endif()
