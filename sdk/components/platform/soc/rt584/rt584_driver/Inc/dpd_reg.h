/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            dpd_reg.h
 * \brief           Deep power down register header file
 */
/*
 * Author:          Kc.tseng
 */
#ifndef DPD_REG_H
#define DPD_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           DPD reset cause register at offet 0x00
 */
typedef union dpd_rst_cause_s {
    struct dpd_rst_cause_b {
        uint32_t rst_cause_por  : 1;            /*!< reset by power on */
        uint32_t rst_cause_ext  : 1;            /*!< reset by external reset */
        uint32_t rst_cause_dpd  : 1;            /*!< reset by deep power down */
        uint32_t rst_cause_ds   : 1;            /*!< reset by deep sleep */
        uint32_t rst_cause_wdt  : 1;            /*!< reset by watch dog */
        uint32_t rst_cause_soft : 1;            /*!< reset by sofware */
        uint32_t rst_cause_lock : 1;            /*!< reset by lock */
        uint32_t reserved       : 1;            /*!< reserved bits */
        uint32_t boot_status    : 3;            /*!< boot status */
        uint32_t reserve2       : 21;           /*!< reserved bits */
    } bit;
    uint32_t reg;
} dpd_rst_cause_t;

/**
 * \brief           DPD command register at offet 0x04
 */
typedef union dpd_cmd_s {
    struct dpd_cmd_b {
        uint32_t clr_rst_cause    : 1;          /*!< clear reset cause */
        uint32_t reserved         : 15;         /*!< reserved bits */
        uint32_t gpio_latch_en    : 1;          /*!< enable latch gpio state 
                                                 in deep power down mode */
        uint32_t reserved2        : 7;          /*!< reserved bits */
        uint32_t uvlo_div_sel     : 1;          /*!< uvlo test only */
        uint32_t uvlo_hys         : 1;          /*!< uvlo test only */
        uint32_t uvlo_ib          : 2;          /*!< uvlo test only */
        uint32_t reserved3        : 3;          /*!< reserved bits */
        uint32_t dpd_flash_dpd_en : 1;          /*!< elable Flash deep power down 
                                                 in low-power modes */
    } bit;
    uint32_t reg;
} dpd_cmd_t;

/**
 * \brief          Deep power down total register 
 */
typedef struct {
    __IO dpd_rst_cause_t dpd_rst_cause;         /*!< 0x00 reset cause register */
    __IO dpd_cmd_t       dpd_cmd;               /*!< 0x04 dpd command register */
    __IO uint32_t        dpd_gpio_en;           /*!< 0x08 set gpio can wakeup in 
                                                 deep power down mode */
    __IO uint32_t        dpd_gpio_inv;          /*!< 0x0c set the wake up polarity 
                                                 of the corresponding GPIO */
    __IO uint32_t        dpd_ret0_reg;          /*!< 0x10 retention 0 register */
    __IO uint32_t        dpd_ret1_reg;          /*!< 0x14 retention 1 register */
    __IO uint32_t        dpd_ret2_reg;          /*!< 0x18 retention 2 register */
    __IO uint32_t        dpd_ret3_reg;          /*!< 0x1C retention 3 register */
} dpd_t;

#define  RESET_CAUSE_POR                            (1<<0)
#define  RESET_CAUSE_EXT                            (1<<1)
#define  RESET_CAUSE_DPD                            (1<<2)
#define  RESET_CAUSE_DS                             (1<<3)
#define  RESET_CAUSE_WDT                            (1<<4)
#define  RESET_CAUSE_SOFT                           (1<<5)
#define  RESET_CAUSE_LOCK                           (1<<6)



#ifdef __cplusplus
}
#endif

#endif /* End of DPD_REG_H */
