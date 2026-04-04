/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_wdt.h
 * \brief           Hosal Watch Dog timer header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_WDT_H
#define HOSAL_WDT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "wdt.h"


/**
 * \defgroup HOSAL_WDT Hosal wdt
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal wdt definitions, structures, and functions
 * @{
 */


/**
 * \brief           Gosal WDT prescale definitions.
 */
#define HOSAL_WDT_PRESCALE_1    WDT_PRESCALE_1
#define HOSAL_WDT_PRESCALE_16   WDT_PRESCALE_16
#define HOSAL_WDT_PRESCALE_32   WDT_PRESCALE_32
#define HOSAL_WDT_PRESCALE_128  WDT_PRESCALE_128
#define HOSAL_WDT_PRESCALE_256  WDT_PRESCALE_256
#define HOSAL_WDT_PRESCALE_1024 WDT_PRESCALE_1024
#define HOSAL_WDT_PRESCALE_4096 WDT_PRESCALE_4096

/**
 * \brief           Hal wdt config structure.
 */
typedef struct {
    uint8_t int_enable   : 1;  /*!< config of interrupt enable */
    uint8_t reset_enable : 1;  /*!< config of reset enable */
    uint8_t lock_enable  : 1;  /*!< config of lockout enable */
    uint16_t prescale    : 12; /*!< config of prescale */
} hosal_wdt_config_mode_t;

/**
 * \brief           Hal wdt ticks config structure.
 */
typedef struct {
    uint32_t wdt_ticks;     /*!< config of load value */
    uint32_t int_ticks;     /*!< config of interrupt value */
    uint32_t wdt_min_ticks; /*!< config of wdt min value */
} hosal_wdt_config_tick_t;

/**
 * \brief           Hosal wdt initialization
 * \param[in]       wdt_mode: wdt parameter setting
 * \param[in]       wdt_cfg_ticks: timeout value
 * \param[in]       wdt_cb: timeout value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
uint32_t hosal_wdt_start(hosal_wdt_config_mode_t wdt_mode,
                         hosal_wdt_config_tick_t wdt_cfg_ticks,
                         void* usr_call_back);

/**
 * \brief           Hosal WDT stop
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_REQUEST
 */
uint32_t hosal_wdt_stop(void);

/**
 * \brief           Get WDT number of resets
 * \param[out]      reset_cnt: Pointer to a user-provided variable where 
 *                             the WDT reset event will be stored.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_wdt_reset_event_get(uint32_t* reset_cnt);

/**
 * \brief           Clear WDT number of resets
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_wdt_reset_event_clear(void);

/**
 * \brief           Reload the watchdog value
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_wdt_kick(void);

/**
 * \brief           Clear Watchdog interrupt flag
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_wdt_interrupt_clear(void);

/**
 * \brief           Get watchdog timer current tick value
 * \param[out]      tick_value: Pointer to a user-provided variable 
 *                              where the WDT value will be stored.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_wdt_current_get(uint32_t* tick_value);

/*@}*/ /* end of RT58X_HOSAL HOSAL_WDT */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_WDT_H */
