cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT   ${CMAKE_CURRENT_LIST_DIR})
set(SOC_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(DTS_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(KCONFIG_ROOT ${CMAKE_CURRENT_LIST_DIR}/Kconfig)

# Register this repo as a Zephyr module so cmake/zephyr_serial/CMakeLists.txt
# is add_subdirectory'd at the correct point in Zephyr's build (after
# drivers__serial is created, before the empty-library check).
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582_helloworld VERSION 1.1.1)

# ── Build metadata ────────────────────────────────────────────────────────────
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE BUILD_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT BUILD_GIT_HASH)
    set(BUILD_GIT_HASH "unknown")
endif()

execute_process(
    COMMAND git status --porcelain
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DIRTY_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(GIT_DIRTY_OUTPUT)
    string(APPEND BUILD_GIT_HASH "+dirty")
endif()

target_compile_definitions(app PRIVATE
    BUILD_GIT_HASH="${BUILD_GIT_HASH}"
)

# Make tprintk.h findable by SDK source files via #include "tprintk.h"
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples/thread/src)

# Rafael IoT SDK vendored into sdk/.
set(RAFAEL_SDK ${CMAKE_CURRENT_LIST_DIR}/sdk)
set(RT582_PLATFORM ${RAFAEL_SDK}/components/platform)

# Application source
target_sources(app PRIVATE examples/thread/src/main.c)

# ── OpenThread ────────────────────────────────────────────────────────────────
if(CONFIG_OPENTHREAD_RT582)
    include(subsys/openthread/CMakeLists.txt)
endif()

# ── RT582 platform drivers ────────────────────────────────────────────────────
# Platform sources (system_mcu.c, sysctrl.c, etc.) and include directories are
# now provided by cmake/zephyr_module/CMakeLists.txt via ZEPHYR_EXTRA_MODULES,
# so they are available to both this app and MCUboot without duplication.
set(RT582_DRV_SRC ${RT582_PLATFORM}/soc/rt582/rt582_driver/Src)

target_include_directories(app PRIVATE
    ${RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
    ${RT582_PLATFORM}/soc/rt582/rt582_system/Include
    ${RT582_PLATFORM}/hosal/rt582_hosal/Inc
)

# comm_subsystem_drv.c is only needed by the Thread example.
target_sources(app PRIVATE
    ${RT582_DRV_SRC}/comm_subsystem_drv.c
)

# ── RT582 UART driver ─────────────────────────────────────────────────────────
# uart_rt582.c + hosal_uart.c are added to drivers__serial via the Zephyr
# module cmake at cmake/zephyr_serial/CMakeLists.txt (see zephyr/module.yml).
# ZEPHYR_EXTRA_MODULES above registers this repo as a module so that cmake
# runs at the correct point in Zephyr's build graph.
