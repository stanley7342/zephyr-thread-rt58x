/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_timer.h
 * \brief           Hosal Timer driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_TIMER_H
#define HOSAL_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "timer.h"



/**
 * \defgroup        HOSAL_TIMER Hosal timer
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal timer definitions, structures, and functions
 * @{
 */

/**
 * \brief           Hosal Timer counting mode definitions.
 */
#define HOSAL_TIMER_DOWN_COUNTING               TIMER_DOWN_COUNTING
#define HOSAL_TIMER_UP_COUNTING                 TIMER_UP_COUNTING

/**
 * \brief           Hosal Timer one-shot mode definitions.
 */
#define HOSAL_TIMER_ONE_SHOT_DISABLE            TIMER_ONE_SHOT_DISABLE
#define HOSAL_TIMER_ONE_SHOT_ENABLE             TIMER_ONE_SHOT_ENABLE

/**
 * \brief           Hosal Timer mode definitions.
 */
#define HOSAL_TIMER_FREERUN_MODE                TIMER_FREERUN_MODE
#define HOSAL_TIMER_PERIODIC_MODE               TIMER_PERIODIC_MODE

/**
 * \brief           Hosal Timer interrupt definitions.
 */
#define HOSAL_TIMER_INT_DISABLE                 TIMER_INT_DISABLE
#define HOSAL_TIMER_INT_ENABLE                  TIMER_INT_ENABLE

/**
 * \brief           Hosal Timer prescale definitions.
 */
#define HOSAL_TIMER_PRESCALE_1                  TIMER_PRESCALE_1
#define HOSAL_TIMER_PRESCALE_2                  TIMER_PRESCALE_2
#define HOSAL_TIMER_PRESCALE_8                  TIMER_PRESCALE_8
#define HOSAL_TIMER_PRESCALE_16                 TIMER_PRESCALE_16
#define HOSAL_TIMER_PRESCALE_32                 TIMER_PRESCALE_32
#define HOSAL_TIMER_PRESCALE_128                TIMER_PRESCALE_128
#define HOSAL_TIMER_PRESCALE_256                TIMER_PRESCALE_256
#define HOSAL_TIMER_PRESCALE_1024               TIMER_PRESCALE_1024

/**
 * \brief           Hosal Timer capture edge definitions.
 */
#define HOSAL_TIMER_CAPTURE_POS_EDGE            TIMER_CAPTURE_POS_EDGE
#define HOSAL_TIMER_CAPTURE_NEG_EDGE            TIMER_CAPTURE_NEG_EDGE

/**
 * \brief           Hosal Timer capture deglich definitions.
 */
#define HOSAL_TIMER_CAPTURE_DEGLICH_DISABLE     TIMER_CAPTURE_DEGLICH_DISABLE
#define HOSAL_TIMER_CAPTURE_DEGLICH_ENABLE      TIMER_CAPTURE_DEGLICH_ENABLE

/**
 * \brief           Hosal Timer capture interrupt definitions.
 */
#define HOSAL_TIMER_CAPTURE_INT_DISABLE         TIMER_CAPTURE_INT_DISABLE
#define HOSAL_TIMER_CAPTURE_INT_ENABLE          TIMER_CAPTURE_INT_ENABLE

/**
 * \brief           Hosal Timer clock source definitions.
 */
#define HOSAL_TIMER_CLOCK_SOURCE_PERI            TIMER_CLOCK_SOURCEC_PERI
#define HOSAL_TIMER_CLOCK_SOURCE_RCO1M           TIMER_CLOCK_SOURCEC_RCO1M
#define HOSAL_TIMER_CLOCK_SOURCE_PMU             TIMER_CLOCK_SOURCEC_PMU


/**
 * \brief           Hal timer config structure holding configuration settings 
 *                  for the timer.
 */
typedef struct {
    uint8_t  counting_mode : 1;                 /*!< Set counting mode */
    uint8_t  oneshot_mode  : 1;                 /*!< Enable one shot */
    uint8_t  mode          : 1;                 /*!< Set Freerun or Periodic mode */
    uint8_t  int_en        : 1;                 /*!< Enable Interrupt */
    uint8_t  prescale      : 3;                 /*!< Set prescale */
    uint16_t user_prescale : 10;                /*!< Set user define prescale */
} hosal_timer_config_t;

/**
 * \brief           Hal timer config structure holding configuration settings 
 *                  for the timer.
 */
typedef struct {
    uint32_t timeload_ticks;                    /*!< Timer reload tick */
    uint32_t timeout_ticks;                     /*!< Timer timeout tick */
} hosal_timer_tick_config_t;

/**
 * \brief           Hal timer capture config structure holding configuration settings
 *                  for the timer.
 */
typedef struct {
    uint8_t counting_mode      : 1;             /*!< Set counting mode */
    uint8_t oneshot_mode       : 1;             /*!< Enable one shot */
    uint8_t mode               : 1;             /*!< Set Freerun or Periodic mode */
    uint8_t int_en             : 1;             /*!< Enable Interrupt */
    uint8_t prescale           : 3;             /*!< Set prescale */
    uint16_t user_prescale     : 10;            /*!< Set user define prescale */
    uint8_t ch0_capture_edge   : 1;             /*!< Set Capture channel0 trigger edge */
    uint8_t ch0_deglich_enable : 1;             /*!< Enable Capture channel0 deglitch */
    uint8_t ch0_int_enable     : 1;             /*!< Enable Capture channel0 interrupt */
    uint8_t ch0_iosel          : 5;             /*!< Set Capture channel0 gpio */
    uint8_t ch1_capture_edge   : 1;             /*!< Set Capture channel1 trigger edge */
    uint8_t ch1_deglich_enable : 1;             /*!< Enable Capture channel1 deglitch */
    uint8_t ch1_int_enable     : 1;             /*!< Enable Capture channel1 interrupt */
    uint8_t ch1_iosel          : 5;             /*!< Set Capture channel1 gpio */
} hosal_timer_capture_config_mode_t;

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
} hosal_timer_pwm_config_mode_t;

/**
 * \brief           timer pwm config structure holding configuration settings 
 *                  for the timer
 */
typedef struct {
    uint32_t timeload_ticks;                    /*!< Timer pwm reload tick */
    uint32_t timeout_ticks;                     /*!< Timer pwm timeout tick */
    uint32_t threshold;                         /*!< Timer pwm threshold tick */
    uint32_t phase;                             /*!< Timer pwm phase */
} hosal_timer_pwm_config_tick_t;

/**
 * \brief           Hosal timer initialization
 * \param[in]       timer_id: timer id number
 * \param[in]       cfg: timer parameter setting
 * \param[in]       usr_call_back: user callback function
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t hosal_timer_init(uint32_t timer_id, hosal_timer_config_t cfg,
                          void* usr_call_back);

/**
 * \brief           Hosal timer start working
 * \param[in]       timer_id: timer id number
 * \param[in]       tick_cfg: timeout and load value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t hosal_timer_start(uint32_t timer_id,
                           hosal_timer_tick_config_t tick_cfg);

/**
 * \brief           Hosal stop timer
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t hosal_timer_stop(uint32_t timer_id);

/**
 * \brief           Hosal timer reload load value 
 * \param[in]       timer_id: timer id number
 * \param[in]       tick_cfg: timeout and load value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_reload(uint32_t timer_id,
                            hosal_timer_tick_config_t tick_cfg);

/**
 * \brief           Hosal close timer 
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_finalize(uint32_t timer_id);

/**
 * \brief           Hosal get timer current tick value
 * \param[in]       timer_id: timer id number
 * \param[out]      tick_value: Pointer to a user-provided variable 
 *                              where the timer current tick value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_current_get(uint32_t timer_id, uint32_t* tick_value);

/**
 * \brief           Hosal timer capture initialization
 * \param[in]       timer_id: timer id number
 * \param[in]       cfg: timer capture parameter setting
 * \param[in]       usr_call_back: user callback function
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t hosal_timer_capture_init(uint32_t timer_id,
                                  hosal_timer_capture_config_mode_t cfg,
                                  void* usr_call_back);

/**
 * \brief           Hosal timer capture start working
 * \param[in]       timer_id: timer id number
 * \param[in]       tick_cfg: timeout and load value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t hosal_timer_capture_start(uint32_t timer_id, uint32_t timeload_ticks,
                                   uint32_t timeout_ticks, bool chanel0_enable,
                                   bool chanel1_enable);

/**
 * \brief           Hosal stop timer capture
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t hosal_timer_capture_stop(uint32_t timer_id);

/**
 * \brief           Hosal close timer capture
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_capture_finalize(uint32_t timer_id);

/**
 * \brief           Get the timer capture channel 0 current value
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      ticl_value: Pointer to a user-provided variable 
 *                              where the timer capture channel 0 tick 
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_ch0_capture_value_get(uint32_t timer_id, uint32_t* ticl_value);

/**
 * \brief           Get the timer capture channel0 interrupt status
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      int_status: Pointer to a user-provided variable 
 *                              where the timer capture channel 0 interrupt status
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_ch0_capture_int_status(uint32_t timer_id, uint32_t* int_status);

/**
 * \brief           Get the timer capture channel 1 current value
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      ticl_value: Pointer to a user-provided variable 
 *                              where the timer capture channel 1 tick 
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_ch1_capture_value_get(uint32_t timer_id, uint32_t* ticl_value);

/**
 * \brief           Get the timer capture channel1 interrupt status
 * \param[in]       timer_id: Specifies timer id number
 * \param[out]      int_status: Pointer to a user-provided variable 
 *                              where the timer capture channel 1 interrupt status
 *                              value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_ch1_capture_int_status(uint32_t timer_id, uint32_t* int_status);

/**
 * \brief           Timer pwm parameter setting 
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       cfg: Timer pwm configuration
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t hosal_timer_pwm_open(uint32_t timer_id, hosal_timer_pwm_config_mode_t cfg);

/**
 * \brief           Start pwm timer
 * \param[in]       timer_id: Specifies timer id number
 * \param[in]       cfg: Timer pwm tick configuration
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, STATUS_NO_INIT
 */
uint32_t hosal_timer_pwm_start(uint32_t timer_id, hosal_timer_pwm_config_tick_t cfg);

/**
 * \brief           Stop pwm timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, STATUS_NO_INIT
 */
uint32_t hosal_timer_pwm_stop(uint32_t timer_id);

/**
 * \brief           Clear pwm timer
 * \param[in]       timer_id: Specifies timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_timer_pwm_close(uint32_t timer_id);


/*@}*/ /* end of RT584_HOSAL HOSAL_TIMER */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_TIMER_H */
