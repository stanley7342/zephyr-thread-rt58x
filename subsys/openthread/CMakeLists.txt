# subsys/openthread/CMakeLists.txt — RT583 OpenThread FTD platform
# Included by the root CMakeLists.txt when CONFIG_OPENTHREAD_RT583=y.

set(OT_PORT_INC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Inc)
set(OT_PORT_SRC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Src)
set(OT_PORT_UTILS ${RAFAEL_SDK}/components/network/thread/openthread_port/utils)
set(OT_ROOT       ${RAFAEL_SDK}/components/network/thread/openthread)

set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt583)
set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt583)
set(RUCI_INC      ${RAFAEL_SDK}/components/network/ruci/Inc)
set(LMAC_INC      ${RAFAEL_SDK}/components/network/lmac15p4/Inc)
set(RT583_DRV_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_driver/Inc)
set(RT583_SYS_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_system/Include)
set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Inc)
set(RT583_CRYPTO_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_crypto/Inc)
set(RT583_CRYPTO_SRC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_crypto)
set(UTIL_INC      ${RAFAEL_SDK}/components/utility/utility/Inc)
set(LOG_INC       ${RAFAEL_SDK}/components/utility/log)
set(MBEDTLS_INC   ${OT_ROOT}/third_party/mbedtls/repo/include)

# ── Collect OT core sources ───────────────────────────────────────────────────
# CLI is excluded for the Matter build (not needed; saves ~37 KB flash + 7 KB BSS).
# When building the Thread FTD CLI example, the CLI sources are needed; add them
# back by setting OT_INCLUDE_CLI=1 before including this file.
file(GLOB_RECURSE OT_CORE_SOURCES
    ${OT_ROOT}/src/core/*.cpp
)
if(NOT DEFINED OT_INCLUDE_CLI OR OT_INCLUDE_CLI)
    file(GLOB_RECURSE OT_CLI_SOURCES
        ${OT_ROOT}/src/cli/*.cpp
    )
    list(APPEND OT_CORE_SOURCES ${OT_CLI_SOURCES})
endif()
# extension_example.cpp requires optional OT Extension API — exclude.
list(FILTER OT_CORE_SOURCES EXCLUDE REGEX "extension_example\\.cpp$")

# ── Collect tcplp sources ─────────────────────────────────────────────────────
file(GLOB_RECURSE OT_TCPLP_SOURCES
    ${OT_ROOT}/third_party/tcplp/bsdtcp/*.c
    ${OT_ROOT}/third_party/tcplp/lib/*.c
)
list(FILTER OT_TCPLP_SOURCES EXCLUDE REGEX "test_all\.c$")

# ── Collect mbedTLS sources ───────────────────────────────────────────────────
# All files are guarded by MBEDTLS_* macros; POSIX-only files excluded below.
file(GLOB OT_MBEDTLS_SOURCES
    ${OT_ROOT}/third_party/mbedtls/repo/library/*.c
)
list(FILTER OT_MBEDTLS_SOURCES EXCLUDE REGEX "net_sockets\\.c$")
list(FILTER OT_MBEDTLS_SOURCES EXCLUDE REGEX "entropy_poll\\.c$")

target_sources(app PRIVATE
    # RT583 OT system overrides (must come before libopenthread_platform.a in
    # link order so --allow-multiple-definition picks our platformRadioInit/Process)
    ${CMAKE_CURRENT_LIST_DIR}/platform/rt583_otsys.c

    # OT platform port
    ${OT_PORT_SRC}/ot_alarm.c
    ${OT_PORT_SRC}/ot_radio.c
    ${OT_PORT_SRC}/ot_entropy.c
    ${OT_PORT_SRC}/ot_settings.c
    ${OT_PORT_SRC}/ot_misc.c
    ${OT_PORT_SRC}/ot_logging.c
    ${OT_PORT_SRC}/ot_memory.c
    ${OT_PORT_SRC}/ot_diag.c
    ${OT_PORT_SRC}/ot_crypto.c
    ${OT_PORT_SRC}/rt583_compat.c
    ${OT_PORT_SRC}/log_shim.c

    # LMAC + RF MCU
    ${RAFAEL_SDK}/components/network/lmac15p4/Src/lmac15p4.c
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src/hosal_rf.c
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src/hosal_trng.c
    ${RT569_RF_DIR}/Src/rf_mcu.c
    ${RT569_RF_DIR}/Src/rf_mcu_ahb.c
    ${RT569_RF_DIR}/Src/rf_common_init.c
    ${RT569_FW_DIR}/Src/rt569mp_fw.c
    ${RT569_FW_DIR}/Src/rt569mp_init.c

    # rt583_crypto (replaces librt583_crypto.a)
    ${RT583_CRYPTO_SRC}/rt_aes.c
    ${RT583_CRYPTO_SRC}/rt_crypto.c
    ${RT583_CRYPTO_SRC}/rt_crypto_util.c
    ${RT583_CRYPTO_SRC}/rt_ecjpake.c
    ${RT583_CRYPTO_SRC}/rt_hmac_drbg.c
    ${RT583_CRYPTO_SRC}/rt_sha256.c

    # RUCI (replaces libruci.a)
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_cmn_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_cmn_hal_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_cmn_sys_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci15p4_mac_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_ble_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_common_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_data.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_fsk_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_ook_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_oqpsk_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_slink_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_zwave_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_host_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_host_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_host_sys_cmd.c

    # Utility (replaces libutility.a)
    ${RAFAEL_SDK}/components/utility/utility/Src/fsm.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_fcs.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_list.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_queue.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_string.c

    # Log (replaces liblog.a)
    ${RAFAEL_SDK}/components/utility/log/log.c

    # OpenThread core + CLI (replaces libopenthread-ftd.a + libopenthread-cli-ftd.a)
    ${OT_CORE_SOURCES}

    # tcplp (replaces libtcplp-ftd.a)
    ${OT_TCPLP_SOURCES}

    # mbedTLS (replaces libmbedtls.a + libmbedx509.a + libmbedcrypto.a)
    ${OT_MBEDTLS_SOURCES}

    # openthread_port utils.
    # soft_source_match_table.c excluded: ot_radio.c provides hardware-backed
    # otPlatRadio*SrcMatch* via lmac15p4 directly.
    ${OT_PORT_UTILS}/mac_frame.cpp
    ${OT_PORT_UTILS}/mac_frame_gen.c

    # spinel (was in libopenthread-platform-utils-static.a)
    ${OT_ROOT}/src/lib/spinel/spinel.c
)

# OT thread management: ot_zephyr.c for the Thread CLI example;
# ot_matter_sysq.c for the Matter build.
#
#   Thread CLI (OT_INCLUDE_CLI=1):
#     ot_zephyr.c — creates a dedicated otrStackTask thread; defines
#     otTaskletsSignalPending/otSysEventSignalPending to give ot_task_sem.
#
#   Matter (OT_INCLUDE_CLI=0):
#     ot_matter_sysq.c — no custom thread; lets Zephyr's openthread_work_q
#     process tasklets.  otSysEventSignalPending() (Zephyr's) submits to the
#     work queue, so OT_NOTIFY from alarm/radio ISRs wakes the right thread.
if(NOT DEFINED OT_INCLUDE_CLI OR OT_INCLUDE_CLI)
    target_sources(app PRIVATE ${OT_PORT_SRC}/ot_zephyr.c)
    target_sources(app PRIVATE ${OT_PORT_SRC}/ot_uart.c)
else()
    target_sources(app PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/platform/ot_matter_sysq.c)
endif()

target_include_directories(app PRIVATE
    ${OT_PORT_INC}
    # tprintk.h is provided by the thread example app — OT port sources include it
    ${CMAKE_CURRENT_LIST_DIR}/../../examples/thread/src
    ${OT_ROOT}/include
    ${OT_ROOT}/src
    ${OT_ROOT}/src/core
    ${OT_ROOT}/src/include
    ${OT_ROOT}/src/lib/spinel
    ${OT_ROOT}/third_party/tcplp/bsdtcp
    ${OT_ROOT}/third_party/tcplp/lib
    ${MBEDTLS_INC}
    ${LMAC_INC}
    ${RT583_DRV_INC}
    ${RT583_SYS_INC}
    ${HOSAL_INC}
    ${RT583_CRYPTO_INC}
    ${UTIL_INC}
    ${LOG_INC}
    ${RT569_RF_DIR}/Inc
    ${RT569_FW_DIR}/Inc
    ${RUCI_INC}
)

# Expose OT port config headers globally (needed for OPENTHREAD_CONFIG_FILE resolution).
# NOTE: ${OT_ROOT}/include (SDK's vendored OT) is intentionally NOT added here as a user
# include (-I) — it would shadow modules/lib/openthread/include and break compilation of
# Zephyr's openthread-ftd library with missing BR diagnostic symbols.
# The SDK OT headers are already in target_include_directories(app PRIVATE ...) above.
zephyr_include_directories(
    ${OT_PORT_INC}
)

# When CONFIG_OPENTHREAD_RT583=y, CONFIG_OPENTHREAD_SOURCES=n so Zephyr's OT module does
# NOT add modules/lib/openthread/include to the global include path. Add it here as a
# system include (-isystem, lower priority) so:
#  - Zephyr's OT L2 layer (openthread.c) can find <openthread/instance.h>
#  - Matter's GN build can find <openthread/instance.h>
#  - SDK OT compilation (app PRIVATE) still uses SDK's own OT headers first (via -I)
zephyr_system_include_directories(
    ${ZEPHYR_BASE}/../modules/lib/openthread/include
    ${ZEPHYR_BASE}/../modules/lib/openthread/examples/platforms
)

target_compile_definitions(app PRIVATE
    OPENTHREAD_CONFIG_FILE="openthread-core-rt583-zephyr-config.h"
    CONFIG_RT583=1
    OPENTHREAD_FTD=1
    OPENTHREAD_MTD=0
    OPENTHREAD_RADIO=0
    CHIP_TYPE=3
    CONFIG_CRYPTO_SECP256R1_ENABLE=1
    # Matter needs its own state-change callback alongside Zephyr L2's.
    OPENTHREAD_CONFIG_MAX_STATECHANGE_HANDLERS=3
    "PACKAGE_NAME=\"OpenThread\""
    "PACKAGE_VERSION=\"1.0.0\""
)

# CONFIG_RF_FW_INCLUDE_PCI and MBEDTLS_CONFIG_FILE may already be defined by
# the BLE subsystem or Zephyr's mbedTLS module (INTERFACE properties that
# propagate before our PRIVATE compile_definitions).  To avoid redefinition
# warnings, emit -U before -D in target_compile_options — which is appended
# after target_compile_definitions in the compiler command.
target_compile_options(app PRIVATE
    -UCONFIG_RF_FW_INCLUDE_PCI
    -DCONFIG_RF_FW_INCLUDE_PCI=TRUE
    -UMBEDTLS_CONFIG_FILE
    "-DMBEDTLS_CONFIG_FILE=\"openthread-mbedtls-config.h\""
)

zephyr_compile_definitions(
    OPENTHREAD_CONFIG_FILE="openthread-core-rt583-zephyr-config.h"
    CONFIG_RT583=1
    OPENTHREAD_FTD=1
)

# _fini is referenced by newlib's __libc_fini_array (C++ destructor path) but not
# provided on bare-metal targets. Define it as zero so the linker is satisfied.
zephyr_ld_options(-Wl,--defsym=_fini=0)
