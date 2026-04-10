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
        "  git clone --recurse-submodules https://github.com/project-chip/connectedhomeip ${WORKSPACE_DIR}/connectedhomeip\n"
        "  cd ${WORKSPACE_DIR}/connectedhomeip && git checkout fdb31dbcb3ebe000ad3e41b386e90849937948f9")
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
# moved mbedtls/ecp.h and mbedtls/bignum.h from the public API to the private
# driver include path (tf-psa-crypto/drivers/builtin/include/mbedtls/private/).
# connectedhomeip still uses those headers (mbedTLS 2.x/3.x public API).
# The mbedtls_shim/ directory provides forwarding headers so that
# #include <mbedtls/ecp.h> finds mbedtls/private/ecp.h.
#
# Three plain -I flags are required instead of relying on the -isystem paths
# already injected by zephyr_get_compile_flags:
#   1. mbedtls_shim/           — provides mbedtls/ecp.h → mbedtls/private/ecp.h
#   2. .../drivers/builtin/include — provides mbedtls/private/ecp.h itself
#   3. .../tf-psa-crypto/include   — provides tf-psa-crypto/build_info.h and
#                                    mbedtls/platform_util.h (included by ecp.h)
#
# zephyr_get_compile_flags emits "-isystemC:/path" (no space between -isystem
# and the drive-letter path) which arm-zephyr-eabi-g++ on Windows does not parse
# correctly — the drive letter is swallowed by the flag name.  Plain -I flags
# are always parsed correctly and take precedence over -isystem.
#
# Paths are normalised with get_filename_component(ABSOLUTE) to eliminate the
# /../ segment so the compiler receives a clean Windows absolute path.
#
# MBEDTLS_ALLOW_PRIVATE_ACCESS enables MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS
# (see mbedtls/private_access.h) which guards ALL function declarations in
# mbedtls/private/ecp.h (mbedtls_ecp_group_init, mbedtls_ecp_mul, …).
# Without it CHIPCryptoPALPSA.cpp fails with 'not declared in this scope'.
get_filename_component(TF_PSA_CRYPTO_INCLUDE
    "${ZEPHYR_BASE}/../modules/crypto/mbedtls/tf-psa-crypto/include"
    ABSOLUTE)
get_filename_component(TF_PSA_CRYPTO_BUILTIN_INCLUDE
    "${ZEPHYR_BASE}/../modules/crypto/mbedtls/tf-psa-crypto/drivers/builtin/include"
    ABSOLUTE)

set(CHIP_CFLAGS
    "-I${CMAKE_CURRENT_LIST_DIR}/mbedtls_shim"
    "-I${TF_PSA_CRYPTO_BUILTIN_INCLUDE}"
    "-I${TF_PSA_CRYPTO_INCLUDE}"
    # mbedtls/private/ecp.h guards ALL function declarations (mbedtls_ecp_group_init,
    # mbedtls_ecp_mul, etc.) inside #if defined(MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS).
    # That macro is only defined when MBEDTLS_ALLOW_PRIVATE_ACCESS is defined
    # (see private_access.h).  Without it, the functions are not declared and
    # CHIPCryptoPALPSA.cpp fails with 'not declared in this scope' errors.
    "-DMBEDTLS_ALLOW_PRIVATE_ACCESS"
    # PSA crypto provides its own entropy; PlatformManagerImpl.cpp must not call
    # add_entropy_source() (the PSA backend returns CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE).
    # Defining CONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR suppresses that code path.
    "-DCONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR=1"
)

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

# ── ODR fix: remove server sources from CMake build ──────────────────────────
# chip_configure_data_model(INCLUDE_SERVER) adds Server.cpp and
# CommissioningWindowManager.cpp to the CMake app target.  These are ALSO
# compiled by the GN build (into libCHIP.a) which uses its own include-path
# order for mbedTLS / TF-PSA-Crypto headers.  PASESession.cpp is compiled
# ONLY by GN.  If CMake and GN resolve psa_pake_operation_t to different
# struct sizes, CommissioningWindowManager (which embeds PASESession) has an
# ODR violation: the GN-compiled PASESession::Clear() writes past the
# CMake-allocated CommissioningWindowManager boundary, corrupting adjacent
# Server members.  Removing these two files from the CMake build forces the
# linker to use the GN-compiled .o files from libCHIP.a, where all TUs see
# the same headers.  --allow-multiple-definition (already set) ensures the
# remaining duplicates (from other server sources) link without error.
get_target_property(_app_srcs app SOURCES)
list(REMOVE_ITEM _app_srcs
    "${CHIP_ROOT}/src/app/server/Server.cpp"
    "${CHIP_ROOT}/src/app/server/CommissioningWindowManager.cpp"
)
set_property(TARGET app PROPERTY SOURCES ${_app_srcs})

# ── Include directories ──────────────────────────────────────────────────────
set(RT583_PLATFORM ${RT583_PROJECT_DIR}/sdk/components/platform)

target_include_directories(app PRIVATE
    main/include
    ${GEN_DIR}
    ${GEN_DIR}/app-common
    ${GEN_DIR}/lighting-app
    ${CHIP_ROOT}/examples/lighting-app/lighting-common
    ${CHIP_ROOT}/examples/providers
    ${RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
    ${RT583_PLATFORM}/soc/rt583/rt583_system/Include
    ${RT583_PLATFORM}/hosal/rt583_hosal/Inc
)

# ── Application sources ──────────────────────────────────────────────────────
target_sources(app PRIVATE
    main/main.cpp
    main/AppTask.cpp
    main/ZclCallbacks.cpp
    main/MallocWrappers.cpp
    main/ecp_enum_compat.c
    ${CHIP_ROOT}/examples/providers/DeviceInfoProviderImpl.cpp
)

# Redirect ALL heap allocations to our sys_heap wrapper (MallocWrappers.cpp).
# --wrap=malloc/calloc/realloc/free covers CHIP SDK and C++ operator new/delete.
# --wrap=_malloc_r/... covers newlib reentrant calls (strdup, printf internals,
# etc.) that bypass the public malloc symbol and call _malloc_r directly.
# Without the _malloc_r wraps, strdup() during Server::Init exhausts the 2 KB
# k_heap (shared with BT GATT) → NULL → CHIP_ERROR_NO_MEMORY.
zephyr_ld_options(
    -Wl,--wrap=malloc
    -Wl,--wrap=calloc
    -Wl,--wrap=realloc
    -Wl,--wrap=free
    -Wl,--wrap=_malloc_r
    -Wl,--wrap=_free_r
    -Wl,--wrap=_calloc_r
    -Wl,--wrap=_realloc_r
)

# ECP enum compatibility: CHIP (TF-PSA-Crypto) passes SECP256R1=1 but OT's linked
# mbedtls_ecp_group_load (mbedTLS 3.6.0) expects SECP256R1=3 → FEATURE_UNAVAILABLE.
# --wrap redirects all callers through __wrap_mbedtls_ecp_group_load (ecp_enum_compat.c)
# which translates 1→3 before forwarding to OT's __real implementation.
# The muladd and mul wraps add diagnostic logging to identify PASE failure root cause.
zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_group_load)
zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_muladd)
zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_mul)

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

# Dummy net interface — provides an IPv6 link-local address so that Matter's
# Server::Init can bind UDP sockets.  RT583's OT radio uses vendored lmac15p4
# (not a Zephyr ieee802154 driver), so no real net_if is registered.
target_sources(app PRIVATE ${RT583_PROJECT_DIR}/drivers/net/dummy_net_if.c)

# RF firmware selection for multi-protocol (BLE + 802.15.4) coexistence.
# rf_common_init_by_fw() checks CONFIG_RF_FW_INCLUDE_* at compile time.
# MULTI_2P4G loads a combined firmware that supports both BLE and Thread
# simultaneously — required for a Matter over Thread device.
# PCI (RUCI/MAC path) and BLE are included so the OT and BLE CMakeLists
# sub-includes below do not override with conflicting FALSE settings.
target_compile_definitions(app PRIVATE
    # MULTI_2P4G: combined BLE+802.15.4 firmware — required for Matter over Thread.
    # PCI and BLE are already defined (with proper -U/-D sequencing) by
    # subsys/openthread/CMakeLists.txt and subsys/ble/CMakeLists.txt; defining
    # them again here causes "redefined" warnings without changing behaviour.
    CONFIG_RF_FW_INCLUDE_MULTI_2P4G=TRUE
    # CONFIG_RF_MCU_CONST_LOAD_SUPPORTED is required for the multi-protocol
    # firmware to boot: without the const calibration data, the RF MCU never
    # asserts SYS_RDY after firmware load.  The readback verification in
    # RfMcu_ConstLoad was patched to skip the check (it spuriously fails on
    # Zephyr even though the write succeeds via the same AHB DMA path used
    # for firmware image load).
    CONFIG_RF_MCU_CONST_LOAD_SUPPORTED=TRUE
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
