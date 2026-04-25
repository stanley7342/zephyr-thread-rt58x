/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

 /**
 * \file           rt_crypto.h
 * \brief          CRYPTO ACCELEATOR driver header file
 */
#ifndef _RT584_CRYPTO_H_
#define _RT584_CRYPTO_H_

#include <stdint.h>

#include "rt_aes.h"

#include "rt_sm4.h"

#include "rt_ecc.h"

#include "rt_sha256.h"

#include "rt_hmac_drbg.h"

#include "rt_crypto_util.h"

#include "rt_sm3.h"

#if defined(CONFIG_CRYPTO_SECP256R1_ENABLE)
#include "rt_ecjpake.h"
#endif


/**
 * \defgroup        CRYPTO Crypto
 * \ingroup         RT584_DRIVER
 * \brief           Define Crypto definitions, structures, and functions
 * @{
 */

#if defined(CONFIG_CRYPTO_INT_ENABLE)
extern volatile uint32_t  crypto_finish;
#endif


#define   AES_FIRMWARE              0x000000AE      /*!< AES firmware */
#define   ECC_FIRMWARE              0xECC00000      /*!< ECDH firmware */
#define   ECC_FIRMWARE_VER          0xECC50000      /*!< ECDSA Verify firmware */
#define   ECC_FIRMWARE_SIG          0xECC80000      /*!< ECDSA Signature firmware */
#define   ECC_FIRMWARE_C25519       0xC2551900      /*!< Curve25519 ECDH firmware */

#define   SM2_FIRMWARE              0xECCCA200      /*!< SM2 */
#define   SM2_FIRMWARE_SIG          0xECCCA500      /*!< SM2 Signature */
#define   SM3_FIRMWARE              0x0000CA53      /*!< SM3 */
#define   SM4_FIRMWARE              0x0000CA54      /*!< SM4 */

#define   SHA_FIRMWARE              0x00005A00      /*!< SHA */

/*@}*/ /* end of RT584_DRIVER CRYPTO */

#endif /* End of _RT584_CRYPTO_H_ */

