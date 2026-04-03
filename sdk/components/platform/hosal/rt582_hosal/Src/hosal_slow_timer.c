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

#include <stdint.h>
#include "hosal_slow_timer.h"
#include "mcu.h"
#include "stdio.h"

uint32_t hosal_slow_timer_init(uint32_t timer_id, hosal_slow_timer_config_t cfg,
                          void* usr_call_back) {
    timer_config_mode_t drv_cfg = {.int_en = cfg.int_enable,
                                   .mode = cfg.mode,
                                   .prescale = cfg.prescale,
                                   .repeat_delay = cfg.repeat_delay};
    uint32_t rval;

    rval = timer_open(timer_id, drv_cfg, usr_call_back);

    return rval;
}

uint32_t hosal_slow_timer_start(uint32_t timer_id, hosal_slow_timer_tick_config_t tick_cfg) {
    uint32_t rval;

    rval = timer_start(timer_id, tick_cfg.timeload_ticks);

    return rval;
}

uint32_t hosal_slow_timer_stop(uint32_t timer_id) { 
    uint32_t rval;

    rval = timer_stop(timer_id); 

    return rval;
}

uint32_t hosal_slow_timer_reload(uint32_t timer_id, hosal_slow_timer_tick_config_t tick_cfg) {
    uint32_t rval;

    rval = timer_load(timer_id, tick_cfg.timeload_ticks);

    return rval;
}

uint32_t hosal_slow_timer_clear_int(uint32_t timer_id) {
    return timer_clear_int(timer_id);
}

uint32_t hosal_slow_timer_finalize(uint32_t timer_id) { 
    uint32_t rval;

    rval = timer_close(timer_id); 

    return rval;
}

uint32_t hosal_slow_timer_current_get(uint32_t timer_id, uint32_t* tick_value) {
    uint32_t rval;

    rval = timer_current_get(timer_id, tick_value);

    return rval;
}