/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            timer.c
 * \brief           timer driver file
 */
/*
 * Author:          Kc.tseng
 */

#include <stdio.h>
#include "timer.h"
#include "sysctrl.h"

/**
 * \brief           Timer and slow timer max number definitions
 */
#define MAX_NUMBER_OF_TIMER           3         /*3 timer*/
#define MAX_NUMBER_OF_SLOW_TIMER      2         /*2 timer*/

/**
 * \brief           Timer and slow timer state definitions
 */
#define TIMER_STATE_OPEN           1
#define TIMER_STATE_CLOSED         0


/**
 * \brief           timer_cb save callback and the timer state
 */
typedef struct {
    timer_cb_fn timer_callback;                 /*!< user application callback. */
    uint8_t     state;                          /*!< device state. */
} timer_cb;

static timer_cb timer_cfg[MAX_NUMBER_OF_TIMER];

/**
 * \brief           slowtimer_cb save callback and the slow timer state
 */
typedef struct {
    timer_cb_fn timer_callback;                 /*!< user application callback */
    uint8_t     state;                          /*!< device state. */
} slowtimer_cb;

static slowtimer_cb slowtimer_cfg[MAX_NUMBER_OF_SLOW_TIMER];

uint32_t timer_callback_register(uint32_t timer_id, timer_cb_fn timer_callback) {
    timer_cfg[timer_id].timer_callback = timer_callback;
    return STATUS_SUCCESS;
}

uint32_t get_timer_enable_status(uint32_t timer_id, uint32_t* en_status) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    *en_status = timer->control.bit.timer_enable_status;

    return STATUS_SUCCESS;
}

uint32_t timer_open(uint32_t timer_id, timer_config_mode_t cfg,
                    timer_cb_fn timer_callback) {
    uint32_t en_status;
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};


    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_CLOSED) {
        /* device already opened */
        return STATUS_INVALID_REQUEST;
    }

    timer = base[timer_id];

    timer->control.reg = 0;
    do {
        get_timer_enable_status(timer_id, &en_status);
    }while(en_status);

    timer->clear = 1;

    if (cfg.user_prescale) {
        timer->prescale = cfg.user_prescale;
    } else {
        timer->prescale = 0;
        timer->control.bit.prescale = cfg.prescale;
    }

    timer->control.bit.up_count = cfg.counting_mode;
    timer->control.bit.one_shot_en = cfg.oneshot_mode;
    timer->control.bit.mode = cfg.mode;
    timer->control.bit.int_enable = cfg.int_en;

    timer_callback_register(timer_id, timer_callback);

    timer_cfg[timer_id].state = TIMER_STATE_OPEN;

    return STATUS_SUCCESS;
}

uint32_t timer_load(uint32_t timer_id, uint32_t timeload_ticks, uint32_t timeout_ticks) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->load = (timeload_ticks - 1);
    timer->expried_value = timeout_ticks;
    while ( !((timer->load == timer->value) && (timer->load == (timeload_ticks - 1))) ) {}

    return STATUS_SUCCESS;
}

uint32_t timer_start(uint32_t timer_id, uint32_t timeload_ticks, uint32_t timeout_ticks) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        /* device should be opened first */
        return STATUS_NO_INIT;    
    }

    timer = base[timer_id];

    timer_load(timer_id, timeload_ticks, timeout_ticks);
    timer->control.bit.en = 1;
    return STATUS_SUCCESS;
}

uint32_t timer_stop(uint32_t timer_id) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        /* device should be opened first */
        return STATUS_NO_INIT;
    }

    timer = base[timer_id];
    timer->control.bit.en = 0;

    return STATUS_SUCCESS;
}

uint32_t timer_close(uint32_t timer_id) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->control.reg = 0;

    

    timer_cfg[timer_id].timer_callback = NULL;
    timer_cfg[timer_id].state = TIMER_STATE_CLOSED;

    return STATUS_SUCCESS;
}

uint32_t timer_clear_int(uint32_t timer_id) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->control.reg = 0;

    

    timer_cfg[timer_id].timer_callback = NULL;
    timer_cfg[timer_id].state = TIMER_STATE_CLOSED;

    return STATUS_SUCCESS;
}

uint32_t timer_int_status_get(uint32_t timer_id, uint32_t* int_status) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    *int_status = timer->control.bit.int_status;

    return STATUS_SUCCESS;
}

uint32_t timer_current_get(uint32_t timer_id, uint32_t* tick_value) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    *tick_value = timer->value;

    return STATUS_SUCCESS;
}

uint32_t timer_capture_open(uint32_t timer_id, timer_capture_config_mode_t cfg,
                            timer_cb_fn timer_callback) {
    uint32_t en_status;
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_CLOSED) {
        /*device already opened*/
        return STATUS_INVALID_REQUEST;    
    }

    timer = base[timer_id];

    timer->control.reg = 0;
    do {
        get_timer_enable_status(timer_id, &en_status);
    }while(en_status);
    timer->clear = 1;
    timer->capture_clear.bit.ch0_capture_int_clear = 1;
    timer->capture_clear.bit.ch1_capture_int_clear = 1;

    if (cfg.user_prescale) {
        timer->prescale = cfg.user_prescale;
    } else {
        timer->prescale = 0;
        timer->control.bit.prescale = cfg.prescale;
    }

    timer->control.bit.up_count = cfg.counting_mode;
    timer->control.bit.one_shot_en = cfg.oneshot_mode;
    timer->control.bit.mode = cfg.mode;
    timer->control.bit.int_enable = cfg.int_en;

    timer->control.bit.ch0_capture_edge = cfg.ch0_capture_edge;
    timer->control.bit.ch0_capture_int_en = cfg.ch0_int_enable;
    timer->control.bit.ch0_deglich_en = cfg.ch0_deglich_enable;
    timer->cap_io_sel.bit.ch0_capture_io_sel = cfg.ch0_iosel;

    timer->control.bit.ch1_capture_edge = cfg.ch1_capture_edge;
    timer->control.bit.ch1_capture_int_en = cfg.ch1_int_enable;
    timer->control.bit.ch1_deglich_en = cfg.ch1_deglich_enable;
    timer->cap_io_sel.bit.ch1_capture_io_sel = cfg.ch1_iosel;

    timer_callback_register(timer_id, timer_callback);

    timer_cfg[timer_id].state = TIMER_STATE_OPEN;

    return STATUS_SUCCESS;
}

uint32_t timer_capture_start(uint32_t timer_id, uint32_t timeload_ticks, 
                             uint32_t timeout_ticks, bool chanel0_enable, 
                             bool chanel1_enable) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        /* device should be opened first */
        return STATUS_NO_INIT;
    }

    timer = base[timer_id];

    timer_load(timer_id, timeload_ticks, timeout_ticks);
    timer->control.bit.en = 1;
    timer->cap_en.bit.ch0_capture_en = chanel0_enable;
    timer->cap_en.bit.ch1_capture_en = chanel1_enable;

    /*printf("Timer%d, chanel0_enable:%d, chanel0_enable:%d\r\n", timer_id,
           chanel0_enable, chanel1_enable);

    printf("control:%8x, cap_io_sel:%d, cap_en:%d\r\n", timer->control.reg,
           timer->cap_io_sel.reg, timer->cap_en.reg);*/

    return STATUS_SUCCESS;
}

uint32_t timer_capture_stop(uint32_t timer_id) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        /* device should be opened first */
        return STATUS_NO_INIT;
    }

    timer = base[timer_id];
    timer->control.bit.en = 1;
    timer->cap_en.bit.ch0_capture_en = 0;
    timer->cap_en.bit.ch1_capture_en = 0;

    return STATUS_SUCCESS;
}

uint32_t timer_capture_close(uint32_t timer_id) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->control.reg = 0;
    timer->cap_io_sel.reg = 0;

    timer_cfg[timer_id].timer_callback = NULL;
    timer_cfg[timer_id].state = TIMER_STATE_CLOSED;

    return STATUS_SUCCESS;
}

uint32_t timer_ch0_capture_value_get(uint32_t timer_id, uint32_t* ticl_value) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};
    uint32_t value;

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];

    *ticl_value = timer->ch0_cap_value;
    timer->capture_clear.bit.ch0_capture_int_clear = 1;

    return STATUS_SUCCESS;
}

uint32_t timer_ch0_capture_int_status(uint32_t timer_id, uint32_t* int_status) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    *int_status = timer->control.bit.ch0_capture_int_status;

    return STATUS_SUCCESS;
}

uint32_t timer_ch1_capture_value_get(uint32_t timer_id, uint32_t* ticl_value) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};
    uint32_t value;

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];

    *ticl_value = timer->ch1_cap_value;
    timer->capture_clear.bit.ch1_capture_int_clear = 1;

    return STATUS_SUCCESS;
}

uint32_t timer_ch1_capture_int_status(uint32_t timer_id, uint32_t* int_status) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    *int_status = timer->control.bit.ch1_capture_int_status;

    return STATUS_SUCCESS;
}

uint32_t timer_pwm_open(uint32_t timer_id,
                        timer_pwm_config_mode_t cfg) {
    uint32_t en_status;
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};


    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_CLOSED) {
        /* device already opened */
        return STATUS_INVALID_REQUEST;    
    }

    timer = base[timer_id];

    timer->control.reg = 0;
    do {
        get_timer_enable_status(timer_id, &en_status);
    }while(en_status);
    timer->clear = 1;

    if (cfg.user_prescale) {
        timer->prescale = cfg.user_prescale;
    } else {
        timer->prescale = 0;
        timer->control.bit.prescale = cfg.prescale;
    }

    timer->control.bit.up_count = cfg.counting_mode;
    timer->control.bit.one_shot_en = cfg.oneshot_mode;
    timer->control.bit.mode = cfg.mode;
    timer->control.bit.int_enable = cfg.int_en;

    /* select PMU_CLK */
    if (timer_id == 0) {
        SYSCTRL->sys_clk_ctrl1.bit.timer0_clk_sel = TIMER_CLOCK_SOURCEC_PMU;
    } else if (timer_id == 1) {
        SYSCTRL->sys_clk_ctrl1.bit.timer1_clk_sel = TIMER_CLOCK_SOURCEC_PMU;
    } else if (timer_id == 2) {
        SYSCTRL->sys_clk_ctrl1.bit.timer2_clk_sel = TIMER_CLOCK_SOURCEC_PMU;
    }

    if (cfg.pwm0_enable) {
        SYSCTRL->soc_pwm_sel.bit.pwm0_src_sel = timer_id + 1;
    }

    if (cfg.pwm1_enable) {
        SYSCTRL->soc_pwm_sel.bit.pwm1_src_sel = timer_id + 1;
    }

    if (cfg.pwm2_enable) {
        SYSCTRL->soc_pwm_sel.bit.pwm2_src_sel = timer_id + 1;
    }

    if (cfg.pwm3_enable) {
        SYSCTRL->soc_pwm_sel.bit.pwm3_src_sel = timer_id + 1;
    }

    if (cfg.pwm4_enable) {
        SYSCTRL->soc_pwm_sel.bit.pwm4_src_sel = timer_id + 1;
    }

    timer_cfg[timer_id].state = TIMER_STATE_OPEN;

    return STATUS_SUCCESS;
}

uint32_t timer_pwm_start(uint32_t timer_id, uint32_t timeload_ticks, 
                         uint32_t timeout_ticks, uint32_t threshold, 
                         bool phase) {
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        /* device should be opened first */
        return STATUS_NO_INIT;
    }

    timer = base[timer_id];

    timer_load(timer_id, timeload_ticks, timeout_ticks);
    timer->cap_en.bit.timer_pwm_en = 1;
    timer->control.bit.en = 1;
    timer->thd = threshold;
    timer->pha.bit.pha = phase;

    return STATUS_SUCCESS;
}

uint32_t timer_pwm_stop(uint32_t timer_id)
{
    timern_t *timer;
    timern_t *base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (timer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        return STATUS_NO_INIT;    /*DEVIC SHOULD BE OPEN FIRST.*/
    }

    timer = base[timer_id];

    timer->cap_en.bit.timer_pwm_en = 0;
    timer->control.bit.en = 0;

    return STATUS_SUCCESS;
}

uint32_t timer_pwm_close(uint32_t timer_id) {
    timern_t* timer;
    timern_t* base[MAX_NUMBER_OF_TIMER] = {TIMER0, TIMER1, TIMER2};

    if (timer_id > MAX_NUMBER_OF_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->control.reg = 0;

    timer_cfg[timer_id].timer_callback = NULL;
    timer_cfg[timer_id].state = TIMER_STATE_CLOSED;

    return STATUS_SUCCESS;
}

uint32_t slowtimer_callback_register(uint32_t timer_id,
                                     timer_cb_fn timer_callback) {
    slowtimer_cfg[timer_id].timer_callback = timer_callback;
    return STATUS_SUCCESS;
}

uint32_t get_slowtimer_enable_status(uint32_t timer_id, uint32_t* en_status) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];

    *en_status = timer->control.bit.timer_enable_status;

    return STATUS_SUCCESS;
}

uint32_t slowtimer_open(uint32_t timer_id, slowtimer_config_mode_t cfg,
                        timer_cb_fn timer_callback) {
    uint32_t en_status;
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (slowtimer_cfg[timer_id].state != TIMER_STATE_CLOSED) {
        return STATUS_INVALID_REQUEST;    /*device already opened*/
    }

    timer = base[timer_id];

    timer->control.reg = 0;
    do {
        get_timer_enable_status(timer_id, &en_status);
    }while(en_status);
    timer->clear = 1;

    if (cfg.user_prescale) {
        timer->prescale = cfg.user_prescale;
    } else {
        timer->prescale = 0;
        timer->control.bit.prescale = cfg.prescale;
    }

    timer->control.bit.up_count = cfg.counting_mode;
    timer->control.bit.one_shot_en = cfg.oneshot_mode;
    timer->control.bit.mode = cfg.mode;
    timer->control.bit.int_enable = cfg.int_en;
    if (cfg.repeat_delay) {
        timer->repeat_delay.bit.int_repeat_delay_disable = 0;
        timer->repeat_delay.bit.int_repeat_delay = cfg.repeat_delay;
    } else {
        timer->repeat_delay.bit.int_repeat_delay_disable = 1;
    }

    slowtimer_cfg[timer_id].state = TIMER_STATE_OPEN;

    slowtimer_callback_register(timer_id, timer_callback);
    

    return STATUS_SUCCESS;
}

uint32_t slowtimer_load(uint32_t timer_id, uint32_t timeload_ticks,
                        uint32_t timeout_ticks) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->load = (timeload_ticks - 1);
    timer->expried_value = timeout_ticks;
    while ( !((timer->load == timer->value) && (timer->load == (timeload_ticks - 1))) );

    return STATUS_SUCCESS;
}

uint32_t slowtimer_start(uint32_t timer_id, uint32_t timeload_ticks,
                         uint32_t timeout_ticks) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (slowtimer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        return STATUS_NO_INIT;    /*DEVIC SHOULD BE OPEN FIRST.*/
    }

    timer = base[timer_id];

    slowtimer_load(timer_id, timeload_ticks, timeout_ticks);
    timer->control.bit.en = 1;

    return STATUS_SUCCESS;
}

uint32_t slowtimer_stop(uint32_t timer_id) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    if (slowtimer_cfg[timer_id].state != TIMER_STATE_OPEN) {
        return STATUS_NO_INIT;    /*DEVIC SHOULD BE OPEN FIRST.*/
    }

    timer = base[timer_id];
    timer->control.bit.en = 0;         /*Disable timer*/

    return STATUS_SUCCESS;
}

uint32_t slowtimer_close(uint32_t timer_id) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->control.reg = 0;         /*Disable timer*/

    slowtimer_cfg[timer_id].timer_callback = NULL;
    slowtimer_cfg[timer_id].state = TIMER_STATE_CLOSED;

    return STATUS_SUCCESS;
}

uint32_t slowtimer_clear_int(uint32_t timer_id) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    timer->clear = 1;
    return STATUS_SUCCESS;
}

uint32_t slowtimer_int_status_get(uint32_t timer_id, uint32_t* int_status) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }

    timer = base[timer_id];
    *int_status = timer->control.bit.int_status;

    return STATUS_SUCCESS;
}

uint32_t slowtimer_current_get(uint32_t timer_id, uint32_t* tick_value) {
    slowtimern_t *timer;
    slowtimern_t* base[MAX_NUMBER_OF_SLOW_TIMER] = {SLOWTIMER0, SLOWTIMER1};

    if (timer_id > MAX_NUMBER_OF_SLOW_TIMER) {
        return STATUS_INVALID_PARAM;
    }
    timer = base[timer_id];
    *tick_value = timer->value;

    return STATUS_SUCCESS;
}

/**
 * \brief           Timer0 Interrupt Handler
 */
void timer0_handler(void)
{
    TIMER0->clear = 1;

    if (timer_cfg[0].timer_callback != NULL) {
        timer_cfg[0].timer_callback(0);
    }
    return;
}

/**
 * \brief           Timer1 Interrupt Handler
 */
void timer1_handler(void) {

    TIMER1->clear = 1;

    if (timer_cfg[1].timer_callback != NULL) {
        timer_cfg[1].timer_callback(1);
    }
    return;
}

/**
 * \brief           Timer2 Interrupt Handler
 */
void timer2_handler(void) {
    TIMER2->clear = 1;

    if (timer_cfg[2].timer_callback != NULL) {
        timer_cfg[2].timer_callback(2);
    }
    return;
}

/**
 * \brief           Slow Timer0 Interrupt Handler
 */
void slowtimer0_handler(void)
{
    if (SLOWTIMER0->control.bit.one_shot_en) {
        SLOWTIMER0->control.bit.en = 0;
    }

    SLOWTIMER0->clear = 1;

    if (slowtimer_cfg[0].timer_callback != NULL) {
        slowtimer_cfg[0].timer_callback(0);
    }
    return;
}

/**
 * \brief           Slow Timer1 Interrupt Handler
 */
void slowtimer1_handler(void) {
    if (SLOWTIMER1->control.bit.one_shot_en) {
        SLOWTIMER1->control.bit.en = 0;
    }

    SLOWTIMER1->clear = 1;

    if (slowtimer_cfg[1].timer_callback != NULL) {
        slowtimer_cfg[1].timer_callback(1);
    }
    return;
}
