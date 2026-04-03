/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            gpio_reg.h
 * \brief           GPIO register header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef GPIO_REG_H
#define GPIO_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Gpio state register at offet 0x00
 */
typedef union gpio_state_s {
    __IM uint32_t state;                /*!< GPIO state */
    __OM uint32_t output_high;          /*!< set GPIO output high */
} gpio_state_t;

/**
 * \brief           Gpio interrupt status register at offet 0x04
 */
typedef union gpio_int_status_s {
    __IM uint32_t int_status;           /*!< GPIO Interrupt status */
    __OM uint32_t output_low;           /*!< set GPIO output low */
} gpio_int_status_t;

/**
 * \brief           Gpio total register
 */
typedef struct {
    __IO gpio_state_t state;            /*!< 0x00 gpio pin state(Read),
                                         set gpio output high(write) */
    __IO gpio_int_status_t int_status;  /*!< 0x04 gpio interrupt status(Read),
                                         set gpio ouput low(write) */
    __IO uint32_t output_en;            /*!< 0x08 gpio output enable */
    __IO uint32_t input_en;             /*!< 0x0C gpio input enable */
    __IO uint32_t enable_int;           /*!< 0x10 gpio interruput enable */
    __IO uint32_t disable_int;          /*!< 0x14 gpio interruput disable */
    __IO uint32_t edge_trig;            /*!< 0x18 edge trigger interrupt */
    __IO uint32_t level_trig;           /*!< 0x1C level trigger interrupt */
    __IO uint32_t postitive;            /*!< 0x20 high or rising edge
                                         trigger interrupt*/
    __IO uint32_t negative;             /*!< 0x24 low or falling edge
                                         trigger interrupt*/
    __IO uint32_t both_edge_en;         /*!< 0x28 both edge trigger interrupt */
    __IO uint32_t both_edge_clr;        /*!< 0x2C disable both edge trigger interrupt*/
    __IO uint32_t edge_int_clr;         /*!< 0x30 edge interrupt status clear*/
    __IO uint32_t reserve[3];           /*!< 0x34 - 0x3C reserved */
    __IO uint32_t debounce_en;          /*!< 0x40 enable debounce */
    __IO uint32_t debounce_dis;         /*!< 0x44 disable debounce */
    __IO uint32_t debounce_time;        /*!< 0x48 debounce time */
} gpio_t;

#ifdef __cplusplus
}
#endif

#endif /* End of GPIO_REG_H */
