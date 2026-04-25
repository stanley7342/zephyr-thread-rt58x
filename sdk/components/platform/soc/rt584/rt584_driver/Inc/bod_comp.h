/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           bod_comp.h
 * \brief          BOD Comparator driver header file
 */
/*
 * This file is part of library_name.
 * Author:          Kc.tseng
 */

#ifndef BOD_COMP_H
#define BOD_COMP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "mcu.h"


/**
 * \defgroup BOD_COMP Bod comp
 * \ingroup RT584_DRIVER
 * \brief  Define Bod comp definitions, structures, and functions
 * @{
 */


/**
 * \brief           Bod comparator interrupt service routine callback for user application.
 * \param[in]       status  status passed to user bod routine for the reason
 */
typedef void (* bod_comp_proc_cb)(uint32_t status);

/**
 * \brief           Debounce time definitions.
 */
typedef enum {
    BOD_SLOW_CLOKC_2,                           /*!< 2 slow clock debounce time */
    BOD_SLOW_CLOKC_4,                           /*!< 4 slow clock debounce time */
    BOD_SLOW_CLOKC_8                            /*!< 8 slow clock debounce time */
} bod_comp_debounce_time_t;

/**
 * \brief           Counter mode edge definitions.
 */
typedef enum {
    BOD_RISING_EDGE,                            /*!< counter trigger when rising edge */
    BOD_FALLING_EDGE,                           /*!< counter trigger when falling edge */
    BOD_BOTH_EDGE                               /*!< counter trigger when both edge */
} bod_comp_counter_mode_edge_t;

/**
 * \brief           Counter mode edge definitions.
 */
typedef enum {
    BOD_LOW_LEVEL,                              /*!< bod comparator without clock to wakeup at low level when deepsleep */
    BOD_HIGH_LEVEL                              /*!< bod comparator without clock to wakeup at high level when deepsleep */
} bod_comp_wakeup_deepslepp_level_t;


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
                                                 When COUNTER_CNT > counter_mode_threshold ,
                                                 Interrupt will be triggered. */
    uint16_t voltage_step;                      /* setup comp_vsel value */
} bod_comp_config_t;



/**
 * \brief           Register user interrupt ISR callback function.
 * \param[in]       bod_comp_callback: Specifies user callback function when the 
 *                                     bod comparator interrupt generated.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_register_callback(bod_comp_proc_cb bod_comp_callback);

/**
 * \brief           Calibration comp_vsel voltage.
 * \param[in]       voltage: expect voltage.
 * \param[out]      voltage_step: 1.8V mapping comp_vsel step value.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_voltage_threshold_calibration(uint16_t voltage, uint32_t *voltage_step);

/**
 * \brief           Init Bod comparator Aanlog setting.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_ana_init(uint32_t voltage_step);
    
/**
 * \brief           Setting Bod comparator configuration.
 * \param[in]       bod_cfg: Bod comparator configuration.
 * \param[in]       bod_comp_callback: Specifies user callback function when the 
 *                                     bod comparator interrupt generated.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_open(bod_comp_config_t bod_cfg, bod_comp_proc_cb bod_comp_callback);

/**
 * \brief           Enable Bod comparator .
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_normal_start(void);

/**
 * \brief           Disable Bod comparator .
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_normal_stop(void);

/**
 * \brief           Enable Bod comparator in sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_sleep_start(void);

/**
 * \brief           Disable Bod comparator in sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_sleep_stop(void);

/**
 * \brief           Enable Bod comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_deep_sleep_start(void);

/**
 * \brief           Disable Bod comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_deep_sleep_stop(void);

/**
 * \brief           Enable clock source when Bod comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_setup_deep_sleep_enable_clock(void);

/**
 * \brief           Disable clock source when Bod comparator in deep sleep mode.
 * \param[in]       wakeup_level: Set wakeup polarity low or high in deepsleep without clock source.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t bod_comp_setup_deep_sleep_disable_clock(uint8_t wakeup_level);

/**
 * \brief           Get Bod comparator count.
 * \param[out]      count: Pointer to a user-provided variable where the 
 *                         bod comp counter count value will be stored.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t get_bod_comp_counter_count(uint32_t *count);

/**
 * \brief           Clear Bod comparator count.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t clear_bod_comp_counter_count(void);

/*@}*/ /* end of RT584_DRIVER BOD_COMP */

#ifdef __cplusplus
}
#endif

#endif /* End of BOD_COMP_H */
