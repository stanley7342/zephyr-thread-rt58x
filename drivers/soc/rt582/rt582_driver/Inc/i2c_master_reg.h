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
    __IO uint32_t status;                       /*!< 0x00 i2c status */
    __I uint32_t rd_data;                       /*!< 0x04 i2c read data */
    __IO uint32_t command;                      /*!< 0x08 i2c master data 
                                                 write to slave */
    __IO uint32_t int_enable;                   /*!< 0x0C i2c interrupt enable */
    __IO uint32_t int_status;                   /*!< 0x10 i2c interrupt status */
    __IO uint32_t control;                      /*!< 0x14 i2c master setting data */
    __IO uint32_t prescale_h;                   /*!< 0x18 i2c divider upper 8 bits */
} i2cm_t;

#ifdef __cplusplus
}
#endif

#endif /* End of I2C_MASTER_REG_H */
