/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_i2c_slave.h
 * \brief           Hosal I2C slave driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_I2C_SLAVE_H
#define HOSAL_I2C_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "i2c_slave.h"



/**
 * \defgroup        HOSAL_I2C_SLAVE Hosal i2c slave
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal i2c slave definitions, structures, and functions
 * @{
 */

/**
 * \brief           I2C Slave callback status definitions
 */
#define HOSAL_I2C_SLAVE_STATUS_ADDR_MATCH I2C_SLAVE_STATUS_ADDR_MATCH
#define HOSAL_I2C_SLAVE_STATUS_DATA_READY I2C_SLAVE_STATUS_DATA_READY
#define HOSAL_I2C_SLAVE_STATUS_STOP       I2C_SLAVE_STATUS_STOP
#define HOSAL_I2C_SLAVE_STATUS_TIMEOUT    I2C_SLAVE_STATUS_TIMEOUT
#define HOSAL_I2C_SLAVE_STATUS_READ       I2C_SLAVE_STATUS_READ
#define HOSAL_I2C_SLAVE_STATUS_WRITE      I2C_SLAVE_STATUS_WRITE
#define HOSAL_I2C_SLAVE_STATUS_ERROR      I2C_SLAVE_STATUS_ERROR

/**
 * \brief           Structure for the I2C slave configuration.
 */
typedef struct {
    i2c_slave_cb_fn i2c_slave_cb_func; /*!< i2c slave callback function */
    uint8_t i2c_bus_timeout_enable;    /*!< i2c bus timeout enable */
    uint8_t i2c_bus_timeout;           /*!< i2c bus timeout value */
    uint8_t i2c_slave_addr;            /*!< i2c slave 7 bits only */
} hosal_i2c_slave_mode_t;

/**
 * \brief           Set I2C slave initialize
 * \param[in]       i2c_slave_client: i2c slave configuration
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_REQUEST
 */
uint32_t hosal_i2c_slave_open(hosal_i2c_slave_mode_t* i2c_slave_client);

/**
 * \brief           Close I2C slave
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_REQUEST
 */
uint32_t hosal_i2c_slave_close(void);

/**
 * \brief           Get one byte from i2c master
 * \param[out]      data: Pointer to a user-provided variable 
 *                              where the i2c master to slave byte 
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t hosal_i2c_slave_read_byte(uint8_t* data) {
    return i2c_slave_read_byte(data);
}

/**
 * \brief           Send one byte from i2c master
 * \param[in]       data: The i2c slave write to master data 
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t hosal_i2c_slave_write_byte(uint8_t data) {
    return i2c_slave_write_byte(data);
}

/*@}*/ /* end of RT584_HOSAL HOSAL_I2C_SLAVE */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_I2C_SLAVE_H */
