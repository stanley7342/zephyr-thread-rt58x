cmake_minimum_required(VERSION 3.20.0)

set(BOARD_ROOT   ${CMAKE_CURRENT_LIST_DIR})
set(SOC_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(DTS_ROOT     ${CMAKE_CURRENT_LIST_DIR})
set(KCONFIG_ROOT ${CMAKE_CURRENT_LIST_DIR}/Kconfig)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt582_helloworld VERSION 1.0.0)

# Rafael IoT SDK vendored into sdk/ — pre-built libs in sdk/lib are no longer used.
# All platform code is compiled from source under drivers/.
set(RAFAEL_SDK ${CMAKE_CURRENT_LIST_DIR}/sdk)

# Application source
target_sources(app PRIVATE src/main.c)

# ── OpenThread ───────────────────────────────────────────────────────────────
if(CONFIG_OPENTHREAD_RT582)

set(OT_PORT_INC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Inc)
set(OT_PORT_SRC   ${RAFAEL_SDK}/components/network/thread/openthread_port/Src)
set(OT_PORT_UTILS ${RAFAEL_SDK}/components/network/thread/openthread_port/utils)
set(OT_ROOT       ${RAFAEL_SDK}/components/network/thread/openthread)

set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt582)
set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt582)
set(RUCI_INC      ${RAFAEL_SDK}/components/network/ruci/Inc)
set(LMAC_INC      ${RAFAEL_SDK}/components/network/lmac15p4/Inc)
set(RT582_DRV_INC ${CMAKE_CURRENT_LIST_DIR}/drivers/soc/rt582/rt582_driver/Inc)
set(RT582_SYS_INC ${CMAKE_CURRENT_LIST_DIR}/drivers/soc/rt582/rt582_system/Include)
set(HOSAL_INC     ${CMAKE_CURRENT_LIST_DIR}/drivers/hosal/rt582_hosal/Inc)
set(RT582_CRYPTO_INC ${CMAKE_CURRENT_LIST_DIR}/drivers/soc/rt582/rt582_crypto/Inc)
set(RT582_CRYPTO_SRC ${CMAKE_CURRENT_LIST_DIR}/drivers/soc/rt582/rt582_crypto)
set(UTIL_INC      ${RAFAEL_SDK}/components/utility/utility/Inc)
set(LOG_INC       ${RAFAEL_SDK}/components/utility/log)
set(MBEDTLS_INC   ${OT_ROOT}/third_party/mbedtls/repo/include)

# ── Collect OT core + CLI sources (replaces libopenthread-ftd.a + cli-ftd.a) ──
file(GLOB_RECURSE OT_CORE_SOURCES
    ${OT_ROOT}/src/core/*.cpp
    ${OT_ROOT}/src/cli/*.cpp
)
# Rafael's openthread_port/utils/mac_frame.cpp provides only C API wrappers (otMacFrame*).
# OT core's mac_frame.cpp provides the C++ class methods (ot::Mac::Frame::*) — keep both.
# extension_example.cpp depends on optional OT Extension API (ExtensionBase) — exclude.
list(FILTER OT_CORE_SOURCES EXCLUDE REGEX "extension_example\\.cpp$")

# ── Collect tcplp sources (replaces libtcplp-ftd.a) ──
file(GLOB_RECURSE OT_TCPLP_SOURCES
    ${OT_ROOT}/third_party/tcplp/bsdtcp/*.c
    ${OT_ROOT}/third_party/tcplp/lib/*.c
)
list(FILTER OT_TCPLP_SOURCES EXCLUDE REGEX "test_all\.c$")

# ── Collect mbedTLS sources (replaces libmbedtls.a + libmbedx509.a + libmbedcrypto.a) ──
# All files are guarded by MBEDTLS_* macros; POSIX-only code (net_sockets.c
# guarded by MBEDTLS_NET_C) compiles to an empty TU on bare-metal targets.
file(GLOB OT_MBEDTLS_SOURCES
    ${OT_ROOT}/third_party/mbedtls/repo/library/*.c
)
# Exclude POSIX-only files that require Unix/Windows system headers.
list(FILTER OT_MBEDTLS_SOURCES EXCLUDE REGEX "net_sockets\\.c$")
list(FILTER OT_MBEDTLS_SOURCES EXCLUDE REGEX "entropy_poll\\.c$")

target_sources(app PRIVATE
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
    ${RAFAEL_SDK}/components/network/lmac15p4/Src/lmac15p4.c
    ${CMAKE_CURRENT_LIST_DIR}/drivers/hosal/rt582_hosal/Src/hosal_rf.c
    ${OT_PORT_SRC}/log_shim.c
    ${CMAKE_CURRENT_LIST_DIR}/drivers/hosal/rt582_hosal/Src/hosal_trng.c
    ${RT569_RF_DIR}/Src/rf_mcu.c
    ${RT569_RF_DIR}/Src/rf_mcu_ahb.c
    ${RT569_RF_DIR}/Src/rf_common_init.c
    ${RT569_FW_DIR}/Src/rt569mp_fw.c
    ${RT569_FW_DIR}/Src/rt569mp_init.c

    # rt582_crypto: compiled from source (replaces librt582_crypto.a)
    ${RT582_CRYPTO_SRC}/rt_aes.c
    ${RT582_CRYPTO_SRC}/rt_crypto.c
    ${RT582_CRYPTO_SRC}/rt_crypto_util.c
    ${RT582_CRYPTO_SRC}/rt_ecjpake.c
    ${RT582_CRYPTO_SRC}/rt_hmac_drbg.c
    ${RT582_CRYPTO_SRC}/rt_sha256.c

    # RUCI: compiled from source (replaces libruci.a)
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

    # Utility: compiled from source (replaces libutility.a)
    ${RAFAEL_SDK}/components/utility/utility/Src/fsm.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_fcs.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_list.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_queue.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_string.c

    # Log: compiled from source (replaces liblog.a)
    ${RAFAEL_SDK}/components/utility/log/log.c

    # OpenThread core + CLI (replaces libopenthread-ftd.a + libopenthread-cli-ftd.a)
    ${OT_CORE_SOURCES}

    # tcplp (replaces libtcplp-ftd.a)
    ${OT_TCPLP_SOURCES}

    # mbedTLS (replaces libmbedtls.a + libmbedx509.a + libmbedcrypto.a)
    ${OT_MBEDTLS_SOURCES}

    # openthread_port utils (replaces libopenthread_port.a + libopenthread-platform-utils-static.a).
    # Compiling from source eliminates the uint32_t ABI mismatch — all TUs use
    # the same toolchain so unsigned int vs unsigned long is consistent.
    ${OT_PORT_UTILS}/mac_frame.cpp
    ${OT_PORT_UTILS}/mac_frame_gen.c
    ${OT_PORT_UTILS}/soft_source_match_table.c

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

# Global include dirs (so main.c can find OT headers)
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

# Allow multiple definitions (ot_radio.c overlaps with soft_source_match_table.c)
zephyr_ld_options(-Wl,--allow-multiple-definition)
# _fini is referenced by newlib's __libc_fini_array (C++ destructor path) but not
# provided on bare-metal targets. Define it as zero so the linker is satisfied.
zephyr_ld_options(-Wl,--defsym=_fini=0)

endif() # CONFIG_OPENTHREAD_RT582

# ── RT582 platform drivers (compiled from source) ────────────────────────────
# Replaces librt582_system.a + librt582_driver.a.
# Excludes uart_drv.c / uart_retarget.c (UART handled via hosal_uart.c + uart_rt582.c).
# Excludes dwt.c (may conflict with Zephyr cycle-counter support).
set(RT582_DRV_SRC ${CMAKE_CURRENT_LIST_DIR}/drivers/soc/rt582/rt582_driver/Src)
set(RT582_SYS_SRC ${CMAKE_CURRENT_LIST_DIR}/drivers/soc/rt582/rt582_system/Src)

# system_mcu.c uses SET_SYS_CLK and CHIP_TYPE at compile time.
# SET_SYS_CLK=SYS_CLK_64MHZ=2 → SystemInit configures BBPLL to 64 MHz.
# CHIP_TYPE=2 (RT582) — chip_define.h defaults to RT581=1, must override.
# CHIP_VERSION defaults to RT58X_MPB=2 in chip_define.h — correct for EVB.
target_compile_definitions(app PRIVATE
    SET_SYS_CLK=2
    CHIP_TYPE=2
)

target_sources(app PRIVATE
    # System init: SystemInit (BBPLL → 64 MHz), systemfrequencyupdate, mp-sector calibration
    ${RT582_SYS_SRC}/system_mcu.c
    ${RT582_SYS_SRC}/mp_sector.c
    # SOC peripheral drivers (only those needed for Thread FTD)
    # Excluded: i2c_master.c (uart_stdio.h missing), qspi.c (QSPI_AUX member mismatch),
    #           swt.c (swt.h missing), sadc.c (FreeRTOS dep), i2s/pwm/comparator/lpm (unused)
    ${RT582_DRV_SRC}/comm_subsystem_drv.c
    ${RT582_DRV_SRC}/dma.c
    ${RT582_DRV_SRC}/flash_bp_protect.c
    ${RT582_DRV_SRC}/flashctl.c
    ${RT582_DRV_SRC}/gpio.c
    ${RT582_DRV_SRC}/rtc.c
    ${RT582_DRV_SRC}/swi.c
    ${RT582_DRV_SRC}/sysctrl.c
    ${RT582_DRV_SRC}/sysfun.c
    ${RT582_DRV_SRC}/timer.c
    ${RT582_DRV_SRC}/trng.c
    ${RT582_DRV_SRC}/wdt.c
)

target_include_directories(app PRIVATE
    drivers/soc/rt582/rt582_driver/Inc
    drivers/soc/rt582/rt582_system/Include
    drivers/hosal/rt582_hosal/Inc
)

# ── RT582 UART driver ────────────────────────────────────────────────────────
if(CONFIG_UART_RT582)
    target_sources(app PRIVATE
        drivers/serial/uart_rt582.c
        drivers/hosal/rt582_hosal/Src/hosal_uart.c
    )
    # Include dirs already added in the platform block above
endif()
