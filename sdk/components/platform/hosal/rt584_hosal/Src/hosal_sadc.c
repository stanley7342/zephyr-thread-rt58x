/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_sadc.c
 * \brief           Hosal SADC driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "stdio.h"
#include <stdint.h>
#include "mcu.h"
#include "hosal_sadc.h"

static hosal_sadc_cb_fn hosal_sadc_reg_handler = NULL;

void hosal_sadc_register_int_callback(hosal_sadc_cb_fn hosal_sadc_int_callback) {
    hosal_sadc_reg_handler = hosal_sadc_int_callback;
    return;
}

void hosal_sadc_cb_handler(sadc_cb_t* p_cb) {
    hosal_sadc_cb_t hosal_cb = {
        .type = p_cb->type,
        .raw.calibration_value = p_cb->raw.calibration_value,
        .raw.compensation_value = p_cb->raw.compensation_value,
        .raw.conversion_value = p_cb->raw.conversion_value,
        .data.done.p_buffer = p_cb->data.done.p_buffer,
        .data.done.size = p_cb->data.done.size,
        .data.sample.channel = p_cb->data.sample.channel,
        .data.sample.value = p_cb->data.sample.value,
    };

    if (hosal_sadc_reg_handler!= NULL) {
        hosal_sadc_reg_handler(&hosal_cb);
    }
}

void hosal_sadc_aio_disable(uint8_t aio_num) {
    sadc_aio_disable(aio_num);
}

void hosal_sadc_aio_enable(uint8_t aio_num) {
    sadc_aio_enable(aio_num);
}

void hosal_sadc_config_enable(hosal_sadc_config_t sadc_cfg, void* sadc_usr_callback) {
    uint32_t resolution, oversample;

    resolution = sadc_cfg.resolution;
    oversample = sadc_cfg.oversample;

    if (sadc_usr_callback != NULL) {
        hosal_sadc_register_int_callback(sadc_usr_callback);
    }
    sadc_config_enable(resolution, oversample, hosal_sadc_cb_handler);
}

uint32_t hosal_sadc_channel_read(hosal_sadc_channel_config_t ch) {
    sadc_input_ch_t read_ch;
    uint32_t rval;

    read_ch = (sadc_input_ch_t) ch.channel;

    if ( read_ch == HOSAL_SADC_CH_TEMPERATURE ) {
        rval = sadc_temp_read();
    } else if ( read_ch == HOSAL_SADC_CH_VBAT ) {
        rval = sadc_vbat_read();
    } else {
        rval = sadc_channel_read(read_ch);
    }
    return rval;
}

void hosal_sadc_compensation_init(uint32_t xPeriodicTimeInSec) {

}


