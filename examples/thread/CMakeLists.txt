cmake_minimum_required(VERSION 3.20.0)

# Project root lives two levels above this example (examples/thread/ → repo root).
get_filename_component(RT582_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOARD_ROOT   ${RT582_PROJECT_DIR})
set(SOC_ROOT     ${RT582_PROJECT_DIR})
set(DTS_ROOT     ${RT582_PROJECT_DIR})
set(KCONFIG_ROOT ${RT582_PROJECT_DIR}/Kconfig)

# Register the project root as a Zephyr module so that cmake/zephyr_module/
# CMakeLists.txt is included at the correct point in Zephyr's build graph.
# This contributes RT582 platform sources, UART, and flash drivers.
list(APPEND ZEPHYR_EXTRA_MODULES ${RT582_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582_ot_ftd_cli VERSION 1.0.0)

target_sources(app PRIVATE src/main.c)
if(CONFIG_OTA)
    target_sources(app PRIVATE src/ota_app.c)
endif()
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

# RT582 platform include dirs for app-level sources (OT port, main.c, etc.)
# Platform sources themselves are provided by cmake/zephyr_module/CMakeLists.txt.
set(RT582_PLATFORM ${RT582_PROJECT_DIR}/sdk/components/platform)
target_include_directories(app PRIVATE
    ${RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
    ${RT582_PLATFORM}/soc/rt582/rt582_system/Include
    ${RT582_PLATFORM}/hosal/rt582_hosal/Inc
)

# comm_subsystem_drv.c is only needed by the Thread example (not MCUboot).
target_sources(app PRIVATE
    ${RT582_PLATFORM}/soc/rt582/rt582_driver/Src/comm_subsystem_drv.c
)

# ── OpenThread subsystem ──────────────────────────────────────────────────────
# RAFAEL_SDK must be set before including subsys/openthread/CMakeLists.txt
# because it uses the variable to locate SDK sources.
if(CONFIG_OPENTHREAD_RT582)
    set(RAFAEL_SDK ${RT582_PROJECT_DIR}/sdk)
    include(${RT582_PROJECT_DIR}/subsys/openthread/CMakeLists.txt)
endif()

# ── OTA subsystem ─────────────────────────────────────────────────────────────
if(CONFIG_OTA)
    include(${RT582_PROJECT_DIR}/subsys/ota/CMakeLists.txt)
endif()
