/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           syscfunc.h
 * \brief          system function header file
 */
/*
 * This file is part of library_name.
 * Author: 
 */


#ifndef SYSFUN_H
#define SYSFUN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "mcu.h"



/**
 * \defgroup        SYSFUN Sysfun
 * \ingroup         RT584_DRIVER
 * \brief           Define Sysfun definitions, structures, and functions
 * @{
 */


#ifndef SET_PMU_MODE
#define SET_PMU_MODE    PMU_MODE_DCDC
#endif

/**
 * \brief           Irq priority definitions.
 */
typedef enum {
    CHIP_TYPE_581 = 0x01,                       /*!<ic type 581 */
    CHIP_TYPE_582 = 0x02,                       /*!<ic type 582 */
    CHIP_TYPE_583 = 0x03,                       /*!<ic type 583 */
    CHIP_TYPE_584 = 0x04,                       /*!<ic type 584 */
    CHIP_TYPE_UNKNOW = 0xFF,
} chip_type_t;

/**
 * \brief           Irq priority definitions.
 */
typedef enum {
    CHIP_VERSION_SHUTTLE = 0x00,                /*!<ic type SHUTTLE */
    CHIP_VERSION_MPA = 0x01,                    /*!<ic type MPA */
    CHIP_VERSION_MPB = 0x02,                    /*!<ic type MPB */
    CHIP_VERSION_UNKNOW = 0xFF,
} chip_version_t;

/**
 * \brief           hardware chip IC version definitions.
 */
typedef enum
{
    /* ==================== Base 0x00 group (RT584Z series) ==================== */
    IC_VER_RT584Z       = 0x00,     // Wafer B
    IC_VER_RT584ZC      = 0x01,     // Wafer C

    /* ==================== Base 0x40 group (RF1301 series) ==================== */
    IC_VER_RF1301       = 0x10,     // Wafer C
    IC_VER_RF1301_F     = 0x11,     // Wafer F

    /* ==================== Base 0x20 group (RT584H series) ==================== */
    IC_VER_RT584H       = 0x20,     // Wafer E
    IC_VER_RT584HA4     = 0x21,     // Wafer E

    /* ==================== Base 0x30 group (RT584L series) ==================== */
    IC_VER_RT584L       = 0x30,     // Wafer D

    /* ==================== Unknown FT Version ==================== */
    IC_VERSION_UNKNOWN   = 0xFF
} ic_version_t;

/**
 * \brief           Irq priority definitions.
 */
typedef struct __attribute__((packed)) {
    chip_type_t    type;                        /*!<  */
    chip_version_t version;                     /*!<  */
}
chip_model_t;

/**
 * \brief           Irq priority definitions.
 */
typedef struct __attribute__((packed)) {
    uint8_t  buf[8];                            /*!<  */
}
otp_version_t;

/**
 * \brief           Irq priority definitions.
 */
typedef enum {
    IRQ_PRIORITY_HIGHEST = 0,                   /*!<  */
    IRQ_PRIORITY_HIGH = 1,                      /*!<  */
    IRQ_PRIORITY_NORMAL = 3,                    /*!<  */
    IRQ_PRIORITY_LOW = 5,                       /*!<  */
    IRQ_PRIORITY_LOWEST = 7,                    /*!<  */
} irq_priority_t;

/**
 * \brief           Irq priority definitions.
 */
typedef enum {
    PMU_MODE_LDO = 0,                           /*!< System PMU LDO Mode */
    PMU_MODE_DCDC,                              /*!< System PMU DCDC Mode */
} pmu_mode_cfg_t;

/**
 * \brief           Irq priority definitions.
 */
typedef enum {
    TX_POWER_0DBM_DEF = 0x3D,                   /*!< System TX Power 0 DBM Default */
    TX_POWER_10DBM_DEF = 0x7B,                  /*!< System TX Power 10 DBM Default */
    TX_POWER_14DBM_DEF = 0x3E,                  /*!< System TX Power 14 DBM Default */
    TX_POWER_20DBM_DEF = 0x7D,                  /*!< System TX Power 20 DBM Default */
} txpower_default_cfg_t;

/**
 * @brief system slow clock mode
 */
typedef enum
{
    RCO20K_MODE = 0,               /*!< System slow clock 20k Mode */
    RCO32K_MODE,                   /*!< System slow clock 32k Mode */
    RCO16K_MODE,                   /*!< System slow clock 32k Mode */
    RCO1M_MODE,                    /*!< System slow clock 32k Mode */
    RCO_MODE,
} slow_clock_mode_cfg_t;

/**
* \brief            Enter critical sections
* \details          This function is nest function, that is, system call this function several times.
*                   This function will mask all interrupt , except non-mask interrupt.
*                   So as short as possible for using this function.
*/
void enter_critical_section(void);

/**
 * \brief           Leave critical sections
 * \details         Because enter critical sections is nest allowed.
 *                  So it will only unmask all inerrupt when the number "enter_critical_section"
 *                  equals "leave_critical_section" times.
 *                  Please be careful design your code when calling enter_critical_section/leave_critical_section.
 *                  One Enter_Critical_Section must have one corresponding leave_critical_section!
 */
void leave_critical_section(void);

/**
 * \brief           Check hardware chip version and software defined version compared value.
 * \details         version_check is help function to check
 *                  software project setting is the same as hardware IC version.
 *                  If software project define "CHIP_VERSION" is
 *                  not matched with hardware IC version, this functio will return 0, otherwise 1.
 * \return          0 --- hardware and system defined matched.
 *                  1 --- hardware and system defined mis-matched.
 */
uint32_t version_check(void);

/**
 * @brief   check hardware chip IC version.
 * @details
 *           GetOtpICVersion is an API to read chip version and convert into type ic_version_t.
 * @return
 * @retval    the enumeration of ic_version_t or IC_VERSION_UNKNOWN if undetermined.
 */
ic_version_t GetOtpICVersion(void);

/**
* \brief            Set the system PMU mode
* \param[in]        pmu_mode: Specifies the system PMU mode
*                   This parameter can be the following values:
*                       \arg PMU_MODE_LDO: Specifies the system PMU LDO mode
*                       \arg PMU_MODE_DCDC: Specifies the system PMU DCDC mode
*/
void sys_pmu_setmode(pmu_mode_cfg_t pmu_mode);

/**
 * \brief           Get the system PMU mode
 * \return          0 --- LDO MODE
 *                  1 --- DCDC Mode
 * \details         return the system config pmu  mode
 */
pmu_mode_cfg_t sys_pmu_getmode(void);

/**
 * \brief   Get the system slow clock mode
 * \details
 *            return the system slow clock mode
 * \return
 * \retval    0 --- RCO20K MODE
 * \retval    1 --- RCO32K Mode
 * \retval    2 --- RCO16K Mode
 */
slow_clock_mode_cfg_t sys_slow_clk_mode(void);

/**
 * \brief           Get the TX power Default
 * \return          0 --- 14 dbm Default
 *                  1 --- 20 dbm Default
 *                  2 --- 0 dbm Default
 * \details         return the system Tx power Default
 */
txpower_default_cfg_t sys_txpower_getdefault(void);

/**
 * \brief   Set the TX power Default
 * \param[in] txpwrdefault Specifies the sub system tx power level
 *     \arg TX_POWER_20DBM_DEF: Specifies tx power 20dbm
 *     \arg TX_POWER_14DBM_DEF: Specifies tx power 14dbm
 *     \arg TX_POWER_0DBM_DEF: Specifies tx power 0dbm
 * \return NONE
 */
void set_sys_txpower_default(txpower_default_cfg_t txpwrdefault);

/**
 * \brief           System reset
 * \details         Reset the system software by using the watchdog timer to reset the chip.
 */
void sys_software_reset(void);


/**
 * \brief           Get otp version
 */
chip_model_t getotpversion();

/*@}*/ /* end of RT584_DRIVER SYSFUN */

#ifdef __cplusplus
}
#endif

#endif /* End of SYSFUN_H */
