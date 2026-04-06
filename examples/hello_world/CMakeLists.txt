cmake_minimum_required(VERSION 3.20.0)

get_filename_component(RT582_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOARD_ROOT   ${RT582_PROJECT_DIR})
set(SOC_ROOT     ${RT582_PROJECT_DIR})
set(DTS_ROOT     ${RT582_PROJECT_DIR})
set(KCONFIG_ROOT ${RT582_PROJECT_DIR}/Kconfig)

list(APPEND ZEPHYR_EXTRA_MODULES ${RT582_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582_hello_world VERSION 1.0.0)

target_sources(app PRIVATE src/main.c)
