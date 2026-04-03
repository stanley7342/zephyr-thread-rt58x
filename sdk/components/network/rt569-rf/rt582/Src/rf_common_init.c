/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * @file common_init.c
 * @author
 * @date
 * @brief Brief single line description use for indexing
 *
 * More detailed description can go here
 *
 *
 * @see http://
 */
/**************************************************************************************************
*    INCLUDES
*************************************************************************************************/
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "chip_define.h"
#include "rf_common_init.h"
#include "rf_mcu.h"
#include "rf_mcu_chip.h"
#include "ruci.h"
#include "system_mcu.h"
#if (RF_CAL_TYPE & (RF_CAL_PWR_ON_MODE | RF_CAL_MP_MODE))
#include "mp_sector.h"
#endif
#include "rf_tx_comp.h"
#include "sadc.h"
#if (RF_CAL_TYPE & RF_CAL_FT_MODE)
#include "ft_sector.h"
#endif

#if (RF_FW_SUPPORT_BANKING)
/* Add prg_ble_conn_mpb_fw.h & prg_ble_non_conn_mpb_fw.h manually since it's not formal feature*/
#include "prg_ble_conn_mpb_fw.h"
#include "prg_ble_non_conn_mpb_fw.h"
#endif

/**************************************************************************************************
*    GLOBAL PARAMETERS
*************************************************************************************************/
#if (RF_MCU_CHIP_BASE == BASE_RAM_TYPE)
#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE)
extern const uint32_t firmware_size_ruci;
extern const uint8_t firmware_program_ruci[];
#endif
#if (CONFIG_RF_FW_INCLUDE_BLE == TRUE)
extern const uint32_t firmware_size_ble;
extern const uint8_t firmware_program_ble[];
#endif
#if (CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)
extern const uint32_t firmware_size_multi;
extern const uint8_t firmware_program_multi[];
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
extern const uint32_t const_size_multi;
extern const uint8_t firmware_const_multi[];
#else
//#warning "CONFIG_RF_MCU_CONST_LOAD_SUPPORTED should be enabled when (CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)"
#endif
#endif
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
extern const uint32_t firmware_size_rfk;
extern const uint8_t firmware_program_rfk[];
#endif
#endif
#if (CONFIG_RF_FW_INCLUDE_MAC_ACC == TRUE)
extern const uint32_t firmware_size_mac_acc;
extern const uint8_t firmware_program_mac_acc[];
#endif
#if (CONFIG_RF_FW_INCLUDE_HYBRID_INST == TRUE)
extern const uint32_t firmware_size_hyb_inst;
extern const uint8_t firmware_program_hyb_inst[];
#endif
#if (CONFIG_RF_FW_INCLUDE_INTERNAL_TEST == TRUE)
extern const uint32_t firmware_size_it;
extern const uint8_t firmware_program_it[];
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
extern const uint32_t const_size_it;
extern const uint8_t firmware_const_it[];
#endif

#endif
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
mp_cal_rf_band_support_t rf_band_info;
#if (RF_CAL_TYPE & (RF_CAL_PWR_ON_MODE | RF_CAL_MP_MODE))
mp_cal_rf_trim_t rf_cal_info[RF_BAND_MAX];
#endif
#elif ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                \
       && (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_B))
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
mp_cal_rf_band_support_t rf_band_info;
#endif
#endif
#if (RF_MCU_PATCH_SUPPORTED)
#if ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                  \
     && (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_A))
#include "prg_multi_m0_patch.h"
#elif ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                \
       && (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_B))
#if (RF_MCU_FW_TARGET == RF_MCU_FW_TARGET_MAC)
#include "prg_pmac_m2_patch.h"
#elif (RF_MCU_FW_TARGET == RF_MCU_FW_TARGET_BLE)
#include "prg_ble_m2_patch.h"
#endif
#endif
#endif
#if (RF_FW_SUPPORT_BANKING)
uint8_t* ble_fw_conn_bank;
uint32_t ble_fw_conn_bank_size = 0;
uint8_t* ble_fw_non_conn_bank;
uint32_t ble_fw_non_conn_bank_size = 0;
#endif
#if (RF_CAL_TYPE & RF_CAL_FT_MODE)
#if ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                  \
     && (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_B))
#define EEPROM_DEV_ADDR    (0x50)
#define EEPROM_PAGE_SIZE   (8)
#define EEPROM_MAX_SIZE    (256)
#define EEPROM_WRITE_CYCLE (5) // Unit: ms

#define EEPROM_GPIO_SCL (20) // GPIO 22 is used to be GPIO interrupt for RF MCU
#define EEPROM_GPIO_SDA (21)
#endif

ft_sector_t g_rf_ft_info;
#endif

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/
RF_MCU_RX_CMDQ_ERROR rf_common_event_get(uint8_t* packet_length,
                                         uint8_t* event_address) {
    RF_MCU_RX_CMDQ_ERROR rx_confirm_error = RF_MCU_RX_CMDQ_ERR_INIT;
    uint8_t state = 0;
    do {
        state = (uint8_t)RfMcu_McuStateRead();
        state = state & RF_MCU_STATE_EVENT_DONE;
    } while (RF_MCU_STATE_EVENT_DONE != state);
    RfMcu_HostCmdSet(RF_MCU_STATE_EVENT_DONE);
    state = 0;
    do {
        state = (uint8_t)RfMcu_McuStateRead();
        state = state & RF_MCU_STATE_EVENT_DONE;
    } while (0 != state);

    (*packet_length) = RfMcu_EvtQueueRead(event_address, &rx_confirm_error);

    return rx_confirm_error;
}

void rf_common_cmd_send(uint8_t* cmd_address, uint8_t cmd_length) {
    while (RF_MCU_TX_CMDQ_SET_SUCCESS
           != RfMcu_CmdQueueSend(cmd_address, cmd_length)) {
        // printf("[E] command queue is FULL\n");
    }
}

bool ruci_ver_check(void) {
    ruci_para_get_fw_ver_t sGetRfbVerCmd = {0};
    ruci_para_get_fw_ver_event_t sGetRfbVerEvent = {0};
    ruci_para_cmn_cnf_event_t sCmnCnfEvent = {0};
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;
    uint8_t get_rfb_ver_event_len = 0;
    RF_MCU_RX_CMDQ_ERROR get_rfb_ver_event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    SET_RUCI_PARA_GET_FW_VER(&sGetRfbVerCmd);

    RUCI_ENDIAN_CONVERT((uint8_t*)&sGetRfbVerCmd, RUCI_GET_FW_VER);

    enter_critical_section();
    rf_common_cmd_send((uint8_t*)&sGetRfbVerCmd, RUCI_LEN_GET_FW_VER);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
    get_rfb_ver_event_status = rf_common_event_get(&get_rfb_ver_event_len,
                                                   (uint8_t*)&sGetRfbVerEvent);
    leave_critical_section();

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_GET_FW_VER)
        || (sCmnCnfEvent.status != 0)) {
        return false;
    }

    RUCI_ENDIAN_CONVERT((uint8_t*)&sGetRfbVerEvent, RUCI_GET_FW_VER_EVENT);
    if ((get_rfb_ver_event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sGetRfbVerEvent.sub_header != RUCI_CODE_GET_FW_VER_EVENT)) {
        return false;
    }
    if (RUCI_VERSION != sGetRfbVerEvent.ruci_fw_ver) {
        return false;
    }
    return true;
}

bool rf_common_pta_default_set(uint8_t enable, uint8_t inverse) {
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    uint16_t int_enable;
    ruci_para_set_pta_default_t sSetPtaDefault;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    /* Store interrupt setting and disable all interrupt */
    int_enable = RfMcu_InterruptEnGet();
    RfMcu_InterruptEnSet(0x0000);

    /* Update tx power compensation setting to lower layer HW */
    SET_RUCI_PARA_SET_PTA_DEFAULT(&sSetPtaDefault, enable, inverse);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sSetPtaDefault, RUCI_SET_PTA_DEFAULT);

    enter_critical_section();
    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sSetPtaDefault, RUCI_LEN_SET_PTA_DEFAULT);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
    leave_critical_section();

    /* Enable interrupt */
    RfMcu_InterruptClear(0xFF);
    RfMcu_InterruptEnSet(int_enable);

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_SET_PTA_DEFAULT)
        || (sCmnCnfEvent.status != 0)) {
        return false;
    }
#endif
    return true;
}

bool rf_common_pmu_operation_mode(pmu_mode_cfg_t pmu_mode) {
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    uint16_t int_enable;
    ruci_para_set_pmu_op_mode_t sSetPmuOpMode;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    /* Store interrupt setting and disable all interrupt */
    int_enable = RfMcu_InterruptEnGet();
    RfMcu_InterruptEnSet(0x0000);

    /* Update tx power compensation setting to lower layer HW */
    SET_RUCI_PARA_SET_PMU_OP_MODE(&sSetPmuOpMode, pmu_mode);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sSetPmuOpMode, RUCI_SET_PMU_OP_MODE);

    enter_critical_section();
    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sSetPmuOpMode, RUCI_LEN_SET_PMU_OP_MODE);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
    leave_critical_section();

    /* Enable interrupt */
    RfMcu_InterruptClear(0xFF);
    RfMcu_InterruptEnSet(int_enable);

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_SET_PMU_OP_MODE)
        || (sCmnCnfEvent.status != 0)) {
        return false;
    }
#endif

    return true;
}

#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
void rf_common_cal_isr_hdlr(uint8_t interrupt_status) {
    RfMcu_InterruptClear(interrupt_status);
}

bool rf_common_cal_enable(RF_BAND band_idx, mp_cal_rf_trim_t* p_rf_cal_info) {
    ruci_para_initiate_ble_t sBleInitCmd;
    ruci_para_initiate_fsk_t sFskInitCmd;
    ruci_para_set_calibration_enable_t sRfCalCmd;
    ruci_para_set_calibration_enable_event_t sRfCalCmdEvent;
    ruci_para_cnf_event_t sCnfEvent;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;
    uint8_t set_calibration_enable_event_len = 0;
    RF_MCU_RX_CMDQ_ERROR set_calibration_enable_event_status =
        RF_MCU_RX_CMDQ_ERR_INIT;

    /* Send HW initialization command */
#if (0)
    if (band_idx == RF_BAND_2P4G)
#else
    if (1) /* Forced RF calibration at 2.4GHz RF band */
#endif
    {
        SET_RUCI_PARA_INITIATE_BLE(&sBleInitCmd);
        RUCI_ENDIAN_CONVERT((uint8_t*)&sBleInitCmd, RUCI_INITIATE_BLE);

        enter_critical_section();
        event_len = 0;
        rf_common_cmd_send((uint8_t*)&sBleInitCmd, RUCI_LEN_INITIATE_BLE);
        event_status = rf_common_event_get(&event_len, (uint8_t*)&sCnfEvent);
        leave_critical_section();

        RUCI_ENDIAN_CONVERT((uint8_t*)&sCnfEvent, RUCI_CNF_EVENT);
        if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
            || (sCnfEvent.pci_cmd_subheader != RUCI_CODE_INITIATE_BLE)
            || (sCnfEvent.status != 0)) {
            return false;
        }
    } else {
        SET_RUCI_PARA_INITIATE_FSK(&sFskInitCmd, 0);
        RUCI_ENDIAN_CONVERT((uint8_t*)&sFskInitCmd, RUCI_INITIATE_FSK);

        enter_critical_section();
        event_len = 0;
        rf_common_cmd_send((uint8_t*)&sFskInitCmd, RUCI_LEN_INITIATE_FSK);
        event_status = rf_common_event_get(&event_len, (uint8_t*)&sCnfEvent);
        leave_critical_section();

        RUCI_ENDIAN_CONVERT((uint8_t*)&sCnfEvent, RUCI_CNF_EVENT);
        if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
            || (sCnfEvent.pci_cmd_subheader != RUCI_CODE_INITIATE_FSK)
            || (sCnfEvent.status != 0)) {
            return false;
        }
    }

    /* Send Calibration command */
    SET_RUCI_PARA_SET_CALIBRATION_ENABLE(&sRfCalCmd, 0);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sRfCalCmd, RUCI_SET_CALIBRATION_ENABLE);

    enter_critical_section();
    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sRfCalCmd, RUCI_LEN_SET_CALIBRATION_ENABLE);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
    set_calibration_enable_event_status = rf_common_event_get(
        &set_calibration_enable_event_len, (uint8_t*)&sRfCalCmdEvent);
    leave_critical_section();

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_SET_CALIBRATION_ENABLE)
        || (sCmnCnfEvent.status != 0)) {
        return false;
    }

    RUCI_ENDIAN_CONVERT((uint8_t*)&sRfCalCmdEvent,
                        RUCI_SET_CALIBRATION_ENABLE_EVENT);
    if ((set_calibration_enable_event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sRfCalCmdEvent.sub_header
            != RUCI_CODE_SET_CALIBRATION_ENABLE_EVENT)) {
        return false;
    }

    /* Update calibration results */
    p_rf_cal_info->cal_cfg = sRfCalCmdEvent.status;
    p_rf_cal_info->rx_filter_cap = sRfCalCmdEvent.rx_filter;
    p_rf_cal_info->tx_dc_offset_i = sRfCalCmdEvent.tx_lo[0];
    p_rf_cal_info->tx_dc_offset_q = sRfCalCmdEvent.tx_lo[1];
    p_rf_cal_info->tx_iqc_a = sRfCalCmdEvent.tx_sb[0];
    p_rf_cal_info->tx_iqc_b = sRfCalCmdEvent.tx_sb[1];
    p_rf_cal_info->tx_iqc_c = sRfCalCmdEvent.tx_sb[2];

    return true;
}
#endif

void rf_common_update_band_info() {
    /* Read RF band info from MP sector */
    if (mpcalrftrimread(MP_ID_RF_BAND_SUPPORT, MP_CNT_RF_BAND_SUPPORT,
                        (uint8_t*)(&rf_band_info))
        != STATUS_SUCCESS) {
        // Force to 2.4GHz RF if MP sector doesn't exist
        rf_band_info.rf_band = RF_BAND_SUPP(RF_BAND_2P4G);
    } else if ((rf_band_info.flag != 1) && (rf_band_info.flag != 2)) {
        // Force to 2.4GHz RF if flag isn't tool k and sw k
        rf_band_info.rf_band = RF_BAND_SUPP(RF_BAND_2P4G);
    }
}

bool rf_common_cal_init() {
#if (RF_CAL_TYPE & (RF_CAL_PWR_ON_MODE | RF_CAL_MP_MODE))
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
    COMM_SUBSYSTEM_ISR_CONFIG isr_config;
    bool is_start_up, is_fw_loaded;
    uint32_t regValue;
#endif
    pmu_mode_cfg_t backup_pmu_mode;
    mp_cal_rf_trim_t* p_rf_cal_info;
    RF_BAND band_idx;
    bool retStatus;
    uint32_t mp_id_map[RF_BAND_MAX] = {MP_ID_RFTRIM_2P4G, MP_ID_RFTRIM_SUBG0,
                                       MP_ID_RFTRIM_SUBG1, MP_ID_RFTRIM_SUBG2,
                                       MP_ID_RFTRIM_SUBG3};

    /* Initial setting */
    retStatus = true;
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
    is_start_up = false;
    is_fw_loaded = false;
#endif

    /* Check if it's start-up or not */
    // Check retention register. If bit 0 = 0 or bit 1 = 0, it means the calibration has been executed at start-up.
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
    sys_get_retention_reg(7, &regValue);
    if (((regValue & 0x00000003) != 0x00000003)) {
        is_start_up = true;
    }
#endif

    backup_pmu_mode = GetPmuMode();
    /* Set PMU state to LDO mode */
    SystemPmuSetMode(PMU_MODE_LDO);

    /* RF calibration loop */
    for (band_idx = 0; band_idx < RF_BAND_MAX; band_idx++) {
        /* By RF band if it's not supported */
        if (!(rf_band_info.rf_band & RF_BAND_SUPP(band_idx))) {
            continue;
        }

        /* Read from MP sector */
        p_rf_cal_info = &rf_cal_info[band_idx];
        if (mpcalrftrimread(mp_id_map[band_idx], MP_CNT_RFTRIM,
                            (uint8_t*)(p_rf_cal_info))
            != STATUS_SUCCESS) {
            // Force to AON mode if MP sector doesn't exist
            p_rf_cal_info->mode = RF_CAL_AON;
        } else if ((p_rf_cal_info->flag == 0) || (p_rf_cal_info->flag > 2)) {
            // Bypass mode
            continue;
        } else if ((p_rf_cal_info->mode != RF_CAL_MP)
                   && (p_rf_cal_info->mode != RF_CAL_STARTUP)
                   && (p_rf_cal_info->mode != RF_CAL_AON)) {
            // Force to AON mode if flag or calibration mode is out of range
            p_rf_cal_info->mode = RF_CAL_AON;
        }

#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
        /* Bypass calibration if it's in MP calibration mode */
        if (p_rf_cal_info->mode == RF_CAL_MP) {
            continue;
        }

        /* Bypass calibration if it's not (AON calibration mode) or (start-up) or (empty calibration default setting) */
        if (!((is_start_up) || (p_rf_cal_info->mode == RF_CAL_AON)
              || ((p_rf_cal_info->cal_cfg == 0xFFFF)
                  && (p_rf_cal_info->rx_filter_cap == 0xFFFF)
                  && (p_rf_cal_info->tx_dc_offset_i == 0xFFFF)
                  && (p_rf_cal_info->tx_dc_offset_q == 0xFFFF)
                  && (p_rf_cal_info->tx_iqc_a == 0xFFFF)
                  && (p_rf_cal_info->tx_iqc_b == 0xFFFF)
                  && (p_rf_cal_info->tx_iqc_c == 0xFFFF)))) {
            continue;
        }

        /* Download RF calibration FW */
        if (!is_fw_loaded) {
            isr_config.commsubsystem_isr = rf_common_cal_isr_hdlr;
            isr_config.content = 0;
            if (RfMcu_SysInit(true, firmware_program_rfk, firmware_size_rfk,
                              isr_config, RF_MCU_INIT_NO_ERROR)
                != RF_MCU_INIT_NO_ERROR) {
                retStatus = false;
                break;
            }
            is_fw_loaded = true;
        }

        /* RF calibration */
        if (rf_common_cal_enable(band_idx, p_rf_cal_info) == false) {
            retStatus = false;
            break;
        }

        /* Store calibration results if RF calibration is in STARTUP mode */
        if (p_rf_cal_info->mode == RF_CAL_STARTUP) {
            MpCalRftrimWrite(mp_id_map[band_idx], p_rf_cal_info);
        }
#endif
    }

    /* Update retention register */
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
    if (retStatus == true) {
        sys_set_retention_reg(7,
                              regValue | 0x00000003); // Raise bit 0 and bit 1
    }
#endif

    /* Reset PMU state */
    SystemPmuSetMode(backup_pmu_mode);

    return retStatus;
#else
    return true;
#endif
}

bool rf_common_cal_set() {
#if (RF_CAL_TYPE & (RF_CAL_PWR_ON_MODE | RF_CAL_MP_MODE))
    mp_cal_rf_trim_t* p_rf_cal_info;
    uint16_t int_enable;
    ruci_para_set_calibration_setting_t sRfCalSettingCmd;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;
    RF_BAND band_idx;
    bool retStatus = true;
    // uint32_t                            mp_id_map[RF_BAND_MAX] = {MP_ID_RFTRIM_2P4G, MP_ID_RFTRIM_SUBG0, MP_ID_RFTRIM_SUBG1, MP_ID_RFTRIM_SUBG2};

    /* Using polling mode. Store interrupt setting and disable all interrupt */
    int_enable = RfMcu_InterruptEnGet();
    RfMcu_InterruptEnSet(0x0000);

    /* Send calibration setting */
    for (band_idx = 0; band_idx < RF_BAND_MAX; band_idx++) {
        /* By RF band if it's not supported */
        if (!(rf_band_info.rf_band & RF_BAND_SUPP(band_idx))) {
            continue;
        }

        /* Read setting from MP sector */
        p_rf_cal_info = &rf_cal_info[band_idx];
        if ((p_rf_cal_info->flag == 0) || (p_rf_cal_info->flag > 2)) {
            // Bypass mode
            continue;
        }
#if (RF_CAL_TYPE == RF_CAL_MP_MODE)
        else if (p_rf_cal_info->mode != RF_CAL_MP) {
            // Only support MP RF calibration
            continue;
        }
#endif

        /* Update calibration setting to lower layer HW */
        SET_RUCI_PARA_SET_CALIBRATION_SETTING(
            &sRfCalSettingCmd, band_idx, (uint8_t)(p_rf_cal_info->cal_cfg),
            (uint8_t)(p_rf_cal_info->rx_filter_cap),
            (uint8_t)(p_rf_cal_info->tx_dc_offset_i),
            (uint8_t)(p_rf_cal_info->tx_dc_offset_q),
            (uint8_t)(p_rf_cal_info->tx_iqc_a),
            (uint8_t)(p_rf_cal_info->tx_iqc_b),
            (uint8_t)(p_rf_cal_info->tx_iqc_c), 0, 0, 0, 0);
        RUCI_ENDIAN_CONVERT((uint8_t*)&sRfCalSettingCmd,
                            RUCI_SET_CALIBRATION_SETTING);

        enter_critical_section();
        event_len = 0;
        rf_common_cmd_send((uint8_t*)&sRfCalSettingCmd,
                           RUCI_LEN_SET_CALIBRATION_SETTING);
        event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
        leave_critical_section();

        RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
        if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
            || (sCmnCnfEvent.cmn_cmd_subheader
                != RUCI_CODE_SET_CALIBRATION_SETTING)
            || (sCmnCnfEvent.status != 0)) {
            retStatus = false;
            break;
        }
    }

    /* Enable interrupt */
    RfMcu_InterruptClear(0xFF);
    RfMcu_InterruptEnSet(int_enable);

    return retStatus;
#else
    return true;
#endif
}

bool rf_common_tx_pwr_set() {
    mp_tx_power_trim_t sTx_power_trim;
    mp_tx_power_trim_2_t sTx_power_trim_2;
    mp_tx_power_trim_3_rssi_trim_2_t sTx_power_trim_3;
    uint16_t int_enable;
    ruci_para_set_tx_power_t sSetTxPower;
    ruci_para_set_tx_power_oqpsk_t sSetTxPower_oqpsk;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;
    RF_BAND band_idx;
    uint8_t gain_idx;
    bool retStatus = true;

    /* Read TX power setting from MP sector */
    if (mpcalrftrimread(MP_ID_TX_POWER_TRIM, MP_CNT_TX_POWER_TRIM,
                        (uint8_t*)(&sTx_power_trim))
        != STATUS_SUCCESS) {
        // No MP sector. Bypass procedure.
        return true;
    }
    if ((sTx_power_trim.flag != 1) && (sTx_power_trim.flag != 2)) {
        // Bypass mode
        return true;
    }

    if (mpcalrftrimread(MP_ID_TX_POWER_TRIM_3_RSSI_TRIM_2,
                        MP_CNT_TX_POWER_TRIM_3_RSSI_TRIM_2,
                        (uint8_t*)(&sTx_power_trim_3))
        != STATUS_SUCCESS) {
        sTx_power_trim_3.flag = 0;
    }

    /* Store interrupt setting and disable all interrupt */
    int_enable = RfMcu_InterruptEnGet();
    RfMcu_InterruptEnSet(0x0000);

    for (band_idx = 0; band_idx < RF_BAND_MAX; band_idx++) {
        /* By RF band if it's not supported */
        if (!((rf_band_info.rf_band & RF_BAND_SUPP(band_idx))
              && (sTx_power_trim.mode & RF_BAND_SUPP(band_idx)))) {
            continue;
        }

        if (band_idx < RF_BAND_SUB1G3) {
            gain_idx = *(&(sTx_power_trim.tx_gain_idx_2g_fsk) + band_idx);
        } else {
            if ((sTx_power_trim_3.flag != 1) && (sTx_power_trim_3.flag != 2)) {
                continue;
            }

            gain_idx = *(&(sTx_power_trim_3.tx_gain_idx_subg3_fsk)
                         + ((band_idx - RF_BAND_SUB1G3) * 2));
        }

        /* Update tx power setting to lower layer HW */
        SET_RUCI_PARA_SET_TX_POWER(&sSetTxPower, band_idx, gain_idx);
        RUCI_ENDIAN_CONVERT((uint8_t*)&sSetTxPower, RUCI_SET_TX_POWER);

        enter_critical_section();
        event_len = 0;
        rf_common_cmd_send((uint8_t*)&sSetTxPower, RUCI_LEN_SET_TX_POWER);
        event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
        leave_critical_section();

        RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
        if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
            || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_SET_TX_POWER)
            || (sCmnCnfEvent.status != 0)) {
            retStatus = false;
            break;
        }
    }
#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE                                          \
     || CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)
    if (mpcalrftrimread(MP_ID_TX_POWER_TRIM_2, MP_CNT_TX_POWER_TRIM_2,
                        (uint8_t*)(&sTx_power_trim_2))
        == STATUS_SUCCESS) {
        if ((sTx_power_trim_2.flag == 1) || (sTx_power_trim_2.flag == 2)) {
            for (band_idx = 0; band_idx < RF_BAND_MAX; band_idx++) {
                /* By RF band if it's not supported */
                if (!((rf_band_info.rf_band & RF_BAND_SUPP(band_idx))
                      && (sTx_power_trim.mode & RF_BAND_SUPP(band_idx)))) {
                    continue;
                }

                if (band_idx < RF_BAND_SUB1G3) {
                    gain_idx = *(&(sTx_power_trim_2.tx_gain_idx_2g_oqpsk)
                                 + band_idx);
                } else {
                    if ((sTx_power_trim_3.flag != 1)
                        && (sTx_power_trim_3.flag != 2)) {
                        continue;
                    }

                    gain_idx = *(&(sTx_power_trim_3.tx_gain_idx_subg3_oqpsk)
                                 + ((band_idx - RF_BAND_SUB1G3) * 2));
                }

                /* Update tx power setting to lower layer HW */
                SET_RUCI_PARA_SET_TX_POWER_OQPSK(&sSetTxPower_oqpsk, band_idx,
                                                 gain_idx);
                RUCI_ENDIAN_CONVERT((uint8_t*)&sSetTxPower_oqpsk,
                                    RUCI_SET_TX_POWER_OQPSK);

                enter_critical_section();
                event_len = 0;
                rf_common_cmd_send((uint8_t*)&sSetTxPower_oqpsk,
                                   RUCI_LEN_SET_TX_POWER_OQPSK);
                event_status = rf_common_event_get(&event_len,
                                                   (uint8_t*)&sCmnCnfEvent);
                leave_critical_section();

                RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent,
                                    RUCI_CMN_CNF_EVENT);
                if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
                    || (sCmnCnfEvent.cmn_cmd_subheader
                        != RUCI_CODE_SET_TX_POWER_OQPSK)
                    || (sCmnCnfEvent.status != 0)) {
                    retStatus = false;
                    break;
                }
            }
        }
    }
#endif
    /* Enable interrupt */
    RfMcu_InterruptClear(0xFF);
    RfMcu_InterruptEnSet(int_enable);

    return retStatus;
}

bool rf_common_rssi_offset_set() {
    mp_rssi_trim_t sRssi_trim;
    mp_tx_power_trim_3_rssi_trim_2_t sRssi_trim_2;
    uint16_t int_enable;
    ruci_para_set_rssi_offset_t sSetRssiOffset;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;
    RF_BAND band_idx;
    uint8_t rssi_offset;
    bool retStatus = true;

    /* Read RSSI setting from MP sector */
    if (mpcalrftrimread(MP_ID_RSSI_TRIM, MP_CNT_RSSI_TRIM,
                        (uint8_t*)(&sRssi_trim))
        != STATUS_SUCCESS) {
        // No MP sector. Bypass procedure.
        return true;
    }
    if ((sRssi_trim.flag != 1) && (sRssi_trim.flag != 2)) {
        // Bypass mode
        return true;
    }

    if (mpcalrftrimread(MP_ID_TX_POWER_TRIM_3_RSSI_TRIM_2,
                        MP_CNT_TX_POWER_TRIM_3_RSSI_TRIM_2,
                        (uint8_t*)(&sRssi_trim_2))
        != STATUS_SUCCESS) {
        sRssi_trim_2.flag = 0;
    }

    /* Store interrupt setting and disable all interrupt */
    int_enable = RfMcu_InterruptEnGet();
    RfMcu_InterruptEnSet(0x0000);

    for (band_idx = 0; band_idx < RF_BAND_MAX; band_idx++) {
        /* By RF band if it's not supported */
        if (!((rf_band_info.rf_band & RF_BAND_SUPP(band_idx))
              && (sRssi_trim.mode & RF_BAND_SUPP(band_idx)))) {
            continue;
        }

        if (band_idx < RF_BAND_SUB1G3) {
            rssi_offset = *(&(sRssi_trim.rssi_trim_2g) + band_idx);
        } else {
            if ((sRssi_trim_2.flag != 1) && (sRssi_trim_2.flag != 2)) {
                continue;
            }

            rssi_offset = sRssi_trim_2.rssi_trim_subg3;
        }

        /* Update rssi offset to lower layer HW */
        SET_RUCI_PARA_SET_RSSI_OFFSET(&sSetRssiOffset, band_idx, rssi_offset);
        RUCI_ENDIAN_CONVERT((uint8_t*)&sSetRssiOffset, RUCI_SET_RSSI_OFFSET);

        enter_critical_section();
        event_len = 0;
        rf_common_cmd_send((uint8_t*)&sSetRssiOffset, RUCI_LEN_SET_RSSI_OFFSET);
        event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
        leave_critical_section();

        RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
        if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
            || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_SET_RSSI_OFFSET)
            || (sCmnCnfEvent.status != 0)) {
            retStatus = false;
            break;
        }
    }

    /* Enable interrupt */
    RfMcu_InterruptClear(0xFF);
    RfMcu_InterruptEnSet(int_enable);

    return retStatus;
}

#elif ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                \
       && (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_B))
bool rf_common_update_from_ft_info(void) {
#if (RF_CAL_TYPE & RF_CAL_FT_MODE)
    ft_sector_t* p_ft_info = &g_rf_ft_info;
    ruci_para_set_pmu_mpk_setting_t sRfPmuMpk;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len;
    uint16_t int_enable;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;
    uint32_t status;

    status = Ft_FtInfoGet(p_ft_info, EEPROM_DEV_ADDR, EEPROM_PAGE_SIZE,
                          EEPROM_MAX_SIZE, EEPROM_WRITE_CYCLE, EEPROM_GPIO_SCL,
                          EEPROM_GPIO_SDA, I2C_CLOCK_400K);

    if (status != STATUS_SUCCESS) {
        return true;
    }

    if (p_ft_info->status == 0) {
        return true;
    }

    /* Using polling mode. Store interrupt setting and disable all interrupt */
    int_enable = RfMcu_InterruptEnGet();
    RfMcu_InterruptDisableAll();

    /* Send RUCI command to update PMU setting */
    SET_RUCI_PARA_SET_PMU_MPK_SETTING(
        &sRfPmuMpk, p_ft_info->status, 0, p_ft_info->sldo.vosel_targ_0,
        p_ft_info->sldo.vosel_targ_1, p_ft_info->sldo.vosel_targ_2,
        p_ft_info->ldolv.vosel_targ_0, p_ft_info->dcdc.vosel_targ_1,
        p_ft_info->ldomv.vosel_targ_1, p_ft_info->dcdc.vosel_targ_0,
        p_ft_info->ldomv.vosel_targ_0, 0, 0);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sRfPmuMpk, RUCI_SET_PMU_MPK_SETTING);

    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sRfPmuMpk, RUCI_LEN_SET_PMU_MPK_SETTING);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);

    /* Enable interrupt */
    RfMcu_InterruptClear(COMM_SUBSYSTEM_INT_ALL_MASK);
    RfMcu_InterruptEnSet(int_enable);

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_SET_PMU_MPK_SETTING)
        || (sCmnCnfEvent.status != 0)) {
        return false;
    }

#endif
    return true;
}

#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
bool rf_common_cal_enable(RF_BAND band_idx, mp_cal_rf_trim_t* p_rf_cal_info) {
#if (RF_MCU_FW_TARGET == RF_MCU_FW_TARGET_MAC)
    ruci_para_initiate_ble_t sBleInitCmd;
    ruci_para_cnf_event_t sCnfEvent;
#endif
    ruci_para_set_calibration_enable_t sRfCalCmd;
    ruci_para_set_calibration_enable_event_t sRfCalCmdEvent;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint16_t int_enable;
    uint8_t event_len;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;
    uint8_t set_calibration_enable_event_len = 0;
    RF_MCU_RX_CMDQ_ERROR set_calibration_enable_event_status =
        RF_MCU_RX_CMDQ_ERR_INIT;
    bool status;

    /* Using polling mode. Store interrupt setting and disable all interrupt */
    int_enable = RfMcu_InterruptEnGet();
    RfMcu_InterruptDisableAll();

    /* Send HW initialization command */
#if (RF_MCU_FW_TARGET == RF_MCU_FW_TARGET_MAC)
    SET_RUCI_PARA_INITIATE_BLE(&sBleInitCmd);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sBleInitCmd, RUCI_INITIATE_BLE);

    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sBleInitCmd, RUCI_LEN_INITIATE_BLE);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCnfEvent);

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCnfEvent, RUCI_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCnfEvent.pci_cmd_subheader != RUCI_CODE_INITIATE_BLE)
        || (sCnfEvent.status != 0)) {
        status = false;

        goto DONE;
    }
#endif

    /* Send Calibration command */
    SET_RUCI_PARA_SET_CALIBRATION_ENABLE(&sRfCalCmd, 0);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sRfCalCmd, RUCI_SET_CALIBRATION_ENABLE);

    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sRfCalCmd, RUCI_LEN_SET_CALIBRATION_ENABLE);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
    set_calibration_enable_event_status = rf_common_event_get(
        &set_calibration_enable_event_len, (uint8_t*)&sRfCalCmdEvent);

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader != RUCI_CODE_SET_CALIBRATION_ENABLE)
        || (sCmnCnfEvent.status != 0)) {
        status = false;

        goto DONE;
    }

    RUCI_ENDIAN_CONVERT((uint8_t*)&sRfCalCmdEvent,
                        RUCI_SET_CALIBRATION_ENABLE_EVENT);
    if ((set_calibration_enable_event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sRfCalCmdEvent.sub_header
            != RUCI_CODE_SET_CALIBRATION_ENABLE_EVENT)) {
        status = false;

        goto DONE;
    }

    /* Update calibration results */
    if (p_rf_cal_info) {
        p_rf_cal_info->cal_cfg = sRfCalCmdEvent.status;
        p_rf_cal_info->rx_filter_cap = sRfCalCmdEvent.rx_filter;
        p_rf_cal_info->tx_dc_offset_i = sRfCalCmdEvent.tx_lo[0];
        p_rf_cal_info->tx_dc_offset_q = sRfCalCmdEvent.tx_lo[1];
        p_rf_cal_info->tx_iqc_a = sRfCalCmdEvent.tx_sb[0];
        p_rf_cal_info->tx_iqc_b = sRfCalCmdEvent.tx_sb[1];
        p_rf_cal_info->tx_iqc_c = sRfCalCmdEvent.tx_sb[2];
    }

    status = true;

DONE:
    /* Enable interrupt */
    RfMcu_InterruptClear(COMM_SUBSYSTEM_INT_ALL_MASK);
    RfMcu_InterruptEnSet(int_enable);

    return status;
}
#endif

void rf_common_update_band_info() {
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
    /* Force to 2.4GHz RF */
    rf_band_info.rf_band = RF_BAND_SUPP(RF_BAND_2P4G);
#endif
}

bool rf_common_cal_init() {
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
    uint32_t regValue;
    RF_BAND band_idx;
    bool retStatus;

    /* Initial setting */
    retStatus = true;

    /* Check if it's start-up or not */
    // Check retention register. If bit 0 = 0 or bit 1 = 0, it means the calibration has been executed at start-up.
    sys_get_retention_reg(7, &regValue);
    if (((regValue & 0x00000003) == 0x00000003)) {
        goto DONE;
    }

    /* RF calibration loop */
    for (band_idx = 0; band_idx < RF_BAND_MAX; band_idx++) {
        /* By RF band if it's not supported */
        if (!(rf_band_info.rf_band & RF_BAND_SUPP(band_idx))) {
            continue;
        }

        /* RF calibration */
        if (rf_common_cal_enable(band_idx, NULL) == false) {
            retStatus = false;
            break;
        }
    }

    /* Update retention register */
    if (retStatus == true) {
        sys_set_retention_reg(7,
                              regValue | 0x00000003); // Raise bit 0 and bit 1
    }

DONE:
    return retStatus;
#else
    return true;
#endif
}

#endif

/**************************************************************************************************
 *    GLOBAL FUNCTIONS
 *************************************************************************************************/
bool rf_common_tx_pwr_comp_set(int8_t offset, uint8_t poly_gain,
                               uint8_t pa_pw_pre, uint8_t modemType) {
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
#if (0) // Workaround for UART bridge and DTM project issue
    ruci_para_set_tx_power_compensation_t sSetTxPwrComp;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    /* Update tx power compensation setting to lower layer HW */
    SET_RUCI_PARA_SET_TX_POWER_COMPENSATION(&sSetTxPwrComp, (uint8_t)(offset),
                                            poly_gain, pa_pw_pre, modemType);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sSetTxPwrComp,
                        RUCI_SET_TX_POWER_COMPENSATION);

    enter_critical_section();
    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sSetTxPwrComp,
                       RUCI_LEN_SET_TX_POWER_COMPENSATION);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
    leave_critical_section();

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader
            != RUCI_CODE_SET_TX_POWER_COMPENSATION)
        || (sCmnCnfEvent.status != 0)) {
        return false;
    }

#else
    ruci_para_set_tx_power_compensation_t sSetTxPwrComp;

    /* Update tx power compensation setting to lower layer HW */
    SET_RUCI_PARA_SET_TX_POWER_COMPENSATION(&sSetTxPwrComp, (uint8_t)(offset),
                                            poly_gain, pa_pw_pre, modemType);
    RUCI_ENDIAN_CONVERT((uint8_t*)&sSetTxPwrComp,
                        RUCI_SET_TX_POWER_COMPENSATION);

    //enter_critical_section();
    rf_common_cmd_send((uint8_t*)&sSetTxPwrComp,
                       RUCI_LEN_SET_TX_POWER_COMPENSATION);
    //leave_critical_section();
#endif
#endif

    return true;
}

bool rf_common_tx_pwr_ch_comp_set(int8_t offset0, int8_t offset1,
                                  int8_t offset2, int8_t offset3,
                                  int8_t modemType) {
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    ruci_para_set_tx_power_channel_compensation_t sSetTxPwrChComp;
    ruci_para_cmn_cnf_event_t sCmnCnfEvent;
    uint8_t event_len = 0;
    RF_MCU_RX_CMDQ_ERROR event_status = RF_MCU_RX_CMDQ_ERR_INIT;

    /* Update tx power compensation setting to lower layer HW */
    SET_RUCI_PARA_SET_TX_POWER_CHANNEL_COMPENSATION(
        &sSetTxPwrChComp, (uint8_t)(offset0), (uint8_t)(offset1),
        (uint8_t)(offset2), (uint8_t)(offset3), (uint8_t)(modemType));
    RUCI_ENDIAN_CONVERT((uint8_t*)&sSetTxPwrChComp,
                        RUCI_SET_TX_POWER_CHANNEL_COMPENSATION);

    enter_critical_section();
    event_len = 0;
    rf_common_cmd_send((uint8_t*)&sSetTxPwrChComp,
                       RUCI_LEN_SET_TX_POWER_CHANNEL_COMPENSATION);
    event_status = rf_common_event_get(&event_len, (uint8_t*)&sCmnCnfEvent);
    leave_critical_section();

    RUCI_ENDIAN_CONVERT((uint8_t*)&sCmnCnfEvent, RUCI_CMN_CNF_EVENT);
    if ((event_status != RF_MCU_RX_CMDQ_GET_SUCCESS)
        || (sCmnCnfEvent.cmn_cmd_subheader
            != RUCI_CODE_SET_TX_POWER_CHANNEL_COMPENSATION)
        || (sCmnCnfEvent.status != 0)) {
        return false;
    }
#endif

    return true;
}

/* RF Common initialization with different FW */
bool rf_common_init_by_fw(RF_FW_LOAD_SELECT fw_select,
                          COMM_SUBSYSTEM_ISR_t isr_func) {

    const uint8_t* fw_load;
    uint32_t firmware_size;
    COMM_SUBSYSTEM_ISR_CONFIG isr_config;
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
    const uint8_t* fw_const = NULL;
    uint32_t const_size = 0;
#endif

#if (RF_MCU_CHIP_BASE != BASE_RAM_TYPE)
    UNUSED(fw_load);
    UNUSED(firmware_size);
#endif

#if (RF_FW_SUPPORT_BANKING)
    ble_fw_conn_bank = (uint8_t*)firmware_program_ble_conn;
    ble_fw_conn_bank_size = sizeof(firmware_program_ble_conn);

    ble_fw_non_conn_bank = (uint8_t*)firmware_program_ble_non_conn;
    ble_fw_non_conn_bank_size = sizeof(firmware_program_ble_non_conn);
#endif

#if (RF_MCU_CHIP_BASE == BASE_RAM_TYPE)
    if (fw_select == RF_FW_LOAD_SELECT_RUCI_CMD) {
#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE)
        /* Pure MAC FW */
        fw_load = firmware_program_ruci;
        firmware_size = firmware_size_ruci;
#else
        return false;
#endif
    } else if (fw_select == RF_FW_LOAD_SELECT_BLE_CONTROLLER) {
#if (CONFIG_RF_FW_INCLUDE_BLE == TRUE)
        /* Pure BLE FW */
        fw_load = firmware_program_ble;
        firmware_size = firmware_size_ble;
#else
        return false;
#endif
    } else if (fw_select == RF_FW_LOAD_SELECT_MULTI_PROTCOL_2P4G) {
#if (CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)
        /* Multi protocol FW for 2.4G */
        fw_load = firmware_program_multi;
        firmware_size = firmware_size_multi;
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
        fw_const = firmware_const_multi;
        const_size = const_size_multi;
#else
        fw_const = NULL;
        const_size = 0;
#endif
#endif
#else
        return false;
#endif
    }
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    else if (fw_select == RF_FW_LOAD_SELECT_RFK) {
#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
        /* RF calibration FW */
        fw_load = firmware_program_rfk;
        firmware_size = firmware_size_rfk;
#else
        return false;
#endif
    }
#endif
    else {
        return false;
    }
#endif

    if (fw_select == RF_FW_LOAD_SELECT_INTERNAL_TEST) {
#if (CONFIG_RF_FW_INCLUDE_INTERNAL_TEST == TRUE)
        /* Internal Test FW */
        fw_load = firmware_program_it;
        firmware_size = firmware_size_it;
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
        fw_const = firmware_const_it;
        const_size = const_size_it;
#endif
#else
        return false;
#endif
    }

    if (fw_select == RF_FW_LOAD_SELECT_MX_MAC_ACCELARATOR) {
#if ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                  \
     && (CONFIG_RF_FW_INCLUDE_MAC_ACC == TRUE))
        fw_load = firmware_program_mac_acc;
        firmware_size = firmware_size_mac_acc;
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
        fw_const = NULL;
        const_size = 0;
#endif
#else
        return false;
#endif
    }

    if (fw_select == RF_FW_LOAD_SELECT_MX_HYBRID_INSTRUCTION) {
#if ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                  \
     && (CONFIG_RF_FW_INCLUDE_HYBRID_INST == TRUE))
        fw_load = firmware_program_hyb_inst;
        firmware_size = firmware_size_hyb_inst;
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
        fw_const = NULL;
        const_size = 0;
#endif
#else
        return false;
#endif
    }

#if (RF_MCU_CHIP_BASE == BASE_RAM_TYPE)
    if (firmware_size == 0) {
        return false;
    }
#endif

    /* ISR initialization */
    isr_config.commsubsystem_isr = isr_func;
    isr_config.content = 0;

#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    /* Update band info. */
    rf_common_update_band_info();

    /* RF calibration */
    if (rf_common_cal_init() == false) {
        return false;
    }
#endif

    /* RF Chip initialization */
#if (RF_MCU_CHIP_BASE == BASE_RAM_TYPE)
#if (RF_MCU_PATCH_SUPPORTED)
    if (RfMcu_SysInitWithPatch(true, fw_load, firmware_size, firmware_patch,
                               sizeof(firmware_patch), isr_config,
                               RF_MCU_INIT_NO_ERROR)
        != RF_MCU_INIT_NO_ERROR) {
        return false;
    }
#elif (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
    if (RfMcu_SysInitWithConst(true, fw_load, firmware_size, fw_const,
                               const_size, isr_config, RF_MCU_INIT_NO_ERROR)
        != RF_MCU_INIT_NO_ERROR) {
        return false;
    }
#else
    if (RfMcu_SysInit(true, fw_load, firmware_size, isr_config,
                      RF_MCU_INIT_NO_ERROR)
        != RF_MCU_INIT_NO_ERROR) {
        return false;
    }
#endif
#else
#if (RF_MCU_PATCH_SUPPORTED)
    if (RfMcu_SysInitWithPatch(false, NULL, 0, firmware_patch,
                               sizeof(firmware_patch), isr_config,
                               RF_MCU_INIT_NO_ERROR)
        != RF_MCU_INIT_NO_ERROR) {
        return false;
    }
#elif (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
    if (RfMcu_SysInitWithConst(true, fw_load, firmware_size, fw_const,
                               const_size, isr_config, RF_MCU_INIT_NO_ERROR)
        != RF_MCU_INIT_NO_ERROR) {
        return false;
    }
#else
    if (RfMcu_SysInit(false, NULL, 0, isr_config, RF_MCU_INIT_NO_ERROR)
        != RF_MCU_INIT_NO_ERROR) {
        return false;
    }
#endif
#endif

#if 0
    /* Check RUCI version */
    if (ruci_ver_check() == false)
    {
        return false;
    }
#endif

#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    if (rf_common_pmu_operation_mode(GetPmuMode()) == false) {
        return false;
    }

    rf_common_pta_default_set(0, 0);

    /* Set calibration setting */
    if (rf_common_cal_set() == false) {
        return false;
    }

    /* Set TX power setting */
    if (rf_common_tx_pwr_set() == false) {
        return false;
    }

    /* Set RSSI setting */
    if (rf_common_rssi_offset_set() == false) {
        return false;
    }

    /* Enable TX power compensation */
#if (RF_TX_POWER_COMP)
    Sadc_Config_Enable(SADC_RES_12BIT, SADC_OVERSAMPLE_256,
                       Tx_Power_Sadc_Int_Callback_Handler);
    Sadc_Disable();
    Sadc_Compensation_Init(13);
    Tx_Power_Compensation_Init(10);
#endif
#elif ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)                                \
       && (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_B))
    /* Update PMU setting from FT info */
    if (rf_common_update_from_ft_info() == false) {
        return false;
    }

    /* Update band info. */
    rf_common_update_band_info();

    /* RF calibration */
    if (rf_common_cal_init() == false) {
        return false;
    }
#endif

    return true;
}

void rf_common_radio_reg_dump (void)
{
    uint32_t reg_val;
    uint16_t reg_addr;

    /* MAC */
    for (reg_addr = 0x100 ; reg_addr < 0x200 ; reg_addr += 4)
    {
        reg_val = RfMcu_RegGet(reg_addr);
        printf("Addr: 0x%04"PRIx16", Val: 0x%08"PRIx32" \r\n", reg_addr, reg_val);
    }

    /* BMU */
    for (reg_addr = 0x200 ; reg_addr < 0x300 ; reg_addr += 4)
    {
        reg_val = RfMcu_RegGet(reg_addr);
        printf("Addr: 0x%04"PRIx16", Val: 0x%08"PRIx32" \r\n", reg_addr, reg_val);
    }

    /* RF */
    for (reg_addr = 0x300 ; reg_addr < 0x400 ; reg_addr += 4)
    {
        reg_val = RfMcu_RegGet(reg_addr);
        printf("Addr: 0x%04"PRIx16", Val: 0x%08"PRIx32" \r\n", reg_addr, reg_val);
    }

    /* PMU */
    for (reg_addr = 0x400 ; reg_addr < 0x500 ; reg_addr += 4)
    {
        reg_val = RfMcu_RegGet(reg_addr);
        printf("Addr: 0x%04"PRIx16", Val: 0x%08"PRIx32" \r\n", reg_addr, reg_val);
    }

    /* FW */
    for (reg_addr = 0x4000 ; reg_addr < 0x4040 ; reg_addr += 4)
    {
        reg_val = RfMcu_RegGet(reg_addr);
        printf("Addr: 0x%04"PRIx16", Val: 0x%08"PRIx32" \r\n", reg_addr, reg_val);
    }
}
