/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            gpio.c
 * \brief           GPIO driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "gpio.h"
#include "sysctrl.h"

/**
 * \brief           Gpio_cb save callback and the p_context
 */
typedef struct {
    gpio_cb_fn gpio_handler;                    /*!< user application ISR handler. */
    void* p_context;                            /*!< the context to callback */
} gpio_cb;

static gpio_cb user_isr[MAX_NUMBER_OF_PINS];

uint32_t gpio_cfg(uint32_t pin_number, gpio_pin_dir_t dir, gpio_pin_int_mode_t int_mode) {
    uint32_t  MASK;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    if( dir >= GPIO_PIN_DIR_INVALID ) {
        return STATUS_INVALID_PARAM;
    }

    MASK = (1 << pin_number);

    if (dir == GPIO_PIN_DIR_INPUT) {
        GPIO->input_en = MASK;
        GPIO->enable_input = MASK;
        pin_set_pullopt(pin_number,PULLUP_100K);

    } else {
        GPIO->disable_input = MASK;
        GPIO->output_en = MASK;
        pin_set_pullopt(pin_number, PULL_NONE);
    }

    //clear IRQ and handler
    GPIO->disable_int = MASK;
    GPIO->edge_int_clr = MASK;
    user_isr[pin_number].gpio_handler = NULL;

    switch (int_mode) {
        case GPIO_PIN_INT_LEVEL_LOW:
            GPIO->negative   = MASK;
            GPIO->level_trig = MASK;
            break;
        case GPIO_PIN_INT_LEVEL_HIGH:
            GPIO->postitive  = MASK;
            GPIO->level_trig = MASK;
            break;
        case GPIO_PIN_INT_EDGE_RISING:
            GPIO->postitive  = MASK;
            GPIO->edge_trig  = MASK;
            break;
        case GPIO_PIN_INT_EDGE_FALLING:
            GPIO->negative   = MASK;
            GPIO->edge_trig  = MASK;
            break;
        case GPIO_PIN_INT_BOTH_EDGE:
            GPIO->both_edge_en = MASK;
            GPIO->edge_trig  = MASK;
            break;

        case GPIO_PIN_NOINT:
        default:
            GPIO->disable_int = MASK;
            break;
    }

    return STATUS_SUCCESS;
}

uint32_t gpio_setup_deep_sleep_io(uint8_t pin_number, gpio_pin_wake_t level) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;
    GPIO->set_ds_en |= mask;
    if ( level == GPIO_LEVEL_LOW ) {
        GPIO->dis_ds_inv |= mask;
    } else {
        GPIO->set_ds_inv |= mask;
    }
    return STATUS_SUCCESS;
}

uint32_t gpio_disable_deep_sleep_io(uint8_t pin_number) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;
    GPIO->dis_ds_en |= mask;

    return STATUS_SUCCESS;
}

uint32_t gpio_setup_deep_powerdown_io(uint8_t pin_number, gpio_pin_wake_t level) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;

    DPD_CTRL->dpd_gpio_en |= mask;

    if ( level == GPIO_LEVEL_LOW ) {
        DPD_CTRL->dpd_gpio_inv &= ~mask;
    } else {
        DPD_CTRL->dpd_gpio_inv |= mask;
    }

    return STATUS_SUCCESS;
}

uint32_t gpio_disable_deep_powerdown_io(uint8_t pin_number) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;

    DPD_CTRL->dpd_gpio_en &= ~mask;

    return STATUS_SUCCESS;
}

uint32_t gpio_setup_io_schmitt(uint8_t pin_number) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;

    SYSCTRL->gpio_en_schmitt |= mask;

    return STATUS_SUCCESS;
}

uint32_t gpio_disable_io_schmitt(uint8_t pin_number) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;

    SYSCTRL->gpio_en_schmitt &= ~mask;

    return STATUS_SUCCESS;
}

uint32_t gpio_setup_io_filter(uint8_t pin_number) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;

    SYSCTRL->gpio_en_filter |= mask;

    return STATUS_SUCCESS;
}

uint32_t gpio_disable_io_filter(uint8_t pin_number) {
    uint32_t mask;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    mask = 1 << pin_number;

    SYSCTRL->gpio_en_filter &= ~mask;

    return STATUS_SUCCESS;
}

uint32_t gpio_register_callback(uint32_t pin_number, gpio_cb_fn app_gpio_callback,
                            void* param) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    if( app_gpio_callback != NULL) {
        user_isr[pin_number].gpio_handler = app_gpio_callback;
        user_isr[pin_number].p_context = param;
    }

    return STATUS_SUCCESS;
}

uint32_t gpio_cancell_callback(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    user_isr[pin_number].gpio_handler = NULL;
    user_isr[pin_number].p_context = NULL;

    return STATUS_SUCCESS;
}

/**
 * \brief           GPIO Interrupt Handler
 */
void gpio_handler(void) {
    gpio_cb_fn app_isr;

    uint32_t  irq_state;
    uint32_t  i = 0, Mask = 1;

    irq_state = GPIO->int_status;
    for (i = 0; i < MAX_NUMBER_OF_PINS; i++, Mask <<= 1) {
        if (irq_state & Mask) {
            app_isr = user_isr[i].gpio_handler;

            /*clear Edgeinterrupt status..
             * if the interrupt source is level trigger, this clear
             * does NOT have change...
             */
            if (app_isr != NULL) {
                app_isr(i, user_isr[i].p_context);
            }
            GPIO->edge_int_clr = Mask;
        }
    }

    return;
}
