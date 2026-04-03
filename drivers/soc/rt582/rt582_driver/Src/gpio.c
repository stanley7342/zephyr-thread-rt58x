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


#include "mcu.h"


/**
 * \brief           Gpio_cb save callback and the p_context
 */
typedef struct {
    gpio_cb_fn gpio_handler; /*!< user application ISR handler. */
    void* p_context;         /*!< the context to callback */
} gpio_cb;

static gpio_cb user_isr[MAX_NUMBER_OF_PINS];

uint32_t gpio_cfg(uint32_t pin_number, gpio_pin_dir_t dir,
              gpio_pin_int_mode_t int_mode) {
    uint32_t MASK;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    if( dir >= GPIO_PIN_DIR_INVALID ) {
        return STATUS_INVALID_PARAM;
    }


    MASK = (1 << pin_number);

    if (dir == GPIO_PIN_DIR_INPUT) {
        GPIO->input_en = MASK;
    } else {
        GPIO->output_en = MASK;

        /* For output we should set the pin to no pull mode
         * Because for pull high mode if system enter to sleep/deep sleep,
         * and this pin output low will cause power leakage.
         */
        pin_set_pullopt(pin_number, PULL_NONE);
    }

    /* should we set interrupt mode to default first? */
    user_isr[pin_number].gpio_handler = NULL;

    switch (int_mode) {
        case GPIO_PIN_INT_LEVEL_LOW:
            GPIO->negative = MASK;
            GPIO->level_trig = MASK;
            break;

        case GPIO_PIN_INT_LEVEL_HIGH:
            GPIO->postitive = MASK;
            GPIO->level_trig = MASK;
            break;

        case GPIO_PIN_INT_EDGE_RISING:
            GPIO->postitive = MASK;
            GPIO->edge_trig = MASK;
            break;

        case GPIO_PIN_INT_EDGE_FALLING:
            GPIO->negative = MASK;
            GPIO->edge_trig = MASK;
            break;

        case GPIO_PIN_INT_BOTH_EDGE:
            GPIO->both_edge_en = MASK;
            GPIO->edge_trig = MASK;
            break;

        case GPIO_PIN_NOINT:
        default: GPIO->disable_int = MASK; break;
    }

    return STATUS_SUCCESS;
}

uint32_t gpio_callback_register(uint32_t pin_number, gpio_cb_fn app_gpio_callback,
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

uint32_t gpio_cfg_output(uint32_t pin_number) {
    return gpio_cfg(pin_number, GPIO_PIN_DIR_OUTPUT, GPIO_PIN_NOINT);
}

uint32_t gpio_cfg_input(uint32_t pin_number, gpio_pin_int_mode_t int_mode) {
    return gpio_cfg(pin_number, GPIO_PIN_DIR_INPUT, int_mode);
}

uint32_t gpio_pin_write(uint32_t pin_number, uint32_t value) {
    if (value == 0) {
        return gpio_pin_clear(pin_number);
    } else {
        return gpio_pin_set(pin_number);
    }
}

uint32_t gpio_pin_toggle(uint32_t pin_number) {
    uint32_t state, MASK;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    MASK = (1 << pin_number);
    state = GPIO->state.state & MASK;

    if (state) {
        GPIO->int_status.output_low = MASK;
    } else {
        GPIO->state.output_high = MASK;
    }
    return STATUS_SUCCESS;
}

uint32_t gpio_debounce_enable(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }
    GPIO->debounce_en = (1 << pin_number);
    return STATUS_SUCCESS;
}

uint32_t gpio_debounce_disable(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }
    GPIO->debounce_dis = (1 << pin_number);
    return STATUS_SUCCESS;
}

uint32_t gpio_set_debounce_time(uint32_t mode) {
    if( mode > DEBOUNCE_SLOWCLOCKS_4096 ) {
        return STATUS_INVALID_PARAM;
    }

    GPIO->debounce_time = mode;
    return STATUS_SUCCESS;
}

/**
 * \brief           GPIO Interrupt Handler
 */
void gpio_handler(void) {
    gpio_cb_fn app_isr;

    uint32_t irq_state, i, Mask;

    i = 0;
    Mask = 1;

    irq_state = GPIO->int_status.int_status;
    //dprintf("get irq_state %08x, pin state %8x \n", irq_state, GPIO->STATE);

    for (i = 0; i < MAX_NUMBER_OF_PINS; i++, Mask <<= 1) {
        if (irq_state & Mask) {
            app_isr = user_isr[i].gpio_handler;

            assert_param(app_isr); /*it should not be NULL!*/

            if (app_isr != NULL) {
                app_isr(i, user_isr[i].p_context);
            }

            /* clear Edgeinterrupt status..
             * if the interrupt source is level trigger, this clear
             * does NOT have change...
             */
            GPIO->edge_int_clr = Mask;
        }
    }

    return;
}
