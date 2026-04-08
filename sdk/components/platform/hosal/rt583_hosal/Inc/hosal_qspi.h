/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/** @file hosal_spi.h
 * @license
 * @description
 */

#ifndef HOSAL_SPI_H
#define HOSAL_SPI_H

#include "qspi.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup HOSAL_SPI Hosal spi
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal spi definitions, structures, and functions
 * @{
 */

/**
 * \brief           hosal qspi const defined
 */
#define MAX_NUMBER_OF_QSPI 2
#define QSPI0_BASE_CLK     20

/**
 * \brief           hosal qspi0 pin defined
 */
#define QSPI0_DATA2 14
#define QSPI0_DATA3 15
#define QSPI0_SCLK  6
#define QSPI0_CS0   7
#define QSPI0_DATA0 8
#define QSPI0_DATA1 9

/**
 * \brief           hosal qspi1 pin defined
 */
#define QSPI1_DATA2 14
#define QSPI1_DATA3 15
#define QSPI1_SCLK  28
#define QSPI1_CS0   29
#define QSPI1_DATA0 30
#define QSPI1_DATA1 31

/**
 * \brief           hosal qspi callback function defined
 */
typedef void (*hosal_qspi_callback_t)(void* p_arg);

/**
 * \brief           hosal qspi id  defined
 */
typedef enum {
    HOSAL_QSPI_ID_0 = 0,
    HOSAL_QSPI_ID_1 = 1,
} hosal_qspi_id_t;

/**
 * \brief           hosal qspi operaiont mode defined
 */
typedef enum {
    HOSAL_QSPI_NORMAL_SPI = QSPI_NORMAL_SPI,
    HOSAL_QSPI_DUAL_SPI = QSPI_DUAL_SPI,
    HOSAL_QSPI_QUAD_SPI = QSPI_QUAD_SPI,
} hosal_qspi_transfer_t;

/**
 * \brief           hosal qspi dataset enum defined
 */
typedef enum {
    HOSAL_QSPI_DATAWIDTH_SET,
    HOSAL_QSPI_DATAWIDTH_GET,
    HOSAL_QSPI_BITORDER_SET,
    HOSAL_QSPI_BITORDER_GET,
    HOSAL_QSPI_PHASE_SET,
    HOSAL_QSPI_PHASE_GET,
    HOSAL_QSPI_POLARITY_SET,
    HOSAL_QSPI_POLARITY_GET,
    HOSAL_QSPI_SLAVESELECT_SET,
    HOSAL_QSPI_SLAVESELECT_GET,
    HOSAL_QSPI_SLAVE_POLARTITY_SET,
    HOSAL_QSPI_SLAVE_POLARTITY_GET,
    HOSAL_QSPI_BAUD_SET,
    HOSAL_QSPI_BAUD_GET,
    HOSAL_QSPI_MODE_SET,
    HOSAL_QSPI_MODE_GET,
} hosal_qspi_dataset_t;

/**
 * \brief           hosal qspi clock enum defined
 */
typedef enum {
    HOSAL_QSPI_BAUDRATE_32M = QSPI_CLK_32M,
    HOSAL_QSPI_BAUDRATE_16M = QSPI_CLK_16M,
    HOSAL_QSPI_BAUDRATE_8M = QSPI_CLK_8M,
    HOSAL_QSPI_BAUDRATE_4M = QSPI_CLK_4M,
    HOSAL_QSPI_BAUDRATE_2M = QSPI_CLK_2M,
    HOSAL_QSPI_BAUDRATE_1M = QSPI_CLK_1M,
} hosal_qspi_baudrate_t;

/**
 * \brief           hosal qspi data bit size enum defined
 */
typedef enum {
    HOSAL_QSPI_DATASIZE_8 = QSPI_BITSIZE_8,
    HOSAL_QSPI_DATASIZE_32 = QSPI_BITSIZE_32,
} hosal_qspi_bitsize_t;

/**
 * \brief           hosal qspi bit order  enum defined
 */
typedef enum {
    HOSAL_QSPI_LSB = SPI_LSB_ORDER,
    HOSAL_QSPI_MSB = SPI_MSB_ORDER,
} hosal_qspi_bitorder_t;

/**
 * \brief           hosal qspi polarity  enum defined
 */
typedef enum {
    HOSAL_QSPI_POLARITY_LOW = 0,
    HOSAL_QSPI_POLARITY_HIGH = 1,
} hosal_qspi_polarity_t;

/**
 * \brief           hosal qspi phase  enum defined
 */
typedef enum {
    HOSAL_QSPI_PHASE_1EDGE = 0,
    HOSAL_QSPI_PHASE_2EDGE = 1,
} hosal_qspi_phase_t;

/**
 * \brief           hosal qspi chip selectt atcive enum defined
 */
typedef enum {
    HOSAL_QSPI_CHIPSEL_ACTIVE_LOW = SPI_CHIPSEL_ACTIVE_LOW,
    HOSAL_QSPI_CHIPSEL_ACTIVE_HIGH = SPI_CHIPSEL_ACTIVE_HIGH,
} hosal_qspi_cs_polarity_t;

/**
 * \brief           hosal qspi slave select enum defined
 */
typedef enum {
    HOSAL_QSPI_SELECT_SLAVE_0 = SPI_SELECT_SLAVE_0,
    HOSAL_QSPI_SELECT_SLAVE_1 = SPI_SELECT_SLAVE_1,
    HOSAL_QSPI_SELECT_SLAVE_2 = SPI_SELECT_SLAVE_2,
    HOSAL_QSPI_SELECT_SLAVE_3 = SPI_SELECT_SLAVE_3,
} hosal_qspi_slave_select_t;

/**
 * \brief           hosal qspi mode enum defined
 */
typedef enum {
    HOSAL_QSPI_SLAVE_MODE = SPI_SLAVE_MODE,
    HOSAL_QSPI_MASTER_MODE = SPI_MASTER_MODE,
} hosal_qspi_mode_t;

/**
 * \brief           hosal qspi operation status enum defined
 */
typedef enum {
    HOSAL_QSPI_STATE_UNINIT = QSPI_STATE_UNINIT,
    HOSAL_QSPI_STATE_IDLE = QSPI_STATE_IDLE,
    HOSAL_QSPI_STATE_TRANSFER = QSPI_STATE_TRANSFER,
    HOSAL_QSPI_STATE_SETUP = QSPI_STATE_SETUP,
    HOSAL_QSPI_STATE_WRITE_DMA = QSPI_STATE_WRITE_DMA,
    HOSAL_QSPI_STATE_READ_DMA = QSPI_STATE_READ_DMA,
} hosal_qspi_state_t;

/**
 * \brief           hosal qspi return status enum defined
 */
typedef enum {
    HOSAL_QSPI_SUCCESS,
    HOSAL_QSPI_INVALID_PARAM,
    HOSAL_QSPI_INVALID_REQUEST,
    HOSAL_QSPI_BUSY,
    HOSAL_QSPI_NO_INIT,
    HOSAL_QSPI_ERROR,
    HOSAL_QSPI_TIMEOUT,
} hosal_qspi_status_t;

/**
 * \brief           hosal qspi callback typedef struce defined
 */
typedef enum {
    HOSAL_QSPI_TRANSFER_DMA,
} hosal_qspi_cb_type_t;

/**
 * \brief           hosal qspi paramater struce defined
 */
typedef struct {
    uint8_t qspi_id;
    uint8_t clk_pin;
    uint8_t cs_pin;
    uint8_t mosi_pin;
    uint8_t miso_pin;
    uint8_t data2;
    uint8_t data3;
    hosal_qspi_baudrate_t baud_rate;
    hosal_qspi_bitsize_t data_width;
    hosal_qspi_bitorder_t bit_order;
    hosal_qspi_polarity_t polarity;
    hosal_qspi_phase_t phase;
    hosal_qspi_cs_polarity_t cs_polarity;
    hosal_qspi_slave_select_t slave_select;
    hosal_qspi_mode_t mode;
} hosal_qspi_config_t;

/**
 * \brief            hosal qspi device struct defined
 */
typedef struct hosal_qspi_dev {
    qspi_t* instance;
    IRQn_Type irq_num;
    hosal_qspi_config_t config;
    uint8_t* tx_buf;
    uint8_t* rx_buf;
    uint8_t* cmd;
    uint16_t tx_xfer_count;
    uint16_t rx_xfer_count;
    uint16_t cmd_length;
    uint16_t data_length;
    hosal_qspi_callback_t transfer_cb;
    void* p_transfer_arg;
    uint8_t qspi_state;
} hosal_qspi_dev_t;

/**
 * \brief           hosal qspi access flash command struct defined
 */
typedef struct {
    uint8_t* cmd_buf;
    uint16_t cmd_length;
    uint8_t* write_buf;
    uint16_t write_length;
    uint8_t* read_buf;
    uint16_t read_length;
    uint8_t transfer_mode;
} hosal_qspi_flash_command_t;


/**
 * \brief           hosal qspi struce defined
 */
typedef hosal_qspi_status_t (*hosal_qspi_dev_dataset)(
    hosal_qspi_dev_t* qspi_dev, void* p_arg);

/**
 * \brief           hosal qspi device defined
 */
#define HOSAL_QSPI_DEV_DECL(dev, qspi, clk, cs, mosi, miso, d2, d3, baud, mod) \
    hosal_qspi_dev_t dev = {                                                   \
        .instance = qspi,                                                      \
        .config =                                                              \
            {                                                                  \
                .clk_pin = clk,                                                \
                .cs_pin = cs,                                                  \
                .mosi_pin = mosi,                                              \
                .miso_pin = miso,                                              \
                .data2 = d2,                                                   \
                .data3 = d3,                                                   \
                .baud_rate = baud,                                             \
                .data_width = HOSAL_QSPI_DATASIZE_8,                           \
                .bit_order = HOSAL_QSPI_MSB,                                   \
                .polarity = HOSAL_QSPI_POLARITY_LOW,                           \
                .phase = HOSAL_QSPI_PHASE_1EDGE,                               \
                .cs_polarity = HOSAL_QSPI_CHIPSEL_ACTIVE_LOW,                  \
                .mode = mod,                                                   \
            },                                                                 \
        .transfer_cb = NULL,                                                   \
    };

/**
 * \brief          qspi initinal function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \return         hosal_qspi_status_t status
 *
 */
hosal_qspi_status_t hosal_qspi_init(hosal_qspi_dev_t* qspi_dev);

/**
 * \brief           qspi transfer functino function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      txbuf    qspi transfer buffer point.
 * \param[in]      rxbuf    qspi receiver buffer point.
 * \param[in]      size    qspi transfer size.
 * \return         hosal_qspi_status_t status
 *
 */
hosal_qspi_status_t hosal_qspi_transfer_dma(hosal_qspi_dev_t* qspi_dev,
                                            uint8_t* txbuf, uint8_t* rxbuf,
                                            uint16_t size);

/**
 * \brief           qspi flash command function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      req    qspi flash commant struct.
 * \return         hosal_qspi_status_t status
 *
 */
hosal_qspi_status_t hosal_qspi_flash_command(hosal_qspi_dev_t* qspi_dev,
                                             hosal_qspi_flash_command_t* req);

/**
 * \brief          qspi transfer polling function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      txbuf        qspi transfer buffer point.
 * \param[in]      rxbuf        qspi receiver buffer point.
 * \param[in]      size         qspi transfer size.
 * \param[in]      timeout      qspi transfer timeout.
 * \return         hosal_qspi_status_t status
 *
 */
hosal_qspi_status_t hosal_qspi_transfer_pio(hosal_qspi_dev_t* qspi_dev,
                                            uint8_t* txbuf, uint8_t* rxbuf,
                                            uint16_t size, uint32_t timeout);

/**
 * \brief          qapi io control function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      txbuf        qspi transfer buffer point.
 * \param[in]      rxbuf        qspi receiver buffer point.
 * \param[in]      size         qspi transfer size.
 * \param[in]      timeout      qspi transfer timeout.
 * \return         hosal_qspi_status_t status
 *
 */                                            
hosal_qspi_status_t hosal_qspi_ioctl(hosal_qspi_dev_t* qspi_dev, int ctl,
                                     void* p_arg);

/**
 * \brief          qspi de initinal function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \return         hosal_qspi_status_t status
 *
 */                                     
hosal_qspi_status_t hosal_qspi_deinit(hosal_qspi_dev_t* qspi_dev);

/**
 * \brief          register callback function.
 * \param[in]      qspi_dev             qspi struct identifier
 * \param[in]      callback_type        qspi callback function type
 * \param[in]      pfn_callback         qspi callback function
 * \param[in]      arg                  qspi callback function paramagter.
 * \return         hosal_qspi_status_t status
 *
 */
hosal_qspi_status_t
hosal_qspi_callback_register(hosal_qspi_dev_t* qspi_dev,
                             hosal_qspi_cb_type_t callback_type,
                             hosal_qspi_callback_t pfn_callback, void* arg);

/**
 * \brief          qspi idle callback function.
 * \param[in]      qspi_dev             qspi struct identifier
 * \return         hosal_qspi_status_t status
 *
 */                        
hosal_qspi_status_t hosal_qspi_idle(hosal_qspi_dev_t* qspi_dev);
/**
 * \brief          qspi abort function.
 * \param[in]      qspi_dev            qspi struct identifier
 * \return         hosal_qspi_status_t status
 *
 */ 
hosal_qspi_status_t hosal_qspi_abort(hosal_qspi_dev_t* qspi_dev);

/**
 * \brief          qspi id get function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \return         id number *
 */ 
uint8_t hosal_qspi_id_get(hosal_qspi_dev_t* qspi_dev);

/**
 * \brief          qspi hanlder get function.
 * \param[in]      id     id
 * \return         hosal_qspi_status_t status
 *
 */ 
hosal_qspi_dev_t* hosal_qspi_handle_get(uint8_t id);

/**
 * \brief          qspi hanlder get function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      id     id
 * \return         hosal_qspi_status_t status
 *
 */ 
hosal_qspi_status_t hosal_qspi_handle_set(hosal_qspi_dev_t* qspi_dev,
                                          uint8_t id);

/**
 * \brief          qspi dma write function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      req     flash command struct
 * \return         hosal_qspi_status_t status
 *
 */ 
hosal_qspi_status_t hosal_qspi_write_dma(hosal_qspi_dev_t* qspi_dev,
                                         hosal_qspi_flash_command_t* req);

/**
 * \brief          qspi dma read function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      req     flash command struct
 * \return         hosal_qspi_status_t status
 *
 */ 
hosal_qspi_status_t hosal_qspi_read_dma(hosal_qspi_dev_t* qspi_dev,
                                        hosal_qspi_flash_command_t* req);

/**
 * \brief          qspi transfer command function.
 * \param[in]      qspi_dev     qspi struct identifier
 * \param[in]      req     flash command struct
 * \return         hosal_qspi_status_t status
 *
 */                                     
hosal_qspi_status_t hosal_qspi_tranfer_command(hosal_qspi_dev_t* qspi_dev,
                                               hosal_qspi_flash_command_t* req);


/*@}*/ /* end of RT58X_HOSAL HOSAL_SPI */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_SPI_H */
