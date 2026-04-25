/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           system_mcu.h
 * \brief          system mcu header file (CM33)
 */
/*
 * This file is part of library_name.
 * Author: 
 */


#ifndef SYSTEM_CM33_H
#define SYSTEM_CM33_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdint.h>
#include "mcu.h"


/**
 *  \brief          Exception / Interrupt Handler Function Prototype
 */



/**
 * \brief           Exception / Interrupt Handler Function Prototype
 */
typedef void(*VECTOR_TABLE_Type)(void);

/**
 * \brief           Processor Clock Frequency
 */
extern uint32_t SystemCoreClock;

/**
 * \brief           System Core Clock Frequency
 */
extern uint32_t SystemFrequency;

/**
 * \brief           Setup the microcontroller system.
 *                  Initialize the System and update the SystemCoreClock variable.
 */
void systeminit(void);

/**
 * \brief           Updates the SystemCoreClock with current core Clock
 *                  retrieved from cpu registers.
 */
void systemcoreclockupdate(void);

#ifdef __cplusplus
}
#endif

#endif /* End of SYSTEM_CM33_H */
