# subsys/openthread/CMakeLists.txt — RT582 OpenThread FTD platform
# Included by the root CMakeLists.txt when CONFIG_OPENTHREAD_RT582=y.

set(OT_PORT_INC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Inc)
set(OT_PORT_SRC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Src)
set(OT_PORT_UTILS ${RAFAEL_SDK}/components/network/thread/openthread_port/utils)
set(OT_ROOT       ${RAFAEL_SDK}/components/network/thread/openthread)

set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt582)
set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt582)
set(RUCI_INC      ${RAFAEL_SDK}/components/network/ruci/Inc)
set(LMAC_INC      ${RAFAEL_SDK}/components/network/lmac15p4/Inc)
set(RT582_DRV_INC ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_driver/Inc)
set(RT582_SYS_INC ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_system/Include)
set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Inc)
set(RT582_CRYPTO_INC ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_crypto/Inc)
set(RT582_CRYPTO_SRC ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_crypto)
set(UTIL_INC      ${RAFAEL_SDK}/components/utility/utility/Inc)
set(LOG_INC       ${RAFAEL_SDK}/components/utility/log)
set(MBEDTLS_INC   ${OT_ROOT}/third_party/mbedtls/repo/include)

# ── Collect OT core + CLI sources ────────────────────────────────────────────
file(GLOB_RECURSE OT_CORE_SOURCES
    ${OT_ROOT}/src/core/*.cpp
    ${OT_ROOT}/src/cli/*.cpp
)
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
    # OT platform port
    ${OT_PORT_SRC}/ot_zephyr.c
    ${OT_PORT_SRC}/ot_alarm.c
    ${OT_PORT_SRC}/ot_radio.c
    ${OT_PORT_SRC}/ot_uart.c
    ${OT_PORT_SRC}/ot_entropy.c
    ${OT_PORT_SRC}/ot_settings.c
    ${OT_PORT_SRC}/ot_misc.c
    ${OT_PORT_SRC}/ot_logging.c
    ${OT_PORT_SRC}/ot_memory.c
    ${OT_PORT_SRC}/ot_diag.c
    ${OT_PORT_SRC}/ot_crypto.c
    ${OT_PORT_SRC}/rt582_compat.c
    ${OT_PORT_SRC}/log_shim.c

    # LMAC + RF MCU
    ${RAFAEL_SDK}/components/network/lmac15p4/Src/lmac15p4.c
    ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Src/hosal_rf.c
    ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Src/hosal_trng.c
    ${RT569_RF_DIR}/Src/rf_mcu.c
    ${RT569_RF_DIR}/Src/rf_mcu_ahb.c
    ${RT569_RF_DIR}/Src/rf_common_init.c
    ${RT569_FW_DIR}/Src/rt569mp_fw.c
    ${RT569_FW_DIR}/Src/rt569mp_init.c

    # rt582_crypto (replaces librt582_crypto.a)
    ${RT582_CRYPTO_SRC}/rt_aes.c
    ${RT582_CRYPTO_SRC}/rt_crypto.c
    ${RT582_CRYPTO_SRC}/rt_crypto_util.c
    ${RT582_CRYPTO_SRC}/rt_ecjpake.c
    ${RT582_CRYPTO_SRC}/rt_hmac_drbg.c
    ${RT582_CRYPTO_SRC}/rt_sha256.c

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

target_include_directories(app PRIVATE
    ${OT_PORT_INC}
    ${OT_ROOT}/include
    ${OT_ROOT}/src
    ${OT_ROOT}/src/core
    ${OT_ROOT}/src/include
    ${OT_ROOT}/src/lib/spinel
    ${OT_ROOT}/third_party/tcplp/bsdtcp
    ${OT_ROOT}/third_party/tcplp/lib
    ${MBEDTLS_INC}
    ${LMAC_INC}
    ${RT582_DRV_INC}
    ${RT582_SYS_INC}
    ${HOSAL_INC}
    ${RT582_CRYPTO_INC}
    ${UTIL_INC}
    ${LOG_INC}
    ${RT569_RF_DIR}/Inc
    ${RT569_FW_DIR}/Inc
    ${RUCI_INC}
)

# Expose OT public headers to the application (main.c uses otAppCliInit etc.)
zephyr_include_directories(
    ${OT_ROOT}/include
    ${OT_PORT_INC}
)

target_compile_definitions(app PRIVATE
    OPENTHREAD_CONFIG_FILE="openthread-core-rt582-zephyr-config.h"
    CONFIG_RT582=1
    OPENTHREAD_FTD=1
    OPENTHREAD_MTD=0
    OPENTHREAD_RADIO=0
    CHIP_TYPE=2
    "CONFIG_RF_FW_INCLUDE_PCI=TRUE"
    CONFIG_CRYPTO_SECP256R1_ENABLE=1
    "MBEDTLS_CONFIG_FILE=\"openthread-mbedtls-config.h\""
    "PACKAGE_NAME=\"OpenThread\""
    "PACKAGE_VERSION=\"1.0.0\""
)

zephyr_compile_definitions(
    OPENTHREAD_CONFIG_FILE="openthread-core-rt582-zephyr-config.h"
    CONFIG_RT582=1
    OPENTHREAD_FTD=1
)

# _fini is referenced by newlib's __libc_fini_array (C++ destructor path) but not
# provided on bare-metal targets. Define it as zero so the linker is satisfied.
zephyr_ld_options(-Wl,--defsym=_fini=0)
