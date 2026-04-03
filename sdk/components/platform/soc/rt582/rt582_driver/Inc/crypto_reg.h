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
 * This file is part of library_name.
 * Author: ives.lee
 */
#ifndef CRYPTO_REG_H
#define CRYPTO_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Crypto control register
 */
typedef union crypto_ctrl_struct {

    struct crypto_ctrl_b {
        __IO uint32_t vlw_op_num     : 8;       /*!< Crypto control parameter */
        __IO uint32_t vlw_sb_num     : 5;       /*!< Crypto control parameter */
        __IO uint32_t reserved1      : 3;       /*!< Reserved bits */
        __IO uint32_t en_crypto      : 1;       /*!< Crypto control parameter */
        __IO uint32_t enable_sha     : 1;       /*!< Crypto control parameter */
        __IO uint32_t reserved2      : 6;       /*!< Reserved bits */
        __IO uint32_t crypto_done    : 1;       /*!< Crypto control parameter */
        __IO uint32_t sha_done       : 1;       /*!< Crypto control parameter */
        __IO uint32_t crypto_busy    : 1;       /*!< Crypto control parameter */
        __IO uint32_t sha_busy       : 1;       /*!< Crypto control parameter */
        __IO uint32_t reserved3      : 3;       /*!< Reserved bits */
        __IO uint32_t clr_crypto_int : 1;       /*!< Crypto control parameter */
    } bit;

    uint32_t reg;

} crypto_ctrl_t;

/**
 * \brief           Crypto total register
 */
typedef struct {
    __IO crypto_ctrl_t crypto_cfg;              /*!< 0x00 Crypto control register */

} CRYPTO_T;

#ifdef __cplusplus
}
#endif

#endif /* End of CRYPTO_REG_H */
