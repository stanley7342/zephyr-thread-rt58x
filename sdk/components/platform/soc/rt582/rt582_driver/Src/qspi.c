/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            qspi.c
 * \brief           qspi driver file
 */
/*
 * Author:          ives.lee
 */

#include "mcu.h"

/*
 * Remark:
 *   This driver is for NORMAL SPI device and DUAL/QUAD Serial Flash, like GD25LQ80 ...
 *   If your system need one external "Q-flash" that controlled by QSPI0 or QSPI1,
 * you can use this driver to transfer flash command to read/write/erase...
 *   One important difference QSPI flash from normal SPI device is that data
 * can be transfered in 2 or 4 bits mode for QSPI flash.
 */

#define MAX_NUMBER_OF_QSPI 2

#define MAX_SLAVE_DEVICE 4

typedef struct {
    /*callback for asynchronous notify finish.*/
    qspi_proc_cb_t qspi_callback;

    /*qspi_state is critical data*/
    uint16_t qspi_state; /*current spi master state*/

} qspi_config_t;

/* this is internal used data structure.*/
static qspi_config_t qspi_cfg[MAX_NUMBER_OF_QSPI] = {{NULL, QSPI_STATE_UNINIT},
                                                     {NULL, QSPI_STATE_UNINIT}};



uint32_t qspi_init(uint32_t qspi_id, const qspi_transfer_mode_t* spi_mode) {
    qspi_t* QSPI;
    uint32_t control_reg;

    if (qspi_id >= MAX_NUMBER_OF_QSPI) {
        return STATUS_INVALID_PARAM;
    }

    if (spi_mode->SPI_CLK > 0x1FF) {
        return STATUS_INVALID_PARAM; /*Invalid Master CLK.*/
    }

    enter_critical_section();

    if (qspi_cfg[qspi_id].qspi_state > QSPI_STATE_IDLE) {
        /*You can not set QSPI mode when QSPI in operation*/
        leave_critical_section();
        return STATUS_ERROR;
    }

    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_SETUP;

    leave_critical_section();

    // Enable QSPI Clk
    enable_perclk((SPI0_CLK_INDEX + qspi_id));

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    /*
     * Notice: check QSPI clock. if current system bus is 32Mhz Clock and QSPI is in Slave mode
     * it can NOT run QSPI in 32Mhz
     */
    if ((spi_mode->SPI_CLK == QSPI_CLK_32M)
        && ((SYSCTRL->sys_clk_ctrl & HCLK_SEL_MASK) == HCLK_SEL_32M)) {

        if (spi_mode->SPI_MASTER == SPI_SLAVE_MODE) {
            QSPI->qspi_clkdiv = QSPI_CLK_16M; /*slave can not run AHB clock*/
        } else {
            QSPI->qspi_clkdiv = QSPI_CLK_32M;
        }

    } else {
        QSPI->qspi_clkdiv = spi_mode->SPI_CLK;
    }

    QSPI->qspi_en = 0; /*clear FIFO.*/

    /*read Enable Register, it should be 0 before next step.*/
    while (QSPI->qspi_en) {}

    QSPI->qspi_int_clr = 0xFF; /*clear all interrupt flag*/

    QSPI->qspi_int_en = 0x0; /*disable all interrupt*/

#ifdef SUPPORT_QSPI0_MULTI_CS
    /*Set qspi device SS pol --- CS active High or active Low setting*/
    QSPI->qspi_slave_sel_pol = ((QSPI->qspi_slave_sel_pol
                                 & ~(1 << spi_mode->SPI_CS))
                                | (spi_mode->SPI_CS_POL << spi_mode->SPI_CS));
#else
    /*Set qspi device SS pol --- only one spi device. so ignore other chip select*/
    QSPI->qspi_slave_sel_pol = (spi_mode->SPI_CS_POL << spi_mode->SPI_CS);
#endif

    QSPI->qspi_slave_sel = (1 << spi_mode->SPI_CS);

    /*Set qspi transfer mode, here we assume bitsize is 8 bit first, Normal SPI mode*/
    QSPI->qspi_aux = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_DISABLE_IN;

    /*clear native DMA register.*/
    QSPI->dma_ier = 0;
    QSPI->dma_int_status = QSPI_DMA_ISR_CLEARALL; /*clear all interrupt*/
    QSPI->dma_rx_enable = 0;
    QSPI->dma_tx_enable = 0;

    /* Enable DMA RX interrupt */
    //QSPI->DMA_IER = QSPI_DMA_IER_TX | QSPI_DMA_IER_RX;
    QSPI->dma_ier = QSPI_DMA_IER_RX;

    /*For native SPI DMA, watermark is 8 entry only*/
    control_reg = QSPI_CNTL_TX_1_4_WATERMARK | QSPI_CNTL_RX_1_4_WATERMARK
                  | (spi_mode->SPI_MASTER << 5) | (spi_mode->SPI_CPOL << 4)
                  | (spi_mode->SPI_CPHA << 3) | (spi_mode->SPI_BIT_ORDER << 2)
                  | QSPI_CNTL_LITTLE_ENDIAN;

    if (spi_mode->SPI_MASTER) {
        QSPI->qspi_control = control_reg | QSPI_CNTL_contXfer_En;
    } else {
        /* This is for normal SPI slave setting.
         * For QSPI, data_N connect data_N, but for normal SPI
         * MOSI and MISO is cross connect pin.
         * So we need a bit to control our internal circuit switch.
         */
        QSPI->qspi_control = control_reg | SPI_CNTL_SLAVE_SDATA_OUT;
    }

    /*Enable QSPI*/
    QSPI->qspi_en = 1;

    if (qspi_id == 0) {
        NVIC_EnableIRQ(Qspi0_IRQn);
    } else {
        NVIC_EnableIRQ(Qspi1_IRQn);
    }

    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_IDLE;

    return STATUS_SUCCESS;
}

uint32_t qspi_change_clk(uint32_t qspi_id, uint16_t SPI_CLK) {
    qspi_t* QSPI;

    if (qspi_id >= MAX_NUMBER_OF_QSPI) {
        return STATUS_INVALID_PARAM;
    }

    if (SPI_CLK > 0x1FF) {
        return STATUS_INVALID_PARAM; /*Invalid Master CLK.*/
    }

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    enter_critical_section();

    if (qspi_cfg[qspi_id].qspi_state > QSPI_STATE_IDLE) {
        /*You can not change QSPI mode when QSPI in operation*/
        leave_critical_section();
        return STATUS_ERROR;
    }

    /*
     * Notice: check QSPI clock. if current system bus is 32Mhz Clock
     * it can NOT run QSPI in 32Mhz
     */
    if ((SPI_CLK == QSPI_CLK_32M)
        && ((SYSCTRL->sys_clk_ctrl & HCLK_SEL_MASK) == HCLK_SEL_32M)) {
        QSPI->qspi_clkdiv = QSPI_CLK_16M;
    } else {
        QSPI->qspi_clkdiv = SPI_CLK;
    }

    leave_critical_section();

    return STATUS_SUCCESS;
}

#if defined(SUPPORT_QSPI0_MULTI_CS)

uint32_t qspi0_set_device_polarity(uint32_t slave_id, uint32_t slave_polarity) {
    /*only QSPI0 support this option...*/
    QSPI_T* QSPI = QSPI0;

    if (slave_id >= MAX_SLAVE_DEVICE) {
        return STATUS_INVALID_PARAM; /*Invalid Slave id.*/
    }

    enter_critical_section();

    if (qspi_cfg[0].qspi_state > QSPI_STATE_IDLE) {
        /*You can not change QSPI mode when QSPI in operation*/
        leave_critical_section();
        return STATUS_ERROR;
    }

    /*
     * Because QSPI_SLAVE_SEL_POL is share register, so we critical section protect
     * this API.
     */

    if (slave_polarity == 0) {
        /*select polarity is active low*/
        QSPI->QSPI_SLAVE_SEL_POL = (QSPI->QSPI_SLAVE_SEL_POL
                                    & ~(1 << slave_id));
    } else {
        /*select polarity is active high*/
        QSPI->QSPI_SLAVE_SEL_POL |= (1 << slave_id);
    }

    leave_critical_section();

    return STATUS_SUCCESS;
}

#endif

static void qspi_idle(uint32_t qspi_id) {
    uint32_t aux_reg;
    qspi_t* QSPI;

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    aux_reg = QSPI->QSPI_AUX;
    aux_reg = aux_reg & ~(QSPI_Xfer_Extend);
    QSPI->QSPI_AUX = aux_reg; /*Disable contXferExtend and stop RX in*/

    enter_critical_section();

    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_IDLE;

    leave_critical_section();
}

uint32_t qspi_transfer(uint32_t qspi_id, const qspi_block_request_t* req) {
    uint32_t aux_reg, remain_len, status_reg, still_required_bytes;
    uint32_t temp;
    uint8_t *cmd_ptr, *write_ptr, *read_ptr;

    qspi_block_request_t qspi_sync_req;

    qspi_t* QSPI;

#if defined(SPI_CHECK_DEBUG)

    if (qspi_id >= MAX_NUMBER_OF_QSPI) {
        return STATUS_INVALID_PARAM;
    }

    /*check input parameter*/
    if ((req->cmd_buf == NULL) || (req->cmd_length == 0)) {
        return STATUS_INVALID_REQUEST; /*command buffer can not be zero*/
    }

    if ((req->write_buf == NULL) && (req->write_length != 0)) {
        return STATUS_INVALID_REQUEST;
    }

    if ((req->read_buf == NULL) && (req->read_length != 0)) {
        return STATUS_INVALID_REQUEST;
    }

    if ((req->write_length != 0) && (req->read_length != 0)) {
        return STATUS_INVALID_REQUEST; /*Flash does NOT have this command*/
    }

#endif

    enter_critical_section();
    /*We can set spi host only when spi_state in idle mode*/
    if (qspi_cfg[qspi_id].qspi_state != QSPI_STATE_IDLE) {
        leave_critical_section();
        return STATUS_INVALID_REQUEST;
    }

    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_TRANSFER;

    leave_critical_section();

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    qspi_sync_req = *req;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    QSPI->QSPI_SLAVE_SEL = (1 << req->select_slave_device);
#endif

    /*TX FIFO Should be empty here.*/

    /*Set Aux, During comamnd phase, we don't hope to get RX */
    aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_DISABLE_IN
              | QSPI_Xfer_Extend;
    QSPI->qspi_aux = aux_reg;

    cmd_ptr = qspi_sync_req.cmd_buf;

    while (qspi_sync_req.cmd_length > 0) {
        if (!(QSPI->qspi_status & QSPI_STATUS_txFull)) {
            QSPI->qspi_tx_fifo = *cmd_ptr++;

            qspi_sync_req.cmd_length--;
        }
    }

    if (qspi_sync_req.write_length != 0) {
        if (qspi_sync_req.write_length >= 4) {
            aux_reg = (req->data_transfer_mode) | QSPI_BITSIZE_32
                      | QSPI_DISABLE_IN | QSPI_Xfer_Extend;
        } else {
            aux_reg = (req->data_transfer_mode) | QSPI_BITSIZE_8
                      | QSPI_DISABLE_IN | QSPI_Xfer_Extend;
        }
    } else if (qspi_sync_req.read_length != 0) {
        if (qspi_sync_req.read_length >= 4) {
            aux_reg = (req->data_transfer_mode) | QSPI_BITSIZE_32
                      | QSPI_Xfer_Extend;
        } else {
            aux_reg = (req->data_transfer_mode) | QSPI_BITSIZE_8
                      | QSPI_Xfer_Extend;
        }
    }

    /*we should wait TX_FIFO_Empty*/
    while (!(QSPI->qspi_status & QSPI_STATUS_AllCmdDone))
        ; /*wait txEmpty and xferIP done*/

    /*write data phase? read data phase?*/
    if ((req->write_length == 0) && (req->read_length == 0)) {
        /*no data phase. nothing to do. clear QSPI_CNTL_contXfer_En*/
        qspi_idle(qspi_id);
        return STATUS_SUCCESS;
    }

    QSPI->qspi_aux = aux_reg;

    if (req->write_length != 0) {
        write_ptr = qspi_sync_req.write_buf;

        while (qspi_sync_req.write_length > 3) {
            /*send data by 32bits fifo mode*/
            if (!(QSPI->qspi_status & QSPI_STATUS_txFull)) {
                temp = *((uint32_t*)write_ptr);
                QSPI->qspi_tx_fifo = temp;

                qspi_sync_req.write_length -= 4;
                write_ptr += 4;
            }
        }

        /*we should wait TX_FIFO_Empty*/
        while (!(QSPI->qspi_status & QSPI_STATUS_AllCmdDone)) {}; 

        QSPI->qspi_aux = (req->data_transfer_mode) | QSPI_BITSIZE_8
                         | QSPI_DISABLE_IN | QSPI_Xfer_Extend;

        if (qspi_sync_req.write_length != 0) {
            /*send data by 8bits fifo mode, the max remain length is 3*/

            while (qspi_sync_req.write_length > 0) {
                QSPI->qspi_tx_fifo = *write_ptr++;
                qspi_sync_req.write_length--;
            }

            /*we should wait TX_FIFO_Empty*/
            while (!(QSPI->qspi_status & QSPI_STATUS_AllCmdDone)) {}
        }
    } else if (req->read_length != 0) {
        read_ptr = qspi_sync_req.read_buf;
        remain_len = req->read_length;

        /*first, we use 32bits to transfer*/
        while (remain_len > 3) {
            status_reg = QSPI->qspi_status;
            if (!(status_reg & QSPI_STATUS_txFull)) {
                QSPI->qspi_tx_fifo = 0;
                remain_len -= 4;
            }

            if (!(status_reg & QSPI_STATUS_rxEmpty)) {
                temp = QSPI->qspi_rx_fifo;
                *((uint32_t*)read_ptr) = temp;
                read_ptr += 4;
            }
        }

        /*remain_len is less than 3 bytes*/
        /*we should wait TX_FIFO_Empty and receive data*/
        still_required_bytes = (uint32_t)qspi_sync_req.read_buf
                               + (req->read_length) - (uint32_t)read_ptr;

        while (remain_len != still_required_bytes) {
            status_reg = QSPI->qspi_status;
            if (!(status_reg & QSPI_STATUS_rxEmpty)) {
                temp = QSPI->qspi_rx_fifo;
                *((uint32_t*)read_ptr) = temp;
                read_ptr += 4;
                still_required_bytes -= 4;
            }
        }

        if (remain_len) {
            /*we should wait TX_FIFO_Empty*/
            while (!(QSPI->qspi_status & QSPI_STATUS_AllCmdDone)) {}

            /*change 8bit mode to transfer*/

            QSPI->qspi_aux = (req->data_transfer_mode) | QSPI_BITSIZE_8
                             | QSPI_Xfer_Extend;
            QSPI->qspi_tx_fifo = 0;

            while (remain_len) {
                while (QSPI->qspi_status & QSPI_STATUS_rxEmpty)
                    ;

                *read_ptr++ = QSPI->qspi_rx_fifo;
                remain_len--;
                if (remain_len) {
                    QSPI->qspi_tx_fifo = 0;
                }
            }
        }
    }

    qspi_idle(qspi_id);
    return STATUS_SUCCESS;
}

static uint32_t qspi_tranfer_command(uint32_t qspi_id,
                                     const qspi_block_request_t* req) {
    qspi_block_request_t qspi_req;
    qspi_t* QSPI;
    uint8_t* cmd_ptr;

    enter_critical_section();
    /*We can set spi host only when spi_state in idle mode*/
    if (qspi_cfg[qspi_id].qspi_state != QSPI_STATE_IDLE) {
        leave_critical_section();
        return STATUS_INVALID_REQUEST;
    }

    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_WRITE_DMA;

    leave_critical_section();

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

#if defined(SUPPORT_QSPI0_MULTI_CS)
    QSPI->QSPI_SLAVE_SEL = (1 << req->select_slave_device);
#endif

    /*Set Aux, During comamnd phase, we don't hope to get RX */
    QSPI->qspi_aux = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_DISABLE_IN
                     | QSPI_Xfer_Extend;

    qspi_req = *req;

    /*TX FIFO Should be empty here.*/
    cmd_ptr = qspi_req.cmd_buf;

    while (qspi_req.cmd_length > 0) {
        if (!(QSPI->qspi_status & QSPI_STATUS_txFull)) {
            QSPI->qspi_tx_fifo = *cmd_ptr++;

            qspi_req.cmd_length--;
        }
    }

    /*we should wait TX_FIFO_Empty and transfer not in progress.
      but we don't wait here.*/

    return STATUS_SUCCESS;
}

/*Notice: spi_transfer_pio is master only, and limit byte length < 32 bytes */
uint32_t spi_transfer_pio(uint32_t qspi_id, const spi_block_request_t* req) {
    qspi_t* QSPI; /*QSPI hardware address.*/
    uint32_t aux_reg, length;

    uint8_t *write_ptr, *read_ptr;

    if (qspi_id >= MAX_NUMBER_OF_QSPI) {
        return STATUS_INVALID_PARAM;
    }

    /* if read_buf==NULL, it means TX only*/
    if ((req == NULL) || (req->write_buf == NULL)) {
        return STATUS_INVALID_PARAM;
    }

    /*required length could not be 0 bytes */
    if (req->read_buf == NULL) { //TX only
        if (req->length == 0) {
            return STATUS_INVALID_PARAM;
        }
    } else {
        if ((req->length == 0) || (req->length >= 32)) {
            return STATUS_INVALID_PARAM;
        }
    }

    enter_critical_section();

    if (qspi_cfg[qspi_id].qspi_state != QSPI_STATE_IDLE) {
        leave_critical_section();
        return STATUS_INVALID_REQUEST;
    }

    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_TRANSFER;

    leave_critical_section();

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

#if defined(SUPPORT_QSPI0_MULTI_CS)
    QSPI->QSPI_SLAVE_SEL = (1 << req->select_slave_device);
#endif

    /*FIFO MODE8 */

    /* Check slave or master*/
    if (QSPI->qspi_control & QSPI_CNTL_MASTER) {
        aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_Xfer_Extend;
    } else {
        aux_reg =
            QSPI_BITSIZE_8
            | QSPI_NORMAL_SPI; /*slave using this pio mode is not good idea.*/
    }

    if (req->read_buf == NULL) {
        aux_reg |=
            QSPI_DISABLE_IN; /*it does NOT need read data. disable data into RX FIFO*/
    }

    QSPI->qspi_aux = aux_reg;

    /*this is polling */
    qspi_cfg[qspi_id].qspi_callback = NULL;

    length = req->length;
    write_ptr = req->write_buf;

    while (length > 0) {
        if (!(QSPI->qspi_status & QSPI_STATUS_txFull)) {
            QSPI->qspi_tx_fifo = *write_ptr++;

            length--;
        }
    }

    while (!(QSPI->qspi_status & QSPI_STATUS_AllCmdDone))
        ; /*wait txEmpty and xferIP done*/

    /*disable CS ASAP*/
    aux_reg = aux_reg & ~(QSPI_Xfer_Extend);
    QSPI->qspi_aux = aux_reg; /*Disable contXferExtend and stop RX in*/

    if (req->read_buf != NULL) {
        read_ptr = req->read_buf;

        while (!(QSPI->qspi_status & QSPI_STATUS_rxEmpty)) {
            *read_ptr++ = QSPI->qspi_rx_fifo;
        }
    }

    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_IDLE;

    return STATUS_SUCCESS;
}

/*SPI native DMA architecture*/

/* Notice:
 *  1. Here QSPI is master mode.
 *  2. Not suggest to call this function in interrupt service routine.
 *  3. write_length can be any value less than
 */
uint32_t qspi_write_dma(uint32_t qspi_id, const qspi_block_request_t* req,
                        qspi_proc_cb_t finish_proc_cb) {
    uint32_t ret;
    qspi_t* QSPI;

    /*ignore read_buf parameter. because this is write_dma function.*/
    ret = qspi_tranfer_command(qspi_id, req);

    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    /*only enable TX */
    QSPI->dma_tx_addr = (uint32_t)(req->write_buf);
    QSPI->dma_tx_len = req->write_length;

    /*we wait TX_FIFO_Empty and transfer not in progress for command here.*/

    while (!(QSPI->qspi_status & QSPI_STATUS_AllCmdDone))
        ; /*wait txEmpty and xferIP done*/

    if (req->write_length & 0x3) {
        QSPI->qspi_aux = (req->data_transfer_mode) | QSPI_BITSIZE_8
                         | QSPI_DISABLE_IN | QSPI_Xfer_Extend; /*!4N case*/
    } else {
        QSPI->QSPI_AUX = (req->data_transfer_mode) | QSPI_BITSIZE_32
                         | QSPI_DISABLE_IN | QSPI_Xfer_Extend; /*4N case*/
    }

    /*user call interrupt..*/
    qspi_cfg[qspi_id].qspi_callback = finish_proc_cb;

    /*  Notice: TX DMA finish does NOT mean all data has been transfered in spi serial bus...
     * It just means "all data written into FIFO" and will be sent soon.
     * There is a interval gap, if QSPI running in high speed, the gap can be ignored
     * But for insured, we wait transfer done, not DMA TX done.
     */
    /*before enable SPI interrupt, we must clear all previous status*/
    QSPI->qspi_int_clr = 0xFF;

    /*enable transfer done interrupt*/
    QSPI->qspi_int_en = QSPI_INT_xferDone;

    /*enable DMA, start to transfer*/
    QSPI->dma_tx_enable = QSPI_DMA_ENABLE;

    return STATUS_SUCCESS;
}

/* Notice:
 *  1. Here  QSPI is master mode.
 *  2. Not suggest to call this function in interrupt service routine.
 */
uint32_t qspi_read_dma(uint32_t qspi_id, const qspi_block_request_t* req,
                       qspi_proc_cb_t finish_proc_cb) {
    uint32_t ret;
    qspi_t* QSPI; /*QSPI hardware address.*/

    ret = qspi_tranfer_command(qspi_id, req);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    /*
     * It can change qspi_state directly without proect here.
     * Becaue the qspi_state is > QSPI_STATE_IDLE
     */
    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_READ_DMA;

    /*set DMA here.*/

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    /*only enable RX */
    QSPI->dma_rx_addr = (uint32_t)(req->read_buf);
    QSPI->dma_rx_len = req->read_length;

    /*we wait TX_FIFO_Empty and transfer not in progress for command here.*/

    while (!(QSPI->qspi_status & QSPI_STATUS_AllCmdDone))
        ; /*wait txEmpty and xferIP done*/

    if (req->read_length & 0x3) {
        QSPI->qspi_aux = (req->data_transfer_mode) | QSPI_BITSIZE_8
                         | QSPI_Xfer_Extend; /*!4N case*/
    } else {
        QSPI->qspi_aux = (req->data_transfer_mode) | QSPI_BITSIZE_32
                         | QSPI_Xfer_Extend; /*4N case*/
    }

    /*user call interrupt..*/
    qspi_cfg[qspi_id].qspi_callback = finish_proc_cb;

    /*before enable SPI interrupt, we must clear all previous status*/
    QSPI->qspi_int_clr = 0xFF;

    /*  DMA ISR RX finish interrupt is later than QSPI_INT_xferDone in master,
     *  so we don't need to wait QSPI_INT_xferDone
     *  However, in QSPI ISR, it still can see the case "QSPI->QSPI_INT_STATUS & QSPI_INT_xferDone"
     */

    /*enable RX DMA, start to transfer --- also enable dummy TX, Dummy data is 0*/
    QSPI->dma_rx_enable = (QSPI_DMA_Dummy_ENABLE | QSPI_DMA_ENABLE);

    return STATUS_SUCCESS;
}

uint32_t spi_transfer(uint32_t qspi_id, const spi_block_request_t* req,
                      qspi_proc_cb_t finish_proc_cb) {
    qspi_t* QSPI; /*QSPI hardware address.*/
    uint32_t aux_reg;
    uint8_t dummy_enable = 0;

    enter_critical_section();

    if (qspi_cfg[qspi_id].qspi_state != QSPI_STATE_IDLE) {
        leave_critical_section();
        return STATUS_INVALID_REQUEST;
    }

    if (req->read_buf == NULL) {
        /*TX only.*/
        qspi_cfg[qspi_id].qspi_state = QSPI_STATE_WRITE_DMA;

    } else {
        qspi_cfg[qspi_id].qspi_state = QSPI_STATE_READ_DMA;

        /*Notice DMA_IER  is QSPI_DMA_IER_RX enable Here.*/
    }

    leave_critical_section();

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

#if defined(SUPPORT_QSPI0_MULTI_CS)
    QSPI->QSPI_SLAVE_SEL = (1 << req->select_slave_device);
#endif

    /*
     *   If SPI request don't care read data, it can set
     * read_buf == NULL, we can disable read.
     *   If SPI is slave, set QSPI_Xfer_Extend has no effect
     */

    /*Auto set transfer FIFO mode.*/

    if (req->length & 3) {
        /*length is not 4N data*/
        if (QSPI->qspi_control & QSPI_CNTL_MASTER) {
            aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_Xfer_Extend;
        } else {
            aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI;
        }

    } else {
        /*length is 4N data*/
        if (QSPI->qspi_control & QSPI_CNTL_MASTER) {
            aux_reg = QSPI_BITSIZE_32 | QSPI_NORMAL_SPI | QSPI_Xfer_Extend;
        } else {
            /*slave mode only use 8 bytes to transfer*/
            aux_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI;
        }
    }

    QSPI->qspi_int_clr = 0xFF; /*clear all interrupt flag*/

    if (req->read_buf == NULL) {
        /*Client does care read data, "WRITE-ONLY".*/
        aux_reg |= QSPI_DISABLE_IN;

        /*enable transfer done interrupt*/
        QSPI->qspi_int_en = QSPI_INT_xferDone;
    }

    if (req->write_buf == NULL) {
        if ((QSPI->qspi_control & QSPI_CNTL_MASTER) == 0) {
            /*This is slave mode, the slave is only receives data from
              master only. Setting this bit to avoid TX FIFO underflow
              in slave. (Master can not set this condition)
             */
            /*TODO: Test is slave mode*/
            aux_reg |= QSPI_DISABLE_OUT; /*required?*/
        } else {
            /*master will send dummy data*/
            dummy_enable = 1;
        }
    }

    QSPI->qspi_aux = aux_reg;

    /*user call interrupt..*/
    qspi_cfg[qspi_id].qspi_callback = finish_proc_cb;

    if (req->read_buf != NULL) {
        QSPI->dma_rx_addr = (uint32_t)req->read_buf;
        QSPI->dma_rx_len = req->length;

        if (dummy_enable) {
            QSPI->dma_rx_enable = (QSPI_DMA_Dummy_ENABLE | QSPI_DMA_ENABLE);
        } else {
            QSPI->dma_rx_enable = QSPI_DMA_ENABLE;
        }
    }

    if (req->write_buf != NULL) {
        QSPI->dma_tx_addr = (uint32_t)req->write_buf;
        QSPI->dma_tx_len = req->length;

        QSPI->dma_tx_enable = QSPI_DMA_ENABLE;
    }
    /*2022/04/28 add.. patch for slave, and host stop before dma request length*/
    if ((QSPI->qspi_control & QSPI_CNTL_MASTER) == 0) {
        QSPI->qspi_int_en = QSPI_INT_xferDone; /*avoid host send short data.*/
    }
    return STATUS_SUCCESS;
}

uint32_t spi_transfer_slave_abort(uint32_t qspi_id) {
    qspi_t* QSPI; /*QSPI hardware address.*/

    /*let devcie to idle mode.*/

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    enter_critical_section();

    if (qspi_cfg[qspi_id].qspi_state == QSPI_STATE_IDLE) {
        leave_critical_section();
        return STATUS_SUCCESS; /*original idle   nothing to do...*/
    }

    /*only slave can abort.. host can NOT*/

    if ((QSPI->qspi_control & QSPI_CNTL_MASTER) != QSPI_CNTL_SLAVE) {
        leave_critical_section();
        return STATUS_INVALID_REQUEST; /*can not abort for host mode*/
    }

    /*let devcie to idle mode.*/

    QSPI->dma_tx_enable = 0;
    QSPI->dma_rx_enable = 0;

    QSPI->dma_tx_len = 0;
    QSPI->dma_rx_len = 0;

    QSPI->qspi_en = 0; /*clear FIFO.*/

    /*read Enable Register, it should be 0 before next step.*/
    while (QSPI->qspi_en)
        ;

    /*Enable QSPI*/
    QSPI->qspi_en = 1;

    QSPI->qspi_int_clr = 0xFF; /*clear all interrupt flag*/
    qspi_cfg[qspi_id].qspi_state = QSPI_STATE_IDLE;

    leave_critical_section();

    return STATUS_SUCCESS;
}

static void spi_handler(uint32_t id) {
    qspi_t* QSPI;

    /*check DMA ISR interrupt*/

    if (id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }

    /* We can not assume QSPI_DMA_ISR_TX and QSPI_INT_xferDone is "near"
       It is possible that SPI clock is low speed, it need time to transfer all data
       from FIFO to SPI. Interrupt overhead is about 1~2 us.
     */

    if (QSPI->dma_int_status & QSPI_DMA_ISR_RX) {
        /*RX complete, clear RX dma status*/
        /*if TX DMA is enable, we also clear it at the same time*/
        QSPI->dma_int_status = (QSPI_DMA_ISR_RX | QSPI_DMA_ISR_TX);
        QSPI->dma_rx_enable = 0;
        QSPI->dma_tx_enable =
            0; /*For the case, SPI normal mode, TX RX start for this operation*/

        QSPI->qspi_int_clr = QSPI_INT_xferDone;

        /*master required, we should complete the transfer, for slave, it is the same setting*/
        QSPI->qspi_aux = QSPI->qspi_aux & ~(QSPI_Xfer_Extend);

        qspi_cfg[id].qspi_state = QSPI_STATE_IDLE;
        /*notify callback*/

        qspi_cfg[id].qspi_callback(id, QSPI_STATUS_TRANSFER_COMPLETE);
    }

    if (QSPI->qspi_int_status & QSPI_INT_xferDone) {

        /*clear status*/
        QSPI->qspi_int_clr = QSPI_INT_xferDone;

        /*only care TX DMA. (RX DMA will be finished in QSPI_DMA_ISR_RX)*/
        if (qspi_cfg[id].qspi_state == QSPI_STATE_WRITE_DMA) {

            /*TX complete, clear TX dma status*/
            QSPI->dma_int_status = QSPI_DMA_ISR_TX;
            QSPI->dma_tx_enable = 0;

            /*we are master, we should complete the transfer*/
            QSPI->qspi_aux = QSPI->qspi_aux & ~(QSPI_Xfer_Extend);

            QSPI->qspi_int_en = 0; /*disable self interrupt*/

            /* transfer done, because we are in priority interrupt,
            * so we don't need critical section protect.
            */

            /*only TX... we should notify finish.*/
            qspi_cfg[id].qspi_state = QSPI_STATE_IDLE;
            /*notify callback*/

            qspi_cfg[id].qspi_callback(id, QSPI_STATUS_TRANSFER_COMPLETE);
        } else {
            /* 2022/04/28... this is specail case for slave mode...
             * host send data short data and stop CS...
             */

            if ((QSPI->qspi_control & QSPI_CNTL_MASTER) == QSPI_CNTL_SLAVE) {
                /*slave see CS inactive... stop to request*/
                if ((QSPI->dma_rx_enable != 0) || (QSPI->dma_tx_enable != 0)) {
                    /*master abort command? error?*/
                    QSPI->dma_tx_enable = 0;
                    QSPI->dma_rx_enable = 0;

                    QSPI->dma_tx_len = 0;
                    QSPI->dma_rx_len = 0;

                    QSPI->qspi_int_clr = 0xFF; /*clear all interrupt flag*/

                    QSPI->qspi_en = 0; /*clear FIFO.*/

                    /*read Enable Register, it should be 0 before next step.*/
                    while (QSPI->qspi_en)
                        ;

                    /*Enable QSPI*/
                    QSPI->qspi_en = 1;

                    qspi_cfg[id].qspi_state = QSPI_STATE_IDLE;

                    qspi_cfg[id].qspi_callback(id, QSPI_STATUS_TRANSFER_SHORT);
                }
            }
        }
    }

    return;
}

void qspi0_handler(void) {
    spi_handler(0);

    return;
}

void qspi1_handler(void) {
    spi_handler(1);

    return;
}

uint16_t get_qspi_state(uint32_t qspi_id) {
    if (qspi_id > MAX_NUMBER_OF_QSPI) {
        return 0; /*No such device...*/
    }

    return qspi_cfg[qspi_id].qspi_state;
}

void qspi_FIFO_clear(uint32_t qspi_id) {
    qspi_t* QSPI;

    if (qspi_id == 0) {
        QSPI = QSPI0;
    } else {
        QSPI = QSPI1;
    }
    QSPI->qspi_en = 0; /*clear FIFO.*/

    /*read Enable Register, it should be 0 before next step.*/
    while (QSPI->qspi_en)
        ;

    QSPI->qspi_en = 1; /*Enable QSPI.*/

    return;
}
