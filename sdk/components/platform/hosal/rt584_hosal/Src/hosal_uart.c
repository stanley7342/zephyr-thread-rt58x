/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_uart.c
 * \brief           Hosal UART driver file
 */
/*
 * Author:      
 */
#include <stdio.h>
#include <stdint.h>
#include "mcu.h"
#include "hosal_sysctrl.h"
#include "hosal_uart.h"


#define MAX_NUMBER_OF_UART  3
#define UART0_BASE_CLK      0

#define UART_ReadLineStatus(pReg) ((pReg)->lsr)

typedef struct {
    uart_t* uart;      /*based hardware address*/
    IRQn_Type irq_num; // UART IRQ Number

    hosal_uart_callback_t rx_cb;
    void* rx_cb_arg;

    hosal_uart_callback_t tx_cb;
    void* tx_cb_arg;

    hosal_uart_callback_t rxdma_cb;
    void* rxdma_cb_arg;

    hosal_uart_callback_t txdma_cb;
    void* txdma_cb_arg;

    hosal_uart_callback_t rx_line_status_cb;
    void* rx_line_status_cb_arg;

    hosal_uart_callback_t mode_cb;
    void* mode_cb_arg;   
} uart_handle_t;

static uart_handle_t g_uart_handle[MAX_NUMBER_OF_UART] = {
    {.uart = UART0,.irq_num = Uart0_IRQn,},
    {.uart = UART1,.irq_num = Uart1_IRQn,},
    {.uart = UART2,.irq_num = Uart2_IRQn,},
};

int hosal_uart_finalize(hosal_uart_dev_t* uart_dev) {


    int rval = 0;
    hosal_uart_config_t* cfg = & uart_dev->config;
    uart_t* uart;
    uart = g_uart_handle[cfg->uart_id].uart;

    NVIC_DisableIRQ(g_uart_handle[cfg->uart_id].irq_num);
    // Clear pending UART interrupts in NVIC
    NVIC_ClearPendingIRQ(g_uart_handle[cfg->uart_id].irq_num);
    

    uart->ier = 0; /*disable uart all interrupt*/

    uart->xdma_rx_enable = xDMA_Stop;
    uart->xdma_tx_enable = xDMA_Stop;

    uart->isr |= (ISR_RDA_INTR | ISR_THRE_INTR | ISR_RX_TIMEOUT | ISR_DMA_RX_INTR |
ISR_DMA_TX_INTR); /*write 1 to clear*/

    /*reset uart FIFO*/
    uart->fcr = 0;
    uart->fcr = FCR_CLEAR_XMIT | FCR_CLEAR_RCVR;

    uart->uart_en = UART_DISABLE;           /*Disablea  UART*/

    return rval;

}

int hosal_uart_init(hosal_uart_dev_t* uart_dev) {

    hosal_uart_config_t* cfg = & uart_dev->config;
    uart_t* uart;
    uint32_t cval;

        uart = g_uart_handle[cfg->uart_id].uart;
    
        if (cfg->uart_id == HOSAL_UART2_ID)
        {
            pin_set_mode(cfg->rx_pin, MODE_UART2_RX); 
            pin_set_mode(cfg->tx_pin, MODE_UART2_TX); 

            if((cfg->flow_control==UART_HWFC_ENABLED))
            {
                pin_set_mode(cfg->rts_pin, MODE_UART2_RTSN); 
                pin_set_mode(cfg->cts_pin, MODE_UART2_CTSN);              
            }
        }
        else if (cfg->uart_id == HOSAL_UART1_ID)
        {
            pin_set_mode(cfg->tx_pin, MODE_UART1_TX);
            pin_set_mode(cfg->rx_pin, MODE_UART1_RX);
            
            if((cfg->flow_control==UART_HWFC_ENABLED))
            {
                pin_set_mode(cfg->rts_pin, MODE_UART1_RTSN); 
                pin_set_mode(cfg->cts_pin, MODE_UART1_CTSN);              
            }      
        }
        else    //UART0 
        {
            pin_set_mode(cfg->rx_pin, MODE_UART0_RX); 
            pin_set_mode(cfg->tx_pin, MODE_UART0_TX); 
        } 
        NVIC_DisableIRQ(g_uart_handle[cfg->uart_id].irq_num);
        NVIC_SetPriority(g_uart_handle[cfg->uart_id].irq_num, 4);
        // Clear UART IRQ
        NVIC_ClearPendingIRQ(g_uart_handle[cfg->uart_id].irq_num);

        uart->uart_en = UART_DISABLE;           /*Disablea  UART*/

        //enable_perclk((UART0_CLK + cfg->uart_id));
        /*clear FIFO, REMAKR: FCR is write-only*/
        uart->fcr = 0;
        uart->fcr |= FCR_DEFVAL; /*reset FIFO*/

        // Disable interrupts
        uart->ier = 0;

        /*set baudrate*/
        uart->dlx = cfg->baud_rate & 0xFFFF ;
        uart->fdl = (cfg->baud_rate >> UART_BR_FRCT_SHIFT) & 0xFF;
           
        if (cfg->baud_rate & ENABLE_LSM)
        {
          uart->lsm = 1;
        }
       else
       {
          uart->lsm = 0;
       } 

        /*bits mode only use two bits.*/
        cval = cfg->data_width & 0x03;

        /*set stop bits*/
        if (cfg->stop_bits == UART_STOPBIT_TWO) {
            cval |= LCR_STOP;
        }

        /*set parity*/
        if (cfg->parity & PARENB) {
            cval |= LCR_PARITY;
        }
        if (!(cfg->parity & PARODD)) {
            cval |= LCR_EPAR;
        }
        if (cfg->parity & CMSPAR) {
            cval |= LCR_SPAR;
        }

        uart->lcr = cval;

        uart->mcr = 0; /*Initial default modem control register.*/
        uart->ier = 0; /*disable uart all interrupt*/

        uart->xdma_rx_enable = xDMA_Stop;
        uart->xdma_tx_enable = xDMA_Stop;

        uart->xdma_tx_len = 0;
        uart->xdma_rx_len = 0;

        /*clear uart all interrupt status*/
        uart->isr |= (ISR_RDA_INTR | ISR_THRE_INTR | ISR_RX_TIMEOUT | ISR_LSR_INTR| ISR_DMA_RX_INTR | ISR_DMA_TX_INTR);/*write 1 to clear*/

        /*enable uart rx line status */
        uart->ier |= UART_IER_RLSI;

        uart->lsr |= UART_LSR_BRK_ERROR_BITS;

        uart->fcr |= (FCR_TRIGGER_8|FCR_DMA_SELECT|FCR_DEFVAL);

         /*enable uart */
        uart->uart_en = UART_ENABLE;

        return (uart->rbr & 0xFF);
}

int hosal_uart_send(hosal_uart_dev_t* uart_dev, const void* data,
                uint32_t size) {

    uint32_t i = 0;
    hosal_uart_config_t* cfg = & uart_dev->config;
    uart_t* uart;
    uart = g_uart_handle[cfg->uart_id].uart;
    while (i < size) {
        while ((UART_ReadLineStatus(uart) & UART_LSR_THRE) == 0) {}
        uart->thr = ((uint8_t*)data)[i];
        i++;
    }


    return 0;
}

void hosal_uart_send_complete(hosal_uart_dev_t* uart_dev) {
    hosal_uart_config_t* cfg = &uart_dev->config;
    uart_t* uart;
    uart = g_uart_handle[cfg->uart_id].uart;

    while ((UART_ReadLineStatus(uart) & UART_LSR_TEMT) == 0) {}
}

inline int hosal_uart_receive(hosal_uart_dev_t* uart_dev, void* data,
                   uint32_t expect_size) {

    uint32_t counter = 0;
    hosal_uart_config_t* cfg = & uart_dev->config;

    uart_t* uart;
    uart = g_uart_handle[cfg->uart_id].uart;

    while (counter < expect_size) {
        if (uart->lsr & UART_LSR_DR) {
            ((uint8_t*)data)[counter] = uart->rbr & 0xFF;
        } else {
            break;
        }
        counter++;
    }

    return counter;
}

static inline void set_config_value(void* config_field, void* p_arg) {
    *(uint32_t*)config_field = (uint32_t)p_arg;
}

static inline void get_config_value(void* config_field, void* p_arg) {
    if (p_arg)
        *(uint32_t*)p_arg = *(uint32_t*)config_field;
}

inline uint32_t hosal_uart_get_lsr(hosal_uart_dev_t* uart_dev) {
    uint32_t status = 0;
    uart_t* uart = g_uart_handle[uart_dev->config.uart_id].uart;

    status = uart->lsr;
    return status;
}

uint32_t hosal_uart_get_dma_int_status(hosal_uart_dev_t* uart_dev) {
    uint32_t status = 0;
    uart_t* uart = g_uart_handle[uart_dev->config.uart_id].uart;

    status = (uart->isr& (ISR_DMA_TX_INTR|ISR_DMA_RX_INTR));
    return status;
}

int hosal_uart_ioctl(hosal_uart_dev_t* uart_dev, int ctl, void* p_arg) {
    hosal_uart_dma_cfg_t* dma_cfg;
    uart_t* uart = g_uart_handle[uart_dev->config.uart_id].uart;
    uint32_t tmp = 0;

    switch (ctl) {
        case HOSAL_UART_BAUD_SET:
            set_config_value(&uart_dev->config.baud_rate, p_arg);
            break;
        case HOSAL_UART_BAUD_GET:
            get_config_value(&uart_dev->config.baud_rate, p_arg);
            break;
        case HOSAL_UART_DATA_WIDTH_SET:
            set_config_value(&uart_dev->config.data_width, p_arg);
            break;
        case HOSAL_UART_DATA_WIDTH_GET:
            get_config_value(&uart_dev->config.data_width, p_arg);
            break;
        case HOSAL_UART_STOP_BITS_SET:
            set_config_value(&uart_dev->config.stop_bits, p_arg);
            break;
        case HOSAL_UART_STOP_BITS_GET:
            get_config_value(&uart_dev->config.stop_bits, p_arg);
            break;
        case HOSAL_UART_FLOWMODE_SET:
            set_config_value(&uart_dev->config.flow_control, p_arg);
            NVIC_DisableIRQ(g_uart_handle[uart_dev->config.uart_id].irq_num);
            NVIC_ClearPendingIRQ(
                g_uart_handle[uart_dev->config.uart_id].irq_num);
            if (p_arg) {
                uart->mcr = MCR_HW_AUTOFLOW_EN;
                uart->ier |= UART_IER_MSI;   
            } else {
                uart->mcr = 0;
                uart->ier &= ~(UART_IER_MSI);
            }
            NVIC_EnableIRQ(g_uart_handle[uart_dev->config.uart_id].irq_num); 
            break;
        case HOSAL_UART_FLOWSTAT_GET:
            get_config_value(&uart_dev->config.flow_control, p_arg);
            break;
        case HOSAL_UART_PARITY_SET:
            set_config_value(&uart_dev->config.parity, p_arg);
            break;
        case HOSAL_UART_PARITY_GET:
            get_config_value(&uart_dev->config.parity, p_arg);
            break;
        case HOSAL_UART_RECEIVE_LINE_STATUS_ENABLE:
            NVIC_DisableIRQ(g_uart_handle[uart_dev->config.uart_id].irq_num);
            NVIC_ClearPendingIRQ(
                g_uart_handle[uart_dev->config.uart_id].irq_num);
            uart->ier |= UART_IER_RLSI;
            NVIC_EnableIRQ(g_uart_handle[uart_dev->config.uart_id].irq_num);
            break;
        case HOSAL_UART_MODE_SET:
            uart->ier = 0;
            NVIC_ClearPendingIRQ(
                g_uart_handle[uart_dev->config.uart_id].irq_num);
            uart_dev->config.mode = (hosal_uart_mode_t)p_arg;

            switch (uart_dev->config.mode) {
                case HOSAL_UART_MODE_POLL: uart->ier = 0;break;
                case HOSAL_UART_MODE_INT_TX:
                    uart->ier |= (UART_IER_THRI);
                    NVIC_EnableIRQ(
                        g_uart_handle[uart_dev->config.uart_id].irq_num);
                    break;
                case HOSAL_UART_MODE_INT_RX:
                    uart->ier |= (UART_IER_RDI|UART_IER_RX_TIMEOUT);
                    NVIC_EnableIRQ(
                        g_uart_handle[uart_dev->config.uart_id].irq_num);
                    break;
                case HOSAL_UART_MODE_INT:
                    uart->ier |= (UART_IER_RDI|UART_IER_THRI|UART_IER_RX_TIMEOUT);
                    NVIC_EnableIRQ(
                        g_uart_handle[uart_dev->config.uart_id].irq_num);
                    break;
                case HOSAL_UART_DMA_MODE_POLL:
                     uart->fcr |= (FCR_DMA_SELECT);
                     break;
                case HOSAL_UART_DMA_MODE_INT_TX:
                    uart->fcr |= (FCR_DMA_SELECT);
                    uart->ier |= (UART_IER_DMA_TX_INTR );
                    NVIC_EnableIRQ(
                        g_uart_handle[uart_dev->config.uart_id].irq_num);
                    break;
                case HOSAL_UART_DMA_MODE_INT_RX:
                   uart->fcr |= (FCR_DMA_SELECT); 
                   uart->ier |= (UART_IER_DMA_RX_INTR);
                    NVIC_EnableIRQ(
                        g_uart_handle[uart_dev->config.uart_id].irq_num);
                    break;

                case HOSAL_UART_DMA_MODE_INT:
                    uart->fcr |= (FCR_DMA_SELECT);
                    uart->ier |= (UART_IER_DMA_TX_INTR | UART_IER_DMA_RX_INTR);
                    NVIC_EnableIRQ(
                        g_uart_handle[uart_dev->config.uart_id].irq_num);
                    break;                
            }
            break;
        case HOSAL_UART_MODE_GET:
            get_config_value(&uart_dev->config.mode, p_arg);
            break;
        case HOSAL_UART_TX_TRIGGER_ON:
        case HOSAL_UART_TX_TRIGGER_OFF: break;
        case HOSAL_UART_DMA_TX_START:
            dma_cfg = (hosal_uart_dma_cfg_t*)p_arg;
            uart->xdma_tx_addr = (uint32_t)dma_cfg->dma_buf;
            uart->xdma_tx_len = dma_cfg->dma_buf_size;
            uart->xdma_tx_enable = xDMA_Enable;
            break;
        case HOSAL_UART_DMA_RX_START: 
            dma_cfg = (hosal_uart_dma_cfg_t*)p_arg;
            uart->xdma_rx_addr = (uint32_t)dma_cfg->dma_buf;
            uart->xdma_rx_len = dma_cfg->dma_buf_size;
            uart->xdma_rx_enable = xDMA_Enable;        
            break;
        case HOSAL_UART_DMA_POL_TX_START:
            uart->xdma_tx_enable = xDMA_Stop;
            dma_cfg = (hosal_uart_dma_cfg_t*)p_arg;
            uart->xdma_tx_addr = (uint32_t)dma_cfg->dma_buf;
            uart->xdma_tx_len = dma_cfg->dma_buf_size;
            uart->xdma_tx_enable = xDMA_Enable;
            break;
        case HOSAL_UART_DMA_POL_RX_START: 
            uart->xdma_rx_enable = xDMA_Stop; 
            dma_cfg = (hosal_uart_dma_cfg_t*)p_arg;
            uart->xdma_rx_addr = (uint32_t)dma_cfg->dma_buf;
            uart->xdma_rx_len = dma_cfg->dma_buf_size;
            uart->xdma_rx_enable = xDMA_Enable;        
            break;            
        case HOSAL_UART_DISABLE_INTERRUPT:
            NVIC_DisableIRQ(g_uart_handle[uart_dev->config.uart_id].irq_num);
            break;
        case HOSAL_UART_ENABLE_INTERRUPT:
            NVIC_EnableIRQ(g_uart_handle[uart_dev->config.uart_id].irq_num);
            break;
        case HOSAL_UART_CLEAR_FIFO:
            tmp = uart->ier;
            uart->ier = 0;
            uart->fcr = 0;
            uart->fcr = FCR_DEFVAL; /*reset tx rx FIFO*/
            uart->ier = tmp;
            break;
        case HOSAL_UART_MODEM_RTS_SET:

            if (p_arg) {
                uart->mcr |= MCR_RTS;
            }else {
                uart->mcr &=~MCR_RTS;    
            }

            break;  
        case HOSAL_UART_CLEAR_DMA_STATUS:
             uart->isr = (uint32_t)p_arg; 
             break;
        case HOSAL_UART_SET_BREAK:

           if (p_arg) {
                uart->lcr |= LCR_SBC;
            } else {
               uart->lcr &=~ LCR_SBC;
            }
            break;
            
        case HOSAL_UART_SET_CLOCK_SOURCE:

            tmp = (hosal_uart_clock_src_t)p_arg;

            if (tmp == HOSAL_UART_CLK_SRC_RCO1M || tmp == HOSAL_UART_CLK_SRC_RCO32K)
            {
                PMU_CTRL->soc_pmu_rco1m.bit.en_rco_1m = 1;
            }

            SYSCTRL->sys_clk_ctrl1.reg = (SYSCTRL->sys_clk_ctrl1.reg & ~(HOSAL_UART_CLOCK_SOURCE_MASK << (uart_dev->config.uart_id << 1)));
            SYSCTRL->sys_clk_ctrl1.reg = (SYSCTRL->sys_clk_ctrl1.reg | (tmp << (uart_dev->config.uart_id << 1)));
            break;
        default: break;
    }

    return 0;
}

static inline void set_callback(hosal_uart_callback_t* dev_cb, void** dev_arg,
                                hosal_uart_callback_t pfn_callback, void* arg,
                                hosal_uart_callback_t* handle_cb,
                                void** handle_arg) {
    *dev_cb = pfn_callback;
    *dev_arg = arg;
    *handle_cb = pfn_callback;
    *handle_arg = arg;
}

int hosal_uart_callback_set(hosal_uart_dev_t* uart_dev, int callback_type,
                            hosal_uart_callback_t pfn_callback, void* arg) {
    hosal_uart_config_t* cfg = &uart_dev->config;

    switch (callback_type) {
        case HOSAL_UART_TX_CALLBACK:
            set_callback(&uart_dev->tx_cb, &uart_dev->p_tx_arg, pfn_callback,
                         arg, &g_uart_handle[cfg->uart_id].tx_cb,
                         &g_uart_handle[cfg->uart_id].tx_cb_arg);
            break;
        case HOSAL_UART_RX_CALLBACK:
            set_callback(&uart_dev->rx_cb, &uart_dev->p_rx_arg, pfn_callback,
                         arg, &g_uart_handle[cfg->uart_id].rx_cb,
                         &g_uart_handle[cfg->uart_id].rx_cb_arg);
            break;
        case HOSAL_UART_TX_DMA_CALLBACK:
            set_callback(&uart_dev->txdma_cb, &uart_dev->p_txdma_arg,
                         pfn_callback, arg,
                         &g_uart_handle[cfg->uart_id].txdma_cb,
                         &g_uart_handle[cfg->uart_id].txdma_cb_arg);
            break;
        case HOSAL_UART_RX_DMA_CALLBACK:
            set_callback(&uart_dev->rxdma_cb, &uart_dev->p_rxdma_arg,
                         pfn_callback, arg,
                         &g_uart_handle[cfg->uart_id].rxdma_cb,
                         &g_uart_handle[cfg->uart_id].rxdma_cb_arg);
            break;
        case HOSAL_UART_RECEIVE_LINE_STATUS_CALLBACK:
            set_callback(&uart_dev->rx_line_status_cb,
                         &uart_dev->p_rx_line_status_arg, pfn_callback, arg,
                         &g_uart_handle[cfg->uart_id].rx_line_status_cb,
                         &g_uart_handle[cfg->uart_id].rx_line_status_cb_arg);
            break;
        case HOSAL_UART_MODE_CALLBACK:
            set_callback(&uart_dev->mode_cb,
                         &uart_dev->p_mode_arg, pfn_callback, arg,
                         &g_uart_handle[cfg->uart_id].mode_cb,
                         &g_uart_handle[cfg->uart_id].mode_cb_arg);
            break;            
        default: return -1;
    }
    return 0;
}


static inline void handle_callback(hosal_uart_callback_t cb, void* cb_arg) {
    if (cb != NULL)
        cb(cb_arg);
}

static inline void __uart_generic_notify_handler(uint8_t id) {

    uart_t* uart = g_uart_handle[id].uart;
    uint32_t isr_status= (uart->isr);
    uint32_t lsr_status;
    uint32_t val;
    
    if ((isr_status &  ISR_LSR_INTR)) {

        handle_callback(g_uart_handle[id].rx_line_status_cb,
                        g_uart_handle[id].rx_line_status_cb_arg);

        lsr_status = uart->lsr;                
        uart->lsr = (lsr_status&UART_LSR_BRK_ERROR_BITS);    
        uart->isr |= (ISR_LSR_INTR); 
    }

    
    if ((isr_status &  ISR_RDA_INTR) || (isr_status &  ISR_RX_TIMEOUT)) {

         while((uart->lsr & UART_LSR_DR)){

            handle_callback(g_uart_handle[id].rx_cb,g_uart_handle[id].rx_cb_arg);
        }   

        if((isr_status &  ISR_RDA_INTR))
        {
           uart->isr |= (ISR_RDA_INTR); 
        }

        if((isr_status &  ISR_RX_TIMEOUT))
        {
           uart->isr |= (ISR_RX_TIMEOUT); 
        }
    }

    if (isr_status &  ISR_THRE_INTR) {

        if (uart->lsr & UART_LSR_THRE) {
            
             handle_callback(g_uart_handle[id].tx_cb,g_uart_handle[id].tx_cb_arg);
        }
        
        uart->isr |= ISR_THRE_INTR;
    }

    if ((isr_status & ISR_DMA_TX_INTR) == ISR_DMA_TX_INTR) {
        
        uart->xdma_tx_enable = xDMA_Stop;
        
        handle_callback(g_uart_handle[id].txdma_cb,g_uart_handle[id].txdma_cb_arg);

        uart->isr |= ISR_DMA_TX_INTR;
    }
    
    if ((isr_status & ISR_DMA_RX_INTR) == ISR_DMA_RX_INTR) {
        
        uart->xdma_rx_enable = xDMA_Stop;
        
        handle_callback(g_uart_handle[id].rxdma_cb,g_uart_handle[id].rxdma_cb_arg);

        uart->isr |= ISR_DMA_RX_INTR;
    }

    if ((isr_status & ISR_MSR_INTR) == ISR_MSR_INTR) {
            val = uart->msr;
            if(val & UART_MSR_DCTS) {
                
                uart->msr = UART_MSR_DCTS; //Clear MSR Interrupt status;
                handle_callback(g_uart_handle[id].mode_cb,g_uart_handle[id].mode_cb_arg);
            }

    }
}

void uart0_handler(void) {
    __uart_generic_notify_handler(0);
    NVIC_ClearPendingIRQ(Uart0_IRQn);
}

void uart1_handler(void) {
    __uart_generic_notify_handler(1);
    NVIC_ClearPendingIRQ(Uart1_IRQn);
}

void uart2_handler(void) {
    __uart_generic_notify_handler(2);
    NVIC_ClearPendingIRQ(Uart2_IRQn);
}
