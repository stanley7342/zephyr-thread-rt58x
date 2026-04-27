cmake_minimum_required(VERSION 3.20.0)

# ── Path setup ──────────────────────────────────────────────────────────────
# RT583_PROJECT_DIR: three levels up from this file (examples/matter/lighting-app → repo root)
get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)
# Workspace root is one level above the repo (standard west workspace layout)
get_filename_component(WORKSPACE_DIR "${RT583_PROJECT_DIR}/.." ABSOLUTE)

# CHIP_ROOT: use CHIP_ROOT env var first, then search common locations.
# install.ps1 may place connectedhomeip either at:
#   <workspace>/connectedhomeip  (standard west layout — workspace is parent of repo)
#   <repo>/connectedhomeip       (install ran from inside the repo dir)
if(DEFINED ENV{CHIP_ROOT})
    get_filename_component(CHIP_ROOT "$ENV{CHIP_ROOT}" ABSOLUTE)
elseif(EXISTS "${WORKSPACE_DIR}/connectedhomeip/src/lib/core/CHIPError.h")
    set(CHIP_ROOT "${WORKSPACE_DIR}/connectedhomeip")
elseif(EXISTS "${RT583_PROJECT_DIR}/connectedhomeip/src/lib/core/CHIPError.h")
    set(CHIP_ROOT "${RT583_PROJECT_DIR}/connectedhomeip")
else()
    set(CHIP_ROOT "${WORKSPACE_DIR}/connectedhomeip")
endif()

if(NOT EXISTS "${CHIP_ROOT}/src/lib/core/CHIPError.h")
    message(FATAL_ERROR
        "connectedhomeip not found.\n"
        "Set the CHIP_ROOT environment variable or clone it:\n"
        "  git clone --depth=1 https://github.com/project-chip/connectedhomeip ${WORKSPACE_DIR}/connectedhomeip\n"
        "  cd ${WORKSPACE_DIR}/connectedhomeip && git fetch --depth=1 origin fdb31dbcb3ebe000ad3e41b386e90849937948f9 && git checkout fdb31dbcb3ebe000ad3e41b386e90849937948f9")
endif()

# ── Python3: prefer the workspace venv so codegen deps (python_path, matter IDL)
#    are available.  chip_codegen.cmake calls find_package(Python3) which would
#    otherwise pick the system Python that lacks these packages.
#
#    NOTE: under `west build --sysbuild` the child Zephyr project is launched by
#    ExternalZephyrProject_Cmake which does NOT inherit VIRTUAL_ENV, so the
#    VIRTUAL_ENV check alone isn't enough.  Fall back to the canonical
#    `<workspace>/.zephyr-venv` path that install.ps1 / install.sh create.
if(NOT DEFINED Python3_EXECUTABLE)
    if(WIN32)
        set(_rt583_venv_py_env  "$ENV{VIRTUAL_ENV}/Scripts/python.exe")
        set(_rt583_venv_py_ws   "${WORKSPACE_DIR}/.zephyr-venv/Scripts/python.exe")
    else()
        set(_rt583_venv_py_env  "$ENV{VIRTUAL_ENV}/bin/python")
        set(_rt583_venv_py_ws   "${WORKSPACE_DIR}/.zephyr-venv/bin/python")
    endif()

    if(DEFINED ENV{VIRTUAL_ENV} AND EXISTS "${_rt583_venv_py_env}")
        set(Python3_EXECUTABLE "${_rt583_venv_py_env}" CACHE FILEPATH "")
    elseif(EXISTS "${_rt583_venv_py_ws}")
        set(Python3_EXECUTABLE "${_rt583_venv_py_ws}" CACHE FILEPATH "")
    endif()
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

# ── mbedTLS 4.0 compat shims (generated in build dir) ───────────────────────
# Zephyr v4.4 ships mbedTLS 4.0 (TF-PSA-Crypto) which moved public mbedtls/
# ecp.h, bignum.h, and pk.h APIs into private headers and redesigned
# mbedtls_pk_context.  connectedhomeip still uses the 3.x public API.
#
# Instead of committing shim files to the source tree, generate them at
# CMake configure time into ${CMAKE_BINARY_DIR}/chip_mbedtls_compat/ so the
# source tree stays clean.  The generated directory is prepended to the GN
# -I path so the shim headers shadow the installed public headers.
#
# Plain -I flags are used (not -isystem) because on Windows arm-zephyr-eabi-
# g++ mis-parses "-isystemC:/path" (drive letter consumed by flag name).
get_filename_component(TF_PSA_CRYPTO_INCLUDE
    "$ENV{ZEPHYR_BASE}/../modules/crypto/mbedtls/tf-psa-crypto/include"
    ABSOLUTE)
get_filename_component(TF_PSA_CRYPTO_BUILTIN_INCLUDE
    "$ENV{ZEPHYR_BASE}/../modules/crypto/mbedtls/tf-psa-crypto/drivers/builtin/include"
    ABSOLUTE)

set(_CHIP_MBEDTLS_COMPAT "${CMAKE_BINARY_DIR}/chip_mbedtls_compat")

# ecp.h — redirect to private header (ECP API moved in 4.x)
file(WRITE "${_CHIP_MBEDTLS_COMPAT}/mbedtls/ecp.h"
[=[/* Auto-generated compat shim — do not edit. */
#pragma once
#include <mbedtls/private/ecp.h>
]=])

# bignum.h — redirect to private header (bignum API moved in 4.x)
file(WRITE "${_CHIP_MBEDTLS_COMPAT}/mbedtls/bignum.h"
[=[/* Auto-generated compat shim — do not edit. */
#pragma once
#include <mbedtls/private/bignum.h>
]=])

# pk.h — (a) pulls in pk_private.h for legacy PK type symbols, and
#         (b) provides mbedtls_pk_ec() which was removed in 4.x.
#
# mbedtls_pk_context in 4.x is PSA-backed: the public key lives in
# pub_raw[] (uncompressed point bytes) and ec_family/bits describe the
# curve.  mbedtls_pk_ec() rebuilds an mbedtls_ecp_keypair from those
# fields using a static local (safe: connectedhomeip only reads the result).
file(WRITE "${_CHIP_MBEDTLS_COMPAT}/mbedtls/pk.h"
[=[/* Auto-generated compat shim — do not edit. */
#ifndef MBEDTLS_PK_SHIM_H
#define MBEDTLS_PK_SHIM_H

#include_next <mbedtls/pk.h>
#include <mbedtls/private/pk_private.h>
#include <mbedtls/private/ecp.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline mbedtls_ecp_group_id mbedtls_pk_ec_psa_to_grp(
        psa_ecc_family_t family, size_t bits)
{
    switch (family) {
        case PSA_ECC_FAMILY_SECP_R1:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_SECP192R1_ENABLED)
                case 192: return MBEDTLS_ECP_DP_SECP192R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP224R1_ENABLED)
                case 224: return MBEDTLS_ECP_DP_SECP224R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP256R1_ENABLED)
                case 256: return MBEDTLS_ECP_DP_SECP256R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP384R1_ENABLED)
                case 384: return MBEDTLS_ECP_DP_SECP384R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP521R1_ENABLED)
                case 521: return MBEDTLS_ECP_DP_SECP521R1;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        case PSA_ECC_FAMILY_SECP_K1:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_SECP192K1_ENABLED)
                case 192: return MBEDTLS_ECP_DP_SECP192K1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP224K1_ENABLED)
                case 224: return MBEDTLS_ECP_DP_SECP224K1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP256K1_ENABLED)
                case 256: return MBEDTLS_ECP_DP_SECP256K1;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        case PSA_ECC_FAMILY_BRAINPOOL_P_R1:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_BP256R1_ENABLED)
                case 256: return MBEDTLS_ECP_DP_BP256R1;
#endif
#if defined(MBEDTLS_ECP_DP_BP384R1_ENABLED)
                case 384: return MBEDTLS_ECP_DP_BP384R1;
#endif
#if defined(MBEDTLS_ECP_DP_BP512R1_ENABLED)
                case 512: return MBEDTLS_ECP_DP_BP512R1;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        case PSA_ECC_FAMILY_MONTGOMERY:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_CURVE25519_ENABLED)
                case 255: return MBEDTLS_ECP_DP_CURVE25519;
#endif
#if defined(MBEDTLS_ECP_DP_CURVE448_ENABLED)
                case 448: return MBEDTLS_ECP_DP_CURVE448;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        default:
            return MBEDTLS_ECP_DP_NONE;
    }
}

static inline mbedtls_ecp_keypair *mbedtls_pk_ec(mbedtls_pk_context ctx)
{
    static mbedtls_ecp_keypair kp;
    mbedtls_ecp_keypair_free(&kp);
    mbedtls_ecp_keypair_init(&kp);
    mbedtls_ecp_group_id grp_id = mbedtls_pk_ec_psa_to_grp(
            ctx.MBEDTLS_PRIVATE(ec_family), ctx.MBEDTLS_PRIVATE(bits));
    if (grp_id == MBEDTLS_ECP_DP_NONE) return NULL;
    if (mbedtls_ecp_group_load(&kp.MBEDTLS_PRIVATE(grp), grp_id) != 0) return NULL;
    if (mbedtls_ecp_point_read_binary(&kp.MBEDTLS_PRIVATE(grp), &kp.MBEDTLS_PRIVATE(Q),
                ctx.MBEDTLS_PRIVATE(pub_raw), ctx.MBEDTLS_PRIVATE(pub_raw_len)) != 0)
        return NULL;
    return &kp;
}

#ifdef __cplusplus
}
#endif
#endif /* MBEDTLS_PK_SHIM_H */
]=])

set(CHIP_CFLAGS
    "-I${_CHIP_MBEDTLS_COMPAT}"
    "-I${TF_PSA_CRYPTO_BUILTIN_INCLUDE}"
    "-I${TF_PSA_CRYPTO_INCLUDE}"
    # MBEDTLS_ALLOW_PRIVATE_ACCESS → private_access.h defines
    # MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS, enabling all declarations in
    # mbedtls/private/ecp.h and mbedtls/private/pk_private.h.
    "-DMBEDTLS_ALLOW_PRIVATE_ACCESS"
    # Suppress add_entropy_source() in PlatformManagerImpl.cpp — the PSA
    # backend returns CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE for that call.
    "-DCONFIG_MBEDTLS_ENTROPY_POLL_ZEPHYR=1"
    # ── ODR fix: ensure ALL GN targets see the same Server class layout ──
    # Zephyr's autoconf.h (-imacros) is only applied to GN targets that use
    # target_cflags_cc (e.g. Server.cpp).  Codegen cluster targets
    # (zzz_generated/app-common/clusters/*) are compiled WITHOUT autoconf.h.
    # CHIPPlatformConfig.h gates several layout-affecting defines on Kconfig
    # symbols from autoconf.h (CONFIG_CHIP_CRYPTO_PSA → SHA256 context size,
    # CONFIG_CHIP_LOG_SIZE_OPTIMIZATION → log module enables).
    # Without these, the Server class has a different memory layout in codegen
    # TUs → inline GetFabricTable() computes the wrong offset → ODR violation
    # → mOperationalKeystore appears NULL → CSR fails with error 3.
    # Propagate the critical Kconfig symbols via CHIP_CFLAGS so every GN TU
    # sees the same class layout.
    "-DCONFIG_CHIP_CRYPTO_PSA"
)

# ── Pre-set GN_EXECUTABLE ────────────────────────────────────────────────────
# chip-module's CMakeLists.txt calls find_program(GN_EXECUTABLE gn) during
# find_package(Zephyr).  On Windows, gn.exe lives in the Zephyr venv alongside
# west and python.exe.  If gn is not on the system PATH (common in CI and fresh
# shells), pre-populate the cache variable before find_program() runs.
#
# Search order:
#  1. Next to WEST_PYTHON (set by west when invoking cmake)
#  2. Next to Python3_EXECUTABLE if explicitly provided
#  3. Well-known venv locations relative to the workspace root
if(NOT GN_EXECUTABLE)
    # Collect candidate script directories to search
    set(_gn_search_dirs "")

    if(DEFINED WEST_PYTHON)
        get_filename_component(_d "${WEST_PYTHON}" DIRECTORY)
        list(APPEND _gn_search_dirs "${_d}")
    endif()

    if(DEFINED Python3_EXECUTABLE)
        get_filename_component(_d "${Python3_EXECUTABLE}" DIRECTORY)
        list(APPEND _gn_search_dirs "${_d}")
    endif()

    # Fallback: common venv locations relative to workspace root
    # (workspace = parent of repo; also check inside repo itself)
    foreach(_venv
            "${WORKSPACE_DIR}/.zephyr-venv/Scripts"
            "${RT583_PROJECT_DIR}/.zephyr-venv/Scripts"
            "${WORKSPACE_DIR}/.venv/Scripts"
            "${RT583_PROJECT_DIR}/.venv/Scripts"
    )
        list(APPEND _gn_search_dirs "${_venv}")
    endforeach()

    if(WIN32)
        set(_gn_name "gn.exe")
    else()
        set(_gn_name "gn")
    endif()

    foreach(_dir ${_gn_search_dirs})
        set(_candidate "${_dir}/${_gn_name}")
        if(EXISTS "${_candidate}")
            set(GN_EXECUTABLE "${_candidate}" CACHE FILEPATH "Path to gn" FORCE)
            message(STATUS "Pre-set GN_EXECUTABLE: ${GN_EXECUTABLE}")
            break()
        endif()
    endforeach()
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt583-matter-lighting VERSION 1.0.0)

# ── C++17 (required by Matter) ───────────────────────────────────────────────
include(${CHIP_ROOT}/config/zephyr/app/enable-gnu-std.cmake)

# ── Matter data model (ZAP-generated code + cluster implementations) ─────────
# connectedhomeip codegen scripts (codegen_paths.py, codegen.py) import:
#   'python_path'  — Pigweed helper (not present without full Pigweed bootstrap)
#   'matter.idl.*' — from ${CHIP_ROOT}/scripts/py_matter_idl/
#
# python_path.py is generated into the build directory at configure time so
# the source tree needs no committed shim files.
set(_CHIP_SCRIPTS_COMPAT "${CMAKE_BINARY_DIR}/chip_scripts_compat")
file(WRITE "${_CHIP_SCRIPTS_COMPAT}/python_path.py"
[=["""Minimal shim for Pigweed's python_path module (auto-generated, do not edit).
connectedhomeip codegen scripts import PythonPath from this module when Pigweed
is not fully bootstrapped.  PythonPath(subdir, relative_to=__file__) is a
context manager that prepends <dirname(relative_to)>/<subdir> to sys.path.
"""
import os, sys

class PythonPath:
    def __init__(self, subdir, relative_to=None):
        base = os.path.dirname(os.path.abspath(relative_to)) if relative_to else os.getcwd()
        self._path = os.path.join(base, subdir)
        self._added = False
    def __enter__(self):
        if self._path not in sys.path:
            sys.path.insert(0, self._path)
            self._added = True
        return self
    def __exit__(self, *_):
        if self._added:
            try: sys.path.remove(self._path)
            except ValueError: pass
        return False
]=])

if(WIN32)
    set(ENV{PYTHONPATH} "${CHIP_ROOT}/scripts;${_CHIP_SCRIPTS_COMPAT};$ENV{PYTHONPATH}")
else()
    set(ENV{PYTHONPATH} "${CHIP_ROOT}/scripts:${_CHIP_SCRIPTS_COMPAT}:$ENV{PYTHONPATH}")
endif()

include(${CHIP_ROOT}/src/app/chip_data_model.cmake)

# ── Diagnostic: dump codegen_paths.py state so sysbuild failures are visible ──
# chip_codegen.cmake runs codegen_paths.py via execute_process without capturing
# return code, so a failure there silently yields an empty OUT_NAMES list and
# later blows up at add_custom_command. Repeat the call with RESULT/ERROR vars.
find_package(Python3 QUIET)
message(STATUS "[rt583-diag] Python3_EXECUTABLE = ${Python3_EXECUTABLE}")
message(STATUS "[rt583-diag] VIRTUAL_ENV = $ENV{VIRTUAL_ENV}")
message(STATUS "[rt583-diag] CHIP_ROOT = ${CHIP_ROOT}")
message(STATUS "[rt583-diag] CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -X utf8
        "${CHIP_ROOT}/scripts/codegen_paths.py"
        "--idl" "${CMAKE_CURRENT_LIST_DIR}/lighting-app.matter"
        "app/PluginApplicationCallbacks.h" "app/callback-stub.cpp"
    RESULT_VARIABLE _rt583_cp_rc
    OUTPUT_VARIABLE _rt583_cp_out
    ERROR_VARIABLE  _rt583_cp_err
)
message(STATUS "[rt583-diag] codegen_paths.py rc=${_rt583_cp_rc}")
message(STATUS "[rt583-diag] codegen_paths.py stdout: ${_rt583_cp_out}")
message(STATUS "[rt583-diag] codegen_paths.py stderr: ${_rt583_cp_err}")

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

# ── ODR fix: align CMake CHIP_CONFIG_* with GN build ─────────────────────────
# chip_configure_data_model() adds CodegenIntegration.cpp files to the CMake
# app target.  These call inline Server accessors (GetFabricTable, etc.) whose
# compiled offset depends on CHIP_CONFIG_* macros from GN-generated headers in
# gen/include (AppBuildConfig.h, ICDServerBuildConfig.h, CHIPDeviceBuildConfig.h).
#
# Problem 1 — wrong CHIP_HAVE_CONFIG_H value:
#   Zephyr sets -DCHIP_HAVE_CONFIG_H (no value → expands to empty → #if evaluates
#   to 0), so AppBuildConfig.h is NOT included and all CHIP_CONFIG_* macros use
#   their defaults.  GN sets -DCHIP_HAVE_CONFIG_H=1 → AppBuildConfig.h IS
#   included.  This causes Server member sizes (SessionManager, ExchangeManager)
#   to differ by 312 bytes between the two build systems.
#
# Problem 2 — missing gen/include search path:
#   The CMake build doesn't add gen/include to its -I list, so even if
#   CHIP_HAVE_CONFIG_H=1 were set, #include <app/AppBuildConfig.h> would fail.
#
# Fix: prepend gen/include to the include search path and redefine
# CHIP_HAVE_CONFIG_H=1 (using -U to clear the valueless definition first).
# This makes all CodegenIntegration.cpp TUs see the same struct layouts as the
# GN-compiled Server.cpp, eliminating the ODR violation on inline accessors.
target_include_directories(app BEFORE PRIVATE
    "${CMAKE_BINARY_DIR}/modules/connectedhomeip/gen/include"
)
target_compile_options(app PRIVATE
    -UCHIP_HAVE_CONFIG_H
    -DCHIP_HAVE_CONFIG_H=1
)

# ── Include directories ──────────────────────────────────────────────────────
# SoC platform/HOSAL/crypto include dirs and hosal_gpio.c are added by the
# module-level CMakeLists (cmake/zephyr_module/CMakeLists.txt) — example only
# adds Matter-specific paths here.
target_include_directories(app PRIVATE
    src/include
    ${GEN_DIR}
    ${GEN_DIR}/app-common
    ${GEN_DIR}/lighting-app
    ${CHIP_ROOT}/examples/lighting-app/lighting-common
    ${CHIP_ROOT}/examples/providers
)

# ── Application sources ──────────────────────────────────────────────────────
target_sources(app PRIVATE
    src/main.cpp
    src/AppTask.cpp
    src/CryptoSelfTest.cpp
    src/ZclCallbacks.cpp
    src/MallocWrappers.cpp
    src/ecp_enum_compat.c
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
# --wrap redirects all callers through __wrap_mbedtls_ecp_group_load which translates
# 1→3 before forwarding to OT's __real implementation.
zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_group_load)
# psa_key_derivation_setup safety-net: restores operation->alg and capacity when
# a linker resolution bug zeroes them.  Root cause also fixed in psa_crypto.c.
zephyr_ld_options(-Wl,--wrap=psa_key_derivation_setup)

# ── BLE + OpenThread subsystems ──────────────────────────────────────────────
if(CONFIG_BLE_RT583 OR CONFIG_BLE_RT584)
    set(RAFAEL_SDK ${RT583_PROJECT_DIR}/sdk)
    include(${RT583_PROJECT_DIR}/subsys/ble/CMakeLists.txt)
endif()

if(CONFIG_OPENTHREAD_RT583 OR CONFIG_OPENTHREAD_RT584)
    # subsys/openthread/CMakeLists.txt expects RAFAEL_SDK; set it here in case
    # the BLE branch above didn't run (rt584 currently builds without BLE).
    if(NOT DEFINED RAFAEL_SDK)
        set(RAFAEL_SDK ${RT583_PROJECT_DIR}/sdk)
    endif()
    # Matter doesn't need the OT CLI — disable it to save ~37 KB flash / 7 KB BSS.
    set(OT_INCLUDE_CLI 0)
    include(${RT583_PROJECT_DIR}/subsys/openthread/CMakeLists.txt)
endif()

# comm_subsystem_drv.c — added by module-level CMake when OT/BLE is on.

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
    CONFIG_RF_FW_INCLUDE_MULTI_2P4G=TRUE
    CONFIG_RF_MCU_CONST_LOAD_SUPPORTED=TRUE
    CONFIG_CRYPTO_AES_ENABLE=1
)

# ── ODR fix: Server class layout must match across Zephyr-side and GN-side ─────
# CONFIG_CHIP_USE_OT_ENDPOINT=y in prj.conf is consumed by
# connectedhomeip/config/common/cmake/chip_gn_kconfig_defaults.cmake only when
# that helper runs under the chip-module Kconfig path — it translates the
# Kconfig symbol into a GN arg that surfaces as
# CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT=1 inside GN's SystemBuildConfig.h.
#
# Every GN-side TU (Server.cpp, FabricTable.cpp, SessionManager.cpp, …) sees
# the flag that way, but Zephyr-side TUs like AppTask.cpp do NOT pull in
# SystemBuildConfig.h.  Without the define, Server.h / SessionManager.h compile
# with different sizeof(UDPEndPoint) + sizeof(TransportMgr) and Server::mFabrics
# lands at a different offset for each side — making Server::GetInstance()
# .GetFabricTable() return a pointer that is 312 bytes off from the instance
# that Server::Init() populated.  Symptom: FabricTable::mOperationalKeystore
# reads NULL and CSRRequest fails with CHIP_ERROR_INCORRECT_STATE (0x03).
#
# Propagate the flag to every Zephyr-side TU (app + related modules) so both
# sides agree on the Server layout.
zephyr_compile_definitions(CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT=1)

# Suppress warnings in vendored SDK sources
target_compile_options(app PRIVATE -Wno-error=maybe-uninitialized)

# OT's bundled mbedTLS 3.6 vs Zephyr's TF-PSA-Crypto 4.0: cipher_context_t
# struct layout differs.  OT code is compiled against 3.6 headers, so the
# compiled .c bodies (cipher.c / constant_time.c / psa_util.c) MUST come from
# OT's 3.6 set — otherwise mbedtls_cipher_free dereferences the wrong struct
# offset for ctx_free_func and jumps to a garbage function pointer.
# -Wl,--allow-multiple-definition + libapp.a ordered before libtfpsacrypto.a
# makes OT's 3.6 versions win the link.
zephyr_ld_options(-Wl,--allow-multiple-definition)
