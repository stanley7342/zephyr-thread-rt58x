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



#ifndef RT584_TRNG_H
#define RT584_TRNG_H

#ifdef __cplusplus
extern "C"
{
#endif



#include "mcu.h"
#include "sysctrl.h"
#include "pufs_rt_regs.h"


/**
 * \defgroup        TRNG Trng
 * \ingroup         RT584_DRIVER
 * \brief           Define Trng definitions, structures, and functions
 * @{
 */

/**
* \brief            Get_random_numberk
* \param[in]        p_buffer:
* \param[in]        number: 
* \retval           STATUS_SUCCESS           If uninitialization was successful.
*/
uint32_t get_random_number(uint32_t *p_buffer, uint32_t number);

/*@}*/ /* end of RT584_DRIVER TRNG */

#ifdef __cplusplus
}
#endif

#endif /* End of RT584_TRNG_H */


