/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_sysctrl.c
 * \brief           Hosal system control driver file
 */
/*
 * Author:          Kc.tseng
 */
#include <stdint.h>
#include <stdio.h>
#include "dwt.h"
#include "hosal_status.h"
#include "hosal_sysctrl.h"
#include "mcu.h"
#include "sysctrl.h"

void hosal_delay_us(volatile uint32_t us) { delay_us(us); }

void hosal_delay_ms(volatile uint32_t ms) { delay_ms(ms); }

uint32_t hosal_pin_get_mode(uint32_t pin_number) {
    return pin_get_mode(pin_number);
}

void hosal_pin_set_mode(uint32_t pin_number, uint32_t mode) {
    pin_set_mode(pin_number, mode);
}

void hosal_pin_set_pullopt(uint32_t pin_number, uint32_t mode) {
    pin_set_pullopt(pin_number, mode);
}

void hosal_enable_pin_opendrain(uint32_t pin_number) {
    enable_pin_opendrain(pin_number);
}

void hosal_disable_pin_opendrain(uint32_t pin_number) {
    disable_pin_opendrain(pin_number);
}

void hosal_config_peripherl_clock(uint32_t per_clk, void* cfg_para) {

    if (cfg_para) {

        enable_perclk(per_clk);
    } else {

        disable_perclk(per_clk);
    }
}

int hosal_get_rco_clock_tick(uint32_t* rco_tick) {
    if (rco_tick == NULL) {

        return HOSAL_STATUS_INVALID_PARAM;
    }

    *rco_tick = 40;

    return HOSAL_STATUS_SUCCESS;
}

int hosal_sysctrl_ioctrl(hosal_sys_tmo_t* sys_tmo, int ctl, void* para) {

    int ret = HOSAL_STATUS_SUCCESS;

    switch (ctl) {
        case HOSAL_SYSCTRL_TIMEOUT_INIT: ret = timeout_init(); break;
        case HOSAL_SYSCTRL_TIMEOUT_UNINIT: ret = timeout_uninit(); break;
        case HOSAL_SYSCTRL_TIMEOUT_START: {
            uint32_t timeout = *(uint32_t*)para; //
            if (para == NULL)
                return STATUS_INVALID_PARAM;

            /* ���o�ǤJ���W�ɼƭ� */
            uint32_t ms = *(uint32_t*)para;

            ret = timeout_start(&sys_tmo->tmo, ms, sys_tmo->cb, /* ���T�Ϊk */
                                sys_tmo->arg,
                                sys_tmo->period); /* �άO�z�w�q�� callback_arg */
            break;
        }
        case HOSAL_SYSCTRL_TIMEOUT_CHECK: ret = istimeout(&sys_tmo->tmo); break;

        case HOSAL_SYSCTRL_TIMEOUT_GET_REMAINING:
            ret = timeout_remaining(&sys_tmo->tmo, (uint32_t*)para);
            break;
    }

    return ret;
}
