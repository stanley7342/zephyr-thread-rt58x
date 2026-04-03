/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_i2x.c
 * \brief           Hosal I2C driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "mcu.h"

#include "hosal_i2s.h"

static hosal_i2s_cb_fn hosal_i2s_reg_handler = NULL;

void hosal_i2s_cb_handler(i2s_cb_t* p_cb) {
    hosal_i2s_cb_t hosal_cb = {
        .type = p_cb->type,
        .blk_size = p_cb->blk_size,
        .seg_size = p_cb->seg_size,
    };
    if (hosal_i2s_reg_handler != NULL)
        hosal_i2s_reg_handler(&hosal_cb);
}

uint32_t hosal_i2s_callback_register(void* i2s_usr_callback) {
    uint32_t rval = STATUS_INVALID_PARAM;

    if (i2s_usr_callback != NULL) {
        hosal_i2s_reg_handler = i2s_usr_callback;
        rval = i2s_callback_register(hosal_i2s_cb_handler);
    }

    return rval;
}

uint32_t hosal_i2s_get_rdma_access_pos(uint32_t* rdma_address) {
    uint32_t rval;

    rval = i2s_get_rdma_access_pos(rdma_address);

    return rval;
};

uint32_t hosal_i2s_get_wdma_access_pos(uint32_t* wdma_address) {
    uint32_t rval;

    rval = i2s_get_wdma_access_pos(wdma_address);

    return rval;
};

uint32_t hosal_i2s_init(hosal_i2s_para_set_t* hosal_i2s_para) {
    i2s_para_set_t i2s_para = {
        .rdma_config = (i2s_rdma_ctrl_ptr_t*)hosal_i2s_para->rdma_config,
        .wdma_config = (i2s_wdma_ctrl_ptr_t*)hosal_i2s_para->wdma_config,
        .sr = (i2s_sample_rate_t)hosal_i2s_para->sr,
        .ch = (i2s_ch_format_t)hosal_i2s_para->ch,
        .trx_mode = (i2s_trx_mode_t)hosal_i2s_para->trx_mode,
        .fmt = (i2s_fmt_t)hosal_i2s_para->fmt,
        .width = (i2s_cfg_txd_wid_t)hosal_i2s_para->width,
        .bck_ratio = (i2s_cfg_bck_ratio_t)hosal_i2s_para->bck_ratio,
        .mck_div = (i2s_mclk_div_t)hosal_i2s_para->mck_div,
        .bck_osr = (i2s_cfg_bck_osr_t)hosal_i2s_para->bck_osr,
        .imck_rate = (i2s_cfg_imck_rate_t)hosal_i2s_para->imck_rate,
    };
    uint32_t rval;

    rval = i2s_init(&i2s_para);

    return rval;
}

uint32_t hosal_i2s_uninit(void) { 
    uint32_t rval;

    rval = i2s_uninit();

    return rval;
}

uint32_t hosal_i2s_loopback_open(void) {
    uint32_t rval;

    rval = i2s_loopback_open();

    return rval;
}

uint32_t hosal_i2s_start(hosal_i2s_para_set_t* hosal_i2s_para) {
    i2s_para_set_t i2s_para = {
        .rdma_config = (i2s_rdma_ctrl_ptr_t*)hosal_i2s_para->rdma_config,
        .wdma_config = (i2s_wdma_ctrl_ptr_t*)hosal_i2s_para->wdma_config,
        .sr = (i2s_sample_rate_t)hosal_i2s_para->sr,
        .ch = (i2s_ch_format_t)hosal_i2s_para->ch,
        .trx_mode = (i2s_trx_mode_t)hosal_i2s_para->trx_mode,
        .fmt = (i2s_fmt_t)hosal_i2s_para->fmt,
        .width = (i2s_cfg_txd_wid_t)hosal_i2s_para->width,
        .bck_ratio = (i2s_cfg_bck_ratio_t)hosal_i2s_para->bck_ratio,
        .mck_div = (i2s_mclk_div_t)hosal_i2s_para->mck_div,
        .bck_osr = (i2s_cfg_bck_osr_t)hosal_i2s_para->bck_osr,
        .imck_rate = (i2s_cfg_imck_rate_t)hosal_i2s_para->imck_rate,
    };
    uint32_t rval;

    rval = i2s_start(&i2s_para);

    return rval;
}

uint32_t hosal_i2s_stop(void) { 
    uint32_t rval;

    rval = i2s_stop();

    return rval;
}
