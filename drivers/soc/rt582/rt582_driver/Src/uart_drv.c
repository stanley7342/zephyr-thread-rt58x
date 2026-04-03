/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            uart_drv.c
 * \brief           uart driver file
 */
/*
 * Author:          ives.lee
 */

#include "mcu.h"

typedef struct {
    uart_callback_t uart_callback;              /*!< user application ISR handler. */
    void* p_arg;                                /*!< . */
} uart_cb_config_t;

typedef struct {
    uart_t* uart;                               /*!< Uart based hardware address */
    IRQn_Type irq_num;                          /*!< Uart IRQ Number */
    uart_config_t cfg;                          /*!< Uart config struct */
    uint32_t rx_len;                            /*!< Receive data length */
    uint32_t tx_len;                            /*!< Receive data length */
    uart_cb_config_t uart_cb[5];                /*!< Uart callback function table */

} uart_handle_t;

static uart_handle_t m_uart_handle[3] = {{
                                             /*UART0 instatnce*/
                                             .uart = UART0,
                                             .irq_num = Uart0_IRQn,
                                             .rx_len = 0,
                                             .tx_len = 0,
                                         },
                                         {
                                             /*UART1 instatnce*/
                                             .uart = UART1,
                                             .irq_num = Uart1_IRQn,
                                             .rx_len = 0,
                                             .tx_len = 0,
                                         },
                                         {
                                             /*UART2 instatnce*/
                                             .uart = UART2,
                                             .irq_num = Uart2_IRQn,
                                             .rx_len = 0,
                                             .tx_len = 0,
                                         }};

uint32_t uart_init(uart_config_t uart_cfg) {
    uart_t* uart = m_uart_handle[uart_cfg.uart_id].uart;
    uint32_t cval = 0, buadrate = 0, temp = 0;

    if (uart_cfg.uart_id > UART_ID_MAX) {
        return STATUS_INVALID_PARAM;
    }

    m_uart_handle[uart_cfg.uart_id].cfg = uart_cfg;

    buadrate = ((32000000 / uart_cfg.baud_rate) / 8);

    // Clear UART IRQ
    NVIC_ClearPendingIRQ(m_uart_handle[uart_cfg.uart_id].irq_num);
    NVIC_DisableIRQ(m_uart_handle[uart_cfg.uart_id].irq_num);

    // Enable UART peripheral clock
    enable_perclk((UART0_CLK + uart_cfg.uart_id));

    /*clear FIFO, REMAKR: FCR is write-only*/
    uart->FCR = UART_FIFO_DISABLE;
    uart->FCR = FCR_DEFVAL; /*reset FIFO*/

    // Disable interrupts
    uart->IER = UART_IER_DISABLE;

    /*Initial default LCR control register.*/
    uart->LCR = UART_LCR_CLEAR;

    /*bits mode only use two bits.*/
    cval |= LCR_WLEN8;

    /*set baudrate*/
    uart->LCR = (LCR_DLAB | cval); /*set uart deafult data width 8 bit.*/
    /*set baudrate*/
    uart->DLL = buadrate & 0xFF;
    uart->DLM = buadrate >> 8;
    uart->LCR = (cval);

    /*set stop bits*/
    if (uart_cfg.stop_bits == UART_STOPBIT_TWO) {
        cval |= LCR_STOP;
    }

    /*set parity*/
    if (uart_cfg.parity & PARENB) {
        cval |= LCR_PARITY;
    }
    if (!(uart_cfg.parity & PARODD)) {
        cval |= LCR_EPAR;
    }
    if (uart_cfg.parity & CMSPAR) {
        cval |= LCR_SPAR;
    }

    uart->LCR = cval;

    temp =
        uart->RBR; /*read temp to clear Timeout IIR noise... only happen when clear RX FIFO and data income..*/

    uart->MCR = 0; /*Initial default modem control register.*/

    if ((uart_cfg.flow_control == UART_HWFC_ENABLED)
        && (uart_cfg.uart_id == UART1_ID)) {
        /*only uart1 support HW flow control pin*/
        uart->MCR =
            MCR_ENABLE_CTS; /*Enable hardware CTS to block transmit data*/
        /*when CTS change, and rx line status error, interrupt event generated */
        uart->IER = UART_IER_RLSI | UART_IER_MSI;
    } else {
        /*Lins status interrupt */
        uart->IER = UART_IER_RLSI;
    }
    /*clear tx and rx fifo , set fifo trigger number , fifo enable */
    uart->FCR = (FCR_TRIGGER_8 | FCR_CLEAR_RCVR | FCR_CLEAR_XMIT | FCR_FIFO_EN);

    uart->xDMA_IER = UART_IER_DISABLE; /*disable xDMA interrupt*/

    uart->xDMA_RX_ENABLE = xDMA_Stop;
    uart->xDMA_TX_ENABLE = xDMA_Stop;

    uart->xDMA_TX_LEN = UART_LEN_CLEAR;
    uart->xDMA_RX_LEN = UART_LEN_CLEAR;

    uart->xDMA_INT_STATUS = xDMA_ISR_RX | xDMA_ISR_TX; /*write 1 to clear*/

    m_uart_handle[uart_cfg.uart_id].tx_len = UART_LEN_CLEAR;
    m_uart_handle[uart_cfg.uart_id].rx_len = UART_LEN_CLEAR;
    //Set uart priority
    NVIC_SetPriority(m_uart_handle[uart_cfg.uart_id].irq_num, 4);

    return STATUS_SUCCESS;
}

uint32_t uart_set_interrupt(uint32_t uart_id, uint32_t enable,
                            uint8_t ierflag) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    if (enable == TRUE) {
        if ((ierflag & UART_IER_MODE_CHK) != 0) {
            uart->xDMA_IER |= (ierflag & UART_IER_MODE_MASK);
        } else {
            uart->IER |= ierflag;
        }
    } else if (enable == FALSE) {
        if ((ierflag & UART_IER_MODE_CHK) != 0) {
            uart->xDMA_IER &= ~(ierflag & UART_IER_MODE_MASK);
        } else {
            uart->IER &= ~ierflag;
        }
    } else {
        return STATUS_INVALID_PARAM;
    }

    if ((uart->IER == UART_IER_DISABLE)
        && (uart->xDMA_IER == UART_IER_DISABLE)) { //No Uart Interrupt Setting
        NVIC_DisableIRQ(m_uart_handle[uart_id].irq_num);
    } else {
        NVIC_EnableIRQ(m_uart_handle[uart_id].irq_num);
    }

    return STATUS_SUCCESS;
}

uint32_t uart_enable_irq(uint32_t uart_id, uint32_t enable) {

    if (enable == FALSE) {
        NVIC_DisableIRQ(m_uart_handle[uart_id].irq_num);
    } else {
        NVIC_EnableIRQ(m_uart_handle[uart_id].irq_num);
    }

    return STATUS_SUCCESS;
}

uint32_t uart_fifo_reset(uint32_t uart_id, uint32_t fifo_flush_sel) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    uint32_t ier_reg_temp;
    ier_reg_temp = uart->IER;

    uart->IER = UART_IER_DISABLE;
    uart->FCR = UART_FIFO_DISABLE;

    if (fifo_flush_sel == FCR_CLEAR_XMIT) { //reset tx fifo
        uart->FCR = FCR_CLEAR_XMIT;
    } else if (fifo_flush_sel == FCR_CLEAR_RCVR) { //reset rx fieo FIFO
        uart->FCR = FCR_CLEAR_RCVR;
    } else {
        uart->FCR = FCR_DEFVAL; /*reset Tx and RX FIFO*/
    }

    uart->IER = ier_reg_temp;

    return STATUS_SUCCESS;
}

uint32_t uart_uninit(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    if (uart_id >= UART_ID_MAX) {
        return STATUS_INVALID_PARAM;
    }

    // Disable UART IRQ,
    NVIC_DisableIRQ(m_uart_handle[uart_id].irq_num);

    uart->xDMA_IER = UART_IER_DISABLE; /*disable xDMA interrupt*/

    uart->xDMA_RX_ENABLE = xDMA_Stop;
    uart->xDMA_TX_ENABLE = xDMA_Stop;

    uart->xDMA_INT_STATUS = xDMA_ISR_RX | xDMA_ISR_TX; /*write 1 to clear*/

    /*reset uart FIFO*/
    uart->FCR = UART_FIFO_DISABLE;
    uart->FCR = FCR_CLEAR_XMIT | FCR_CLEAR_RCVR;

    // Disable UART peripheral clock
    disable_perclk(UART0_CLK + uart_id);

    // Clear pending UART interrupts in NVIC
    NVIC_ClearPendingIRQ(m_uart_handle[uart_id].irq_num);

    return STATUS_SUCCESS;
}

uint32_t uart_register_callback(uint32_t uart_id, uart_callfun_id_t cb_id,
                                uart_callback_t uart_callback_func, void* arg) {
    if (cb_id > UART_CB_ID_MAX) {
        return STATUS_INVALID_PARAM;
    }

    if (cb_id == UART_TX_IT_ID) {
        m_uart_handle[uart_id].uart_cb[UART_TX_IT_ID].uart_callback =
            uart_callback_func;
        m_uart_handle[uart_id].uart_cb[UART_TX_IT_ID].p_arg = arg;
    } else if (cb_id == UART_RX_IT_ID) {
        m_uart_handle[uart_id].uart_cb[UART_RX_IT_ID].uart_callback =
            uart_callback_func;
        m_uart_handle[uart_id].uart_cb[UART_RX_IT_ID].p_arg = arg;
    }
    if (cb_id == UART_DMA_TX_IT_ID) {
        m_uart_handle[uart_id].uart_cb[UART_DMA_TX_IT_ID].uart_callback =
            uart_callback_func;
        m_uart_handle[uart_id].uart_cb[UART_DMA_TX_IT_ID].p_arg = arg;
    } else if (cb_id == UART_DMA_RX_IT_ID) {
        m_uart_handle[uart_id].uart_cb[UART_DMA_RX_IT_ID].uart_callback =
            uart_callback_func;
        m_uart_handle[uart_id].uart_cb[UART_DMA_RX_IT_ID].p_arg = arg;
    }

    else if (cb_id == UART_LSR_IT_ID) {
        m_uart_handle[uart_id].uart_cb[UART_LSR_IT_ID].uart_callback =
            uart_callback_func;
        m_uart_handle[uart_id].uart_cb[UART_LSR_IT_ID].p_arg = arg;
    } else {
        return STATUS_INVALID_PARAM;
    }

    return STATUS_SUCCESS;
}

bool uart_transfer_ready(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    return ((uart->LSR & UART_LSR_THRE) ? true : false);
}

uint32_t uart_transfer(uint32_t uart_id, uint8_t data) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    uart->THR = data;
    return STATUS_SUCCESS;
}

uint32_t uart_transfer_pol(uint32_t uart_id, uint8_t* p_data, uint32_t length) {
    uint16_t offset = 0;
    uart_t* uart = m_uart_handle[uart_id].uart;

    if ((p_data == NULL) || (length == 0) || (length > 65535)
        || (uart_id >= UART_ID_MAX)) {
        return STATUS_INVALID_PARAM;
    }

    m_uart_handle[uart_id].tx_len = length;

    do {
        while (!uart_transfer_ready(uart_id)) {
            ;
        }
        uart_transfer(uart_id, (p_data[offset])); //
        offset += 1;
        length--;

    } while (length);

    //if(p_data[offset]==0x0A) {
    //uart_transfer(uart_id,0x0A);//
    //uart_transfer(uart_id,0x0D);//
    //}

    m_uart_handle[uart_id].tx_len = length;

    return STATUS_SUCCESS;
}

uint32_t uart_transfer_dma(uint32_t uart_id, uint8_t* p_data, uint32_t length) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    if ((p_data == NULL) || (length == 0) || (length > 65535)
        || (uart_id >= UART_ID_MAX)) {
        return STATUS_INVALID_PARAM;
    }

    m_uart_handle[uart_id].tx_len = length;

    uart->xDMA_INT_STATUS = xDMA_ISR_TX;
    uart->xDMA_TX_ADDR = (uint32_t)(p_data);
    uart->xDMA_TX_LEN = length;
    uart->xDMA_TX_ENABLE = xDMA_Enable;

    return STATUS_SUCCESS;
}

uint32_t uart_transfer_it(uint32_t uart_id, uint8_t data) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    if (m_uart_handle[uart_id].tx_len != 0) {
        m_uart_handle[uart_id].tx_len -= 1;
        uart->THR = data;
        return STATUS_EBUSY;
    } else {
        m_uart_handle[uart_id].tx_len = 0;
        return STATUS_SUCCESS;
    }
}

uint32_t uart_receive_it(uint32_t uart_id, uint8_t* data) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    if (m_uart_handle[uart_id].rx_len != 0) {
        m_uart_handle[uart_id].rx_len -= 1;
        *data = (uart->RBR & 0xFF);
        return STATUS_EBUSY;
    } else {
        m_uart_handle[uart_id].rx_len = 0;
        return STATUS_SUCCESS;
    }
}

bool uart_receive_ready(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    return ((uart->LSR & UART_LSR_DR) ? true : false);
}

uint32_t uart_receive(uint32_t uart_id, uint8_t* data) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    *data = (uart->RBR & 0xFF);

    return STATUS_SUCCESS;
}

uint32_t uart_transfer_dma_ready(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;
    uint32_t status;

    if ((uart->xDMA_INT_STATUS & xDMA_ISR_TX) == 0) {
        uart->xDMA_TX_ADDR = 0;
        uart->xDMA_TX_LEN = 0;
        status = true;
    } else {
        uart->xDMA_INT_STATUS = xDMA_ISR_TX;
        uart->xDMA_TX_ENABLE = xDMA_Stop;
        status = false;
    }

    return status;
}

uint32_t uart_receive_dma_ready(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;
    uint32_t status;

    if ((uart->xDMA_INT_STATUS & xDMA_ISR_RX) == 0) {

        uart->xDMA_RX_ADDR = 0;
        uart->xDMA_RX_LEN = 0;

        status = true;
    } else {
        uart->xDMA_INT_STATUS = xDMA_ISR_RX;
        uart->xDMA_RX_ENABLE = xDMA_Stop;
        status = false;
    }

    return status;
}

uint32_t uart_receive_dma(uint32_t uart_id, uint8_t* p_data, uint32_t length) {

    uart_t* uart = m_uart_handle[uart_id].uart;

    uart->xDMA_INT_STATUS = xDMA_ISR_RX;
    uart->xDMA_RX_ADDR = (uint32_t)(p_data);
    uart->xDMA_RX_LEN = length;
    uart->xDMA_RX_ENABLE = xDMA_Enable;
    return STATUS_SUCCESS;
}

bool uart_receive_dma_finish(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;
    bool status;

    if ((uart->xDMA_INT_STATUS & xDMA_ISR_RX) == xDMA_ISR_RX) {
        status = true;
    } else {
        status = false;
    }

    return status;
}

bool uart_transfer_dma_finish(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;
    bool status;

    if ((uart->xDMA_INT_STATUS & xDMA_ISR_TX) == xDMA_ISR_TX) {
        status = true;
    } else {
        status = false;
    }

    return status;
}

/**
 * @brief
 */
uint32_t uart_transfer_dma_disable(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    uart->xDMA_INT_STATUS = xDMA_ISR_TX;
    uart->xDMA_TX_ADDR = 0;
    uart->xDMA_TX_LEN = 0;
    uart->xDMA_TX_ENABLE = xDMA_Stop;

    return STATUS_SUCCESS;
}

uint32_t uart_get_config(uart_config_t* uart_cfg) {
    uart_cfg = &m_uart_handle[uart_cfg->uart_id].cfg;

    return STATUS_SUCCESS;
}

uint32_t uart_set_config(uart_config_t uart_cfg) {
    uart_t* uart = m_uart_handle[uart_cfg.uart_id].uart;

    uint32_t uart_lcr = 0;

    /*Initial default LCR control register.*/
    uart->LCR = 0;
    /*bits mode only use two bits.*/
    uart_lcr |= uart_cfg.data_width;

    /*set baudrate*/
    uart->LCR = (LCR_DLAB | uart_lcr); /*set uart deafult data width 8 bit.*/
    /*set baudrate*/
    uart->DLL = uart_cfg.baud_rate & 0xFF;
    uart->DLM = uart_cfg.baud_rate >> 8;
    uart->LCR = (uart_lcr);

    /*set stop bits*/
    if (uart_cfg.stop_bits == UART_STOPBIT_TWO) {
        uart_lcr |= LCR_STOP;
    }

    /*set parity*/
    if (uart_cfg.parity & PARENB) {
        uart_lcr |= LCR_PARITY;
    }
    if (!(uart_cfg.parity & PARODD)) {
        uart_lcr |= LCR_EPAR;
    }
    if (uart_cfg.parity & CMSPAR) {
        uart_lcr |= LCR_SPAR;
    }

    uart->LCR = uart_lcr;

    /*copy uart confit into uart handle table*/
    m_uart_handle[uart_cfg.uart_id].cfg = uart_cfg;

    return STATUS_SUCCESS;
}

void uart_receive_abort(uint32_t uart_id) {

    uart_t* uart = m_uart_handle[uart_id].uart;

    if (uart_id >= UART2_ID) {
        return;
    }

    /*Disable receive data available interrupt*/
    uart->IER &= ~UART_IER_RDI;

    /*Disable Uart Dma rx and rx interrupt*/
    uart->xDMA_RX_ENABLE = xDMA_Stop;
    uart->xDMA_INT_STATUS = xDMA_ISR_RX;
    // Reset receive FIFO and set fifo trigger length
    uart->FCR = FCR_TRIGGER_8 | FCR_CLEAR_RCVR | FCR_FIFO_EN;
}

void uart_transfer_abort(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    if (uart_id >= UART2_ID) {
        return;
    }

    // Disable transmit holding register empty interrupt
    uart->IER &= ~UART_IER_THRI;
    uart->xDMA_TX_ENABLE = xDMA_Stop;
    uart->xDMA_INT_STATUS = xDMA_ISR_TX;

    // FCR is write-only.
    // Transmit FIFO reset
    uart->FCR = UART_FIFO_DISABLE; /*diable FIFO */
    uart->FCR = FCR_CLEAR_XMIT;    /*Reset FIFO*/
    uart->FCR = FCR_CLEAR_XMIT | FCR_TRIGGER_8 | FCR_FIFO_EN;

    /*if data in tranmist holding register, wait it empty.*/
    while ((uart->LSR & UART_LSR_TEMT) == 0)
        ;
}

uint32_t uart_set_break(uint32_t uart_id, uint32_t sbc_state) {

    uart_t* uart = m_uart_handle[uart_id].uart;

    if (uart_id != UART1_ID) {
        return STATUS_INVALID_PARAM; /*Invalid device id*/
    }

    if (sbc_state == TRUE) {
        uart->LCR |= LCR_SBC; /*set break control */
    } else {
        uart->LCR &= ~LCR_SBC; /*Clear Break*/
    }

    return STATUS_SUCCESS;
}

uint32_t uart_set_modem_status(uint32_t uart_id, uint32_t rts_state) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    if (uart_id != UART1_ID) {
        return STATUS_INVALID_PARAM; /*Only UART1 support modem RTS*/
    }

    if (rts_state) {
        /*
         * set RTS, nRTS will be low (nRTS is low active)
         * UART other  side can send data to us.
         */
        uart->MCR |= MCR_RTS;
    } else {
        /*
         * clear RTS, nRTS will be high (nRTS is low active)
         * UART other side SHOULD STOP to send data.
         */
        uart->MCR &= ~MCR_RTS;
    }

    return STATUS_SUCCESS;
}

/**
 * @brief
 */
static uint32_t uart_rx_line_int_handler(uint32_t lsr) {
    uint32_t event = 0;
    // OverRun error
    if (lsr & UART_LSR_OE) {
        event |= UART_EVENT_RX_OVERFLOW;
    }

    // Parity error
    if (lsr & UART_LSR_PE) {
        event |= UART_EVENT_RX_PARITY_ERROR;
    }

    // Break detected
    if (lsr & UART_LSR_BI) {
        event |= UART_EVENT_RX_BREAK;
    }

    // Framing error
    if (lsr & UART_LSR_FE) {
        event |= UART_EVENT_RX_FRAMING_ERROR;
    }

    return event;
}

/**
 * @brief
 */
static void __uart_notify_handler(uint32_t uart_id) {
    uart_t* uart = m_uart_handle[uart_id].uart;

    uint32_t iir, event, lsr;

    iir = uart->IIR & IIR_INTID_MSK;

    event = 0;

    if ((iir & IIR_INTSTATUS) == 0) {
        /*rx line status*/
        if (iir == IIR_INTID_RLS) {

            lsr = uart->LSR;

            event = uart_rx_line_int_handler(lsr);

            if (m_uart_handle[uart_id].uart_cb[UART_LSR_IT_ID].uart_callback
                != NULL) {

                m_uart_handle[uart_id].uart_cb[UART_LSR_IT_ID].uart_callback(
                    &event);
            }
        }

        /*data ready and Character Timeout*/
        if ((iir == IIR_INTID_RDA) || (iir == IIR_INTID_CTI)) {
            //fifo one byte the data read flag will be set
            //fifo length over 1 the iir will timeout
            m_uart_handle[uart_id].rx_len += 1;

            event |= UART_EVENT_RX_DONE;

            if (m_uart_handle[uart_id].uart_cb[UART_RX_IT_ID].uart_callback
                != NULL) {

                m_uart_handle[uart_id].uart_cb[UART_RX_IT_ID].uart_callback(
                    &event);
            }
        }

        /*data transfer*/
        if ((uart->LSR & UART_LSR_THRE)) {

            event |= UART_EVENT_TX_DONE;

            if (m_uart_handle[uart_id].tx_len != 0) {
                m_uart_handle[uart_id].tx_len -= 1;
            }

            if (m_uart_handle[uart_id].uart_cb[UART_TX_IT_ID].uart_callback
                != NULL) {

                m_uart_handle[uart_id].uart_cb[UART_TX_IT_ID].uart_callback(
                    &event);
            }
        }

        /*uart1 Hardware flow control*/
        if (iir == IIR_INTID_MS) {

            /*Check CTS state changed*/
            if (uart->MSR & UART_MSR_DCTS) {

                event |= UART_EVENT_CTS;
            }
        }
    }

    /*Uart dma transfer interrupt*/
    if ((uart->xDMA_INT_STATUS & xDMA_ISR_TX) == xDMA_ISR_TX
        && (uart->xDMA_IER & xDMA_IER_TX) != 0) {

        uart->xDMA_INT_STATUS = xDMA_ISR_TX;
        uart->xDMA_TX_ENABLE = xDMA_Stop;
        event |= UART_EVENT_DMA_TX_DONE;
        m_uart_handle[uart_id].tx_len = uart->xDMA_TX_LEN;
        if (m_uart_handle[uart_id].uart_cb[UART_DMA_TX_IT_ID].uart_callback
            != NULL) {

            m_uart_handle[uart_id].uart_cb[UART_DMA_TX_IT_ID].uart_callback(
                &event);
        }
    }

    /*Uart dma receiver interrutp*/
    if ((uart->xDMA_INT_STATUS & xDMA_ISR_RX) == xDMA_ISR_RX
        && (uart->xDMA_IER & xDMA_IER_RX) != 0) {

        uart->xDMA_INT_STATUS = xDMA_ISR_RX;
        uart->xDMA_RX_ENABLE = xDMA_Stop;

        event |= UART_EVENT_DMA_RX_DONE;

        m_uart_handle[uart_id].rx_len = uart->xDMA_RX_LEN;

        if (m_uart_handle[uart_id].uart_cb[UART_DMA_RX_IT_ID].uart_callback
            != NULL) {

            m_uart_handle[uart_id].uart_cb[UART_DMA_RX_IT_ID].uart_callback(
                &event);
        }
    }
}

/**
 * \brief           UART0 Interrupt handler.
 */
void uart0_handler(void) {

    __uart_notify_handler(UART0_ID);
    NVIC_ClearPendingIRQ(Uart0_IRQn);
}

/**
 * \brief           UART1 Interrupt handler.
 */
void uart1_handler(void) {

    __uart_notify_handler(UART1_ID);
    NVIC_ClearPendingIRQ(Uart1_IRQn);
}

/**
 * \brief           UART2 Interrupt handler.
 */
void uart2_handler(void) {

    __uart_notify_handler(UART2_ID);
    NVIC_ClearPendingIRQ(Uart2_IRQn);
}
