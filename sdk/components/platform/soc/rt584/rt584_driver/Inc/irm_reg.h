/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           irm.h
 * \brief          IRM register definition header file
 */
/*
 * Author:         Kc.tseng
 */


#ifndef IRM_REG_H
#define IRM_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Irm config register at offet 0x00
 */
typedef union irm_conf_s {
    struct irm_conf_b {
        uint32_t reserved  : 1;                 /*!< reserved bits */
        uint32_t op_mode   : 1;                 /*!< operation mode */
        uint32_t out_mode  : 2;                 /*!< output Logic Function */
        uint32_t no_car    : 1;                 /*!< no carrier */
        uint32_t car_ini   : 1;                 /*!< initial carrier Value */
        uint32_t reserved2 : 26;                /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_conf_t;

/**
 * \brief           Irm carrier register at offet 0x04
 */
typedef union irm_carrier_s {
    struct irm_carrier_b {
        uint32_t car_base_cnt : 16;             /*!< carrier base count */
        uint32_t car_low_cnt  : 4;              /*!< carrier low value count */
        uint32_t car_high_cnt : 4;              /*!< carrier high value count */
        uint32_t reserved     : 8;              /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_carrier_t;

/**
 * \brief           Irm fifo in register at offet 0x08
 */
typedef union irm_fifo_in_s {
    struct irm_fifo_in_b {
        uint32_t env_cnt  : 16;                 /*!< envelope duration */
        uint32_t env_mark : 1;                  /*!< envelope mark */
        uint32_t env_init : 1;                  /*!< envelope interrupt */
        uint32_t env_last : 1;                  /*!< last envelope */
        uint32_t reserved : 13;                 /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_fifo_in_t;

/**
 * \brief           Irm status register at offet 0x0C
 */
typedef union irm_status_s
{
    struct irm_status_b
    {
        uint32_t fifo_lvl   : 5;                /*!< current fifo level */
        uint32_t fifo_full  : 1;                /*!< fifo full flag */
        uint32_t fifo_empty : 1;                /*!< fifo empty flag */
        uint32_t reserved   : 25;               /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_status_t;

/**
 * \brief           Irm command register at offet 0x10
 */
typedef union irm_cmd_s
{
    struct irm_cmd_b
    {
        uint32_t ir_ena   : 1;                  /*!< IR enable */
        uint32_t ir_dis   : 1;                  /*!< IR disable */
        uint32_t ir_start : 1;                  /*!< IR start */
        uint32_t ir_rst   : 1;                  /*!< IR reset */
        uint32_t reserved : 28;                 /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_cmd_t;

/**
 * \brief           Irm interrupt status register at offet 0x14
 */
typedef union irm_int_status_s
{
    struct irm_int_status_b
    {
        uint32_t env_start_int : 1;             /*!< envelop start interrupt */
        uint32_t env_last_int  : 1;             /*!< envelop last interrupt */
        uint32_t env_ufl_int   : 1;             /*!< fifo underflow interrupt */
        uint32_t reserved      : 29;            /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_int_status_t;

/**
 * \brief           Irm interrupt enable register at offet 0x18
 */
typedef union irm_int_ena_s
{
    struct irm_int_ena_b
    {
        uint32_t env_start_ena : 1;             /*!< enable/disable env_start interrupt */
        uint32_t env_last_ena  : 1;             /*!< enable/disable env_last interrupt */
        uint32_t env_ufl_ena   : 1;             /*!< enable/disable fifo_ufl interrupt */
        uint32_t reserved      : 29;            /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_int_ena_t;

/**
 * \brief           Irm interrupt clear register at offet 0x1C
 */
typedef union irm_int_clr_s
{
    struct irm_int_clr_b
    {
        uint32_t env_start_clr : 1;             /*!< clear env_start interrupt */
        uint32_t env_last_clr  : 1;             /*!< clear env_last interrupt */
        uint32_t env_ufl_clr   : 1;             /*!< clear fifo_ufl interrupt */
        uint32_t reserved      : 29;            /*!< reserved bits */
    } bit;
    uint32_t reg;
} irm_int_clr_t;

/**
 * \brief           IRM total register
 */
typedef struct {
    __IO irm_conf_t       ir_conf;              /*!< 0x00 ir configuration register */
    __IO irm_carrier_t    carrier ;             /*!< 0x04 ir carrier configuration register */
    __IO irm_fifo_in_t    fifo_in;              /*!< 0x08 fifo register */
    __I  irm_status_t     status ;              /*!< 0x0c fifo status register */
    __IO irm_cmd_t        ir_cmd;               /*!< 0x10 ir command register */
    __I  irm_int_status_t int_status;           /*!< 0x14 ir interrupt status register */
    __IO irm_int_ena_t    int_ena;              /*!< 0x18 ir interrupt enable register */
    __IO irm_int_clr_t    int_clr;              /*!< 0x1c ir interrupt clear register */

} irm_t;


#ifdef __cplusplus
}
#endif

#endif /* End of IRM_REG_H */
