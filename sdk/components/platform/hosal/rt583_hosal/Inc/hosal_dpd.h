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

#include "dpd.h"



/**
 * \defgroup        HOSAL_DPD Hosal dpd
 * \ingroup         RT58X_HOSAL
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
 * \brief           Reset by WDT or not.
 * \param[out]      by_wdt:
 *                  0: reset not by WDT, 
 *                  1: reset by WDT
 */
__STATIC_INLINE void hosal_reset_by_wdt(uint32_t *by_wdt) {
    reset_by_wdt(by_wdt);
}

/**
 * \brief           Clear reset cause.
 */
__STATIC_INLINE void hosal_clear_reset_cause(void) {
    clear_reset_cause();
}

/*@}*/ /* end of RT58X_HOSAL HOSAL_DPD */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_DPD_H */
