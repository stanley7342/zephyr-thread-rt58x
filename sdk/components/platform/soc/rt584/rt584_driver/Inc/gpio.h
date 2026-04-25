/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            gpio.h
 * \brief           GPIO driver header file
 */
/*
 * Author:          Kc.tseng
 */
#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "mcu.h"


/**
 * \defgroup        GPIO Gpio
 * \ingroup         RT584_DRIVER
 * \brief           Define Gpio comp definitions, structures, and functions
 * @{
 */

/**
 * \brief           Max number of gpio pin
 */
#define GPIO0        0
#define GPIO1        1
#define GPIO2        2
#define GPIO3        3
#define GPIO4        4
#define GPIO5        5
#define GPIO6        6
#define GPIO7        7
#define GPIO8        8
#define GPIO9        9
#define GPIO10       10
#define GPIO11       11
#define GPIO12       12
#define GPIO13       13
#define GPIO14       14
#define GPIO15       15
#define GPIO16       16
#define GPIO17       17
#define GPIO18       18
#define GPIO19       19
#define GPIO20       20
#define GPIO21       21
#define GPIO22       22
#define GPIO23       23
#define GPIO24       24
#define GPIO25       25
#define GPIO26       26
#define GPIO27       27
#define GPIO28       28
#define GPIO29       29
#define GPIO30       30
#define GPIO31       31
#define MAX_NUMBER_OF_PINS    (32)             /*!< Specify Maximum Pins of GPIO */

/**
 * \brief           GPIO_DEBOUNCE CLOCK Constant Definitions
 */
#define  DEBOUNCE_SLOWCLOCKS_32     (0)         /*!< setting for sampling cycle 
                                                    = 32 clocks */
#define  DEBOUNCE_SLOWCLOCKS_64     (1)         /*!< setting for sampling cycle 
                                                    = 64 clocks */
#define  DEBOUNCE_SLOWCLOCKS_128    (2)         /*!< setting for sampling cycle 
                                                    = 128 clocks */
#define  DEBOUNCE_SLOWCLOCKS_256    (3)         /*!< setting for sampling cycle 
                                                    = 256 clocks */
#define  DEBOUNCE_SLOWCLOCKS_512    (4)         /*!< setting for sampling cycle 
                                                    = 512 clocks */
#define  DEBOUNCE_SLOWCLOCKS_1024   (5)         /*!< setting for sampling cycle 
                                                    = 1024 clocks */
#define  DEBOUNCE_SLOWCLOCKS_2048   (6)         /*!< setting for sampling cycle 
                                                    = 2048 clocks */
#define  DEBOUNCE_SLOWCLOCKS_4096   (7)         /*!< setting for sampling cycle 
                                                    = 4096 clocks */


/**
 * \brief           Pin direction definitions.
 */
typedef enum {
    GPIO_PIN_DIR_INPUT,                         /*!< GPIO Input Mode */
    GPIO_PIN_DIR_OUTPUT,                        /*!< GPIO Output Mode */
    GPIO_PIN_DIR_INVALID
} gpio_pin_dir_t;

/**
 * \brief           Selecting the pin to sense high or low level, edge for pin input.
 */
typedef enum {
    GPIO_PIN_NOINT,                             /*!< GPIO Interrupt mode disable */
    GPIO_PIN_INT_LEVEL_LOW,                     /*!< GPIO Interrupt enable for 
                                                    Level-Low */
    GPIO_PIN_INT_LEVEL_HIGH,                    /*!< GPIO Interrupt enable for 
                                                    Level-High */
    GPIO_PIN_INT_EDGE_RISING,                   /*!< GPIO Interrupt enable for 
                                                    Rising Edge */
    GPIO_PIN_INT_EDGE_FALLING,                  /*!< GPIO Interrupt enable for 
                                                    Falling Edge */
    GPIO_PIN_INT_BOTH_EDGE,                     /*!< GPIO Interrupt enable for 
                                                    both Rising and Falling Edge */
} gpio_pin_int_mode_t;

/**
 * \brief           Selecting the pin to wake up high or low level.
 */
typedef enum {
    GPIO_LEVEL_LOW,                             /*!< GPIO Level-Low wake up */
    GPIO_LEVEL_HIGH,                            /*!< GPIO Level-High wake up */
} gpio_pin_wake_t;

/**
 * \brief           GPIO interrupt service routine callback for user application.
 * \param[in]       pin_number: interrupt pin number
 * \param[in]       isr_param: isr_param passed to user interrupt handler
 */
typedef void (*gpio_cb_fn)(uint32_t pin_number, void* isr_param);

/**
 * \brief           GPIO pin configuration function
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       dir: pin direction.
 * \param[in]       int_mode: pin interrupt mode
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_cfg(uint32_t pin_number, gpio_pin_dir_t dir, gpio_pin_int_mode_t int_mode);

/**
 * \brief           Register user interrupt ISR callback function.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       gpio_cb: specifies user callback function when 
 *                           the pin interrupt generated.
 * \param[in]       param: passed to user interrupt handler "gpio_cb"
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_register_callback(uint32_t pin_number, gpio_cb_fn gpio_cb,
                            void* param);

/**
 * \brief           Setup gpio wakeup from deep sleep with level high or low.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       level: set wakeup polarity low or high in deepsleep.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_setup_deep_sleep_io(uint8_t pin_number, gpio_pin_wake_t level);

/**
 * \brief           Disable gpio wakeup from deep sleep.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_disable_deep_sleep_io(uint8_t pin_number);

/**
 * \brief           Setup gpio wakeup from deep power down with level high or low.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       level: set wakeup polarity low or high in deep power down.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_setup_deep_powerdown_io(uint8_t pin_number, gpio_pin_wake_t level);

/**
 * \brief           Disable gpio wakeup from deep power down.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_disable_deep_powerdown_io(uint8_t pin_number);

/**
 * \brief           Setup gpio Schmitt.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_setup_io_schmitt(uint8_t pin_number);

/**
 * \brief           Disable gpio Schmitt.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_disable_io_schmitt(uint8_t pin_number);

/**
 * \brief           Setup gpio filter.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_setup_io_filter(uint8_t pin_number);

/**
 * \brief           Disable gpio filter.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_disable_io_filter(uint8_t pin_number);

/**
 * \brief           Set gpio pin to output mode.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_cfg_output(uint32_t pin_number) {
    return gpio_cfg(pin_number, GPIO_PIN_DIR_OUTPUT, GPIO_PIN_NOINT);
}

/**
 * \brief           Set pin input enable.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t gpio_input_enable(uint32_t pin_number) {
    /* because W1R, so only write pin mask */
    GPIO->input_en = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Set gpio pin to input mode.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       int_mode: specifies the pin number interrupt if this pin 
 *                            need to be gpio interrupt source
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_cfg_input(uint32_t pin_number, 
                                    gpio_pin_int_mode_t int_mode) {
    return gpio_cfg(pin_number, GPIO_PIN_DIR_INPUT, int_mode);
}

/**
 * \brief           Set gpio pin output high.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_set(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    GPIO->output_high = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Set gpio pin output low.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_clear(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    GPIO->output_low = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Set gpio pin output value.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       value: value 0 for output low, value 1 for output high.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_write(uint32_t pin_number, uint32_t value) {
    if (value == 0)     {
        return gpio_pin_clear(pin_number);
    } else {
        return gpio_pin_set(pin_number);
    }
}

/**
 * \brief           Toggle gpio pin output value.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_toggle(uint32_t pin_number) {
    uint32_t state, MASK;

    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    MASK = (1 << pin_number);
    state = GPIO->output_state & MASK;

    if (state) {
        GPIO->output_low = MASK;
    } else {
        GPIO->output_high = MASK;
    }
    return STATUS_SUCCESS;
}

/**
 * \brief           Get gpio pin input value.
 * \param[in]       pin_number: specifies the pin number
 * \param[out]      value: the pointer to get gpio input value
 *                  1 for input pin is high, 0 for input is low.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_get(uint32_t pin_number, uint32_t* value) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    *value = ((GPIO->state & (1 << pin_number)) ? 1 : 0);
    return STATUS_SUCCESS;
}

/**
 * \brief           Enable gpio pin interrupt.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_int_enable(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }
    GPIO->enable_int = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Disable gpio pin interrupt
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_int_disable(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }
    GPIO->disable_int = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Enable gpio pin debounce function.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_debounce_enable(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }
    GPIO->debouce_en = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Disable gpio pin debounce function.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_debounce_disable(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }
    GPIO->debouce_dis = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Set GPIO debounce time.
 * \param[in]       time: Specifies the sampling clock of debounce function.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_set_debounce_time(uint32_t time) {
    if( time > DEBOUNCE_SLOWCLOCKS_4096 ) {
        return STATUS_INVALID_PARAM;
    }

    GPIO->debounce_time = time;

    return STATUS_SUCCESS;
}

/**
 * \brief           Get gpio output state.
 * \param[in]       pin_number: specifies the pin number
 * \param[out]      value: the pointer to get gpio input value
 *                  1 for input pin is high, 0 for input is low.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_get_output_state(uint32_t* value) {

    *value = GPIO->output_state;
    return STATUS_SUCCESS;
}


/**
 * \brief           Disable gpio callback.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t gpio_cancell_callback(uint32_t pin_number);

/*@}*/ /* end of RT584_DRIVER GPIO */

#ifdef __cplusplus
}
#endif

#endif /* End of GPIO_H */

