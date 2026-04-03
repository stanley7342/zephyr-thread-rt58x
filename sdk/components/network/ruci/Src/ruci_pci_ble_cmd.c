/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_pci_ble_cmd.c
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
#include "ruci_pci_ble_cmd.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_PCI)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: initiate_ble ----------------------------------------------------------
const uint8_t ruci_elmt_type_initiate_ble[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_initiate_ble[] =
{
    1, 1, 1
};

// RUCI: set_ble_modem ---------------------------------------------------------
const uint8_t ruci_elmt_type_set_ble_modem[] =
{
    1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_ble_modem[] =
{
    1, 1, 1, 1, 1
};

// RUCI: set_ble_mac -----------------------------------------------------------
const uint8_t ruci_elmt_type_set_ble_mac[] =
{
    1, 1, 1, 4, 1, 1, 4
};
const uint8_t ruci_elmt_num_set_ble_mac[] =
{
    1, 1, 1, 1, 1, 1, 1
};

// RUCI: set_htrp_para ---------------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_para[] =
{
    1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 1
};
const uint8_t ruci_elmt_num_set_htrp_para[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// RUCI: set_htrp_map_channel --------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_map_channel[] =
{
    1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_htrp_map_channel[] =
{
    1, 1, 1, 1, 1, 40
};

// RUCI: set_htrp_sec ----------------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_sec[] =
{
    1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_htrp_sec[] =
{
    1, 1, 1, 1, 16, 16
};

// RUCI: set_htrp_tx_enable ----------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_tx_enable[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_set_htrp_tx_enable[] =
{
    1, 1, 1
};

// RUCI: set_htrp_rx_enable ----------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_rx_enable[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_set_htrp_rx_enable[] =
{
    1, 1, 1
};

// RUCI: set_htrp_disable ------------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_disable[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_set_htrp_disable[] =
{
    1, 1, 1
};

// RUCI: initiate_htrp ---------------------------------------------------------
const uint8_t ruci_elmt_type_initiate_htrp[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_initiate_htrp[] =
{
    1, 1, 1, 1
};

// RUCI: get_htrp_status_report ------------------------------------------------
const uint8_t ruci_elmt_type_get_htrp_status_report[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_get_htrp_status_report[] =
{
    1, 1, 1, 1
};

#endif /* RUCI_ENABLE_PCI */
#endif /* RUCI_ENDIAN_INVERSE */
