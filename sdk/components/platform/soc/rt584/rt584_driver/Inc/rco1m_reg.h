/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           rco1m_reg.h
 * \brief          rc1om register table file
 */
/*
 * This file is part of library_name.
 * Author: 
 */


#ifndef RCO1M_REG_H
#define RCO1M_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \brief           Rco 1m configuration 0 register at offet 0x00
 */
typedef union rco1m_cfg0_s {
    struct rco1m_cfg0_b {
        uint32_t cfg_cal_target : 18;           /*!< */
        uint32_t reserved1      : 6;            /*!< */
        uint32_t cfg_cal_en     : 1;            /*!< */
        uint32_t reserved2      : 7;            /*!< */
    } bit;
    uint32_t reg;
} rco1m_cfg0_t;

/**
 * \brief           Rco 1m configuration 1 register at offet 0x04
 */
typedef union rco1m_cfg1_s {
    struct rco1m_cfg1_b {
        uint32_t cfg_cal_lock_err    : 8;       /*!< */
        uint32_t cfg_cal_avg_coarse  : 2;       /*!< */
        uint32_t cfg_cal_avg_fine    : 2;       /*!< */
        uint32_t cfg_cal_avg_lock    : 2;       /*!< */
        uint32_t cfg_cal_dly         : 2;       /*!< */
        uint32_t cfg_cal_fine_gain   : 4;       /*!< */
        uint32_t cfg_cal_lock_gain   : 4;       /*!< */
        uint32_t cfg_cal_track_en    : 1;       /*!< */
        uint32_t cfg_cal_skip_coarse : 1;       /*!< */
        uint32_t cfg_cal_bound_mode  : 1;       /*!< */
        uint32_t cfg_tune_rco_sel    : 1;       /*!< */
        uint32_t en_ck_cal           : 1;       /*!< */
        uint32_t reserved1           : 3;       /*!< */
    } bit;
    uint32_t reg;
} rco1m_cfg1_t;

/**
 * \brief           Rco 1m result 0 register at offet 0x08
 */
typedef union rco1m_result0_s {
    struct rco1m_result0_b {
        uint32_t est_rco_result       : 18;     /*!< */
        uint32_t reserved1            : 5;      /*!< */
        uint32_t est_rco_result_valid : 1;      /*!< */
        uint32_t reserved2            : 3;      /*!< */
        uint32_t cal_busy             : 1;      /*!< */
        uint32_t cal_lock             : 1;      /*!< */
        uint32_t cal_timeout          : 1;      /*!< */
        uint32_t reserved3            : 1;      /*!< */
    } bit;
    uint32_t reg;
} rco1m_result0_t;

/**
 * \brief           Rco 1m result 1 register at offet 0x0C
 */
typedef union rco1m_result1_s {
    struct rco1m_result1_b {
        uint32_t tune_fine_rco   : 7;           /*!< */
        uint32_t reserved1       : 9;           /*!< */
        uint32_t tune_coarse_rco : 4;           /*!< */
        uint32_t reserved2       : 12;          /*!< */
    } bit;
    uint32_t reg;
} rco1m_result1_t;

/**
 * \brief          Rco 1m calibration total register 
 */
typedef struct {
    __IO rco1m_cfg0_t    cal1m_cfg0;            /*!< offset:0x00 */
    __IO rco1m_cfg1_t    cal1m_cfg1;            /*!< offset:0x04 */
    __I  rco1m_result0_t cal1m_result0;         /*!< offset:0x08 */
    __I  rco1m_result1_t cal1m_result1;         /*!< offset:0x0C */
} rco1m_cal_t;


#ifdef __cplusplus
}
#endif

#endif /* end of RCO1M_REG_H */
