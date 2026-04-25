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
#include "ruci.h"
#include "rf_mcu.h"
#include "rf_mcu_chip.h"
#include "rf_common_init.h"
#include "rf_test.h"

/******************************************************************************
 *  External functions
 *****************************************************************************/
RF_MCU_RX_CMDQ_ERROR rf_common_event_get(uint8_t *packet_length, uint8_t *event_address);
void rf_common_cmd_send(uint8_t *cmd_address, uint8_t cmd_length);
 
/******************************************************************************
 *  Common helpers
 *****************************************************************************/
#define RUCI_SEND_CMD(cmd) \
    rf_common_ruci_direct_cmd((uint8_t *)(cmd), sizeof(cmd), (cmd)[1])

/******************************************************************************
 *  Core RUCI direct command handler
 *****************************************************************************/
bool rf_common_ruci_direct_cmd(uint8_t *cmd,
                               uint8_t  cmd_len,
                               uint8_t  expected_subheader)
{
    uint16_t int_en_backup;
    uint8_t  event_len = 0;

    RF_MCU_RX_CMDQ_ERROR            status;
    ruci_para_cmn_cnf_event_t       event;

    /* Backup & disable interrupts */
    int_en_backup = RfMcu_InterruptEnGet();
    RfMcu_InterruptEnSet(0x0000);

    enter_critical_section();
    rf_common_cmd_send(cmd, cmd_len);
    status = rf_common_event_get(&event_len, (uint8_t *)&event);
    leave_critical_section();

    /* Restore interrupts */
    RfMcu_InterruptClear(0xFF);
    RfMcu_InterruptEnSet(int_en_backup);

    RUCI_ENDIAN_CONVERT((uint8_t *)&event, RUCI_CMN_CNF_EVENT);

    if (status != RF_MCU_RX_CMDQ_GET_SUCCESS ||
        event.cmn_cmd_subheader != expected_subheader ||
        event.status != 0)
    {
        return false;
    }

    return true;
}

/******************************************************************************
 *  BLE / RF configuration APIs
 *****************************************************************************/

bool rf_ble_init_direct(void)
{
    static uint8_t const cmd[] = {0x12, 0x01, 0x00};
    return RUCI_SEND_CMD(cmd);
}

bool rf_set_rf_freq_direct(uint32_t freq)
{
    uint8_t cmd[] =
    {
        0x10, 0x01, 0x04,
        (uint8_t)(freq >> 0),
        (uint8_t)(freq >> 8),
        (uint8_t)(freq >> 16),
        (uint8_t)(freq >> 24),
    };

    return RUCI_SEND_CMD(cmd);
}

bool rf_set_ble_modem_direct(ble_data_rate_t rate)
{
    uint8_t cmd[] = {0x12, 0x02, 0x02, 0x00, 0x00};

    switch (rate)
    {
        case BLE_DR_2M:
        case BLE_DR_1M:
            cmd[3] = rate;
            break;

        case BLE_DR_CODED_S8:
        case BLE_DR_CODED_S2:
            cmd[3] = 2;
            cmd[4] = (rate - BLE_DR_CODED_S8);
            break;

        default:
            return false;
    }

    return RUCI_SEND_CMD(cmd);
}

bool rf_set_ble_mac_direct(void)
{
    static uint8_t const cmd[] =
    {
        0x12, 0x03, 0x0A,
        0x29, 0x41, 0x76, 0x71,
        0x00, 0x00,
        0x55, 0x55, 0x55,
        0x00
    };

    return RUCI_SEND_CMD(cmd);
}

bool rf_set_rfe_tx_enable_direct(rfe_tx_data_type_t type, uint16_t pkt_count)
{
    uint8_t cmd[] =
    {
        0x10, 0x18, 0x14,
        0xFF, 0x00, 0x25,
        0x00, 0x00, 0x00,
        (uint8_t)(pkt_count >> 0), (uint8_t)(pkt_count >> 8),
        (uint8_t)type,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00
    };

    return RUCI_SEND_CMD(cmd);
}

bool rf_set_rfe_rx_enable_direct(uint8_t data_type)
{
    uint8_t cmd[] =
    {
        0x10, 0x1A, 0x10,
        0x00, data_type,
        0x00, 0x01,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00
    };

    return RUCI_SEND_CMD(cmd);
}

bool rf_set_rfe_tx_disable(void)
{
    uint8_t cmd[] =
    {
        0x10, 0x19, 0x00,
    };

    return RUCI_SEND_CMD(cmd);
}

bool rf_set_rfe_rx_disable(void)
{
    uint8_t cmd[] =
    {
        0x10, 0x1b, 0x00
    };

    return RUCI_SEND_CMD(cmd);
}

bool rf_set_agc(uint8_t agc_en, uint8_t lna_gain, uint8_t tia_gain, uint8_t vga_gain)
{
    uint8_t cmd[] =
    {
        0x31, 0x01, 0x04,
        (uint8_t)(agc_en),
        (uint8_t)(lna_gain),
        (uint8_t)(tia_gain),
        (uint8_t)(vga_gain),
    };

    return RUCI_SEND_CMD(cmd);
}

/******************************************************************************
 *  Statistics
 *****************************************************************************/

uint32_t rf_get_pkt_count_direct(void)
{
    uint8_t cmd[] = {0x10, 0x0C, 0x00};
    uint8_t event_len;

    ruci_para_get_crc_report_event_t event;

    enter_critical_section();
    rf_common_ruci_direct_cmd(cmd, sizeof(cmd), cmd[1]);
    rf_common_event_get(&event_len, (uint8_t *)&event);
    RfMcu_InterruptClear(0xFF);
    leave_critical_section();

    return event.crc_success_count;
}

/******************************************************************************
 *  Simple DTM tests
 *****************************************************************************/

void rf_ble_simple_dtm_tx_test(void)
{
    rf_ble_init_direct();
    rf_set_rf_freq_direct(2402);
    rf_set_ble_modem_direct(BLE_DR_1M);
    rf_set_ble_mac_direct();
    rf_set_rfe_tx_enable_direct(RFE_TX_PRBS9, 0xffff);
}

void rf_ble_simple_dtm_rx_test(void)
{
    rf_ble_init_direct();
    rf_set_rf_freq_direct(2402);
    rf_set_ble_modem_direct(BLE_DR_1M);
    rf_set_ble_mac_direct();
    rf_set_rfe_rx_enable_direct(0xFF);

    uint32_t recv_pkt_cnt = rf_get_pkt_count_direct();
    (void)recv_pkt_cnt;
}

void rf_ble_simple_change_tx_frequency(uint32_t frequency)
{
    rf_set_rfe_tx_disable();
    rf_set_rf_freq_direct(frequency);
    rf_set_rfe_tx_enable_direct(RFE_TX_PRBS9, 0xffff);
}
