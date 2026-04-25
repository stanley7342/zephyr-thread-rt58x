/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            i2s.c
 * \brief           i2s driver 
 */
/*
 * This file is part of library_name.
 * Author: 
 */
#include "i2s.h"

static i2s_cb_fn i2s_reg_handler = NULL;

uint32_t i2s_callback_register(i2s_cb_fn i2s_int_callback) {
    i2s_reg_handler = i2s_int_callback;

    return STATUS_SUCCESS;
}

uint32_t i2s_init(i2s_para_set_t *i2s_para) {
    i2s_t *i2s = I2S_MASTER;
    i2s_xdma_ctrl_ptr_t *i2s_xdma_config;
    uint8_t blk_osr = 0;

    if ((i2s_para->fmt >= I2S_FMT_MAX) |
            (i2s_para->sr >= I2S_SR_MAX)) {
        //Fail Cases
        return STATUS_INVALID_PARAM;
    }

    // Make I2S reset
    i2s->ms_ctl1.bit.cfg_i2s_rst = 1;


    i2s->mclk_set0.bit.cfg_mck_isel = i2s_para->imck_rate;

    i2s->mclk_set1.bit.cfg_mck_div = i2s_para->mck_div;

    i2s->ms_set0.reg = 0;
    i2s->ms_set0.bit.cfg_bck_osr = i2s_para->bck_osr;
    i2s->ms_set0.bit.cfg_i2s_mod = i2s_para->trx_mode;
    i2s->ms_set0.bit.cfg_i2s_fmt = i2s_para->fmt;
    i2s->ms_set0.bit.cfg_bck_len = i2s_para->bck_ratio;
    i2s->ms_set0.bit.cfg_txd_wid = i2s_para->width;
    i2s->ms_set0.bit.cfg_rxd_wid = i2s_para->width;
    i2s->ms_set0.bit.cfg_txd_chn = i2s_para->ch;
    i2s->ms_set0.bit.cfg_rxd_chn = i2s_para->ch;

    // Reset XDMA
    i2s->rdma_ctl1.reg = I2S_RDMA_RESET;
    i2s->wdma_ctl1.reg = I2S_WDMA_RESET;
    // Clear XDMA IRQ
    i2s->int_clear = I2S_RDMA_IRQ_CLR | I2S_RDMA_ERR_IRQ_CLR | I2S_WDMA_IRQ_CLR | I2S_WDMA_ERR_IRQ_CLR;

    if (i2s_para->trx_mode == I2S_TRX_MODE_TXRX) {
        // Enable XDMA irq
        i2s->int_mask = ~(I2S_RDMA_IRQ_MASK_ENABLE | I2S_RDMA_ERR_IRQ_MASK_ENABLE | I2S_WDMA_IRQ_MASK_ENABLE | I2S_WDMA_ERR_IRQ_MASK_ENABLE);

        i2s_xdma_config = i2s_para->rdma_config;
        i2s->rdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size << I2S_RDMA_SEG_SIZE_SHFT) & I2S_RDMA_SEG_SIZE_MASK) |
                             (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size << I2S_RDMA_BLK_SIZE_SHFT) & I2S_RDMA_BLK_SIZE_MASK);
        i2s->rdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;

        i2s_xdma_config = i2s_para->wdma_config;
        i2s->wdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size << I2S_RDMA_SEG_SIZE_SHFT) & I2S_RDMA_SEG_SIZE_MASK) |
                             (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size << I2S_RDMA_BLK_SIZE_SHFT) & I2S_RDMA_BLK_SIZE_MASK);
        i2s->wdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_TX) {
        // Enable XDMA irq
        i2s->int_mask = ~(I2S_RDMA_IRQ_MASK_ENABLE | I2S_RDMA_ERR_IRQ_MASK_ENABLE);

        i2s_xdma_config = i2s_para->rdma_config;
        i2s->rdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size << I2S_RDMA_SEG_SIZE_SHFT) & I2S_RDMA_SEG_SIZE_MASK) |
                             (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size << I2S_RDMA_BLK_SIZE_SHFT) & I2S_RDMA_BLK_SIZE_MASK);
        i2s->rdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_RX) {
        // Enable XDMA irq
        i2s->int_mask = ~(I2S_WDMA_IRQ_MASK_ENABLE | I2S_WDMA_ERR_IRQ_MASK_ENABLE);

        i2s_xdma_config = i2s_para->wdma_config;
        i2s->wdma_set0 = (((uint32_t)i2s_xdma_config->i2s_xdma_seg_size << I2S_RDMA_SEG_SIZE_SHFT) & I2S_RDMA_SEG_SIZE_MASK) |
                             (((uint32_t)i2s_xdma_config->i2s_xdma_blk_size << I2S_RDMA_BLK_SIZE_SHFT) & I2S_RDMA_BLK_SIZE_MASK);
        i2s->wdma_set1 = i2s_xdma_config->i2s_xdma_start_addr;
    } else {
        return STATUS_INVALID_PARAM;
    }

    return STATUS_SUCCESS;
}

uint32_t i2s_uninit(void) {
    i2s_t *i2s = I2S_MASTER;

    // Make I2S reset
    i2s->ms_ctl1.bit.cfg_i2s_rst = 1;

    // Reset XDMA
    i2s->rdma_ctl1.reg = I2S_RDMA_RESET;
    i2s->wdma_ctl1.reg = I2S_WDMA_RESET;

    // Clear XDMA IRQ
    i2s->int_clear = I2S_RDMA_IRQ_CLR | I2S_RDMA_ERR_IRQ_CLR | I2S_WDMA_IRQ_CLR | I2S_WDMA_ERR_IRQ_CLR;

    // Disable XDMA irq
    i2s->int_mask = (I2S_RDMA_IRQ_MASK_ENABLE | I2S_RDMA_ERR_IRQ_MASK_ENABLE | I2S_WDMA_IRQ_MASK_ENABLE | I2S_WDMA_ERR_IRQ_MASK_ENABLE);
    NVIC_DisableIRQ(I2s0_IRQn);

    return STATUS_SUCCESS;
}

uint32_t i2s_loopback_open(void) {
    i2s_t *i2s = I2S_MASTER;

    i2s->ms_set0.bit.cfg_i2s_tst = 0x13;

    return STATUS_SUCCESS;
}

uint32_t i2s_start(i2s_para_set_t *i2s_para) {
    i2s_t *i2s = I2S_MASTER;
    if (i2s_para->trx_mode == I2S_TRX_MODE_TXRX) {
        i2s->rdma_ctl0.reg |= I2S_RDMA_ENABLE;
        i2s->wdma_ctl0.reg |= I2S_WDMA_ENABLE;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_TX) {
        i2s->rdma_ctl0.reg = I2S_RDMA_ENABLE;
    } else if (i2s_para->trx_mode == I2S_TRX_MODE_RX) {
        i2s->wdma_ctl0.reg = I2S_WDMA_ENABLE;
    } else {
        return STATUS_INVALID_PARAM;
    }
    // Enable I2S
    i2s->ms_ctl0.bit.cfg_mck_ena = 1;
    i2s->ms_ctl0.bit.cfg_i2s_ena = 1;

    return STATUS_SUCCESS;
}

uint32_t i2s_stop(void) {
    i2s_t *i2s = I2S_MASTER;

    // Disable I2S
    i2s->ms_ctl0.bit.cfg_i2s_ena = 0;
    i2s->ms_ctl0.bit.cfg_mck_ena = 0;

    return STATUS_SUCCESS;
}


void i2s0_handler(void) {
    i2s_t *i2s = I2S_MASTER;
    i2s_cb_t cb;

    if (i2s->int_status & I2S_RDMA_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_RDMA_IRQ_CLR;

        if (((i2s->rdma_set0 & 0xFFFF0000) >> 16) == 0) {
            //If one-shot mode happened, this condition  fuction is valid.
            i2s->rdma_ctl0.reg = I2S_RDMA_ENABLE;
        }
        cb.type = I2S_CB_RDMA;
        i2s_get_rdma_blk_size(&cb.blk_size);
        i2s_get_rdma_seg_size(&cb.seg_size);
        if( i2s_reg_handler != NULL ) {
            i2s_reg_handler(&cb);
        }
    } else if (i2s->int_status & I2S_RDMA_ERR_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_RDMA_ERR_IRQ_CLR;
    } else if (i2s->int_status & I2S_WDMA_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_WDMA_IRQ_CLR;

        if (((i2s->wdma_set0 & 0xFFFF0000) >> 16) == 0) {
            //If one-shot mode happened, this condition  fuction is valid.
            i2s->wdma_ctl0.reg = I2S_WDMA_ENABLE;
        }
        cb.type = I2S_CB_WDMA;
        i2s_get_wdma_blk_size(&cb.blk_size);
        i2s_get_wdma_seg_size(&cb.seg_size);

        if( i2s_reg_handler != NULL ) {
            i2s_reg_handler(&cb);
        }
    } else if (i2s->int_status & I2S_WDMA_ERR_IRQ_MASK_MASK) {
        i2s->int_clear |= I2S_WDMA_ERR_IRQ_CLR;
    }

    return;
}

