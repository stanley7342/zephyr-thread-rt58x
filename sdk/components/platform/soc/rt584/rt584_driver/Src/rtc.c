/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            rtc.c
 * \brief           RTC driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "rtc.h"

static rtc_cb_fn rtc_notify_cb;

__STATIC_INLINE uint32_t bcd2bin(uint32_t val) {
    return ((val & 0xF) + ((val & 0xFF) >> 4) * 10);
}

__STATIC_INLINE uint32_t bin2bcd(uint32_t val) {
    return ((val / 10) << 4) | (val % 10);
}

__STATIC_INLINE uint32_t bcd2bin_ms(uint32_t val) {
    return ((val & 0xF) + ((val & 0xFF) >> 4) * 10 + ((val & 0xFFF) >> 8) * 100);
}

__STATIC_INLINE uint32_t bin2bcd_ms(uint32_t val) {
    return (((val / 100) << 8) | (((val / 10) % 10) << 4) | (val % 10));
}

uint32_t rtc_get_time(rtc_time_t *tm) {
    uint32_t temp;

    if (tm == NULL) {
        return STATUS_INVALID_PARAM;
    }

read_again:

    tm->tm_msec = bcd2bin_ms(RTC->rtc_msecond);
    tm->tm_sec  = bcd2bin(RTC->rtc_second);
    tm->tm_min  = bcd2bin(RTC->rtc_minute);
    tm->tm_hour = bcd2bin(RTC->rtc_hour);
    tm->tm_day  = bcd2bin(RTC->rtc_day);
    tm->tm_mon  = bcd2bin(RTC->rtc_month);
    tm->tm_year = bcd2bin(RTC->rtc_year);
    
    temp = bcd2bin(RTC->rtc_second);    /*recheck second again.*/
    
    
    if (temp != tm->tm_sec) {
        /* maybe HH:MM:59 to become HH:(MM+1):00
         * so we read again.*/
        if (temp == 0) {
            goto read_again;
        } else {
            tm->tm_sec = temp;    /*just second update one second.*/
        }
    }
    return STATUS_SUCCESS;
}

uint32_t rtc_set_time(rtc_time_t *tm) {
    uint32_t temp;
    if (tm == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if ( !(RTC->rtc_control & RTC_CTRL_ENABLE) ) {
        rtc_enable();
    }

    /*we don't check input data is valid or not here.
      Caller should ensure it*/
    
    RTC->rtc_msecond = bin2bcd_ms(tm->tm_msec);    
    RTC->rtc_second = bin2bcd(tm->tm_sec);
    RTC->rtc_minute = bin2bcd(tm->tm_min);
    RTC->rtc_hour   = bin2bcd(tm->tm_hour);
    RTC->rtc_day    = bin2bcd(tm->tm_day);
    RTC->rtc_month  = bin2bcd(tm->tm_mon);
    RTC->rtc_year   = bin2bcd(tm->tm_year);

    RTC->rtc_load  = RTC_LOAD_TIME;

    /*wait this take effect, wait RTC_LOAD bcome 0*/
    while (RTC->rtc_load) {}

    return STATUS_SUCCESS;
}

uint32_t rtc_get_alarm(rtc_time_t *tm) {
    if (tm == NULL) {
        return STATUS_INVALID_PARAM;
    }

    tm->tm_msec = bcd2bin_ms(RTC->rtc_alarm_msecond);
    tm->tm_sec  = bcd2bin(RTC->rtc_alarm_second);
    tm->tm_min  = bcd2bin(RTC->rtc_alarm_minute);
    tm->tm_hour = bcd2bin(RTC->rtc_alarm_hour);
    tm->tm_day  = bcd2bin(RTC->rtc_alarm_day);
    tm->tm_mon  = bcd2bin(RTC->rtc_alarm_month);
    tm->tm_year = bcd2bin(RTC->rtc_alarm_year);

    return STATUS_SUCCESS;
}

uint32_t rtc_set_alarm(rtc_time_t* tm, uint32_t rtc_int_mode,
                       rtc_cb_fn rtc_usr_isr) {
    uint32_t  temp_int_reg;

    /*We don't check rtc_int_mode is correct or not...
      caller should use correct setting.*/
    if (tm == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (rtc_usr_isr == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if ( !(RTC->rtc_control & RTC_CTRL_ENABLE) ) {
        rtc_enable();
    }

    /*
     * IRQ Event and IRQ Event Repeat bit in hardware register is bit8 and bit9
     * But In our software flag, each event flag in different position, so we need
     * adjust the shift number depends on the flag type.
     */

    RTC->rtc_alarm_msecond = bin2bcd_ms(tm->tm_msec) | ((rtc_int_mode & RTC_MSEC_MASK) << RTC_IRQEVENT_MSEC_SHIFT);
    RTC->rtc_alarm_second = bin2bcd(tm->tm_sec) | ((rtc_int_mode & RTC_SEC_MASK) << RTC_IRQEVENT_SEC_SHIFT);
    RTC->rtc_alarm_minute = bin2bcd(tm->tm_min) | ((rtc_int_mode & RTC_MIN_MASK) << RTC_IRQEVENT_MIN_SHIFT);
    RTC->rtc_alarm_hour = bin2bcd(tm->tm_hour) | ((rtc_int_mode & RTC_HOUR_MASK) << RTC_IRQEVENT_HOUR_SHIFT);
    RTC->rtc_alarm_day = bin2bcd(tm->tm_day)
                         | ((rtc_int_mode & RTC_DAY_MASK)
                            << RTC_IRQEVENT_DAY_SHIFT);

    RTC->rtc_alarm_month = bin2bcd(tm->tm_mon)
                           | ((rtc_int_mode & RTC_MONTH_MASK)
                              << RTC_IRQEVENT_MONTH_SHIFT);

    RTC->rtc_alarm_year = bin2bcd(tm->tm_year)
                          | ((rtc_int_mode & RTC_YEAR_MASK)
                             >> RTC_IRQEVENT_YEAR_RSHIFT);

    temp_int_reg = (rtc_int_mode >> RTC_INTERRUPT_MASK_SHIFT) & RTC_INTERRUPT_MASK;
    /*clear all interrupt source first*/
    RTC->rtc_int_clear = RTC_INTERRUPT_MASK;

    RTC->rtc_int_control = temp_int_reg;

    /*remember user callback function*/
    rtc_notify_cb = rtc_usr_isr;

    RTC->rtc_load  = RTC_LOAD_ALARM;

    while (RTC->rtc_load) {}

    return STATUS_SUCCESS;
}

uint32_t rtc_disable_alarm(void) {
    /* set control register to disable all interrpt */
    RTC->rtc_int_control = 0;

    /* clear all interrupt source first */
    RTC->rtc_int_clear = RTC_INTERRUPT_MASK;

    return STATUS_SUCCESS;
}


uint32_t calculate_frequency_integer(uint32_t *freq) {
uint32_t i = 0, caldiv = 0;

    do {
        if(RCO32K_CAL->cal32k_result0.bit.cal32k_lock==1) {
            break;
        }
    }
    while(1);

        if((RCO32K_CAL->cal32k_result0.bit.est_32k_result_valid==1) && (RCO32K_CAL->cal32k_result0.bit.cal32k_lock==1)) {
            uint32_t R1 = RCO32K_CAL->cal32k_result0.bit.est_32k_result;  // Mask 20 bits
            //printf("R1:%.8x\r\n",R1);
            
            if (R1 == 0) {
                return STATUS_ERROR;
            }

            uint64_t numerator = ((uint64_t)512000000ULL << 16);
            uint64_t denominator = (uint64_t)R1 * 125ULL;
            uint32_t F_Q16 = (uint32_t)(numerator / denominator);

            uint8_t int_part = (F_Q16 >> 16) & 0xFF;  // ? 8 bit ???
            uint16_t frac_part = (uint16_t)( F_Q16 & 0xFFFF);
            caldiv = (((uint32_t)int_part << 16) | frac_part);
            *freq = caldiv;
            //printf(" %d caldiv=%8x, freq:%.8x\r\n",i,caldiv,*freq);
            
        }
    return STATUS_SUCCESS;
}



uint32_t rtc_set_clk(uint32_t clk) {
    uint32_t status = 0;
    uint32_t cal_clk = clk;

#if defined(CONFIG_EXTRCO32K_ENABLE)
    clk = 0x20C49C; //RCO 32.768K
     //printf("cal_clk=%.8x\r\n",cal_clk);
#else
    status = calculate_frequency_integer(&cal_clk);
    //printf("cal_clk=%.8x\r\n",cal_clk);
    if( status == STATUS_SUCCESS) {
        clk = cal_clk;
    } 
#endif

    RTC->rtc_clock_div = (clk) & 0xFFFFFF; /*only 24bits.*/
    RTC->rtc_load = RTC_LOAD_DIVISOR;

    if ( !(RTC->rtc_control & RTC_CTRL_ENABLE) ) {
        rtc_enable();
    }

    /*wait this take effect, wait RTC_LOAD bcome 0*/
    while (RTC->rtc_load) {}

    return STATUS_SUCCESS;
}

uint32_t rtc_reset(void) {
    RTC->rtc_control = RTC_CTRL_CLRPLS;
    while (RTC->rtc_control & RTC_CTRL_CLRPLS);            /*clear in progress*/

    return STATUS_SUCCESS;
}

uint32_t setup_rtc_wakeup_from_deep_sleep(void) {
    SYSCTRL->sram_lowpower_3.bit.cfg_ds_rco32k_off = 0;

    return STATUS_SUCCESS;
}

uint32_t rtc_enable(void) {
    RTC->rtc_control |= RTC_CTRL_ENABLE;

    return STATUS_SUCCESS;
}

uint32_t rtc_disable(void) {
    RTC->rtc_control &= ~RTC_CTRL_ENABLE;

    return STATUS_SUCCESS;
}

/**
 * \brief           RTC interrupt
 */
void rtc_handler(void) {
    uint32_t status;

    status = RTC->rtc_int_status;
    RTC->rtc_int_clear = status;     /*clear interrupt status.*/
    if (rtc_notify_cb != NULL) {
        /*call RTC user isr*/
        rtc_notify_cb(status);
    }

}
