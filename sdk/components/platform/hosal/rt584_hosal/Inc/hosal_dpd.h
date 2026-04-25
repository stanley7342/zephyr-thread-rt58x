/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_dpd.h
 * \brief           Hosal deep power down header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_DPD_H
#define HOSAL_DPD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"
#include "dpd.h"



/**
 * \defgroup        HOSAL_DPD Hosal dpd
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal dpd definitions, structures, and functions
 * @{
 */

/**
 * \brief           Cause of the reset
 */
#define HOSAL_RESET_BY_POWER_ON                           RESET_BY_POWER_ON
#define HOSAL_RESET_BY_EXT_RESET                          RESET_BY_EXT_RESET
#define HOSAL_RESET_BY_DEEP_POWER_DOWN                    RESET_BY_DEEP_POWER_DOWN
#define HOSAL_RESET_BY_DEEP_SLEEP                         RESET_BY_DEEP_SLEEP
#define HOSAL_RESET_BY_WATCH_DOG                          RESET_BY_WATCH_DOG
#define HOSAL_RESET_BY_SOFT_RESET                         RESET_BY_SOFT_RESET
#define HOSAL_RESET_BY_MCU_LOCKUP                         RESET_BY_MCU_LOCKUP



/**
 * \brief           Get reset all cause.
 * \param[out]      reset_cause: reset cause
 */
__STATIC_INLINE void hosal_get_all_reset_cause(uint32_t *reset_cause) {
    get_all_reset_cause(reset_cause);
}

/**
 * \brief           Reset by power on or not.
 * \param[out]      by_power_on:
 *                  0: reset not by power on, 
 *                  1: reset by power on
 */
__STATIC_INLINE void hosal_reset_by_power_on(uint32_t *by_power_on) {
    reset_by_power_on(by_power_on);
}

/**
 * \brief           Reset by external reset or not.
 * \param[out]      by_ext_rst:
 *                  0: reset not by external reset, 
 *                  1: reset by external reset
 */
__STATIC_INLINE void hosal_reset_by_external(uint32_t *by_ext_rst) {
    reset_by_external(by_ext_rst);
}

/**
 * \brief           Reset by deep power down or not.
 * \param[out]      by_deep_power_down:
 *                  0: reset not by deep power down, 
 *                  1: reset by deep power down
 */
__STATIC_INLINE void hosal_reset_by_deep_power_down(uint32_t *by_deep_power_down) {
    reset_by_deep_power_down(by_deep_power_down);
}

/**
 * \brief           Reset by deep sleep or not.
 * \param[out]      by_deep_sleep:
 *                  0: reset not by deep sleep, 
 *                  1: reset by deep sleep
 */
__STATIC_INLINE void hosal_reset_by_deep_sleep(uint32_t *by_deep_sleep) {
    reset_by_deep_sleep(by_deep_sleep);
}

/**
 * \brief           Reset by WDT or not.
 * \param[out]      by_wdt:
 *                  0: reset not by WDT, 
 *                  1: reset by WDT
 */
__STATIC_INLINE void hosal_reset_by_wdt(uint32_t *by_wdt) {
    reset_by_wdt(by_wdt);
}

/**
 * \brief           Reset by software or not.
 * \param[out]      by_soft_rst:
 *                  0: reset not by software, 
 *                  1: reset by software
 */
__STATIC_INLINE void hosal_reset_by_software(uint32_t *by_soft_rst) {
    reset_by_software(by_soft_rst);
}

/**
 * \brief           Reset by mcu lockup or not.
 * \param[out]      by_lock:
 *                  0: reset not by mcu lockup, 
 *                  1: reset by mcu lockup
 */
__STATIC_INLINE void hosal_reset_by_lock(uint32_t *by_lock) {
    reset_by_lock(by_lock);
}

/**
 * \brief           Clear reset cause.
 */
__STATIC_INLINE void hosal_clear_reset_cause(void) {
    clear_reset_cause();
}

/*@}*/ /* end of RT584_HOSAL HOSAL_DPD */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_DPD_H */
