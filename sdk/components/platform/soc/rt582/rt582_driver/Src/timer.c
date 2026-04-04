/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            timer.c
 * \brief           Timer driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "mcu.h"

/* timer0/1/2, slow timer0/1, total 5 timer */
#define MAX_NUMBER_OF_TIMER 5

#define TIMER_STATE_OPEN   1
#define TIMER_STATE_CLOSED 0

/**
 * \brief           The structure save timer info
 */
typedef struct {
    timer_cb_fn timer_cb;                       /*!< user callback function. */
    timer_config_mode_t cfg;                    /*!< Point X coordinate */
    uint8_t state : 2;                          /*!< device state */
} timer_config_t;

static timer_config_t timer_cfg[MAX_NUMBER_OF_TIMER];

uint32_t timer_callback_register(uint32_t timer_id, timer_cb_fn timer_cb) {
    timer_cfg[timer_id].timer_cb = timer_cb;
    return STATUS_SUCCESS;
}

uint32_t timer_open(uint32_t timer_id, timer_config_mode_t cfg,
                    timer_cb_fn timer_cb) {
    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_CLOSED) {
        return STATUS_INVALID_REQUEST; /*device already opened*/
    }

    timer_callback_register(timer_id, timer_cb);
    timer_cfg[timer_id].cfg = cfg;

    timer_cfg[timer_id].state = TIMER_STATE_OPEN;

    return STATUS_SUCCESS;
}

uint32_t timer_load(uint32_t timer_id, uint32_t timeout_ticks) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2, TIMER3,
                                           TIMER4};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];

    timer->load = (timeout_ticks - 1);

    return STATUS_SUCCESS;
}

uint32_t timer_start(uint32_t timer_id, uint32_t timeout_ticks) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2, TIMER3,
                                           TIMER4};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        return STATUS_NO_INIT; /* DEVIC SHOULD BE OPEN FIRST */
    }

    timer = base[timer_id];
    timer_load(timer_id, timeout_ticks);
    timer->control.bit.int_enable = timer_cfg[timer_id].cfg.int_en;

    if (timer_cfg[timer_id].cfg.mode == TIMER_FREERUN_MODE) {
        timer->control.bit.mode = 0;
    } else {
        timer->control.bit.mode = 1;
    }

    if (timer_id == 3 || timer_id == 4) {
        timer->repeat_delay.bit.int_repeat_delay =
            timer_cfg[timer_id].cfg.repeat_delay;
    }

    timer->control.bit.prescale = timer_cfg[timer_id].cfg.prescale;
    delay_ms(1); /* For register synchronization in 32KHz clock domain */
    timer->control.bit.en = 1;

    return STATUS_SUCCESS;
}

uint32_t timer_stop(uint32_t timer_id) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2, TIMER3,
                                           TIMER4};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        return STATUS_NO_INIT; /* DEVIC SHOULD BE OPEN FIRST */
    }

    timer = base[timer_id];
    timer->control.reg = 0; /* Disable timer */

    return STATUS_SUCCESS;
}

uint32_t timer_clear_int(uint32_t timer_id) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2, TIMER3,
                                           TIMER4};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->clear = 1;

    return STATUS_SUCCESS;
}


uint32_t timer_close(uint32_t timer_id) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2, TIMER3,
                                           TIMER4};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->control.reg = 0; /* Disable timer */

    timer_cfg[timer_id].timer_cb = NULL;
    timer_cfg[timer_id].state = TIMER_STATE_CLOSED;

    return STATUS_SUCCESS;
}

uint32_t timer_current_get(uint32_t timer_id, uint32_t* tick_value) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2, TIMER3,
                                           TIMER4};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }
    timer = base[timer_id];
    *tick_value = timer->value;

    return STATUS_SUCCESS;
}

/**
 * \brief           Timer0 Interrupt Handler
 */
void timer0_handler(void) {
    TIMER0->clear = 1; /* clear interrupt */

    if (timer_cfg[0].timer_cb != NULL) {
        timer_cfg[0].timer_cb(0);
    }
    return;
}

/**
 * \brief           Timer1 Interrupt Handler
 */
void timer1_handler(void) {
    TIMER1->clear = 1; /* clear interrupt */

    if (timer_cfg[1].timer_cb != NULL) {
        timer_cfg[1].timer_cb(1);
    }
    return;
}

/**
 * \brief           Timer2 Interrupt Handler
 */
void timer2_handler(void) {
    TIMER2->clear = 1; /* clear interrupt */

    if (timer_cfg[2].timer_cb != NULL) {
        timer_cfg[2].timer_cb(2);
    }
    return;
}

#if ((CHIP_VERSION == RT58X_MPA) || (CHIP_VERSION == RT58X_MPB))

/**
 * \brief           Slow Timer0 Interrupt Handler
 */
void timer3_handler(void) {
    TIMER3->clear = 1; /* clear interrupt */

    if (timer_cfg[3].timer_cb != NULL) {
        timer_cfg[3].timer_cb(3);
    }
    return;
}

/**
 * \brief           Slow Timer1 Interrupt Handler
 */
void timer4_handler(void) {
    TIMER4->clear = 1; /* clear interrupt */

    if (timer_cfg[4].timer_cb != NULL) {
        timer_cfg[4].timer_cb(4);
    }
    return;
}

#endif
