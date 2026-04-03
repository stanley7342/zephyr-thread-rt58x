/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_sf_host_sys_cmd.c
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
#include "ruci_sf_host_sys_cmd.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_SF)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: tx_pwr_comp_cfg -------------------------------------------------------
const uint8_t ruci_elmt_type_tx_pwr_comp_cfg[] =
{
    1, 1, 2, 1
};
const uint8_t ruci_elmt_num_tx_pwr_comp_cfg[] =
{
    1, 1, 1, 1
};

// RUCI: set_verify_mode -------------------------------------------------------
const uint8_t ruci_elmt_type_set_verify_mode[] =
{
    1, 1, 1, 1, 1, 2
};
const uint8_t ruci_elmt_num_set_verify_mode[] =
{
    1, 1, 1, 1, 1, 1
};

// RUCI: set_verify_ble_para ---------------------------------------------------
const uint8_t ruci_elmt_type_set_verify_ble_para[] =
{
    1, 1, 1, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_verify_ble_para[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 6
};

// RUCI: set_verify_pci_para ---------------------------------------------------
const uint8_t ruci_elmt_type_set_verify_pci_para[] =
{
    1, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 1, 2, 1, 1
};
const uint8_t ruci_elmt_num_set_verify_pci_para[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 8
};

// RUCI: set_verify_test -------------------------------------------------------
const uint8_t ruci_elmt_type_set_verify_test[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_verify_test[] =
{
    1, 1, 1, 1
};

// RUCI: set_htrp_test_para ----------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_test_para[] =
{
    1, 1, 1, 1, 1, 2, 1, 1, 1, 4, 4, 4, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_htrp_test_para[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 16, 16
};

// RUCI: set_htrp_test ---------------------------------------------------------
const uint8_t ruci_elmt_type_set_htrp_test[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_htrp_test[] =
{
    1, 1, 1, 1
};

#endif /* RUCI_ENABLE_SF */
#endif /* RUCI_ENDIAN_INVERSE */
