cmake_minimum_required(VERSION 3.20.0)

get_filename_component(REPO_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(BOARD_ROOT   ${REPO_DIR})
set(SOC_ROOT     ${REPO_DIR})
set(DTS_ROOT     ${REPO_DIR})
set(KCONFIG_ROOT ${REPO_DIR}/Kconfig)
list(APPEND ZEPHYR_EXTRA_MODULES ${REPO_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt584_hello_bare)

target_sources(app PRIVATE src/main.c)
