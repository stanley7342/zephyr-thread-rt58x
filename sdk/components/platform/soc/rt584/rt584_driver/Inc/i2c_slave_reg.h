/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2c_slave_reg.h
 * \brief           i2c slave register header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef I2C_SLAVE_REG_H
#define I2C_SLAVE_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           I2c slave total register 
 */
typedef struct {
    __IO uint32_t i2c_slave_data;           /*!< 0x00 i2c slave T/Rx data */
    __IO uint32_t i2c_slave_addr;           /*!< 0x04 i2c slave address */
    __IO uint32_t i2c_slave_int_enable;     /*!< 0x08 i2c slave interrupt enable 
                                                register */
    __IO uint32_t i2c_slave_int_status;     /*!< 0x0C i2c slave interrupt status 
                                                register */
    __IO uint32_t i2c_slave_timeout;        /*!< 0x10 i2c slave timeout register */
    __IO uint32_t i2c_slave_enable;         /*!< 0x14 i2c slave enable register */
    __I  uint32_t i2c_slave_status;         /*!< 0x18 i2c slave status register */
} i2c_slave_t;


#ifdef __cplusplus
}
#endif

#endif /* End of I2C_SLAVE_REG_H */
