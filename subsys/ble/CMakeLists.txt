# subsys/ble/CMakeLists.txt — RT583 BLE platform sources
# Included by example CMakeLists.txt when CONFIG_BLE_RT583=y.
#
# Expects RAFAEL_SDK and RT583_PROJECT_DIR to be set by the caller.

set(RT569_RF_DIR  ${RAFAEL_SDK}/components/network/rt569-rf/rt583)
set(RT569_FW_DIR  ${RAFAEL_SDK}/components/network/rt569-fw/rt583)
set(RUCI_INC      ${RAFAEL_SDK}/components/network/ruci/Inc)
set(HOSAL_INC     ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Inc)
set(RT583_DRV_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_driver/Inc)
set(RT583_SYS_INC ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_system/Include)
set(UTIL_INC      ${RAFAEL_SDK}/components/utility/utility/Inc)
set(LOG_INC       ${RAFAEL_SDK}/components/utility/log)

# ── RF MCU + firmware + RUCI + utility ────────────────────────────────────────
target_sources(app PRIVATE
    # HOSAL RF (Zephyr port) + LPM
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src/hosal_rf.c
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src/hosal_lpm.c
    ${RAFAEL_SDK}/components/platform/soc/rt583/rt583_driver/Src/lpm.c

    # RF MCU
    ${RT569_RF_DIR}/Src/rf_mcu.c
    ${RT569_RF_DIR}/Src/rf_mcu_ahb.c
    ${RT569_RF_DIR}/Src/rf_common_init.c
    ${RT569_FW_DIR}/Src/rt569mp_fw.c
    ${RT569_FW_DIR}/Src/rt569mp_init.c

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

    # HCI driver (only when Zephyr BT host stack is enabled)
    $<$<BOOL:${CONFIG_BT}>:${RT583_PROJECT_DIR}/drivers/bluetooth/hci_rt583.c>
)

target_include_directories(app PRIVATE
    ${HOSAL_INC}
    ${RT583_DRV_INC}
    ${RT583_SYS_INC}
    ${UTIL_INC}
    ${LOG_INC}
    ${RT569_RF_DIR}/Inc
    ${RT569_FW_DIR}/Inc
    ${RUCI_INC}
)

target_compile_definitions(app PRIVATE
    CONFIG_RT583=1
    CHIP_TYPE=3
    "CONFIG_RF_FW_INCLUDE_BLE=TRUE"
    "CONFIG_RF_FW_INCLUDE_PCI=FALSE"
)

# Suppress warnings in vendored SDK sources we don't own
set_source_files_properties(
    ${RT569_RF_DIR}/Src/rf_common_init.c
    ${RAFAEL_SDK}/components/platform/hosal/rt583_hosal/Src/hosal_lpm.c
    PROPERTIES COMPILE_FLAGS "-Wno-unused-variable"
)
