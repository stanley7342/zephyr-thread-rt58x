/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           comm_subsystem_drv.h
 * \brief          communication_subsystem.h file
 */
#ifndef __COMM_SUBSYSTEM_DRV_H__
#define __COMM_SUBSYSTEM_DRV_H__

/**
 * \defgroup COMM_SUBSYSTEM_DRV Comm_subsystem
 * \ingroup RT584_DRIVER
 * \brief  Define Comm_subsystem definitions, structures, and functions
 * @{
 */

void Comm_Subsystem_Sram_Deep_Sleep_Init(void);

uint32_t Get_Comm_Subsystem_Rtc_Cnt_Slow(void);

/*@}*/ /* end of RT584_DRIVER COMM_SUBSYSTEM_DRV */

#endif





