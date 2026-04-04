/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2s.c
 * \brief           I2S driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "i2s.h"
#include "mcu.h"


static i2s_cb_fn i2s_reg_handler = NULL;

uint32_t i2s_callback_register(i2s_cb_fn i2s_int_callback) {
    i2s_reg_handler = i2s_int_callback;

    return STATUS_SUCCESS;
}

uint32_t i2s_init(i2s_para_set_t* i2s_para) {
    i2s_t* i2s = I2S0;
    i2s_xdma_ctrl_ptr_t* i2s_xdma_config;
    i2s_mclk_isel_t cfg_imck = I2S_MCLK_ISEL_MAX;

    i2s_reg_handler = NULL;

    if ((i2s_para->fmt >= I2S_FMT_MAX) | (i2s_para->sr >= I2S_SR_MAX)) {
        return STATUS_INVALID_PARAM;
    }

    i2s->ms_ctl1.bit.i2s_reset = 1;

    // Check PLL first
    // Set iMCLK
    switch (SystemCoreClock) {
        /* 32MHz */
        case (32000000UL):
            if (i2s_para->imck_rate == I2S_IMCLK_12P288M) {
                cfg_imck = I2S_MCLK_ISEL_0;
            } else if (i2s_para->imck_rate == I2S_IMCLK_8P192M) {
                cfg_imck = I2S_MCLK_ISEL_1;
            }
            break;
        /* 48MHz */
        case (48000000UL):
            if (i2s_para->imck_rate == I2S_IMCLK_12P288M) {
                cfg_imck = I2S_MCLK_ISEL_2;
            } else if (i2s_para->imck_rate == I2S_IMCLK_8P192M) {
                cfg_imck = I2S_MCLK_ISEL_3;
            }
            break;
        /* 64MHz */
        case (64000000UL):
            if (i2s_para->imck_rate == I2S_IMCLK_24P576M) {
                cfg_imck = I2S_MCLK_ISEL_4;
            } else if (i2s_para->imck_rate == I2S_IMCLK_16P384M) {
                cfg_imck = I2S_MCLK_ISEL_5;
            }

            break;
        default: break;
    }

    if (cfg_imck == I2S_MCLK_ISEL_MAX) {
        return STATUS_INVALID_PARAM;
    }
    i2s->mclk_set0.bit.mclk_isel = cfg_imck;

    i2s->mclk_set1.bit.mclk_div = i2s_para->mck_div;

    i2s->ms_set0.bit.cfg_bck_osr = i2s_para->bck_osr;
    i2s->ms_set0.bit.cfg_i2s_mod = i2s_para->trx_mode;
    i2s->ms_set0.bit.cfg_i2s_fmt = i2s_para->fmt;
    i2s->ms_set0.bit.cfg_bck_len = i2s_para->bck_ratio;
    i2s->ms_set0.bit.cfg_txd_wid = i2s_para->width;
    i2s->ms_set0.bit.cfg_rxd_wid = i2s_para->width;
    i2s->ms_set0.bit.cfg_txd_chn = i2s_para->ch;
    i2s->ms_set0.bit.cfg_rxd_chn = i2s_para->ch;

    /* Reset XDMA */
    i2s->rdma_ctl1 = I2S_RDMA_RESET;
    i2s->wdma_ctl1 = I2S_WDMA_RESET;
    /* Clear XDMA IRQ */
    i2s->int_clear = I2S_RDMA_IRQ_CLR | I2S_RDMA_ERR_IRQ_CLR | I2S_WDMA_IRQ_CLR
                     | I2S_WDMA_ERR_IRQ_CLR;

    if (i2s_para->trx_mode == I2S_TRX_MODE_TXRX) {
        /* Enable XDMA irq */
        i2s->int_mask = ~(
            I2S_RDMA_IRQ_MASK_ENABLE | I2S_RDMA_ERR_IRQ_MASK_ENABLE
            | I2S_WDMA_IRQ_MASK_ENABLE | I2S_WDMA_ERR_IRQ_MASK_ENABLE);

        i2s_xdma_config = i2s_para->rdma_config;
        i2s->rdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size
                           << I2S_RDMA_SEG_SIZE_SHFT)
                          & I2S_RDMA_SEG_SIZE_MASK)
                         | (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size
                             << I2S_RDMA_BLK_SIZE_SHFT)
                            & I2S_RDMA_BLK_SIZE_MASK);
        i2s->rdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;

        i2s_xdma_config = i2s_para->wdma_config;
        i2s->wdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size
                           << I2S_RDMA_SEG_SIZE_SHFT)
                          & I2S_RDMA_SEG_SIZE_MASK)
                         | (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size
                             << I2S_RDMA_BLK_SIZE_SHFT)
                            & I2S_RDMA_BLK_SIZE_MASK);
        i2s->wdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_TX) {
        /* Enable XDMA irq */
        i2s->int_mask = ~(I2S_RDMA_IRQ_MASK_ENABLE
                          | I2S_RDMA_ERR_IRQ_MASK_ENABLE);

        i2s_xdma_config = i2s_para->rdma_config;
        i2s->rdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size
                           << I2S_RDMA_SEG_SIZE_SHFT)
                          & I2S_RDMA_SEG_SIZE_MASK)
                         | (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size
                             << I2S_RDMA_BLK_SIZE_SHFT)
                            & I2S_RDMA_BLK_SIZE_MASK);
        i2s->rdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_RX) {
        /* Enable XDMA irq */
        i2s->int_mask = ~(I2S_WDMA_IRQ_MASK_ENABLE
                          | I2S_WDMA_ERR_IRQ_MASK_ENABLE);

        i2s_xdma_config = i2s_para->wdma_config;
        i2s->wdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size
                           << I2S_RDMA_SEG_SIZE_SHFT)
                          & I2S_RDMA_SEG_SIZE_MASK)
                         | (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size
                             << I2S_RDMA_BLK_SIZE_SHFT)
                            & I2S_RDMA_BLK_SIZE_MASK);
        i2s->wdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;
    } else {
        return STATUS_INVALID_PARAM;
    }

    return STATUS_SUCCESS;
}

uint32_t i2s_uninit(void) {
    i2s_t* i2s = I2S0;

    i2s->ms_ctl1.bit.i2s_reset = 1;

    /* Reset XDMA */
    i2s->rdma_ctl1 = I2S_RDMA_RESET;
    i2s->wdma_ctl1 = I2S_WDMA_RESET;

    /* Clear XDMA IRQ */
    i2s->int_clear = I2S_RDMA_IRQ_CLR | I2S_RDMA_ERR_IRQ_CLR | I2S_WDMA_IRQ_CLR
                     | I2S_WDMA_ERR_IRQ_CLR;

    /* Disable XDMA irq */
    i2s->int_mask = (I2S_RDMA_IRQ_MASK_ENABLE | I2S_RDMA_ERR_IRQ_MASK_ENABLE
                     | I2S_WDMA_IRQ_MASK_ENABLE | I2S_WDMA_ERR_IRQ_MASK_ENABLE);
    NVIC_DisableIRQ(I2s0_IRQn);

    return STATUS_SUCCESS;
}

uint32_t i2s_loopback_open(void) {
    i2s_t *i2s = I2S0;

    i2s->ms_set0.bit.cfg_i2s_tst = 0x13;

    return STATUS_SUCCESS;
}

uint32_t i2s_start(i2s_para_set_t* i2s_para) {
    i2s_t* i2s = I2S0;
    if (i2s_para->trx_mode == I2S_TRX_MODE_TXRX) {
        i2s->wdma_ctl0 |= I2S_WDMA_ENABLE;
        i2s->rdma_ctl0 |= I2S_RDMA_ENABLE;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_TX) {
        i2s->rdma_ctl0 = I2S_RDMA_ENABLE;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_RX) {
        i2s->wdma_ctl0 = I2S_WDMA_ENABLE;
    } else {
        return STATUS_INVALID_PARAM;
    }

    /* Enable I2S */
    i2s->ms_ctl0.bit.mclk_enable = 1;
    i2s->ms_ctl0.bit.i2s_enable = 1;

    return STATUS_SUCCESS;
}

uint32_t i2s_stop(void) {
    i2s_t* i2s = I2S0;

    /* Disable I2S */
    i2s->ms_ctl0.bit.i2s_enable = 0;
    i2s->ms_ctl0.bit.mclk_enable = 0;

    return STATUS_SUCCESS;
}

/**
 * \brief           I2S Interrupt Handler
 */
void i2s_handler(void) {
    i2s_t* i2s = I2S0;
    i2s_cb_t cb;

    if (i2s->int_status & I2S_RDMA_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_RDMA_IRQ_CLR;

        if (((i2s->rdma_set0 & 0xFFFF0000) >> 16) == 0) {
            /* If one-shot mode happened, this condition  fuction is valid. */
            i2s->rdma_ctl0 = I2S_RDMA_ENABLE;
        }

        cb.type = I2S_CB_RDMA;
        i2s_get_rdma_blk_size(&cb.blk_size);
        i2s_get_rdma_seg_size(&cb.seg_size);

        if (i2s_reg_handler != NULL) {
            i2s_reg_handler(&cb);
        }

        i2s->int_clear |= I2S_RDMA_IRQ_CLR;

    } else if (i2s->int_status & I2S_RDMA_ERR_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_RDMA_ERR_IRQ_CLR;
    } else if (i2s->int_status & I2S_WDMA_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_WDMA_IRQ_CLR;

        if (((i2s->wdma_set0 & 0xFFFF0000) >> 16) == 0) {
            /* If one-shot mode happened, this condition  fuction is valid. */
            i2s->wdma_ctl0 = I2S_WDMA_ENABLE;
        }

        cb.type = I2S_CB_WDMA;
        i2s_get_wdma_blk_size(&cb.blk_size);
        i2s_get_wdma_seg_size(&cb.seg_size);

        if (i2s_reg_handler != NULL) {
            i2s_reg_handler(&cb);
        }
        i2s->int_clear |= I2S_WDMA_IRQ_CLR;
    } else if (i2s->int_status & I2S_WDMA_ERR_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_WDMA_ERR_IRQ_CLR;
    }

    i2s->int_clear = i2s->int_status;

    return;
}
