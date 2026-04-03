/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_sf_host_event.c
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
#include "ruci_sf_host_event.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_SF)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: mcu_state -------------------------------------------------------------
const uint8_t ruci_elmt_type_mcu_state[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_mcu_state[] =
{
    1, 1, 1, 1
};

// RUCI: host_cnf_event --------------------------------------------------------
const uint8_t ruci_elmt_type_host_cnf_event[] =
{
    1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_host_cnf_event[] =
{
    1, 1, 1, 1, 1, 1
};

// RUCI: apci_int_state --------------------------------------------------------
const uint8_t ruci_elmt_type_apci_int_state[] =
{
    1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_apci_int_state[] =
{
    1, 1, 1, 1, 1
};

#endif /* RUCI_ENABLE_SF */
#endif /* RUCI_ENDIAN_INVERSE */
