/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file    sysfun.h        
 * \brief   system function header file
 */
/*
 * Author:        
 */

#ifndef SYSFUN_H
#define SYSFUN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

/**
 * \defgroup SYSFUN Sysfun
 * \ingroup RT58X_DRIVER
 * \brief  Define Sysfun definitions, structures, and functions
 * @{
 */


#define OTP_IC_VERSION_OFFSET 0x00
#define OTP_FT_VERSION_OFFSET 0x04

/**
 * \brief           Chip model version definitions.
 */
typedef enum {
    CHIP_TYPE_581 = 0x01,                       /*!< ic type 581 */
    CHIP_TYPE_582 = 0x02,                       /*!< ic type 582 */
    CHIP_TYPE_583 = 0x03,                       /*!< ic type 583 */
    CHIP_TYPE_584 = 0x04,                       /*!< ic type 584 */
    CHIP_TYPE_UNKNOW = 0xFF,                    /*!< unknow ic type */
} chip_type_t;

/**
 * \brief           Chip type definitions.
 */
typedef enum {
    CHIP_VERSION_SHUTTLE = 0x00,                /*!< ic type SHUTTLE */
    CHIP_VERSION_MPA = 0x01,                    /*!< ic type MPA */
    CHIP_VERSION_MPB = 0x02,                    /*!< ic type MPB */
    CHIP_VERSION_UNKNOW = 0xFF,                 /*!< unknow ic type */
} chip_version_t;

typedef enum {
    IRQ_PRIORITY_HIGHEST = 0,
    IRQ_PRIORITY_HIGH = 1,
    IRQ_PRIORITY_NORMAL = 3,
    IRQ_PRIORITY_LOW = 5,
    IRQ_PRIORITY_LOWEST = 7,
} irq_priority_t;

/**
 * \brief           Chip model data structure definitions.
 */
typedef struct __attribute__((packed)) {
    chip_type_t type;
    chip_version_t version;
} chip_model_t;

/**
 * \brief           PMU mode
 */
typedef enum {
    PMU_MODE_LDO = 0,                           /*!< System PMU LDO Mode */
    PMU_MODE_DCDC,                              /*!< System PMU DCDC Mode */
} pmu_mode_cfg_t;

/**
 * \brief           Otp version buffer
 */
typedef struct __attribute__((packed)) {
    uint8_t buf[8];

} otp_version_t;

/**
 * \brief           Get chip mode
 * \details
 *                  Get otp version chip_define.h
 * \retval          return chip_mode_t.
 */
chip_model_t getotpversion(void);

/**
* \brief            sys_software_reset
* \details          The function waits until the flash operation is complete and then resets the system.
*/
void sys_software_reset(void);

/**
* \brief            Critical section init
* \details          This function clear critical_section_counter;
*/
void critical_section_init(void);

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
 *                  One enter_critical_section must have one corresponding leave_critical_section!
 */
void leave_critical_section(void);

/**
 * \brief           Check hardware chip version and software defined version compared value.
 * \details
 *                  version_check is help function to check
 *                  software project setting is the same as hardware IC version.
 *                  If software project define "CHIP_VERSION" is
 *                  not matched with hardware IC version, this functio will return 0, otherwise 1.
 * \return
 * \retval          0 --- hardware and system defined matched.
 * \retval          1 --- hardware and system defined mis-matched.
 */

uint32_t version_check(void);

/**
 * \brief           Get pmu dcdc or ldo mode
 */
pmu_mode_cfg_t GetPmuMode(void);

/*@}*/ /* end of RT58X_DRIVER SYSFUN */

#ifdef __cplusplus
}
#endif

#endif /* End of SYSFUN_H */
