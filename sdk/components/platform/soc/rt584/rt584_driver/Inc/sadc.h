/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           sadc.h
 * \brief          Sadc driver header file
 */
/*
 * This file is part of library_name.
 * Author:          Kc.tseng
 */

#ifndef SADC_H
#define SADC_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"


/**
 * \defgroup        SADC Sadc
 * \ingroup         RT584_DRIVER
 * \brief           Define Sadc definitions, structures, and functions
 * @{
 */

/**
 * \brief           SADC test mode enable.
 */
#define SADC_TEST_MODE          0

/**
 * \brief           SADC test mode test value.
 */
#if (SADC_TEST_MODE == 1)
#define SADC_TEST_VALUE         0x5AF
#endif

/**
 * \brief           SADC monitor threshold default definition.
 */
#define SADC_MONITOR_LOW_THD_DEFAULT   0
#define SADC_MONITOR_HIGH_THD_DEFAULT  0x3FFF

/**
 * \brief           SADC channel register offset definition.
 */
#define SADC_CH_REG_OFFSET      4    /*unit: 4-bytes*/

/**
 * \brief           SADC interrupt definition.
 */
#define SADC_INT_DISABLE_ALL    0xFFFFFFFF
#define SADC_INT_CLEAR_ALL      0xFFFFFFFF

/**
 * \brief           SADC register default value definition.
 */
#define SADC_PNSEL_CH_REG_DEFAULT      0x240000FF
#define SADC_SET_CH_REG_DEFAULT        0x80000000
#define SADC_THD_CH_REG_DEFAULT        0x3FFF0000

/**
 * \brief           SADC gain and pull value definition.
 */
#define SADC_GAIN_AIO  0x02
#define SADC_GAIN_TEMP 0x03
#define SADC_GAIN_VBAT 0x07
#define SADC_PULL_AIO  0x00
#define SADC_PULL_TEMP 0x00
#define SADC_PULL_VBAT 0x03

/**
 * \brief           SADC value definition.
 */
typedef int32_t sadc_value_t;

/**
 * \brief           SADC resolution bit definition.
 */
typedef enum {
    SADC_RES_8BIT  = 0,                         /*!< 8 bit resolution. */
    SADC_RES_10BIT = 1,                         /*!< 10 bit resolution. */
    SADC_RES_12BIT = 2,                         /*!< 12 bit resolution. */
    SADC_RES_14BIT = 3,                         /*!< 14 bit resolution. */
} sadc_config_resolution_t;

/**
 * \brief           SADC oversample ratio definition.
 */
typedef enum {
    SADC_OVERSAMPLE_0   = 0,                    /*!< No oversampling*/
    SADC_OVERSAMPLE_2   = 1,                    /*!< Oversampling ratio multiple 2*/
    SADC_OVERSAMPLE_4   = 2,                    /*!< Oversampling ratio multiple 4*/
    SADC_OVERSAMPLE_8   = 3,                    /*!< Oversampling ratio multiple 8*/
    SADC_OVERSAMPLE_16  = 4,                    /*!< Oversampling ratio multiple 16*/
    SADC_OVERSAMPLE_32  = 5,                    /*!< Oversampling ratio multiple 32*/
    SADC_OVERSAMPLE_64  = 6,                    /*!< Oversampling ratio multiple 64*/
    SADC_OVERSAMPLE_128 = 7,                    /*!< Oversampling ratio multiple 128*/
    SADC_OVERSAMPLE_256 = 8,                    /*!< Oversampling ratio multiple 256*/
} sadc_config_oversample_t;

/**
 * \brief           SADC sample mode definition.
 */
typedef enum {
    SADC_SAMPLE_START = 0,                      /*!< SADC conversion is started by software
                                                 SADC start in SADC sample one shot mode*/
    SADC_SAMPLE_TIMER = 1,                      /*!< SADC conversion is started by Timer in
                                                 SADC sample timer mode*/
} sadc_config_sample_t;

/**
 * \brief           SADC source clock definition.
 */
typedef enum {
    SADC_TIMER_SYSTEM_CLK = 0,                  /*!< Select timer clock source to system
                                                 clock(48MHz/32MHz) in SADC sample timer mode*/
    SADC_TIMER_SLOW_CLK   = 1,                  /*!< Select timer clock source to slow clock (32KHz)
                                                 in SADC sample timer mode*/
} sadc_config_timer_clk_t;

/**
 * \brief           SADC channel definition.
 */
typedef enum {
    SADC_CHANNEL_0   = 0,                       /*!< Channel 0. */
    SADC_CHANNEL_1   = 1,                       /*!< Channel 1. */
    SADC_CHANNEL_2   = 2,                       /*!< Channel 2. */
    SADC_CHANNEL_3   = 3,                       /*!< Channel 3. */
    SADC_CHANNEL_4   = 4,                       /*!< Channel 4. */
    SADC_CHANNEL_5   = 5,                       /*!< Channel 5. */
    SADC_CHANNEL_6   = 6,                       /*!< Channel 6. */
    SADC_CHANNEL_7   = 7,                       /*!< Channel 7. */
    SADC_CHANNEL_8   = 8,                       /*!< Channel 8. */
    SADC_CHANNEL_9   = 9,                       /*!< Channel 9. */
    SADC_CHANNEL_MAX = 10,                      /*!< Max Channel 10. */
} sadc_config_channel_t;

/**
 * \brief           Input selection of the analog-to-digital converter.
 */
typedef enum {
    SADC_AIN_0        = 0,                      /*!< Input 0. */
    SADC_AIN_1        = 1,                      /*!< Input 1. */
    SADC_AIN_2        = 2,                      /*!< Input 2. */
    SADC_AIN_3        = 3,                      /*!< Input 3. */
    SADC_AIN_4        = 4,                      /*!< Input 4. */
    SADC_AIN_5        = 5,                      /*!< Input 5. */
    SADC_AIN_6        = 6,                      /*!< Input 6. */
    SADC_AIN_7        = 7,                      /*!< Input 7. */
    SADC_AIN_8        = 8,                      /*!< Input 8. Temperature Sensor */
    SADC_TEMPERATURE  = 8,                      /*!< Input 8. Temperature Sensor */
    SADC_AIN_9        = 9,                      /*!< Input 9. Bypass VGA */
    SADC_BYPASS_VGA   = 9,                      /*!< Input 9. Bypass VGA */
    SADC_AIN_10       = 10,                     /*!< Input 10. VBAT */
    SADC_VBAT         = 10,                     /*!< Input 10. VBAT */
    SADC_AIN_DISABLED0 = 11,                    /*!< No input selected. */
    SADC_AIN_DISABLED1 = 12,                    /*!< No input selected. */
    SADC_AIN_DISABLED2 = 13,                    /*!< No input selected. */
    SADC_AIN_DISABLED3 = 14,                    /*!< No input selected. */
    SADC_AIN_DISABLED4 = 15,                    /*!< No input selected. */
} sadc_config_input_t;

/**
 * \brief           SADC channel Input selection definition.
 */
typedef enum {
    SADC_CH_AIN0         = 0,                   /*!< Input AIO0. */
    SADC_CH_AIN1         = 1,                   /*!< Input AIO1. */
    SADC_CH_AIN2         = 2,                   /*!< Input AIO2. */
    SADC_CH_AIN3         = 3,                   /*!< Input AIO3. */
    SADC_CH_AIN4         = 4,                   /*!< Input AIO4. */
    SADC_CH_AIN5         = 5,                   /*!< Input AIO5. */
    SADC_CH_AIN6         = 6,                   /*!< Input AIO6. */
    SADC_CH_AIN7         = 7,                   /*!< Input AIO7. */
    SADC_CH_TEMPERATURE  = 8,                   /*!< Input TEMPERATURE. */
    SADC_CH_VBAT         = 9,                   /*!< Input VBAT. */
    SADC_CH_NC,                                 /*!< Input No Connect. */
} sadc_input_ch_t;

/**
 * \brief           SADC timeing definition.
 */
typedef enum {
    SADC_TACQ_EDLY_TIME_0P3US = 0,              /*!< Edly time 0.3us. */
    SADC_TACQ_EDLY_TIME_1US   = 1,              /*!< Edly time 1us. */
    SADC_TACQ_EDLY_TIME_2US   = 2,              /*!< Edly time 2us. */
    SADC_TACQ_EDLY_TIME_3US   = 3,              /*!< Edly time 3us. */
    SADC_TACQ_EDLY_TIME_4US   = 4,              /*!< Edly time 4us. */
    SADC_TACQ_EDLY_TIME_8US   = 5,              /*!< Edly time 8us. */
    SADC_TACQ_EDLY_TIME_12US  = 6,              /*!< Edly time 12us. */
    SADC_TACQ_EDLY_TIME_16US  = 7,              /*!< Edly time 16us. */
} sadc_config_tacq_edly_t;

/**
 * \brief           SADC burst enablg types.
 */
typedef enum {
    SADC_BURST_DISABLE    = 0,                  /*!< Sadc burst enable. */
    SADC_BURST_ENABLE     = 1,                  /*!< Sadc burst disable. */
} sadc_config_burst_t;

/**
 * \brief           SADC callback types.
 */
typedef enum {
    SADC_CB_DONE   = 0,                          /*!< CB generated when the buffer is filled with samples. */
    SADC_CB_SAMPLE = 1,                          /*!< CB generated when the requested channel is sampled. */
} sadc_cb_type_t;

/**
 * \brief           SADC convert status types.
 */
typedef enum {
    SADC_CONVERT_IDLE = 0,                      /*!< Sadc convert idle state. */
    SADC_CONVERT_START,                         /*!< Sadc convert start state. */
    SADC_CONVERT_DONE,                          /*!< Sadc convert done state. */
} sadc_convert_state_t;

/**
 * \brief           SADC calibration types.
 */
typedef enum {
    SADC_CALIBRATION_VBAT  = 0,                 /*!< Sadc vbat calibration. */
    SADC_CALIBRATION_AIO,                       /*!< Sadc aio calibration. */
    SADC_CALIBRATION_TEMP,                      /*!< Sadc temp calibration. */
} sadc_cal_type_t;

/**
 * \brief           SADC xdma configuration structure.
 */
typedef struct {
    uint32_t enable;                            /*!< Sadc xdma enable. */
    uint32_t start_addr;                        /*!< Sadc xdma start address. */
    uint16_t seg_size;                          /*!< Sadc xdma segment size. */
    uint16_t blk_size;                          /*!< Sadc xdma block size. */
} sadc_config_xdma_t;

/**
 * \brief           SADC timer config struct
 */
typedef struct {
    sadc_config_timer_clk_t timer_clk_src;      /*!< Sadc timer mode source timer. */
    uint32_t timer_clk_div;                     /*!< Sadc timer mode timer cloclk divide. */
} sadc_config_timer_t;

/**
 * \brief           SADC configuration.
 */
typedef struct {
    sadc_int_t               sadc_int_mask;     /*!< SADC interrupt. */
    sadc_config_resolution_t sadc_resolution;   /*!< SADC resolution. */
    sadc_config_oversample_t sadc_oversample;   /*!< SADC oversample. */
    sadc_config_xdma_t       sadc_xdma;         /*!< SADC XDMA. */
    sadc_config_sample_t     sadc_sample_mode;  /*!< SADC sample mode. */
    sadc_config_timer_t      sadc_timer;        /*!< SADC timer. */
} sadc_config_t;

/**
 * \brief           SADC channel configuration.
 */
typedef struct {
    sadc_config_channel_t   ch_sel;             /*!< SADC AIN0~AIN9 channel
                                                 Selection*/
    sadc_config_input_t     pi_sel;             /*!< SADC AIN0~AIN9 P channel
                                                 Selection*/
    sadc_config_input_t     ni_sel;             /*!< SADC AIN0~AIN9 N channel
                                                 Selection*/
    uint32_t                gain;               /*!< SADC AIN0~AIN9 Gain
                                                 settings*/
    uint32_t                pull;               /*!< SADC AIN0~AIN9 Pull high
                                                 or low*/
    sadc_config_tacq_edly_t tacq;               /*!< SADC result acquisition time
                                                 for system clock 32M*/
    sadc_config_tacq_edly_t edly;               /*!< SADC end delay time for system
                                                 clock 32M */
    sadc_config_burst_t     burst;              /*!< SADC Burst mode selection*/
    uint32_t                low_thd;            /*!< SADC Low threshold*/
    uint32_t                high_thd;           /*!< SADC High threshold*/
} sadc_channel_config_t;

/**
 * \brief           Analog-to-digital converter driver DONE cb.
 */
typedef struct {
    sadc_value_t* p_buffer;                     /*!< Pointer to buffer with converted
                                                 samples. */
    uint16_t      size;                         /*!< Number of samples in the buffer. */
} sadc_done_cb_t;

/**
 * \brief           Analog-to-digital converter driver raw cb.
 */
typedef struct {
    sadc_value_t conversion_value;              /*!< Converted sample. */
    sadc_value_t compensation_value;            /*!< Compensation sample. */
    sadc_value_t calibration_value;             /*!< Calibration sample. */
} sadc_raw_cb_t;

/**
 * \brief           Analog-to-digital converter driver SAMPLE cb.
 */
typedef struct {
    sadc_value_t value;                         /*!< Converted sample. */
    uint32_t     channel;                       /*!< Converted channel. */
} sadc_sample_cb_t;

/**
 * \brief           Analog-to-digital converter driver cb.
 */
typedef struct {
    sadc_cb_type_t type;                        /*!< CB type. */
    sadc_raw_cb_t raw;                          /*!< raw data. */
    union {
        sadc_done_cb_t   done;                  /*!< Data for DONE cb. */
        sadc_sample_cb_t sample;                /*!< Data for SAMPLE cb. */
    } data;
} sadc_cb_t;

/**
 * \brief           User cb handler prototype.
 * \param[in]       p_cb: callback function
 */
typedef void (*sadc_proc_cb)(sadc_cb_t* p_cb);


/**
 * \brief           Register the sadc interrupt callback service routine function
 * \param[in]       sadc_int_callback: Config the interrupt callback service routine function.
 */
void sadc_register_int_callback(sadc_proc_cb sadc_int_callback);

/**
 * \brief           Enable the specified SADC interrupts
 * \param[in]       int_mask: Specifies the SADC interrupt sources to be enabled
 */
void sadc_int_enable(uint32_t int_mask);

/**
 * \brief           Disable SADC interrupts
 */
void sadc_int_disable(void);

/**
 * \brief           Config sadc xdma start address, segment size, block size
 * \param[in]       xdma_start_addr: Config xdma start address
 * \param[in]       xdma_seg_size: Config xdma segment size
 * \param[in]       xdma_blk_size: Config xdma block size
 */
void sadc_xdma_config(uint32_t xdma_start_addr,
                      uint16_t xdma_seg_size,
                      uint16_t xdma_blk_size);

/**
 * \brief           Config SADC resolution compensation
 * \param[in]       p_data: SADC Channel Value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 */
uint32_t sadc_resolution_compensation(sadc_value_t* p_data);

/**
 * \brief           Config the sadc channel
 * \param[in]       config_channel: Config the interrupt callback service routine function.
 */
void sadc_channel_enable(sadc_channel_config_t* config_channel);

/**
 * \brief           SADC Channel Disable
 * \param[in]       ch_sel: SADC Channel Select (0~9)
 */
void sadc_channel_disable(sadc_config_channel_t ch_sel);

/**
 * \brief           Initinal SADC
 * \param[in]       p_config: Config the sadc params
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 */
uint32_t sadc_init(sadc_config_t* p_config, sadc_proc_cb sadc_int_callback);

/**
 * \brief           SADC channel configuration, enable, and register the sadc interrupt callback service routine function
 * \param[in]       res: SADC resolution configuration
 * \param[in]       os: SADC oversample configuration
 * \param[in]       sadc_int_callback: SADC interrupt callback service routine function configuration
 */
void sadc_config_enable(sadc_config_resolution_t res,
                        sadc_config_oversample_t os, 
                        sadc_proc_cb sadc_int_callback);
/**
 * \brief           SADC vbat configuration, enable, and register the sadc interrupt callback service routine function
 * \param[in]       res: SADC resolution configuration
 * \param[in]       os: SADC oversample configuration
 * \param[in]       sadc_int_callback: SADC interrupt callback service routine function configuration
 */
void sadc_vbat_config_enable(sadc_config_resolution_t res,
                             sadc_config_oversample_t os,
                             sadc_proc_cb sadc_int_callback);

/**
 * \brief           SADC temperature configuration, enable, and register the sadc interrupt callback service routine function
 * \param[in]       res: SADC resolution configuration
 * \param[in]       os: SADC oversample configuration
 * \param[in]       sadc_int_callback: SADC interrupt callback service routine function configuration
 */
void sadc_temp_config_enable(sadc_config_resolution_t res,
                             sadc_config_oversample_t os,
                             sadc_proc_cb sadc_int_callback);

/**
 * \brief           SADC Disable
 */
void sadc_disable(void);

/**
 * \brief           SADC Enable
 */
void sadc_enable(void);

/**
 * \brief           SADC AIO Enable
 * \param[in]       aio_num: Aio number
 */
void sadc_aio_disable(uint8_t aio_num);

/**
 * \brief           SADC AIO Disnable
 * \param[in]       aio_num: Aio number
 */
void sadc_aio_enable(uint8_t aio_num);

/**
 * \brief           SADC start convert
 */
void sadc_start(void);

/**
 * \brief           Get SADC convert state
 * \param[in]       p_config: Config the sadc params
 * \return          SADC convert status, SADC_CONVERT_IDLE, SADC_CONVERT_START, SADC_CONVERT_DONE
 */
sadc_convert_state_t sadc_convert_state_get(void);

/**
 * \brief           Trigger to read SADC channel
 * \param[in]       ch: Config the sadc params
 * \return          SADC channel read status, STATUS_EBUSY, STATUS_SUCCESS
 */
uint32_t sadc_channel_read(sadc_input_ch_t ch);

/**
 * \brief           Trigger to read SADC vbat value
 * \return          SADC channel read status, STATUS_EBUSY, STATUS_SUCCESS
 */
uint32_t sadc_vbat_read(void);

/**
 * \brief           Trigger to read SADC temperature value
 * \return          SADC channel read status, STATUS_EBUSY, STATUS_SUCCESS
 */
uint32_t sadc_temp_read(void);

/*@}*/ /* end of RT584_DRIVER SADC */

#ifdef __cplusplus
}
#endif

#endif /* End of SADC_H */
