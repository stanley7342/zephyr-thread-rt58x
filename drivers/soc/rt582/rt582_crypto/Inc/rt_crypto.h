/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**************************************************************************//**
 * @file     CRYPTO.h
 * @version
 * @brief    CRYPTO ACCELEATOR driver header file
 *
 ******************************************************************************/

#ifndef CRYPTO_H
#define CRYPTO_H


#include "mcu.h"


//#if defined(CONFIG_FREERTOS)
//#include "FreeRTOS.h"
//#include "task.h"
//#include "timers.h"
//#include "queue.h"
//#endif


#include "rt_aes.h"

#include "rt_ecc.h"

#include "rt_sha256.h"

#include "rt_hmac_drbg.h"

#include "rt_crypto_util.h"

#if defined(CONFIG_CRYPTO_SECP256R1_ENABLE)
#include "rt_ecjpake.h"
#endif

#endif

