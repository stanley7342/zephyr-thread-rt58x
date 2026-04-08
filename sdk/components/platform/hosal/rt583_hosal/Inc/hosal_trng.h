/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_wdt.h
 * \brief           Hosal Watch Dog timer header file
 */
/*
 * Author:          
 */

#ifndef HOSAL_TRNG_H
#define HOSAL_TRNG_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "mcu.h"
#include "trng.h"


/**
 * \defgroup HOSAL_TRNG Hosal trng
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal trng definitions, structures, and functions
 * @{
 */


/**
 * \brief           Hosal trng get random number
 * \param[in]       buffer: random data buffer
 * \param[in]       number: get random number times
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
int hosal_trng_get_random_number(uint32_t *buffer, uint32_t number);

/*@}*/ /* end of RT58X_HOSAL HOSAL_TRNG */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_TRNG_H */
