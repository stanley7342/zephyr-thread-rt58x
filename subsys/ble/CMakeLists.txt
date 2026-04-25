# subsys/ble/CMakeLists.txt — Rafael (rt583/rt584) BLE platform sources
# Included by example CMakeLists.txt when CONFIG_BLE_RT583=y or CONFIG_BLE_RT584=y.
#
# Expects RAFAEL_SDK and RT583_PROJECT_DIR to be set by the caller.

set(RUCI_INC      ${RAFAEL_SDK}/components/network/ruci/Inc)
set(UTIL_INC      ${RAFAEL_SDK}/components/utility/utility/Inc)
set(LOG_INC       ${RAFAEL_SDK}/components/utility/log)

# SoC-specific paths.
if(CONFIG_BLE_RT584)
    set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt584)
    # rt584 RF / FW header dirs are lowercase `include/`; rt583 uses `Inc/`.
    set(RT569_RF_INC  ${RT569_RF_DIR}/include)
    set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt584)
    set(RT569_FW_INC  ${RT569_FW_DIR}/include)
    set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt584_hosal/Inc)
    set(RT_DRV_INC    ${RAFAEL_SDK}/components/platform/soc/rt584/rt584_driver/Inc)
    set(RT_SYS_INC    ${RAFAEL_SDK}/components/platform/soc/rt584/rt584_system/Include)
    set(HOSAL_SRC     ${RAFAEL_SDK}/components/platform/hosal/rt584_hosal/Src)
    set(SOC_DRV_SRC   ${RAFAEL_SDK}/components/platform/soc/rt584/rt584_driver/Src)
    set(RF_FW_SRCS
        ${RT569_FW_DIR}/Src/rt569s_fw.c
        ${RT569_FW_DIR}/Src/rt569s_init.c
    )
else()
    set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt583)
    set(RT569_RF_INC  ${RT569_RF_DIR}/Inc)
    set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt583)
    set(RT569_FW_INC  ${RT569_FW_DIR}/Inc)
    set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Inc)
    set(RT_DRV_INC    ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_driver/Inc)
    set(RT_SYS_INC    ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_system/Include)
    set(HOSAL_SRC     ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src)
    set(SOC_DRV_SRC   ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_driver/Src)
    set(RF_FW_SRCS
        ${RT569_FW_DIR}/Src/rt569mp_fw.c
        ${RT569_FW_DIR}/Src/rt569mp_init.c
    )
endif()

# ── RF MCU + firmware + RUCI + utility ────────────────────────────────────────
# hosal_rf.c is taken from rt583_hosal/Src — that path holds our hand-written
# Zephyr port (FreeRTOS → k_sem/k_msgq/k_thread). The rt584 vendor copy is
# the original FreeRTOS source and won't compile under Zephyr. The hosal_rf
# API surface matches between the two SoCs.
target_sources(app PRIVATE
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src/hosal_rf.c
    ${HOSAL_SRC}/hosal_lpm.c
    ${SOC_DRV_SRC}/lpm.c

    # RF MCU
    ${RT569_RF_DIR}/Src/rf_mcu.c
    ${RT569_RF_DIR}/Src/rf_mcu_ahb.c
    ${RT569_RF_DIR}/Src/rf_common_init.c
    ${RF_FW_SRCS}

    # RUCI (command protocol)
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_cmn_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_cmn_hal_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_cmn_sys_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_ble_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_common_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_pci_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_host_cmd.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_host_event.c
    ${RAFAEL_SDK}/components/network/ruci/Src/ruci_sf_host_sys_cmd.c

    # Utility
    ${RAFAEL_SDK}/components/utility/utility/Src/util_queue.c
    ${RAFAEL_SDK}/components/utility/utility/Src/util_string.c

    # Log
    ${RAFAEL_SDK}/components/utility/log/log.c
    ${RAFAEL_SDK}/components/network/thread/openthread_port/Src/log_shim.c

    # HCI driver (only when Zephyr BT host stack is enabled). Same source
    # for rt583 / rt584 — uses hosal_rf API, not SoC registers directly.
    $<$<BOOL:${CONFIG_BT}>:${RT583_PROJECT_DIR}/drivers/bluetooth/hci_rt583.c>
)

target_include_directories(app PRIVATE
    ${HOSAL_INC}
    ${RT_DRV_INC}
    ${RT_SYS_INC}
    ${UTIL_INC}
    ${LOG_INC}
    ${RT569_RF_INC}
    ${RT569_FW_INC}
    ${RUCI_INC}
)

# SoC variant define drives RF firmware blob selection in rt569{mp,s}_fw.c:
#   rt583 → CONFIG_RT583=1 + CHIP_TYPE=3
#   rt584 → CONFIG_RT584L=1 (or CONFIG_RT584H=1 for the H variant)
if(CONFIG_BLE_RT584)
    target_compile_definitions(app PRIVATE
        CONFIG_RT584L=1
        "CONFIG_RF_FW_INCLUDE_BLE=TRUE"
        "CONFIG_RF_FW_INCLUDE_PCI=FALSE"
    )
else()
    target_compile_definitions(app PRIVATE
        CONFIG_RT583=1
        CHIP_TYPE=3
        "CONFIG_RF_FW_INCLUDE_BLE=TRUE"
        "CONFIG_RF_FW_INCLUDE_PCI=FALSE"
    )
endif()

# Suppress warnings in vendored SDK sources we don't own
set_source_files_properties(
    ${RT569_RF_DIR}/Src/rf_common_init.c
    ${HOSAL_SRC}/hosal_lpm.c
    PROPERTIES COMPILE_FLAGS "-Wno-unused-variable"
)
