# subsys/openthread/CMakeLists.txt — Rafael (rt583/rt584) OpenThread FTD platform
# Included by the root CMakeLists.txt when CONFIG_OPENTHREAD_RT583=y or
# CONFIG_OPENTHREAD_RT584=y. SoC-specific paths (RF MCU sources/firmware,
# driver/system include dirs, hosal) are selected from the active SoC; the
# rest of the OT stack (lmac15p4, RUCI, openthread_port glue) is shared.

set(OT_PORT_INC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Inc)
set(OT_PORT_SRC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Src)
set(OT_PORT_UTILS ${RAFAEL_SDK}/components/network/thread/openthread_port/utils)
set(OT_ROOT       ${RAFAEL_SDK}/components/network/thread/openthread)
set(RUCI_INC      ${RAFAEL_SDK}/components/network/ruci/Inc)
set(LMAC_INC      ${RAFAEL_SDK}/components/network/lmac15p4/Inc)

if(CONFIG_OPENTHREAD_RT584)
    set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt584)
    # rt584 RF header dir is lowercase `include/`; rt583 uses `Inc/`.
    set(RT569_RF_INC  ${RT569_RF_DIR}/include)
    set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt584)
    set(RT569_FW_INC  ${RT569_FW_DIR}/include)
    set(RT583_DRV_INC ${RAFAEL_SDK}/components/platform/soc/rt584/rt584_driver/Inc)
    set(RT583_SYS_INC ${RAFAEL_SDK}/components/platform/soc/rt584/rt584_system/Include)
    set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt584_hosal/Inc)
    # rt584 crypto sources live in rt584_crypto/ (vendored upstream); rt583
    # has rt583_crypto/. Share the variable names so the rest of this file
    # stays SoC-agnostic.
    set(RT583_CRYPTO_INC ${RAFAEL_SDK}/components/platform/soc/rt584/rt584_crypto/Inc)
    set(RT583_CRYPTO_SRC ${RAFAEL_SDK}/components/platform/soc/rt584/rt584_crypto)
else()
    set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt583)
    set(RT569_RF_INC  ${RT569_RF_DIR}/Inc)
    set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt583)
    set(RT569_FW_INC  ${RT569_FW_DIR}/Inc)
    set(RT583_DRV_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_driver/Inc)
    set(RT583_SYS_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_system/Include)
    set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Inc)
    set(RT583_CRYPTO_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_crypto/Inc)
    set(RT583_CRYPTO_SRC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_crypto)
endif()
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
# Keep crypto_platform.cpp: the Thread CLI build (no CONFIG_OPENTHREAD_CRYPTO_PSA)
# relies on its default otPlatCryptoEcdsa* / otPlatCryptoPbkdf2GenerateKey
# implementations.  Conflicts with our ot_crypto.c are absorbed by
# -Wl,--allow-multiple-definition (ot_crypto.c wins because libapp.a is ahead
# in link order) — see examples/thread/CMakeLists.txt for the rationale.

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
# Exclude OT's mbedTLS 3.6.0 psa_crypto_mac.c — it uses a different
# psa_hash_operation_s / mbedtls_psa_hmac_operation_t layout than
# TF-PSA-Crypto 4.0 (which the Matter stack is built against).
# Without this exclusion, --allow-multiple-definition causes OT's version to
# win the link, placing hmac->opad 4 bytes early (into sha256.is224 = 0x00),
# producing the wrong HKDF PRK and a wrong cB → PASE integrity check failure.
list(FILTER OT_MBEDTLS_SOURCES EXCLUDE REGEX "psa_crypto_mac\\.c$")
# Exclude OT's sha256.c — it is compiled with MBEDTLS_SHA224_C (from
# openthread-mbedtls-config.h), so mbedtls_sha256_starts writes ctx->is224=0
# at ctx+104 bytes.  TF-PSA-Crypto 4.0's psa_crypto_mac.c is compiled without
# SHA224_C, placing opad at hmac+116 — exactly where OT's sha256_starts clobbers
# it.  Using TF-PSA-Crypto's sha256.c (no SHA224_C, no is224 write) removes the
# corruption.
list(FILTER OT_MBEDTLS_SOURCES EXCLUDE REGEX "sha256\\.c$")
# cipher.c / constant_time.c / psa_util.c are kept even though they also exist
# in Zephyr's TF-PSA-Crypto 4.0: struct layouts (e.g. mbedtls_cipher_context_t)
# differ between OT's bundled 3.6 and TF-PSA 4.0, so OT code compiled against
# 3.6 headers MUST call the 3.6 .c body at runtime — otherwise mbedtls_cipher_free
# dereferences ctx_free_func at the wrong struct offset and jumps to garbage.
# Duplicate-definition link errors between libapp.a and libtfpsacrypto.a are
# absorbed by -Wl,--allow-multiple-definition in examples/matter/lighting-app/
# CMakeLists.txt (libapp.a comes first → OT 3.6 wins).

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

    # RT583 hardware crypto ALT backends (AES + ECC share one accelerator)
    # rt583_crypto_mutex.c provides crypto_mutex_lock/unlock via Zephyr k_mutex.
    # aes_alt.c / ecdsa_alt.c / ecdh_alt.c wire mbedTLS ALT hooks to hardware.
    # CONFIG_SUPPORT_MULTITASKING (below) activates the mutex calls in rt_aes.c
    # and rt_crypto.c so that concurrent AES and ECC requests serialise safely.
    ${OT_PORT_SRC}/rt583_crypto_mutex.c
    ${OT_PORT_SRC}/aes_alt.c
    ${OT_PORT_SRC}/ecdsa_alt.c
    ${OT_PORT_SRC}/ecdh_alt.c

    # LMAC + RF MCU
    # NOTE: hosal_trng.c is compiled once by drivers/entropy (drivers__entropy
    # library) via cmake/zephyr_module/CMakeLists.txt — do NOT add it here or
    # hosal_trng_get_random_number duplicates across two archives.
    ${RAFAEL_SDK}/components/network/lmac15p4/Src/lmac15p4.c
    ${RT569_RF_DIR}/Src/rf_mcu.c
    ${RT569_RF_DIR}/Src/rf_mcu_ahb.c
    ${RT569_RF_DIR}/Src/rf_common_init.c

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

# SoC-specific RF firmware blob filenames differ:
#   rt583 → rt569mp_{fw,init}.c
#   rt584 → rt569s_{fw,init}.c
# The hosal_rf source is always the rt583 path because that file is our
# hand-written Zephyr port (FreeRTOS → k_sem/k_msgq/k_thread). rt584's
# upstream hosal_rf.c is the original FreeRTOS vendor code; can't be
# compiled here. The hosal_rf API surface matches between the two SoCs,
# and the rt584 RF MCU flow is driven through rf_mcu.c (SoC-specific) so
# the Zephyr-port hosal_rf.c works for both.
target_sources(app PRIVATE
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src/hosal_rf.c
)
if(CONFIG_OPENTHREAD_RT584)
    target_sources(app PRIVATE
        ${RT569_FW_DIR}/Src/rt569s_fw.c
        ${RT569_FW_DIR}/Src/rt569s_init.c
    )
    # SDK driver sources (sadc / mp_sector / wdt / trng / otp / hosal_trng)
    # and rt584_compat.c are now pulled in at module level
    # (cmake/zephyr_module/CMakeLists.txt) under
    #   if(CONFIG_OPENTHREAD_RT584 OR CONFIG_BLE_RT584)
    # so the BLE-only build also gets them. No duplication needed here.
else()
    target_sources(app PRIVATE
        ${RT569_FW_DIR}/Src/rt569mp_fw.c
        ${RT569_FW_DIR}/Src/rt569mp_init.c
    )
endif()

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
    # Tell rt583_otsys.c NOT to compile the no-op ot_uartTask stub —
    # ot_uart.c provides the real implementation for the CLI build.
    target_compile_definitions(app PRIVATE OT_INCLUDE_CLI=1)
else()
    target_sources(app PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/platform/ot_matter_sysq.c)
    # Tell rt583_otsys.c to compile the no-op ot_uartTask stub —
    # ot_uart.c is not compiled in the Matter build.
    target_compile_definitions(app PRIVATE OT_INCLUDE_CLI=0)
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
    ${RT569_RF_INC}
    ${RT569_FW_INC}
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

if(CONFIG_OPENTHREAD_RT584)
    # rt584 RF firmware blob is selected by CONFIG_RT584H / CONFIG_RT584L
    # (preprocessor defines, not Kconfig). User's chip is rt584L (chip ID
    # 0x1584). To target rt584H, change to CONFIG_RT584H=1.
    target_compile_definitions(app PRIVATE CONFIG_RT584L=1)
else()
    target_compile_definitions(app PRIVATE CHIP_TYPE=3)
endif()

target_compile_definitions(app PRIVATE
    OPENTHREAD_CONFIG_FILE="openthread-core-rt583-zephyr-config.h"
    OPENTHREAD_FTD=1
    OPENTHREAD_MTD=0
    OPENTHREAD_RADIO=0
    CONFIG_CRYPTO_SECP256R1_ENABLE=1
    # Enable AES hardware acceleration in rt_aes.c (guards aes_key_init,
    # aes_ecb_encrypt, etc. behind #if defined(CONFIG_CRYPTO_AES_ENABLE)).
    CONFIG_CRYPTO_AES_ENABLE=1
    # Enable mutex calls in rt_aes.c and rt_crypto.c so that concurrent AES
    # and ECC operations serialise through crypto_mutex_lock/unlock.
    CONFIG_SUPPORT_MULTITASKING=1
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
    # Multi-protocol (BLE + 15.4) firmware blob — required when
    # hosal_rf_init(HOSAL_RF_MODE_MULTI_PROTOCOL) is used (Matter build).
    # Without this, firmware_program_multi[] = {0} (stub) and the RF MCU
    # loads an invalid firmware → Thread RX completely non-functional.
    -UCONFIG_RF_FW_INCLUDE_MULTI_2P4G
    -DCONFIG_RF_FW_INCLUDE_MULTI_2P4G=TRUE
    -UMBEDTLS_CONFIG_FILE
    "-DMBEDTLS_CONFIG_FILE=\"openthread-mbedtls-config.h\""
)

zephyr_compile_definitions(
    OPENTHREAD_CONFIG_FILE="openthread-core-rt583-zephyr-config.h"
    OPENTHREAD_FTD=1
)
if(NOT CONFIG_OPENTHREAD_RT584)
    zephyr_compile_definitions(CONFIG_RT583=1)
endif()

# _fini is referenced by newlib's __libc_fini_array (C++ destructor path) but not
# provided on bare-metal targets. Define it as zero so the linker is satisfied.
zephyr_ld_options(-Wl,--defsym=_fini=0)
