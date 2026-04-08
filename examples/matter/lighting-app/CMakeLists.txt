cmake_minimum_required(VERSION 3.20.0)

# ── Path setup ──────────────────────────────────────────────────────────────
# RT583_PROJECT_DIR: three levels up from this file (examples/matter/lighting-app → repo root)
get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
# Workspace root is one level above the repo (standard west workspace layout)
get_filename_component(WORKSPACE_DIR "${RT583_PROJECT_DIR}/.." ABSOLUTE)

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
set(BOARD_ROOT   ${RT583_PROJECT_DIR})
set(SOC_ROOT     ${RT583_PROJECT_DIR})
set(DTS_ROOT     ${RT583_PROJECT_DIR})
set(KCONFIG_ROOT ${RT583_PROJECT_DIR}/Kconfig)

list(APPEND ZEPHYR_EXTRA_MODULES ${RT583_PROJECT_DIR})

# Matter chip-module (nrfconnect variant — most complete Zephyr BT+OpenThread support)
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/zephyr/chip-module)

# mbedTLS 4.0 compat: Zephyr v4.4 ships mbedTLS 4.0 (TF-PSA-Crypto) which
# removed mbedtls/ecp.h and mbedtls/bignum.h from the public API.
# connectedhomeip still uses those headers (mbedTLS 2.x/3.x public API).
# The shim directory re-exports them from mbedtls/private/ so the GN build
# compiles without source patches.  The -I flag has higher priority than the
# -isystem flags that chip-module later injects from Zephyr's cflags.
set(CHIP_CFLAGS "-I${CMAKE_CURRENT_LIST_DIR}/mbedtls_shim")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt583-matter-lighting VERSION 1.0.0)

# ── C++17 (required by Matter) ───────────────────────────────────────────────
include(${CHIP_ROOT}/config/zephyr/app/enable-gnu-std.cmake)

# ── Matter data model (ZAP-generated code + cluster implementations) ─────────
include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

chip_configure_data_model(app
    INCLUDE_SERVER
    ZAP_FILE ${CMAKE_CURRENT_LIST_DIR}/lighting-app.zap
    MATTER_IDLS ${CMAKE_CURRENT_LIST_DIR}/lighting-app.matter
)

# ── Include directories ──────────────────────────────────────────────────────
set(RT583_PLATFORM ${RT583_PROJECT_DIR}/sdk/components/platform)

target_include_directories(app PRIVATE
    main/include
    ${GEN_DIR}
    ${GEN_DIR}/app-common
    ${GEN_DIR}/lighting-app
    ${CHIP_ROOT}/examples/lighting-app/lighting-common
    ${RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
    ${RT583_PLATFORM}/soc/rt583/rt583_system/Include
    ${RT583_PLATFORM}/hosal/rt583_hosal/Inc
)

# ── Application sources ──────────────────────────────────────────────────────
target_sources(app PRIVATE
    main/main.cpp
    main/AppTask.cpp
    main/ZclCallbacks.cpp
)

# ── BLE + OpenThread subsystems (RT583 platform) ─────────────────────────────
if(CONFIG_BLE_RT583)
    set(RAFAEL_SDK ${RT583_PROJECT_DIR}/sdk)
    include(${RT583_PROJECT_DIR}/subsys/ble/CMakeLists.txt)
endif()

if(CONFIG_OPENTHREAD_RT583)
    # Matter doesn't need the OT CLI — disable it to save ~37 KB flash / 7 KB BSS.
    set(OT_INCLUDE_CLI 0)
    include(${RT583_PROJECT_DIR}/subsys/openthread/CMakeLists.txt)
endif()

# comm_subsystem_drv.c — RF MCU communication (needed by both BLE and OT)
target_sources(app PRIVATE
    ${RT583_PLATFORM}/soc/rt583/rt583_driver/Src/comm_subsystem_drv.c
)

# Suppress warnings in vendored SDK sources
target_compile_options(app PRIVATE -Wno-error=maybe-uninitialized)

# Multiple definition suppressions for Matter + OT coexistence:
#  1. otPlatRadioGetIeeeEui64: defined in both ot_radio.c (ours, HW-backed) and
#     Zephyr's subsys/net/l2/openthread/openthread.c (needed when
#     CONFIG_NET_L2_OPENTHREAD=y). Our version wins (first in link order).
#  2. mbedtls ssl_*/x509_*/pk_* symbols: OT bundles its own mbedTLS 2.x/3.x
#     (compiled from source) while Zephyr also links its system mbedTLS 4.0.
#     OT uses its own ssl_* internally (DTLS); Matter uses PSA, not ssl_* directly.
#     --allow-multiple-definition keeps the first definition (ours, from libapp.a).
zephyr_ld_options(-Wl,--allow-multiple-definition)
