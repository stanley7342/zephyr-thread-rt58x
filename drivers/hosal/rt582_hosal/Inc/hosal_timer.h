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
 * \defgroup HOSAL_TIMER Hosal timer
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal timer definitions, structures, and functions
 * @{
 */

/**
 * \brief           Hosal Timer none use parameter definitions.
 */
#define HOSAL_TIMER_NONE_USE_PAR 0

/**
 * \brief           Hosal Timer counting mode definitions.
 */
#define HOSAL_TIMER_DOWN_COUNTING HOSAL_TIMER_NONE_USE_PAR
#define HOSAL_TIMER_UP_COUNTING   HOSAL_TIMER_NONE_USE_PAR

/**
 * \brief           Hosal Timer one-shot mode definitions.
 */
#define HOSAL_TIMER_ONE_SHOT_DISABLE HOSAL_TIMER_NONE_USE_PAR
#define HOSAL_TIMER_ONE_SHOT_ENABLE  HOSAL_TIMER_NONE_USE_PAR

/**
 * \brief           Hosal Timer mode definitions.
 */
#define HOSAL_TIMER_FREERUN_MODE  TIMER_FREERUN_MODE
#define HOSAL_TIMER_PERIODIC_MODE TIMER_PERIODIC_MODE

/**
 * \brief           Hosal Timer interrupt definitions.
 */
#define HOSAL_TIMER_INT_DISABLE 0
#define HOSAL_TIMER_INT_ENABLE  1

/**
 * \brief           Hosal Timer prescale definitions.
 */
#define HOSAL_TIMER_PRESCALE_1    TIMER_PRESCALE_1
#define HOSAL_TIMER_PRESCALE_2    TIMER_PRESCALE_2
#define HOSAL_TIMER_PRESCALE_8    TIMER_PRESCALE_8
#define HOSAL_TIMER_PRESCALE_16   TIMER_PRESCALE_16
#define HOSAL_TIMER_PRESCALE_32   TIMER_PRESCALE_32
#define HOSAL_TIMER_PRESCALE_128  TIMER_PRESCALE_128
#define HOSAL_TIMER_PRESCALE_256  TIMER_PRESCALE_256
#define HOSAL_TIMER_PRESCALE_1024 TIMER_PRESCALE_1024

/**
 * \brief           Hosal Timer capture edge definitions.
 */
#define HOSAL_TIMER_CAPTURE_POS_EDGE HOSAL_TIMER_NONE_USE_PAR
#define HOSAL_TIMER_CAPTURE_NEG_EDGE HOSAL_TIMER_NONE_USE_PAR

/**
 * \brief           Hosal Timer capture deglich definitions.
 */
#define HOSAL_TIMER_CAPTURE_DEGLICH_DISABLE HOSAL_TIMER_NONE_USE_PAR
#define HOSAL_TIMER_CAPTURE_DEGLICH_ENABLE  HOSAL_TIMER_NONE_USE_PAR

/**
 * \brief           Hosal Timer capture interrupt definitions.
 */
#define HOSAL_TIMER_CAPTURE_INT_DISABLE HOSAL_TIMER_NONE_USE_PAR
#define HOSAL_TIMER_CAPTURE_INT_ENABLE  HOSAL_TIMER_NONE_USE_PAR

/**
 * \brief           Hosal Timer clock source definitions.
 */
#define HOSAL_TIMER_CLOCK_SOURCE_PERI  HOSAL_TIMER_NONE_USE_PAR
#define HOSAL_TIMER_CLOCK_SOURCE_RCO1M HOSAL_TIMER_NONE_USE_PAR
#define HOSAL_TIMER_CLOCK_SOURCE_PMU   HOSAL_TIMER_NONE_USE_PAR

/**
 * \brief           Hal timer config structure holding configuration settings 
 *                  for the timer.
 */
typedef struct {
    uint8_t counting_mode   : 1;                /*!< Set counting mode */
    uint8_t oneshot_mode    : 1;                /*!< Enable one shot */
    uint8_t mode            : 1;                /*!< Set Freerun or Periodic mode */
    uint8_t int_en          : 1;                /*!< Enable Interrupt */
    uint8_t prescale        : 3;                /*!< Set prescale */
    uint16_t user_prescale  : 10;               /*!< Set user define prescale */
} hosal_timer_config_t;

/**
 * \brief           Hal timer config structure holding configuration settings 
 *                  for the timer.
 */
typedef struct {
    uint32_t timeload_ticks; /*!< Timer reload tick */
    uint32_t timeout_ticks;  /*!< Timer timeout tick */
} hosal_timer_tick_config_t;

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
uint32_t hosal_timer_start(uint32_t timer_id, hosal_timer_tick_config_t tick_cfg);

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
uint32_t hosal_timer_reload(uint32_t timer_id, hosal_timer_tick_config_t tick_cfg);

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

/*@}*/ /* end of RT58X_HOSAL HOSAL_TIMER */

#ifdef __cplusplus
}
#endif

#endif /* HOSAL_TIMER_H */
