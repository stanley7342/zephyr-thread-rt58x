/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File         ruci_pci_event.c
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
#include "ruci_pci_event.h"

#if (RUCI_ENDIAN_INVERSE)
#if (RUCI_ENABLE_PCI)

/******************************************************************************
* GLOBAL PARAMETERS
******************************************************************************/
// RUCI: cnf_event -------------------------------------------------------------
const uint8_t ruci_elmt_type_cnf_event[] =
{
    1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_cnf_event[] =
{
    1, 1, 1, 1, 1, 1
};

// RUCI: get_reg_event ---------------------------------------------------------
const uint8_t ruci_elmt_type_get_reg_event[] =
{
    1, 1, 1, 4
};
const uint8_t ruci_elmt_num_get_reg_event[] =
{
    1, 1, 1, 1
};

// RUCI: get_crc_report_event --------------------------------------------------
const uint8_t ruci_elmt_type_get_crc_report_event[] =
{
    1, 1, 1, 4, 4
};
const uint8_t ruci_elmt_num_get_crc_report_event[] =
{
    1, 1, 1, 1, 1
};

// RUCI: dtm_burst_tx_done_event -----------------------------------------------
const uint8_t ruci_elmt_type_dtm_burst_tx_done_event[] =
{
    1, 1, 1
};
const uint8_t ruci_elmt_num_dtm_burst_tx_done_event[] =
{
    1, 1, 1
};

// RUCI: get_rssi_event --------------------------------------------------------
const uint8_t ruci_elmt_type_get_rssi_event[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_get_rssi_event[] =
{
    1, 1, 1, 1
};

// RUCI: get_zwave_rssi_event --------------------------------------------------
const uint8_t ruci_elmt_type_get_zwave_rssi_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_get_zwave_rssi_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// RUCI: get_phy_status_event --------------------------------------------------
const uint8_t ruci_elmt_type_get_phy_status_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_get_phy_status_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// RUCI: htrp_conn_timeout_event -----------------------------------------------
const uint8_t ruci_elmt_type_htrp_conn_timeout_event[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_htrp_conn_timeout_event[] =
{
    1, 1, 1, 1
};

// RUCI: htrp_per_event --------------------------------------------------------
const uint8_t ruci_elmt_type_htrp_per_event[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_htrp_per_event[] =
{
    1, 1, 1, 16
};

// RUCI: htrp_rssi_event -------------------------------------------------------
const uint8_t ruci_elmt_type_htrp_rssi_event[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_htrp_rssi_event[] =
{
    1, 1, 1, 1
};

// RUCI: htrp_fw_status_event --------------------------------------------------
const uint8_t ruci_elmt_type_htrp_fw_status_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_htrp_fw_status_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 16, 16
};

// RUCI: htrp_hw_status_event --------------------------------------------------
const uint8_t ruci_elmt_type_htrp_hw_status_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1
};
const uint8_t ruci_elmt_num_htrp_hw_status_event[] =
{
    1, 1, 1, 1, 1, 1, 1, 1, 1
};

// RUCI: get_csl_accuracy_event ------------------------------------------------
const uint8_t ruci_elmt_type_get_csl_accuracy_event[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_get_csl_accuracy_event[] =
{
    1, 1, 1, 1
};

// RUCI: get_csl_uncertainty_event ---------------------------------------------
const uint8_t ruci_elmt_type_get_csl_uncertainty_event[] =
{
    1, 1, 1, 1
};
const uint8_t ruci_elmt_num_get_csl_uncertainty_event[] =
{
    1, 1, 1, 1
};

#endif /* RUCI_ENABLE_PCI */
#endif /* RUCI_ENDIAN_INVERSE */
