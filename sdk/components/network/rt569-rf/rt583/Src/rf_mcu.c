/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


/**************************************************************************//**
 * @file     rf_mcu.c
 * @version
 * @brief    API layer of Communication subsystem

 ******************************************************************************/


#include "mcu.h"
#include "rf_mcu_chip.h"
#include "rf_mcu.h"
#include <zephyr/sys/printk.h>
#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_AHB)
#include "rf_mcu_ahb.h"
#elif (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
#include "rf_mcu_spi.h"
#else
#error "CFG_RF_MCU_CTRL_TYPE not supported!"
#endif
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
#include "rt569mp_init.h"
#elif (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)
#if (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_A)
#include "rt569mxa_init.h"
#elif (RF_MCU_CHIP_VER == RF_MCU_CHIP_VER_B)
#if (RF_MCU_FW_TARGET == RF_MCU_FW_TARGET_MAC)
#include "rt569mxb_init.h"
#elif (RF_MCU_FW_TARGET == RF_MCU_FW_TARGET_BLE)
#include "rt569mxb_ble_init.h"
#endif
#endif
#endif
#include "stdio.h"

#define RF_MCU_MEM_CHECK_SIZE                    (8)

volatile COMM_SUBSYSTEM_ISR_CONFIG gRfMcuIsrCfg;
#if (RF_MCU_PATCH_SUPPORTED)
RF_MCU_INIT_STATUS RfMcu_DefaultEntry(const uint8_t *patch_addr, uint32_t patch_length);
const uint8_t *pPatchAddr = NULL;
uint32_t uiPatchLength = 0;
RF_MCU_PATCH_ENTRY vPatchEntry = RfMcu_DefaultEntry;
#endif

#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
const uint8_t *pRfMcuConstAddr = NULL;
uint32_t uiRfMcuConstLength = 0;
#endif


typedef struct __attribute__((packed)) hybrid_inst_cfg_ctrl_s
{
    uint16_t cfg_sfr;
    uint8_t cfg_val[4];
} hybrid_inst_cfg_ctrl_t;
#define HYBRID_INST_CFG_NO     (sizeof(HYBRID_INST_MEM_CFG_LIST)/sizeof(HYBRID_INST_MEM_CFG_LIST[0]))


#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_AHB)

void RfMcu_MemorySet(uint16_t sys_addr, const uint8_t *p_data, uint16_t data_length)
{
    RfMcu_MemorySetAhb(sys_addr, p_data, data_length);
}


void RfMcu_MemoryGet(uint16_t sys_addr, uint8_t *p_data, uint16_t data_length)
{
    RfMcu_MemoryGetAhb(sys_addr, p_data, data_length);
}


void RfMcu_IoSet(uint8_t queue_id, const uint8_t *p_data, uint16_t data_length)
{
    RfMcu_IoSetAhb(queue_id, p_data, data_length);
}


void RfMcu_IoGet(uint16_t queue_id, uint8_t *p_data, uint16_t data_length)
{
    RfMcu_IoGetAhb(queue_id, p_data, data_length);
}


void RfMcu_HostCmdSet(uint8_t cmd)
{
    RfMcu_HostCmdSetAhb(cmd);
}


void RfMcu_HostWakeUpMcu(void)
{
    RfMcu_HostWakeUpMcuAhb();
}


void RfMcu_HostCtrl(uint32_t ctrl)
{
    RfMcu_HostCtrlAhb(ctrl);
}


void RfMcu_HostModeEnable(void)
{
    RfMcu_HostCtrl(COMM_SUBSYSTEM_HOST_CTRL_ENABLE_HOST_MODE);
}


void RfMcu_HostModeDisable(void)
{
    RfMcu_HostCtrl(COMM_SUBSYSTEM_HOST_CTRL_DISABLE_HOST_MODE);
}


void RfMcu_HostResetMcu(void)
{
    RfMcu_HostCtrl(COMM_SUBSYSTEM_HOST_CTRL_RESET);
}

void RfMcu_ChipIdCheck(void)
{
    /* In AHB interface, there's no chip ID information */
}

void RfMcu_InterruptDisableAll(void)
{
    RfMcu_InterruptDisableAhb();
}


void RfMcu_InterruptEnableAll(void)
{
    RfMcu_InterruptEnableAhb();
}


uint16_t RfMcu_InterruptEnGet(void)
{
    return RfMcu_InterruptEnGetAhb();
}


void RfMcu_InterruptEnSet(uint16_t int_enable)
{
    RfMcu_InterruptEnSetAhb(int_enable);
}


void RfMcu_InterruptClear(uint32_t value)
{
    RfMcu_InterruptClearAhb(value);
}


bool RfMcu_RxQueueCheck(void)
{
    return RfMcu_RxQueueIsReadyAhb();
}


uint16_t RfMcu_RxQueueRead(uint8_t *rx_data, RF_MCU_RXQ_ERROR *rx_queue_error)
{
    return RfMcu_RxQueueReadAhb(rx_data, rx_queue_error);
}


bool RfMcu_EvtQueueCheck(void)
{
    return RfMcu_EvtQueueIsReadyAhb();
}


uint16_t RfMcu_EvtQueueRead(uint8_t *evt, RF_MCU_RX_CMDQ_ERROR *rx_evt_error)
{
    return RfMcu_EvtQueueReadAhb(evt, rx_evt_error);
}


bool RfMcu_CmdQueueFullCheck(void)
{
    return RfMcu_CmdQueueFullCheckAhb();
}


RF_MCU_TX_CMDQ_ERROR RfMcu_CmdQueueSend(const uint8_t *cmd, uint32_t cmd_length)
{
    return RfMcu_CmdQueueSendAhb(cmd, cmd_length);
}


bool RfMcu_TxQueueFullCheck(void)
{
    return RfMcu_TxQueueFullCheckAhb();
}

uint8_t RfMcu_TxQueueGet(void)
{
    return RfMcu_TxQueueGetAhb();
}

RF_MCU_TXQ_ERROR RfMcu_TxQueueSendById(uint8_t queue_id, const uint8_t *tx_data, uint32_t data_length)
{
    return RfMcu_TxQueueSendAhb(queue_id, tx_data, data_length);
}


RF_MCU_STATE RfMcu_McuStateRead(void)
{
    return RfMcu_McuStateReadAhb();
}


void RfMcu_SysRdySignalWait(void)
{
    RfMcu_SysRdySignalWaitAhb();
}


uint8_t RfMcu_PowerStateCheck(void)
{
    RF_MCU_PWR_STATE power_state = RfMcu_PowerStateGetAhb();

    return (uint8_t)(power_state);
}


void RfMcu_DmaInit(void)
{
    /* AHB DmaInit: enable NVIC and RF MCU DMA interrupt only.
     * No RESET here — matches reference rt584 design.
     * The RESET+SysRdyWait is done in RfMcu_SysInit() after HostModeEnable. */
    NVIC_EnableIRQ(CommSubsystem_IRQn);
    RfMcu_InterruptEnSetAhb(COMM_SUBSYSTEM_DMA_INT_ENABLE);
}


void RfMcu_SpiSfrSet(uint8_t sfrOffset, uint8_t value)
{
    return;
}


uint8_t RfMcu_SpiSfrGet(uint8_t sfrOffset)
{
    return 0;
}


void RfMcu_IsrHandler(void)
{
    RfMcu_AhbIsrHandler(gRfMcuIsrCfg.commsubsystem_isr);
}

void RfMcu_IceModeCtrl(bool enable)
{
    /* TO DO or not used*/
}

void RFMcu_LoadExtMemInit(void)
{
    /* In AHB Case, it should not implement now */
}

#elif (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)

void RfMcu_MemorySet(uint16_t sys_addr, const uint8_t *p_data, uint16_t data_length)
{
    RfMcu_MemorySetSpi(sys_addr, p_data, data_length);
}


void RfMcu_MemoryGet(uint16_t sys_addr, uint8_t *p_data, uint16_t data_length)
{
    RfMcu_MemoryGetSpi(sys_addr, p_data, data_length);
}


void RfMcu_IoSet(uint8_t queue_id, const uint8_t *p_data, uint16_t data_length)
{
    RfMcu_IoSetSpi(queue_id, p_data, data_length);
}


void RfMcu_IoGet(uint16_t queue_id, uint8_t *p_data, uint16_t data_length)
{
    RfMcu_IoGetSpi(queue_id, p_data, data_length);
}


void RfMcu_HostCmdSet(uint8_t cmd)
{
    RfMcu_HostCmdSetSpi(cmd);
}


void RfMcu_HostWakeUpMcu(void)
{
    RfMcu_HostWakeUpMcuSpi();
}


void RfMcu_HostCtrl(uint32_t ctrl)
{
    /* No used */
}


void RfMcu_HostModeEnable(void)
{
    uint8_t value = RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_MODE_SELECT);
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_MODE_SELECT, (value | RF_MCU_SPI_HOST_MODE_ENABLE_BIT));
}


void RfMcu_HostModeDisable(void)
{
    uint8_t value = RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_MODE_SELECT);
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_MODE_SELECT, (value & ~RF_MCU_SPI_HOST_MODE_ENABLE_BIT));
}


void RfMcu_HostResetMcu(void)
{
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_HOST_CTRL, RF_MCU_SPI_HOST_CTRL_RESET_BIT);

}

void RfMcu_ChipIdCheck(void)
{
    while (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_CHIP_ID) != RF_MCU_CHIP_ID);
}

void RfMcu_InterruptDisableAll(void)
{
    RfMcu_InterruptDisableSpi();
}


void RfMcu_InterruptEnableAll(void)
{
    RfMcu_InterruptEnableSpi();
}


uint16_t RfMcu_InterruptEnGet(void)
{
    return RfMcu_InterruptEnGetSpi();
}


void RfMcu_InterruptEnSet(uint16_t int_enable)
{
    RfMcu_InterruptEnSetSpi(int_enable);
}


void RfMcu_InterruptClear(uint32_t value)
{
    RfMcu_InterruptClearSpi(value);
}


bool RfMcu_RxQueueCheck(void)
{
    return RfMcu_RxQueueIsReadySpi();
}


uint16_t RfMcu_RxQueueRead(uint8_t *rx_data, RF_MCU_RXQ_ERROR *rx_queue_error)
{
    return RfMcu_RxQueueReadSpi(rx_data, rx_queue_error);
}


bool RfMcu_EvtQueueCheck(void)
{
    return RfMcu_EvtQueueIsReadySpi();
}


uint16_t RfMcu_EvtQueueRead(uint8_t *evt, RF_MCU_RX_CMDQ_ERROR *rx_evt_error)
{
    return RfMcu_EvtQueueReadSpi(evt, rx_evt_error);
}


bool RfMcu_CmdQueueFullCheck(void)
{
    return RfMcu_CmdQueueFullCheckSpi();
}


RF_MCU_TX_CMDQ_ERROR RfMcu_CmdQueueSend(const uint8_t *cmd, uint32_t cmd_length)
{
    return RfMcu_CmdQueueSendSpi(cmd, cmd_length);
}


bool RfMcu_TxQueueFullCheck(void)
{
    return RfMcu_TxQueueFullCheckSpi();
}

uint8_t RfMcu_TxQueueGet(void)
{
    return RfMcu_TxQueueGetSpi();
}

RF_MCU_TXQ_ERROR RfMcu_TxQueueSendById(uint8_t queue_id, const uint8_t *tx_data, uint32_t data_length)
{
    return RfMcu_TxQueueSendSpi(queue_id, tx_data, data_length);
}


RF_MCU_STATE RfMcu_McuStateRead(void)
{
    return RfMcu_McuStateReadSpi();
}


void RfMcu_SysRdySignalWait(void)
{
    RfMcu_SysRdySignalWaitSpi();
}


uint8_t RfMcu_PowerStateCheck(void)
{
    RF_MCU_PWR_STATE power_state = RfMcu_PowerStateGetSpi();

    return (uint8_t)(power_state);
}

void RfMcu_GpioIsrHandler(uint32_t pin, void *isr_param)
{
    if (RF_MCU_SPI_GPIO_ISR_SELECT == pin)
    {
        RfMcu_SpiIsrHandler(gRfMcuIsrCfg.commsubsystem_isr);
    }
}

void RfMcu_DmaInit(void)
{
    RfMcu_SpiInit();

    pin_set_mode(RF_MCU_SPI_GPIO_ISR_SELECT, MODE_GPIO);
    gpio_cfg_input(RF_MCU_SPI_GPIO_ISR_SELECT, GPIO_PIN_INT_LEVEL_LOW);
    gpio_register_isr(RF_MCU_SPI_GPIO_ISR_SELECT, RfMcu_GpioIsrHandler, NULL);
    gpio_int_enable(RF_MCU_SPI_GPIO_ISR_SELECT);
}


void RfMcu_SpiSfrSet(uint8_t sfrOffset, uint8_t value)
{
    RfMcu_SpiSetSFR(sfrOffset, value);
}


uint8_t RfMcu_SpiSfrGet(uint8_t sfrOffset)
{
    return RfMcu_SpiGetSFR(sfrOffset);
}


void RfMcu_IsrHandler(void)
{
    /* RfMcu_SpiIsrHandler(gRfMcuIsrCfg.commsubsystem_isr); In SPI Case, it would not receive ISR from Communication_subsystem_ISR */
    BREAK();
}

void RfMcu_IceModeCtrl(bool enable)
{
    RfMcu_SpiIceModeCtrl(enable);
}

RF_MCU_INIT_STATUS RfMcu_CheckFwByByte(uint32_t start_addr, const uint8_t *fw_image, uint32_t image_size)
{
    uint32_t count;
    uint8_t check_count, buff[RF_MCU_MEM_CHECK_SIZE];
    for (count = 0; count < (image_size / RF_MCU_MEM_CHECK_SIZE) ; count++)
    {
        RfMcu_MemoryGet(COMM_SUBSYS_PROGRAM_START_ADDR + count * RF_MCU_MEM_CHECK_SIZE, buff, RF_MCU_MEM_CHECK_SIZE);
        for (check_count = 0 ; check_count < RF_MCU_MEM_CHECK_SIZE ; check_count++)
        {
            if (buff[check_count] != fw_image[start_addr + count * RF_MCU_MEM_CHECK_SIZE + check_count])
            {
                return RF_MCU_FIRMWARE_LOADING_FAIL;
            }
        }
    }
    return RF_MCU_INIT_NO_ERROR;
}


RF_MCU_INIT_STATUS RfMcu_CheckPatchByByte(const uint8_t *p_patch_image, uint32_t patch_image_size)
{
    uint32_t count;
    uint8_t check_count, buff[RF_MCU_MEM_CHECK_SIZE];

    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_1);

    for (count = 0; count < (patch_image_size / RF_MCU_MEM_CHECK_SIZE) ; count++)
    {
        RfMcu_MemoryGet(RF_MCU_PATCH_START_ADDR + count * RF_MCU_MEM_CHECK_SIZE, buff, RF_MCU_MEM_CHECK_SIZE);
        for (check_count = 0 ; check_count < RF_MCU_MEM_CHECK_SIZE ; check_count++)
        {
            if (buff[check_count] != p_patch_image[count * RF_MCU_MEM_CHECK_SIZE + check_count])
            {
                RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);
                return RF_MCU_PATCH_LOADING_FAIL;
            }
        }
    }

    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);

    return RF_MCU_INIT_NO_ERROR;
}


RF_MCU_INIT_STATUS RfMcu_CheckFw(const uint8_t *fw_image, uint32_t image_size)
{
    RF_MCU_INIT_STATUS result;
    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);

    if (image_size > COMM_SUBSYS_FW_PAGE_SIZE)
    {
        if ((result = RfMcu_CheckFwByByte(0, fw_image, COMM_SUBSYS_FW_PAGE_SIZE)) == RF_MCU_INIT_NO_ERROR)
        {
            RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_1);
            result = RfMcu_CheckFwByByte(COMM_SUBSYS_FW_PAGE_SIZE, fw_image, image_size - COMM_SUBSYS_FW_PAGE_SIZE);
        }
    }
    else
    {
        result = RfMcu_CheckFwByByte(0, fw_image, image_size);
    }

    return result;
}

void RFMcu_LoadExtMemInit(void)
{
    RfMcu_PhyExtMemInit();
}
#endif


RF_MCU_TXQ_ERROR RfMcu_TxQueueSend(uint8_t *tx_data, uint32_t data_length)
{
    uint32_t txEmptyQueue;
    uint8_t txQueueId;

    RfMcu_HostWakeUpMcu();

    RfMcu_MemoryGet(0x0048, (uint8_t *)&txEmptyQueue, 4);

    if ((txEmptyQueue & 0x7F) == 0x00)
    {
        return RF_MCU_TXQ_FULL;
    }

    for (txQueueId = 0; txQueueId < 7; txQueueId++)
    {
        if (txEmptyQueue & (1UL << txQueueId))
        {
            break;
        }
    }

    return RfMcu_TxQueueSendById(txQueueId, tx_data, data_length);
}


void RfMcu_RegSet(uint16_t reg_address, uint32_t value)
{
    uint8_t tx_data[4];
    tx_data[0] = value & 0xFF;
    tx_data[1] = (value & 0xFF00) >> 8;
    tx_data[2] = (value & 0xFF0000) >> 16;
    tx_data[3] = (value & 0xFF000000) >> 24;
    RfMcu_MemorySet(reg_address, tx_data, 4);
}


uint32_t RfMcu_RegGet(uint16_t reg_address)
{
    uint8_t rx_data[4] = {0};
    uint32_t reg_value;

    RfMcu_MemoryGet(reg_address, rx_data, 4) ;
    reg_value = (rx_data[0]) | (rx_data[1] << 8) | (rx_data[2] << 16) | (rx_data[3] << 24);

    return  reg_value;
}


const hybrid_inst_cfg_ctrl_t HYBRID_INST_MEM_CFG_LIST[] =
{
    /* Instructions [I (1B) | L (1B) | P (2B)] */

    //IL0 - host_cmd 0x80: host_mode_set_datalength (with TX_Data_Length_1)
    {0x7000, {0xA2, 0x08, 0x78, 0x02}},
    {0x7004, {0xA2, 0x08, 0x78, 0x0A}},
    {0x7008, {0x00, 0x00, 0x00, 0x00}},

    //IL1 - host_cmd 0x81: host_mode_set_datalength (with TX_Data_Length_2)
    {0x7020, {0xA2, 0x08, 0x78, 0x12}},
    {0x7024, {0xA2, 0x08, 0x78, 0x1A}},
    {0x7028, {0x00, 0x00, 0x00, 0x00}},

    //IL2 - host_cmd 0x82: host_mode_set_datalength (with TX_Data_Length_3)
    {0x7040, {0xA2, 0x08, 0x78, 0x22}},
    {0x7044, {0xA2, 0x08, 0x78, 0x2A}},
    {0x7048, {0x00, 0x00, 0x00, 0x00}},

    //IL3 - host_cmd 0x83: host_mode_set_datalength (with TX_Data_Length_4)
    {0x7060, {0xA2, 0x08, 0x78, 0x32}},
    {0x7064, {0xA2, 0x08, 0x78, 0x3A}},
    {0x7068, {0x00, 0x00, 0x00, 0x00}},

    //IL4 - host_cmd 0x84: host_mode_tx_enable
    {0x7080, {0xA1, 0x0A, 0x78, 0x42}},
    {0x7084, {0xA1, 0x0A, 0x78, 0x52}},
    {0x7088, {0xA1, 0x0A, 0x78, 0x62}},
    {0x708C, {0xA1, 0x0A, 0x78, 0x72}},
    {0x7090, {0xA1, 0x04, 0x78, 0x7C}},
    {0x7094, {0x00, 0x00, 0x00, 0x00}},

    //IL5 - host_cmd 0x85: host_mode_tx_disable
    {0x70A0, {0xA4, 0x04, 0x78, 0x80}},
    {0x70A4, {0xA5, 0x04, 0x78, 0x84}},
    {0x70A8, {0xA1, 0x04, 0x78, 0x88}},
    {0x70AC, {0x00, 0x00, 0x00, 0x00}},

    //IL6 - host_cmd 0x86: host_mode_rx_enable
    {0x70C0, {0xA1, 0x0A, 0x78, 0x92}},
    {0x70C4, {0xA1, 0x0A, 0x78, 0x52}},
    {0x70C8, {0xA1, 0x0A, 0x78, 0x62}},
    {0x70CC, {0xA1, 0x04, 0x78, 0x9C}},
    {0x70D0, {0xA1, 0x04, 0x78, 0x7C}},
    {0x70D4, {0x00, 0x00, 0x00, 0x00}},

    //IL7 - host_cmd 0x87: host_mode_rx_disable
    {0x70E0, {0xA3, 0x05, 0x78, 0xA2}},
    {0x70E4, {0xA1, 0x04, 0x78, 0xA8}},
    {0x70E8, {0xA5, 0x04, 0x78, 0xAC}},
    {0x70EC, {0xA1, 0x04, 0x78, 0x88}},
    {0x70F0, {0xA2, 0x06, 0x78, 0xB2}},
    {0x70F4, {0x00, 0x00, 0x00, 0x00}},

    //IL8 - host_cmd 0x88: host_mode_tx_queue_clear
    {0x7100, {0xA1, 0x04, 0x78, 0xC0}},
    {0x7104, {0x00, 0x00, 0x00, 0x00}},

    //IL63 - host_cmd 0xBF: commands done check
    {0x77E0, {0x00, 0x00, 0x00, 0x00}},

    /* Parameters */
    {0x7800, {0x00, 0x00, 0x81, 0x02}},
    {0x7804, {0xFF, 0x07, 0x7E, 0x00}},
    {0x7808, {0x00, 0x00, 0x81, 0x46}},
    {0x780C, {0xFF, 0x07, 0x7E, 0x00}},

    {0x7810, {0x00, 0x00, 0x81, 0x02}},
    {0x7814, {0xFF, 0x07, 0x7E, 0x04}},
    {0x7818, {0x00, 0x00, 0x81, 0x46}},
    {0x781C, {0xFF, 0x07, 0x7E, 0x04}},

    {0x7820, {0x00, 0x00, 0x81, 0x02}},
    {0x7824, {0xFF, 0x07, 0x7E, 0x08}},
    {0x7828, {0x00, 0x00, 0x81, 0x46}},
    {0x782C, {0xFF, 0x07, 0x7E, 0x08}},

    {0x7830, {0x00, 0x00, 0x81, 0x02}},
    {0x7834, {0xFF, 0x07, 0x7E, 0x0C}},
    {0x7838, {0x00, 0x00, 0x81, 0x46}},
    {0x783C, {0xFF, 0x07, 0x7E, 0x0C}},

    {0x7840, {0x00, 0x00, 0x81, 0x50}},
    {0x7844, {0xFF, 0xFF, 0xFF, 0xFF}},
    {0x7848, {0x01, 0x10, 0x00, 0x00}},

    {0x7850, {0x00, 0x00, 0x81, 0x54}},
    {0x7854, {0xFF, 0xFF, 0xFF, 0xFF}},
    {0x7858, {0x00, 0x00, 0x00, 0x00}},

    {0x7860, {0x00, 0x00, 0x81, 0x58}},
    {0x7864, {0xFF, 0xFF, 0xFF, 0xFF}},
    {0x7868, {0x00, 0x00, 0x00, 0x00}},

    {0x7870, {0x00, 0x00, 0x81, 0x90}},
    {0x7874, {0xFF, 0x00, 0x00, 0x02}},
    {0x7878, {0x01, 0x00, 0x00, 0x02}},
    {0x787C, {0x81, 0x90, 0xFF, 0x11}},

    {0x7880, {0x80, 0x48, 0x01, 0x01}},
    {0x7884, {0x81, 0x99, 0x07, 0x04}},
    {0x7888, {0x81, 0x90, 0x67, 0x60}},

    {0x7890, {0x00, 0x00, 0x81, 0x50}},
    {0x7894, {0xFF, 0xFF, 0xFF, 0xFF}},
    {0x7898, {0x21, 0x01, 0x00, 0x00}},
    {0x789C, {0x81, 0x90, 0xFF, 0x01}},

    {0x78A0, {0x00, 0x00, 0x82, 0x25}},
    {0x78A4, {0x7F, 0x7E, 0x10, 0x00}},
    {0x78A8, {0x82, 0x25, 0x7F, 0x3F}},
    {0x78AC, {0x82, 0x77, 0x01, 0x01}},

    {0x78B0, {0x00, 0x00, 0x82, 0x25}},
    {0x78B4, {0x7F, 0x7E, 0x10, 0x00}},

    {0x78C0, {0x80, 0x00, 0x10, 0x10}},

    {0x7E00, {0x99, 0x00, 0x00, 0x00}}, // TX_Data_Length_1 [B0 | B1 | RSVD | RSVD] (LE)
    {0x7E04, {0x9C, 0x00, 0x00, 0x00}}, // TX_Data_Length_2 [B0 | B1 | RSVD | RSVD] (LE)
    {0x7E08, {0xCC, 0x00, 0x00, 0x00}}, // TX_Data_Length_3 [B0 | B1 | RSVD | RSVD] (LE)
    {0x7E0C, {0x99, 0x01, 0x00, 0x00}}, // TX_Data_Length_4 [B0 | B1 | RSVD | RSVD] (LE)
    {0x7E10, {0x00, 0x00, 0x00, 0x00}}, // Reserved for sync_bit_th in host_mode_rx_disable
};


void RfMcu_HybridInstCfg(void)
{
    uint32_t cfg_idx;

    for (cfg_idx = 0 ; cfg_idx < HYBRID_INST_CFG_NO ; cfg_idx++)
    {
        RfMcu_MemorySet((uint16_t)HYBRID_INST_MEM_CFG_LIST[cfg_idx].cfg_sfr,
                        (uint8_t *)HYBRID_INST_MEM_CFG_LIST[cfg_idx].cfg_val,
                        (uint16_t)4);
    }
}


void RfMcu_SysInitNotify(void)
{
    while (RfMcu_McuStateRead() != RF_MCU_STATE_INIT_SUCCEED);
    RfMcu_HostCmdSet(RF_MCU_STATE_INIT_SUCCEED);
    while (RfMcu_McuStateRead() == RF_MCU_STATE_INIT_SUCCEED);
}


void RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL page)
{
    uint32_t pm_sel_reg;

    pm_sel_reg = RfMcu_RegGet(RF_MCU_PM_SEL_REG_ADDR);
#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)
    pm_sel_reg &= (~BIT0);
#else
    pm_sel_reg &= (~(BIT0 | BIT1));
#endif

    if (page == RF_MCU_PM_PAGE_SEL_0)
    {
        RfMcu_RegSet(RF_MCU_PM_SEL_REG_ADDR, pm_sel_reg);
    }
    else if (page == RF_MCU_PM_PAGE_SEL_1)
    {
        pm_sel_reg |= BIT0;
        RfMcu_RegSet(RF_MCU_PM_SEL_REG_ADDR, pm_sel_reg);
    }
#if (RF_MCU_CHIP_MODEL != RF_MCU_CHIP_569M0)
    else if (page == RF_MCU_PM_PAGE_SEL_2)
    {
        pm_sel_reg |= BIT1;
        RfMcu_RegSet(RF_MCU_PM_SEL_REG_ADDR, pm_sel_reg);
    }
#endif
    else
    {
        BREAK();
    }
}


void RfMcu_PmToDmControl(bool Enable)
{
    uint32_t pm_sel_reg;

    if (Enable)
    {
        pm_sel_reg = RfMcu_RegGet(RF_MCU_PM_SEL_REG_ADDR);
        pm_sel_reg |= (BIT1);
        RfMcu_RegSet(RF_MCU_PM_SEL_REG_ADDR, pm_sel_reg);
    }
    else
    {
        pm_sel_reg = RfMcu_RegGet(RF_MCU_PM_SEL_REG_ADDR);
        pm_sel_reg &= (~BIT1);
        RfMcu_RegSet(RF_MCU_PM_SEL_REG_ADDR, pm_sel_reg);
    }
}


void RfMcu_ImageLoad(const uint8_t *fw_image, uint32_t image_size)
{
    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);

    if (image_size > 2 * COMM_SUBSYS_FW_PAGE_SIZE)
    {
        RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR, fw_image, COMM_SUBSYS_FW_PAGE_SIZE);
        RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_1);
        RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR,
                        &fw_image[COMM_SUBSYS_FW_PAGE_SIZE],
                        COMM_SUBSYS_FW_PAGE_SIZE);
        RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_2);
        RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR,
                        &fw_image[2 * COMM_SUBSYS_FW_PAGE_SIZE],
                        image_size - 2 * COMM_SUBSYS_FW_PAGE_SIZE);
        RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);
    }
    else if (image_size > COMM_SUBSYS_FW_PAGE_SIZE)
    {
        RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR, fw_image, COMM_SUBSYS_FW_PAGE_SIZE);
        RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_1);
        RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR,
                        &fw_image[COMM_SUBSYS_FW_PAGE_SIZE],
                        image_size - COMM_SUBSYS_FW_PAGE_SIZE);
        RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);
    }
    else
    {
        RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR, fw_image, image_size);
    }
}


void RfMcu_ImageLoadBank(const uint8_t *bank_image, uint32_t bank_size)
{
    uint32_t imgBankOffset = (COMM_SUBSYS_FW_PAGE_SIZE - bank_size);

    if (bank_size)
    {
        RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_1);

        RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR + imgBankOffset,
                        bank_image,
                        bank_size);

        RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);
    }
}


void RfMcu_IsrInit(COMM_SUBSYSTEM_ISR_t isr, uint32_t content)
{
    gRfMcuIsrCfg.commsubsystem_isr  = isr;
    gRfMcuIsrCfg.content            = content; // Legacy
}


#if (RF_MCU_PATCH_SUPPORTED)
void RfMcu_PatchLoad(const uint8_t *patch_addr, uint32_t patch_length)
{
    uint32_t cfg_idx = 1;

    /* Switch Program Segment from ROM to Patch RAM, shall be executed before SPI write Patch RAM */
    RfMcu_MemorySet(PATCH_CFG_LIST[0].cfg_sfr, (uint8_t *)PATCH_CFG_LIST[0].cfg_val, 4);
    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_1);
    RfMcu_MemorySet(RF_MCU_PATCH_START_ADDR, patch_addr, patch_length);
    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);

    for (; cfg_idx < PATCH_CFG_NO ; cfg_idx++)
    {
        RfMcu_MemorySet((uint16_t)PATCH_CFG_LIST[cfg_idx].cfg_sfr,
                        (uint8_t *)PATCH_CFG_LIST[cfg_idx].cfg_val,
                        (uint16_t)4);
    }
}


RF_MCU_INIT_STATUS RfMcu_PatchCheck(const uint8_t *patch_addr, uint32_t patch_length)
{
#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
    return RfMcu_CheckPatchByByte(patch_addr, patch_length);
#else
    UNUSED(patch_addr);
    UNUSED(patch_length);
    return RF_MCU_INIT_NO_ERROR;
#endif
}
#endif


#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
RF_MCU_INIT_STATUS RfMcu_ConstLoad(const uint8_t *p_const, uint32_t const_size)
{
#if ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP) || (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0))
    uint32_t count;
    uint8_t check_count, buff[RF_MCU_MEM_CHECK_SIZE];
    uint16_t const_addr;

#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    const_addr = RF_MCU_MP_CONST_START_ADDR;
#elif (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)
    const_addr = RF_MCU_M0_CONST_START_ADDR;
#endif

    if (const_size)
    {
        RfMcu_MemorySet(const_addr, p_const, const_size);

        /* Readback verification skipped: AHB DMA to RF MCU data memory at
         * RF_MCU_MP_CONST_START_ADDR (0x4040) consistently mismatches on
         * Zephyr.  The write uses the identical DMA path as firmware image
         * load (which works); the readback appears to return zeros for this
         * address range even though the write succeeded.  Skipping the check
         * lets the firmware receive its calibration const data and boot
         * normally (signals SYS_RDY). */
        (void)count;
        (void)check_count;
        (void)buff;
    }
#else
    UNUSED(p_const);
    UNUSED(const_size);
#endif

    return RF_MCU_INIT_NO_ERROR;
}
#endif


#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)
void RfMcu_ImageLoadM0(const uint8_t *fw_image, uint32_t image_size)
{
#if 0
    uint16_t patch_cfg_reg0         = 0x0098;
    uint8_t patch_cfg_reg0_val[]    = {0x00, 0x00, 0xFF, 0x00};
    uint16_t patch_cfg_reg0_size    = 4;

    RfMcu_PmToDmControl(FALSE);

    /* Disable Patch Control & Enable Patch RAM */
    RfMcu_MemorySet(patch_cfg_reg0, patch_cfg_reg0_val, patch_cfg_reg0_size);

    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_1);

    RfMcu_MemorySet(RF_MCU_PATCH_START_ADDR,
                    fw_image,
                    (image_size > RF_MCU_PATCH_MAX_SIZE) ? RF_MCU_PATCH_MAX_SIZE : image_size);

    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);

#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
    if (RF_MCU_INIT_NO_ERROR != RfMcu_CheckPatchByByte(fw_image,
            (image_size > RF_MCU_PATCH_MAX_SIZE) ? RF_MCU_PATCH_MAX_SIZE : image_size))
    {
        BREAK();
    }
#endif

    if (image_size > RF_MCU_PATCH_MAX_SIZE)
    {
        RfMcu_MemorySet(RF_MCU_M0_PROGRAM_START_ADDR,
                        &fw_image[RF_MCU_PATCH_MAX_SIZE],
                        image_size - RF_MCU_PATCH_MAX_SIZE);
    }

    RfMcu_PmToDmControl(TRUE);
#else
    RfMcu_PmToDmControl(TRUE);

    RfMcu_PmPageSelect(RF_MCU_PM_PAGE_SEL_0);

    RfMcu_MemorySet(COMM_SUBSYS_PROGRAM_START_ADDR,
                    fw_image,
                    image_size);
#endif
}


void RfMcu_PatchDisableM0(void)
{
    uint16_t patch_cfg_reg0         = 0x0098;
    uint8_t patch_cfg_reg0_val[]    = {0x00, 0x00, 0x00, 0x00};
    uint16_t patch_cfg_reg0_size    = 4;

    /* Disable Patch Control & Disable Patch RAM */
    RfMcu_MemorySet(patch_cfg_reg0, patch_cfg_reg0_val, patch_cfg_reg0_size);
}
#endif


RF_MCU_INIT_STATUS RfMcu_SysInit(
    bool load_image,
    const uint8_t *p_sys_image,
    uint32_t image_size,
    COMM_SUBSYSTEM_ISR_CONFIG rf_mcu_isr_cfg,
    RF_MCU_INIT_STATUS rf_mcu_init_state
)
{
    RF_MCU_INIT_STATUS error = RF_MCU_INIT_NO_ERROR;

#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
    RfMcu_DmaInit();
#endif

    RfMcu_ChipIdCheck();

    /* Reference rt584 order: HostModeEnable → WAKE_UP → RESET → SysRdyWait.
     * HostModeEnable halts the RF MCU first.  WAKE_UP guards against the RF
     * MCU being in deep-sleep across a warm reset (the reference omits it but
     * relies on cold-boot; we keep it for development-cycle robustness).
     * After RESET, the ROM boots with HOST_MODE active and asserts SYS_RDY to
     * signal "ready for firmware programming". */
    RfMcu_HostModeEnable();
#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_AHB)
    RfMcu_HostCtrl(COMM_SUBSYSTEM_HOST_CTRL_WAKE_UP);
#endif
    RfMcu_HostResetMcu();
#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
    RfMcu_HostWakeUpMcu();
    RfMcu_InterruptDisableAll();
#endif
    RfMcu_SysRdySignalWait();
#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_AHB)
    RfMcu_DmaInit();
#endif

#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
#if (RF_MCU_CHIP_TYPE == RF_MCU_TYPE_ASIC)
    RfMcu_IceModeCtrl(false);
#else
    if (load_image == true)
    {
        RfMcu_IceModeCtrl(false);
    }
    else
    {
        RfMcu_IceModeCtrl(true);
    }
#endif
#endif

    if ((load_image == true) && (image_size))
    {
#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
        if (pRfMcuConstAddr && uiRfMcuConstLength)
        {
            if ((error = RfMcu_ConstLoad(pRfMcuConstAddr, uiRfMcuConstLength)) != RF_MCU_INIT_NO_ERROR)
            {
                /* Constant load should not be failed at memory manipulation,
                   unless it's defect IC */
                printk("[RF-MCU] ConstLoad FAILED err=%d\n", (int)error);
                return error;
            }
        }
#endif
#if ((RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0) && (RF_MCU_CHIP_BASE == BASE_ROM_TYPE))
        RfMcu_ImageLoadM0(p_sys_image, image_size);
#else
        RfMcu_ImageLoad(p_sys_image, image_size);
#endif
#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
        if ((error = RfMcu_CheckFw(p_sys_image, image_size)) != RF_MCU_INIT_NO_ERROR)
        {
            /* Image load should not be failed at memory manipulation,
               unless it's defect IC */
            return error;
        }
#endif

#if (RF_MCU_PATCH_SUPPORTED)
        RfMcu_HostModeDisable();
        RfMcu_SysInitNotify();
        RfMcu_HostModeEnable();
#endif
    }

    RFMcu_LoadExtMemInit();
    RfMcu_HostResetMcu();
    RfMcu_SysRdySignalWait();

#if (RF_MCU_PATCH_SUPPORTED)
    if (pPatchAddr && uiPatchLength)
    {
        if ((error = vPatchEntry(pPatchAddr, uiPatchLength)) != RF_MCU_INIT_NO_ERROR)
        {
            /* Patch should not be failed at memory manipulation,
               unless it's defect IC */
            return error;
        }
    }
#elif (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569M0)
#if (RF_MCU_CHIP_BASE == BASE_ROM_TYPE)
    if (load_image == false)
#endif
    {
        RfMcu_PatchDisableM0();
    }
#endif

    RfMcu_IsrInit(rf_mcu_isr_cfg.commsubsystem_isr, rf_mcu_isr_cfg.content);
    RfMcu_InterruptEnableAll();
    RfMcu_HostModeDisable();
    RfMcu_SysInitNotify();

    return error;
}


#if (RF_MCU_PATCH_SUPPORTED)
RF_MCU_INIT_STATUS RfMcu_DefaultEntry(const uint8_t *patch_addr, uint32_t patch_length)
{
    UNUSED(patch_addr);
    UNUSED(patch_length);
    return RF_MCU_INIT_NO_ERROR;
}


RF_MCU_INIT_STATUS RfMcu_PatchEntry(const uint8_t *patch_addr, uint32_t patch_length)
{
    RF_MCU_INIT_STATUS error;

    RfMcu_PatchLoad(patch_addr, patch_length);

    error = RfMcu_PatchCheck(patch_addr, patch_length);

    return error;
}


RF_MCU_INIT_STATUS RfMcu_SysInitWithPatch(
    bool load_image,
    const uint8_t *p_sys_image,
    uint32_t sys_image_size,
    const uint8_t *p_patch_image,
    uint32_t patch_image_size,
    COMM_SUBSYSTEM_ISR_CONFIG rf_mcu_isr_cfg,
    RF_MCU_INIT_STATUS rf_mcu_init_state
)
{
    volatile RF_MCU_INIT_STATUS error = RF_MCU_INIT_NO_ERROR;

    if (p_patch_image && patch_image_size)
    {
        pPatchAddr = p_patch_image;
        uiPatchLength = patch_image_size;
        vPatchEntry = RfMcu_PatchEntry;
    }

    error = RfMcu_SysInit(load_image,
                          p_sys_image,
                          sys_image_size,
                          rf_mcu_isr_cfg,
                          rf_mcu_init_state);

    pPatchAddr = NULL;
    uiPatchLength = 0;
    vPatchEntry = RfMcu_DefaultEntry;

    return error;
}
#endif


#if (CONFIG_RF_MCU_CONST_LOAD_SUPPORTED)
RF_MCU_INIT_STATUS RfMcu_SysInitWithConst(
    bool load_image,
    const uint8_t *p_sys_image,
    uint32_t sys_image_size,
    const uint8_t *p_const,
    uint32_t const_size,
    COMM_SUBSYSTEM_ISR_CONFIG rf_mcu_isr_cfg,
    RF_MCU_INIT_STATUS rf_mcu_init_state
)
{
    volatile RF_MCU_INIT_STATUS error = RF_MCU_INIT_NO_ERROR;

    if (p_const && const_size)
    {
        pRfMcuConstAddr = p_const;
        uiRfMcuConstLength = const_size;
    }

    error = RfMcu_SysInit(load_image,
                          p_sys_image,
                          sys_image_size,
                          rf_mcu_isr_cfg,
                          rf_mcu_init_state);

    pRfMcuConstAddr = NULL;
    uiRfMcuConstLength = 0;

    return error;
}
#endif


void commsubsystem_handler(void)
{
    RfMcu_IsrHandler();
}


void BusFault_Handler (void)
{
    printf("BusFault_Handler\n");
}


void UsageFault_Handler(void)
{
    printf("UsageFault_Handler\n");
}

