/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_rtc.c
 * \brief           Hosal RTC driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "stdio.h"
#include <stdint.h>
#include "mcu.h"
#include "hosal_rtc.h"


uint32_t hosal_rtc_get_time(void *tm) {
    uint32_t rval;

    rval = rtc_get_time(tm);

    return rval;
}

uint32_t hosal_rtc_set_time(void *tm) {
    uint32_t rval;

    rval = rtc_set_time(tm);

    return rval;
}

uint32_t hosal_rtc_get_alarm(void *tm) {
    uint32_t rval;

    rval = rtc_get_alarm(tm);

    return rval;
}

uint32_t hosal_rtc_set_alarm(void *tm, uint32_t rtc_int_mode,
                             void *rtc_usr_isr) {
    uint32_t rval;

    rval = rtc_set_alarm(tm, rtc_int_mode, rtc_usr_isr);

    return rval;
}

uint32_t hosal_rtc_disable_alarm(void) {
    uint32_t rval;

    rval = rtc_disable_alarm();

    return rval;
}

uint32_t hosal_rtc_set_clk(uint32_t clk) {
    uint32_t rval;

    rval = rtc_set_clk(clk);

    return rval;
}

uint32_t hosal_rtc_reset(void) {
    uint32_t rval;

    rval = rtc_reset();

    return rval;
}

uint32_t hosal_rtc_enable(void) { 
    uint32_t rval;

    rval = rtc_enable();

    return rval;
}

uint32_t hosal_rtc_disable(void) { 
    uint32_t rval;

    rval = rtc_disable();

    return rval;
}
