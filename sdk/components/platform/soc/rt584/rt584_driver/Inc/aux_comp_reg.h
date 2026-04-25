/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           aux_comp_reg.h
 * \brief          Aux Comparator register definition header file
 */
/*
 * This file is part of library_name.
 * Author:          Kc.tseng
 */

#ifndef AUX_COMP_REG_H
#define AUX_COMP_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Aux Comparator analog control register at offet 0x00
 */
typedef union aux_comp_ana_ctl_s {
    struct aux_comp_ana_ctl_b {
        uint32_t comp_selref   : 1;             /*!< analog setting */
        uint32_t comp_selinput : 1;             /*!< analog setting */
        uint32_t comp_pw       : 2;             /*!< analog setting */
        uint32_t comp_selhys   : 2;             /*!< analog setting */
        uint32_t comp_swdiv    : 1;             /*!< analog setting */
        uint32_t comp_psrr     : 1;             /*!< analog setting */
        
        uint32_t comp_vsel     : 4;             /*!< analog setting */
        uint32_t comp_refsel   : 4;             /*!< analog setting */
        
        uint32_t comp_chsel    : 4;             /*!< analog setting */
        uint32_t comp_tc       : 1;             /*!< analog setting */
        uint32_t reserved1     : 3;             /*!< reserved bits */

        uint32_t comp_en_start : 2;             /*!< analog setting */
        uint32_t reserved2     : 6;             /*!< reserved bits */
    } bit;
    uint32_t reg;
} aux_comp_ana_ctl_t;

/**
 * \brief           Aux Comparator digtal control 0 register at offet 0x04
 */
typedef union aux_comp_dig_ctl0_s {
    struct aux_comp_dig_ctl0_b {
        uint32_t comp_en_nm        : 1;         /*!< enable comparator in normal mode */
        uint32_t comp_en_sp        : 1;         /*!< enable comparator in sleep mode */
        uint32_t comp_en_ds        : 1;         /*!< enable comparator in deep sleep mode */
        uint32_t reserved1         : 1;         /*!< reserved bits */
        uint32_t debounce_en       : 1;         /*!< enable debounce */
        uint32_t reserved2         : 1;         /*!< reserved bits */
        uint32_t debounce_sel      : 2;         /*!< debounce time select */

        uint32_t counter_mode_en   : 1;         /*!< enable counter mode */
        uint32_t reserved3         : 1;         /*!< reserved bits */
        uint32_t counter_mode_edge : 2;         /*!< set the trigger polarity of counter
                                                 mode */
        uint32_t ds_wakeup_en      : 1;         /*!< enable wakeup in Deepsleep when slow 
                                                 clock is disabled */
        uint32_t ds_wakeup_pol     : 1;         /*!< set the wakeup polarity in Deepsleep 
                                                 when ds_wakeup_en is enabled */
        uint32_t reserved4         : 2;         /*!< reserved bits */
        
        uint32_t counter_trigger_th : 16;
    } bit;
    uint32_t reg;
} aux_comp_dig_ctl0_t;

/**
 * \brief           Aux Comparator digtal control 1 register at offet 0x08
 */
typedef union aux_comp_dig_ctl1_s {
    struct aux_comp_dig_ctl1_b {
        uint32_t en_intr_rising   : 1;          /*!< enable counter interrupt */
        uint32_t en_intr_falling  : 1;          /*!< enable counter interrupt */
        uint32_t en_intr_counter  : 1;          /*!< enable counter interrupt */
        uint32_t reserved1        : 5;          /*!< reserved bits */

        uint32_t clr_intr_rising  : 1;          /*!< clear rising edge interrupt
                                                 status */
        uint32_t clr_intr_falling : 1;          /*!< clear falling edge interrupt
                                                 status */
        uint32_t clr_intr_counter : 1;          /*!< clear counter interrupt status */
        uint32_t clr_counter      : 1;          /*!< clear counter value */
        uint32_t reserved2        : 4;          /*!< reserved bits */

        uint32_t comp_settle_time : 4;          /*!< numbers of counter count */
        uint32_t reserved3        : 12;         /*!< reserved bits */
    } bit;
    uint32_t reg;
} aux_comp_dig_ctl1_t;

/**
 * \brief           Aux Comparator digtal control 2 register at offet 0x0C
 */
typedef union aux_comp_dig_ctl2_s {
    struct aux_comp_dig_ctl2_b {
        uint32_t sta_intr_rising  : 1;          /*!< rising edge interrupt status */
        uint32_t sta_intr_falling : 1;          /*!< falling edge interrupt status */
        uint32_t sta_intr_counter : 1;          /*!< counter interrupt status */
        uint32_t reserved1        : 5;          /*!< reserved bits */
        uint32_t comp_out         : 1;          /*!< comparator output value */
        uint32_t reserved2        : 7;          /*!< reserved bits */
        uint32_t counter_cnt      : 16;         /*!< numbers of counter count */
    } bit;
    uint32_t reg;
} aux_comp_dig_ctl2_t;

/**
 * \brief           Aux Comparator total register
 */
typedef struct {
    __IO aux_comp_ana_ctl_t  comp_ana_ctrl;     /*!< 0x00 analog control */
    __IO aux_comp_dig_ctl0_t comp_dig_ctrl0;    /*!< 0x04 digtal control 0 */
    __IO aux_comp_dig_ctl1_t comp_dig_ctrl1;    /*!< 0x08 digtal control 1 */
    __IO aux_comp_dig_ctl2_t comp_dig_ctrl2;    /*!< 0x0C digtal control 2 */
} aux_comp_ctl_t;

#ifdef __cplusplus
}
#endif

#endif /* End of AUX_COMP_REG_H */
