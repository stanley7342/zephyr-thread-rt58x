/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            timer.h
 * \brief           Timer driver header file
 */
/*
 * Author:          Kc.tseng
 */
#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"


/**
 * \defgroup TIMER Timer
 * \ingroup RT58X_DRIVER
 * \brief  Define Timer definitions, structures, and functions
 * @{
 */

/**
 * \brief           Timer mode definitions.
 */
#define TIMER_FREERUN_MODE  0
#define TIMER_PERIODIC_MODE 1

/**
 * \brief           Timer prescale definitions.
 */
#define TIMER_PRESCALE_1    0
#define TIMER_PRESCALE_2    3
#define TIMER_PRESCALE_8    4
#define TIMER_PRESCALE_16   1
#define TIMER_PRESCALE_32   5
#define TIMER_PRESCALE_128  6
#define TIMER_PRESCALE_256  2
#define TIMER_PRESCALE_1024 7

/**
 * \brief           TIMER interrupt service routine callback for user application.
 * \param[in]       timer_id: timer id number
 */
typedef void (*timer_cb_fn)(uint32_t timer_id);

/**
 * \brief           Timer config structure holding configuration settings for the 
 *                  timer and slow timer.
 */
typedef struct {
    uint8_t mode     : 1;                       /*!< Timer operating mode setting. */
    uint8_t prescale : 3;                       /*!< Timer prescale setting. */
    uint8_t int_en   : 1;                       /*!< Timer interrupt enable setting. */
    uint16_t repeat_delay;                      /*!< Slow timer repeat delay times */
} timer_config_mode_t;

/**
 * \brief           Register timer callback function
 * \param[in]       timer_id: timer id number 
 * \param[in]       timer_cb: user callback function
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t timer_callback_register(uint32_t timer_id, timer_cb_fn timer_cb);

/**
 * \brief           Timer parameter setting 
 * \param[in]       timer_id: timer id number
 * \param[in]       mode: timer configuration
 * \param[in]       timer_cb: timer callback function
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t timer_open(uint32_t timer_id, timer_config_mode_t mode,
                    timer_cb_fn timer_cb);

/**
 * \brief           Timer load timeout value 
 * \param[in]       timer_id: timer id number
 * \param[in]       timeout_ticks: timeout value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_load(uint32_t timer_id, uint32_t timeout_ticks);

/**
 * \brief           Timer start working with tiomeout value
 * \param[in]       timer_id: timer id number
 * \param[in]       timeout_ticks: timeout value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, S
 *                  TATUS_NO_INIT
 */
uint32_t timer_start(uint32_t timer_id, uint32_t timeout_ticks);

/**
 * \brief           Stop timer
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t timer_stop(uint32_t timer_id);

/**
 * \brief           Clear timer interrupt 
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_clear_int(uint32_t timer_id);

/**
 * \brief           Close timer 
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_close(uint32_t timer_id);

/**
 * \brief           Get timer current tick value
 * \param[in]       timer_id: timer id number
 * \param[out]      tick_value: Pointer to a user-provided variable 
 *                              where the timer current tick value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_current_get(uint32_t timer_id, uint32_t* tick_value);

/*@}*/ /* end of RT58X_DRIVER TIMER */

#ifdef __cplusplus
}
#endif

#endif /* End of TIMER_H */
