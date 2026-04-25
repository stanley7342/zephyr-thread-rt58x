/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_lpm.c
 * \brief           hosal low power mode driver file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#include <stdint.h>
#include "hosal_lpm.h"
#include "lpm.h"
#include "mcu.h"
#include "stdio.h"
#include "hosal_status.h"
#include "comm_subsystem_drv.h"

int hosal_lpm_init(void) {
    lpm_init();
    return HOSAL_STATUS_SUCCESS;
}

int hosal_get_lpm_ioctrl(int ctl, uint32_t* para) {

    int status = HOSAL_STATUS_SUCCESS;

    switch (ctl) {
        case HOSAL_LPM_GET_MASK:
            para[0] = lpm_get_low_power_mask_status();
            break;
        case HOSAL_LPM_GET_POWER_LEVEL:
            para[0] = lpm_get_low_power_level();
            break;            
        default: return -1;
    }

    return HOSAL_STATUS_SUCCESS;
}

int hosal_lpm_ioctrl(int ctl, uint32_t para) {

    int status = HOSAL_STATUS_SUCCESS;

    switch (ctl) {

        case HOSAL_LPM_MASK: lpm_low_power_mask((uint32_t)para); break;

        case HOSAL_LPM_SUBSYSTEM_MASK:
            lpm_comm_subsystem_wakeup_mask((uint32_t)para);
            break;

        case HOSAL_LPM_UNMASK: lpm_low_power_unmask((uint32_t)para); break;

        case HOSAL_LPM_SUBSYSTEM_UNMASK:
            lpm_comm_subsystem_wakeup_unmask((uint32_t)para);
            break;

        case HOSAL_LPM_SRAM0_RETAIN:
            lpm_set_sram_sleep_deepsleep_shutdown((uint32_t)para);
            break;

        case HOSAL_LPM_SET_POWER_LEVEL: lpm_set_low_power_level((uint32_t)para); break;
        case HOSAL_LPM_ENABLE_WAKE_UP_SOURCE:
            lpm_enable_low_power_wakeup((uint32_t)para);
            break;

        case HOSAL_LPM_DISABLE_WAKE_UP_SOURCE:
            lpm_disable_low_power_wakeup((uint32_t)para);
            break;

        case HOSAL_LPM_ENTER_LOW_POWER: lpm_enter_low_power_mode(); break;

        case HOSAL_LPM_SUBSYSTEM_SRAM_DEEP_SLEEP_INIT:
            Comm_Subsystem_Sram_Deep_Sleep_Init();
            break;    

        case HOSAL_LPM_SUBSYSTEM_ENTER_LOW_POWER:
            lpm_sub_system_low_power_mode(para);
            break;   
           
        default: return -1;
    }

    return HOSAL_STATUS_SUCCESS;
}
