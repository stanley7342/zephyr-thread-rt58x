/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_timer.h
 * \brief           Hosal Timer driver header file
 */
/*
 * Author:          Kc.tseng
 */

#include <stdint.h>
#include "hosal_timer.h"
#include "lpm.h"
#include "mcu.h"

uint32_t hosal_timer_init(uint32_t timer_id, hosal_timer_config_t cfg,
                          void* usr_call_back) {
    timer_config_mode_t drv_cfg = {
        .int_en = cfg.int_en,
        .mode = cfg.mode,
        .prescale = cfg.prescale,
        .oneshot_mode = cfg.oneshot_mode,
        .counting_mode = cfg.counting_mode,
        .user_prescale = cfg.user_prescale,
    };
    uint32_t rval;

    rval = timer_open(timer_id, drv_cfg, usr_call_back);

    return rval;
}

uint32_t hosal_timer_start(uint32_t timer_id,
                           hosal_timer_tick_config_t tick_cfg) {
    uint32_t rval;

    rval = timer_start(timer_id, tick_cfg.timeload_ticks, tick_cfg.timeout_ticks);

    return rval;
}

uint32_t hosal_timer_stop(uint32_t timer_id) {
    uint32_t rval;

    rval = timer_stop(timer_id);

    return rval;
}

uint32_t hosal_timer_reload(uint32_t timer_id,
                            hosal_timer_tick_config_t tick_cfg) {
    uint32_t rval;

    rval = timer_load(timer_id, tick_cfg.timeload_ticks, tick_cfg.timeout_ticks);

    return rval;
}

uint32_t hosal_timer_finalize(uint32_t timer_id) {
    uint32_t rval;

    rval = timer_close(timer_id);

    return rval;
}

uint32_t hosal_timer_current_get(uint32_t timer_id, uint32_t* tick_value) {
    uint32_t value;

    value = timer_current_get(timer_id, tick_value);

    return value;
}

uint32_t hosal_timer_capture_init(uint32_t timer_id,
                                  hosal_timer_capture_config_mode_t cfg,
                                  void* usr_call_back) {
    timer_capture_config_mode_t drv_cfg = {
        .int_en = cfg.int_en,
        .mode = cfg.mode,
        .prescale = cfg.prescale,
        .oneshot_mode = cfg.oneshot_mode,
        .counting_mode = cfg.counting_mode,
        .user_prescale = cfg.user_prescale,
        .ch0_capture_edge = cfg.ch0_capture_edge,
        .ch0_deglich_enable = cfg.ch0_deglich_enable,
        .ch0_int_enable = cfg.ch0_int_enable,
        .ch0_iosel = cfg.ch0_iosel,
        .ch1_capture_edge = cfg.ch1_capture_edge,
        .ch1_deglich_enable = cfg.ch1_deglich_enable,
        .ch1_int_enable = cfg.ch1_int_enable,
        .ch1_iosel = cfg.ch1_iosel,
    };
    uint32_t rval;

    rval = timer_capture_open(timer_id, drv_cfg, usr_call_back);

    return rval;
}

uint32_t hosal_timer_capture_start(uint32_t timer_id, uint32_t timeload_ticks,
                                   uint32_t timeout_ticks, bool chanel0_enable,
                                   bool chanel1_enable) {
    uint32_t rval;

    rval = timer_capture_start(timer_id, timeload_ticks, timeout_ticks,
                               chanel0_enable, chanel1_enable);

    return rval;
}

uint32_t hosal_timer_capture_stop(uint32_t timer_id) {
    uint32_t rval;

    rval = timer_capture_stop(timer_id);

    return rval;
}

uint32_t hosal_timer_capture_finalize(uint32_t timer_id) {
    uint32_t rval;

    rval = timer_capture_close(timer_id);

    return rval;
}

uint32_t hosal_timer_ch0_capture_value_get(uint32_t timer_id, uint32_t* tick_value) {
    uint32_t rval;

    rval = timer_ch0_capture_value_get(timer_id, tick_value);

    return rval;
}

uint32_t hosal_timer_ch0_capture_int_status(uint32_t timer_id, uint32_t* int_status) {
    uint32_t rval;

    rval = timer_ch0_capture_int_status(timer_id, int_status);

    return rval;
}

uint32_t hosal_timer_ch1_capture_value_get(uint32_t timer_id, uint32_t* tick_value) {
    uint32_t rval;

    rval = timer_ch1_capture_value_get(timer_id, tick_value);

    return rval;
}

uint32_t hosal_timer_ch1_capture_int_status(uint32_t timer_id, uint32_t* int_status) {
    uint32_t rval;

    rval = timer_ch1_capture_int_status(timer_id, int_status);

    return rval;
}

uint32_t hosal_timer_pwm_open(uint32_t timer_id, hosal_timer_pwm_config_mode_t cfg) {
    timer_pwm_config_mode_t drv_cfg = {
        .counting_mode  = cfg.counting_mode,
        .oneshot_mode   = cfg.oneshot_mode,
        .mode           = cfg.mode,
        .int_en         = cfg.int_en,
        .prescale       = cfg.prescale,
        .user_prescale  = cfg.user_prescale,
        .pwm0_enable    = cfg.pwm0_enable,
        .pwm1_enable    = cfg.pwm1_enable,
        .pwm2_enable    = cfg.pwm2_enable,
        .pwm3_enable    = cfg.pwm3_enable,
        .pwm4_enable    = cfg.pwm4_enable,
    };
    uint32_t rval;


    rval = timer_pwm_open(timer_id, drv_cfg);

    return rval;
}

uint32_t hosal_timer_pwm_start(uint32_t timer_id, hosal_timer_pwm_config_tick_t cfg) {
    uint32_t rval;

    rval = timer_pwm_start(timer_id, cfg.timeload_ticks, cfg.timeout_ticks, 
                           cfg.threshold, cfg.phase);
    lpm_enable_timer_pwm();
    return rval;
}


uint32_t hosal_timer_pwm_stop(uint32_t timer_id) {
    uint32_t rval;

    rval = timer_pwm_stop(timer_id);
    
    return rval;
}

uint32_t hosal_timer_pwm_close(uint32_t timer_id) {
    uint32_t rval;

    rval = timer_pwm_close(timer_id);

    return rval;
}
