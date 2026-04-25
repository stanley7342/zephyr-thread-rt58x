/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_slow_timer.h
 * \brief           Hosal Slow Timer driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_SLOW_TIMER_H
#define HOSAL_SLOW_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "timer.h"



/**
 * \defgroup        HOSAL_AUX_COMP Hosal aux comp
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal aux comp definitions, structures, and functions
 * @{
 */


/**
 * \brief           Slow timer counting mode definitions.
 */
#define HOSAL_SLOW_TIMER_DOWN_COUNTING TIMER_DOWN_COUNTING
#define HOSAL_SLOW_TIMER_UP_COUNTING   TIMER_UP_COUNTING

/**
 * \brief           Slow timer one-shot mode definitions.
 */
#define HOSAL_SLOW_TIMER_ONE_SHOT_DISABLE TIMER_ONE_SHOT_DISABLE
#define HOSAL_SLOW_TIMER_ONE_SHOT_ENABLE  TIMER_ONE_SHOT_ENABLE

/**
 * \brief           Slow timer mode definitions.
 */
#define HOSAL_SLOW_TIMER_FREERUN_MODE  TIMER_FREERUN_MODE
#define HOSAL_SLOW_TIMER_PERIODIC_MODE TIMER_PERIODIC_MODE

/**
 * \brief           Slow timer interrupt definitions.
 */
#define HOSAL_SLOW_TIMER_INT_DISABLE TIMER_INT_DISABLE
#define HOSAL_SLOW_TIMER_INT_ENABLE  TIMER_INT_ENABLE

/**
 * \brief           Slow timer prescale definitions.
 */
#define HOSAL_SLOW_TIMER_PRESCALE_1    TIMER_PRESCALE_1
#define HOSAL_SLOW_TIMER_PRESCALE_2    TIMER_PRESCALE_2
#define HOSAL_SLOW_TIMER_PRESCALE_8    TIMER_PRESCALE_8
#define HOSAL_SLOW_TIMER_PRESCALE_16   TIMER_PRESCALE_16
#define HOSAL_SLOW_TIMER_PRESCALE_32   TIMER_PRESCALE_32
#define HOSAL_SLOW_TIMER_PRESCALE_128  TIMER_PRESCALE_128
#define HOSAL_SLOW_TIMER_PRESCALE_256  TIMER_PRESCALE_256
#define HOSAL_SLOW_TIMER_PRESCALE_1024 TIMER_PRESCALE_1024

/**
 * \brief Hal slow timer config structure holding configuration settings 
 *        for the timer.
 */
typedef struct {
    uint8_t  counting_mode : 1;                 /*!< Set counting mode */
    uint8_t  one_shot_mode : 1;                 /*!< Enable one shot */
    uint8_t  mode          : 1;                 /*!< Set Freerun or Periodic mode */
    uint8_t  int_enable    : 1;                 /*!< Enable Interrupt */
    uint8_t  prescale      : 3;                 /*!< Set prescale */
    uint16_t user_prescale : 10;                /*!< Set user define prescale */
    uint16_t repeat_delay;                      /*!< Set how mant times to delay 
                                                    trigger interrupt */
} hosal_slow_timer_config_t;

/**
 * \brief Hal slow timer config structure holding configuration settings 
 *        for the timer.
 */
typedef struct {
	uint32_t timeload_ticks;                    /*!< Timer reload tick */
	uint32_t timeout_ticks;                     /*!< Timer timeout tick */
} hosal_slow_timer_tick_config_t;


/**
 * \brief           Hosal slow timer initialization
 * \param[in]       timer_id: timer id number
 * \param[in]       cfg: timeout value
 * \param[in]       usr_call_back: timeout value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t hosal_slow_timer_init(uint32_t timer_id, hosal_slow_timer_config_t cfg, 
                               void* usr_call_back);

/**
 * \brief           Hosal slow timer start working
 * \param[in]       timer_id: timer id number
 * \param[in]       tick_cfg: timeout and load value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t hosal_slow_timer_start(uint32_t timer_id, 
                                hosal_slow_timer_tick_config_t tick_cfg);

/**
 * \brief           Hosal stop slow timer
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_NO_INIT
 */
uint32_t hosal_slow_timer_stop(uint32_t timer_id);

/**
 * \brief           Hosal slow timer reload load value 
 * \param[in]       timer_id: timer id number
 * \param[in]       tick_cfg: timeout and load value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_slow_timer_reload(uint32_t timer_id, 
                                 hosal_slow_timer_tick_config_t tick_cfg);

/**
 * \brief           Hosal slow timer clear interrupt 
 * \param[in]       timer_id: timer id number
 * \param[in]       tick_cfg: timeout and load value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_slow_timer_clear_int(uint32_t timer_id);

/**
 * \brief           Hosal close slow timer 
 * \param[in]       timer_id: timer id number
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_slow_timer_finalize(uint32_t timer_id);

/**
 * \brief           Hosal get slow timer current tick value
 * \param[in]       timer_id: timer id number
 * \param[out]      tick_value: Pointer to a user-provided variable 
 *                              where the timer current tick value will be stored.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_slow_timer_current_get(uint32_t timer_id, uint32_t* tick_value);

/*@}*/ /* end of RT584_HOSAL HOSAL_SLOW_TIMER */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HOSAL_SLOW_TIMER_H */

