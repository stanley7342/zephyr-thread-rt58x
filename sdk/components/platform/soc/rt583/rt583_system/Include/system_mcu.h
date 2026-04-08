/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file      System_mcu   CM3 Device MCU       
 * \brief     System Initialization header file for Cortex-M3 device based on CMSIS-CORE
 */
/*
 * Author:        
 */
#ifndef SYSTEM_CM3MCU_H
#define SYSTEM_CM3MCU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "sysfun.h"

/**
 * \defgroup SYSTEM_MCU System mcu
 * \ingroup RT58X_DRIVER
 * \brief  Define System mcu definitions, structures, and functions
 * @{
 */

/**
 * \brief           PMU mode type definitions.
 */
#define PMU_LDO_MODE  0
#define PMU_DCDC_MODE 1

#ifndef SET_PMU_MODE
#define SET_PMU_MODE PMU_DCDC_MODE
#endif

/**
 * \brief           Set the system PMU mode
 * \param[in]       pmu_mode Specifies the system PMU mode
 *                  This parameter can be the following values:
 *                      \arg PMU_MODE_LDO: Specifies the system PMU LDO mode
 *                      \arg PMU_MODE_DCDC: Specifies the system PMU DCDC mode
 * \return          None
 */
void SystemPmuSetMode(pmu_mode_cfg_t pmu_mode);
extern uint32_t SystemFrequency;                /*!< System Clock Frequency (Core Clock)  */
extern uint32_t SystemCoreClock;                /*!< Processor Clock Frequency            */

/**
 * \brief           Setup the microcontroller system.
 *                  Initialize the System and update the SystemCoreClock variable.
 */
void SystemInit(void);

/**
 * \brief           Updates the SystemCoreClock with current core Clock
 *                  retrieved from cpu registers.
 */
void systemcoreclockupdate(void);

/**
 * \brief           Updates the SystemFrequency with current core Clock
 *                  retrieved from clock mode.
 */

void systemfrequencyupdate(void);

/*@}*/ /* end of RT58X_DRIVER SYSTEM_MCU */

#ifdef __cplusplus
}
#endif

#endif /* end of SYSTEM_CM3MCU_H */
