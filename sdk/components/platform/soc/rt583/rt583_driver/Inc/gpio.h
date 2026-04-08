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

#include "mcu.h"


/**
 * \defgroup GPIO Gpio
 * \ingroup RT58X_DRIVER
 * \brief  Define Gpio definitions, structures, and functions
 * @{
 */

/**
 * \brief           Gpio pin define
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

/**
 * \brief           Max number of gpio pin
 */
#define MAX_NUMBER_OF_PINS    (32)             /*!< Specify Maximum Pins of GPIO */

/**
 * \brief           Gpio debounce_time definitions
 */
#define DEBOUNCE_SLOWCLOCKS_32 (0)              /*!< setting for sampling cycle =
                                                 32 clocks   */
#define DEBOUNCE_SLOWCLOCKS_64 (1)              /*!< setting for sampling cycle =
                                                 64 clocks   */
#define DEBOUNCE_SLOWCLOCKS_128 (2)             /*!< setting for sampling cycle =
                                                 128 clocks  */
#define DEBOUNCE_SLOWCLOCKS_256 (3)             /*!< setting for sampling cycle =
                                                 256 clocks  */
#define DEBOUNCE_SLOWCLOCKS_512 (4)             /*!< setting for sampling cycle =
                                                 512 clocks  */
#define DEBOUNCE_SLOWCLOCKS_1024 (5)            /*!< setting for sampling cycle =
                                                 1024 clocks */
#define DEBOUNCE_SLOWCLOCKS_2048 (6)            /*!< setting for sampling cycle =
                                                 2048 clocks */
#define DEBOUNCE_SLOWCLOCKS_4096 (7)            /*!< setting for sampling cycle =
                                                 4096 clocks */

/**
 * \brief           Pin direction definitions.
 */
typedef enum {
    GPIO_PIN_DIR_INPUT,                         /*!< GPIO Input Mode   */
    GPIO_PIN_DIR_OUTPUT,                        /*!< GPIO Output Mode  */
    GPIO_PIN_DIR_INVALID
} gpio_pin_dir_t;

/**
 * \brief           Selecting the pin to sense high or low level, edge for pin input.
 */
typedef enum {
    GPIO_PIN_NOINT,                 /*!< GPIO Interrupt mode disable  */
    GPIO_PIN_INT_LEVEL_LOW,         /*!< GPIO Interrupt enable for Level-Low */
    GPIO_PIN_INT_LEVEL_HIGH,        /*!< GPIO Interrupt enable for Level-High  */
    GPIO_PIN_INT_EDGE_RISING,       /*!< GPIO Interrupt enable for Rising Edge */
    GPIO_PIN_INT_EDGE_FALLING,      /*!< GPIO Interrupt enable for Falling Edge */
    GPIO_PIN_INT_BOTH_EDGE,         /*!< GPIO Interrupt enable for both Rising and Falling Edge  */
} gpio_pin_int_mode_t;

/**
 * \brief           GPIO interrupt service routine callback for user application.
 * \param[in]       pin: interrupt pin number
 * \param[in]       isr_param: isr_param passed to user interrupt handler
 */
typedef void (*gpio_cb_fn)(uint32_t pin, void* isr_param);

/**
 * \brief           GPIO pin configuration function
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       dir: pin direction.
 * \param[in]       int_mode: pin interrupt mode
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_cfg(uint32_t pin_number, gpio_pin_dir_t dir,
              gpio_pin_int_mode_t int_mode);

/**
 * \brief           Register user interrupt ISR callback function.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       gpio_cb: Specifies user callback function when 
 *                           the pin interrupt generated.
 * \param[in]       param: passed to user interrupt handler "gpio_cb"
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_callback_register(uint32_t pin_number, gpio_cb_fn gpio_cb,
                            void* param);

/**
 * \brief           Set gpio pin to output mode
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_cfg_output(uint32_t pin_number);

/**
 * \brief           Set gpio pin to input mode
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       int_mode: specifies the pin number interrupt if this pin 
 *                            need to be gpio interrupt source
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_cfg_input(uint32_t pin_number, gpio_pin_int_mode_t int_mode);

/**
 * \brief           Set gpio pin output high
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_set(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    GPIO->state.output_high = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Set gpio pin output low
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_clear(uint32_t pin_number) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    GPIO->int_status.output_low = (1 << pin_number);
    return STATUS_SUCCESS;
}

/**
 * \brief           Set gpio pin output value
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       value: value 0 for output low, value 1 for output high
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_pin_write(uint32_t pin_number, uint32_t value);

/**
 * \brief           Toggle gpio pin output value
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_pin_toggle(uint32_t pin_number);

/**
 * \brief           Get gpio pin input value
 * \param[in]       pin_number: specifies the pin number
 * \param[out]      value: the pointer to get gpio input value
 *                  1 for input pin is high, 0 for input is low.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
__STATIC_INLINE uint32_t gpio_pin_get(uint32_t pin_number, uint32_t* value) {
    if( pin_number >= MAX_NUMBER_OF_PINS ) {
        return STATUS_INVALID_PARAM;
    }

    *value =  ((GPIO->state.state & (1 << pin_number)) ? 1 : 0);
    return STATUS_SUCCESS;
}

/**
 * \brief           Enable gpio pin interrupt
 * \param[in]       pin_number: specifies the pin number that enable interrup
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAMt
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
 * \param[in]       pin_number: specifies the pin number that disable interrupt
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
 * \brief           Enable gpio pin debounce function
 * \param[in]       pin_number: specifies the pin number that enable debounce 
 *                              when interrupt happened
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_debounce_enable(uint32_t pin_number);

/**
 * \brief           Disable gpio pin debounce function
 * \param[in]       pin_number: specifies the pin number that disable debounce 
 *                              when interrupt happened
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_debounce_disable(uint32_t pin_number);

/**
 * \brief           Set GPIO debounce time
 * \param[in]       mode: Specifies the sampling clock of debounce function 
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t gpio_set_debounce_time(uint32_t mode);

/*@}*/ /* end of RT58X_DRIVER GPIO */

#ifdef __cplusplus
}
#endif

#endif /* End of GPIO_H */
