/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


/**************************************************************************//**
 * @file     comm_subsystem_drv.c
 * @version
 * @brief    Use comm_subsystem driver to enable comm_subsystem SRAM auto deep sleep

 ******************************************************************************/


#include "rf_mcu_types.h"

#define COMM_SUBSYSTEN_PMU_MEM_CTRL_ADDR (0x428)
#define REG_LENGTH        (4)

void Commsubsystem_Reg_Ctrl_Init(void)
{
    /* Enable HOST mode */
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST = COMM_SUBSYSTEM_HOST_CTRL_ENABLE_HOST_MODE;

    /* System reset */
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST = COMM_SUBSYSTEM_HOST_CTRL_RESET;

    /* DMA init */
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST = COMM_SUBSYSTEM_HOST_CTRL_WAKE_UP;
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST = COMM_SUBSYSTEM_HOST_CTRL_RESET;
}
void Commsubsystem_Reg_Ctrl(uint8_t reg_rw, uint8_t *p_data)
{
    uint16_t reg_address = COMM_SUBSYSTEN_PMU_MEM_CTRL_ADDR;
    uint16_t data_length = REG_LENGTH;
    uint32_t status = 0;

    /* DMA transfer set and init */
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_AHB_ADDR = (uint32_t)p_data;
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_DMA1 = ((data_length << 16) | ((reg_address >> 2) & 0x3FFF));
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_DMA_TYPE = reg_rw;
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST = COMM_SUBSYSTEM_HOST_CTRL_DMA_ENABLE;

    /* Polling DMA interrupt status until it is not pending */
    do
    {
        status = COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INTR_STATUS;
    } while ((status & COMM_SUBSYSTEM_DMA_INT_CLR) == 0);

    /* DMA interrupt status clear */
    COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INTR_CLR = COMM_SUBSYSTEM_DMA_INT_CLR;
}

void Comm_Subsystem_Sram_Deep_Sleep_Init(void)
{
    uint32_t status;
    uint8_t reg_val[4];

    /* Enable dma and system reset */
    Commsubsystem_Reg_Ctrl_Init();

    do
    {
        status = COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO;
    } while (((status & COMM_SUBSYSTEM_SYS_RDY) != 1));

    /* Enable communication subsystem SRAM auto deep-sleep control */
    Commsubsystem_Reg_Ctrl(COMM_SUBSYSTEM_DMA_TYPE_MEM_READ, reg_val);
    reg_val[1] |= (1 << 3);
    Commsubsystem_Reg_Ctrl(COMM_SUBSYSTEM_DMA_TYPE_MEM_WRITE, reg_val);
}
