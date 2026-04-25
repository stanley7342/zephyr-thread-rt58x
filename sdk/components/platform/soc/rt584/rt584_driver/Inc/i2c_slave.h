/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2c_slave.h
 * \brief           i2c slave driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"



/**
 * \defgroup        I2C_SLAVE I2C slave
 * \ingroup         RT584_DRIVER
 * \brief           Define I2C slave definitions, structures, and functions
 * @{
 */


/**
 * \brief           I2C Slave callback status definitions
 */
#define I2C_SLAVE_STATUS_ADDR_MATCH          (1 << 0)
#define I2C_SLAVE_STATUS_DATA_READY          (1 << 1)
#define I2C_SLAVE_STATUS_STOP                (1 << 2)
#define I2C_SLAVE_STATUS_TIMEOUT             (1 << 3)
#define I2C_SLAVE_STATUS_READ                (1 << 4)
#define I2C_SLAVE_STATUS_WRITE               (1 << 5)
#define I2C_SLAVE_STATUS_ERROR               (1 << 31)

/**
 * \brief           I2C Slave interrupt register definitions
 */
#define  I2C_SLAVE_MATCH_ADDR             (0x01)
#define  I2C_SLAVE_DATA_READY             (0x02)
#define  I2C_SLAVE_BUS_STOP               (0x04)
#define  I2C_SLAVE_BUS_TIMEOUT            (0x08)
#define  I2C_SLAVE_ALL_INT          (I2C_SLAVE_MATCH_ADDR | I2C_SLAVE_DATA_READY \
                                   | I2C_SLAVE_BUS_STOP | I2C_SLAVE_BUS_TIMEOUT)

/**
 * \brief           I2C Slave status register definitions
 */
#define  I2C_SLAVE_READ_OP                (1<<1)


/**
 * \brief           I2C slave finish routine notify callback for user application
 * \param[in]       status: I2C slave transfer status.
 */
typedef void (*i2c_slave_cb_fn)(uint32_t status);

/**
 * \brief           Structure for the I2C slave configuration.
 */
typedef struct
{
    i2c_slave_cb_fn i2c_slave_cb_func;          /*!< i2c slave callback function */
    uint8_t  i2c_bus_timeout_enable;            /*!< i2c bus timeout enable */
    uint8_t  i2c_bus_timeout;                   /*!< i2c bus timeout value */
    uint8_t  i2c_slave_addr;                    /*!< i2c slave 7 bits only */
} i2c_slave_mode_t;

/**
 * \brief           Set I2C slave initialize
 * \param[in]       i2c_slave_client: i2c slave configuration
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_REQUEST
 */
uint32_t i2c_slave_open(i2c_slave_mode_t *i2c_slave_client);

/**
 * \brief           Close I2C slave
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_REQUEST
 */
uint32_t i2c_slave_close(void);

/**
 * \brief           Get one byte from i2c master
 * \param[out]      data: Pointer to a user-provided variable 
 *                              where the i2c master to slave byte 
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2c_slave_read_byte(uint8_t* data) {
    *data = (I2C_SLAVE->i2c_slave_data & 0xFF);
    return STATUS_SUCCESS;
}

/**
 * \brief           Send one byte from i2c master
 * \param[in]       data: The i2c slave write to master data 
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2c_slave_write_byte(uint8_t data) {
    I2C_SLAVE->i2c_slave_data = data;
    return STATUS_SUCCESS;
}

/*@}*/ /* end of RT584_DRIVER I2C_SLAVE */

#ifdef __cplusplus
}
#endif

#endif /* End of I2C_SLAVE_H */
