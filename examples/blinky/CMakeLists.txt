cmake_minimum_required(VERSION 3.20.0)

get_filename_component(RT582_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOARD_ROOT   ${RT582_PROJECT_DIR})
set(SOC_ROOT     ${RT582_PROJECT_DIR})
set(DTS_ROOT     ${RT582_PROJECT_DIR})
set(KCONFIG_ROOT ${RT582_PROJECT_DIR}/Kconfig)

list(APPEND ZEPHYR_EXTRA_MODULES ${RT582_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582_blinky VERSION 1.0.0)

target_sources(app PRIVATE src/main.c)

set(RT582_PLATFORM ${RT582_PROJECT_DIR}/sdk/components/platform)
target_include_directories(app PRIVATE
    ${RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
    ${RT582_PLATFORM}/soc/rt582/rt582_system/Include
    ${RT582_PLATFORM}/hosal/rt582_hosal/Inc
)
