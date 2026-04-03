/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_qspi.c
 * \brief           Hosal QSPI driver file
 */
/*
 * Author:          
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "hosal_common.h"
#include "hosal_qspi.h"
#include "mcu.h"
#include "stdio.h"
#include "string.h"
#include "task.h"
#include "hosal_status.h"

#define HOSAL_QSPI_CHECK(err, condi)                                           \
    {                                                                          \
        if (condi)                                                             \
            return err;                                                        \
    }

static volatile hosal_qspi_dev_t g_qspi_handle[MAX_NUMBER_OF_QSPI];

static hosal_qspi_status_t hosal_qspi_mode_set(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg);
static hosal_qspi_status_t hosal_qspi_mode_get(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg);
static hosal_qspi_status_t hosal_qspi_baud_set(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg);
static hosal_qspi_status_t hosal_qspi_baud_get(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg);
static hosal_qspi_status_t hosal_qspi_datawidth_set(hosal_qspi_dev_t* qspi_dev,
                                                    void* p_arg);
static hosal_qspi_status_t hosal_qspi_datawidth_get(hosal_qspi_dev_t* qspi_dev,
                                                    void* p_arg);
static hosal_qspi_status_t hosal_qspi_bitorder_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);
static hosal_qspi_status_t hosal_qspi_bitorder_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);
static hosal_qspi_status_t hosal_qspi_phase_set(hosal_qspi_dev_t* qspi_dev,
                                                void* p_arg);
static hosal_qspi_status_t hosal_qspi_phase_get(hosal_qspi_dev_t* qspi_dev,
                                                void* p_arg);
static hosal_qspi_status_t hosal_qspi_polarity_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);
static hosal_qspi_status_t hosal_qspi_polarity_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);
static hosal_qspi_status_t hosal_qspi_slave_select_set(hosal_qspi_dev_t* qspi_dev,
                                                       void* p_arg);
static hosal_qspi_status_t hosal_qspi_slave_select_get(hosal_qspi_dev_t* qspi_dev,
                                                       void* p_arg);
static hosal_qspi_status_t hosal_qspi_slave_polarity_set(hosal_qspi_dev_t* qspi_dev,
                                                         void* p_arg);
static hosal_qspi_status_t hosal_qspi_slave_polarity_get(hosal_qspi_dev_t* qspi_dev,
                                                         void* p_arg);

static hosal_qspi_dev_dataset hosal_qspi_dataset_access[] = {
    [HOSAL_QSPI_DATAWIDTH_SET] = hosal_qspi_datawidth_set,
    [HOSAL_QSPI_DATAWIDTH_GET] = hosal_qspi_datawidth_get,
    [HOSAL_QSPI_BITORDER_SET] = hosal_qspi_bitorder_set,
    [HOSAL_QSPI_BITORDER_GET] = hosal_qspi_bitorder_get,
    [HOSAL_QSPI_PHASE_SET] = hosal_qspi_phase_set,
    [HOSAL_QSPI_PHASE_GET] = hosal_qspi_phase_get,
    [HOSAL_QSPI_POLARITY_SET] = hosal_qspi_polarity_set,
    [HOSAL_QSPI_POLARITY_GET] = hosal_qspi_polarity_get,
    [HOSAL_QSPI_SLAVESELECT_SET] = hosal_qspi_slave_select_set,
    [HOSAL_QSPI_SLAVESELECT_GET] = hosal_qspi_slave_select_get,
    [HOSAL_QSPI_SLAVE_POLARTITY_SET] = hosal_qspi_slave_polarity_set,
    [HOSAL_QSPI_SLAVE_POLARTITY_GET] = hosal_qspi_slave_polarity_get,
    [HOSAL_QSPI_BAUD_SET] = hosal_qspi_baud_set,
    [HOSAL_QSPI_BAUD_GET] = hosal_qspi_baud_get,
    [HOSAL_QSPI_MODE_SET] = hosal_qspi_mode_set,
    [HOSAL_QSPI_MODE_GET] = hosal_qspi_mode_get,
};

static hosal_qspi_status_t hosal_qspi_mode_set(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    qspi_dev->config.mode = *(hosal_qspi_mode_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_mode_get(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    *(hosal_qspi_mode_t*)p_arg = qspi_dev->config.mode;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_baud_set(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    qspi_dev->config.baud_rate = *(hosal_qspi_baudrate_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_baud_get(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    *(hosal_qspi_baudrate_t*)p_arg = qspi_dev->config.baud_rate;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_datawidth_set(hosal_qspi_dev_t* qspi_dev,
                                                    void* p_arg) {
    qspi_dev->config.data_width = *(hosal_qspi_bitsize_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_datawidth_get(hosal_qspi_dev_t* qspi_dev,
                                                    void* p_arg) {
    *(hosal_qspi_bitsize_t*)p_arg = qspi_dev->config.data_width;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_bitorder_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    qspi_dev->config.bit_order = *(hosal_qspi_bitorder_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_bitorder_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    *(hosal_qspi_bitorder_t*)p_arg = qspi_dev->config.bit_order;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_phase_set(hosal_qspi_dev_t* qspi_dev,
                                                void* p_arg) {
    qspi_dev->config.phase = *(hosal_qspi_phase_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_phase_get(hosal_qspi_dev_t* qspi_dev,
                                                void* p_arg) {
    *(hosal_qspi_phase_t*)p_arg = qspi_dev->config.phase;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_polarity_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    qspi_dev->config.polarity = *(hosal_qspi_polarity_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_polarity_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    *(hosal_qspi_polarity_t*)p_arg = qspi_dev->config.polarity;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t
hosal_qspi_slave_select_set(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    qspi_dev->config.slave_select = *(hosal_qspi_slave_select_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t
hosal_qspi_slave_select_get(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    *(hosal_qspi_slave_select_t*)p_arg = qspi_dev->config.slave_select;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t
hosal_qspi_slave_polarity_set(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    qspi_dev->config.cs_polarity = *(hosal_qspi_cs_polarity_t*)p_arg;
    return HOSAL_STATUS_SUCCESS;
}

static hosal_qspi_status_t
hosal_qspi_slave_polarity_get(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    *(hosal_qspi_cs_polarity_t*)p_arg = qspi_dev->config.cs_polarity;
    return HOSAL_STATUS_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_deinit(hosal_qspi_dev_t* qspi_dev) {
    return HOSAL_STATUS_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_init(hosal_qspi_dev_t* qspi_dev) {
    hosal_qspi_status_t err = HOSAL_STATUS_SUCCESS;
    uint32_t control_reg = 0;
    uint32_t qspi_id;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    qspi_id = (id == 0) ? MODE_QSPI0 : MODE_QSPI1;
    qspi_dev->irq_num = (id == 0) ? Qspi0_IRQn : Qspi1_IRQn;
    qspi_dev->config.qspi_id = id;

    memcpy(ptr, qspi_dev, sizeof(hosal_qspi_dev_t));
    pin_set_mode(ptr->config.clk_pin, qspi_id);  /* SPI SCLK */
    pin_set_mode(ptr->config.cs_pin, qspi_id);   /* SPI CS */
    pin_set_mode(ptr->config.mosi_pin, qspi_id); /* SPI DATA0 */
    pin_set_mode(ptr->config.miso_pin, qspi_id); /* SPI DATA1 */
    if (ptr->config.data2 != 0)
        pin_set_mode(ptr->config.data2, qspi_id); /* SPI DATA2 */
    if (ptr->config.data3 != 0)
        pin_set_mode(ptr->config.data3, qspi_id); /* SPI DATA3 */

    ptr->qspi_state = HOSAL_QSPI_STATE_SETUP;
    enable_perclk((QSPI0_BASE_CLK + ptr->config.qspi_id));

    ptr->instance->qspi_clkdiv = ((ptr->config.baud_rate == QSPI_CLK_32M)
                                  && ((SYSCTRL->sys_clk_ctrl & HCLK_SEL_MASK)
                                      == HCLK_SEL_32M)
                                  && (ptr->config.baud_rate == SPI_SLAVE_MODE))
                                     ? QSPI_CLK_16M
                                     : ptr->config.baud_rate;

    ptr->instance->qspi_en = 0; /* Clear FIFO */

    /* Read Enable Register, it should be 0 before next step */
    while (ptr->instance->qspi_en) {}

    ptr->instance->qspi_int_clr = 0xFF; /* Clear all interrupt flag */
    ptr->instance->qspi_int_en = 0x0;   /* Disable all interrupt */

#if defined(SUPPORT_QSPI0_MULTI_CS)
    /* Set qspi device SS pol --- CS active High or active Low setting */
    ptr->instance->qspi_slave_sel_pol =
        ((ptr->config.cs_polarity & ~(1 << ptr->config.slave_select))
         | (ptr->config.cs_polarity << ptr->config.slave_select));
#else
    /* Set qspi device SS pol --- only one spi device. so ignore other chip select */
    ptr->instance->qspi_slave_sel_pol = (ptr->config.cs_polarity
                                         << ptr->config.slave_select);
#endif

    ptr->instance->qspi_slave_sel = (1 << ptr->config.slave_select);

    /* Set qspi transfer mode, here we assume bitsize is 8 bit first, Normal SPI mode */
    ptr->instance->qspi_aux = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI
                              | QSPI_DISABLE_IN;

    /* Clear native DMA register */
    ptr->instance->dma_ier = 0;
    /* Clear all interrupt */
    ptr->instance->dma_int_status = QSPI_DMA_ISR_CLEARALL;
    ptr->instance->dma_rx_enable = 0;
    ptr->instance->dma_tx_enable = 0;

    /* Enable DMA RX interrupt */
    ptr->instance->dma_ier = QSPI_DMA_IER_RX;

    /* For native SPI DMA, watermark is 8 entry only */
    control_reg = QSPI_CNTL_TX_1_4_WATERMARK | QSPI_CNTL_RX_1_4_WATERMARK
                  | (ptr->config.mode << 5) | (ptr->config.polarity << 4)
                  | (ptr->config.phase << 3) | (ptr->config.bit_order << 2)
                  | QSPI_CNTL_LITTLE_ENDIAN;

    /* This is for normal SPI slave setting */
    ptr->instance->qspi_control = control_reg
                                  | ((ptr->config.mode)
                                         ? QSPI_CNTL_contXfer_En
                                         : SPI_CNTL_SLAVE_SDATA_OUT);

    /* Enable ptr->instance */
    ptr->instance->qspi_en = 1;
    ptr->qspi_state = HOSAL_QSPI_STATE_IDLE;

    return err;
}

hosal_qspi_status_t hosal_qspi_transfer_dma(hosal_qspi_dev_t* qspi_dev,
                                            uint8_t* txbuf, uint8_t* rxbuf,
                                            uint16_t size) {
    if (size == 0 || qspi_dev == NULL || txbuf == NULL || rxbuf == NULL)
        return HOSAL_QSPI_INVALID_PARAM;

    uint8_t id = hosal_qspi_id_get(qspi_dev);
    hosal_qspi_dev_t* ptr = hosal_qspi_handle_get(id);

    if (ptr->config.qspi_id >= MAX_NUMBER_OF_QSPI
        || ptr->qspi_state != QSPI_STATE_IDLE)
        return HOSAL_QSPI_INVALID_REQUEST;

    ptr->qspi_state = HOSAL_QSPI_STATE_TRANSFER;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    ptr->instance->QSPI_SLAVE_SEL = (1 << ptr->config.slave_select);
#endif

    ptr->tx_buf = txbuf;
    ptr->data_length = size;
    ptr->rx_buf = rxbuf;

    uint32_t aux_reg;
    /*Auto set transfer FIFO mode.*/
    if (ptr->data_length & 3) {
        /*length is not 4N data*/
        aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI
                  | ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                         ? (QSPI_Xfer_Extend)
                         : (0));
    } else {
        /*length is 4N data*/
        /*slave mode only use 8 bytes to transfer*/
        aux_reg = QSPI_NORMAL_SPI
                  | ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                         ? (QSPI_BITSIZE_32 | QSPI_Xfer_Extend)
                         : (QSPI_BITSIZE_8));
    }

    ptr->instance->qspi_int_clr = 0xFF; /* clear all interrupt flag */

    if (ptr->rx_buf == NULL) {
        aux_reg |= QSPI_DISABLE_IN;
        ptr->instance->qspi_int_en =
            QSPI_INT_xferDone; /* enable transfer done interrupt */
    }

    uint8_t dummy_enable = 0;
    if (ptr->tx_buf == NULL) {
        if ((ptr->instance->qspi_control & QSPI_CNTL_MASTER) == 0)
            aux_reg |= QSPI_DISABLE_OUT; /* required? */
        else
            dummy_enable = 1; /* master will send dummy data */
    }

    ptr->instance->qspi_aux = aux_reg;

    if (ptr->rx_buf != NULL) {
        ptr->instance->dma_rx_addr = (uint32_t)ptr->rx_buf;
        ptr->instance->dma_rx_len = ptr->data_length;
        ptr->instance->dma_rx_enable =
            QSPI_DMA_ENABLE | (dummy_enable ? QSPI_DMA_Dummy_ENABLE : 0);
    }

    if (ptr->tx_buf != NULL) {
        ptr->instance->dma_tx_addr = (uint32_t)ptr->tx_buf;
        ptr->instance->dma_tx_len = ptr->data_length;
        ptr->instance->dma_tx_enable = QSPI_DMA_ENABLE;
    }

    /* avoid host send short data */
    if ((ptr->instance->qspi_control & QSPI_CNTL_MASTER) == 0)
        ptr->instance->qspi_int_en = QSPI_INT_xferDone;

    return HOSAL_STATUS_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_flash_command(hosal_qspi_dev_t* qspi_dev,
                                             hosal_qspi_flash_command_t* req) {
    if (req->cmd_length == 0 || qspi_dev == NULL || req == NULL)
        return HOSAL_QSPI_INVALID_PARAM;

    uint8_t id = hosal_qspi_id_get(qspi_dev);
    hosal_qspi_dev_t* ptr = hosal_qspi_handle_get(id);

    if (ptr->config.qspi_id >= MAX_NUMBER_OF_QSPI
        || ptr->qspi_state != QSPI_STATE_IDLE)
        return HOSAL_QSPI_INVALID_REQUEST;

    ptr->qspi_state = HOSAL_QSPI_STATE_TRANSFER;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    ptr->instance->qspi_slave_sel = (1 << ptr->config.slave_select);
#endif

    uint32_t aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_DISABLE_IN
                       | QSPI_Xfer_Extend;
    ptr->instance->qspi_aux = aux_reg;

    uint8_t* cmd_ptr = req->cmd_buf;
    uint32_t remain_len = req->cmd_length;
    while (remain_len > 0) {
        if (!(ptr->instance->qspi_status & QSPI_STATUS_txFull)) {
            ptr->instance->qspi_tx_fifo = *cmd_ptr++;
            remain_len--;
        }
    }
    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {}

    if (req->write_length == 0 && req->read_length == 0) {
        hosal_qspi_idle(ptr);
        return HOSAL_STATUS_SUCCESS;
    }

    if (req->write_length != 0 && req->write_buf != NULL) {
        uint8_t* write_ptr = req->write_buf;
        remain_len = req->write_length;
        while (remain_len > 0) {
            if (ptr->instance->qspi_status & QSPI_STATUS_txEmpty) {
                uint32_t bitsize = (remain_len > 3) ? QSPI_BITSIZE_32
                                                    : QSPI_BITSIZE_8;
                ptr->instance->qspi_aux = req->transfer_mode | QSPI_DISABLE_IN
                                          | QSPI_Xfer_Extend | bitsize;
                ptr->instance->qspi_tx_fifo = (remain_len > 3)
                                                  ? *(uint32_t*)write_ptr
                                                  : *(uint8_t*)write_ptr;
                write_ptr += (remain_len > 3) ? 4 : 1;
                remain_len -= (remain_len > 3) ? 4 : 1;
            }
        }
    } else if (req->read_length != 0 && req->read_buf != NULL) {
        uint8_t* read_ptr = req->read_buf;
        remain_len = req->read_length;
        while (remain_len > 0) {
            uint32_t bitsize = (remain_len > 3) ? QSPI_BITSIZE_32
                                                : QSPI_BITSIZE_8;
            uint32_t step = (remain_len > 3) ? 4 : 1;
            ptr->instance->qspi_aux = req->transfer_mode | bitsize
                                      | QSPI_Xfer_Extend;
            if (ptr->instance->qspi_status & QSPI_STATUS_txEmpty)
                ptr->instance->qspi_tx_fifo = 0;
            while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {}
            if (!(ptr->instance->qspi_status & QSPI_STATUS_rxEmpty)) {
                if (remain_len > 3)
                    *(uint32_t*)read_ptr = ptr->instance->qspi_rx_fifo;
                else
                    *(uint8_t*)read_ptr = ptr->instance->qspi_rx_fifo;
                read_ptr += step;
                remain_len -= step;
            }
        }
    }
    hosal_qspi_idle(ptr);
    return HOSAL_STATUS_SUCCESS;
}

hosal_qspi_status_t
hosal_qspi_tranfer_command(hosal_qspi_dev_t* qspi_dev,
                           hosal_qspi_flash_command_t* req) {
    hosal_qspi_status_t err = HOSAL_STATUS_SUCCESS;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    // enter_critical_section();
    /*We can set spi host only when spi_state in idle mode*/
    // if (qspi_cfg[qspi_id].qspi_state != QSPI_STATE_IDLE)
    // {
    //     leave_critical_section();
    //     return STATUS_INVALID_REQUEST;
    // }

    // leave_critical_section();

    ptr->qspi_state = QSPI_STATE_WRITE_DMA;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    QSPI->qspi_slave_sel = (1 << req->select_slave_device);
#endif

    /*Set Aux, During comamnd phase, we don't hope to get RX */
    ptr->instance->qspi_aux = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_DISABLE_IN
                              | QSPI_Xfer_Extend;
    while (req->cmd_length > 0) {
        if (!(ptr->instance->qspi_status & QSPI_STATUS_txFull)) {
            ptr->instance->qspi_tx_fifo = *req->cmd_buf++;
            req->cmd_length--;
        }
    }

    /*we should wait TX_FIFO_Empty and transfer not in progress.
      but we don't wait here.*/
    return err;
}

hosal_qspi_status_t hosal_qspi_write_dma(hosal_qspi_dev_t* qspi_dev,
                                         hosal_qspi_flash_command_t* req) {
    hosal_qspi_status_t err = HOSAL_STATUS_SUCCESS;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

#if defined(SPI_CHECK_DEBUG)

    /*command buffer can not be zero*/
    HOSAL_QSPI_CHECK(STATUS_INVALID_PARAM, qspi_id >= MAX_NUMBER_OF_QSPI);
    /*check input parameter*/
    HOSAL_QSPI_CHECK(STATUS_INVALID_REQUEST,
                     (req->cmd_buf == NULL) || (req->cmd_length == 0));
    HOSAL_QSPI_CHECK(STATUS_INVALID_REQUEST, req->write_buf == NULL);
    HOSAL_QSPI_CHECK(STATUS_INVALID_REQUEST, finish_proc_cb == NULL);

#endif
    /*ignore read_buf parameter. because this is write_dma function.*/
    //taskENTER_CRITICAL();
    enter_critical_section();
    err = hosal_qspi_tranfer_command(ptr, req);
    //taskEXIT_CRITICAL();
    leave_critical_section();
    HOSAL_QSPI_CHECK(err, err != HOSAL_STATUS_SUCCESS);

    /*only enable TX */
    ptr->instance->dma_tx_addr = (uint32_t)(req->write_buf);
    ptr->instance->dma_tx_len = req->write_length;

    /*we wait TX_FIFO_Empty and transfer not in progress for command here.*/

    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {}
    /*wait txEmpty and xferIP done*/

    ptr->instance->qspi_aux = (req->transfer_mode)
                              | ((req->write_length & 0x3) ? QSPI_BITSIZE_8
                                                           : QSPI_BITSIZE_32)
                              | QSPI_DISABLE_IN | QSPI_Xfer_Extend; /*!4N case*/

    /*  Notice: TX DMA finish does NOT mean all data has been transfered in spi serial bus...
     * It just means "all data written into FIFO" and will be sent soon.
     * There is a interval gap, if QSPI running in high speed, the gap can be ignored
     * But for insured, we wait transfer done, not DMA TX done.
     */

    /*before enable SPI interrupt, we must clear all previous status*/
    ptr->instance->qspi_int_clr = 0xFF;
    /*enable transfer done interrupt*/
    ptr->instance->qspi_int_en = QSPI_INT_xferDone;
    /*enable DMA, start to transfer*/
    ptr->instance->dma_tx_enable = QSPI_DMA_ENABLE;
    return err;
}

/* Notice:
 *  1. Here  QSPI is master mode.
 *  2. Not suggest to call this function in interrupt service routine.
 */
hosal_qspi_status_t hosal_qspi_read_dma(hosal_qspi_dev_t* qspi_dev,
                                        hosal_qspi_flash_command_t* req) {
    hosal_qspi_status_t err = HOSAL_STATUS_SUCCESS;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

#if defined(SPI_CHECK_DEBUG)

    /*command buffer can not be zero*/
    HOSAL_QSPI_CHECK(STATUS_INVALID_PARAM, qspi_id >= MAX_NUMBER_OF_QSPI);
    /*check input parameter*/
    HOSAL_QSPI_CHECK(STATUS_INVALID_REQUEST,
                     (req->cmd_buf == NULL) || (req->cmd_length == 0));
    HOSAL_QSPI_CHECK(STATUS_INVALID_REQUEST, req->read_buf == NULL);
    HOSAL_QSPI_CHECK(STATUS_INVALID_REQUEST, finish_proc_cb == NULL);

#endif

    //taskENTER_CRITICAL();
    enter_critical_section();
    err = hosal_qspi_tranfer_command(ptr, req);
    leave_critical_section();
    //taskEXIT_CRITICAL();
    HOSAL_QSPI_CHECK(err, err != HOSAL_STATUS_SUCCESS);

    /*
     * It can change qspi_state directly without proect here.
     * Becaue the qspi_state is > QSPI_STATE_IDLE
     */
    // qspi_cfg[qspi_id].qspi_state = QSPI_STATE_READ_DMA;
    ptr->qspi_state = QSPI_STATE_READ_DMA;

    /*set DMA here.*/

    /*only enable RX */
    ptr->instance->dma_rx_addr = (uint32_t)(req->read_buf);
    ptr->instance->dma_rx_len = req->read_length;

    /*we wait TX_FIFO_Empty and transfer not in progress for command here.*/

    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone))
        ;
    /*wait txEmpty and xferIP done*/

    ptr->instance->qspi_aux = (req->transfer_mode)
                              | ((req->read_length & 0x3) ? QSPI_BITSIZE_8
                                                          : QSPI_BITSIZE_32)
                              | QSPI_Xfer_Extend; /*!4N case*/

    /*before enable SPI interrupt, we must clear all previous status*/
    ptr->instance->qspi_int_clr = 0xFF;

    /*  DMA ISR RX finish interrupt is later than QSPI_INT_xferDone in master,
     *  so we don't need to wait QSPI_INT_xferDone
     *  However, in QSPI ISR, it still can see the case "QSPI->QSPI_INT_STATUS & QSPI_INT_xferDone"
     */

    /*enable RX DMA, start to transfer --- also enable dummy TX, Dummy data is 0*/
    ptr->instance->dma_rx_enable = (QSPI_DMA_Dummy_ENABLE | QSPI_DMA_ENABLE);
    return err;
}

hosal_qspi_status_t hosal_qspi_transfer_pio(hosal_qspi_dev_t* qspi_dev,
                                            uint8_t* txbuf, uint8_t* rxbuf,
                                            uint16_t size, uint32_t timeout) {
    hosal_qspi_status_t err = HOSAL_STATUS_SUCCESS;
    uint32_t aux_reg = 0;
    uint32_t tickstart = 0;

    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    tickstart = xTaskGetTickCount();

    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM, size == 0);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     (qspi_dev == NULL) || (txbuf == NULL) || (rxbuf == NULL));
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     ptr->config.qspi_id >= MAX_NUMBER_OF_QSPI);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     ptr->qspi_state != QSPI_STATE_IDLE);

    ptr->qspi_state = HOSAL_QSPI_STATE_TRANSFER;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    ptr->instance->QSPI_SLAVE_SEL = (1 << ptr->config.slave_select);
#endif

    aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI
              | ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                     ? QSPI_Xfer_Extend
                     : 0);

    ptr->instance->qspi_aux = aux_reg;
    ptr->transfer_cb = NULL;

    ptr->tx_xfer_count = size;
    ptr->rx_xfer_count = size;
    ptr->tx_buf = txbuf;
    ptr->rx_buf = rxbuf;

    while (ptr->tx_xfer_count > 0 || ptr->rx_xfer_count > 0) {
        if (!(ptr->instance->qspi_status & QSPI_STATUS_txFull)
            && (ptr->tx_xfer_count > 0)) {
            ptr->instance->qspi_tx_fifo = *ptr->tx_buf;
            ptr->tx_buf++;
            ptr->tx_xfer_count--;
        }

        if (!(ptr->instance->qspi_status & QSPI_STATUS_rxEmpty)
            && (ptr->rx_xfer_count > 0)) {
            *(uint8_t*)ptr->rx_buf = ptr->instance->qspi_rx_fifo;
            ptr->rx_buf++;
            ptr->rx_xfer_count--;
        }

        /* 
            if (xTaskGetTickCount() - tickstart >= timeout) {
                hosal_qspi_abort(ptr);
                return HOSAL_QSPI_TIMEOUT;
            }
        */
    }

    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {
        /*
        if (xTaskGetTickCount() - tickstart >= timeout) {
            hosal_qspi_abort(ptr);
            return HOSAL_QSPI_TIMEOUT;
        }
        */
        ;
    }
    /*disable CS ASAP*/
    aux_reg &= ~(QSPI_Xfer_Extend);
    ptr->instance->qspi_aux = aux_reg; /*Disable contXferExtend and stop RX in*/

    ptr->qspi_state = QSPI_STATE_IDLE;

    return err;
}

hosal_qspi_status_t hosal_qspi_ioctl(hosal_qspi_dev_t* qspi_dev, int ctl,
                                     void* p_arg) {
    return hosal_qspi_dataset_access[ctl](qspi_dev, p_arg);
}

hosal_qspi_status_t
hosal_qspi_callback_register(hosal_qspi_dev_t* qspi_dev,
                             hosal_qspi_cb_type_t callback_type,
                             hosal_qspi_callback_t pfn_callback, void* arg) {
    if (callback_type == HOSAL_QSPI_TRANSFER_DMA) {
        qspi_dev->transfer_cb = pfn_callback;
        qspi_dev->p_transfer_arg = arg;
    }
    return HOSAL_STATUS_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_idle(hosal_qspi_dev_t* qspi_dev) {
    uint32_t aux_reg = 0;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    aux_reg = ptr->instance->qspi_aux;
    aux_reg = aux_reg & ~(QSPI_Xfer_Extend);
    ptr->instance->qspi_aux = aux_reg; /*Disable contXferExtend and stop RX in*/

    ptr->qspi_state = QSPI_STATE_IDLE;
    return HOSAL_STATUS_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_abort(hosal_qspi_dev_t* qspi_dev) {
    hosal_qspi_status_t err = HOSAL_STATUS_SUCCESS;
    uint32_t aux_reg = 0;

    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     (ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                         != QSPI_CNTL_SLAVE);

    aux_reg = ptr->instance->qspi_aux;
    aux_reg = aux_reg & ~(QSPI_Xfer_Extend);
    ptr->instance->qspi_aux = aux_reg; /*Disable contXferExtend and stop RX in*/

    /*let devcie to idle mode.*/
    ptr->instance->dma_tx_enable = 0;
    ptr->instance->dma_rx_enable = 0;

    ptr->instance->dma_tx_len = 0;
    ptr->instance->dma_rx_len = 0;
    ptr->instance->qspi_en = 0; /*clear fifo.*/

    /*read Enable Register, it should be 0 before next step.*/
    while (ptr->instance->qspi_en)
        ;

    /*Enable QSPI*/
    ptr->instance->qspi_en = 1;
    ptr->instance->qspi_int_clr = 0xff; /*clear all interrupt flag*/
    ptr->qspi_state = QSPI_STATE_IDLE;
    return err;
}

uint8_t hosal_qspi_id_get(hosal_qspi_dev_t* qspi_dev) {
    return (qspi_dev->instance != QSPI0);
}

hosal_qspi_dev_t* hosal_qspi_handle_get(uint8_t id) {
    return (hosal_qspi_dev_t*)&g_qspi_handle[id];
}

hosal_qspi_status_t hosal_qspi_handle_set(hosal_qspi_dev_t* qspi_dev,
                                          uint8_t id) {
    memcpy((void*)&g_qspi_handle[id], qspi_dev, sizeof(hosal_qspi_dev_t));
}

void hosal_qspi_isr_notification(uint8_t id) {
    hosal_qspi_dev_t* ptr = hosal_qspi_handle_get(id);

    if (ptr->instance->dma_int_status & QSPI_DMA_ISR_RX) {
        /* RX complete, clear RX dma status */
        ptr->instance->dma_int_status = (QSPI_DMA_ISR_RX | QSPI_DMA_ISR_TX);
        ptr->instance->dma_rx_enable = 0;
        /* For the case, SPI normal mode, TX RX start for this operation */
        ptr->instance->dma_tx_enable = 0;
        ptr->instance->qspi_int_clr = QSPI_INT_xferDone;
        ptr->instance->qspi_aux &= ~QSPI_Xfer_Extend;
        ptr->qspi_state = QSPI_STATE_IDLE;

        /* Notify callback */
        if (ptr->transfer_cb != NULL)
            ptr->transfer_cb(ptr->p_transfer_arg);
        return;
    }

    if (ptr->instance->qspi_int_status & QSPI_INT_xferDone) {
        /* Clear status */
        ptr->instance->qspi_int_clr = QSPI_INT_xferDone;

        if (ptr->qspi_state == QSPI_STATE_WRITE_DMA) {
            /* TX complete, clear TX dma status */
            ptr->instance->dma_int_status = QSPI_DMA_ISR_TX;
            ptr->instance->dma_tx_enable = 0;
            ptr->instance->qspi_aux &= ~QSPI_Xfer_Extend;
            ptr->instance->qspi_int_en = 0; /* Disable self interrupt */
            ptr->qspi_state = QSPI_STATE_IDLE;

            /* Notify callback */
            if (ptr->transfer_cb != NULL) {
                ptr->transfer_cb(ptr->p_transfer_arg);
            }
            return;
        }

        /* Special case for slave mode */
        if ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
            == QSPI_CNTL_SLAVE) {
            if (ptr->instance->dma_rx_enable != 0
                || ptr->instance->dma_tx_enable != 0) {
                /* Master abort command? error? */
                ptr->instance->dma_tx_enable = 0;
                ptr->instance->dma_rx_enable = 0;
                ptr->instance->dma_tx_len = 0;
                ptr->instance->dma_rx_len = 0;
                ptr->instance->qspi_int_clr =
                    0xFF;                   /* Clear all interrupt flag */
                ptr->instance->qspi_en = 0; /* Clear FIFO */

                /* Read Enable Register, it should be 0 before next step */
                while (ptr->instance->qspi_en) {}

                /* Enable QSPI */
                ptr->instance->qspi_en = 1;
                ptr->qspi_state = QSPI_STATE_IDLE;

                /* Notify callback */
                if (ptr->transfer_cb != NULL)
                    ptr->transfer_cb(ptr->p_transfer_arg);
            }
        }
    }
}

void qspi0_handler(void) { hosal_qspi_isr_notification(0); }

void qspi1_handler(void) { hosal_qspi_isr_notification(1); }
