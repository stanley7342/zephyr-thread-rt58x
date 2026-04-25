/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            timer.h
 * \brief           timer driver header file
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
 * \defgroup        TIMER Timer
 * \ingroup         RT584_DRIVER
 * \brief           Define Timer definitions, structures, and functions
 * @{
 */

/**
 * \brief           Timer counting mode definitions.
 */
#define TIMER_DOWN_COUNTING                     0
#define TIMER_UP_COUNTING                       1

/**
 * \brief           Timer one-shot mode definitions.
 */
#define TIMER_ONE_SHOT_DISABLE                  0
#define TIMER_ONE_SHOT_ENABLE                   1

/**
 * \brief           Timer mode definitions.
 */
#define TIMER_FREERUN_MODE                      0
#define TIMER_PERIODIC_MODE                     1

/**
 * \brief           Timer interrupt definitions.
 */
#define TIMER_INT_DISABLE                       0
#define TIMER_INT_ENABLE                        1

/**
 * \brief           Timer prescale definitions.
 */
#define TIMER_PRESCALE_1                        0
#define TIMER_PRESCALE_2                        3
#define TIMER_PRESCALE_8                        4
#define TIMER_PRESCALE_16                       1
#define TIMER_PRESCALE_32                       5
#define TIMER_PRESCALE_128                      6
#define TIMER_PRESCALE_256                      2
#define TIMER_PRESCALE_1024                     7

/**
 * \brief           Timer capture edge definitions.
 */
#define TIMER_CAPTURE_POS_EDGE                  0
#define TIMER_CAPTURE_NEG_EDGE                  1

/**
 * \brief           Timer capture deglich definitions.
 */
#define TIMER_CAPTURE_DEGLICH_DISABLE           0
#define TIMER_CAPTURE_DEGLICH_ENABLE            1

/**
 * \brief           Timer capture interrupt definitions.
 */
#define TIMER_CAPTURE_INT_DISABLE               0
#define TIMER_CAPTURE_INT_ENABLE                1

/**
 * \brief           Timer clock source definitions.
 */
#define TIMER_CLOCK_SOURCEC_PERI                0
#define TIMER_CLOCK_SOURCEC_RCO1M               2
#define TIMER_CLOCK_SOURCEC_PMU                 3



/**
 * \brief           Timer interrupt service routine callback for user application.
 * \param[in]       timer_id: timer id for the interrupt source
 */
typedef void (*timer_cb_fn)(uint32_t timer_id);

/**
 * \brief           timer config structure holding configuration settings for the timer
 */
typedef struct {
    uint8_t  counting_mode : 1;                 /*!< Set counting mode */
    uint8_t  oneshot_mode  : 1;                 /*!< Enable one shot */
    uint8_t  mode          : 1;                 /*!< Set Freerun or Periodic mode */
    uint8_t  int_en        : 1;                 /*!< Enable Interrupt */
    uint8_t  prescale      : 3;                 /*!< Set prescale */
    uint16_t user_prescale : 10;                /*!< Set user define prescale */
} timer_config_mode_t;


/**
 * \brief           timer capture config structure holding configuration settings
 *                  for the timer.
 */
typedef struct {
    uint8_t  counting_mode      : 1;            /*!< Set counting mode */
    uint8_t  oneshot_mode       : 1;            /*!< Enable one shot */
    uint8_t  mode               : 1;            /*!< Set Freerun or Periodic mode */
    uint8_t  int_en             : 1;            /*!< Enable Interrupt */
    uint8_t  prescale           : 3;            /*!< Set prescale */
    uint16_t user_prescale      : 10;           /*!< Set user define prescale */
    uint8_t  ch0_capture_edge   : 1;            /*!< Set Capture channel0 trigger edge */
    uint8_t  ch0_deglich_enable : 1;            /*!< Enable Capture channel0 deglitch */
    uint8_t  ch0_int_enable     : 1;            /*!< Enable Capture channel0 interrupt */
    uint8_t  ch0_iosel          : 5;            /*!< Set Capture channel0 gpio */
    uint8_t  ch1_capture_edge   : 1;            /*!< Set Capture channel1 trigger edge */
    uint8_t  ch1_deglich_enable : 1;            /*!< Enable Capture channel1 deglitch */
    uint8_t  ch1_int_enable     : 1;            /*!< Enable Capture channel1 interrupt */
    uint8_t  ch1_iosel          : 5;            /*!< Set Capture channel1 gpio */
} timer_capture_config_mode_t;

/**
 * \brief           timer pwm config structure holding configuration settings 
 *                  for the timer
 */
typedef struct {
    uint8_t  counting_mode : 1;                 /*!< Set counting mode */
    uint8_t  oneshot_mode  : 1;                 /*!< Enable one shot */
    uint8_t  mode          : 1;                 /*!< Set Freerun or Periodic mode */
    uint8_t  int_en        : 1;                 /*!< Enable Interrupt */
    uint8_t  prescale      : 3;                 /*!< Set prescale */
    uint16_t user_prescale : 10;                /*!< Set user define prescale */
    uint8_t  pwm0_enable   : 1;                 /*!< Enable Pwm channel0 */
    uint8_t  pwm1_enable   : 1;                 /*!< Enable Pwm channel1 */
    uint8_t  pwm2_enable   : 1;                 /*!< Enable Pwm channel2 */
    uint8_t  pwm3_enable   : 1;                 /*!< Enable Pwm channel3 */
    uint8_t  pwm4_enable   : 1;                 /*!< Enable Pwm channel4 */
} timer_pwm_config_mode_t;

/**
 * \brief           slow timer config structure holding configuration settings 
 *                  for the timer.
 */
typedef struct {
    uint8_t     counting_mode : 1;              /*!< Set counting mode */
    uint8_t     oneshot_mode  : 1;              /*!< Enable one shot */
    uint8_t     mode          : 1;              /*!< Set Freerun or Periodic mode */
    uint8_t     int_en        : 1;              /*!< Enable Interrupt */
    uint8_t     prescale      : 3;              /*!< Set prescale */
    uint16_t    user_prescale : 10;             /*!< Set user define prescale */
    uint16_t    repeat_delay;                   /*!< Set repeat delay count */
} slowtimer_config_mode_t;

/**
 * \brief           Register timer callback function
 * \param[in]       timer_id: Specifies timer id number 
 * \param[in]       timer_cb: User callback function
 */
uint32_t timer_callback_register(uint32_t timer_id, timer_cb_fn timer_cb);

/**
 * \brief           Timer parameter setting 
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       cfg: Timer configuration
 * \param[in]       timer_cb: Timer callback function
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t timer_open(uint32_t timer_id, timer_config_mode_t cfg,
                    timer_cb_fn timer_cb);

/**
 * \brief           Load timer ticks
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       timeload_ticks: Timer reload tick
 * \param[in]       timeout_ticks: Timer timeout tick
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 */
uint32_t timer_load(uint32_t timer_id, uint32_t timeload_ticks,
                    uint32_t timeout_ticks);

/**
 * \brief           Start timer
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       timeload_ticks: Timer reload tick
 * \param[in]       timeout_ticks: Timer timeout tick
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t timer_start(uint32_t timer_id, uint32_t timeload_ticks,
                     uint32_t timeout_ticks);

/**
 * \brief           Stop timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t timer_stop(uint32_t timer_id);

/**
 * \brief           Clear timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM
 */
uint32_t timer_close(uint32_t timer_id);

/**
 * \brief           Get the timer interrupt status
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      int_status: Pointer to a user-provided variable 
 *                              where the slow timer interrupt status
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_int_status_get(uint32_t timer_id, uint32_t* int_status);

/**
 * \brief           Get the timer current value
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      tick_value: Pointer to a user-provided variable 
 *                              where the timer current tick value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_current_get(uint32_t timer_id, uint32_t* tick_value);

/**
 * \brief           Get the timer enable status
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      en_status: Pointer to a user-provided variable 
 *                             where the slow timer enable status will be stored.
 *                             0 : enable
 *                             1 : disable
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t get_timer_enable_status(uint32_t timer_id, uint32_t* en_status);

/**
 * \brief           Timer capture parameter setting 
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       cfg: Timer capture configuration
 * \param[in]       timer_cb: Timer callback function
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t timer_capture_open(uint32_t timer_id, timer_capture_config_mode_t cfg,
                            timer_cb_fn timer_cb);

/**
 * \brief           Start capture timer
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       timeload_ticks: Timer reload tick
 * \param[in]       timeout_ticks: Timer timeout tick
 * \param[in]       chanel0_enable: Enable channel0
 * \param[in]       chanel1_enable: Enable channel1
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t timer_capture_start(uint32_t timer_id, uint32_t timeload_ticks, 
                             uint32_t timeout_ticks, bool chanel0_enable, 
                             bool chanel1_enable);

/**
 * \brief           Stop capture timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t timer_capture_stop(uint32_t timer_id);

/**
 * \brief           Clear capture timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_capture_close(uint32_t timer_id);

/**
 * \brief           Get the timer capture channel 0 current value
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      ticl_value: Pointer to a user-provided variable 
 *                              where the timer capture channel 0 tick 
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_ch0_capture_value_get(uint32_t timer_id, uint32_t* ticl_value);

/**
 * \brief           Get the timer capture channel0 interrupt status
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      int_status: Pointer to a user-provided variable 
 *                              where the timer capture interrupt status
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_ch0_capture_int_status(uint32_t timer_id, uint32_t* int_status);

/**
 * \brief           Get the timer capture channel 1 current value
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      ticl_value: Pointer to a user-provided variable 
 *                              where the timer capture channel 1 tick 
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_ch1_capture_value_get(uint32_t timer_id, uint32_t* ticl_value);

/**
 * \brief           Get the timer capture channel1 interrupt status
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      int_status: Pointer to a user-provided variable 
 *                              where the timer capture interrupt status
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t timer_ch1_capture_int_status(uint32_t timer_id, uint32_t* int_status);

/**
 * \brief           Timer pwm parameter setting 
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       cfg: Timer pwm configuration
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t timer_pwm_open(uint32_t timer_id, timer_pwm_config_mode_t cfg);

/**
 * \brief           Start pwm timer
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       timeload_ticks: Timer reload tick
 * \param[in]       timeout_ticks: Timer timeout tick
 * \param[in]       threshold: The pwm change phase threshold
 * \param[in]       phase: The pwm start phase
 * \return          Function status: 
 *                  STATUS_SUCCESS, 
 *                  STATUS_INVALID_PARAM,
 *                  STATUS_NO_INIT
 */
uint32_t timer_pwm_start(uint32_t timer_id, uint32_t timeload_ticks, 
                         uint32_t timeout_ticks, uint32_t threshold, bool phase);

/**
 * \brief           Stop pwm timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM,
 *                  STATUS_NO_INIT
 */
uint32_t timer_pwm_stop(uint32_t timer_id);

/**
 * \brief           Clear pwm timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM,
 */
uint32_t timer_pwm_close(uint32_t timer_id);

/**
 * \brief           Register slow timer callback function
 * \param[in]       timer_id: Specifies slow timer id number 
 * \param[in]       timer_cb: User callback function
 */
uint32_t slowtimer_callback_register(uint32_t timer_id, timer_cb_fn timer_cb);

/**
 * \brief           Get the slow timer enable status
 * \param[in]       timer_id: Specifies slow timer id number
 * \param[out]      en_status: Pointer to a user-provided variable 
 *                             where the slow timer enable status will be stored.
 *                             0 : enable
 *                             1 : disable
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t get_slowtimer_enable_status(uint32_t timer_id, uint32_t* en_status);

/**
 * \brief           Slow timer parameter setting 
 * \param[in]       timer_id: Specifies slow timer id number
 * \param[in]       cfg: Slow timer configuration
 * \param[in]       timer_cb: Slow timer callback function
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM,
 *                  STATUS_INVALID_REQUEST
 */
uint32_t slowtimer_open(uint32_t timer_id, slowtimer_config_mode_t cfg,
                        timer_cb_fn timer_cb);

/**
 * \brief           Load slow timer ticks
 * \param[in]       timer_id: Specifies slow timer id number
 * \param[in]       timeload_ticks: Slow timer reload tick
 * \param[in]       timeout_ticks: Slow timer timeout tick
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM, 
 */
uint32_t slowtimer_load(uint32_t timer_id, uint32_t timeload_ticks,
                        uint32_t timeout_ticks);

/**
 * \brief           Start slow timer
 * \param[in]       timer_id: Specifies slow timer id number
 * \param[in]       timeload_ticks: Slow timer reload tick
 * \param[in]       timeout_ticks: Slow timer timeout tick
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t slowtimer_start(uint32_t timer_id, uint32_t timeload_ticks,
                          uint32_t timeout_ticks);

/**
 * \brief           Stop slow timer
 * \param[in]       timer_id: Specifies slow timer id number
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t slowtimer_stop(uint32_t timer_id);

/**
 * \brief           Clear slow timer
 * \param[in]       timer_id: Specifies slow timer id number
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM, 
 */
uint32_t slowtimer_close(uint32_t timer_id);

/**
 * \brief           Clear slow timer interrupt
 * \param[in]       timer_id: Specifies slow timer id number
 * \return          Function status:
 *                  STATUS_SUCCESS,
 *                  STATUS_INVALID_PARAM, 
 */
uint32_t slowtimer_clear_int(uint32_t timer_id);

/**
 * \brief           Get the slow timer interrupt status
 * \param[in]       timer_id: Specifies slow timer id number
 * \param[out]      int_status: Pointer to a user-provided variable 
 *                              where the slow timer interrupt status
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t slowtimer_int_status_get(uint32_t timer_id, uint32_t* int_status);

/**
 * \brief           Get the slow timer current value
 * \param[in]       timer_id: Specifies slow timer id number
 * \param[out]      tick_value: Pointer to a user-provided variable 
 *                              where the timer current tick value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t slowtimer_current_get(uint32_t timer_id, uint32_t* tick_value);

/*@}*/ /* end of RT584_DRIVER TIMER */

#ifdef __cplusplus
}
#endif

#endif /* End of TIMER_H */
