/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_pci_common_cmd.c
* @Version
* $Revision:8070
* $Date: 2026-03-25
* @Brief
* @Note
*
*****************************************************************************/

/******************************************************************************
* INCLUDES
******************************************************************************/
#include "ruci_pci_common_cmd.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_PCI)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: set_rf_frequency ------------------------------------------------------
const uint8_t ruci_elmt_type_set_rf_frequency[] =
{
    1, 1, 1, 4
};
const uint8_t ruci_elmt_num_set_rf_frequency[] =
{
    1, 1, 1, 1
};

// RUCI: set_wake_on_radio -----------------------------------------------------
const uint8_t ruci_elmt_type_set_wake_on_radio[] =
{
    1, 1, 1, 4, 2, 4
};
const uint8_t ruci_elmt_num_set_wake_on_radio[] =
{
    1, 1, 1, 1, 1, 1
};

// RUCI: set_rx_enable ---------------------------------------------------------
const uint8_t ruci_elmt_type_set_rx_enable[] =
{
    1, 1, 1, 4
};
const uint8_t ruci_elmt_num_set_rx_enable[] =
{
    1, 1, 1, 1
};

// RUCI: set_single_tone_mode --------------------------------------------------
const uint8_t ruci_elmt_type_set_single_tone_mode[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_single_tone_mode[] =
{
    1, 1, 1, 1
};

// RUCI: set2_ch_scan_frequency ------------------------------------------------
const uint8_t ruci_elmt_type_set2_ch_scan_frequency[] =
{
    1, 1, 1, 1, 4, 4
};
const uint8_t ruci_elmt_num_set2_ch_scan_frequency[] =
{
    1, 1, 1, 1, 1, 1
};

// RUCI: set_rx_reserve_enable -------------------------------------------------
const uint8_t ruci_elmt_type_set_rx_reserve_enable[] =
{
    1, 1, 1, 4, 4
};
const uint8_t ruci_elmt_num_set_rx_reserve_enable[] =
{
    1, 1, 1, 1, 1
};

// RUCI: get_crc_count ---------------------------------------------------------
const uint8_t ruci_elmt_type_get_crc_count[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_get_crc_count[] =
{
    1, 1, 1
};

// RUCI: set_rf_sleep ----------------------------------------------------------
const uint8_t ruci_elmt_type_set_rf_sleep[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_rf_sleep[] =
{
    1, 1, 1, 1
};

// RUCI: set_rf_idle -----------------------------------------------------------
const uint8_t ruci_elmt_type_set_rf_idle[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_set_rf_idle[] =
{
    1, 1, 1
};

// RUCI: set_duty_cycle --------------------------------------------------------
const uint8_t ruci_elmt_type_set_duty_cycle[] =
{
    1, 1, 1, 4, 4
};
const uint8_t ruci_elmt_num_set_duty_cycle[] =
{
    1, 1, 1, 1, 1
};

// RUCI: get_rssi --------------------------------------------------------------
const uint8_t ruci_elmt_type_get_rssi[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_get_rssi[] =
{
    1, 1, 1
};

// RUCI: get_phy_status --------------------------------------------------------
const uint8_t ruci_elmt_type_get_phy_status[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_get_phy_status[] =
{
    1, 1, 1
};

// RUCI: set_clock_mode --------------------------------------------------------
const uint8_t ruci_elmt_type_set_clock_mode[] =
{
    1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_clock_mode[] =
{
    1, 1, 1, 1, 1, 1
};

// RUCI: set_rfb_auto_state ----------------------------------------------------
const uint8_t ruci_elmt_type_set_rfb_auto_state[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_rfb_auto_state[] =
{
    1, 1, 1, 1
};

// RUCI: set_rfe_security ------------------------------------------------------
const uint8_t ruci_elmt_type_set_rfe_security[] =
{
    1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_rfe_security[] =
{
    1, 1, 1, 16, 13
};

// RUCI: set_rfe_tx_enable -----------------------------------------------------
const uint8_t ruci_elmt_type_set_rfe_tx_enable[] =
{
    1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 4, 1, 2, 2
};
const uint8_t ruci_elmt_num_set_rfe_tx_enable[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// RUCI: set_rfe_tx_disable ----------------------------------------------------
const uint8_t ruci_elmt_type_set_rfe_tx_disable[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_set_rfe_tx_disable[] =
{
    1, 1, 1
};

// RUCI: set_rfe_rx_enable -----------------------------------------------------
const uint8_t ruci_elmt_type_set_rfe_rx_enable[] =
{
    1, 1, 1, 1, 1, 1, 1, 4, 1, 2, 1, 2, 2
};
const uint8_t ruci_elmt_num_set_rfe_rx_enable[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// RUCI: set_rfe_rx_disable ----------------------------------------------------
const uint8_t ruci_elmt_type_set_rfe_rx_disable[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_set_rfe_rx_disable[] =
{
    1, 1, 1
};

// RUCI: set_rfe_mode ----------------------------------------------------------
const uint8_t ruci_elmt_type_set_rfe_mode[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_rfe_mode[] =
{
    1, 1, 1, 1
};

// RUCI: set_gaussian_filter_type ----------------------------------------------
const uint8_t ruci_elmt_type_set_gaussian_filter_type[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_gaussian_filter_type[] =
{
    1, 1, 1, 1
};

#endif /* RUCI_ENABLE_PCI */
#endif /* RUCI_ENDIAN_INVERSE */
