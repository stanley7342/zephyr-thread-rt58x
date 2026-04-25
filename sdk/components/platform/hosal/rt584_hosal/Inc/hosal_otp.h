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

#ifndef HOSAL_OTP_H
#define HOSAL_OTP_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "mcu.h"



/**
 * \defgroup        HOSAL_OTP Hosal otp
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal otp definitions, structures, and functions
 * @{
 */

/**
 * \brief           Gosal OTP prescale definitions.
 */

#define HOSAL_PUF_READ_VERSION      0
#define HOSAL_PUF_READ_UID          1
#define HOSAL_PUF_GET_RAND          2
#define HOSAL_PUF_READ_OTP1         3
#define HOSAL_PUF_READ_OTP2         4
#define HOSAL_PUF_GET_LOCK_OTP2     5
#define HOSAL_PUF_WRITE_OTP2        6
#define HOSAL_PUF_SET_LOCK_OTP2     7
#define HOSAL_PUF_GET_LOCK_OTP1     8
#define HOSAL_PUF_WRITE_OTP1        9
#define HOSAL_PUF_SET_LOCK_OTP1     10

/**
 * \brief           hosal_otp_init
 */

int hosal_otp_init(void);
/**
 * \brief           hosal_otp_ioctrl.
 */

int hosal_otp_ioctrl(int ctl,uint32_t address ,void* data, uint32_t len);


/*@}*/ /* end of RT584_HOSAL HOSAL_OTP */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_OTP_H */
