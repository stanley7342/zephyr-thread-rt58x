/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           rt_sm3.h
 * \brief          
 */
#ifndef _RT584_SM3_H_
#define _RT584_SM3_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup        SM3 Sm3
 * \ingroup         RT584_DRIVER
 * \brief           Define Sm3 definitions, structures, and functions
 * @{
 */

#define  SM3_DIGEST_SIZE                32
#define  SM3_BLOCK_SIZE                 64


#if defined(CONFIG_CRYPTO_SM3_ENABLE)

/**
 * \brief          SM3 context structure
 */
typedef struct {
    uint32_t   total[2];       /*!< number of bytes processed  */
    uint8_t    buffer[64];     /*!< data block being processed */
} sm3_context;


/**
 * \brief          SM3 context setup
 * \param ctx      context to be initialized
 */
void sm3_init(sm3_context *ctx);

/**
 * \brief          SM3 process buffer
 * \param   ctx      SM3 context
 * \param   input    buffer holding the  data
 * \param   length   length of the input data
 */
uint32_t sm3_update(sm3_context *ctx, uint8_t *input, uint32_t length);

/**
 * \brief          SM3 final digest
 * \param   ctx         SM3 context
 * \param   digest      SM3 digest
 */
void sm3_final(sm3_context *ctx, uint8_t *digest);

#endif   /*end for CONFIG_CRYPTO_SM3_ENABLE */

/*@}*/ /* end of RT584_DRIVER SM3 */

#ifdef __cplusplus
}
#endif

#endif /* _RT584_SM3_H_ */
