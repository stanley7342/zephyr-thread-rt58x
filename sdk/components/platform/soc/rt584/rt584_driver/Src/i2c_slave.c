/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2c_slave.c
 * \brief           i2c slave driver file
 */
/*
 * Author:          Kc.tseng
 */
#include "i2c_slave.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define I2C_SLAVE_STATE_CLOSE            0
#define I2C_STATE_STATE_OPEN             1

#define I2C_SLAVE_STATE_IDLE             2
#define I2C_SLAVE_STATE_READ             3
#define I2C_SLAVE_STATE_WRITE            4

static i2c_slave_cb_fn user_slave_cb = NULL;
static uint8_t i2c_slave_state = I2C_SLAVE_STATE_CLOSE;


uint32_t i2c_slave_open(i2c_slave_mode_t *i2c_slave_client) {
    /*
     * Because I2C slave driver is interrupt driven, there must have user
     * i2c_slave_cb_func to process master request. This driver just handle
     * i2c slave bus interrupt, NOT i2c receive/send data.
     */
    if ((i2c_slave_client == NULL) || (i2c_slave_client->i2c_slave_cb_func == NULL)) {
        return STATUS_INVALID_PARAM;
    }

    enter_critical_section();

    if (i2c_slave_state != I2C_SLAVE_STATE_CLOSE) {
        leave_critical_section();
        /* SLAVE DEVICE already opened */
        return STATUS_EBUSY;            
    }

    i2c_slave_state = I2C_STATE_STATE_OPEN;
    leave_critical_section();

    user_slave_cb = i2c_slave_client->i2c_slave_cb_func;

    I2C_SLAVE->i2c_slave_enable = 0;

    /* clear all interrupt */
    I2C_SLAVE->i2c_slave_int_status = I2C_SLAVE_ALL_INT;

    /* set I2C slave address */
    I2C_SLAVE->i2c_slave_addr = (i2c_slave_client->i2c_slave_addr << 1) & 0xFF;

    I2C_SLAVE->i2c_slave_int_enable = I2C_SLAVE_ALL_INT;

    if (i2c_slave_client->i2c_bus_timeout_enable) {
        I2C_SLAVE->i2c_slave_timeout = (i2c_slave_client->i2c_bus_timeout << 8)
                                       | 1;
    } else {
        I2C_SLAVE->i2c_slave_timeout = 0;
    }

    I2C_SLAVE->i2c_slave_enable = 1;

    return STATUS_SUCCESS;
}

uint32_t i2c_slave_close(void) {

    NVIC_DisableIRQ(I2C_Slave_IRQn);
    I2C_SLAVE->i2c_slave_enable = 0;
    I2C_SLAVE->i2c_slave_int_enable = 0;
    I2C_SLAVE->i2c_slave_int_status = I2C_SLAVE_ALL_INT;

    enter_critical_section();
    i2c_slave_state = I2C_SLAVE_STATE_CLOSE;
    leave_critical_section();

    return STATUS_SUCCESS;
}

/**
 * \brief           I2C slave Interrupt
 */
void i2c_slave_handler(void) {
    uint32_t i2c_slave_status;
    uint8_t  temp;


    if (user_slave_cb == NULL) {
        /*serious bug... why? */
        while (1);
    }

    i2c_slave_status = I2C_SLAVE->i2c_slave_int_status;
    if (i2c_slave_status & I2C_SLAVE_BUS_TIMEOUT) {
        /* i2c slave device return to IDLE mode */
        i2c_slave_state = I2C_SLAVE_STATE_IDLE;
        user_slave_cb(I2C_SLAVE_STATUS_TIMEOUT);
        I2C_SLAVE->i2c_slave_int_status = I2C_SLAVE_BUS_TIMEOUT;
    } else {
        if (i2c_slave_status & I2C_SLAVE_STATUS_STOP) {
            /* 
             * i2c master finish i2c command --
             * notice: we don't report this is read or write stop.
             */
            i2c_slave_state = I2C_SLAVE_STATE_IDLE;
            /* notify I2C command end */
            user_slave_cb(I2C_SLAVE_STATUS_STOP);
            I2C_SLAVE->i2c_slave_int_status = I2C_SLAVE_STATUS_STOP;
        } else if (i2c_slave_status & I2C_SLAVE_MATCH_ADDR) {
            /* write 1 clear */
            I2C_SLAVE->i2c_slave_int_status = I2C_SLAVE_MATCH_ADDR; 

            /* check read operation or write operation (I2C Master viewpoint) */
            if (I2C_SLAVE->i2c_slave_status & I2C_SLAVE_READ_OP) {
                /*
                 * it is possible the two start combined condition without 
                 * no stop condition 
                 */
                i2c_slave_state = I2C_SLAVE_STATE_READ;
                /*
                 * notify I2C master a new request to read data
                 * user callback should sent data back to I2C master
                 */
                user_slave_cb(I2C_SLAVE_STATUS_READ | I2C_SLAVE_STATUS_ADDR_MATCH);
            } else {
                /*I2C_SLAVE_STATUS bit1 must be 0*/
                i2c_slave_state = I2C_SLAVE_STATE_WRITE;

                /*Dummy read rx to release SCL*/
                temp = I2C_SLAVE->i2c_slave_data;

                /*this is debug check --- it should be zero*/
                if ((temp & BIT0) == 1)
                {
                    user_slave_cb(I2C_SLAVE_STATUS_ERROR);
                }
                /* notify I2C master a new request to write data */
                user_slave_cb(I2C_SLAVE_STATUS_WRITE | I2C_SLAVE_STATUS_ADDR_MATCH);
            }
        } else if (i2c_slave_status & I2C_SLAVE_DATA_READY) {
            /*clear interrupt*/
            I2C_SLAVE->i2c_slave_int_status = I2C_SLAVE_DATA_READY;
            if (I2C_SLAVE->i2c_slave_status & I2C_SLAVE_READ_OP) {
                /* notify I2C master request to read data */
                user_slave_cb(I2C_SLAVE_STATUS_READ | I2C_SLAVE_STATUS_DATA_READY);
            } else {
                /* notify I2C master request to read data */
                user_slave_cb(I2C_SLAVE_STATUS_WRITE | I2C_SLAVE_STATUS_DATA_READY);    
            }

        } else {
            /* debug check --- fatal Error */
            user_slave_cb(I2C_SLAVE_STATUS_ERROR);
        }
    }
}
