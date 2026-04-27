cmake_minimum_required(VERSION 3.20.0)

# Project root lives four levels above this example.
get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)

set(BOARD_ROOT   ${RT583_PROJECT_DIR})
set(SOC_ROOT     ${RT583_PROJECT_DIR})
set(DTS_ROOT     ${RT583_PROJECT_DIR})
set(KCONFIG_ROOT ${RT583_PROJECT_DIR}/Kconfig)

# Module-level glue (cmake/zephyr_module/CMakeLists.txt) provides the
# SoC-aware platform sources, include dirs and driver wiring.
list(APPEND ZEPHYR_EXTRA_MODULES ${RT583_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rafael_ble_hrs VERSION 1.1.1)

target_sources(app PRIVATE src/main.c)

# BLE subsystem (SDK port) — sets up app's BLE/HOSAL/RF include paths internally.
if(CONFIG_BLE_RT583 OR CONFIG_BLE_RT584)
    set(RAFAEL_SDK ${RT583_PROJECT_DIR}/sdk)
    include(${RT583_PROJECT_DIR}/subsys/ble/CMakeLists.txt)
endif()
