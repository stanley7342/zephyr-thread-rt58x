/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            timer_reg.h
 * \brief           timer register header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef TIMER_REG_H
#define TIMER_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Timer control register at offet 0x08
 */
typedef union timern_ctrl_s {
    struct timern_ctrl_b {
        uint32_t reserved1  : 2;                /*!< reserved bits */
        uint32_t prescale   : 3;                /*!< timer prescale setting */
        uint32_t int_enable : 1;                /*!< timer interrupt enable bit */
        uint32_t mode       : 1;                /*!< timer mode setting */
        uint32_t en         : 1;                /*!< timer enable bit */
        uint32_t int_status : 1;                /*!< timer interrupt status */
        uint32_t reserved2  : 23;               /*!< reserved bits */
    } bit;

    uint32_t reg;
} timern_ctrl_t;

/**
 * \brief           Timer repeat delay register at offet 0x10
 */
typedef union timern_repdly_s {
    struct timern_repdly_b {
        uint32_t int_repeat_delay : 16;         /*!< slow timer0/1 to use,
                                                 how many times to delay trigger interrupt */
        uint32_t reserved1        : 16;         /*!< reserved bits */
    } bit;

    uint32_t reg;
} timern_repdly_t;

/**
 * \brief           Timer total register 
 */
typedef struct {
    __IO uint32_t load;                         /*!< 0x00, timer load value */
    __IO uint32_t value;                        /*!< 0x04, timer current value */
    __IO timern_ctrl_t control;                 /*!< 0x08, timer control register */
    __IO uint32_t clear;                        /*!< 0x0C, write 1 to clear interrupt  */
    __IO timern_repdly_t repeat_delay;          /*!< 0x10, slow timer0/1 in sleep mode use, 
                                                 how many times to delay trigger interrupt */
} timern_t;

#ifdef __cplusplus
}
#endif

#endif /* End of TIMER_REG_H */
