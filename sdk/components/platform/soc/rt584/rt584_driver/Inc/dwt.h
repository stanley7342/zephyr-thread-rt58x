/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            dwt.h
 * \brief           dwt header file
 */

/*
 * This file is part of library_name.
 * Author:     
 */

#ifndef ___DWT_H__
#define ___DWT_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ---  --- */
/* Environment Detection for FreeRTOS */
#if defined(CONFIG_FREERTOS)
    #include "FreeRTOS.h"
    #include "task.h"
    #define _IS_RTOS_ACTIVE() (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
#else
    #define _IS_RTOS_ACTIVE() (false)
#endif

/* Timer structure for tracking elapsed time */
/* --- Callback Type Definition --- */
typedef void (*TimeoutCallback_t)(void *arg);

/* --- Timer Structure --- */
typedef struct {
    uint32_t          start_time;        /* Initial tick or SysTick value */
    uint32_t          timeout_ms;        /* Target duration in ms */
    bool              mode_at_start;     /* Track if started in RTOS mode */
    TimeoutCallback_t callback;          /* Function to call on timeout */
    void             *callback_arg;      /* Argument for the callback */
    bool              callback_executed; /* Safeguard for single execution */
    bool              auto_reload;       // true: repeat, false: one time
} TimeoutTimer;


/**
 * \brief  Initializes the hardware timer resource.
 * \return STATUS_SUCCESS
 */
uint32_t timeout_init(void);

/**
 * \brief  De-initializes the timer resource and releases hardware if unused.
 * \return STATUS_SUCCESS
 */
uint32_t timeout_uninit(void);


/**
 * \brief  Starts/Resets a timeout monitoring object.
 * \param  timer: Pointer to the timer structure.
 * \param  ms: Duration to wait in milliseconds.
 * \return STATUS_SUCCESS or STATUS_ERROR
 */
uint32_t timeout_start(TimeoutTimer *timer, uint32_t ms, TimeoutCallback_t cb, void *arg, bool periodic);

/**
 * \brief  Checks if the timer has reached the specified timeout.
 * \param  timer: Pointer to the timer structure.
 * \return STATUS_TIMEOUT: Time elapsed, STATUS_SUCCESS: Still within time.
 */
uint32_t istimeout(TimeoutTimer *timer);
/**
 * \brief  Calculates the remaining time before the timeout occurs.
 * \param  timer: Pointer to the timer structure.
 * \param  remaining_ms: Pointer to store the calculated remaining milliseconds.
 * \return STATUS_SUCCESS: Calculation successful, STATUS_TIMEOUT: Already expired.
 */
uint32_t timeout_remaining(TimeoutTimer *timer, uint32_t *remaining_ms);
/**
 * \brief  Stop timer
 * \param  timer: Pointer to the timer structure.
 * \return STATUS_SUCCESS: Calculation successful.
 */
uint32_t timeout_stop(TimeoutTimer *timer);
/*@}*/ /* end of RT584_DRIVER DWT */

#ifdef __cplusplus
}
#endif

#endif /* __DWT_H__ */


