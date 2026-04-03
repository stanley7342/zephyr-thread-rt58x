/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


/**
 * \file            hosal_pwm.c
 * \brief           Hosal PWM driver file
 */
/*
 * Author:          ives.lee
 */
#include <stdint.h>
#include <stdio.h>
#include "mcu.h"
#include "hosal_pwm.h"
#include "hosal_status.h"
#include "hosal_sysctrl.h"

int hosal_pwm_pin_conifg(uint32_t id, uint32_t pin_number) {

    if ((pin_number < 8) || (pin_number > 23) || (pin_number == 16)
        || (pin_number == 17))
        return HOSAL_STATUS_INVALID_PARAM;

    switch (id) {
        case 0:
            hosal_pin_set_mode(pin_number, (pin_number == 8) ? MODE_PWM : MODE_PWM0);
            break;
        case 1:
            hosal_pin_set_mode(pin_number, (pin_number == 9) ? MODE_PWM : MODE_PWM1);
            break;
        case 2:
            hosal_pin_set_mode(pin_number, (pin_number == 14) ? MODE_PWM : MODE_PWM2);
            break;
        case 3:
            hosal_pin_set_mode(pin_number, (pin_number == 15) ? MODE_PWM : MODE_PWM3);
            break;
        case 4: pin_set_mode(pin_number, MODE_PWM4); break;
        default: break;
    }
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_init_fmt1(hosal_pwm_dev_t* dev) {
    hosal_pwm_pin_conifg(dev->config.id, dev->config.pin_out);
    return pwm_init_fmt1(dev->config.id, dev->config.frequency);
}


int hosal_pwm_init_fmt0(hosal_pwm_dev_t* dev) {
    hosal_pwm_pin_conifg(dev->config.id, dev->config.pin_out);
    return pwm_init_fmt0(dev->config.id, dev->config.frequency,
                            dev->config.count_end_val);
}


int hosal_pwm_fmt1_duty(uint32_t id, uint8_t duty) {
    pwm_fmt1_duty(id, duty);
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_fmt0_duty(uint32_t id, uint8_t duty) {
    pwm_fmt0_duty(id, duty);
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_fmt1_count(uint32_t id, uint32_t count) {
    pwm_fmt1_count(id, count);
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_fmt0_count(uint32_t id, uint32_t count) {
    pwm_fmt0_count(id, count);
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_multi_init(hosal_pwm_dev_t* pwm_dev) {
    hosal_pwm_pin_conifg(pwm_dev->config.id, pwm_dev->config.pin_out);
    pwm_multi_init(pwm_dev->config, pwm_dev->config.frequency);
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_multi_fmt1_duty(uint32_t id, hosal_pwm_dev_t* pwm_dev,
                                 uint32_t element, uint8_t duty) {
    switch (pwm_dev->config.seq_order) {
        case PWM_SEQ_ORDER_T:
        case PWM_SEQ_ORDER_R:
            pwm_multi_fmt1_duty(id, pwm_dev->config.seq_order, element,
                                   duty);
            break;
        default: return HOSAL_STATUS_INVALID_PARAM;
    }
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_multi_fmt0_duty(uint32_t id, hosal_pwm_dev_t* pwm_dev,
                                 uint32_t element, uint8_t thd1_duty,
                                 uint8_t thd2_duty) {
    switch (pwm_dev->config.seq_order) {
        case PWM_SEQ_ORDER_T:
        case PWM_SEQ_ORDER_R:
            pwm_multi_fmt0_duty(id, pwm_dev->config.seq_order, element,
                                   thd1_duty, thd2_duty);
            break;
        default: return HOSAL_STATUS_INVALID_PARAM;
    }
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_multi_fmt1_count(uint32_t id, hosal_pwm_dev_t* pwm_dev,
                                  uint32_t element, uint32_t count) {
    switch (pwm_dev->config.seq_order) {
        case PWM_SEQ_ORDER_T:
        case PWM_SEQ_ORDER_R:
            pwm_multi_fmt1_count(id, pwm_dev->config.seq_order, element,
                                    count);
            break;
        default: return HOSAL_STATUS_INVALID_PARAM;
    }
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_multi_fmt0_count(uint32_t id, hosal_pwm_dev_t* pwm_dev,
                                  uint32_t element, uint32_t thd1_Count,
                                  uint32_t thd2_count) {
    switch (pwm_dev->config.seq_order) {
        case PWM_SEQ_ORDER_T:
        case PWM_SEQ_ORDER_R:
            pwm_multi_fmt0_count(id, pwm_dev->config.seq_order, element,
                                    thd1_Count, thd2_count);
            break;
        default: return HOSAL_STATUS_INVALID_PARAM;
    }
    return HOSAL_STATUS_SUCCESS;
}


int hosal_pwm_start(uint32_t id) { return (int)pwm_start(id); }


int hosal_pwm_stop(uint32_t id) { return (int)pwm_stop(id); }


int hosal_pwm_ioctl(hosal_pwm_dev_t* dev, int ctl, void* p_arg) {
    switch (ctl) {
        case HOSAL_PWM_GET_FRQUENCY:
            return pwm_get_frequency(dev->config.id, p_arg);
        case HOSAL_PWM_SET_FRQUENCY:
            return pwm_set_frequency(dev->config.id, dev->config.frequency);
        case HOSAL_PWM_SET_CLOCK_DIVIDER:
            return pwm_clock_divider(dev->config.id, dev->config.clk_div);
        case HOSAL_PWM_GET_PHASE: return pwm_get_pahse(dev->config.id, p_arg);
        case HOSAL_PWM_SET_PHASE:
            return pwm_set_pahse(dev->config.id, dev->config.phase);
        case HOSAL_PWM_GET_COUNT:
            return pwm_get_count(dev->config.id, p_arg);
        case HOSAL_PWM_SET_COUNT_MODE:
            return pwm_set_counter_mode(dev->config.id,
                                           dev->config.counter_mode);
        case HOSAL_PWM_SET_COUNT_END_VALUE:
            return pwm_set_counter_end_value(dev->config.id,
                                                dev->config.count_end_val);
        case HOSAL_PWM_SET_DMA_FORMAT:
            return pwm_set_dma_format(dev->config.id,
                                         dev->config.dma_smp_fmt);
        case HOSAL_PWM_SET_REPEAT_NUMBER:
            return pwm_set_repeat_number(
                dev->config.id, dev->config.seq_order, *(uint32_t*)p_arg);
        case HOSAL_PWM_GET_REPEAT_NUMBER:
            return pwm_get_repeat_number(dev->config.id,
                                            dev->config.seq_order, p_arg);
        case HOSAL_PWM_SET_DELAY_NUMBER:
            return pwm_set_delay_number(
                dev->config.id, dev->config.seq_order, *(uint32_t*)p_arg);
        case HOSAL_PWM_GET_DELAY_NUMBER:
            return pwm_get_delay_number(dev->config.id,
                                           dev->config.seq_order, p_arg);
        case HOSAL_PWM_SET_INVERT:
            return pwm_set_inveter(dev->config.id,(uint32_t)p_arg);   
         case HOSAL_PWM_GET_INVERT:
            return pwm_get_inveter(dev->config.id,p_arg); 
       case HOSAL_PWM_REGISTER_CALLBACK:
            return pwm_register_callback_function(dev->config.id, (pwm_callback_t) p_arg);                                                                                              
        default: return -1;
    }
}
