/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_pci_ook_cmd.c
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
#include "ruci_pci_ook_cmd.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_PCI)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: initiate_ook ----------------------------------------------------------
const uint8_t ruci_elmt_type_initiate_ook[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_initiate_ook[] =
{
    1, 1, 1, 1
};

// RUCI: set_ook_modem ---------------------------------------------------------
const uint8_t ruci_elmt_type_set_ook_modem[] =
{
    1, 1, 1, 4
};
const uint8_t ruci_elmt_num_set_ook_modem[] =
{
    1, 1, 1, 1
};

#endif /* RUCI_ENABLE_PCI */
#endif /* RUCI_ENDIAN_INVERSE */
