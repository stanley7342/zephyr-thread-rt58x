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
    __IO uint32_t rtc_second;                   /*!< 0x00 rtc second value */
    __IO uint32_t rtc_minute;                   /*!< 0x04 rtc minute value */
    __IO uint32_t rtc_hour;                     /*!< 0x08 rtc hour value */
    __IO uint32_t rtc_day;                      /*!< 0x0C rtc day value */
    __IO uint32_t rtc_month;                    /*!< 0x10 rtc month value */
    __IO uint32_t rtc_year;                     /*!< 0x14 rtc year value */
    __IO uint32_t rtc_control;                  /*!< 0x18 rtc control register*/
    __IO uint32_t rtc_clock_div;                /*!< 0x1C rtc clock devide register */
    __IO uint32_t rtc_alarm_second;             /*!< 0x20 rtc alarm second value */
    __IO uint32_t rtc_alarm_minute;             /*!< 0x24 rtc alarm minute value */
    __IO uint32_t rtc_alarm_hour;               /*!< 0x28 rtc alarm hour value */
    __IO uint32_t rtc_alarm_day;                /*!< 0x2C rtc alarm day value */
    __IO uint32_t rtc_alarm_month;              /*!< 0x30 rtc alarm month value */
    __IO uint32_t rtc_alarm_year;               /*!< 0x34 rtc alarm year value */
    __IO uint32_t rtc_int_control;              /*!< 0x38 rtc interrupt control register */
    __IO uint32_t rtc_int_status;               /*!< 0x3C rtc interrupt status */
    __IO uint32_t rtc_int_clear;                /*!< 0x40 rtc interrupt clear */
    __IO uint32_t rtc_load;                     /*!< 0x44 rtc load register */
    __IO uint32_t rtc_msecond;                  /*!< 0x48 rtc milli-second value */
    __IO uint32_t rtc_alarm_msecond;            /*!< 0x4C rtc alarm milli-second value */
} rtc_t;

/**
 * \brief           RTC control register shift bit definitions
 */
#define  RTC_CTRL_CLRPLS     (1<<7)
#define  RTC_CTRL_ENABLE     (1<<8)

/**
 * \brief           RTC load register shift bit definitions
 */
#define  RTC_LOAD_TIME       (1<<0)
#define  RTC_LOAD_ALARM      (1<<1)
#define  RTC_LOAD_DIVISOR    (1<<2)


#ifdef __cplusplus
}
#endif

#endif /* End of RTC_REG_H */
