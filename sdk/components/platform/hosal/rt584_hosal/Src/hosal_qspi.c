/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */
/**
 * \file            hosal_qspi.c
 * \brief           Hosal spi driver file
 *          
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "mcu.h"
#include "qspi.h"
#include "hosal_qspi.h"
#include "hosal_sysctrl.h"


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
static hosal_qspi_status_t
hosal_qspi_slave_select_set(hosal_qspi_dev_t* qspi_dev, void* p_arg);
static hosal_qspi_status_t
hosal_qspi_slave_select_get(hosal_qspi_dev_t* qspi_dev, void* p_arg);
static hosal_qspi_status_t
hosal_qspi_slave_polarity_set(hosal_qspi_dev_t* qspi_dev, void* p_arg);
static hosal_qspi_status_t
hosal_qspi_slave_polarity_get(hosal_qspi_dev_t* qspi_dev, void* p_arg);



static hosal_qspi_status_t hosal_qspi_pre_delay_count_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);
static hosal_qspi_status_t hosal_qspi_pre_delay_count_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);										   
static hosal_qspi_status_t hosal_qspi_inter_delay_count_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);
static hosal_qspi_status_t hosal_qspi_inter_delay_count_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) ;
static hosal_qspi_status_t hosal_qspi_post_delay_count_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg);
static hosal_qspi_status_t hosal_qspi_post_delay_count_get(hosal_qspi_dev_t* qspi_dev,
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
    [HOSAL_QSPI_PRE_DLY_SET] = hosal_qspi_pre_delay_count_set,
    [HOSAL_QSPI_PRE_DLY_GET] = hosal_qspi_pre_delay_count_get,
    [HOSAL_QSPI_INTER_DLY_SET] = hosal_qspi_inter_delay_count_set,
    [HOSAL_QSPI_INTER_DLY_GET] = hosal_qspi_inter_delay_count_get,
    [HOSAL_QSPI_POST_DLY_SET] = hosal_qspi_post_delay_count_set,
    [HOSAL_QSPI_POST_DLY_GET] = hosal_qspi_post_delay_count_get,
};

static hosal_qspi_status_t hosal_qspi_mode_set(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    qspi_dev->config.mode = *(hosal_qspi_mode_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_mode_get(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    *(hosal_qspi_mode_t*)p_arg = qspi_dev->config.mode;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_baud_set(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    qspi_dev->config.baud_rate = *(hosal_qspi_baudrate_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_baud_get(hosal_qspi_dev_t* qspi_dev,
                                               void* p_arg) {
    *(hosal_qspi_baudrate_t*)p_arg = qspi_dev->config.baud_rate;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_datawidth_set(hosal_qspi_dev_t* qspi_dev,
                                                    void* p_arg) {
    qspi_dev->config.data_width = *(hosal_qspi_bitsize_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_datawidth_get(hosal_qspi_dev_t* qspi_dev,
                                                    void* p_arg) {
    *(hosal_qspi_bitsize_t*)p_arg = qspi_dev->config.data_width;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_bitorder_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    qspi_dev->config.bit_order = *(hosal_qspi_bitorder_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_bitorder_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    *(hosal_qspi_bitorder_t*)p_arg = qspi_dev->config.bit_order;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_phase_set(hosal_qspi_dev_t* qspi_dev,
                                                void* p_arg) {
    qspi_dev->config.phase = *(hosal_qspi_phase_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_phase_get(hosal_qspi_dev_t* qspi_dev,
                                                void* p_arg) {
    *(hosal_qspi_phase_t*)p_arg = qspi_dev->config.phase;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_polarity_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    qspi_dev->config.polarity = *(hosal_qspi_polarity_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_polarity_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    *(hosal_qspi_polarity_t*)p_arg = qspi_dev->config.polarity;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_pre_delay_count_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    qspi_dev->config.pre_dly_count = *(hosal_qspi_polarity_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_pre_delay_count_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    *(uint8_t*)p_arg = qspi_dev->config.pre_dly_count;
    return HOSAL_QSPI_SUCCESS;
}
												   
static hosal_qspi_status_t hosal_qspi_inter_delay_count_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    qspi_dev->config.pre_dly_count = *(uint8_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_inter_delay_count_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    *(uint8_t*)p_arg = qspi_dev->config.inter_dly_count;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_post_delay_count_set(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    qspi_dev->config.pre_dly_count = *(uint8_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t hosal_qspi_post_delay_count_get(hosal_qspi_dev_t* qspi_dev,
                                                   void* p_arg) {
    *(uint8_t*)p_arg = qspi_dev->config.post_dly_count;
    return HOSAL_QSPI_SUCCESS;
}


static hosal_qspi_status_t
hosal_qspi_slave_select_set(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    qspi_dev->config.slave_select = *(hosal_qspi_slave_select_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t
hosal_qspi_slave_select_get(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    *(hosal_qspi_slave_select_t*)p_arg = qspi_dev->config.slave_select;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t
hosal_qspi_slave_polarity_set(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    qspi_dev->config.cs_polarity = *(hosal_qspi_cs_polarity_t*)p_arg;
    return HOSAL_QSPI_SUCCESS;
}

static hosal_qspi_status_t
hosal_qspi_slave_polarity_get(hosal_qspi_dev_t* qspi_dev, void* p_arg) {
    *(hosal_qspi_cs_polarity_t*)p_arg = qspi_dev->config.cs_polarity;
    return HOSAL_QSPI_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_deinit(hosal_qspi_dev_t* qspi_dev) {
    return HOSAL_QSPI_SUCCESS;
}


static hosal_qspi_status_t hosal_qspi_pin_init(hosal_qspi_dev_t* qspi_dev)
{

        hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;

        if(qspi_dev->config.mode==SPI_MASTER_MODE)
        {
            if((qspi_dev->config.data2==0) && (qspi_dev->config.data3==0))
            {
                if(qspi_dev->config.qspi_id==HOSAL_QSPI_ID_0)
                {   
                    hosal_pin_set_mode(qspi_dev->config.clk_pin, HOSAL_MODE_SPI0_MASTER_SCLK);  /*SPI SCLK*/
                    hosal_pin_set_mode(qspi_dev->config.cs_pin, HOSAL_MODE_SPI0_MASTER_CSN0);   /*SPI CS*/
                    hosal_pin_set_mode(qspi_dev->config.mosi_pin, HOSAL_MODE_SPI0_MASTER_MOSI); /*SPI DATA0*/
                            
                    #if defined(CONFIG_EPD_FOUR_BIT_MODE) || defined(CONFIG_EPD_THREE_BIT_MODE)                                //for support e-paper spi operation
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI0_MASTER_SDATA1); /*SPI DATA1*/
                    #else                    
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI0_MASTER_MISO); /*SPI DATA1*/
                    #endif
                   

                }
                else if(qspi_dev->config.qspi_id==HOSAL_QSPI_ID_1)
                {

                    hosal_pin_set_mode(qspi_dev->config.clk_pin, HOSAL_MODE_SPI1_MASTER_SCLK);  /*SPI SCLK*/
                    hosal_pin_set_mode(qspi_dev->config.cs_pin, HOSAL_MODE_SPI1_MASTER_CSN0);   /*SPI CS*/
                    hosal_pin_set_mode(qspi_dev->config.mosi_pin, HOSAL_MODE_SPI1_MASTER_MOSI); /*SPI DATA0*/

                    #if defined(CONFIG_EPD_FOUR_BIT_MODE) || defined(CONFIG_EPD_THREE_BIT_MODE)    //for support e-paper spi operation
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI1_MASTER_SDATA1); /*SPI DATA1*/
                    #else                    
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI1_MASTER_MISO); /*SPI DATA1*/
                    #endif

                } 
                else
                {
                      err = HOSAL_QSPI_INVALID_PARAM;
                }               
            }
            else
            {
                if(qspi_dev->config.qspi_id==HOSAL_QSPI_ID_0)
                {
                    hosal_pin_set_mode(qspi_dev->config.clk_pin, HOSAL_MODE_SPI0_MASTER_SCLK);  /*SPI SCLK*/
                    hosal_pin_set_mode(qspi_dev->config.cs_pin, HOSAL_MODE_SPI0_MASTER_CSN0);   /*SPI CS*/
                    hosal_pin_set_mode(qspi_dev->config.mosi_pin, HOSAL_MODE_SPI0_MASTER_SDATA0); /*SPI DATA0*/
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI0_MASTER_SDATA1); /*SPI DATA1*/
                    hosal_pin_set_mode(qspi_dev->config.data2, HOSAL_MODE_SPI0_MASTER_SDATA2); /*SPI DATA2*/
                    hosal_pin_set_mode(qspi_dev->config.data3, HOSAL_MODE_SPI0_MASTER_SDATA3); /*SPI DATA3*/                    
                }
                else if(qspi_dev->config.qspi_id==HOSAL_QSPI_ID_1)
                {
                    hosal_pin_set_mode(qspi_dev->config.clk_pin, HOSAL_MODE_SPI1_MASTER_SCLK);  /*SPI SCLK*/
                    hosal_pin_set_mode(qspi_dev->config.cs_pin, HOSAL_MODE_SPI1_MASTER_CSN0);   /*SPI CS*/
                    hosal_pin_set_mode(qspi_dev->config.mosi_pin, HOSAL_MODE_SPI1_MASTER_SDATA0); /*SPI DATA0*/
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI1_MASTER_SDATA1); /*SPI DATA1*/
                    hosal_pin_set_mode(qspi_dev->config.data2, HOSAL_MODE_SPI1_MASTER_SDATA2); /*SPI DATA2*/
                    hosal_pin_set_mode(qspi_dev->config.data3, HOSAL_MODE_SPI1_MASTER_SDATA3); /*SPI DATA3*/                    
                }
                else 
                {
                      err = HOSAL_QSPI_INVALID_PARAM;
                }    
            }
         }
         else  if(qspi_dev->config.mode==SPI_SLAVE_MODE)
         {
             if((qspi_dev->config.data2==0) && (qspi_dev->config.data3==0))
            {
                if(qspi_dev->config.qspi_id==HOSAL_QSPI_ID_0)
                {
                    hosal_pin_set_mode(qspi_dev->config.clk_pin, HOSAL_MODE_SPI0_SLAVE_SCLK);  /*SPI SCLK*/
                    hosal_pin_set_mode(qspi_dev->config.cs_pin, HOSAL_MODE_SPI0_SLAVE_CSN0);   /*SPI CS*/
                    hosal_pin_set_mode(qspi_dev->config.mosi_pin, HOSAL_MODE_SPI0_SLAVE_MOSI); /*SPI DATA0*/
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI0_SLAVE_MISO); /*SPI DATA1*/
                }
                else if(qspi_dev->config.qspi_id==HOSAL_QSPI_ID_1)
                {
                    hosal_pin_set_mode(qspi_dev->config.clk_pin, HOSAL_MODE_SPI1_SLAVE_SCLK);  /*SPI SCLK*/
                    hosal_pin_set_mode(qspi_dev->config.cs_pin, HOSAL_MODE_SPI1_SLAVE_CSN0);   /*SPI CS*/
                    hosal_pin_set_mode(qspi_dev->config.mosi_pin, HOSAL_MODE_SPI1_SLAVE_MOSI); /*SPI DATA0*/
                    hosal_pin_set_mode(qspi_dev->config.miso_pin, HOSAL_MODE_SPI1_SLAVE_MISO); /*SPI DATA1*/
                } 
                else
                {
                      err = HOSAL_QSPI_INVALID_PARAM;
                }               
            }
            else
            {
               err = HOSAL_QSPI_INVALID_PARAM;   
            }
         }
         else
         {

             err = HOSAL_QSPI_INVALID_PARAM;

         }

        return err;
}




hosal_qspi_status_t hosal_qspi_init(hosal_qspi_dev_t* qspi_dev) {
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
    uint32_t control_reg = 0;
    uint32_t qspi_id;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    do {
        id = hosal_qspi_id_get(qspi_dev);
        ptr = hosal_qspi_handle_get(id);

        if (id == 0) {
            qspi_id = HOSAL_QSPI_ID_0;
            qspi_dev->irq_num = Qspi0_IRQn;
            qspi_dev->config.qspi_id = 0;
        } else {
            qspi_id = HOSAL_QSPI_ID_1;
            qspi_dev->irq_num = Qspi1_IRQn;
            qspi_dev->config.qspi_id = 1;
        }
		
		qspi_dev->config.pre_dly = HOSAL_QSPI_PRE_DEALY_DISABLE;
		qspi_dev->config.inter_dly = HOSAL_QSPI_INTER_DEALY_DISABLE;	
		qspi_dev->config.post_dly = HOSAL_QSPI_POST_DEALY_DISABLE;

        memcpy(ptr, qspi_dev, sizeof(hosal_qspi_dev_t));

        hosal_qspi_pin_init(qspi_dev);

        ptr->qspi_state = HOSAL_QSPI_STATE_SETUP;


        if ((ptr->config.baud_rate == QSPI_CLK_32M) && (hosal_get_periperhal_clock() == HOSAL_PERIPHERAL_CLK_32M)) 
        {
            /*slave can not run AHB clock*/
            ptr->instance->qspi_clkdiv = (ptr->config.mode == SPI_SLAVE_MODE) ? QSPI_CLK_16M : QSPI_CLK_32M ;
        }
        else 
        {

            ptr->instance->qspi_clkdiv = ptr->config.baud_rate;
        }

        ptr->instance->qspi_en = 0; /*clear FIFO.*/

        /*read Enable Register, it should be 0 before next step.*/
        while (ptr->instance->qspi_en) {}

        ptr->instance->qspi_int_clr = 0xFF; /*clear all interrupt flag*/
        ptr->instance->qspi_int_en = 0x0;   /*disable all interrupt*/

#if defined(SUPPORT_QSPI0_MULTI_CS)
//        /*Set qspi device SS pol --- CS active High or active Low setting*/
//        ptr->instance->QSPI_SLAVE_SEL_POL =
//            ((ptr->config.cs_polarity & ~(1 << ptr->config.slave_select))
//             | (ptr->config.cs_polarity << ptr->config.slave_select));
     /*
     *  there are multi spi slave device, so we should set slave device
     * polarity and slave index carefully.
     *  another method is create a global variable to save ss polarity
     */
    /*set ss polarity*/
    ptr->instance->qspi_ss_config = (ptr->instance->qspi_ss_config & ~(1 << (ptr->config.slave_select + QSPI_CFG_SSPOL_SHIFT)) |
                            (ptr->config.cs_polarity << (ptr->config.slave_select + QSPI_CFG_SSPOL_SHIFT));
                            /*set slave select output enable*/
    ptr->instance->qspi_ss_config = (ptr->instance->qspi_ss_config & ~(QSPI_SSOUT_MASK)) | (1 << ptr->config.slave_select);             
#else

    /*Set qspi device SS pol --- only one spi device. so ignore other chip select*/
    ptr->instance->qspi_ss_config = (ptr->config.cs_polarity << (ptr->config.slave_select + QSPI_CFG_SSPOL_SHIFT)) |
                           (1 << ptr->config.slave_select);
	
#endif

        if(ptr->config.mcs_en==1)
        {
         ptr->instance->qspi_ss_config &=~ (1<<(ptr->config.slave_select + QSPI_CFG_SS_MANUAL_SHIFT));
         ptr->instance->qspi_ss_config |= (1<<(ptr->config.slave_select + QSPI_CFG_SS_MANUUAL_ENABLE_SHIFT));

        }
        /*Set qspi transfer mode, here we assume bitsize is 8 bit first, Normal SPI mode*/
        ptr->instance->qspi_control2 = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI
                                  | QSPI_DISABLE_IN;

        /*clear native DMA register.*/
        ptr->instance->dma_ier = 0;
        ptr->instance->dma_int_status =
            QSPI_DMA_ISR_CLEARALL; /*clear all interrupt*/
        ptr->instance->dma_rx_enable = 0;
        ptr->instance->dma_tx_enable = 0;

        /* Enable DMA RX interrupt */
        //ptr->instance->DMA_IER = QSPI_DMA_IER_TX | QSPI_DMA_IER_RX;
        ptr->instance->dma_ier = QSPI_DMA_IER_RX;

//        /*For native SPI DMA, watermark is 8 entry only*/
//        control_reg = QSPI_CNTL_TX_1_4_WATERMARK | QSPI_CNTL_RX_1_4_WATERMARK
//                      | (ptr->config.mode << QSPI_CNTL_MASTER_SHIFT) | (ptr->config.polarity << 4)
//                      | (ptr->config.phase << 3) | (ptr->config.bit_order << 2)
//                      | QSPI_CNTL_LITTLE_ENDIAN;


		control_reg = QSPI_CNTL_TX_1_4_WATERMARK | QSPI_CNTL_RX_1_4_WATERMARK |
				  (ptr->config.mode << QSPI_CNTL_MASTER_SHIFT) |
				  (ptr->config.polarity << QSPI_CNTL_CPOL_SHIFT) |
				  (ptr->config.phase << QSPI_CNTL_CPHA_SHIFT) |
				  (ptr->config.bit_order << QSPI_CNTL_MSB_SHIFT) |
				  (ptr->config.pre_dly << QSPI_CNTL_PREDELAY_SHIFT) |
				  (ptr->config.inter_dly << QSPI_CNTL_INTER_DELAY_SHIFT) |
				  (ptr->config.post_dly << QSPI_CNTL_POSTDELAY_SHIFT) |
				  QSPI_CNTL_LITTLE_ENDIAN;
				  

		/*set Delay.*/
		ptr->instance->qspi_delays = (ptr->config.post_dly_count << QSPI_POST_DELAY_SHIFT) |
				(ptr->config.inter_dly_count << QSPI_INTER_DELAY_SHIFT) |
				(ptr->config.pre_dly_count << QSPI_PRE_DELAY_SHIFT) ;
									
									
        /* This is for normal SPI slave setting.
        * For ptr->instance, data_N connect data_N, but for normal SPI
        * MOSI and MISO is cross connect pin.
        * So we need a bit to control our internal circuit switch.
        */
        ptr->instance->qspi_control = control_reg
                                      | ((ptr->config.mode)
                                             ? QSPI_CNTL_contXfer_En
                                             : SPI_CNTL_SLAVE_SDATA_OUT);




        /*Enable ptr->instance*/
        ptr->instance->qspi_en = 1;
        ptr->qspi_state = HOSAL_QSPI_STATE_IDLE;
    } while (0);

    return err;
}

hosal_qspi_status_t hosal_qspi_transfer_dma(hosal_qspi_dev_t* qspi_dev,
                                            uint8_t* txbuf, uint8_t* rxbuf,
                                            uint16_t size) {
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
    uint32_t contrl2_reg=0;
    uint8_t dummy_enable = 0;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM, size == 0);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     (qspi_dev == NULL) || (txbuf == NULL) || (rxbuf == NULL));
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     ptr->config.qspi_id >= MAX_NUMBER_OF_QSPI);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     ptr->qspi_state != QSPI_STATE_IDLE);

    ptr->qspi_state = HOSAL_QSPI_STATE_TRANSFER;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    /*Notice: it is possible that control register need change too!*/
    ptr->instance->qspi_ss_config = (ptr->instance->qspi_ss_config & ~(QSPI_SSOUT_MASK)) |
                           (1 << ptr->config.slave_select) ;
#endif

    ptr->tx_buf = txbuf;
    ptr->data_length = size;
    ptr->rx_buf = rxbuf;
    ptr->data_length = size;

    /*Auto set transfer FIFO mode.*/
    if (ptr->data_length & 3) {
        /*length is not 4N data*/

        contrl2_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI
                  | ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                         ? (QSPI_Xfer_Extend)
                         : (0));
    } else {
        /*length is 4N data*/
        /*slave mode only use 8 bytes to transfer*/
        contrl2_reg = QSPI_NORMAL_SPI
                  | ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                         ? (QSPI_BITSIZE_32 | QSPI_Xfer_Extend)
                         : (QSPI_BITSIZE_8));
    }

    ptr->instance->qspi_int_clr = 0xFF; /*clear all interrupt flag*/

    if (ptr->rx_buf == NULL) {
        /*Client does care read data, "WRITE-ONLY".*/
        contrl2_reg |= QSPI_DISABLE_IN;

        /*enable transfer done interrupt*/
        ptr->instance->qspi_int_en = QSPI_INT_xferDone;
    }

    if (ptr->tx_buf == NULL) {
        if ((ptr->instance->qspi_control & QSPI_CNTL_MASTER) == 0) {
            /*This is slave mode, the slave is only receives data from
              master only. Setting this bit to avoid TX FIFO underflow
              in slave. (Master can not set this condition)
             */
            /*TODO: Test is slave mode*/
            contrl2_reg |= QSPI_DISABLE_OUT; /*required?*/
        } else {
            /*master will send dummy data*/
            dummy_enable = 1;
        }
    }

    ptr->instance->qspi_control2 = contrl2_reg;

    if (ptr->rx_buf != NULL) {
        ptr->instance->dma_rx_addr = (uint32_t)ptr->rx_buf;
        ptr->instance->dma_rx_len = ptr->data_length;

        ptr->instance->dma_rx_enable =
            QSPI_DMA_ENABLE | ((dummy_enable) ? QSPI_DMA_Dummy_ENABLE : 0);
    }

    if (ptr->tx_buf != NULL) {
        ptr->instance->dma_tx_addr = (uint32_t)ptr->tx_buf;
        ptr->instance->dma_tx_len = ptr->data_length;
        ptr->instance->dma_tx_enable = QSPI_DMA_ENABLE;
    }

    /*2022/04/28 add.. patch for slave, and host stop before dma request length*/
    if ((ptr->instance->qspi_control & QSPI_CNTL_MASTER) == 0) {
        ptr->instance->qspi_int_en =
            QSPI_INT_xferDone; /*avoid host send short data.*/
    }
    return err;
}

hosal_qspi_status_t hosal_qspi_flash_command(hosal_qspi_dev_t* qspi_dev,
                                             hosal_qspi_flash_command_t* req) {
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
    uint32_t remain_len, step,control2_reg;
    uint8_t *cmd_ptr, *write_ptr, *read_ptr;

    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM, req->cmd_length == 0);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     (qspi_dev == NULL) || (req == NULL));
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     ptr->config.qspi_id >= MAX_NUMBER_OF_QSPI);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     ptr->qspi_state != QSPI_STATE_IDLE);

    ptr->qspi_state = HOSAL_QSPI_STATE_TRANSFER;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    /*Notice: it is possible that control register need change too!*/
    ptr->instance->qspi_ss_config = (ptr->instance->qspi_ss_config & ~(QSPI_SSOUT_MASK)) |
                           (1 << ptr->config.slave_select) ;
#endif

    control2_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_DISABLE_IN
              | QSPI_Xfer_Extend;
    ptr->instance->qspi_control2 = control2_reg;

    cmd_ptr = req->cmd_buf;
    remain_len = req->cmd_length;
    while (remain_len > 0) {
        if (!(ptr->instance->qspi_status & QSPI_STATUS_txFull)) {
            ptr->instance->qspi_tx_fifo = *cmd_ptr++;
            remain_len--;
        }
    }
    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {}

    if ((req->write_length == 0) && (req->read_length == 0)) {
        hosal_qspi_idle(ptr);
        return STATUS_SUCCESS;
    }

    if ((req->write_length != 0) && (req->write_buf != NULL)) {
        write_ptr = req->write_buf;
        remain_len = req->write_length;
        while (remain_len > 0) {
            if (ptr->instance->qspi_status & QSPI_STATUS_txEmpty) {
                if (remain_len > 3) {
                    ptr->instance->qspi_control2 = req->transfer_mode
                                              | QSPI_DISABLE_IN
                                              | QSPI_Xfer_Extend
                                              | QSPI_BITSIZE_32;
                    ptr->instance->qspi_tx_fifo = *(uint32_t*)write_ptr;
                    write_ptr += 4;
                    remain_len -= 4;
                } else {
                    ptr->instance->qspi_control2 = req->transfer_mode
                                              | QSPI_DISABLE_IN
                                              | QSPI_Xfer_Extend
                                              | QSPI_BITSIZE_8;
                    ptr->instance->qspi_tx_fifo = *(uint8_t*)write_ptr;
                    write_ptr += 1;
                    remain_len -= 1;
                }
            }
        }
    } else if ((req->read_length != 0) && (req->read_buf != NULL)) {
        read_ptr = req->read_buf;
        remain_len = req->read_length;
        while (remain_len > 0) {
            if (remain_len > 3) {
                ptr->instance->qspi_control2 = req->transfer_mode | QSPI_BITSIZE_32
                                          | QSPI_Xfer_Extend;
                step = 4;
            } else {
                ptr->instance->qspi_control2 = req->transfer_mode | QSPI_BITSIZE_8
                                          | QSPI_Xfer_Extend;
                step = 1;
            }
            if (ptr->instance->qspi_status & QSPI_STATUS_txEmpty) {
                ptr->instance->qspi_tx_fifo = 0;
            }
            while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {}
            if (!(ptr->instance->qspi_status & QSPI_STATUS_rxEmpty)) {
                if (remain_len > 3) {
                    *(uint32_t*)read_ptr = ptr->instance->qspi_rx_fifo;
                } else {
                    *(uint8_t*)read_ptr = ptr->instance->qspi_rx_fifo;
                }
                read_ptr += step;
                remain_len -= step;
            }
        }
    }
    hosal_qspi_idle(ptr);
    return err;
}

hosal_qspi_status_t
hosal_qspi_tranfer_command(hosal_qspi_dev_t* qspi_dev,
                           hosal_qspi_flash_command_t* req) {
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
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
    /*Notice: it is possible that control register need change too!*/
    ptr->instance->qspi_ss_config = (ptr->instance->qspi_ss_config & ~(QSPI_SSOUT_MASK)) |
                           (1 << ptr->config.slave_select) ;
#endif

    /*Set Aux, During comamnd phase, we don't hope to get RX */
    ptr->instance->qspi_control2 = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_DISABLE_IN
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
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
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
        #if defined(CONFIG_FREERTOS)
        taskENTER_CRITICAL();
        #else
        enter_critical_section();
        #endif
    err = hosal_qspi_tranfer_command(ptr, req);

        #if defined(CONFIG_FREERTOS)
        taskEXIT_CRITICAL();
        #else
        leave_critical_section();
        #endif

    HOSAL_QSPI_CHECK(err, err != HOSAL_QSPI_SUCCESS);

    /*only enable TX */
    ptr->instance->dma_tx_addr = (uint32_t)(req->write_buf);
    ptr->instance->dma_tx_len = req->write_length;

    /*we wait TX_FIFO_Empty and transfer not in progress for command here.*/

    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {}
    /*wait txEmpty and xferIP done*/

    ptr->instance->qspi_control2 = (req->transfer_mode)
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
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
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
        #if defined(CONFIG_FREERTOS)
        taskENTER_CRITICAL();
        #else
        enter_critical_section();
        #endif
    err = hosal_qspi_tranfer_command(ptr, req);
        #if defined(CONFIG_FREERTOS)
        taskEXIT_CRITICAL();
        #else
        leave_critical_section();
        #endif

    HOSAL_QSPI_CHECK(err, err != HOSAL_QSPI_SUCCESS);

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

    ptr->instance->qspi_control2 = (req->transfer_mode)
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
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
    uint32_t control2_reg;
    uint32_t tickstart = 0;

    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    #if defined(CONFIG_FREERTOS)
    tickstart = xTaskGetTickCount();
    #else
    tickstart = 0;
    #endif
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM, size == 0);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     (qspi_dev == NULL) || (txbuf == NULL) || (rxbuf == NULL));
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     ptr->config.qspi_id >= MAX_NUMBER_OF_QSPI);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     ptr->qspi_state != QSPI_STATE_IDLE);

    ptr->qspi_state = HOSAL_QSPI_STATE_TRANSFER;

#if defined(SUPPORT_QSPI0_MULTI_CS)

    /*Notice: it is possible that control register need change too!*/
    ptr->instance->qspi_ss_config = (ptr->instance->qspi_ss_config & ~(QSPI_SSOUT_MASK)) |
                           (1 << ptr->config.slave_select) ;
						   
#endif


    control2_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI
              | ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                     ? QSPI_Xfer_Extend
                     : 0);

    ptr->instance->qspi_control2 = control2_reg;
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

        #if defined(CONFIG_FREERTOS)
        if (xTaskGetTickCount() - tickstart >= timeout) {
            hosal_qspi_abort(ptr);
            return HOSAL_QSPI_TIMEOUT;
        }
        #endif
    }

    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {
        
        #if defined(CONFIG_FREERTOS)
        if (xTaskGetTickCount() - tickstart >= timeout) {
            hosal_qspi_abort(ptr);
            return HOSAL_QSPI_TIMEOUT;
        }
        #endif

    }
    /*disable CS ASAP*/
    control2_reg &= ~(QSPI_Xfer_Extend);
    ptr->instance->qspi_control2 = control2_reg; /*Disable contXferExtend and stop RX in*/

    ptr->qspi_state = QSPI_STATE_IDLE;

    return err;
}

hosal_qspi_status_t hosal_qspi_transfer_pio_epd(hosal_qspi_dev_t* qspi_dev,
                                            uint8_t* txbuf, uint8_t* rxbuf,
                                            uint16_t size, uint32_t timeout)
{
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
    uint32_t control2_reg;
    uint32_t tickstart = 0;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

#if defined(CONFIG_FREERTOS)
    tickstart = xTaskGetTickCount();
#endif

    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM, qspi_dev == NULL);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM, txbuf == NULL);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM, size == 0);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_PARAM,
                     ptr->config.qspi_id >= MAX_NUMBER_OF_QSPI);
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     ptr->qspi_state != QSPI_STATE_IDLE);

    /* only support master pio */
    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     !(ptr->instance->qspi_control & QSPI_CNTL_MASTER));

    ptr->qspi_state = HOSAL_QSPI_STATE_TRANSFER;

#if defined(SUPPORT_QSPI0_MULTI_CS)
    ptr->instance->qspi_ss_config =
        (ptr->instance->qspi_ss_config & ~(QSPI_SSOUT_MASK)) |
        (1 << ptr->config.slave_select);
#endif

#if defined(CONFIG_EPD_FOUR_BIT_MODE) || defined(CONFIG_EPD_THREE_BIT_MODE)
    control2_reg = QSPI_BITSIZE_16 | QSPI_NORMAL_SPI | QSPI_Xfer_Extend;
#else
    control2_reg = QSPI_BITSIZE_8 | QSPI_NORMAL_SPI | QSPI_Xfer_Extend;
#endif

    if (rxbuf == NULL) {
        control2_reg |= QSPI_DISABLE_IN;
    }

    ptr->instance->qspi_control2 = control2_reg;
    ptr->transfer_cb = NULL;

#if defined(CONFIG_EPD_FOUR_BIT_MODE)
    ptr->instance->qspi_epd_func.bit.cfg_short_cycle_en = 1;
    ptr->instance->qspi_epd_func.bit.cfg_short_cycle_num = 7;
    ptr->instance->qspi_epd_func.bit.cfg_direct_bit_en = 1;
    ptr->instance->qspi_epd_func.bit.cfg_direct_bit_num = 8;
#elif defined(CONFIG_EPD_THREE_BIT_MODE)
    ptr->instance->qspi_epd_func.bit.cfg_short_cycle_en = 1;
    ptr->instance->qspi_epd_func.bit.cfg_short_cycle_num = 8;
    ptr->instance->qspi_epd_func.bit.cfg_direct_bit_en = 0;
    ptr->instance->qspi_epd_func.bit.cfg_direct_bit_num = 8;
#endif

#if defined(CONFIG_EPD_FOUR_BIT_MODE) || defined(CONFIG_EPD_THREE_BIT_MODE)
    {
        uint16_t remain = size;
        uint8_t *p = txbuf;

        while (remain > 0) {
            if (!(ptr->instance->qspi_status & QSPI_STATUS_txFull)) {
                ptr->instance->qspi_tx_fifo = *(uint16_t *)p;
                p += 2;
                remain -= 2;
            }

#if defined(CONFIG_FREERTOS)
            if (xTaskGetTickCount() - tickstart >= timeout) {
                hosal_qspi_abort(ptr);
                return HOSAL_QSPI_TIMEOUT;
            }
#endif
        }
    }
#else
    {
        uint16_t remain = size;
        uint8_t *p = txbuf;

        while (remain > 0) {
            if (!(ptr->instance->qspi_status & QSPI_STATUS_txFull)) {
                ptr->instance->qspi_tx_fifo = *p++;
                remain--;
            }

#if defined(CONFIG_FREERTOS)
            if (xTaskGetTickCount() - tickstart >= timeout) {
                hosal_qspi_abort(ptr);
                return HOSAL_QSPI_TIMEOUT;
            }
#endif
        }
    }
#endif

    while (!(ptr->instance->qspi_status & QSPI_STATUS_AllCmdDone)) {
#if defined(CONFIG_FREERTOS)
        if (xTaskGetTickCount() - tickstart >= timeout) {
            hosal_qspi_abort(ptr);
            return HOSAL_QSPI_TIMEOUT;
        }
#endif
    }

    control2_reg &= ~QSPI_Xfer_Extend;
    ptr->instance->qspi_control2 = control2_reg;

    if (rxbuf != NULL) {
#if defined(CONFIG_EPD_FOUR_BIT_MODE) || defined(CONFIG_EPD_THREE_BIT_MODE)
        uint16_t i = 0;
        while (!(ptr->instance->qspi_status & QSPI_STATUS_rxEmpty)) {
            *(uint16_t *)(rxbuf + i) = ptr->instance->qspi_rx_fifo;
            i += 2;
        }
#else
        uint8_t *r = rxbuf;
        while (!(ptr->instance->qspi_status & QSPI_STATUS_rxEmpty)) {
            *r++ = ptr->instance->qspi_rx_fifo;
        }
#endif
    }

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
    return HOSAL_QSPI_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_idle(hosal_qspi_dev_t* qspi_dev) {
    uint32_t contrl2_reg = 0;
    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    contrl2_reg = ptr->instance->qspi_control2;
    contrl2_reg = contrl2_reg & ~(QSPI_Xfer_Extend);
    ptr->instance->qspi_control2 = contrl2_reg; /*Disable contXferExtend and stop RX in*/

    ptr->qspi_state = QSPI_STATE_IDLE;
    return HOSAL_QSPI_SUCCESS;
}

hosal_qspi_status_t hosal_qspi_abort(hosal_qspi_dev_t* qspi_dev) {
    hosal_qspi_status_t err = HOSAL_QSPI_SUCCESS;
	uint32_t contrl2_reg = 0;

    hosal_qspi_dev_t* ptr;
    uint8_t id;

    id = hosal_qspi_id_get(qspi_dev);
    ptr = hosal_qspi_handle_get(id);

    HOSAL_QSPI_CHECK(HOSAL_QSPI_INVALID_REQUEST,
                     (ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                         != QSPI_CNTL_SLAVE);

    contrl2_reg = ptr->instance->qspi_control2;
    contrl2_reg = contrl2_reg & ~(QSPI_Xfer_Extend);
    ptr->instance->qspi_control2 = contrl2_reg; /*Disable contXferExtend and stop RX in*/

    /*let devcie to idle mode.*/
    ptr->instance->dma_tx_enable = 0;
    ptr->instance->dma_rx_enable = 0;

    ptr->instance->dma_tx_len = 0;
    ptr->instance->dma_rx_len = 0;
    ptr->instance->qspi_en = 0; /*clear FIFO.*/

    /*read Enable Register, it should be 0 before next step.*/
    while (ptr->instance->qspi_en)
        ;

    /*Enable QSPI*/
    ptr->instance->qspi_en = 1;
    ptr->instance->qspi_int_clr = 0xFF; /*clear all interrupt flag*/
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
    hosal_qspi_dev_t* ptr;

    ptr = hosal_qspi_handle_get(id);
    if (ptr->instance->dma_int_status & QSPI_DMA_ISR_RX) {
        /*RX complete, clear RX dma status*/
        /*if TX DMA is enable, we also clear it at the same time*/
        ptr->instance->dma_int_status = (QSPI_DMA_ISR_RX | QSPI_DMA_ISR_TX);
        ptr->instance->dma_rx_enable = 0;
        ptr->instance->dma_tx_enable =
            0; /*For the case, SPI normal mode, TX RX start for this operation*/

        ptr->instance->qspi_int_clr = QSPI_INT_xferDone;
        /*master required, we should complete the transfer, for slave, it is the same setting*/
        ptr->instance->qspi_control2 = ptr->instance->qspi_control2 & ~(QSPI_Xfer_Extend);

        ptr->qspi_state = QSPI_STATE_IDLE;
        /*notify callback*/
        if (ptr->transfer_cb != NULL) {
            ptr->transfer_cb(ptr->p_transfer_arg);
        }
    }
    if (ptr->instance->qspi_int_status & QSPI_INT_xferDone) {
        /*clear status*/
        ptr->instance->qspi_int_clr = QSPI_INT_xferDone;
        /*only care TX DMA. (RX DMA will be finished in QSPI_DMA_ISR_RX)*/
        if (ptr->qspi_state == QSPI_STATE_WRITE_DMA) {
            /*TX complete, clear TX dma status*/
            ptr->instance->dma_int_status = QSPI_DMA_ISR_TX;
            ptr->instance->dma_tx_enable = 0;
            /*we are master, we should complete the transfer*/
            ptr->instance->qspi_control2 = ptr->instance->qspi_control2
                                      & ~(QSPI_Xfer_Extend);
            ptr->instance->qspi_int_en = 0; /*disable self interrupt*/

            /* transfer done, because we are in priority interrupt,
            * so we don't need critical section protect.
            */
            /*only TX... we should notify finish.*/
            ptr->qspi_state = QSPI_STATE_IDLE;
            /*notify callback*/
            if (ptr->transfer_cb != NULL) {
                ptr->transfer_cb(ptr->p_transfer_arg);
            }
        } else {
            /* 2022/04/28... this is specail case for slave mode...
             * host send data short data and stop CS...
             */
            if ((ptr->instance->qspi_control & QSPI_CNTL_MASTER)
                == QSPI_CNTL_SLAVE) {
                /*slave see CS inactive... stop to request*/
                if ((ptr->instance->dma_rx_enable != 0)
                    || (ptr->instance->dma_tx_enable != 0)) {
                    /*master abort command? error?*/
                    ptr->instance->dma_tx_enable = 0;
                    ptr->instance->dma_rx_enable = 0;
                    ptr->instance->dma_tx_len = 0;
                    ptr->instance->dma_rx_len = 0;
                    ptr->instance->qspi_int_clr =
                        0xFF;                   /*clear all interrupt flag*/
                    ptr->instance->qspi_en = 0; /*clear FIFO.*/

                    /*read Enable Register, it should be 0 before next step.*/
                    while (ptr->instance->qspi_en) {}

                    /*Enable QSPI*/
                    ptr->instance->qspi_en = 1;
                    ptr->qspi_state = QSPI_STATE_IDLE;
                    if (ptr->transfer_cb != NULL) {
                        ptr->transfer_cb(ptr->p_transfer_arg);
                    }
                }
            }
        }
    }
}

void qspi0_handler(void) { hosal_qspi_isr_notification(0); }

void qspi1_handler(void) { hosal_qspi_isr_notification(1); }
