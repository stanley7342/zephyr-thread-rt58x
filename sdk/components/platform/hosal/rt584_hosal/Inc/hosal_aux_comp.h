/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_aux_comp.h
 * \brief           hosal aux comparator driver header file
 */
/*
 * This file is part of library_name.
 * Author:          Kc.tseng
 */
#ifndef HOSAL_AUX_COMP_H
#define HOSAL_AUX_COMP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "aux_comp.h"



/**
 * \defgroup        HOSAL_AUX_COMP Hosal aux comp
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal aux comp definitions, structures, and functions
 * @{
 */

/**
 * \brief           Debounce time definitions.
 */
#define HOSAL_AUX_SLOW_CLOKC_2  AUX_SLOW_CLOKC_2
#define HOSAL_AUX_SLOW_CLOKC_4  AUX_SLOW_CLOKC_4
#define HOSAL_AUX_SLOW_CLOKC_8  AUX_SLOW_CLOKC_8

/**
 * \brief           Counter mode edge definitions.
 */
#define HOSAL_AUX_RISING_EDGE   AUX_RISING_EDGE
#define HOSAL_AUX_FALLING_EDGE  AUX_FALLING_EDGE
#define HOSAL_AUX_BOTH_EDGE     AUX_BOTH_EDGE

/**
 * \brief           Counter mode edge definitions.
 */
#define HOSAL_AUX_LOW_LEVEL     AUX_LOW_LEVEL
#define HOSAL_AUX_HIGH_LEVEL    AUX_HIGH_LEVEL

/**
 * \brief           Aux comparator config structure.
 */
typedef struct {
    uint8_t  debounce_en             : 1;       /*!< set debounce enable */
    uint8_t  debounce_sel            : 2;       /*!< debounce time select */
    uint8_t  counter_mode_en         : 1;       /*!< set counter mode enable */
    uint8_t  counter_mode_edge       : 2;       /*!< set the trigger polarity of counter mode */
    uint8_t  counter_mode_int_en     : 1;       /*!< counter mode interrupt enable */
    uint8_t  rising_edge_int_en      : 1;       /*!< set rising edge interrupt enable */
    uint8_t  falling_edge_int_en     : 1;       /*!< set falling edge interrupt enable */
    uint16_t counter_mode_threshold;            /*!< set the trigger threshold of the counter mode. 
                                                 When COUNTER_CNT > counter_mode_threshold , Interrupt will be triggered. */
    uint16_t voltage_threshold;                 /* voltage threshold value */
} hosal_aux_comp_config_t;


/**
 * \brief           Setting Aux comparator configuration.
 * \param[in]       hosal_aux_cfg: Aux comparator configuration.
 * \param[in]       hosal_aux_comp_callback: Specifies user callback function when the aux comparator interrupt generated.
 */
uint32_t hosal_aux_comp_open(hosal_aux_comp_config_t hosal_aux_cfg, void* hosal_aux_comp_callback);

/**
 * \brief           Enable Aux comparator .
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_normal_start(void);

/**
 * \brief           Disable Aux comparator .
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_normal_stop(void);

/**
 * \brief           Enable Aux comparator in sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_sleep_start(void);

/**
 * \brief           Disable Aux comparator in sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_sleep_stop(void);

/**
 * \brief           Enable Aux comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_deep_sleep_start(void);

/**
 * \brief           Disable Aux comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_deep_sleep_stop(void);

/**
 * \brief           Enable clock source when Aux comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_setup_deep_sleep_enable_clock(void);

/**
 * \brief           Disable clock source when Aux comparator in deep sleep mode.
 * \param[in]       wakeup_level: Set wakeup polarity low or high in deepsleep without clock source.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_aux_comp_setup_deep_sleep_disable_clock(uint8_t wakeup_level);

/**
 * \brief           Get Aux comparator count.
 * \param[out]      count: Pointer to a user-provided variable where the 
 *                         aux comp counter count value will be stored.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_get_aux_comp_counter_count(uint32_t *count);

/**
 * \brief           Clear Aux comparator count.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_clear_aux_comp_counter_count(void);

/*@}*/ /* end of RT584_HOSAL HOSAL_AUX_COMP */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_AUX_COMP_H */
