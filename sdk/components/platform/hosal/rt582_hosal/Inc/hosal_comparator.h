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
 * Author:          Ives.lee
 */
#ifndef HOSAL_AUX_COMP_H
#define HOSAL_AUX_COMP_H

#include "comparator.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup HOSAL_COMPARATOR Hosal comparator
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal comparator definitions, structures, and functions
 * @{
 */

/**
 * \brief           Comparator Input N source internal/external enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_SELREF_INTERNAL = COMP_CONFIG_SELREF_INTERNAL,   /*!< Input N source internal bg_0.6v. */
    HOSAL_COMP_CONFIG_SELREF_EXTERNAL = COMP_CONFIG_SELREF_EXTERNAL,   /*!< Input N source external. */
} hosal_comp_config_selref_t;

/**
 * \brief           Comparator Input P source internal/external enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_SELINPUT_INTERNAL = COMP_CONFIG_SELINPUT_INTERNAL,    /*!< Input P source internal vdd div. */
    HSOAL_COMP_CONFIG_SELINPUT_EXTERNAL = COMP_CONFIG_SELINPUT_EXTERNAL,    /*!< Input P source external. */
} hosal_comp_config_selinput_t;

/**
 * \brief           Comparator N input channel enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_0 = COMP_CONFIG_REFSEL_CHANNEL_0,      /*!< Select Comparator N input channel 0. */
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_1 = COMP_CONFIG_REFSEL_CHANNEL_1,      /*!< Select Comparator N input channel 1. */
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_2 = COMP_CONFIG_REFSEL_CHANNEL_2,      /*!< Select Comparator N input channel 2. */
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_3 = COMP_CONFIG_REFSEL_CHANNEL_3,      /*!< Select Comparator N input channel 3. */
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_4 = COMP_CONFIG_REFSEL_CHANNEL_4,      /*!< Select Comparator N input channel 4. */
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_5 = COMP_CONFIG_REFSEL_CHANNEL_5,      /*!< Select Comparator N input channel 5. */
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_6 = COMP_CONFIG_REFSEL_CHANNEL_6,      /*!< Select Comparator N input channel 6. */
    HOSAL_COMP_CONFIG_REFSEL_CHANNEL_7 = COMP_CONFIG_REFSEL_CHANNEL_7,      /*!< Select Comparator N input channel 7. */
    HOSAL_COMP_CONFIG_REFSEL_IOLDO     = COMP_CONFIG_REFSEL_IOLDO,          /*!< Select Comparator N input IOLDO. */
    HOSAL_COMP_CONFIG_REFSEL_AVSS      = COMP_CONFIG_REFSEL_AVSS,           /*!< Select Comparator N input AVSS. */
} hosal_comp_config_ref_sel_t;

/**
 * \brief           Comparator current select enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_PW_SMALLEST = COMP_CONFIG_PW_SMALLEST,    /*!< Comparator current select smallest. */
    HOSAL_COMP_CONFIG_PW_LARGEST  = COMP_CONFIG_PW_LARGEST,     /*!< Comparator current select largest. */
} hosal_comp_config_pw_t;

/**
 * \brief           Comparator hys window select enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_HYS_40  = COMP_CONFIG_HYS_40,     /*!< Comparator hys window select 40mV. */
    HOSAL_COMP_CONFIG_HYS_80  = COMP_CONFIG_HYS_80,     /*!< Comparator hys window select 80mV. */
    HOSAL_COMP_CONFIG_HYS_100 = COMP_CONFIG_HYS_100,    /*!< Comparator hys window select 100mV. */
    HOSAL_COMP_CONFIG_HYS_130 = COMP_CONFIG_HYS_130,    /*!< Comparator hys window select 130mV. */
} hosal_comp_config_hys_sel_t;

/**
 * \brief           Comparator switch vdd div enum struct.
 */
typedef enum {
    HOSAL_COMP_CONFIG_SWDIV_MOS = COMP_CONFIG_SWDIV_MOS,    /*!< Comparator switch vdd MOS div. */
    HOSAL_COMP_CONFIG_SWDIV_RES = COMP_CONFIG_SWDIV_RES,    /*!< Comparator switch vdd RES div. */
} hosal_comp_config_swdiv_t;/**
 * \brief           Comparator waking up in deepsleep .
 */
typedef enum {
    HOSAL_COMP_CONFIG_DS_WAKEUP_DISABLE = COMP_CONFIG_DS_WAKEUP_DISABLE,    /*!< Disable the Comparator wakeup in DeepSleep. */
    HOSAL_COMP_CONFIG_DS_WAKEUP_ENABLE  = COMP_CONFIG_DS_WAKEUP_ENABLE,     /*!< Enable the Comparator wakeup in DeepSleep. */
} hosal_comp_config_ds_wakeup_t;

/**
 * \brief           Comparator output invert for waking up form deepsleep .
 */
typedef enum {
    HOSAL_COMP_CONFIG_DS_INVERT_DISABLE      = COMP_CONFIG_DS_INVERT_DISABLE,   /*!< Disable the Comparator output invert for waking up from DeepSleep. */
    HOSAL_COMP_CONFIG_DS_INVERT_ENABLE       = COMP_CONFIG_DS_INVERT_ENABLE,    /*!< Enable the Comparator output invert for waking up from DeepSleep. */
} hosal_comp_config_ds_inv_t;

/**
 * \brief           Comparator PSRR definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_PSRR_REGULAR = COMP_CONFIG_PSRR_REGULAR,  /*!< Comparator PSRR regular. */
    HOSAL_COMP_CONFIG_PSRR_HIGH_V  = COMP_CONFIG_PSRR_HIGH_V,   /*!< Comparator PSRR high V. */
} hosal_comp_config_psrr_t;

/**
 * \brief           Comparator temperature coefficient enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_TC_REGULAR    = COMP_CONFIG_TC_REGULAR,       /*!< Comparator temperature coefficient regular. */
    HOSAL_COMP_CONFIG_TC_COMPENSATE = COMP_CONFIG_TC_COMPENSATE,    /*!< Comparator temperature coefficient compensate. */
} hosal_comp_config_tc_t;

/**
 * \brief           Comparator enable start enum definitions.
 */
typedef enum {
     HOSAL_COMP_CONFIG_EN_START_0 = COMP_CONFIG_EN_START_0,
     HOSAL_COMP_CONFIG_EN_START_1 = COMP_CONFIG_EN_START_1,
     HOSAL_COMP_CONFIG_EN_START_2 = COMP_CONFIG_EN_START_2,
     HOSAL_COMP_CONFIG_EN_START_3 = COMP_CONFIG_EN_START_3,
} hosal_comp_config_en_start_t;

/**
 * \brief           Comparator interrupt enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_INT_POL_RISING  = COMP_CONFIG_INT_POL_RISING,     /*!< Rising edge. */
    HOSAL_COMP_CONFIG_INT_POL_FALLING = COMP_CONFIG_INT_POL_FALLING,    /*!< Falling edge. */
    HOSAL_COMP_CONFIG_INT_POL_BOTH    = COMP_CONFIG_INT_POL_BOTH,       /*!< Both edge. */
} hosal_comp_config_int_pol_t;

/**
 * \brief           Select Comparator P input channel definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_0   = COMP_CONFIG_CHSEL_CHANNEL_0,      /*!< Select Comparator P input channel 0. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_1   = COMP_CONFIG_CHSEL_CHANNEL_1,      /*!< Select Comparator P input channel 1. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_2   = COMP_CONFIG_CHSEL_CHANNEL_2,      /*!< Select Comparator P input channel 2. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_3   = COMP_CONFIG_CHSEL_CHANNEL_3,      /*!< Select Comparator P input channel 3. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_4   = COMP_CONFIG_CHSEL_CHANNEL_4,      /*!< Select Comparator P input channel 4. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_5   = COMP_CONFIG_CHSEL_CHANNEL_5,      /*!< Select Comparator P input channel 5. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_6   = COMP_CONFIG_CHSEL_CHANNEL_6,      /*!< Select Comparator P input channel 6. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_7   = COMP_CONFIG_CHSEL_CHANNEL_7,      /*!< Select Comparator P input channel 7. */
    HOSAL_COMP_CONFIG_CHSEL_AVDD_1V     = COMP_CONFIG_CHSEL_AVDD_1V,        /*!< Select Comparator P input AVDD 1V. */
    HOSAL_COMP_CONFIG_CHSEL_AVSS        = COMP_CONFIG_CHSEL_AVSS,           /*!< Select Comparator P input AVSS. */
    HOSAL_COMP_CONFIG_CHSEL_CHANNEL_MAX = COMP_CONFIG_CHSEL_CHANNEL_MAX,    /*!< Max Channel 10. */
} hosal_comp_config_ch_sel_t;

/**
 * \brief           Comparator internal vdd div voltage enum definitions.
 */
typedef enum {
    HOSAL_COMP_CONFIG_VOLTAGE_0   = COMP_CONFIG_VOLTAGE_0,      /*!< Select the Comparator internal vdd div voltage: Power Fail 1.78V. */
    HOSAL_COMP_CONFIG_VOLTAGE_1   = COMP_CONFIG_VOLTAGE_1,      /*!< Select the Comparator internal vdd div voltage: Power Fail 1.82V. */
    HOSAL_COMP_CONFIG_VOLTAGE_2   = COMP_CONFIG_VOLTAGE_2,      /*!< Select the Comparator internal vdd div voltage: Power Fail 1.85V. */
    HOSAL_COMP_CONFIG_VOLTAGE_3   = COMP_CONFIG_VOLTAGE_3,      /*!< Select the Comparator internal vdd div voltage: Power Fail 1.89V. */
    HOSAL_COMP_CONFIG_VOLTAGE_4   = COMP_CONFIG_VOLTAGE_4,      /*!< Select the Comparator internal vdd div voltage: Power Fail 1.93V. */
    HOSAL_COMP_CONFIG_VOLTAGE_5   = COMP_CONFIG_VOLTAGE_5,      /*!< Select the Comparator internal vdd div voltage: Power Fail 1.97V. */
    HOSAL_COMP_CONFIG_VOLTAGE_6   = COMP_CONFIG_VOLTAGE_6,      /*!< Select the Comparator internal vdd div voltage: Power Fail 2.01V. */
    HOSAL_COMP_CONFIG_VOLTAGE_7   = COMP_CONFIG_VOLTAGE_7,      /*!< Select the Comparator internal vdd div voltage: Power Fail 2.06V. */
    HOSAL_COMP_CONFIG_VOLTAGE_8   = COMP_CONFIG_VOLTAGE_8,      /*!< Select the Comparator internal vdd div voltage: Power Fail 2.10V. */
    HOSAL_COMP_CONFIG_VOLTAGE_9   = COMP_CONFIG_VOLTAGE_9,      /*!< Select the Comparator internal vdd div voltage: Power Fail 2.15V. */
    HOSAL_COMP_CONFIG_VOLTAGE_10  = COMP_CONFIG_VOLTAGE_10,     /*!< Select the Comparator internal vdd div voltage: Power Fail 2.20V. */
    HOSAL_COMP_CONFIG_VOLTAGE_11  = COMP_CONFIG_VOLTAGE_11,     /*!< Select the Comparator internal vdd div voltage: Power Fail 2.25V. */
    HOSAL_COMP_CONFIG_VOLTAGE_12  = COMP_CONFIG_VOLTAGE_12,     /*!< Select the Comparator internal vdd div voltage: Power Fail 2.31V. */
    HOSAL_COMP_CONFIG_VOLTAGE_13  = COMP_CONFIG_VOLTAGE_13,     /*!< Select the Comparator internal vdd div voltage: Power Fail 2.37V. */
    HOSAL_COMP_CONFIG_VOLTAGE_14  = COMP_CONFIG_VOLTAGE_14,     /*!< Select the Comparator internal vdd div voltage: Power Fail 2.43V. */
    HOSAL_COMP_CONFIG_VOLTAGE_15  = COMP_CONFIG_VOLTAGE_15,     /*!< Select the Comparator internal vdd div voltage: Power Fail 2.50V. */
    HOSAL_COMP_CONFIG_VOLTAGE_MAX = COMP_CONFIG_VOLTAGE_MAX,    /*!< Max Voltage Level. */
} hosal_comp_config_v_sel_t;

/**
 * \brief           Comparator configuration struct definitions.
 */
typedef struct {
    hosal_comp_config_selref_t    comp_selref;      /*!< Comparator selref*/
    hosal_comp_config_selinput_t  comp_selinput;    /*!< Comparator input P source select*/
    hosal_comp_config_ref_sel_t   comp_ref_sel;     /*!< Comparator input N source select*/
    hosal_comp_config_pw_t        comp_pw;          /*!< Comparator current select*/
    hosal_comp_config_hys_sel_t   comp_hys_sel;     /*!< Comparator hys window select*/
    hosal_comp_config_swdiv_t     comp_swdiv;       /*!< Comparator vdd div type switch*/
    hosal_comp_config_ds_wakeup_t comp_ds_wakeup;   /*!< Comparator wakeup in DeepSleep*/
    hosal_comp_config_ds_inv_t    comp_ds_inv;      /*!< Comparator output invert for waking up from DeepSleep*/
    hosal_comp_config_psrr_t      comp_psrr;        /*!< Comparator PSRR select*/
    hosal_comp_config_tc_t        comp_tc;          /*!< Comparator temperature coefficient select*/
    hosal_comp_config_en_start_t  comp_en_start;    /*!< Comparator en start select*/
    hosal_comp_config_int_pol_t   comp_int_pol;     /*!< Comparator interrupt polarity*/
    hosal_comp_config_ch_sel_t    comp_ch_sel;      /*!< Comparator AIO channel*/
    hosal_comp_config_v_sel_t     comp_v_sel;       /*!< Comparator internal vdd div voltage*/
} hosal_comp_config_t;

#define HOSAL_COMP_ENABLE()                     COMP_ENABLE()                /*!< Enable the Comparator module*/
#define HOSAL_COMP_ENABLE_SP()                  COMP_ENABLE_SP()             /*!< Enable the Comparator module at Sleep*/
#define HOSAL_COMP_ENABLE_DS()                  COMP_ENABLE_DS()             /*!< Enable the Comparator module at DeepSleep*/
#define HOSAL_COMP_INT_ENABLE()                 COMP_INT_ENABLE()            /*!< Enable the Comparator interrupt*/
#define HOSAL_COMP_INT_DISABLE()                COMP_INT_DISABLE()           /*!< Disable the Comparator interrupt*/
#define HOSAL_COMP_INT_CLEAR()                  COMP_INT_CLEAR()             /*!< Clear the Comparator interrupt status*/
#define HOSAL_COMP_INT_STATUS_GET()             COMP_INT_STATUS_GET()        /*!< Return the Comparator interrupt status*/
#define HOSAL_COMP_OUT_GET()                    COMP_OUT_GET()               /*!< Return the Comparator output result*/
#define HOSAL_COMP_INT_POL(para_set)            COMP_INT_POL(para_set)       /*!< Set the Comparator interrupt polarity*/
#define HOSAL_COMP_CH_SEL(para_set)             COMP_CH_SEL(para_set)        /*!< Select the Comparator P input for comparision*/
#define HOSAL_COMP_V_SEL(para_set)              COMP_V_SEL(para_set)         /*!< Select the Comparator internal vdd div voltage to compare with selected AIO channel*/
#define HOSAL_COMP_V_SEL_GET()                  COMP_V_SEL_GET()             /*!< Return the Comparator internal vdd div voltage to compare with selected AIO channel*/
#define HOSAL_COMP_INT_POL_GET()                COMP_INT_POL_GET()             /*!< Return the Comparator internal vdd div voltage to compare with selected AIO channel*/

#define HOSAL_COMP_SELREF_SEL(para_set)         COMP_SELREF_SEL(para_set)    /*!< Select the Comparator selref*/
#define HOSAL_COMP_SELINPUT_SEL(para_set)       COMP_SELINPUT_SEL(para_set)  /*!< Select the Comparator input P source*/
#define HOSAL_COMP_REF_SEL(para_set)            COMP_REF_SEL(para_set)       /*!< Select the Comparator N input for comparision*/
#define HOSAL_COMP_PW_SEL(para_set)             COMP_PW_SEL(para_set)        /*!< Select the Comparator current*/
#define HOSAL_COMP_HYS_SEL(para_set)            COMP_HYS_SEL(para_set)       /*!< Select the Comparator hys window*/
#define HOSAL_COMP_SWDIV_SEL(para_set)          COMP_SWDIV_SEL(para_set)     /*!< Switch the Comparator vdd div type*/
#define HOSAL_COMP_DS_WAKEUP(para_set)          COMP_DS_WAKEUP(para_set)     /*!< Select the Comparator wakeup in DeepSleep*/
#define HOSAL_COMP_DS_INV(para_set)             COMP_DS_INV(para_set)        /*!< Select the invert of the Comparator output for waking up from DeepSleep*/
#define HOSAL_COMP_PSRR_SEL(para_set)           COMP_PSRR_SEL(para_set)      /*!< Select the Comparator PSRR for constant Gm*/
#define HOSAL_COMP_TC_SEL(para_set)             COMP_TC_SEL(para_set)        /*!< Select the Comparator temperature coefficient*/
#define HOSAL_COMP_EN_START_SEL(para_set)       COMP_EN_START_SEL(para_set)  /*!< Select the Comparator en start*/


/**
 * \brief           Comparator interrupt service routine callback for user application.
 * \param[in]       comp_int_callback: Comparator interrupt callback handler
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_comp_register_int_callback(void* comp_int_callback);

/**
 * \brief           Setting Aux comparator configuration.
 * \param[in]       hosal_comp_config_t: Aux comparator configuration.
 * \param[in]       hosal_comparator_callback: Specifies user callback function when the aux comparator interrupt generated.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_comparator_open(hosal_comp_config_t* cfg, void* hosal_comparator_callback);

/**
 * \brief           Enable comparator
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_comparator_enable(void);


/**
 * \brief           Enable comparator in sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_comparator_sleep_enable(void);

/**
 * \brief           Enable comparator in deep sleep mode.
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_comparator_deep_sleep_enable(void);

/*@}*/ /* end of RT58X_HOSAL HOSAL_COMPARATOR */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_COMPARATOR_H */
