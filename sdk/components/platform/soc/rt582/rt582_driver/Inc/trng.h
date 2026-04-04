/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           trng.h
 * \brief          true random number generator definition header file
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#ifndef RT582_TRNG_H
#define RT582_TRNG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "mcu.h"

/**
 * \defgroup TRNG Trng
 * \ingroup RT58X_DRIVER
 * \brief  Define Trng definitions, structures, and functions
 * @{
 */


/**
 * \brief           Generate a true 32-bits random number.
 * \return          A 32-bits random number
 * \details
 *                  Generate a true 32bits random number.
 *                  Please notice: this function is block mode, it will block about 4ms ~ 6ms.
 *                  If you want to use non-block mode, maybe you should use interrupt mode.
 */
int get_random_number(uint32_t* p_buffer, uint32_t number);

/*@}*/ /* end of RT58X_DRIVER TRNG */

#ifdef __cplusplus
}
#endif

#endif /* End of RT582_TRNG_H */


