/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_irm.c
 * \brief           Hosal irm driver 
 */
/*
 * This file is part of library_name.
 * Author: 
 */
#include "hosal_irm.h"
#include "hosal_status.h"

ir_state_t ir_state;
ir_buffer_t buffer;
ir_fifo_t fifo[IR_BUFFER_MAX_LENGTH];

static irm_proc_cb user_cb = NULL;

uint32_t hosal_ir_carrier_config(uint8_t carrier_high_cnt, uint8_t carrier_low_cnt,
                       uint16_t carrier_base_cnt) {
    uint32_t rval;

    rval = ir_carrier_config(carrier_high_cnt, carrier_low_cnt, carrier_base_cnt);

    return rval;
}

uint32_t hosal_irm_open(hosal_irm_mode_t* hosal_irm_cfg) {
    irm_mode_t irm_cfg = {
        .irm_cb_func = hosal_irm_cfg->irm_cb_func,
        .op_mode = hosal_irm_cfg->op_mode,
        .ir_out_mode = hosal_irm_cfg->ir_out_mode,
        .irm_int_en = hosal_irm_cfg->irm_int_en,
    };
    uint32_t rval;

    rval = irm_open(&irm_cfg);

    return rval;
} 

uint32_t hosal_ir_nec_encoder(uint16_t cmd, uint16_t address) {
    uint32_t rval;

    rval = ir_nec_encoder(cmd, address);

    return rval;
}

uint32_t hosal_ir_nec_repeat_encoder(void) {
    uint32_t rval;

    rval = ir_nec_repeat_encoder();

    return rval;
}

uint32_t hosal_ir_sirc_encoder(uint8_t cmd, uint8_t address, uint8_t address_length) {
    uint32_t rval;

    rval = ir_sirc_encoder(cmd, address, address_length);

    return rval;
}

uint32_t hosal_ir_rc6_encoder(uint16_t cmd, uint8_t address, uint8_t rc6_toggle) {
    uint32_t rval;

    rval = ir_rc6_encoder(cmd, address, rc6_toggle);

    return rval;
}

uint32_t hosal_ir_buffer_fill_in(void) {
    uint32_t rval;

    rval = ir_buffer_fill_in();

    return rval;
}

uint32_t hosal_ir_buffer_repeat_fill_in(void) {
    uint32_t rval;

    rval = ir_buffer_repeat_fill_in();

    return rval;
}

uint32_t hosal_ir_enable(void) { 
    uint32_t rval;

    rval = ir_enable();

    return rval;
}

uint32_t hosal_ir_disable(void) { 
    uint32_t rval;

    rval = ir_disable();

    return rval;
}

uint32_t hosal_ir_start(void) {
    uint32_t rval;

    rval = ir_start();

    return rval;
}

uint32_t hosal_ir_stop(void) {
    uint32_t rval;

    rval = ir_stop();

    return rval;
}

uint32_t hosal_ir_fifo_level(uint32_t *fifo_lvl) { 
    uint32_t rval;

    rval = ir_fifo_level(fifo_lvl); 

    return rval;
}

uint32_t hosal_ir_fifo_empty(uint32_t *empty) { 
    uint32_t rval;

    rval = ir_fifo_empty(empty);

    return rval;
}

uint32_t hosal_ir_fifo_full(uint32_t *full) { 
    uint32_t rval;

    rval = ir_fifo_full(full);

    return rval;
}
