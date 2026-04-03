/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            rtc.h
 * \brief           RTC driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef RTC_H
#define RTC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"

/**
 * \defgroup RTC Rtc
 * \ingroup RT58X_DRIVER
 * \brief  Define Rtc definitions, structures, and functions
 * @{
 */

/*
 *  RTC_MODE_EVERY_SEC_INTERRUPT mode will trigger rtc "second interrupt" for
 *  every "second" if RTC_MODE_EN_SEC_INTERRUPT mode set.
 *  RTC_MODE_MATCH_SEC_INTERRUPT mode will only trigger rtc "second interrupt"
 * "current second" = "alarm second" once per minute, if RTC_MODE_EN_SEC_INTERRUPT
 *  mode set.
 *  RTC_MODE_SECOND_EVENT_INTERRUPT will generate "event interupt" every SS matched,
 *  if RTC_MODE_EVENT_INTERRUPT should be enable.
 *  RTC_MODE_EVERY_MIN_INTERRUPT mode will trigger rtc "min interrupt" for every
 * "minute" if RTC_MODE_EN_MIN_INTERRUPT mode set.
 *  RTC_MODE_MATCH_MIN_INTERRUPT mode will only trigger rtc "minute interrupt"
 * "current minute" = "alarm minute" once per hour, if RTC_MODE_EN_MIN_INTERRUPT
 *  mode set.
 *  RTC_MODE_MINUTE_EVENT_INTERRUPT will generate "event interupt" for MM:SS matched.
 *  (RTC_MODE_SECOND_EVENT_INTERRUPT should NOT set for this option. And
 *  RTC_MODE_EVENT_INTERRUPT should be enable.)
 *  RTC_MODE_EVERY_HOUR_INTERRUPT mode will trigger rtc "hour interrupt" for every
 * "HOUR" if RTC_MODE_EN_HOUR_INTERRUPT mode set.
 *  RTC_MODE_MATCH_HOUR_INTERRUPT mode will only trigger rtc "hour interrupt"
 * "current hour" = "alarm hour" once per day, if RTC_MODE_EN_HOUR_INTERRUPT mode set.
 *  RTC_MODE_HOUR_EVENT_INTERRUPT will generate "event interupt"for HH:MM:SS matched.
 *  (RTC_MODE_MIN_EVENT_INTERRUPT and  RTC_MODE_SEC_EVENT_INTERRUPT should
 *   NOT set for this option.  And RTC_MODE_EVENT_INTERRUPT should be enable.)
 *  RTC_MODE_EVERY_DAY_INTERRUPT mode will trigger rtc "day interrupt" for every
 * "DAY" if RTC_MODE_EN_DAY_INTERRUPT mode set.
 *  RTC_MODE_MATCH_DAY_INTERRUPT mode will only trigger rtc "day interrupt"
 * "current day" = "alarm day" once per month, if RTC_MODE_EN_DAY_INTERRUPT mode set.
 *  RTC_MODE_DAY_EVENT_INTERRUPT will generate "event interupt" for DAY HH:MM:SS matched
 *  (RTC_MODE_HOUR_EVENT_INTERRUPT, RTC_MODE_MIN_EVENT_INTERRUPT and
 *   RTC_MODE_SEC_EVENT_INTERRUPT should NOT set for this option.
 *   And RTC_MODE_EVENT_INTERRUPT should be enable.)
 *  RTC_MODE_EVERY_MONTH_INTERRUPT mode will trigger rtc "month interrupt" for every
 * "month" if RTC_MODE_EN_MONTH_INTERRUPT mode set.
 *  RTC_MODE_MATCH_MONTH_INTERRUPT mode will only trigger rtc "month interrupt"
 * "current month" = "alarm month" once per year, if RTC_MODE_EN_MONTH_INTERRUPT mode set.
 *  RTC_MODE_MONTH_EVENT_INTERRUPT will generate "event interupt" for MON:DAY HH:MM:SS matched.
 *  (RTC_MODE_DAY_EVENT_INTERRUPT, RTC_MODE_HOUR_EVENT_INTERRUPT,
 *   RTC_MODE_MIN_EVENT_INTERRUPT and RTC_MODE_SEC_EVENT_INTERRUPT should
 *   NOT set for this option. And RTC_MODE_EVENT_INTERRUPT should be enable.)
 *  RTC_MODE_EVERY_YEAR_INTERRUPT mode will trigger rtc "year interrupt" for every
 * "year" if RTC_MODE_EN_YEAR_INTERRUPT mode set.
 *  RTC_MODE_MATCH_YEAR_INTERRUPT mode will only trigger rtc "year interrupt"
 * "current year" = "alarm year" once per century, if RTC_MODE_EN_YEAR_INTERRUPT mode set.
 *  RTC_MODE_MONTH_EVENT_INTERRUPT will generate "event interupt" for YEAR:MON:DAY HH:MM:SS matched.
 *  (RTC_MODE_MONTH_EVENT_INTERRUPT, RTC_MODE_DAY_EVENT_INTERRUPT, RTC_MODE_HOUR_EVENT_INTERRUPT,
 *   RTC_MODE_MIN_EVENT_INTERRUPT and RTC_MODE_SEC_EVENT_INTERRUPT should
 *   NOT set for this option.)
*/

/**
 * \brief           Rtc alarm mode
 */
/* Geneate second interrupt for every new seconds */
#define RTC_MODE_EVERY_SEC_INTERRUPT (1 << 0)
/* Geneate second interrupt only for matched second of every minutes */
#define RTC_MODE_MATCH_SEC_INTERRUPT (0 << 0)
/* Geneate event interrupt for matched second of every munutes */
#define RTC_MODE_SECOND_EVENT_INTERRUPT (1 << 1)
/* Geneate minute interrupt for every new minutes (**:00) */
#define RTC_MODE_EVERY_MIN_INTERRUPT (1 << 2)
/* Geneate minute interrupt only for matched minutes (mm:00) of every hours, mm is the value set in alarm minute */
#define RTC_MODE_MATCH_MIN_INTERRUPT (0 << 2)
/* Geneate event interrupt for matched minutes (mm:ss) of every hours */
#define RTC_MODE_MINUTE_EVENT_INTERRUPT (1 << 3)
/* Geneate hour interrupt for every new hours (**:00:00)  */
#define RTC_MODE_EVERY_HOUR_INTERRUPT (1 << 4)
/* Geneate hour interrupt only for matched hours (hh:00:00) of every days, HH is the value set in alarm hour */
#define RTC_MODE_MATCH_HOUR_INTERRUPT (0 << 4)
/* Geneate event interrupt only for matched hours (hh:mm:ss) of every days  */
#define RTC_MODE_HOUR_EVENT_INTERRUPT (1 << 5)
/* Geneate day interrupt for every new day (00:00:00) */
#define RTC_MODE_EVERY_DAY_INTERRUPT (1 << 6)
/* Geneate day interrupt only for match day of months ( dd 00:00:00) */
#define RTC_MODE_MATCH_DAY_INTERRUPT (0 << 6)
/* Geneate event interrupt for matched day (dd hh:mm:ss) */
#define RTC_MODE_DAY_EVENT_INTERRUPT (1 << 7)
/* Geneate month interrupt for every new month ( **:00 00:00:00) */
#define RTC_MODE_EVERY_MONTH_INTERRUPT (1 << 8)
/* Geneate month interrupt only for matched month of years ( mm:00 00:00:00) */
#define RTC_MODE_MATCH_MONTH_INTERRUPT (0 << 8)
/* Geneate event interrupt for matched month (mm:dd hh:mm:ss) */
#define RTC_MODE_MONTH_EVENT_INTERRUPT (1 << 9)
/* Geneate year interrupt for every new year ( **:00:00 00:00:00) */
#define RTC_MODE_EVERY_YEAR_INTERRUPT (1 << 10)
/* Geneate year interrupt only for matched year of century ( YY:00:00 00:00:00) */
#define RTC_MODE_MATCH_YEAR_INTERRUPT (0 << 10)
/* Geneate event interrupt for matched year:month:day hour:minute:second (yy:mm:dd hh:mm:ss) */
#define RTC_MODE_YEAR_EVENT_INTERRUPT (1 << 11)

/**
 * \brief           RTC alarm mode mask
 */
#define RTC_SEC_MASK   (0x3 << 0)
#define RTC_MIN_MASK   (0x3 << 2)
#define RTC_HOUR_MASK  (0x3 << 4)
#define RTC_DAY_MASK   (0x3 << 6)
#define RTC_MONTH_MASK (0x3 << 8)
#define RTC_YEAR_MASK  (0x3 << 10)

/**
 * \brief           RTC interrupt mask
 */
#define RTC_INTERRUPT_MASK_SHIFT (16)
#define RTC_INTERRUPT_MASK       (0x7F)

/**
 * \brief           RTC interrupt enable
 */
/* Geneate Second interrupt */
#define RTC_MODE_EN_SEC_INTERRUPT (1 << RTC_INTERRUPT_MASK_SHIFT)
/* Geneate Minute interrupt */
#define RTC_MODE_EN_MIN_INTERRUPT (1 << (RTC_INTERRUPT_MASK_SHIFT + 1))
/* Geneate Hour interrupt */
#define RTC_MODE_EN_HOUR_INTERRUPT (1 << (RTC_INTERRUPT_MASK_SHIFT + 2))
/* Geneate Day interrupt */
#define RTC_MODE_EN_DAY_INTERRUPT (1 << (RTC_INTERRUPT_MASK_SHIFT + 3))
/* Geneate Month interrupt */
#define RTC_MODE_EN_MONTH_INTERRUPT (1 << (RTC_INTERRUPT_MASK_SHIFT + 4))
/* Geneate Year interrupt */
#define RTC_MODE_EN_YEAR_INTERRUPT (1 << (RTC_INTERRUPT_MASK_SHIFT + 5))
/* Geneate Event interrupt */
#define RTC_MODE_EVENT_INTERRUPT (1 << (RTC_INTERRUPT_MASK_SHIFT + 6))

/**
 * \brief           RTC interrupt event bit shift
 */
#define RTC_IRQEVENT_SEC_SHIFT   (8)
#define RTC_IRQEVENT_MIN_SHIFT   (6)
#define RTC_IRQEVENT_HOUR_SHIFT  (4)
#define RTC_IRQEVENT_DAY_SHIFT   (2)
#define RTC_IRQEVENT_MONTH_SHIFT (0)
#define RTC_IRQEVENT_YEAR_RSHIFT (2)

/**
 * \brief           RTC interrupt status
 */
/* Indicate second interrupt flag */
#define RTC_INT_SECOND_BIT (1 << 0)
/* Indicate minute interrupt flag */
#define RTC_INT_MINUTE_BIT (1 << 1)
/* Indicate hour interrupt flag */
#define RTC_INT_HOUR_BIT (1 << 2)
/* Indicate day interrupt flag */
#define RTC_INT_DAY_BIT (1 << 3)
/* Indicate month interrupt flag */
#define RTC_INT_MONTH_BIT (1 << 4)
/* Indicate year interrupt flag */
#define RTC_INT_YEAR_BIT (1 << 5)
/* Indicate event interrupt flag */
#define RTC_INT_EVENT_BIT (1 << 6)

/**
 * \brief           rtc_control register clrpls bit
 */
#define RTC_CTRL_CLRPLS (1 << 7)

/**
 * \brief           rtc_load register bit
 */
#define RTC_LOAD_TIME    (1 << 0)
#define RTC_LOAD_ALARM   (1 << 1)
#define RTC_LOAD_DIVISOR (1 << 2)

/**
 * \brief           RTC interrupt service routine callback for user application.
 * \param[in]       rtc_status: the reason of rtc alarm routine trigger
 */
typedef void (*rtc_cb_fn)(uint32_t rtc_status);

/**
 * \brief           RTC timer structure for RTC time value
 */
typedef struct {
    uint32_t tm_sec;                            /*!< rtc second in decimal */
    uint32_t tm_min;                            /*!< rtc minute in decimal */
    uint32_t tm_hour;                           /*!< rtc hour in decimal */
    uint32_t tm_day;                            /*!< rtc day in decimal */
    uint32_t tm_mon;                            /*!< rtc month in decimal */
    uint32_t tm_year;                           /*!< rtc year in decimal */
} rtc_time_t;

/**
 * \brief           Get current rtc time value
 * \param[out]      tm: save time value you want to get
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t rtc_get_time(rtc_time_t* tm);

/**
 * \brief           Set rtc current time value
 * \param[in]       tm: the time value you want to set
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t rtc_set_time(rtc_time_t* tm);

/**
 * \brief           Get rtc alarm  time value
 * \param[out]      tm: save the time alarm value you want to get
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t rtc_get_alarm(rtc_time_t* tm);

/**
 * \brief           Set rtc alarm time value and alarm mode
 * \param[in]       tm: the time value you want to alarm
 * \param[in]       rtc_int_mode: the rtc alarm mode
 * \param[in]       rtc_usr_isr: when rtc interrupt happen it will call rtc_usr_isr to
 *                               notify the interrupt happen
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t rtc_set_alarm(rtc_time_t* tm, uint32_t rtc_int_mode,
                       rtc_cb_fn rtc_usr_isr);

/**
 * \brief           Use to disable rtc alarm
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t rtc_disable_alarm(void);

/**
 * \brief           Use to set rtc ticks for second
 * \param[in]       clk: Set ticks for one second used for RTC counter
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t rtc_set_clk(uint32_t clk);

/**
 * \brief           Use to reset the RTC to default setting
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t rtc_reset(void);

/*@}*/ /* end of RT58X_DRIVER RTC */

#ifdef __cplusplus
}
#endif

#endif /* End of RTC_H */
