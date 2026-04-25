/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            crypto_reg.h
 * \brief           crypto_reg.h header file
 */

/*
 * Author:          
 */
#ifndef CRYPTO_REG_H
#define CRYPTO_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \brief          Crypto control register at offet 0x00
 */
typedef union crypto_ctrl_struct {

    struct crypto_ctrl_b {
        __IO uint32_t vlw_op_num     : 8;       /*!< */
        __IO uint32_t vlw_sb_num     : 5;       /*!< */
        __IO uint32_t reserved1      : 3;       /*!< */
        __IO uint32_t en_crypto      : 1;       /*!< */
        __IO uint32_t enable_sha     : 1;       /*!< */
        __IO uint32_t reserved2      : 6;       /*!< */
        __IO uint32_t crypto_done    : 1;       /*!< */
        __IO uint32_t sha_done       : 1;       /*!< */
        __IO uint32_t crypto_busy    : 1;       /*!< */
        __IO uint32_t sha_busy       : 1;       /*!< */
        __IO uint32_t reserved3      : 3;       /*!< */
        __IO uint32_t clr_crypto_int : 1;       /*!< */
    } bit;

    uint32_t reg;

} crypto_ctrl_t;

/**
 * \brief          Crypto total register 
 */
typedef struct {
    __IO crypto_ctrl_t crypto_cfg;              /*!< */
    __IO uint32_t      sha_digest_base;         /*!< */
    __IO uint32_t      sha_k_base;              /*!< */
    __IO uint32_t      sha_dma_base;            /*!< */
    __IO uint32_t      sha_dma_length;          /*!< */
    __IO uint32_t      sha_misc_cfg;            /*!< */
} crypto_t;



#ifdef __cplusplus
}
#endif

#endif /* End of CRYPTO_REG_H */
