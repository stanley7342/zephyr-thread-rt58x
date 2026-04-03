/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            dpd.h
 * \brief           Deep power down header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef DPD_H
#define DPD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "mcu.h"


/**
 * \defgroup        DPD Dpd
 * \ingroup         RT58x_DRIVER
 * \brief           Define Dpd definitions, structures, and functions
 *                  Use retension register 5 to known reset cause
 * @{
 */


 /**
 * \brief           Cause of the reset
 */
#define RESET_BY_POWER_ON                           0x00
#define RESET_BY_EXT_RESET                          0x00
#define RESET_BY_DEEP_POWER_DOWN                    0x00
#define RESET_BY_DEEP_SLEEP                         0x00
#define RESET_BY_WATCH_DOG                          0x10
#define RESET_BY_SOFT_RESET                         0x00
#define RESET_BY_MCU_LOCKUP                         0x00


/**
 * \brief           Record reset cause enable or not
 */
extern uint32_t record_reset_cause_en;

/**
 * \brief           Get reset all cause.
 * \return          get cause register value
 */
__STATIC_INLINE uint32_t get_all_reset_cause(uint32_t *value) {
    record_reset_cause_en = 1;
    *value = inp32((SYSCTRL_BASE + SYS_SCRATCH_OFFSET) + (5 << 2));
}

/**
 * \brief           Reset by WDT or not.
 * \param[out]      by_wdt:
 *                  0: reset not by WDT, 
 *                  1: reset by WDT
 */
__STATIC_INLINE void reset_by_wdt(uint32_t *by_wdt) {
    uint32_t value;
    value = (inp32((SYSCTRL_BASE + SYS_SCRATCH_OFFSET) + (5 << 2)));
    *by_wdt = value & RESET_BY_WATCH_DOG;
}

/**
 * \brief           Clear reset cause.
 */
__STATIC_INLINE void clear_reset_cause(void) {
    outp32(((SYSCTRL_BASE + SYS_SCRATCH_OFFSET) + (5 << 2)), 0);
}

/**
 * \brief           Before WDT reset, set up cause.
 */
__STATIC_INLINE void set_wdt_reset_cause(void) {
    outp32(((SYSCTRL_BASE + SYS_SCRATCH_OFFSET) + (5 << 2)), RESET_BY_WATCH_DOG);
}


/*@}*/ /* end of RT58X_DRIVER DPD */

#ifdef __cplusplus
}
#endif

#endif /* End of DPD_H */
