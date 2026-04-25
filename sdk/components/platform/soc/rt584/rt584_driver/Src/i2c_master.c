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

#include "gpio.h"
#include "i2c_master.h"
#include "sysctrl.h"

/**
 * \brief           I2C software state definitions
 *                  Please do NOT change the order of these define
 */
#define I2C_STATE_UNINIT           0
#define I2C_STATE_IDLE             1
#define I2C_STATE_WRITE            2
#define I2C_STATE_WRITE_DATA       3
#define I2C_STATE_READ             4
#define I2C_STATE_READ_DATA        5
#define I2C_STATE_READ_DATA_STOP   6
/* BIT5 as stop MASK */
#define I2C_STOP_MASK             16     
#define I2C_STATE_NOACK           16
#define I2C_STATE_STOP            17
#define I2C_STATE_ESTOP           18
/* this mode only in i2c_init, avoid multi-task i2c_init at same time */
#define I2C_STATE_HOST_INIT       32

/**
 * \brief           I2C master max divice number
 */
#define  I2C_MASTER_NUMBER_MAX          2

/**
 * \brief           I2C clock speed devide definitions
 */
#define  HIGH_SPEED_DIV        (20-1)
#define  MEDIUM_SPEED_DIV      (40-1)
#define  NORMAL_SPEED_DIV      (80-1)
#define  SUPER_HIGH_SPEED_DIV  (10-1)

/**
 * \brief           I2C mmaster ode definitions
 */
#define  I2C_MODE_READ          1
#define  I2C_MODE_WRITE         0

/**
 * \brief           I2C max data packet size for one transfer
 */
#define  I2C_MAX_DATASIZE      (1024)       /*max data packet size for one transfer*/

/**
 * \brief i2c_master_handle_t save i2c master information
 */
typedef struct {
    i2c_master_t *i2c_hw_dev_master;            /*!< based hardware address */
    uint16_t     i2c_state;                     /*!< current i2c master state, is critical data */
    uint16_t     i2c_substate;                  /*!< current i2c master substate, is critical data */
    uint32_t     remain_data_length;            /*!< the maximum data length is 1K */
    uint32_t     next_read_data_length;         /*!< the data need to read */
    uint8_t      *pdata;                        /*!< save write/read data */
    i2cm_cb_fn   notify_cb;                     /*!< user callback */
    IRQn_Type    irq_num;                       /*!< IRQ Number */

} i2c_master_handle_t;

static i2c_master_handle_t  m_i2c_master_handle[I2C_MASTER_NUMBER_MAX] = {
    {
        I2C_MASTER0,
        I2C_STATE_UNINIT,
        0,
        0,
        0,
        NULL,
        NULL,
        I2C_Master0_IRQn
    },
    {
        I2C_MASTER1,
        I2C_STATE_UNINIT,
        0,
        0,
        0,
        NULL,
        NULL,
        I2C_Master1_IRQn
    }
};

static uint32_t i2c_check_state(uint32_t master_channel, uint16_t next_state) {
    i2c_master_handle_t *i2c_handle;

    if (master_channel >= I2C_MASTER_NUMBER_MAX) {
        return STATUS_INVALID_PARAM;
    }

    i2c_handle = &m_i2c_master_handle[master_channel];

    enter_critical_section();

    if ((i2c_handle->i2c_state) != I2C_STATE_IDLE) {
        leave_critical_section();
        if ((i2c_handle->i2c_state) != I2C_STATE_UNINIT) {
            return STATUS_EBUSY;                /* I2C host in read/write state... busy. */
        } else {
            return STATUS_NO_INIT;
        }
    }
    i2c_handle->i2c_state = next_state;         /* so other task can not read/write now. */

    leave_critical_section();

    return STATUS_SUCCESS;
}

uint32_t i2c_preinit(uint32_t master_id) {
    i2c_master_handle_t *i2c_handle;
    i2c_master_t *i2c_master;

    if (master_id >= I2C_MASTER_NUMBER_MAX) {
        return STATUS_INVALID_PARAM;
    }

    i2c_handle = &m_i2c_master_handle[master_id];
    i2c_master = i2c_handle->i2c_hw_dev_master;


    i2c_master->slck_gen = NORMAL_SPEED_DIV;     /* Bit15: Select APB clock (0) */
    i2c_master->control  = I2CM_CONTROL_ENABLE;
    
    i2c_master->control  = I2CM_CONTROL_ENABLE | I2CM_CONTROL_FIFO_CLEAR ;

    while (i2c_master->control & I2CM_CONTROL_FIFO_CLEAR) {}

    i2c_master->control  = I2CM_CONTROL_ENABLE | I2CM_CONTROL_BUS_CLEAR;

    while (i2c_master->control & I2CM_CONTROL_BUS_CLEAR) {}

    i2c_master->control = 0;

    return STATUS_SUCCESS;
}

uint32_t i2c_master_init(uint32_t master_id, uint32_t i2c_speed) {
    i2c_master_handle_t *i2c_handle;
    i2c_master_t *i2c_master;

    uint32_t div;

    if (master_id >= I2C_MASTER_NUMBER_MAX) {
        return STATUS_INVALID_PARAM;
    }

    i2c_handle = &m_i2c_master_handle[master_id];

    enter_critical_section();
    /* We can set i2c host only when i2c_state in idle/uninit mode */
    if ( (i2c_handle->i2c_state) > I2C_STATE_IDLE) {
        leave_critical_section();
        return STATUS_INVALID_REQUEST;
    }

    /*
     * i2c host in UNINIT or IDLE mode now
     * this is temp state, only avoid multi-task call i2c_init
     */
    i2c_handle->i2c_state = I2C_STATE_HOST_INIT;

    leave_critical_section();

    if (i2c_speed == I2C_CLOCK_400K) {
        div = HIGH_SPEED_DIV;
    } else if (i2c_speed == I2C_CLOCK_200K) {
        div = MEDIUM_SPEED_DIV;
    } else if (i2c_speed == I2C_CLOCK_800K) {
        div = SUPER_HIGH_SPEED_DIV;
    } else if (i2c_speed == I2C_CLOCK_1M) {
        div = 7;
    } else {
        div = NORMAL_SPEED_DIV;
    }

    i2c_master = i2c_handle->i2c_hw_dev_master;

    i2c_master->control = 0;

    if ( SYSCTRL->sys_clk_ctrl.bit.per_clk_sel == PERCLK_SEL_16M ) {
        /* peripheral clock 16M */
        i2c_master->slck_gen = div / 2;
    } else if ( SYSCTRL->sys_clk_ctrl.bit.per_clk_sel == PERCLK_SEL_RCO1M ) {
        /* peripheral clock 1M */
        i2c_master->slck_gen = 3;
    } else {
        /* at peripheral clock 32M */
        i2c_master->slck_gen = div;
    }

    i2c_master->int_enable = 0;

    /* clear I2C interrupt status register */
    i2c_master->int_clear = I2CM_INT_MASK_ALL;

    enter_critical_section();
    /* OK, right now we can use I2C host to transfer data */
    i2c_handle->i2c_state = I2C_STATE_IDLE;
    leave_critical_section();

    return STATUS_SUCCESS;
}

uint32_t i2c_master_write(uint32_t master_channel,
                          const i2c_master_mode_t *slave,
                          uint8_t *data,
                          uint32_t len) {
    i2c_master_handle_t *i2c_handle;
    i2c_master_t *i2c_master;

    uint32_t status, avalible_fifos;

    if (master_channel >= I2C_MASTER_NUMBER_MAX) {
        return STATUS_INVALID_PARAM;
    }

    /* slave data should not be NULL */
    assert_param(slave);
    /* data should not be NULL */
    assert_param(data);

    if (len > I2C_MAX_DATASIZE) {
        /* packet data is > 1K bytes... */
        return STATUS_INVALID_PARAM;
    }

    if (slave->endproc_cb == NULL) {
        /* No I2C complete callback function. */
        return STATUS_INVALID_PARAM;
    }

    status = i2c_check_state(master_channel, I2C_STATE_WRITE);

    if (status) {
        return status;
    }

    i2c_handle = &m_i2c_master_handle[master_channel];
    i2c_master = i2c_handle->i2c_hw_dev_master;

    i2c_handle->pdata = data;
    i2c_handle->remain_data_length = len;
    i2c_handle->notify_cb = slave->endproc_cb;

    i2c_handle->i2c_substate = I2C_STATE_WRITE_DATA;

    i2c_master = i2c_handle->i2c_hw_dev_master;

    /* Here Command FIFO should be empty. */
    if (!( (i2c_master->int_raw_status) & I2CM_INT_TX_EMPTY)) {
#ifdef  DEBUG
        printf("Warning Check why I2C CMDFIFO is not empty \n");
#endif
    }

    i2c_master->tar = slave->dev_addr;

    /* 
     * In fact, available fifo is 16 bytes..
     * It always has at least one byte for address register,
     * so we decrease it first.
     */
    avalible_fifos = I2CM_FIFO_NUM - 1;

    /* Write data to I2C device */
    if (slave->bFlag_16bits) {
        avalible_fifos--;
        i2c_master->buf = (slave->reg_addr >> 8);
    }

    i2c_master->buf = (slave->reg_addr & 0xFF);

    if (avalible_fifos > len) {
        avalible_fifos = len;
        i2c_handle->remain_data_length = 0;
    } else {
        i2c_handle->remain_data_length = len - avalible_fifos;
    }

    while (avalible_fifos > 0) {
        i2c_master->buf = *(i2c_handle->pdata);
        i2c_handle->pdata++;
        avalible_fifos--;
    }

    /* enable interrupt of I2CM_INT_TX_EMPTY/I2CM_INT_ADDR_NACK/I2CM_INT_WRITE_NACK */
    i2c_master->int_enable = (I2CM_INT_TX_OVER | I2CM_INT_TX_EMPTY 
                            | I2CM_INT_ADDR_NACK | I2CM_INT_WRITE_NACK | I2CM_INT_LOSTARB);

    if (i2c_handle->remain_data_length) {
        /* Enable I2C controller. */
        i2c_master->control = I2CM_CONTROL_ENABLE;
    } else {
        /* all data already in fifo. */
        i2c_handle->i2c_substate = I2C_STATE_STOP;
        i2c_master->control = I2CM_CONTROL_ENABLE | I2CM_CONTROL_STOP_EN;
    }

    return STATUS_SUCCESS;
}

uint32_t i2c_master_read(uint32_t master_channel,
                         const i2c_master_mode_t  *slave,
                         uint8_t  *data,
                         uint32_t len) {
    i2c_master_handle_t* i2c_handle;
    i2c_master_t* i2c_master;

    uint32_t status, avalible_fifos;

    if (master_channel >= I2C_MASTER_NUMBER_MAX) {
        return STATUS_INVALID_PARAM;
    }

    /* slave data should not be NULL */
    assert_param(slave);
    /* data should not be NULL */
    assert_param(data);

    if ((len > I2C_MAX_DATASIZE) || (len == 0)) {
        /* packet data is > 1K bytes. or zero bytes */
        return STATUS_INVALID_PARAM;
    }

    if (slave->endproc_cb == NULL) {
        /* I2C complete function must exist */
        return STATUS_INVALID_PARAM;
    }

    status = i2c_check_state(master_channel, I2C_STATE_READ);

    if (status) {
        return status;
    }

    i2c_handle = &m_i2c_master_handle[master_channel];
    i2c_master = i2c_handle->i2c_hw_dev_master;

    i2c_handle->pdata = data;

    i2c_handle->remain_data_length = len;

    i2c_handle->notify_cb = slave->endproc_cb;

    i2c_handle->i2c_substate = I2C_STATE_READ_DATA;

    /* Here Command FIFO should be empty. */
    if (!(i2c_master->int_raw_status & I2CM_INT_TX_EMPTY)) {
#ifdef  DEBUG
        printf("Warning Check why I2C CMDFIFO is not empty \n");
#endif
    }

    i2c_master->tar = slave->dev_addr ;

    /* 
     * In fact, available fifo is 16 bytes..
     * It always has at least one byte for address register,
     * so we decrease it first.
     */
    avalible_fifos = I2CM_FIFO_NUM - 1;

    /* Write register data to I2C device */
    if (slave->bFlag_16bits) {
        avalible_fifos--;
        i2c_master->buf = (slave->reg_addr >> 8);
    }

    i2c_master->buf = (slave->reg_addr & 0xFF);

    if (avalible_fifos > len) {
        avalible_fifos = len;
    }

    i2c_handle->next_read_data_length = avalible_fifos;
    i2c_handle->remain_data_length -= avalible_fifos;

    /* read data */
    while (avalible_fifos > 0) {
        i2c_master->buf = 0x100;
        avalible_fifos--;
    }

    /* enable interrupt of I2CM_INT_RX_FIINISH/I2CM_INT_ADDR_NACK/I2CM_INT_WRITE_NACK */
    i2c_master->int_enable = (I2CM_INT_RX_FINISH | I2CM_INT_ADDR_NACK 
                            | I2CM_INT_WRITE_NACK);

    if (i2c_handle->remain_data_length) {
        /* Enable I2C controller. */
        i2c_master->control = I2CM_CONTROL_ENABLE | I2CM_CONTROL_RESTART;
    }
    else
    {
        /* 
         * all data already in fifo.
         * i2c_substate = I2C_STATE_READ_DATA_STOP; 
         */
        i2c_master->control = I2CM_CONTROL_ENABLE | I2CM_CONTROL_STOP_EN | I2CM_CONTROL_RESTART;
    }

    return STATUS_SUCCESS;
}

/**
 * \brief           I2C Master Interrupt
 */
static void i2c_master_handler(i2c_master_handle_t* handle) {
    i2c_master_t* i2c_master;
    uint32_t status, ret_status, avalible_fifos;
    volatile uint32_t delay;

    i2c_master = handle->i2c_hw_dev_master;

    /* read interrupt status and clear. */
    status = i2c_master->int_status;
    i2c_master->int_clear = status;

    /* Disable all interrupt */
    i2c_master->int_enable = 0;

    if (status & I2CM_INT_IDLE) {
        handle->i2c_state = I2C_STATE_IDLE;
        ret_status = I2C_STATUS_OK;

        if (handle->i2c_substate == I2C_STATE_READ_DATA_STOP) {
            /* this is read end */
            while (handle->next_read_data_length > 0) {
                /* read data */
                *(handle->pdata) = i2c_master->buf;
                handle->pdata++;
                handle->next_read_data_length--;
            }

            /* debug check */
            if (handle->remain_data_length != 0) {
                /* debug check */
#ifdef  DEBUG
                printf("\t check why remain_data_length!=0 \n");
#endif
            }
        } else if ((handle->i2c_substate) == I2C_STATE_STOP) {
            /* this is write end. */
        } else {
            /* this is error NACK end */
            ret_status = I2C_STATUS_ERR_NOACK;
        }

        /* check Stop_EN should be clear. this check is clear very soon. */
        while ((i2c_master->control) & I2CM_CONTROL_STOP_EN) {}

        i2c_master->control = 0;

        handle->notify_cb(ret_status);
        return;
    }


    if (status & (I2CM_INT_ADDR_NACK | I2CM_INT_WRITE_NACK)) {
        /* 
         * error. no address device or write NACK (device is busy).
         * stop to send/receive any data. because error
         */
        handle->remain_data_length = 0;         
        handle->i2c_substate = I2C_STATE_ESTOP;

        /*
         * stop will auto see when controller see NACK error
         * but current stop happened?
         */

        /*clear FIFO*/
        i2c_master->control = (I2CM_CONTROL_ENABLE | I2CM_CONTROL_FIFO_CLEAR);

        while ((i2c_master->control) & I2CM_CONTROL_FIFO_CLEAR) {}
        /*wait this bit clear.*/

        /* 
         * i2c_idle ... this is interrupt ISR..
         *  so it does NOT worry about critical section
         */

        if ((i2c_master->int_raw_status) & I2CM_INT_IDLE) {
            /* if we are in idle we will close I2C */
            i2c_master->control = 0;      
            handle->i2c_state = I2C_STATE_IDLE;
            ret_status = I2C_STATUS_ERR_NOACK;
            handle->notify_cb(ret_status);
        } else {
            handle->i2c_substate = I2C_STATE_ESTOP;
            i2c_master->int_enable = I2CM_INT_IDLE;
        }
        return;
    }


    if (status & I2CM_INT_TX_EMPTY) {
        /* this must be write state. */

        /* write command */
        avalible_fifos = I2CM_FIFO_NUM;

        if (avalible_fifos > handle->remain_data_length) {
            avalible_fifos = handle->remain_data_length;
            handle->remain_data_length = 0;
        } else {
            handle->remain_data_length -= avalible_fifos;
        }

        while (avalible_fifos > 0) {
            i2c_master->buf = *(handle->pdata);
            handle->pdata++;
            avalible_fifos--;
        }

        if (handle->remain_data_length) {
            i2c_master->int_enable =  (I2CM_INT_TX_EMPTY | I2CM_INT_WRITE_NACK);
            i2c_master->control = I2CM_CONTROL_ENABLE;
        } else {
            /* all data already in fifo. */
            handle->i2c_substate = I2C_STATE_STOP;

            i2c_master->int_enable =  (I2CM_INT_IDLE | I2CM_INT_WRITE_NACK);
            i2c_master->control = I2CM_CONTROL_ENABLE | I2CM_CONTROL_STOP_EN;
        }
    }

    if (status & I2CM_INT_RX_FINISH) {
        /* this must be read state */

        while (handle->next_read_data_length > 0) {
            /* read data */
            *(handle->pdata) = i2c_master->buf;       
            handle->pdata++;
            handle->next_read_data_length--;
        }

        /* read next */
        avalible_fifos = I2CM_FIFO_NUM;

        if (avalible_fifos > handle->remain_data_length) {
            avalible_fifos = handle->remain_data_length;
        }

        handle->next_read_data_length = avalible_fifos;
        handle->remain_data_length -= avalible_fifos;

        /* read data */
        while (avalible_fifos > 0) {
            i2c_master->buf = 0x100;
            avalible_fifos--;
        }

        if (handle->remain_data_length) {
            /* 
             * Enable I2C controller.
             * enable interrupt of I2CM_INT_RX_FINISH/I2CM_INT_ADDR_NACK/I2CM_INT_WRITE_NACK
             */
            i2c_master->int_enable = (I2CM_INT_RX_FINISH | I2CM_INT_ADDR_NACK | I2CM_INT_WRITE_NACK);
            i2c_master->control = I2CM_CONTROL_ENABLE | I2CM_CONTROL_RESTART;
        } else {
            /* all data already in fifo. */
            handle->i2c_substate = I2C_STATE_READ_DATA_STOP;
            /* enable interrupt of I2CM_INT_TX_EMPTY/I2CM_INT_ADDR_NACK/I2CM_INT_WRITE_NACK */
            i2c_master->int_enable = (I2CM_INT_IDLE | I2CM_INT_ADDR_NACK | I2CM_INT_WRITE_NACK);
            i2c_master->control = I2CM_CONTROL_ENABLE | I2CM_CONTROL_STOP_EN | I2CM_CONTROL_RESTART;
        }
    }
    return;
}

/**
 * \brief           I2C Master 0 Interrupt
 */
void i2c_master0_handler(void) {
    i2c_master_handler(&m_i2c_master_handle[0]);
}

/**
 * \brief           I2C Master 1 Interrupt
 */
void i2c_master1_handler(void) {
    i2c_master_handler(&m_i2c_master_handle[1]);
}
