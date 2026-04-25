/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           rco32k_reg.h
 * \brief          rco32k register table file
 */
/*
 * This file is part of library_name.
 * Author: 
 */


#ifndef RCO32K_REG_H
#define RCO32K_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Rco 1m configuration 0 register at offet 0x00
 */
typedef union rco32k_cfg0_s {
    struct rco32k_cfg0_b {
        uint32_t cfg_cal32k_target : 18;        /*!< */
        uint32_t reserved1         : 6;         /*!< */
        uint32_t cfg_cal32k_en     : 1;         /*!< */
        uint32_t reserved2         : 7;         /*!< */
    } bit;
    uint32_t reg;
} rco32k_cfg0_t;

/**
 * \brief           Rco 1m configuration 1 register at offet 0x04
 */
typedef union rco32k_cfg1_s {
    struct rco32k_cfg1_b {
        uint32_t cfg_cal32k_lock_err    : 8;    /*!< */
        uint32_t cfg_cal32k_avg_coarse  : 2;    /*!< */
        uint32_t cfg_cal32k_avg_fine    : 2;    /*!< */
        uint32_t cfg_cal32k_avg_lock    : 2;    /*!< */
        uint32_t cfg_cal32k_dly         : 2;    /*!< */
        uint32_t cfg_cal32k_fine_gain   : 4;    /*!< */
        uint32_t cfg_cal32k_lock_gain   : 4;    /*!< */
        uint32_t cfg_cal32k_track_en    : 1;    /*!< */
        uint32_t cfg_cal32k_skip_coarse : 1;    /*!< */
        uint32_t cfg_cal32k_bound_mode  : 1;    /*!< */
        uint32_t cfg_32k_rc_sel         : 1;    /*!< */
        uint32_t en_ck_cal32k           : 1;    /*!< */
        uint32_t reserved1              : 3;    /*!< */
    } bit;
    uint32_t reg;
} rco32k_cfg1_t;

/**
 * \brief           Rco 32k result 0 register at offet 0x08
 */
typedef union rco32k_result0_s {
    struct rco32k_result0_b {
        uint32_t est_32k_result       : 20;     /*!< */
        uint32_t reserved1            : 4;      /*!< */
        uint32_t est_32k_result_valid : 1;      /*!< */
        uint32_t reserved2            : 3;      /*!< */
        uint32_t cal32k_busy          : 1;      /*!< */
        uint32_t cal32k_lock          : 1;      /*!< */
        uint32_t cal32k_timeout       : 1;      /*!< */
        uint32_t reserved3            : 1;      /*!< */
    } bit;
    uint32_t reg;
} rco32k_result0_t;

/**
 * \brief           Rco 32k result 1 register at offet 0x0C
 */
typedef union rco32k_result1_s {
    struct rco32k_result1_b {
        uint32_t tune_fine_cal32k   : 8;        /*!< */
        uint32_t reserved1          : 8;        /*!< */
        uint32_t tune_coarse_cal32k : 2;        /*!< */
        uint32_t reserved2          : 14;       /*!< */
    } bit;
    uint32_t reg;
} rco32k_result1_t;

/**
 * \brief          Rco 32k calibration total register 
 */
typedef struct {
    __IO rco32k_cfg0_t    cal32k_cfg0;          /*!< offset:0x00 */
    __IO rco32k_cfg1_t    cal32k_cfg1;          /*!< offset:0x04 */
    __I  rco32k_result0_t cal32k_result0;       /*!< offset:0x08 */
    __I  rco32k_result1_t cal32k_result1;       /*!< offset:0x0C */
} rco32k_cal_t;


#ifdef __cplusplus
}
#endif

#endif /* end of RCO32K_REG_H */
