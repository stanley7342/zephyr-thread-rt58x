/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_pci_data.c
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
#include "ruci_pci_data.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_PCI)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: set_tx_control_field --------------------------------------------------
const uint8_t ruci_elmt_type_set_tx_control_field[] =
{
    1, 1, 2, 1, 1
};
const uint8_t ruci_elmt_num_set_tx_control_field[] =
{
    1, 1, 1, 1, 1
};

// RUCI: rx_control_field ------------------------------------------------------
const uint8_t ruci_elmt_type_rx_control_field[] =
{
    1, 1, 2, 1, 1, 1
};
const uint8_t ruci_elmt_num_rx_control_field[] =
{
    1, 1, 1, 1, 1, 1
};

#endif /* RUCI_ENABLE_PCI */
#endif /* RUCI_ENDIAN_INVERSE */
