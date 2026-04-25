/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_slow_timer.h
 * \brief           Hosal Slow Timer driver header file
 */
/*
 * Author:          Kc.tseng
 */

#include <stdio.h>

#include "hosal_slow_timer.h"
#include "mcu.h"

uint32_t hosal_slow_timer_init(uint32_t timer_id, hosal_slow_timer_config_t cfg, 
                          void* usr_call_back) {
    slowtimer_config_mode_t drv_cfg = {
        .counting_mode = cfg.counting_mode,
        .oneshot_mode = cfg.one_shot_mode,
        .mode = cfg.mode,
        .int_en = cfg.int_enable,
        .prescale = cfg.prescale,
        .user_prescale = cfg.user_prescale,
        .repeat_delay = cfg.repeat_delay
    };

    return slowtimer_open(timer_id, drv_cfg, usr_call_back);
}

uint32_t hosal_slow_timer_start(uint32_t timer_id, 
                           hosal_slow_timer_tick_config_t tick_cfg) {
    uint32_t rval;

    rval = slowtimer_start(timer_id, tick_cfg.timeload_ticks,
                           tick_cfg.timeout_ticks);
    return rval;
}

uint32_t hosal_slow_timer_stop(uint32_t timer_id) {
    uint32_t rval;

    rval = slowtimer_stop(timer_id);

    return rval;
}

uint32_t hosal_slow_timer_reload(uint32_t timer_id, 
                            hosal_slow_timer_tick_config_t tick_cfg) {
    uint32_t rval;

    rval = slowtimer_load(timer_id, tick_cfg.timeload_ticks,
                          tick_cfg.timeout_ticks);

    return rval;
}

uint32_t hosal_slow_timer_clear_int(uint32_t timer_id) {
    return slowtimer_clear_int(timer_id);
}

uint32_t hosal_slow_timer_finalize(uint32_t timer_id) {
    uint32_t rval;

    rval = slowtimer_close(timer_id);

    return rval;
}

uint32_t hosal_slow_timer_current_get(uint32_t timer_id, uint32_t* tick_value) {
    uint32_t value;

    value = slowtimer_current_get(timer_id, tick_value);

    return value;
}

