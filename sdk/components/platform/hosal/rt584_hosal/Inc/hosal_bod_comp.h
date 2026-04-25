/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           hosal_bod_comp.h
 * \brief          hosal bod comparator driver header file
 */
/*
 * This file is part of library_name.
 * Author:          Kc.tseng
 */

#ifndef HOSAL_BOD_COMP_H
#define HOSAL_BOD_COMP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "bod_comp.h"



/**
 * \defgroup        HOSAL_BOD_COMP Hosal bod comp
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal bod comp definitions, structures, and functions
 * @{
 */

/**
 * \brief           Debounce time definitions.
 */
#define HOSAL_BOD_SLOW_CLOKC_2  BOD_SLOW_CLOKC_2    /*!< 2 slow clock debounce time */
#define HOSAL_BOD_SLOW_CLOKC_4  BOD_SLOW_CLOKC_4    /*!< 4 slow clock debounce time */
#define HOSAL_BOD_SLOW_CLOKC_8  BOD_SLOW_CLOKC_8    /*!< 8 slow clock debounce time */

/**
 * \brief           Counter mode edge definitions.
 */

#define HOSAL_BOD_RISING_EDGE   BOD_RISING_EDGE     /*!< counter trigger when rising edge */
#define HOSAL_BOD_FALLING_EDGE  BOD_FALLING_EDGE    /*!< counter trigger when falling edge */
#define HOSAL_BOD_BOTH_EDGE     BOD_BOTH_EDGE       /*!< counter trigger when both edge */


/**
 * \brief           Counter mode edge definitions.
 */
#define HOSAL_BOD_LOW_LEVEL     BOD_LOW_LEVEL   /*!< bod comparator without clock to wakeup at low level when deepsleep */
#define HOSAL_BOD_HIGH_LEVEL    BOD_HIGH_LEVEL  /*!< bod comparator without clock to wakeup at high level when deepsleep */



/**
 * \brief           bod comparator config structure.
 */
typedef struct {
    uint8_t debounce_en             : 1;        /*!< set debounce enable */
    uint8_t debounce_sel            : 2;        /*!< debounce time select */
    uint8_t counter_mode_en         : 1;        /*!< set counter mode enable */
    uint8_t counter_mode_edge       : 2;        /*!< set the trigger polarity of counter mode */
    uint8_t counter_mode_int_en     : 1;        /*!< counter mode interrupt enable */
    uint8_t rising_edge_int_en      : 1;        /*!< set rising edge interrupt enable */
    uint8_t falling_edge_int_en     : 1;        /*!< set falling edge interrupt enable */
    uint16_t counter_mode_threshold;            /*!< set the trigger threshold of the counter mode. 
                                                 When COUNTER_CNT > counter_mode_threshold , Interrupt will be triggered. */
    uint16_t voltage_threshold;                 /* voltage threshold value */
} hosal_bod_comp_config_t;

/**
 * \brief           Setting Bod comparator configuration.
 * \param[in]       bod_cfg: Bod comparator configuration.
 * \param[in]       bod_comp_callback: Specifies user callback function when the 
 *                                     bod comparator interrupt generated.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_open(hosal_bod_comp_config_t bod_cfg, void* bod_comp_callback);

/**
 * \brief           Enable Bod comparator.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_normal_start(void);

/**
 * \brief           Disable Bod comparator.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_normal_stop(void);

/**
 * \brief           Enable Bod comparator in sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_sleep_start(void);

/**
 * \brief           Disable Bod comparator in sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_sleep_stop(void);

/**
 * \brief           Enable Bod comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_deep_sleep_start(void);

/**
 * \brief           Disable Bod comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_deep_sleep_stop(void);

/**
 * \brief           Enable clock source when Bod comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_setup_deep_sleep_enable_clock(void);

/**
 * \brief           Disable clock source when Bod comparator in deep sleep mode.
 * \param[in]       wakeup_level: Set wakeup polarity low or high in deepsleep without clock source.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_bod_comp_setup_deep_sleep_disable_clock(uint8_t wakeup_level);

/**
 * \brief           Get Bod comparator count.
 * \param[out]      count: Pointer to a user-provided variable where the 
 *                         bod comp counter count value will be stored.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_get_bod_comp_counter_count(uint32_t *count);

/**
 * \brief           Clear Bod comparator count.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_clear_bod_comp_counter_count(void);

/*@}*/ /* end of RT584_HOSAL HOSAL_BOD_COMP */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_BOD_COMP_H */
