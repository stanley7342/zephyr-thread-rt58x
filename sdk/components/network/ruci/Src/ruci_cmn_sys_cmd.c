/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_cmn_sys_cmd.c
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
#include "ruci_cmn_sys_cmd.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_CMN)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: get_fw_ver ------------------------------------------------------------
const uint8_t ruci_elmt_type_get_fw_ver[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_get_fw_ver[] =
{
    1, 1, 1
};

// RUCI: set_pta_default -------------------------------------------------------
const uint8_t ruci_elmt_type_set_pta_default[] =
{
    1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_pta_default[] =
{
    1, 1, 1, 1, 1
};

// RUCI: set_ic_version --------------------------------------------------------
const uint8_t ruci_elmt_type_set_ic_version[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_set_ic_version[] =
{
    1, 1, 1, 1
};

#endif /* RUCI_ENABLE_CMN */
#endif /* RUCI_ENDIAN_INVERSE */
