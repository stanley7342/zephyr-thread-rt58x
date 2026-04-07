cmake_minimum_required(VERSION 3.20.0)

# ── Path setup ──────────────────────────────────────────────────────────────
# RT582_PROJECT_DIR: three levels up from this file (examples/matter/lighting-app → repo root)
get_filename_component(RT582_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
# Workspace root is one level above the repo (standard west workspace layout)
get_filename_component(WORKSPACE_DIR "${RT582_PROJECT_DIR}/.." ABSOLUTE)

# CHIP_ROOT: use CHIP_ROOT env var first, fall back to <workspace>/connectedhomeip
if(DEFINED ENV{CHIP_ROOT})
    get_filename_component(CHIP_ROOT "$ENV{CHIP_ROOT}" ABSOLUTE)
else()
    set(CHIP_ROOT "${WORKSPACE_DIR}/connectedhomeip")
endif()

if(NOT EXISTS "${CHIP_ROOT}/src/lib/core/CHIPError.h")
    message(FATAL_ERROR
        "connectedhomeip not found at ${CHIP_ROOT}.\n"
        "Set the CHIP_ROOT environment variable or clone it to ${WORKSPACE_DIR}/connectedhomeip:\n"
        "  git clone --recurse-submodules https://github.com/project-chip/connectedhomeip ${WORKSPACE_DIR}/connectedhomeip")
endif()

get_filename_component(GEN_DIR "${CHIP_ROOT}/zzz_generated" ABSOLUTE)

# ── Zephyr module setup ──────────────────────────────────────────────────────
set(BOARD_ROOT   ${RT582_PROJECT_DIR})
set(SOC_ROOT     ${RT582_PROJECT_DIR})
set(DTS_ROOT     ${RT582_PROJECT_DIR})
set(KCONFIG_ROOT ${RT582_PROJECT_DIR}/Kconfig)

list(APPEND ZEPHYR_EXTRA_MODULES ${RT582_PROJECT_DIR})

# Matter chip-module (nrfconnect variant — most complete Zephyr BT+OpenThread support)
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/nrfconnect/chip-module)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582-matter-lighting VERSION 1.0.0)

# ── C++17 (required by Matter) ───────────────────────────────────────────────
include(${CHIP_ROOT}/config/nrfconnect/app/enable-gnu-std.cmake)

# ── Matter data model (ZAP-generated code + cluster implementations) ─────────
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CHIP_ROOT}/examples/lighting-app/lighting-common/lighting-app.zap
)

# ── Include directories ──────────────────────────────────────────────────────
set(RT582_PLATFORM ${RT582_PROJECT_DIR}/sdk/components/platform)

target_include_directories(app PRIVATE
    main/include
    ${GEN_DIR}
    ${GEN_DIR}/app-common
    ${GEN_DIR}/lighting-app
    ${CHIP_ROOT}/examples/lighting-app/lighting-common
    ${RT582_PLATFORM}/soc/rt582/rt582_driver/Inc
    ${RT582_PLATFORM}/soc/rt582/rt582_system/Include
    ${RT582_PLATFORM}/hosal/rt582_hosal/Inc
)

# ── Application sources ──────────────────────────────────────────────────────
target_sources(app PRIVATE
    main/main.cpp
    main/AppTask.cpp
    main/ZclCallbacks.cpp
)

# ── BLE + OpenThread subsystems (RT582 platform) ─────────────────────────────
if(CONFIG_BLE_RT582)
    set(RAFAEL_SDK ${RT582_PROJECT_DIR}/sdk)
    include(${RT582_PROJECT_DIR}/subsys/ble/CMakeLists.txt)
endif()

if(CONFIG_OPENTHREAD_RT582)
    include(${RT582_PROJECT_DIR}/subsys/openthread/CMakeLists.txt)
endif()

# comm_subsystem_drv.c — RF MCU communication (needed by both BLE and OT)
target_sources(app PRIVATE
    ${RT582_PLATFORM}/soc/rt582/rt582_driver/Src/comm_subsystem_drv.c
)

# Suppress warnings in vendored SDK sources
target_compile_options(app PRIVATE -Wno-error=maybe-uninitialized)
