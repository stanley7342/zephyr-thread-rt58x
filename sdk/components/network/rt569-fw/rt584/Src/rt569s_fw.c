/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


/**************************************************************************//**
* @file     rt569mp_fw.c
* @version
* @brief    RF MCU FW initialization

******************************************************************************/

#include "mcu.h"
#if 0
#include "chip_define.h"
#endif
#include "rf_mcu.h"
#include "rf_mcu_chip.h"
#include "rf_common_init.h"


/**************************************************************************************************
 *    GLOBAL VARIABLES
 *************************************************************************************************/
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569S)
/* Use top level CHIP_VERSION */

/* RF1301 FW */
#if defined(CONFIG_RF1301) 

#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE)
#include "prg_rf1301_mpa_asic_pci_fw.h"
#endif

#if (CONFIG_RF_FW_INCLUDE_BLE == TRUE)
#include "prg_rf1301_mpa_asic_ble_fw.h"
#endif

#if (CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)
#warning "rf1301 does not support 2.4G multi-protocol"
#undef CONFIG_RF_FW_INCLUDE_MULTI_2P4G
#endif

#endif

/* RT584H FW */
#if defined(CONFIG_RT584H) || defined(CONFIG_RT584HA4)

#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE)
#include "prg_rt584h_mpa_asic_pci_fw.h"
#endif

#if (CONFIG_RF_FW_INCLUDE_BLE == TRUE)
#include "prg_rt584h_mpa_asic_ble_fw.h"
#endif

#if (CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)
#include "prg_rt584h_mpa_asic_multi_fw.h"
#endif

#endif

/* RT584L FW */
#if defined(CONFIG_RT584L) 

#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE)
#include "prg_rt584l_mpa_asic_pci_fw.h"
#endif

#if (CONFIG_RF_FW_INCLUDE_BLE == TRUE)
#include "prg_rt584l_mpa_asic_ble_fw.h"
#endif

#if (CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)
#include "prg_rt584l_mpa_asic_multi_fw.h"
#endif

#endif

/* General FW Symbol Declaration */

#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE)
const uint32_t firmware_size_ruci = sizeof(firmware_program_ruci);
#else
const uint8_t firmware_program_ruci[] = {0};
const uint32_t firmware_size_ruci = 0;
#endif

#if (CONFIG_RF_FW_INCLUDE_BLE == TRUE)
const uint32_t firmware_size_ble = sizeof(firmware_program_ble);
#else
const uint8_t firmware_program_ble[] = {0};
const uint32_t firmware_size_ble = 0;
#endif

#if (CONFIG_RF_FW_INCLUDE_MULTI_2P4G == TRUE)
const uint32_t firmware_size_multi = sizeof(firmware_program_multi);
#else
const uint8_t firmware_program_multi[] = {0};
const uint32_t firmware_size_multi = 0;
#endif

/* COMMON RADIO FW FOR INTERNAL TEST */

#if (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE)
#if (CONFIG_RF_FW_INCLUDE_PCI == TRUE)
const uint8_t *firmware_program_rfk = firmware_program_ruci;
const uint32_t firmware_size_rfk = sizeof(firmware_program_ruci);
#else
#error "CONFIG_RF_FW_INCLUDE_PCI is the prerequisite for RF_CAL_PWR_ON_MODE"
#endif
#else
const uint8_t firmware_program_rfk[] = {0};
const uint32_t firmware_size_rfk = 0;
#endif /* (RF_CAL_TYPE & RF_CAL_PWR_ON_MODE) */

#if (CONFIG_RF_FW_INCLUDE_INTERNAL_TEST == TRUE)
const uint8_t firmware_program_it[] = {0};
const uint32_t firmware_size_it = 0;
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
const uint8_t firmware_const_it[] = {0};
const uint32_t const_size_it = 0;
#endif
#endif /* (RF_FW_INCLUDE_INTERNAL_TEST == TRUE) */

#endif /* (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569S) */

