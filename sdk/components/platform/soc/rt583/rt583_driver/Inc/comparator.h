/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            comparator.h
 * \brief           comparator header file
 *          Define software build code based on chip type and chip version.
 *          Please change the definition of CHIP_TYPE and CHIP_VERSION to match real product IC.
 *          User can redefine CHIP_TYPE and CHIP_VERSION by one of two options:
 *          1. Select Target in Keil IDE will change CHIP_TYPE and CHIP_VERSION definitions in the Preprocessor Symbols field.
 *          2. User can define CHIP_TYPE and CHIP_VERSION in this file if they are not defined in the Preprocessor Symbols field.
 *          User can use the CHIP_MODEL definition which is defined by CHIP_TYPE and CHIP_VERSION.
 */
/*
 * This file is part of library_name.
 * Author:     
 */

#ifndef COMPARATOR_H
#define COMPARATOR_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "mcu.h"

/**
 * \defgroup COMPARATOR Comparator
 * \ingroup RT58X_DRIVER
 * \brief  Define Comparator definitions, structures, and functions
 * @{
 */

typedef enum {
    COMP_CONFIG_SELREF_INTERNAL = 0,            /*!< Input N source internal bg_0.6v. */
    COMP_CONFIG_SELREF_EXTERNAL = 1,            /*!< Input N source external. */
} comp_config_selref_t;

typedef enum {
    COMP_CONFIG_SELINPUT_INTERNAL = 0,          /*!< Input P source internal vdd div. */
    COMP_CONFIG_SELINPUT_EXTERNAL = 1,          /*!< Input P source external. */
} comp_config_selinput_t;

typedef enum {
    COMP_CONFIG_REFSEL_CHANNEL_0 = 0,           /*!< Select Comparator N input  channel 0. */
    COMP_CONFIG_REFSEL_CHANNEL_1 = 1,           /*!< Select Comparator N input  channel 1. */
    COMP_CONFIG_REFSEL_CHANNEL_2 = 2,           /*!< Select Comparator N input channel 2. */
    COMP_CONFIG_REFSEL_CHANNEL_3 = 3,           /*!< Select Comparator N input  channel 3. */
    COMP_CONFIG_REFSEL_CHANNEL_4 = 4,           /*!< Select Comparator N input  channel 4. */
    COMP_CONFIG_REFSEL_CHANNEL_5 = 5,           /*!< Select Comparator N input  channel 5. */
    COMP_CONFIG_REFSEL_CHANNEL_6 = 6,           /*!< Select Comparator N input  channel 6. */
    COMP_CONFIG_REFSEL_CHANNEL_7 = 7,           /*!< Select Comparator N input  channel 7. */
    COMP_CONFIG_REFSEL_IOLDO = 8,               /*!< Select Comparator N input IOLDO. */
    COMP_CONFIG_REFSEL_AVSS = 9,                /*!< Select Comparator N input AVSS. */
} comp_config_ref_sel_t;

typedef enum {
    COMP_CONFIG_PW_SMALLEST = 0,                /*!< Comparator current select smallest. */
    COMP_CONFIG_PW_LARGEST = 3,                 /*!< Comparator current select largest. */
} comp_config_pw_t;

typedef enum {
    COMP_CONFIG_HYS_40 = 0,                     /*!< Comparator hys window select 40mV. */
    COMP_CONFIG_HYS_80 = 1,                     /*!< Comparator hys window select 80mV. */
    COMP_CONFIG_HYS_100 = 2,                    /*!< Comparator hys window select100mV. */
    COMP_CONFIG_HYS_130 = 3,                    /*!< Comparator hys window select 130mV. */
} comp_config_hys_sel_t;

typedef enum {
    COMP_CONFIG_SWDIV_MOS = 0,                  /*!< Comparator switch vdd MOS div. */
    COMP_CONFIG_SWDIV_RES = 1,                  /*!< Comparator switch vdd RES div. */
} comp_config_swdiv_t;

typedef enum {
    COMP_CONFIG_DS_WAKEUP_DISABLE = 0,          /*!< Disable the Comparator wakeup in DeepSleep. */
    COMP_CONFIG_DS_WAKEUP_ENABLE = 1,           /*!< Enable the Comparator wakeup in DeepSleep. */
} comp_config_ds_wakeup_t;

typedef enum {
    COMP_CONFIG_DS_INVERT_DISABLE = 0,          /*!< Disable the Comparator output invert for waking up from DeepSleep. */
    COMP_CONFIG_DS_INVERT_ENABLE = 1,           /*!< Enable the Comparator output  invert for waking up from DeepSleep. */
} comp_config_ds_inv_t;

typedef enum {
    COMP_CONFIG_PSRR_REGULAR = 0,               /*!< Comparator PSRR regular. */
    COMP_CONFIG_PSRR_HIGH_V = 1,                /*!< Comparator PSRR high V. */
} comp_config_psrr_t;

typedef enum {
    COMP_CONFIG_TC_REGULAR =  0,                /*!< Comparator temperature coefficient regular. */
    COMP_CONFIG_TC_COMPENSATE = 1,              /*!< Comparator temperature coefficient compensate. */
} comp_config_tc_t;

typedef enum {
    COMP_CONFIG_EN_START_0 = 0,
    COMP_CONFIG_EN_START_1 = 1,
    COMP_CONFIG_EN_START_2 = 2,
    COMP_CONFIG_EN_START_3 = 3,
} comp_config_en_start_t;

/**
 * \brief           Comparator trigger edge
 */
typedef enum {
    COMP_CONFIG_INT_POL_RISING = 0,             /*!< Rising edge. */
    COMP_CONFIG_INT_POL_FALLING = 1,            /*!< Falling edge. */
    COMP_CONFIG_INT_POL_BOTH = 2,               /*!< Both edge. */
} comp_config_int_pol_t;

/**
 * \brief           P input select.
 */
typedef enum {
    COMP_CONFIG_CHSEL_CHANNEL_0 = 0,    /*!< Select Comparator P input channel 0. */
    COMP_CONFIG_CHSEL_CHANNEL_1 = 1,    /*!< Select Comparator P input channel 1. */
    COMP_CONFIG_CHSEL_CHANNEL_2 = 2,    /*!< Select Comparator P input channel 2. */
    COMP_CONFIG_CHSEL_CHANNEL_3 = 3,    /*!< Select Comparator P input channel 3. */
    COMP_CONFIG_CHSEL_CHANNEL_4 = 4,    /*!< Select Comparator P input channel 4. */
    COMP_CONFIG_CHSEL_CHANNEL_5 = 5,    /*!< Select Comparator P input channel 5. */
    COMP_CONFIG_CHSEL_CHANNEL_6 = 6,    /*!< Select Comparator P input channel 6. */
    COMP_CONFIG_CHSEL_CHANNEL_7 = 7,    /*!< Select Comparator P input channel 7. */
    COMP_CONFIG_CHSEL_AVDD_1V = 8,      /*!< Select Comparator P input AVDD 1V. */
    COMP_CONFIG_CHSEL_AVSS = 9,         /*!< Select Comparator P input AVSS. */
    COMP_CONFIG_CHSEL_CHANNEL_MAX = 10, /*!< Max Channel 10. */
} comp_config_ch_sel_t;

/**
 * \brief           Comparator voltage threshold.
 */
typedef enum {
    COMP_CONFIG_VOLTAGE_0 = 0,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 1.78V. */
    COMP_CONFIG_VOLTAGE_1 = 1,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 1.82V. */
    COMP_CONFIG_VOLTAGE_2 = 2,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 1.85V. */
    COMP_CONFIG_VOLTAGE_3 = 3,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 1.89V. */
    COMP_CONFIG_VOLTAGE_4 = 4,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 1.93V. */
    COMP_CONFIG_VOLTAGE_5 = 5,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 1.97V. */
    COMP_CONFIG_VOLTAGE_6 = 6,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.01V. */
    COMP_CONFIG_VOLTAGE_7 = 7,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.06V. */
    COMP_CONFIG_VOLTAGE_8 = 8,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.10V. */
    COMP_CONFIG_VOLTAGE_9 = 9,      /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.15V. */
    COMP_CONFIG_VOLTAGE_10 = 10,    /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.20V. */
    COMP_CONFIG_VOLTAGE_11 = 11,    /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.25V. */
    COMP_CONFIG_VOLTAGE_12 = 12,    /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.31V. */
    COMP_CONFIG_VOLTAGE_13 = 13,    /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.37V. */
    COMP_CONFIG_VOLTAGE_14 = 14,    /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.43V. */
    COMP_CONFIG_VOLTAGE_15 = 15,    /*!< Select the Comparator internal vdd div
                                     voltage: Power Fail 2.50V. */
    COMP_CONFIG_VOLTAGE_MAX = 16,   /*!< Max Voltage Level. */
} comp_config_v_sel_t;

/**
 * \brief           Comparator configuration.
 */
typedef struct {
    comp_config_selref_t comp_selref;           /*!< Comparator selref */
    comp_config_selinput_t comp_selinput;       /*!< Comparator input P source
                                                 select */
    comp_config_ref_sel_t comp_ref_sel;         /*!< Comparator input N source
                                                 select */
    comp_config_pw_t comp_pw;                   /*!< Comparator current select */
    comp_config_hys_sel_t comp_hys_sel;         /*!< Comparator hys window 
                                                 select */
    comp_config_swdiv_t comp_swdiv;             /*!< Comparator vdd div type
                                                 switch */
    comp_config_ds_wakeup_t comp_ds_wakeup;     /*!< Comparator wakeup in
                                                 DeepSleep */
    comp_config_ds_inv_t comp_ds_inv;           /*!< Comparator output invert
                                                 for waking up from DeepSleep */
    comp_config_psrr_t comp_psrr;               /*!< Comparator PSRR select */
    comp_config_tc_t comp_tc;                   /*!< Comparator temperature
                                                 coefficient select */
    comp_config_en_start_t comp_en_start;       /*!< Comparator en start
                                                 select */
    comp_config_int_pol_t comp_int_pol;         /*!< Comparator interrupt
                                                 polarity */
    comp_config_ch_sel_t comp_ch_sel;           /*!< Comparator AIO channel */
    comp_config_v_sel_t comp_v_sel;             /*!< Comparator internal vdd
                                                 div voltage */
} comp_config_t;

/**
 * \brief           User cb handler prototype.
 *                  This function is called when the requested number of samples has been processed.
 */
typedef void (*comp_isr_handler_t)(void);

#define COMP_ENABLE()                                                          \
    (PMU->pmu_comp0.bit.aux_comp_en_nm =                                       \
         ENABLE) /*!< Enable the Comparator module*/
#define COMP_ENABLE_SP()                                                       \
    (PMU->pmu_comp0.bit.aux_comp_en_sp =                                       \
         ENABLE) /*!< Enable the Comparator module at Sleep*/
#define COMP_ENABLE_DS()                                                       \
    (PMU->pmu_comp0.bit.aux_comp_en_ds =                                       \
         ENABLE) /*!< Enable the Comparator module at DeepSleep*/
#define COMP_INT_ENABLE()                                                      \
    (PMU->pmu_comp0.bit.aux_comp_int_en =                                      \
         ENABLE) /*!< Enable the Comparator interrupt*/
#define COMP_INT_DISABLE()                                                     \
    (PMU->pmu_comp0.bit.aux_comp_int_en =                                      \
         DISABLE) /*!< Disable the Comparator interrupt*/
#define COMP_INT_CLEAR()                                                       \
    (PMU->pmu_comp1.bit.aux_comp_int_clr =                                     \
         ENABLE) /*!< Clear the Comparator interrupt status*/
#define COMP_INT_STATUS_GET()                                                  \
    (PMU->pmu_comp2.bit                                                        \
         .aux_comp_int_sta) /*!< Return the Comparator interrupt status*/
#define COMP_OUT_GET()                                                         \
    (PMU->pmu_comp2.bit.aux_comp_out) /*!< Return the Comparator output result*/
#define COMP_INT_POL(para_set)                                                 \
    (PMU->pmu_comp0.bit.aux_comp_int_pol =                                     \
         para_set) /*!< Set the Comparator interrupt polarity*/
#define COMP_CH_SEL(para_set)                                                  \
    (PMU->pmu_comp0.bit.aux_comp_chsel =                                       \
         para_set) /*!< Select the Comparator P input for comparision*/
#define COMP_V_SEL(para_set)                                                   \
    (PMU->pmu_comp0.bit.aux_comp_vsel =                                        \
         para_set) /*!< Select the Comparator internal vdd div voltage to compare with selected AIO channel*/
#define COMP_V_SEL_GET()                                                       \
    (PMU->pmu_comp0.bit                                                        \
         .aux_comp_vsel) /*!< Return the Comparator internal vdd div voltage to compare with selected AIO channel*/
#define COMP_INT_POL_GET()                                                     \
    (PMU->pmu_comp0.bit                                                        \
         .aux_comp_int_pol) /**< get the Comparator interrupt polarity*/


#define COMP_SELREF_SEL(para_set)                                              \
    (PMU->pmu_comp0.bit.aux_comp_selref =                                      \
         para_set) /*!< Select the Comparator selref*/
#define COMP_SELINPUT_SEL(para_set)                                            \
    (PMU->pmu_comp0.bit.aux_comp_selinput =                                    \
         para_set) /*!< Select the Comparator input P source*/
#define COMP_REF_SEL(para_set)                                                 \
    (PMU->pmu_comp0.bit.aux_comp_refsel =                                      \
         para_set) /*!< Select the Comparator N input for comparision*/
#define COMP_PW_SEL(para_set)                                                  \
    (PMU->pmu_comp0.bit.aux_comp_pw =                                          \
         para_set) /*!< Select the Comparator current*/
#define COMP_HYS_SEL(para_set)                                                 \
    (PMU->pmu_comp0.bit.aux_comp_selhys =                                      \
         para_set) /*!< Select the Comparator hys window*/
#define COMP_SWDIV_SEL(para_set)                                               \
    (PMU->pmu_comp0.bit.aux_comp_swdiv =                                       \
         para_set) /*!< Switch the Comparator vdd div type*/
#define COMP_DS_WAKEUP(para_set)                                               \
    (PMU->pmu_comp0.bit.aux_comp_ds_wakeup_en =                                \
         para_set) /*!< Select the Comparator wakeup in DeepSleep*/
#define COMP_DS_INV(para_set)                                                  \
    (PMU->pmu_comp0.bit.aux_comp_ds_inv =                                      \
         para_set) /*!< Select the invert of the Comparator output for waking up from DeepSleep*/
#define COMP_PSRR_SEL(para_set)                                                \
    (PMU->pmu_comp0.bit.aux_comp_psrr =                                        \
         para_set) /*!< Select the Comparator PSRR for constant Gm*/
#define COMP_TC_SEL(para_set)                                                  \
    (PMU->pmu_comp0.bit.aux_comp_tc =                                          \
         para_set) /*!< Select the Comparator temperature coefficient*/
#define COMP_EN_START_SEL(para_set)                                            \
    (PMU->pmu_comp0.bit.aux_en_start =                                         \
         para_set) /*!< Select the Comparator en start*/

/**
 * \brief           Comparator interrupt service routine callback for user application.
 * \param[in]       comp_int_callback Comparator interrupt callback handler
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_register_int_callback(comp_isr_handler_t comp_int_callback);

/**
 * \brief           Enable the specified Comparator interrupts
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_int_enable(void);

/** 
 * \brief           Disable all Comparator interrupt(s)
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_int_disable(void);

/**
 * \brief           Initinal the comparator config
 * \param[in]       p_config:
 *                      \arg comp_selref        Comparator selref
 *                      \arg comp_selinput      Comparator input P source select
 *                      \arg comp_ref_sel       Comparator input N source select
 *                      \arg comp_pw            Comparator current select
 *                      \arg comp_hys_sel       Comparator hys window select
 *                      \arg comp_swdiv         Comparator vdd div type switch
 *                      \arg comp_ds_wakeup     Comparator wakeup in DeepSleep
 *                      \arg comp_ds_inv        Comparator output invert for waking up from DeepSleep
 *                      \arg comp_psrr          Comparator PSRR select
 *                      \arg comp_tc            Comparator temperature coefficient select
 *                      \arg comp_en_start      Comparator en start select
 *                      \arg comp_int_pol       Comparator interrupt polarity
 *                      \arg comp_ch_sel        Comparator AIO channel
 *                      \arg comp_v_sel         Comparator internal vdd div voltage
 * \param[in]       comp_int_callback: register comparator interrutp callback function
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_init(comp_config_t* p_config,
                   comp_isr_handler_t comp_int_callback);

/**
 * \brief           Comparator Enable
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_enable(void);

/**
 * \brief           Comparator Enable At Sleep
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_enable_at_sleep(void);

/**
 * \brief           Comparator Enable At DeepSleep
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_enable_at_deepsleep(void);

/**
 * \brief           Comparator Enable counter for avoid noise
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t comp_int_callback_counter(uint32_t counter);

/*@}*/ /* end of RT58X_DRIVER COMPARATOR */

#ifdef __cplusplus
}
#endif

#endif /* End of COMPARATOR_H */