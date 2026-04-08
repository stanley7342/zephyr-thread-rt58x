/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_wdt.c
 * \brief           Hosal WDT driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "hosal_wdt.h"
#include "mcu.h"

uint32_t hosal_wdt_start(hosal_wdt_config_mode_t hal_wdt_cfg,
                         hosal_wdt_config_tick_t hal_wdt_tick_cfg,
                         void* usr_call_back) {
    /* mode setting */
    wdt_config_mode_t wdt_cfg = {
        .int_enable = hal_wdt_cfg.int_enable,
        .reset_enable = hal_wdt_cfg.reset_enable,
        .lock_enable = hal_wdt_cfg.lock_enable,
        .prescale = hal_wdt_cfg.prescale,
    };

    /* tick setting */
    wdt_config_tick_t wdt_tick_cfg = {
        .wdt_ticks = hal_wdt_tick_cfg.wdt_ticks,
        .int_ticks = hal_wdt_tick_cfg.int_ticks,
        .wdt_min_ticks = hal_wdt_tick_cfg.wdt_min_ticks,
    };
    return wdt_start(wdt_cfg, wdt_tick_cfg, usr_call_back);
}

uint32_t hosal_wdt_stop(void) {
    uint32_t rval;

    rval =  wdt_stop();

    return rval;
}

uint32_t hosal_wdt_reset_event_get(uint32_t* reset_cnt) {
    uint32_t rval;

    rval =  wdt_reset_event_get(reset_cnt);

    return rval;
}

uint32_t hosal_wdt_reset_event_clear(void) {
    uint32_t rval;

    rval = wdt_reset_event_clear();
    
    return rval;
}

uint32_t hosal_wdt_kick(void) {
    uint32_t rval;

    rval = wdt_kick();

    return rval;
}

uint32_t hosal_wdt_interrupt_clear(void) {
    uint32_t rval;

    rval = wdt_interrupt_clear();

    return rval;
}

uint32_t hosal_wdt_current_get(uint32_t* tick_value) {
    uint32_t rval;

    rval = wdt_current_get(tick_value);

    return rval;
}