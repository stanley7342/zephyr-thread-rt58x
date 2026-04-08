/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2c_master.h
 * \brief           i2c master driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"


/**
 * \defgroup I2C_MASTER I2C Master
 * \ingroup RT58X_DRIVER
 * \brief  Define I2C Master definitions, structures, and functions
 * @{
 */

/**
 * \brief           I2C CLOCK Constant Definitions
 */
#define I2C_CLOCK_800K 3
#define I2C_CLOCK_400K 0
#define I2C_CLOCK_200K 1
#define I2C_CLOCK_100K 2

/**
 * \brief           I2C Master return ERROR Constant Definitions
 */
#define I2C_STATUS_OK        0
#define I2C_STATUS_ERR_NOACK 1

/**
 * \brief           I2C Master status register bit mask
 */
#define I2CM_STATUS_CMDFIFO_EMPTY  0x0001
#define I2CM_STATUS_DATA_READY     0x0002
#define I2CM_STATUS_I2C_ERR        0x0004
#define I2CM_STATUS_I2C_ERR_CLEAR  0x0004
#define I2CM_STATUS_LOST_ARB       0x0008
#define I2CM_STATUS_RDFIFO_UNDFL   0x0010
#define I2CM_STATUS_CMDFIFO_OVERFL 0x0020
#define I2CM_STATUS_FIFO_FULL      0x0040
#define I2CM_STATUS_TRANSFER       0x0080
#define I2CM_STATUS_RDFIFO_OVFRFL  0x0100
#define I2CM_STATUS_CMDFIFO_UNDFL  0x0200
#define I2CM_STATUS_FIFO_CLR       0x0400

/**
 * \brief           I2C Master control register bit mask
 */
#define I2CM_CTRL_ENABLE       0x80
#define I2CM_CTRL_DISABLE      0x00
#define I2CM_CTRL_CLK_SRC_APB  0x40
#define I2CM_CTRL_CLK_SRC_EXT  0x00
#define I2CM_CTRL_CLK_DIV_MASK 0x3F

/**
 * \brief           I2C Master prescale register bit definitions
 */
#define I2CM_CLK_PRESCALE_SHIFT 6

/**
 * \brief           I2C Master interrupt register mask bit definitions
 */
#define I2CM_INT_CMDFIFO_EMPTY 0x01
#define I2CM_INT_DATA_READY    0x02
#define I2CM_INT_ERROR         0x04
#define I2CM_INT_LOST_ARB      0x08
#define I2CM_INT_COMPLETE      0x10

/**
 * \brief           I2C Master engine commands definitions
 */
/* Command string NULL terminator (only when state machine is expecting a command,
   not data) */
#define I2CM_CMD_NULL 0x00
/* write a 0 as 1 bit of data - this can be used as a Master ACK */
#define I2CM_CMD_WDAT0 0x10
/* write a 1 as 1 bit of data */
#define I2CM_CMD_WDAT1 0x11
/* next byte in command queue is 8 bit data to write (can be word address) */
#define I2CM_CMD_WDAT8 0x12
/* read 8 bits of data */
#define I2CM_CMD_RDAT8 0x13
/* transmit a stop  bit */
#define I2CM_CMD_STOP 0x14
/* transmit a start bit */
#define I2CM_CMD_STRT 0x15
/* verify that device is sending an ACK */
#define I2CM_CMD_VACK 0x16
/* verify that device is sending a NACK */
#define I2CM_CMD_VNACK 0x17

/**
 * \brief           I2C finish routine notify callback for user application
 * \param[in]       status: I2C transfer status.
 *                          It must be I2C_STATUS_OK or I2C_STATUS_ERR_NOACK
 */
typedef void (*i2cm_cb_fn)(uint32_t status);

/**
 * \brief           Structure for the I2C transfer request.
 */
typedef struct {
    uint8_t dev_addr;     /*!< I2C device address, 7bits only   */
    uint8_t bFlag_16bits; /*!< 1 for register address is 16bits, 
                                                     0 for register address is 8bits.  */
    uint16_t reg_addr;    /*!< I2C register address value   */
} i2c_master_mode_t;

/**
 * \brief           I2C master pre-initial function
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t i2c_preinit(void);

/**
 * \brief           Set I2C master initialize
 * \param[in]       i2c_speed: Set the I2C master bus clock frequency
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_REQUEST
 */
uint32_t i2c_init(uint32_t i2c_speed);

/**
 * \brief           I2C write data to slave
 * \param[in]       slave: Specifies the I2C slave address and register address
 * \param[in]       data: Pointer to buffer with data to write to I2C slave
 * \param[in]       len:Number of data bytes to transmit, maxmum size is 1024 bytes
 * \param[in]       i2c_usr_isr: Specifies the interrupt ISR callback function 
 *                               when I2C request completed or aborted
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t i2c_write(const i2c_master_mode_t* slave, uint8_t* data, uint32_t len,
                   i2cm_cb_fn i2c_usr_isr);

/**
 * \brief           I2C read data to slave
 * \param[in]       slave: Specifies the I2C slave address and register address
 * \param[in]       data: Pointer to buffer with data to receive from I2C slave
 * \param[in]       len: Number of data bytes to receive, maxmum size is 1024 bytes
 * \param[in]       i2c_usr_isr: Specifies the interrupt ISR callback function 
 *                               when I2C request completed or aborted
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT, STATUS_EBUSY
 */
uint32_t i2c_read(const i2c_master_mode_t* slave, uint8_t* data, uint32_t len,
                  i2cm_cb_fn i2c_usr_isr);

/*@}*/ /* end of RT58X_DRIVER I2C_MASTER */


#ifdef __cplusplus
}
#endif

#endif /* End of I2C_MASTER_H */
