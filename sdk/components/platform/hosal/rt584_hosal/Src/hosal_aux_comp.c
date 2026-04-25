/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_aux_com.c
 * \brief           hosal aux comparator driver 
 */
/*
 * Author:          Kc.tseng
 */

#include "hosal_aux_comp.h"

uint32_t hosal_aux_comp_open(hosal_aux_comp_config_t cfg, void* hosal_aux_comp_callback) {

    uint32_t step_value, rval;

    aux_comp_config_t aux_cfg =
    {
        .debounce_en = cfg.debounce_en,
        .debounce_sel = cfg.debounce_sel,
        .counter_mode_en = cfg.counter_mode_en,
        .counter_mode_edge = cfg.counter_mode_edge,
        .counter_mode_int_en = cfg.counter_mode_int_en,
        .rising_edge_int_en = cfg.rising_edge_int_en,
        .falling_edge_int_en = cfg.falling_edge_int_en,
        .counter_mode_threshold = cfg.counter_mode_threshold,
    };
    aux_voltage_threshold_calibration(cfg.voltage_threshold,&step_value);
    aux_cfg.voltage_step = step_value;

    rval = aux_comp_open(aux_cfg, hosal_aux_comp_callback);

    return rval;
}

uint32_t hosal_aux_comp_normal_start(void) {
    uint32_t rval;

    rval = aux_comp_normal_start();

    return rval;
}

uint32_t hosal_aux_comp_normal_stop(void) {
    uint32_t rval;

    rval = aux_comp_normal_stop();

    return rval;
}

uint32_t hosal_aux_comp_sleep_start(void) {
    uint32_t rval;

    rval = aux_comp_sleep_start();

    return rval;
}

uint32_t hosal_aux_comp_sleep_stop(void) {
    uint32_t rval;

    rval = aux_comp_sleep_stop();

    return rval;
}

uint32_t hosal_aux_comp_deep_sleep_start(void) {
    uint32_t rval;

    rval = aux_comp_deep_sleep_start();

    return rval;
}

uint32_t hosal_aux_comp_deep_sleep_stop(void) {
    uint32_t rval;

    rval = aux_comp_deep_sleep_stop();

    return rval;
}

uint32_t hosal_aux_comp_setup_deep_sleep_enable_clock(void) {
    uint32_t rval;

    rval = aux_comp_setup_deep_sleep_enable_clock();

    return rval;
}

uint32_t hosal_aux_comp_setup_deep_sleep_disable_clock(uint8_t wakeup_level) {
    uint32_t rval;

    rval = aux_comp_setup_deep_sleep_disable_clock(wakeup_level);

    return rval;
}

uint32_t hosal_get_aux_comp_counter_count(uint32_t *count) {
    uint32_t rval;

    rval = get_aux_comp_counter_count(count);

    return rval;
}

uint32_t hosal_clear_aux_comp_counter_count(void) {
    uint32_t rval;

    rval = clear_aux_comp_counter_count();

    return rval;
}
