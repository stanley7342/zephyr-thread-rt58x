/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_gpio.h
 * \brief           Hosal GPIO driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_GPIO_H
#define HOSAL_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gpio.h"



/**
 * \defgroup        HOSAL_GPIO Hosal gpio
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal gpio definitions, structures, and functions
 * @{
 */

/**
 * \brief           Selecting the pin to sense high or low level, edge for pin input.
 */
#define HOSAL_GPIO_PIN_NOINT            GPIO_PIN_NOINT              /*!< GPIO Interrupt 
                                                                    mode disable */
#define HOSAL_GPIO_PIN_INT_LEVEL_LOW    GPIO_PIN_INT_LEVEL_LOW      /*!< GPIO Interrupt 
                                                                    enable for Level-Low */
#define HOSAL_GPIO_PIN_INT_LEVEL_HIGH   GPIO_PIN_INT_LEVEL_HIGH     /*!< GPIO Interrupt 
                                                                    enable for Level-High */
#define HOSAL_GPIO_PIN_INT_EDGE_RISING  GPIO_PIN_INT_EDGE_RISING    /*!< GPIO Interrupt 
                                                                    enable for Rising Edge */
#define HOSAL_GPIO_PIN_INT_EDGE_FALLING GPIO_PIN_INT_EDGE_FALLING   /*!< GPIO Interrupt 
                                                                    enable for Falling Edge */
#define HOSAL_GPIO_PIN_INT_BOTH_EDGE    GPIO_PIN_INT_BOTH_EDGE      /*!< GPIO Interrupt 
                                                                    enable for both Rising and Falling Edge */

/**
 * \brief           Selecting the pin to wake up high or low level.
 */
#define HOSAL_GPIO_LEVEL_LOW    GPIO_LEVEL_LOW  /*!< GPIO Level-Low wake up */
#define HOSAL_GPIO_LEVEL_HIGH   GPIO_LEVEL_HIGH /*!< GPIO Level-High wake up */

/**
 * \brief           Input configuration structure
 */
typedef struct {
    void* usr_cb;
    void* param;
    uint32_t pin_int_mode;
} hosal_gpio_input_config_t;


/**
 * \brief           Set gpio pin to output mode
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_cfg_output(uint32_t pin_number);

/**
 * \brief           Set gpio pin to input mode
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       input_cfg: specifies the pin number input parameter
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_cfg_input(uint32_t pin_number,
                          hosal_gpio_input_config_t input_cfg);

/**
 * \brief           Set gpio pin output high
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_pin_set(uint32_t pin_number);

/**
 * \brief           Set gpio pin output low
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_pin_clear(uint32_t pin_number);

/**
 * \brief           Set gpio pin output value
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       value: value 0 for output low, value 1 for output high
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_pin_write(uint32_t pin_number, uint32_t value);

/**
 * \brief           Toggle gpio pin output value
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_pin_toggle(uint32_t pin_number);

/**
 * \brief           Get gpio pin input value
 * \param[in]       pin_number: specifies the pin number
 * \param[out]      value: the pointer to get gpio input value
 *                  pin input value,0 is low level, 1 is high level
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_pin_get(uint32_t pin_number, uint32_t* value);

/**
 * \brief           Enable gpio pin interrupt
 * \param[in]       pin_number: specifies the pin number that enable interrupt
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_int_enable(uint32_t pin_number);

/**
 * \brief           Disable gpio pin interrupt
 * \param[in]       pin_number: specifies the pin number that disable interrupt
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_int_disable(uint32_t pin_number);

/**
 * \brief           Enable gpio pin debounce function
 * \param[in]       pin_number: specifies the pin number that enable debounce
 *                              when interrupt happened
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_debounce_enable(uint32_t pin_number);

/**
 * \brief           Disable gpio pin debounce function
 * \param[in]       pin_number: specifies the pin number that disable debounce
 *                              when interrupt happened
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_debounce_disable(uint32_t pin_number);

/**
 * \brief           Set GPIO debounce time
 * \param[in]       time: Specifies the sampling clock of debounce function
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_set_debounce_time(uint32_t time);

/**
 * \brief           Setup gpio wakeup from deep sleep with level high or low.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       level: set wakeup polarity low or high in deepsleep.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_setup_deep_sleep_io(uint8_t pin_number, gpio_pin_wake_t level);

/**
 * \brief           Disable gpio wakeup from deep sleep.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_disable_deep_sleep_io(uint8_t pin_number);

/**
 * \brief           Setup gpio wakeup from deep power down with level high or low.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       level: set wakeup polarity low or high in deep power down.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_setup_deep_powerdown_io(uint8_t pin_number, gpio_pin_wake_t level);

/**
 * \brief           Disable gpio wakeup from deep power down.
 * \param[in]       pin_number: specifies the pin number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_disable_deep_powerdown_io(uint8_t pin_number);


/**
 * \brief           Set gpio pin to input mode and related parameter.
 * \param[in]       pin_number: specifies the pin number
 * \param[in]       input_cfg: specifies the pin number input parameter
 * \param[in]       debounce_en: debounce enable or not
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_gpio_cfg_input_parameters(uint32_t pin_number, 
                                         hosal_gpio_input_config_t input_cfg, bool debounce_en);

/*@}*/ /* end of RT584_HOSAL HOSAL_GPIO */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_GPIO_H */
