/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * @file rf_test.h
 * @author
 * @date
 */

#ifndef _RF_TEST_H__
#define _RF_TEST_H__

/**************************************************************************************************
 *    TYPEDEFS
 *************************************************************************************************/
typedef enum
{
    BLE_DR_2M        = 0,
    BLE_DR_1M        = 1,
    BLE_DR_CODED_S8  = 2,
    BLE_DR_CODED_S2  = 3,
} ble_data_rate_t;

typedef enum
{
    RFE_TX_PRBS9 = 0,
    RFE_TX_0F,
    RFE_TX_55,
    RFE_TX_FF,
    RFE_TX_00,
    RFE_TX_F0,
    RFE_TX_AA,
    RFE_TX_88,
    RFE_TX_77,
} rfe_tx_data_type_t;

/**************************************************************************************************
 *    Global Prototypes
 *************************************************************************************************/
bool rf_ble_init_direct(void);
bool rf_set_rf_freq_direct(uint32_t freq);
bool rf_set_ble_modem_direct(ble_data_rate_t rate);
bool rf_set_ble_mac_direct(void);
bool rf_set_rfe_tx_enable_direct(rfe_tx_data_type_t type, uint16_t pkt_count);
bool rf_set_rfe_rx_enable_direct(uint8_t data_type);
bool rf_set_rfe_tx_disable(void);
bool rf_set_rfe_rx_disable(void);
uint32_t rf_get_pkt_count_direct(void);
bool rf_set_agc(uint8_t agc_en, uint8_t lna_gain, uint8_t tia_gain, uint8_t vga_gain);

#endif  //_RF_TEST_H__


