/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            dwt.c
 * \brief           dwt driver file
 */

/*
 * This file is part of library_name.
 * Author:     
 */
#include <stdio.h>
#include <stdint.h>
#include "mcu.h"
#include "dwt.h"
#include "status.h"


/* Reference counter to manage shared hardware access */
static uint8_t g_timeout_init_count = 0;

/*  */
static uint32_t g_last_val = 0;
static uint32_t g_accumulated_ms = 0;
static uint32_t g_fractional_ticks = 0;


static void _update_bare_metal_tick(void) {
    uint32_t current_val = SysTick->VAL;
    uint32_t load = SysTick->LOAD;
    uint32_t ticks_per_ms = SystemCoreClock / 1000;
    uint32_t elapsed_ticks = 0;

    if (ticks_per_ms == 0) return;

    // 
    if (current_val <= g_last_val) {
        elapsed_ticks = g_last_val - current_val;
    } else {
        elapsed_ticks = g_last_val + (load - current_val) + 1;
    }
    g_last_val = current_val;

    // 
    g_fractional_ticks += elapsed_ticks;
    if (g_fractional_ticks >= ticks_per_ms) {
        g_accumulated_ms += (g_fractional_ticks / ticks_per_ms);
        g_fractional_ticks %= ticks_per_ms;
    }
}

// get tick
static uint32_t _get_elapsed_ms() {
#if defined(CONFIG_FREERTOS)
    if (_IS_RTOS_ACTIVE()) {
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    }
#endif
    _update_bare_metal_tick();
    return g_accumulated_ms;
}


uint32_t timeout_init(void) {
    if (g_timeout_init_count == 0) {

        if (!_IS_RTOS_ACTIVE()) {
           
            if (!(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)) {
                
                SysTick->LOAD = 0xFFFFFF; 
                SysTick->VAL  = 0;
                SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;
            }
            
            g_last_val = SysTick->VAL;
            g_accumulated_ms = 0;
            g_fractional_ticks = 0;
        }
    }
    
    g_timeout_init_count++;
    return STATUS_SUCCESS;
}


uint32_t timeout_uninit(void) {
    if (g_timeout_init_count > 0) {
        g_timeout_init_count--;
    }

    /* Shut down hardware only if count is zero and RTOS is not managing it */
    if (g_timeout_init_count == 0 && !_IS_RTOS_ACTIVE()) {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    }
    return STATUS_SUCCESS;
}


uint32_t timeout_start(TimeoutTimer *timer, uint32_t ms, TimeoutCallback_t cb, void *arg, bool periodic) {
    if (timer == NULL) return STATUS_INVALID_PARAM;

    timer->timeout_ms = ms;
    timer->callback = cb;
    timer->callback_arg = arg;
    timer->callback_executed = false;
    timer->auto_reload = periodic;
    timer->mode_at_start = _IS_RTOS_ACTIVE();
    
    timer->start_time = _get_elapsed_ms();
    return STATUS_SUCCESS;
}

uint32_t istimeout(TimeoutTimer *timer) {

    if (timer == NULL) return STATUS_INVALID_PARAM;
    
    if (timer->timeout_ms == 0) {
        return STATUS_SUCCESS; 
    }

    bool current_mode = _IS_RTOS_ACTIVE();
    
    if (current_mode != timer->mode_at_start) {
        timeout_start(timer, timer->timeout_ms, timer->callback, timer->callback_arg, timer->auto_reload);
        return STATUS_SUCCESS;
    }

    uint32_t now = _get_elapsed_ms();
    uint32_t elapsed_ms = now - timer->start_time;

    if (elapsed_ms >= timer->timeout_ms) {
        if (timer->callback != NULL && (timer->auto_reload || !timer->callback_executed)) {
            timer->callback(timer->callback_arg);
            if (timer->auto_reload) {
                
                timer->start_time += timer->timeout_ms;
            } else {
                timer->callback_executed = true;
            }
        }
        return STATUS_TIMEOUT;
    }
    return STATUS_SUCCESS;
}

uint32_t timeout_remaining(TimeoutTimer *timer, uint32_t *remaining_ms) {
    if (timer == NULL || remaining_ms == NULL) return STATUS_INVALID_PARAM;
    if (_IS_RTOS_ACTIVE() != timer->mode_at_start) {
        *remaining_ms = 0;
        return STATUS_TIMEOUT;
    }

    uint32_t now = _get_elapsed_ms();
    uint32_t elapsed_ms = now - timer->start_time;

    if (elapsed_ms >= timer->timeout_ms) {
        *remaining_ms = 0;
        return STATUS_TIMEOUT;
    } else {
        *remaining_ms = timer->timeout_ms - elapsed_ms;
        return STATUS_SUCCESS;
    }
}

uint32_t timeout_stop(TimeoutTimer *timer) {
    if (timer == NULL) return STATUS_INVALID_PARAM;

    timer->timeout_ms = 0;          // 
    timer->callback = NULL;         // 
    timer->callback_executed = true; // 
    timer->auto_reload = false;     // 

    return STATUS_SUCCESS;
}