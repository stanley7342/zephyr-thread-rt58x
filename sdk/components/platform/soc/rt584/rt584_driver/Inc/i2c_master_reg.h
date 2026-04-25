/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2c_master_reg.h
 * \brief           i2c master register header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef I2C_MASTER_REG_H
#define I2C_MASTER_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           i2c master total register 
 */
typedef struct {
    __IO uint32_t control;                      /*!< 0x00 i2c control register */
    __IO uint32_t tar;                          /*!< 0x04 i2c target address */
    __IO uint32_t buf;                          /*!< 0x08 i2c command register */
    __I  uint32_t int_status;                   /*!< 0x0C i2c interrupt status 
                                                    register*/
    __IO uint32_t int_enable;                   /*!< 0x10 i2c interrupt enable 
                                                    register */
    __I  uint32_t int_raw_status;               /*!< 0x14 i2c interrupt raw status 
                                                    register*/
    __IO uint32_t int_clear;                    /*!< 0x18 i2c interrupt clear 
                                                    register */
    __IO uint32_t slck_gen;                     /*!< 0x1C i2c source clock divider */
} i2c_master_t;

#ifdef __cplusplus
}
#endif

#endif /* End of I2C_MASTER_REG_H */
