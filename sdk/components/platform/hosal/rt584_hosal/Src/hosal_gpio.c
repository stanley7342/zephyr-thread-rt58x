/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_gpio.c
 * \brief           Hosal GPIO driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "stdio.h"
#include <stdint.h>
#include "mcu.h"
#include "hosal_gpio.h"


uint32_t hosal_gpio_cfg_output(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_cfg(pin_number, GPIO_PIN_DIR_OUTPUT, GPIO_PIN_NOINT);

    return rval;
}

uint32_t hosal_gpio_cfg_input(uint32_t pin_number,
                          hosal_gpio_input_config_t input_cfg) {
    uint32_t rval;

    rval = gpio_cfg(pin_number, GPIO_PIN_DIR_INPUT, 
             (gpio_pin_int_mode_t)input_cfg.pin_int_mode);

    if (rval == STATUS_SUCCESS) {
        if (input_cfg.usr_cb != NULL) {
            rval = gpio_register_callback(pin_number, input_cfg.usr_cb, input_cfg.param);
        }
    }
    
    return rval;
}

uint32_t hosal_gpio_pin_set(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_pin_set(pin_number);

    return rval;
}

uint32_t hosal_gpio_pin_clear(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_pin_clear(pin_number);

    return rval;
}

uint32_t hosal_gpio_pin_write(uint32_t pin_number, uint32_t value) {
    uint32_t rval;

    rval = gpio_pin_write(pin_number, value);

    return rval;
}

uint32_t hosal_gpio_pin_toggle(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_pin_toggle(pin_number);

    return rval;
}

uint32_t hosal_gpio_pin_get(uint32_t pin_number, uint32_t* value) {
    uint32_t rval;

    rval = gpio_pin_get(pin_number, value);

    return rval;
}

uint32_t hosal_gpio_int_enable(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_int_enable(pin_number);

    return rval;
}

uint32_t hosal_gpio_int_disable(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_int_disable(pin_number);

    return rval;
}

uint32_t hosal_gpio_debounce_enable(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_debounce_enable(pin_number);

    return rval;
}

uint32_t hosal_gpio_debounce_disable(uint32_t pin_number) {
    uint32_t rval;

    rval = gpio_debounce_disable(pin_number);

    return rval;
}

uint32_t hosal_gpio_set_debounce_time(uint32_t time) {
    uint32_t rval;

    rval = gpio_set_debounce_time(time);

    return rval;
}

uint32_t hosal_gpio_setup_deep_sleep_io(uint8_t pin_number, gpio_pin_wake_t level) {
    uint32_t rval;

    rval = gpio_setup_deep_sleep_io(pin_number, level);

    return rval;
}

uint32_t hosal_gpio_disable_deep_sleep_io(uint8_t pin_number) {
    uint32_t rval;

    rval = gpio_disable_deep_sleep_io(pin_number);

    return rval;
}

uint32_t hosal_gpio_setup_deep_powerdown_io(uint8_t pin_number, gpio_pin_wake_t level) {
    uint32_t rval;

    rval = gpio_setup_deep_powerdown_io(pin_number, level);

    return rval;
}

uint32_t hosal_gpio_disable_deep_powerdown_io(uint8_t pin_number) {
    uint32_t rval;

    rval = gpio_disable_deep_powerdown_io(pin_number);

    return rval;
}


uint32_t hosal_gpio_cfg_input_parameters(uint32_t pin_number, 
                                         hosal_gpio_input_config_t input_cfg, bool debounce_en) {
    uint32_t rval;

    rval = gpio_cfg(pin_number, GPIO_PIN_DIR_INPUT, 
             (gpio_pin_int_mode_t)input_cfg.pin_int_mode);
    if (rval == STATUS_SUCCESS) {
        if (input_cfg.usr_cb != NULL) {
            rval = gpio_register_callback(pin_number, input_cfg.usr_cb, input_cfg.param);
        }
    }

    if ( debounce_en ) {
        rval = gpio_debounce_enable(pin_number);
    }

    if ( input_cfg.pin_int_mode != GPIO_PIN_NOINT) {
        rval = gpio_int_enable(pin_number);
    }
    return rval;
}
