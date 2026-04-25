/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


/**************************************************************************//**
 * @file     rf_mcu_spi.c
 * @version
 * @brief    Communication subsystem with SPI

 ******************************************************************************/

#include <string.h>
#include "mcu.h"
#include "qspi.h"
#include "rf_mcu_spi.h"
#include "stdio.h"

#if (CFG_RF_MCU_CTRL_TYPE == RF_MCU_CTRL_BY_SPI)
#define QSPI_ID_0                           0
#define QSPI_ID_1                           1

#define SPI_PORT_SELECT_SET_1               0    /* GPIO 6 7 8 9*/
#define SPI_PORT_SELECT_SET_2               1    /* GPIO 28 29 30 31*/

#define RF_MCU_RX_DATA_Q_ID                 0x0
#define RF_MCU_RX_EVENT_Q_ID                0x1

#define RF_MCU_SPI_HEADER_LENGTH            4
#define RF_MCU_SPI_SFR_CMD_LENGTH           2
#define RF_MCU_SPI_SFR_RSP_LENGTH           1

#define RF_MCU_SFR_READ_OPCODE              0x0
#define RF_MCU_SFR_WRITE_OPCODE             0x1
#define RF_MCU_MEM_READ_OPCODE              0x2
#define RF_MCU_MEM_WRITE_OPCODE             0x3
#define RF_MCU_CMD_READ_OPCODE              0x4
#define RF_MCU_CMD_WRITE_OPCODE             0x8

#define RF_MCU_SFR_OPCODE_OFFSET            4
#define RF_MCU_MEM_OPCODE_OFFSET            14
#define RF_MCU_CMD_OPCODE_OFFSET            11

#define RF_MCU_MEM_ADDR_4BYTE_OFFSET        2

#define RF_MCU_SPI_ACTIVE_CHOICE            SPI_CHIPSEL_ACTIVE_LOW
#define RF_MCU_SPI_ORDER                    SPI_MSB_ORDER
#define RF_MCU_SPI_MODE                     SPI_MASTER_MODE
#define RF_MCU_SPI_TARGET_SLAVE             SPI_SELECT_SLAVE_0
#define RF_MCU_SPI_CLOCK                    QSPI_CLK_8M
#define RF_MCU_SPI_SELECT                   QSPI_ID_0
#define RF_MCU_SPI_PORT_SELECT              SPI_PORT_SELECT_SET_1

#define RF_MCU_SPI_BUFF_SIZE                0x800
#define RF_MCU_SPI_PIO_MAX_RX_SIZE          31

#define RF_MCU_SPI_ISR_ENABLE_ALL_MASK      COMM_SUBSYSTEM_INT_STATUS_MASK
#define RF_MCU_SPI_ISR_DISABLE_ALL_MASK     0x00

#define RF_MCU_SPI_RX_DATA_Q_MASK           0x01
#define RF_MCU_SPI_RX_EVENT_Q_MASK          0x02
#define RF_MCU_SPI_RX_Q_MSB_MASK            0x0F
#define RF_MCU_SPI_RX_Q_MSB_OFFSET          0x08

#define RF_MCU_SPI_CMD_Q_READY_MASK         0x80

#define RF_MCU_SPI_SYS_READY_BIT            0x01
#define RF_MCU_SPI_POWER_STATE_OFFSET       0x01
#define RF_MCU_SPI_POWER_STATE_MASK         0x03

#define GET_WORDLENGTH(data_length)    ((data_length + 1) >> 1)

uint8_t g_rf_mcu_spi_tx_buff[RF_MCU_SPI_BUFF_SIZE + RF_MCU_SPI_HEADER_LENGTH];
uint8_t g_rf_mcu_spi_rx_buff[RF_MCU_SPI_BUFF_SIZE + RF_MCU_SPI_HEADER_LENGTH];

void RfMcu_SpiIsrCallback(uint32_t qspi_id, uint32_t status)
{
}

void RfMcu_SpiSetSFR(uint8_t reg_addr, uint8_t value)
{
    spi_block_request_t     master_xfer_request;

    enter_critical_section(); //Disable SPI Interrupt
    while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE) //wait until SPI in IDLE
    {};

    g_rf_mcu_spi_tx_buff[0] = ((RF_MCU_SFR_WRITE_OPCODE << RF_MCU_SFR_OPCODE_OFFSET) | (reg_addr & 0x0F));
    g_rf_mcu_spi_tx_buff[1] = value;

    master_xfer_request.write_buf = g_rf_mcu_spi_tx_buff;
    master_xfer_request.read_buf = NULL;
    master_xfer_request.length = RF_MCU_SPI_SFR_CMD_LENGTH;

    spi_transfer_pio(RF_MCU_SPI_SELECT, &master_xfer_request);
    leave_critical_section();//Enable SPI Interrupt
}

uint8_t RfMcu_SpiGetSFR(uint8_t reg_addr)
{
    volatile uint8_t        value;
    spi_block_request_t     master_xfer_request;

    enter_critical_section(); //Disable SPI Interrupt
    while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE) //wait until SPI in IDLE
    {};

    g_rf_mcu_spi_tx_buff[0] = ((RF_MCU_SFR_READ_OPCODE << RF_MCU_SFR_OPCODE_OFFSET) | (reg_addr & 0x0F));
    g_rf_mcu_spi_tx_buff[1] = 0;  //dummy
    g_rf_mcu_spi_tx_buff[2] = 0;  //dummy

    master_xfer_request.write_buf = g_rf_mcu_spi_tx_buff;
    master_xfer_request.read_buf = g_rf_mcu_spi_rx_buff;
    master_xfer_request.length = RF_MCU_SPI_SFR_CMD_LENGTH + RF_MCU_SPI_SFR_RSP_LENGTH;

    spi_transfer_pio(RF_MCU_SPI_SELECT, &master_xfer_request);
    leave_critical_section();//Enable SPI Interrupt

    value = g_rf_mcu_spi_rx_buff[2];

    return value;
}

void RfMcu_MemorySetSpi(uint16_t sys_addr, const uint8_t *p_data, uint16_t data_length)
{
    uint16_t tmp = 0;
    uint16_t tx_length, tx_offset = 0;

    spi_block_request_t     master_xfer_request;

    while (data_length > 0)
    {
        enter_critical_section(); //Disable SPI Interrupt
        while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE); //wait until SPI in IDLE

        tx_length = (data_length > RF_MCU_SPI_BUFF_SIZE ? RF_MCU_SPI_BUFF_SIZE : data_length);

        tmp = ((sys_addr + tx_offset) >> RF_MCU_MEM_ADDR_4BYTE_OFFSET);  // To make address written is 4 bytes aligned
        tmp |= (RF_MCU_MEM_WRITE_OPCODE << RF_MCU_MEM_OPCODE_OFFSET);  //bit[15:14] = OPCODE

        g_rf_mcu_spi_tx_buff[0] = tmp >> 8;
        g_rf_mcu_spi_tx_buff[1] = (tmp & 0xFF);
        g_rf_mcu_spi_tx_buff[2] = 0;  //dummy
        g_rf_mcu_spi_tx_buff[3] = 0;  //dummy

        memcpy(g_rf_mcu_spi_tx_buff + RF_MCU_SPI_HEADER_LENGTH, p_data + tx_offset, tx_length);

        master_xfer_request.write_buf = g_rf_mcu_spi_tx_buff;
        master_xfer_request.read_buf = NULL;
        master_xfer_request.length = tx_length + RF_MCU_SPI_HEADER_LENGTH;

        spi_transfer_pio(RF_MCU_SPI_SELECT, &master_xfer_request);
        leave_critical_section();//Enable SPI Interrupt

        data_length -= tx_length;
        tx_offset += tx_length;
    };
}


void RfMcu_MemoryGetSpi(uint16_t sys_addr, uint8_t *p_data, uint16_t data_length)
{
    //Caution! rx_data_length must < 32 if use SPI PIO mode
    uint16_t tmp = 0;
    uint16_t rx_length, rx_offset = 0;
    uint8_t mod;

    spi_block_request_t     master_xfer_request;

    while (data_length > 0)
    {
        enter_critical_section();//Disable SPI Interrupt
        while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE); //wait until SPI in IDLE

        mod = ((sys_addr + rx_offset) % 4);
        rx_length = (data_length > (RF_MCU_SPI_PIO_MAX_RX_SIZE - RF_MCU_SPI_HEADER_LENGTH - mod) ? (RF_MCU_SPI_PIO_MAX_RX_SIZE - RF_MCU_SPI_HEADER_LENGTH) : data_length + mod);

        tmp = ((sys_addr + rx_offset) >> RF_MCU_MEM_ADDR_4BYTE_OFFSET);  // To make address written is 4 bytes aligned
        tmp |= (RF_MCU_MEM_READ_OPCODE << RF_MCU_MEM_OPCODE_OFFSET);  //bit[15:14]= OPCODE.

        g_rf_mcu_spi_tx_buff[0] = tmp >> 8;
        g_rf_mcu_spi_tx_buff[1] = (tmp & 0xFF);
        g_rf_mcu_spi_tx_buff[2] = 0;  //dummy
        g_rf_mcu_spi_tx_buff[3] = 0;  //dummy

        memset(&g_rf_mcu_spi_tx_buff[0] + RF_MCU_SPI_HEADER_LENGTH, 0x00, rx_length);

        master_xfer_request.write_buf = g_rf_mcu_spi_tx_buff;
        master_xfer_request.read_buf = g_rf_mcu_spi_rx_buff;
        master_xfer_request.length = rx_length + RF_MCU_SPI_HEADER_LENGTH;

        spi_transfer_pio(RF_MCU_SPI_SELECT, &master_xfer_request);
        leave_critical_section();//Enable SPI Interrupt

        while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE); //wait until SPI in IDLE

        memcpy(p_data + rx_offset, master_xfer_request.read_buf + RF_MCU_SPI_HEADER_LENGTH + mod, rx_length - mod);

        data_length -= (rx_length - mod);
        rx_offset += (rx_length - mod);

    };
}


void RfMcu_IoSetSpi(uint8_t queue_id, const uint8_t *p_data, uint16_t data_length)
{
    uint16_t tmp = 0;
    uint16_t tmp_tx_data_length;

    spi_block_request_t     master_xfer_request;

    enter_critical_section(); //Disable SPI Interrupt
    while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE); //wait until SPI in IDLE

    tmp_tx_data_length = GET_WORDLENGTH(data_length);//To calculate the wordlength value in IO_Write cmd
    data_length = tmp_tx_data_length << 1;//To calculate SPI clock after dummy cycles to transmit data

    tmp = (RF_MCU_CMD_WRITE_OPCODE << RF_MCU_CMD_OPCODE_OFFSET);//To make OP code, bit[15:14]=1 for write mode.
    tmp |= ((queue_id & 0x7) << RF_MCU_CMD_OPCODE_OFFSET);//To make OP code
    tmp |= (tmp_tx_data_length & 0x7FF);//To append the wordlength value after OP code

    g_rf_mcu_spi_tx_buff[0] = tmp >> 8;
    g_rf_mcu_spi_tx_buff[1] = (tmp & 0xFF);
    g_rf_mcu_spi_tx_buff[2] = 0;  //dummy
    g_rf_mcu_spi_tx_buff[3] = 0;  //dummy

    memcpy(g_rf_mcu_spi_tx_buff + RF_MCU_SPI_HEADER_LENGTH, p_data, data_length);

    master_xfer_request.write_buf = g_rf_mcu_spi_tx_buff;
    master_xfer_request.read_buf = NULL;
    master_xfer_request.length = data_length + RF_MCU_SPI_HEADER_LENGTH;

    spi_transfer_pio(RF_MCU_SPI_SELECT, &master_xfer_request);
    leave_critical_section();//Enable SPI Interrupt
}


void RfMcu_IoGetSpi(uint16_t queue_id, uint8_t *p_data, uint16_t data_length)
{
    uint16_t tmp = 0;
    uint16_t tmp_rx_data_length;

    spi_block_request_t     master_xfer_request;

    enter_critical_section(); //Disable SPI Interrupt
    while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE); //wait until SPI in IDLE

    tmp_rx_data_length = GET_WORDLENGTH(data_length);//To calculate the wordlength value in IO_Write cmd
    data_length = tmp_rx_data_length << 1;//To calculate SPI clock after dummy cycles to transmit data

    tmp = (RF_MCU_CMD_READ_OPCODE << RF_MCU_CMD_OPCODE_OFFSET);//To make OP code, bit[15:14]=1 for write mode.
    tmp |= ((queue_id & 0x3) << RF_MCU_CMD_OPCODE_OFFSET);//To make OP code
    tmp |= (tmp_rx_data_length & 0x7FF);//To append the wordlength value after OP code

    g_rf_mcu_spi_tx_buff[0] = tmp >> 8;
    g_rf_mcu_spi_tx_buff[1] = (tmp & 0xFF);
    g_rf_mcu_spi_tx_buff[2] = 0;  //dummy
    g_rf_mcu_spi_tx_buff[3] = 0;  //dummy

    memset(g_rf_mcu_spi_tx_buff + RF_MCU_SPI_HEADER_LENGTH, 0, data_length);

    master_xfer_request.write_buf = g_rf_mcu_spi_tx_buff;
    master_xfer_request.read_buf = g_rf_mcu_spi_rx_buff;
    master_xfer_request.length = data_length + RF_MCU_SPI_HEADER_LENGTH;

    spi_transfer(RF_MCU_SPI_SELECT, &master_xfer_request, RfMcu_SpiIsrCallback);
    leave_critical_section();//Enable SPI Interrupt

    while (get_qspi_state(RF_MCU_SPI_SELECT) != QSPI_STATE_IDLE); //wait until SPI in IDLE

    memcpy(p_data, master_xfer_request.read_buf + RF_MCU_SPI_HEADER_LENGTH, data_length);
}


void RfMcu_HostCmdSetSpi(uint8_t cmd)
{
    while (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_RXQ_CMD_STATE) & RF_MCU_HOST_CMD_BUSY_BIT);

    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_HOST_CMD_STATE, cmd);
}


void RfMcu_HostCtrlSpi(uint32_t ctrl)
{
    /* No used */
}


void RfMcu_InterruptEnableSpi(void)
{
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_ISR_ENABLE_MASK, RF_MCU_SPI_ISR_ENABLE_ALL_MASK);
}


void RfMcu_InterruptDisableSpi(void)
{
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_ISR_ENABLE_MASK, RF_MCU_SPI_ISR_DISABLE_ALL_MASK);
}


uint16_t RfMcu_InterruptEnGetSpi(void)
{
    return (uint16_t)(RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_ISR_ENABLE_MASK) & COMM_SUBSYSTEM_INT_ALL_MASK);
}


void RfMcu_InterruptEnSetSpi(uint16_t int_enable)
{
    uint8_t value = (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_ISR_ENABLE_MASK) & (~COMM_SUBSYSTEM_INT_ALL_MASK)) | int_enable;
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_ISR_ENABLE_MASK, value);
}


void RfMcu_InterruptClearSpi(uint32_t value)
{
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_ISR_STATE_CLR, (value & COMM_SUBSYSTEM_INT_ALL_MASK));
}


bool RfMcu_RxQueueIsReadySpi(void)
{
    return (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_RXQ_CMD_STATE) & RF_MCU_SPI_RX_DATA_Q_MASK ? TRUE : FALSE);
}


uint16_t RfMcu_RxQueueReadSpi(uint8_t *rx_data, RF_MCU_RXQ_ERROR *rx_queue_error)
{
    uint16_t length;
    if (RfMcu_RxQueueIsReadySpi() == FALSE)
    {
        *rx_queue_error = RF_MCU_RXQ_NOT_AVAILABLE;
        return 0;
    }

    length = (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_RX_LENGTH) |
              ((RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_RX_INFO) & RF_MCU_SPI_RX_Q_MSB_MASK) << RF_MCU_SPI_RX_Q_MSB_OFFSET));

    if (length > RF_MCU_SPI_BUFF_SIZE)
    {
        *rx_queue_error = RF_MCU_RXQ_NOT_AVAILABLE;
        return 0;
    }

    RfMcu_IoGetSpi(RF_MCU_RX_DATA_Q_ID, rx_data, length);

    *rx_queue_error = RF_MCU_RXQ_GET_SUCCESS;
    return length;
}


bool RfMcu_EvtQueueIsReadySpi(void)
{
    return (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_RXQ_CMD_STATE) & RF_MCU_SPI_RX_EVENT_Q_MASK ? TRUE : FALSE);
}


uint16_t RfMcu_EvtQueueReadSpi(uint8_t *evt, RF_MCU_RX_CMDQ_ERROR *rx_evt_error)
{
    uint16_t length;
    if (RfMcu_EvtQueueIsReadySpi() == FALSE)
    {
        *rx_evt_error = RF_MCU_RX_CMDQ_NOT_AVAILABLE;
        return 0;
    }

    length = (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_CMD_LENGTH_1) |
              ((RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_CMD_LENGTH_2) & RF_MCU_SPI_RX_Q_MSB_MASK) << RF_MCU_SPI_RX_Q_MSB_OFFSET));

    if (length > RF_MCU_SPI_BUFF_SIZE)
    {
        *rx_evt_error = RF_MCU_RX_CMDQ_NOT_AVAILABLE;
        return 0;
    }

    RfMcu_IoGetSpi(RF_MCU_RX_EVENT_Q_ID, evt, length);

    *rx_evt_error = RF_MCU_RX_CMDQ_GET_SUCCESS;

    return length;
}


bool RfMcu_TxQueueIsOccupiedSpi(uint8_t queue_id)
{
    return ((RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_TXQ_STATE) >> queue_id) & 0x01 ? FALSE : TRUE);
}


bool RfMcu_TxQueueFullCheckSpi(void)
{
    return (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_TXQ_STATE) & 0x7F ? FALSE : TRUE);
}


RF_MCU_TXQ_ERROR RfMcu_TxQueueSendSpi(uint8_t queue_id, const uint8_t *tx_data, uint32_t data_length)
{
    if (RfMcu_TxQueueIsOccupiedSpi(queue_id))
    {
        return RF_MCU_TXQ_FULL;
    }

    RfMcu_IoSetSpi(queue_id, tx_data, data_length);

    RfMcu_SpiGetSFR(0x0); // Workaround for sleep

    return RF_MCU_TXQ_SET_SUCCESS;
}


bool RfMcu_CmdQueueFullCheckSpi(void)
{
    return (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_TXQ_STATE) & RF_MCU_SPI_CMD_Q_READY_MASK ? FALSE : TRUE);
}


RF_MCU_TX_CMDQ_ERROR RfMcu_CmdQueueSendSpi(const uint8_t *cmd, uint32_t cmd_length)
{
    if (RfMcu_CmdQueueFullCheckSpi())
    {
        return RF_MCU_TX_CMDQ_FULL;
    }

    RfMcu_IoSetSpi(COMM_SUBSYS_TX_CMD_QUEUE_ID, cmd, cmd_length);

    return RF_MCU_TX_CMDQ_SET_SUCCESS;
}


RF_MCU_STATE RfMcu_McuStateReadSpi(void)
{
    return (RF_MCU_STATE) RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_MCU_STATE);
}


void RfMcu_SysRdySignalWaitSpi(void)
{
    while ((RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_CHIP_STATE) & RF_MCU_SPI_SYS_READY_BIT) == 0) {};
}


RF_MCU_PWR_STATE RfMcu_PowerStateGetSpi(void)
{
    return (RF_MCU_PWR_STATE) ((RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_CHIP_STATE) >> RF_MCU_SPI_POWER_STATE_OFFSET) & RF_MCU_SPI_POWER_STATE_MASK);
}


void RfMcu_HostWakeUpMcuSpi (void)
{
    while (RfMcu_PowerStateGetSpi() != RF_MCU_PWR_STATE_NORMAL)
    {
        RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_HOST_CTRL, RF_MCU_SPI_HOST_CTRL_HOST_WAKEUP);
    }
}

void RfMcu_SpiIceModeCtrl(bool enable)
{
    uint8_t value = (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_MODE_SELECT) & ~RF_MCU_SPI_ICE_MODE_ENABLE_BIT);
    value |= (enable ? RF_MCU_SPI_ICE_MODE_ENABLE_BIT : 0);
    RfMcu_SpiSetSFR(RF_MCU_SPI_SFR_ADDR_MODE_SELECT, value);
}

void RfMcu_SpiIsrHandler(COMM_SUBSYSTEM_ISR_t isr_cb)
{
    uint8_t state = (RfMcu_SpiGetSFR(RF_MCU_SPI_SFR_ADDR_ISR_STATE));

    if ((state & ~RF_MCU_SPI_ISR_ENABLE_ALL_MASK))
    {
        RfMcu_InterruptClearSpi(state & ~RF_MCU_SPI_ISR_ENABLE_ALL_MASK);
    }

    state &= COMM_SUBSYSTEM_INT_STATUS_MASK;

    if (state && isr_cb != NULL)
    {
        isr_cb(state);
    }
}

void RfMcu_SpiInit(void)
{
    qspi_transfer_mode_t    spi0_config_mode;

    /*Set mode0, SPI clock is 8MHz */
    spi0_config_mode.SPI_BIT_ORDER = RF_MCU_SPI_ORDER;

    spi0_config_mode.SPI_CPOL      = RF_MCU_SPI_ACTIVE_CHOICE;
    spi0_config_mode.SPI_CPHA      = RF_MCU_SPI_ACTIVE_CHOICE;

    spi0_config_mode.SPI_CS_POL    = RF_MCU_SPI_ACTIVE_CHOICE;      /*CS low active, idled in high */
    spi0_config_mode.SPI_MASTER    = RF_MCU_SPI_MODE;               /*controller spi as SPI host*/
    spi0_config_mode.SPI_CS        = RF_MCU_SPI_TARGET_SLAVE;

    spi0_config_mode.SPI_CLK       = RF_MCU_SPI_CLOCK;

    qspi_init(RF_MCU_SPI_SELECT, &spi0_config_mode);

#if (RF_MCU_SPI_PORT_SELECT == SPI_PORT_SELECT_SET_1)
    pin_set_mode(6, MODE_QSPI0);    /*SPI SCLK*/
    pin_set_mode(7, MODE_QSPI0);    /*SPI CS*/
    pin_set_mode(8, MODE_QSPI0);    /*SPI DATA0*/
    pin_set_mode(9, MODE_QSPI0);    /*SPI DATA1*/
    pin_set_drvopt(6, 2);           /* Make driving strength weaker to avoid that glitch influence SPI operation */
#elif (RF_MCU_SPI_PORT_SELECT == SPI_PORT_SELECT_SET_2)
    pin_set_mode(28, MODE_QSPI0);   /*SPI SCLK*/
    pin_set_mode(29, MODE_QSPI0);   /*SPI CS*/
    pin_set_mode(30, MODE_QSPI0);   /*SPI DATA0*/
    pin_set_mode(31, MODE_QSPI0);   /*SPI DATA1*/
    pin_set_drvopt(28, 2);          /* Make driving strength weaker to avoid that glitch influence SPI operation */
#endif
}
#endif /* CFG_RF_MCU_CTRL_TYPE */
