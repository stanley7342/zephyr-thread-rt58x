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
 * \ingroup         RT584_DRIVER
 * \brief           Define Dpd definitions, structures, and functions
 * @{
 */

#define CLEAR_RESET_CAUSE           (1)
#define DPD_GPIO_LATCH_ENABLE       (1<<16)
#define FLASH_DPD_ENABLE            (1<<31)

#define  DPD_RET3_SKIP_ISP                          (0x01)      /*<! Retention Register 3. SKIP ISP */
#define  DPD_RET3_DS_FAST_BOOT                      (0x04)      /*<! Retention Register 3. Deep sleep wakeup*/
#define  DPD_CMD_LATCH_ENABLE                       (1<<16)     /*<! Retention Register 3. latch enable*/
/* This is software define setting, in DeepSleep mode, RCO32K is ON or OFF */
#define  DS_WAKEUP_MASK                             (1<<5)
#define  DS_WAKEUP_LOW                              (0)
#define  DS_WAKEUP_HIGH                             (1<<5)

#define  DS_RCO32K_OFF                              (0)
#define  DS_RCO32K_ON                               (1<<6)
#define  DS_RCO32K_MASK                             (1<<6)

/* This setting only used for Deep-power down mode. */
#define  DPD_GPIO_LATCH_MASK                        (1)
#define  DPD_GPIO_NO_LATCH                          (0)

/**
 * \brief           Cause of the reset
 */
#define RESET_BY_POWER_ON                           0x01
#define RESET_BY_EXT_RESET                          0x02
#define RESET_BY_DEEP_POWER_DOWN                    0x04
#define RESET_BY_DEEP_SLEEP                         0x08
#define RESET_BY_WATCH_DOG                          0x10
#define RESET_BY_SOFT_RESET                         0x20
#define RESET_BY_MCU_LOCKUP                         0x40

/**
 * \brief           Get reset all cause.
 * \param[out]      reset_cause: reset cause
 */
__STATIC_INLINE void get_all_reset_cause(uint32_t *reset_cause) {
    *reset_cause = (DPD_CTRL->dpd_rst_cause.reg);
}

/**
 * \brief           Reset by power on or not.
 * \param[out]      by_power_on:
 *                  0: reset not by power on, 
 *                  1: reset by power on
 */
__STATIC_INLINE void reset_by_power_on(uint32_t *by_power_on) {
    *by_power_on = (DPD_CTRL->dpd_rst_cause.bit.rst_cause_por);
}

/**
 * \brief           Reset by external reset or not.
 * \param[out]      by_ext_rst:
 *                  0: reset not by external reset, 
 *                  1: reset by external reset
 */
__STATIC_INLINE void reset_by_external(uint32_t *by_ext_rst) {
    *by_ext_rst = (DPD_CTRL->dpd_rst_cause.bit.rst_cause_ext);
}

/**
 * \brief           Reset by deep power down or not.
 * \param[out]      by_deep_power_down:
 *                  0: reset not by deep power down, 
 *                  1: reset by deep power down
 */
__STATIC_INLINE void reset_by_deep_power_down(uint32_t *by_deep_power_down) {
    *by_deep_power_down = (DPD_CTRL->dpd_rst_cause.bit.rst_cause_dpd);
}

/**
 * \brief           Reset by deep sleep or not.
 * \param[out]      by_deep_sleep:
 *                  0: reset not by deep sleep, 
 *                  1: reset by deep sleep
 */
__STATIC_INLINE void reset_by_deep_sleep(uint32_t *by_deep_sleep) {
    *by_deep_sleep = (DPD_CTRL->dpd_rst_cause.bit.rst_cause_ds);
}

/**
 * \brief           Reset by WDT or not.
 * \param[out]      by_wdt:
 *                  0: reset not by WDT, 
 *                  1: reset by WDT
 */
__STATIC_INLINE void reset_by_wdt(uint32_t *by_wdt) {
    *by_wdt = (DPD_CTRL->dpd_rst_cause.bit.rst_cause_wdt);
}

/**
 * \brief           Reset by software or not.
 * \param[out]      by_soft_rst:
 *                  0: reset not by software, 
 *                  1: reset by software
 */
__STATIC_INLINE void reset_by_software(uint32_t *by_soft_rst) {
    *by_soft_rst = (DPD_CTRL->dpd_rst_cause.bit.rst_cause_soft);
}

/**
 * \brief           Reset by mcu lockup or not.
 * \param[out]      by_lock:
 *                  0: reset not by mcu lockup, 
 *                  1: reset by mcu lockup
 */
__STATIC_INLINE void reset_by_lock(uint32_t *by_lock) {
    *by_lock = (DPD_CTRL->dpd_rst_cause.bit.rst_cause_lock);
}

/**
 * \brief           Clear reset cause.
 */
__STATIC_INLINE void clear_reset_cause(void) {
    DPD_CTRL->dpd_cmd.bit.clr_rst_cause = 1;
}

/**
 * \brief           Set retention reg 0.
 * \param           value: value save at retention reg 0
 */
__STATIC_INLINE void set_retention_reg0(uint32_t value) {
    DPD_CTRL->dpd_ret0_reg = value;
}

/**
 * \brief           Get retention reg 0.
 * \param           value: the address for return value.
 */
__STATIC_INLINE void get_retention_reg0(uint32_t *value) {
    *value =  DPD_CTRL->dpd_ret0_reg;
}

/**
 * \brief           Set retention reg 1.
 * \param           value: value save at retention reg 1
 */
__STATIC_INLINE void set_retention_reg1(uint32_t value) {
    DPD_CTRL->dpd_ret1_reg = value;
}

/**
 * \brief           Get retention reg 1.
 * \param           value: the address for return value.
 */
__STATIC_INLINE void get_retention_reg1(uint32_t *value) {
    *value =  DPD_CTRL->dpd_ret1_reg;
}

/**
 * \brief           Set retention reg 2.
 * \param           value: value save at retention reg 2
 */
__STATIC_INLINE void set_retention_reg2(uint32_t value) {
    DPD_CTRL->dpd_ret2_reg = value;
}

/**
 * \brief           Get retention reg 2.
 * \param           value: the address for return value.
 */
__STATIC_INLINE void get_Retention_reg2(uint32_t *value) {
    *value =  DPD_CTRL->dpd_ret2_reg;
}


/**
 * \brief           Set Deep slee wake up fast boot
 */
__STATIC_INLINE void dpd_set_deepsleep_wakeup_fast_boot(void) {
    DPD_CTRL->dpd_ret3_reg |= (DPD_RET3_SKIP_ISP | DPD_RET3_DS_FAST_BOOT);
}

/**
 * \brief           Clear dpd latch enable
 */
__STATIC_INLINE void dpd_clear_latch(void) {
    DPD_CTRL->dpd_cmd.reg &= ~(DPD_CMD_LATCH_ENABLE);
}

/**
 * \brief           Clear dpd latch enable
 */
__STATIC_INLINE void dpd_flash_enable(void) {
    DPD_CTRL->dpd_cmd.reg |= (FLASH_DPD_ENABLE);
}

/*@}*/ /* end of RT584_DRIVER DPD */

#ifdef __cplusplus
}
#endif

#endif /* End of DPD_H */
