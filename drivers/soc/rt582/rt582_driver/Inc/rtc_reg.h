/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            rtc_reg.h
 * \brief           rtc register header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef RTC_REG_H
#define RTC_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           RTC total register 
 */
typedef struct {
    __IO uint32_t second;                       /*!< 0x00 rtc second value */
    __IO uint32_t minute;                       /*!< 0x04 rtc minute value */
    __IO uint32_t hour;                         /*!< 0x08 rtc hour value */
    __IO uint32_t day;                          /*!< 0x0C rtc day value */
    __IO uint32_t month;                        /*!< 0x10 rtc month value */
    __IO uint32_t year;                         /*!< 0x14 rtc year value */
    __IO uint32_t control;                      /*!< 0x18 rtc control register */
    __IO uint32_t clock_div;                    /*!< 0x1C rtc clock divisor */
    __IO uint32_t alarm_second;                 /*!< 0x20 rtc alarm second value */
    __IO uint32_t alarm_minute;                 /*!< 0x24 rtc alarm minute value */
    __IO uint32_t alarm_hour;                   /*!< 0x28 rtc alarm hour value */
    __IO uint32_t alarm_day;                    /*!< 0x2C rtc alarm day value */
    __IO uint32_t alarm_month;                  /*!< 0x30 rtc alarm month value */
    __IO uint32_t alarm_year;                   /*!< 0x34 rtc alarm year value */
    __IO uint32_t int_control;                  /*!< 0x38 rtc interrupt control 
                                                                    register */
    __IO uint32_t int_status;                   /*!< 0x3C rtc interrupt status */
    __IO uint32_t int_clear;                    /*!< 0x40 rtc interrupt clear */
    __IO uint32_t load;                         /*!< 0x44 rtc load register */
} rtc_t;

#ifdef __cplusplus
}
#endif

#endif /* End of RTC_REG_H */
