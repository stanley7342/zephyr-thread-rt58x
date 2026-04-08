/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            uart_drv.h
 * \brief           uart driver header file
 */
/*
 * Author:
 */

#ifndef UART_DRV_H
#define UART_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup UART_DRV Uart driver
 * \ingroup RT58X_DRIVER
 * \brief  Define Uart driver definitions, structures, and functions
 * @{
 */

typedef int (*uart_callback_t)(void* p_arg);


/* 
 * Notice: This define is not hardware register bit setting, so don't be confused with
 * real hardware bit setting.
 */
#define CMSPAR (1 << 2) /*sticky, hidden defined, not original linux*/
#define PARENB (1 << 1) /*partity enabled, hidden defined, not original linux */
#define PARODD (1 << 0) /*partity ODD, hidden defined, not original linux */


/**
 * \brief            UART
 */
typedef enum {
    UART_TX_IT_ID = 0,                          /*!<*/
    UART_RX_IT_ID = 1,                          /*!<*/
    UART_DMA_TX_IT_ID = 2,                      /*!<*/
    UART_DMA_RX_IT_ID = 3,                      /*!<*/
    UART_LSR_IT_ID = 4,                         /*!<*/
    UART_CB_ID_MAX,
} uart_callfun_id_t;

/**
 * \brief            UART id Type Definitions
 */
typedef enum {
    UART0_ID = 0,                               /*!< UART0  */
    UART1_ID = 1,                               /*!< UART1  */
    UART2_ID = 2,                               /*!< UART2  */
    UART_ID_MAX = 3                             /*!< UART2  */
} uart_id_t;

/**
 * \brief           UART PARITY Type Definitions
 */
typedef enum {
    UART_PARITY_NONE = 0,                         /*!< NONE PARITY  */
    UART_PARITY_EVEN = (PARENB),                  /*!< EVEN PARITY  */
    UART_PARITY_ODD = (PARENB | PARODD),          /*!< ODD PARITY   */
    UART_PARITY_SPACE = (CMSPAR | PARENB),        /*!< PARITY SPACE */
    UART_PARITY_MARK = (CMSPAR | PARENB | PARODD) /*!< PARITY MARK  */
} uart_parity_t;

/**
 * \brief           Types of UART flow control modes.
 */
typedef enum {
    UART_HWFC_DISABLED = 0,                     /*!< HW flow control disabled.   */
    UART_HWFC_ENABLED                           /*!< HW flow control enabled.    */
} uart_hwfc_t;

/**
 * \brief           Types of UART stop bit modes.
 */
typedef enum {
    UART_STOPBIT_ONE = 0,                       /*!< 1 STOP BIT  */
    UART_STOPBIT_TWO                            /*!< 2 STOP BIT  */
} uart_stopbit_t;

/**
 * \brief           Types of UART baudrate modes.
 */
#define UART_BAUDRATE_Baud2400    (1667)
#define UART_BAUDRATE_Baud4800    (833)
#define UART_BAUDRATE_Baud9600    (417)
#define UART_BAUDRATE_Baud14400   (278)
#define UART_BAUDRATE_Baud19200   (208)
#define UART_BAUDRATE_Baud28800   (139)
#define UART_BAUDRATE_Baud38400   (104)
#define UART_BAUDRATE_Baud57600   (69)
#define UART_BAUDRATE_Baud76800   (52)
#define UART_BAUDRATE_Baud115200  (35)
#define UART_BAUDRATE_Baud500000  (8)
#define UART_BAUDRATE_Baud1000000 (4)
#define UART_BAUDRATE_Baud2000000 (2)

/**
 * \brief           Baudrates supported by UART.
 */
typedef enum {
    UART_BAUDRATE_2400 = 2400,                  /*!< 2400 baud.  */
    UART_BAUDRATE_4800 = 4800,                  /*!< 4800 baud.  */
    UART_BAUDRATE_9600 = 9600,                  /*!< 9600 baud.  */
    UART_BAUDRATE_14400 = 14400,                /*!< 14400 baud. */
    UART_BAUDRATE_19200 = 19200,                /*!< 19200 baud. */
    UART_BAUDRATE_28800 = 28800,                /*!< 28800 baud. */
    UART_BAUDRATE_38400 = 38400,                /*!< 38400 baud. */
    UART_BAUDRATE_57600 = 57600,                /*!< 57600 baud.  */
    UART_BAUDRATE_76800 = 76800,                /*!< 76800 baud.  */
    UART_BAUDRATE_115200 = 115200,              /*!< 115200 baud. */
    UART_BAUDRATE_500000 = 500000,              /*!< 500000 baud. */
    UART_BAUDRATE_1000000 = 1000000,            /*!< 1000000 baud. */
    UART_BAUDRATE_2000000 = 2000000,            /*!< 2000000 baud. */
} uart_baudrate_t;

/**
 * \brief           Data bits for one character supported by UART.
 */
typedef enum {
    UART_DATA_BITS_5 = 0,                       /*!< 5-bits.  */
    UART_DATA_BITS_6,                           /*!< 6-bits.  */
    UART_DATA_BITS_7,                           /*!< 7-bits.  */
    UART_DATA_BITS_8                            /*!< 8-bits.  */
} uart_databits_t;

/**
 * \brief            UART EVENT
 * \details          UART_EVENT_RX_TIMEOUT is not error, it just signal the event
 *                   "during receive when idle time is detected between consecutive characters."
 */

#define UART_EVENT_TX_DONE (1UL << 0)           /*!< Send completed; however UART may still transmit data*/
#define UART_EVENT_RX_DONE (1UL << 1)           /*!< Receive completed*/
#define UART_EVENT_RX_OVERFLOW (1UL << 2)       /*!< Receive data overflow*/
#define UART_EVENT_RX_TIMEOUT (1UL << 3)        /*!< Receive character timeout (optional)*/
#define UART_EVENT_RX_BREAK (1UL << 4)          /*!< Break detected on receive*/
#define UART_EVENT_RX_FRAMING_ERROR (1UL << 5)  /*!< Framing error detected on receive*/
#define UART_EVENT_RX_PARITY_ERROR (1UL << 6)   /*!< Parity error detected on receive*/
#define UART_EVENT_CTS (1UL << 7)               /*!< CTS state changed (optional)*/
#define UART_EVENT_RX_RECV (1UL << 8)           /*!< Receive data*/

#define UART_EVENT_DMA_TX_DONE (1UL << 9)       /*!< Send completed;*/
#define UART_EVENT_DMA_RX_DONE (1UL << 10)      /*!< Receive completed*/

#define UART_POWER_OFF (0)                      /*!< Power off the uart device*/
#define UART_POWER_ON  (1)                      /*!< Power on the uart device*/

#define UART_FIFO_DISABLE (0)                   /*!<*/
#define UART_LCR_CLEAR    (0)                   /*!<*/
#define UART_LEN_CLEAR    (0)                   /*!<*/

/**
 * \brief           Structure for UART status.
 */
typedef struct _uart_status {
    uint32_t tx_busy : 1;                       /*!< Transmitter busy flag*/
    uint32_t rx_busy : 1;                       /*!< Receiver busy flag*/
    uint32_t tx_underflow : 1;                  /*!< Transmit data underflow detected  (cleared on start of next send operation) */
    uint32_t rx_overflow : 1;                   /*!< Receive data overflow detected  (cleared on start of next receive operation */
    uint32_t rx_break : 1;                      /*!< Break detected on receive (cleared on start of next receive operation)*/
    uint32_t rx_framing_error : 1;              /*!< Framing error detected on receive (cleared on start of next receive operation)*/
    uint32_t rx_parity_error : 1;               /*!< Parity error detected on receive  (cleared on start of next receive operation)*/
    uint32_t reserved0     : 1;
    uint32_t msr_cts_state : 1;                 /*!< modem CTS status*/
    uint32_t reserved1     : 23;
} uart_status;

/**
 * \brief           Structure for UART configuration.
 */
typedef struct {
    uint8_t uart_id;                            /*!< UART id */
    uint32_t baud_rate;                         /*!< UART baud rate */
    uart_databits_t data_width;                 /*!< UART data width */
    uart_parity_t parity;                       /*!< UART parity bit */
    uart_stopbit_t stop_bits;                   /*!< UART stop btis */
    uart_hwfc_t flow_control;                   /*!< UART flow control */
} uart_config_t;

/**
 * \brief           UART interrupt event handler.
 * \param[in]       event: Signal for UART Event.
 * \param[in]       p_context: Context passed to interrupt handler, set on initialization.
 */
typedef void (*uart_event_handler_t)(uint32_t event, void* p_context);

/**
 * \brief           Uart config paramater
 * \param[in]       uart_cfg:
 * \return          STATUS
 */
uint32_t uart_init(uart_config_t uart_cfg);

/**
 * \brief           Set uart interrupt 
 * \param[in]       uart_id: uart identification
 * \param[in]       enable: enable interrupt flag
 * \param[in]       interrupt_flag: interrupt value
 * \return          None
 */
uint32_t uart_set_interrupt(uint32_t uart_id, uint32_t enable,
                            uint8_t interrupt_flag);

/**
 * \brief           Enabel uart interrupt
 * \param[in]       uart_id:         uart identification
 * \param[in]       enable:          enable interrupt flag
 * \return          None
 */
uint32_t uart_enable_irq(uint32_t uart_id, uint32_t enable);

/**
 * \brief           Uart unintintal function
 * \param[in]       uart_id: uart identification
 * \return          None
 */
uint32_t uart_uninit(uint32_t uart_id);

/**
 * \brief           Uart fifo reset
 * \param[in]       uart_id: uart identification
 * \param[in]       fifo_sel: uart fifo clear slection 
 * \return None
 */
uint32_t uart_fifo_reset(uint32_t uart_id, uint32_t fifo_sel);

/**
 * \brief           Register callback function
 * \param[in]       uart_id: uart identification
 * \param[in]       cb_id: uart callback function identification
 * \param[in]       uart_callback_func: uart callback function 
 * \param[in]       arg: uart callback paramater
 * \return          None
 */
uint32_t uart_register_callback(uint32_t uart_id, uart_callfun_id_t cb_id,
                                uart_callback_t uart_callback_func, void* arg);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \param[in]       p_data: uart data buffer point
 * \param[in]       length: uart data buffer length
 * \return          None
 */
uint32_t uart_transfer_dma(uint32_t uart_id, uint8_t* p_data, uint32_t length);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \param[in]       p_data: uart data buffer point
 * \param[in]       length: uart data buffer length
 * \return          None
 */
uint32_t uart_transfer_pol(uint32_t uart_id, uint8_t* p_data, uint32_t length);

/**
 * \brief
 * \param[in]       uart_dev: uart config paramater struct
 * \return None
 */
uint32_t uart_get_config(uart_config_t* uart_dev);

/**
 * \brief
 * \param[in]       uart_dev: uart config paramater struct
 * \return None
 */
uint32_t uart_set_config(uart_config_t uart_dev);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \return          None
 */
bool uart_receive_ready(uint32_t uart_id);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \return          None
 */
bool uart_transfer_ready(uint32_t uart_id);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \param[in]       data: uart data buffer point
 * \return          None
 */
uint32_t uart_receive(uint32_t uart_id, uint8_t* data);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \param[in]       data: uart data buffer point
 * \return          None
 */
uint32_t uart_receive_it(uint32_t uart_id, uint8_t* data);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \param[in]       data: uart data
 * \return          None
 */
uint32_t uart_transfer(uint32_t uart_id, uint8_t data);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \param[in]       data: uart data
 * \return          None
 */
uint32_t uart_transfer_it(uint32_t uart_id, uint8_t data);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \param[in]       p_data: uart data buffer point
 * \param[in]       length: uart receive data length
 * \return          None
 */
uint32_t uart_receive_dma(uint32_t uart_id, uint8_t* p_data, uint32_t length);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \return          None
 */
uint32_t uart_receive_dma_ready(uint32_t uart_id);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \return          None
 */
uint32_t uart_transfer_dma_ready(uint32_t uart_id);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \return          None
 */
void uart_receive_abort(uint32_t uart_id);

/**
 * \brief
 * \param[in]       uart_id: uart identification
 * \return          None
 */
void uart_transfer_abort(uint32_t uart_id);

/**
 * \brief
 * \param[in]       uart_id : uart identification
*  \param[in]       sbc_state: uart set break status 
 * \return          None
 */
uint32_t uart_set_break(uint32_t uart_id, uint32_t sbc_state);

/**
 * \brief
 * \param[in]       uart_id: uart identification
*  \param[in]       rts_state: uart set mode status 
 * \return          None
 */
uint32_t uart_set_modem_status(uint32_t uart_id, uint32_t rts_state);

/**
 * \brief
 * \param[in]       uart_id:
 * \return          None
 */
bool uart_receive_dma_finish(uint32_t uart_id);

/**
 * \brief
 * \param[in]       uart_id:
 * \return          None
 */
bool uart_transfer_dma_finish(uint32_t uart_id);

/*@}*/ /* end of RT58X_DRIVER UART_DRV */

#ifdef __cplusplus
}
#endif

#endif /* End of UART_DRV_H */
