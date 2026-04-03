if(CONFIG_OPENTHREAD_RT582)

# ── Paths ─────────────────────────────────────────────────────────────────
get_filename_component(RAFAEL_SDK
    "${CMAKE_SOURCE_DIR}/../Rafael-IoT-SDK-Internal" ABSOLUTE)

set(OT_PORT_INC   ${RAFAEL_SDK}/components/network/thread/openthread_port/include)
set(OT_PORT_UTILS ${RAFAEL_SDK}/components/network/thread/openthread_port/utils)
set(OT_ROOT       ${RAFAEL_SDK}/components/network/thread/openthread)
set(SDK_LIBS      ${RAFAEL_SDK}/build/lib)

set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt582)
set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt582)
set(RUCI_INC      ${RAFAEL_SDK}/components/network/ruci/include)
set(LMAC_INC      ${RAFAEL_SDK}/components/network/lmac15p4/Inc)
set(RT582_DRV_INC ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_driver/Inc)
set(RT582_SYS_INC ${RAFAEL_SDK}/components/platform/soc/rt582/rt582_system/Include)
set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Inc)
set(UTIL_INC      ${RAFAEL_SDK}/components/utility/utility/Inc)
set(LOG_INC       ${RAFAEL_SDK}/components/utility/log)
set(MBEDTLS_INC   ${OT_ROOT}/third_party/mbedtls/repo/include)

# ── Platform sources compiled directly into 'app' ─────────────────────────
# Adding to the 'app' CMake target puts the objects in app/libapp.a which
# Zephyr links with --whole-archive.  This ensures every platform function
# (otPlatLog, otPlatReset, FreeRTOS shims, etc.) is present in the final
# image even though the OT core libs reference them after --no-whole-archive.
target_sources(app PRIVATE
    # Zephyr-ported OpenThread platform
    platform/ot_zephyr.c
    platform/ot_alarm.c
    platform/ot_radio.c
    platform/ot_uart.c
    platform/ot_entropy.c
    platform/ot_settings.c
    platform/ot_misc.c
    platform/ot_logging.c
    platform/ot_memory.c
    platform/ot_diag.c
    platform/ot_crypto.c

    # FreeRTOS → Zephyr shim (satisfies liblmac15p4.a dependencies)
    platform/freertos_shim.c

    # hosal: Zephyr-native hosal_rf + remaining hosal functions from source
    # (replaces librt582_hosal.a)
    platform/hosal_rf_zephyr.c
    ${RAFAEL_SDK}/components/platform/hosal/rt582_hosal/Src/hosal_trng.c

    # rt569-rf: RF MCU driver sources (replaces librt569-rf.a)
    ${RT569_RF_DIR}/Src/rf_mcu.c
    ${RT569_RF_DIR}/Src/rf_mcu_ahb.c
    ${RT569_RF_DIR}/Src/rf_common_init.c

    # rt569-fw: RF MCU firmware loader (replaces librt569_fw.a)
    ${RT569_FW_DIR}/Src/rt569mp_fw.c
    ${RT569_FW_DIR}/Src/rt569mp_init.c

    # Rafael SDK log shim (vprint stub needed by liblog.a)
    platform/log_shim.c

    # NOTE: soft_source_match_table.c and mac_frame_gen.c are NOT compiled here
    # because they are already included in libopenthread-platform-utils-static.a
    # (linked below with --whole-archive).  Compiling them again would cause
    # multiple-definition errors and waste RAM.

    # AES, SHA-256, EC-JPAKE from OT's vendored mbedTLS.
    # The SDK's libmbedcrypto.a was built without these; the hardware-accelerated
    # *_alt.c objects in libopenthread_port.a are FreeRTOS-dependent, so we
    # compile the portable software versions from OT's embedded mbedTLS source.
    ${OT_ROOT}/third_party/mbedtls/repo/library/aes.c
    ${OT_ROOT}/third_party/mbedtls/repo/library/sha256.c
    ${OT_ROOT}/third_party/mbedtls/repo/library/ecjpake.c
)

# mac_frame.cpp — use the pre-compiled object from libopenthread_port.a.
# Compiling from source produces a uint32_t ABI mismatch (unsigned int vs
# unsigned long) with the SDK's libopenthread-ftd.a, causing undefined
# references to ot::Mac::Frame::SetFrameCounter(unsigned int).  The object
# in libopenthread_port.a was built with matching flags and has no FreeRTOS
# or other problematic external dependencies.
set(MAC_FRAME_OBJ_DIR ${CMAKE_BINARY_DIR}/extracted_objs)
file(MAKE_DIRECTORY ${MAC_FRAME_OBJ_DIR})
set(MAC_FRAME_OBJ ${MAC_FRAME_OBJ_DIR}/mac_frame.cpp.obj)
if(NOT EXISTS ${MAC_FRAME_OBJ})
    execute_process(
        COMMAND ${CMAKE_AR} -x ${SDK_LIBS}/libopenthread_port.a mac_frame.cpp.obj
        WORKING_DIRECTORY ${MAC_FRAME_OBJ_DIR}
        RESULT_VARIABLE ar_rc
    )
    if(ar_rc)
        message(FATAL_ERROR "Failed to extract mac_frame.cpp.obj from libopenthread_port.a")
    endif()
endif()
target_sources(app PRIVATE ${MAC_FRAME_OBJ})

# ── Include directories ────────────────────────────────────────────────────
target_include_directories(app PRIVATE
    platform                          # openthread_port.h + config header
    ${OT_PORT_INC}                    # code_utils.h, mac_frame.h, etc.
    ${OT_ROOT}/include                # openthread/*.h  public API
    ${OT_ROOT}/src                    # top-level src
    ${OT_ROOT}/src/core               # core/common/logging.hpp
    ${OT_ROOT}/src/include            # common/code_utils.hpp
    ${OT_ROOT}/src/lib/spinel         # spinel headers
    ${MBEDTLS_INC}                    # mbedtls/*.h for ecjpake.c
    ${LMAC_INC}                       # lmac15p4.h
    ${RT582_DRV_INC}                  # mcu.h, flashctl.h, timer.h, wdt.h, trng.h
    ${RT582_SYS_INC}                  # system_mcu.h
    ${HOSAL_INC}                      # hosal_trng.h, hosal_uart.h
    ${UTIL_INC}                       # util_list.h, util_string.h
    ${LOG_INC}                        # log.h (for log_set global variable)
    ${RT569_RF_DIR}/include             # rf_mcu.h, rf_common_init.h, etc.
    ${RT569_FW_DIR}/include             # rt569mp_init.h, prg_*_fw.h
    ${RUCI_INC}                         # ruci.h (needed by rf_mcu_ahb.c)
    platform/freertos                   # stub FreeRTOS.h + task.h for log.h macros
)

# ── Global include dirs (so main.c can find OT headers) ───────────────────
zephyr_include_directories(
    ${OT_ROOT}/include                # <openthread/cli.h> etc.
    platform                          # openthread_port.h
)

# ── Compile definitions ────────────────────────────────────────────────────
target_compile_definitions(app PRIVATE
    OPENTHREAD_CONFIG_FILE="openthread-core-rt582-zephyr-config.h"
    CONFIG_RT582=1
    OPENTHREAD_FTD=1
    # CHIP_TYPE=2 (RT582) + CHIP_VERSION=RT58X_MPB are needed by SDK sources compiled
    # from source (rf_common_init.c, rf_mcu.c, etc.).  The pre-built libs were built
    # with CHIP_TYPE=RT582; without this define chip_define.h defaults CHIP_TYPE=RT581
    # making rf_common_init_by_fw() compile to 'return false'.
    CHIP_TYPE=2
    # CONFIG_RF_FW_INCLUDE_PCI=TRUE enables the RUCI/MAC firmware load path in
    # rf_common_init_by_fw().  Without it the function compiles to 'return false'
    # (guarded by #if (CONFIG_RF_FW_INCLUDE_PCI == TRUE) in rf_common_init.c).
    # This flag was implicit in the pre-built librt569-rf.a but must be explicit
    # when compiling rf_common_init.c / rt569mp_fw.c from source.
    "CONFIG_RF_FW_INCLUDE_PCI=TRUE"
    # CONFIG_FREERTOS=1 causes log.h to include <FreeRTOS.h>/<task.h>.
    # We provide stub headers in platform/freertos/ that forward-declare the
    # three FreeRTOS timing functions used in the SDK logging macros.
    CONFIG_FREERTOS=1
)
zephyr_compile_definitions(
    OPENTHREAD_CONFIG_FILE="openthread-core-rt582-zephyr-config.h"
    CONFIG_RT582=1
    OPENTHREAD_FTD=1
)

# ── Allow first-definition wins for soft_source_match_table symbols ────────
# ot_radio.c (in app/libapp.a) implements otPlatRadioAddSrcMatchShortEntry
# etc. directly.  libopenthread-platform-utils-static.a also contains
# soft_source_match_table.c.obj with the same symbols (pulled in because
# ot_radio.c calls utilsSoftSrcMatch* helpers from that same object).
# --allow-multiple-definition keeps the first definition (app) silently.
zephyr_ld_options(-Wl,--allow-multiple-definition)

# ── Pre-built SDK libraries ────────────────────────────────────────────────
# These are added via zephyr_link_libraries (INTERFACE of zephyr_interface).
# OT core libs need --whole-archive to retain all weak/template symbols.
# libtcplp-ftd.a provides TCPLite (tcp6.cpp in libopenthread-ftd.a).
# liblog.a provides log_printk and log_set (used by liblmac15p4.a).
zephyr_link_libraries(
    -Wl,--whole-archive
    ${SDK_LIBS}/libopenthread-cli-ftd.a
    ${SDK_LIBS}/libopenthread-ftd.a
    -Wl,--no-whole-archive
    # --start-group lets the linker make multiple passes so circular references
    # among the SDK libs (e.g. hosal_rf → librt569-rf → librt582_driver) resolve
    # regardless of ordering.
    -Wl,--start-group
    ${SDK_LIBS}/libopenthread-platform-utils-static.a
    ${SDK_LIBS}/libtcplp-ftd.a
    ${SDK_LIBS}/liblmac15p4.a
    # librt569-rf.a and librt569_fw.a replaced by source (rf_mcu.c, rf_mcu_ahb.c,
    # rf_common_init.c, rt569mp_fw.c, rt569mp_init.c compiled directly into app)
    ${SDK_LIBS}/libruci.a
    ${SDK_LIBS}/libmbedtls.a
    ${SDK_LIBS}/libmbedx509.a
    ${SDK_LIBS}/libmbedcrypto.a
    ${SDK_LIBS}/librt582_driver.a
    # librt582_hosal.a replaced by platform/hosal_rf_zephyr.c
    ${SDK_LIBS}/librt582_crypto.a
    ${SDK_LIBS}/librt582_system.a
    ${SDK_LIBS}/libutility.a
    ${SDK_LIBS}/liblog.a
    -Wl,--end-group
)

endif() # CONFIG_OPENTHREAD_RT582
