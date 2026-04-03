/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2c_master.c
 * \brief           i2c master driver file
 */
/*
 * Author:          Kc.tseng
 */
#include <stdio.h>
#include <string.h>
#include "mcu.h"
#include "uart_stdio.h"

/**
 * \brief           I2C software state definitions
 */
#define I2C_STATE_UNINIT     0
#define I2C_STATE_IDLE       1
#define I2C_STATE_WRITE      2
#define I2C_STATE_WRITE_DATA 3
#define I2C_STATE_READ       4
#define I2C_STATE_READ_WADDR 5
#define I2C_STATE_READ_RDATA 6
/* BIT5 as stop MASK */
#define I2C_STOP_MASK   16
#define I2C_STATE_NOACK 16
#define I2C_STATE_STOP  17
#define I2C_STATE_ESTOP 18
/* this mode only in i2c_init, avoid multi-task i2c_init at same time */
#define I2C_STATE_HOST_INIT 32

/**
 * \brief           I2C clock speed devide definitions
 */
#define HIGH_SPEED_DIV       (20 - 1)
#define MEDIUM_SPEED_DIV     (40 - 1)
#define NORMAL_SPEED_DIV     (80 - 1)
#define SUPER_HIGH_SPEED_DIV (10 - 1)

/**
 * \brief           I2C mode definitions
 */
#define I2C_MODE_READ  1
#define I2C_MODE_WRITE 0

/**
 * \brief           I2C max data packet size for one transfer
 */
#define I2C_MAX_DATASIZE (1024)

/*
 * i2c_state is critical data
 * current i2c host state
 */
static uint16_t i2c_state;
static uint16_t i2c_substate;

/*
 * local data used in interrupt service routine
 * the maximum data length is 1K
*/
static uint16_t remain_data_length;
static uint8_t* pdata;
static i2cm_cb_fn notify_cb;
static i2c_master_mode_t read_slave;

uint32_t i2c_preinit(void) {
    uint32_t times = 0;

    /* clear I2C FIFO */
    I2CM->status = I2CM_STATUS_FIFO_CLR;

    while (I2CM->status & I2CM_STATUS_FIFO_CLR) {}

    /* I2C disable first */
    I2CM->control = 0;

    i2c_state = I2C_STATE_UNINIT;

    return STATUS_SUCCESS;
}

uint32_t i2c_init(uint32_t i2c_speed) {
    uint32_t div;

    enter_critical_section();
    /* We can set i2c host only when i2c_state in idle/uninit mode */
    if (i2c_state > I2C_STATE_IDLE) {
        leave_critical_section();
        return STATUS_INVALID_REQUEST;
    }

    /*
     * i2c host in UNINIT or IDLE mode now
     * this is temp state, only avoid multi-task call i2c_init
     */
    i2c_state = I2C_STATE_HOST_INIT;

    leave_critical_section();

    if (i2c_speed == I2C_CLOCK_400K) {
        div = HIGH_SPEED_DIV;
    } else if (i2c_speed == I2C_CLOCK_200K) {
        div = MEDIUM_SPEED_DIV;
    } else if (i2c_speed == I2C_CLOCK_800K) {
        div = SUPER_HIGH_SPEED_DIV;
    } else {
        /* I2C default is 100K */
        div = NORMAL_SPEED_DIV;
    }

    I2CM->prescale_h = (div >> I2CM_CLK_PRESCALE_SHIFT);
    /* I2C Master use external clock */
    I2CM->control = (div & I2CM_CTRL_CLK_DIV_MASK) | I2CM_CTRL_CLK_SRC_EXT;

    /*
     * Notice: We don't enable I2C right now..
     * We will enable it in read/write transfer
     */
    I2CM->int_enable = 0;

    /* clear I2C interrupt status register */
    I2CM->int_status = 0xF;

    enter_critical_section();
    i2c_state = I2C_STATE_IDLE;
    leave_critical_section();

    return STATUS_SUCCESS;
}

static uint32_t i2c_check_state(uint16_t next_state) {
    enter_critical_section();

    if (i2c_state != I2C_STATE_IDLE) {
        leave_critical_section();

        if (i2c_state != I2C_STATE_UNINIT) {
            return STATUS_EBUSY;
        } else {
            return STATUS_NO_INIT;
        }
    }
    /* so other task can not read/write now. */
    i2c_state = next_state;

    leave_critical_section();
    return STATUS_SUCCESS;
}

static void i2c_write_addr_reg(const i2c_master_mode_t* slave,
                               uint32_t bReadFlag) {
    I2CM->command = I2CM_CMD_STRT; /*send Start condition*/
    I2CM->command = I2CM_CMD_WDAT8;

    if (bReadFlag) {
        /* I2C read bit8 is 1 */
        I2CM->command = (slave->dev_addr << 1) | 1;
    } else {
        /* I2C write bit8 is 0 */
        I2CM->command = (slave->dev_addr << 1);
    }

    I2CM->command = I2CM_CMD_VACK;

    if (bReadFlag) {
        /* Read operation do NOT have register address */
        return;
    }

    if (slave->bFlag_16bits) {
        /* If slave device need 16bits register address */
        I2CM->command = I2CM_CMD_WDAT8;
        /* High register address */
        I2CM->command = (slave->reg_addr >> 8);
        I2CM->command = I2CM_CMD_VACK;
    }

    I2CM->command = I2CM_CMD_WDAT8;
    /* Low register address */
    I2CM->command = (slave->reg_addr & 0xFF);
    I2CM->command = I2CM_CMD_VACK;

    return;
}

uint32_t i2c_write(const i2c_master_mode_t* slave, uint8_t* data, uint32_t len,
                   i2cm_cb_fn i2c_usr_isr) {
    uint32_t status;

    /* slave data should not be NULL */
    assert_param(slave);
    /* data should not be NULL */
    assert_param(data);
    /* endproc_cb hook should not be NULL */
    assert_param(i2c_usr_isr);

    if (len > I2C_MAX_DATASIZE) {
        /* packet data is > 1K bytes */
        return STATUS_INVALID_PARAM;
    }

    status = i2c_check_state(I2C_STATE_WRITE);

    if (status) {
        return status;
    }

    pdata = data;
    remain_data_length = len;
    notify_cb = i2c_usr_isr;

    /* Here Command FIFO should be empty */
    if (!(I2CM->status & I2CM_STATUS_CMDFIFO_EMPTY)) {
        puts("Warning Check why I2C CMDFIFO is not empty \n");
    }

    i2c_write_addr_reg(slave, I2C_MODE_WRITE);
    i2c_substate = I2C_STATE_WRITE_DATA;
    /* Enable I2C_ENABLE bit in control register */
    I2CM->control |= I2CM_CTRL_ENABLE;
    /* enable interrupt */
    I2CM->int_enable = (I2CM_INT_CMDFIFO_EMPTY | I2CM_INT_ERROR);

    return STATUS_SUCCESS;
}

uint32_t i2c_read(const i2c_master_mode_t* slave, uint8_t* data, uint32_t len,
                  i2cm_cb_fn i2c_usr_isr) {
    uint32_t status;

    /* slave data should not be NULL */
    assert_param(slave);
    /* data should not be NULL */
    assert_param(data);
    /* endproc_cb hook should not be NULL */
    assert_param(i2c_usr_isr);

    if ((len > I2C_MAX_DATASIZE) || (len == 0)) {
        /* packet data is > 1K bytes. or zero bytes? */
        return STATUS_INVALID_PARAM;
    }
    //printf("i2c_read data:%.8x \r\n", data);
    status = i2c_check_state(I2C_STATE_READ);

    if (status) {
        return status;
    }

    pdata = data;
    remain_data_length = len;
    notify_cb = i2c_usr_isr;

    /* Here Command FIFO should be empty */
    if (!(I2CM->status & I2CM_STATUS_CMDFIFO_EMPTY)) {
        //puts("Warning Check why I2C CMDFIFO is not empty \n");
    }
    read_slave.bFlag_16bits = slave->bFlag_16bits;
    read_slave.dev_addr = slave->dev_addr;
    read_slave.reg_addr = slave->reg_addr;

    i2c_substate = I2C_STATE_READ_WADDR;

    i2c_write_addr_reg(slave, I2C_MODE_WRITE);

    /* Enable I2C_ENABLE bit in control register */
    I2CM->control |= I2CM_CTRL_ENABLE;

    /*
     * enable interrupt, we don't enable DATAREADY interrupt
     * but we will check it in ISR when command fifo empty.
     * Becaue we don't want too many interrupts happen.
     */
    I2CM->int_enable = (I2CM_INT_CMDFIFO_EMPTY | I2CM_INT_ERROR);

    return STATUS_SUCCESS;
}

static void i2c_idle(void) {
    enter_critical_section();
    i2c_state = I2C_STATE_IDLE;
    leave_critical_section();
    I2CM->int_enable = 0;
    /*
     * Notice: IF you in read operation, I2CM->STATUS
     * will get 0x5 for the end transfer, but bit2 is not
     * error, it just "NAK" that host send.
     */
}

/**
 * \brief           I2C Master Interrupt
 */
void i2cm_handler(void) {
    uint32_t status;

    status = I2CM->int_status;
    I2CM->int_status = status;

    if (status & I2CM_INT_ERROR) {
        /*
         * this is highest priority --- No ACK
         * in most case, this error should not happen
         */

        if (!(I2CM->status & I2CM_STATUS_CMDFIFO_EMPTY)) {
            /* we should clear FIFO when error */
            I2CM->status = (I2CM_STATUS_I2C_ERR_CLEAR | I2CM_STATUS_FIFO_CLR);
            /* We don't do FIFO clear check.. because it is very fast */
        }

        if (i2c_substate < I2C_STATE_NOACK) {
            /*
             * we can not send CMD_STOP here.. because it be influenced by FIFO_CLR
             * stop to send/receive any data. because error
             */
            remain_data_length = 0;
            i2c_substate = I2C_STATE_NOACK;
        }
    }

    /* check data in recv fifo... read phase */
    while (I2CM->status & I2CM_STATUS_DATA_READY) {
        /*
         * Only i2c_substate I2C_STATE_READ_RDATA or I2C_STATE_STOP (the last)
         * will enter this read loop
         * read one data in Recv FIFO
         */
        *pdata++ = I2CM->rd_data;

        /*
         * then try to send command for next one byte
         * If required.
         */
        if (remain_data_length) {
            if (I2CM->status & I2CM_STATUS_CMDFIFO_OVERFL) {
                /* this is check status... should not happen */
                puts("Error CMDFIFO overflow \n");
            }

            I2CM->command = I2CM_CMD_RDAT8;

            remain_data_length--;
            if (remain_data_length) {
                I2CM->command = I2CM_CMD_WDAT0;
            } else {
                I2CM->command = I2CM_CMD_WDAT1;
                /* the last one byte, so we add stop condition */
                I2CM->command = I2CM_CMD_STOP;
                i2c_substate = I2C_STATE_STOP;
            }
        }
    }

    if (status & I2CM_INT_CMDFIFO_EMPTY) {
        /*command fifo empty, ok we can stuff next data*/

        if (I2CM->status & I2CM_STATUS_CMDFIFO_EMPTY) {
            if (i2c_substate == I2C_STATE_WRITE_DATA) {
                if (remain_data_length > 0) {
                    int i;
                    /* we will write 8 bytes into FIFO */
                    for (i = 0; i < 8; ++i) {

                        I2CM->command = I2CM_CMD_WDAT8;
                        I2CM->command = *pdata++;
                        I2CM->command = I2CM_CMD_VACK;

                        remain_data_length--;

                        if (remain_data_length == 0) {
                            /* send stop to terminate the transfer. */
                            I2CM->command = I2CM_CMD_STOP;
                            i2c_substate = I2C_STATE_STOP;
                            break;
                        }
                    }
                }
            } else if (i2c_substate == I2C_STATE_READ_WADDR) {
                i2c_substate = I2C_STATE_READ_RDATA;
                i2c_write_addr_reg(&read_slave, I2C_MODE_READ);

                /* send read data request */
                if (remain_data_length > 0) {
                    int i;
                    /* we will read maximum 12 bytes into FIFO */
                    for (i = 0; i < 12; i++) {
                        I2CM->command = I2CM_CMD_RDAT8;

                        remain_data_length--;
                        if (remain_data_length) {
                            I2CM->command = I2CM_CMD_WDAT0;
                        } else {
                            I2CM->command = I2CM_CMD_WDAT1;
                            /* the last one byte, so we add stop condition. */
                            I2CM->command = I2CM_CMD_STOP;
                            i2c_substate = I2C_STATE_STOP;
                            break;
                        }
                    }
                }
            } else if (i2c_substate & I2C_STOP_MASK) {
                /* send stop for NOACK case... this is error case. */
                if (i2c_substate == I2C_STATE_NOACK) {
                    /* send stop to terminate the transfer. */
                    I2CM->command = I2CM_CMD_STOP;
                    /* avoid to send CMD_STOP TWICE. (next cmd fifo empty, don't enter here) */
                    i2c_substate = I2C_STATE_ESTOP;
                }
                /*we will want to know STOP send finish*/
                I2CM->int_enable = I2CM_INT_COMPLETE;
            }
        } else {
            /*
             * Because we stuff some command into fifo, like
             * CMD_STOP to fifo. so we see this state.
             * Ignore this case.
             */
            return;
        }
    }

    if (i2c_substate >= I2C_STATE_STOP) {
        /* only need to know the last STOP condition in BUS? */
        if (!(I2CM->status & I2CM_STATUS_CMDFIFO_EMPTY)) {
            return; /*STOP still in command FIFO*/
        }

        if (status & I2CM_INT_COMPLETE) {
            uint32_t ret_status;

            i2c_idle();
            if (i2c_substate == I2C_STATE_STOP) {
                ret_status = I2C_STATUS_OK;
            } else {
                ret_status = I2C_STATUS_ERR_NOACK;
            }
            /* let application hook to know error */
            notify_cb(ret_status);
        }
    }

    return;
}
