/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            sadc.h
 * \brief           SADC driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef SADC_H
#define SADC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \defgroup SADC Sadc
 * \ingroup RT58X_DRIVER
 * \brief  Define Sadc definitions, structures, and functions
 * @{
 */


/**
 * \brief           Sadc channel default setting value 
 */
#define SADC_PNSEL_CH_REG_DEFAULT 0x240000FF
#define SADC_SET_CH_REG_DEFAULT   0x80000000
#define SADC_THD_CH_REG_DEFAULT   0x3FFF0000

#define SADC_CH_REG_OFFSET 4                    /*unit: 4-bytes*/

/**
 * \brief           Sadc threshold default value interrupt enable 
 */
#define SADC_MONITOR_LOW_THD_DEFAULT  0
#define SADC_MONITOR_HIGH_THD_DEFAULT 0x3FFF

/**
 * \brief           Sadc interrupt interrupt related value
 */
#define SADC_INT_DISABLE_ALL 0xFFFFFFFF
#define SADC_INT_CLEAR_ALL   0xFFFFFFFF

/**
 * \brief           Sadc value
 */
typedef int32_t sadc_value_t;

/**
 * \brief           Sadc resolution bit setting
 */
typedef enum {
    SADC_RES_8BIT = 0,                          /*!< 8 bit resolution. */
    SADC_RES_10BIT = 1,                         /*!< 10 bit resolution. */
    SADC_RES_12BIT = 2,                         /*!< 12 bit resolution. */
    SADC_RES_14BIT = 3,                         /*!< 14 bit resolution. */
} sadc_config_resolution_t;

/**
 * \brief           Sadc oversampleing rate setting
 */
typedef enum {
    SADC_OVERSAMPLE_0 = 0,                      /*!< No oversampling*/
    SADC_OVERSAMPLE_2 = 1,                      /*!< Oversampling ratio multiple 2*/
    SADC_OVERSAMPLE_4 = 2,                      /*!< Oversampling ratio multiple 4*/
    SADC_OVERSAMPLE_8 = 3,                      /*!< Oversampling ratio multiple 8*/
    SADC_OVERSAMPLE_16 = 4,                     /*!< Oversampling ratio multiple 16*/
    SADC_OVERSAMPLE_32 = 5,                     /*!< Oversampling ratio multiple 32*/
    SADC_OVERSAMPLE_64 = 6,                     /*!< Oversampling ratio multiple 64*/
    SADC_OVERSAMPLE_128 = 7,                    /*!< Oversampling ratio multiple 128*/
    SADC_OVERSAMPLE_256 = 8,                    /*!< Oversampling ratio multiple 256*/
} sadc_config_oversample_t;

/**
 * \brief           Sadc sample mode setting
 */
typedef enum {
    SADC_SAMPLE_START = 0,                      /*!< SADC conversion is started by software SADC start in SADC sample one shot mode*/
    SADC_SAMPLE_TIMER = 1,                      /*!< SADC conversion is started by Timer in SADC sample timer mode*/
} sadc_config_sample_t;

/**
 * \brief           Sadc timer sample mode clock source setting
 */
typedef enum {
    SADC_TIMER_SYSTEM_CLK = 0,                  /*!< Select timer clock source to
                                                 system clock(48MHz/32MHz) in SADC sample timer mode*/
    SADC_TIMER_SLOW_CLK = 1,                    /*!< Select timer clock source to
                                                 slow clock (32KHz) in SADC sample timer mode*/
} sadc_config_timer_clk_t;

/**
 * \brief           Sadc channel number
 */
typedef enum {
    SADC_CHANNEL_0 = 0,                         /*!< Channel 0. */
    SADC_CHANNEL_1 = 1,                         /*!< Channel 1. */
    SADC_CHANNEL_2 = 2,                         /*!< Channel 2. */
    SADC_CHANNEL_3 = 3,                         /*!< Channel 3. */
    SADC_CHANNEL_4 = 4,                         /*!< Channel 4. */
    SADC_CHANNEL_5 = 5,                         /*!< Channel 5. */
    SADC_CHANNEL_6 = 6,                         /*!< Channel 6. */
    SADC_CHANNEL_7 = 7,                         /*!< Channel 7. */
    SADC_CHANNEL_8 = 8,                         /*!< Channel 8. */
    SADC_CHANNEL_9 = 9,                         /*!< Channel 9. */
    SADC_CHANNEL_MAX = 10,                      /*!< Max Channel 10. */
} sadc_config_channel_t;

/**
 * \brief           Sadc input selection of the analog-to-digital converter.
 */
typedef enum {
    SADC_AIN_0 = 0,                             /*!< Input 0. */
    SADC_AIN_1 = 1,                             /*!< Input 1. */
    SADC_AIN_2 = 2,                             /*!< Input 2. */
    SADC_AIN_3 = 3,                             /*!< Input 3. */
    SADC_AIN_4 = 4,                             /*!< Input 4. */
    SADC_AIN_5 = 5,                             /*!< Input 5. */
    SADC_AIN_6 = 6,                             /*!< Input 6. */
    SADC_AIN_7 = 7,                             /*!< Input 7. */
    SADC_AIN_8 = 8,                             /*!< Input 8. Temperature Sensor */
    SADC_TEMPERATURE = 8,                       /*!< Input 8. Temperature Sensor */
    SADC_AIN_9 = 9,                             /*!< Input 9. Bypass VGA */
    SADC_BYPASS_VGA = 9,                        /*!< Input 9. Bypass VGA */
    SADC_AIN_10 = 10,                           /*!< Input 10. VBAT */
    SADC_VBAT = 10,                             /*!< Input 10. VBAT */
    SADC_VBAT_0VADC = 10,                       /*!< Input 10. VBAT */
    SADC_AIN_DISABLED = 11,                     /*!< No input selected. */
} sadc_config_input_t;

/**
 * \brief           Sadc channel input selection of the analog-to-digital converter.
 */
typedef enum {
    SADC_CH_AIN0 = 0,                           /*!< Input AIO0. */
    SADC_CH_AIN1 = 1,                           /*!< Input AIO1. */
    SADC_CH_AIN2 = 2,                           /*!< Input AIO2. */
    SADC_CH_AIN3 = 3,                           /*!< Input AIO3. */
    SADC_CH_AIN4 = 4,                           /*!< Input AIO4. */
    SADC_CH_AIN5 = 5,                           /*!< Input AIO5. */
    SADC_CH_AIN6 = 6,                           /*!< Input AIO6. */
    SADC_CH_AIN7 = 7,                           /*!< Input AIO7. */
    SADC_CH_VBAT = 8,                           /*!< Input VBAT. */
    SADC_CH_NC = 13,                            /*!< Input No Connect. */
    SADC_COMP_VBAT = 9,                         /*!< Input VBAT. */
    SADC_COMP_TEMPERATURE = 10,                 /*!< Input TEMPERATURE. */
    SADC_COMP_VCM = 11,                         /*!< Input VCM. */
    SADC_0VADC = 12,                            /*!< Input VBAT 0V. */
} sadc_input_ch_t;

/**
 * \brief           Sadc channel tacq setting
 */
typedef enum {
    SADC_TACQ_EDLY_TIME_0P3US = 0,              /*!< Edly time 0.3us. */
    SADC_TACQ_EDLY_TIME_1US = 1,                /*!< Edly time 1us. */
    SADC_TACQ_EDLY_TIME_2US = 2,                /*!< Edly time 2us. */
    SADC_TACQ_EDLY_TIME_3US = 3,                /*!< Edly time 3us. */
    SADC_TACQ_EDLY_TIME_4US = 4,                /*!< Edly time 4us. */
    SADC_TACQ_EDLY_TIME_8US = 5,                /*!< Edly time 8us. */
    SADC_TACQ_EDLY_TIME_12US = 6,               /*!< Edly time 12us. */
    SADC_TACQ_EDLY_TIME_16US = 7,               /*!< Edly time 163us. */
} sadc_config_tacq_edly_t;

/**
 * \brief           Sadc burst enablg types
 */
typedef enum {
    SADC_BURST_DISABLE = 0,                     /*!< Sadc burst enable. */
    SADC_BURST_ENABLE = 1,                      /*!< Sadc burst disable. */
} sadc_config_burst_t;

/**
 * \brief           Sadc callback types
 */
typedef enum {
    SADC_CB_DONE,                               /*!< CB generated when the buffer 
                                                 is filled with samples. */
    SADC_CB_SAMPLE,                             /*!< CB generated when the requested
                                                 channel is sampled. */
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
    SADC_CALIBRATION_VBAT = 0,                  /*!< Sadc vbat calibration. */
    SADC_CALIBRATION_AIO,                       /*!< Sadc aio calibration. */
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
 * \brief           Sadc configuration struct
 */
typedef struct {
    sadc_int_t sadc_int_mask;
    sadc_config_resolution_t sadc_resolution;   /*!< SADC resolution*/
    sadc_config_oversample_t sadc_oversample;   /*!< SADC oversample*/
    sadc_config_xdma_t sadc_xdma;               /*!< SADC XDMA*/
    sadc_config_sample_t sadc_sample_mode;      /*!< SADC sample mode*/
    sadc_config_timer_t sadc_timer;             /*!< SADC timer*/
} sadc_config_t;

/**
 * \brief           Sadc channel configuration struct
 */
typedef struct {
    sadc_config_channel_t ch_sel;               /*!< SADC AIN0~AIN9 channel Selection*/
    sadc_config_input_t pi_sel;                 /*!< SADC AIN0~AIN9 P channel Selection*/
    sadc_config_input_t ni_sel;                 /*!< SADC AIN0~AIN9 N channel Selection*/
    uint32_t gain;                              /*!< SADC AIN0~AIN9 Gain settings*/
    uint32_t pull;                              /*!< SADC AIN0~AIN9 Pull high or low*/
    sadc_config_tacq_edly_t tacq;               /*!< SADC result acquisition time
                                                 for system clock 32M*/
    sadc_config_tacq_edly_t edly;               /*!< SADC end delay time for system
                                                 clock 32M */
    sadc_config_burst_t burst;                  /*!< SADC Burst mode selection*/
    uint32_t low_thd;                           /*!< SADC Low threshold*/
    uint32_t high_thd;                          /*!< SADC High threshold*/
} sadc_channel_config_t;

/**
 * \brief           Sadc analog-to-digital converter driver DONE cb
 */
typedef struct {
    sadc_value_t* p_buffer;                     /*!< Pointer to buffer with converted
                                                 samples. */
    uint16_t size;                              /*!< Number of samples in the buffer. */
} sadc_done_cb_t;

/**
 * \brief           Sadc analog-to-digital converter driver raw SAMPLE cb
 */
typedef struct {
    sadc_value_t conversion_value;              /*!< Converted sample. */
    sadc_value_t compensation_value;            /*!< Compensation sample. */
    sadc_value_t calibration_value;             /*!< Calibration sample. */
} sadc_raw_cb_t;

/**
 * \brief           Sadc analog-to-digital converter driver SAMPLE cb
 */
typedef struct {
    sadc_value_t value;                         /*!< Converted sample. */
    uint32_t channel;                           /*!< Converted channel. */
} sadc_sample_cb_t;

/**
 * \brief           Sadc analog-to-digital converter driver cb
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
 * \brief           Sadc interrupt service routine callback for user application.
 * \param[in]       p_cb: CB
 */
typedef void (*sadc_cb_fn)(sadc_cb_t* p_cb);

/**
 * \brief           Reset sadc module
 */
__STATIC_INLINE void sadc_reset(void) {
    SADC->control1.bit.cfg_sadc_rst = ENABLE;
    return;
};

/**
 * \brief           Get the last SADC conversion result data for regular channel
 * \return          SADC conversion result data
 */
__STATIC_INLINE uint32_t sadc_get_adc_value(void) {
    return SADC->r0.bit.sadc_o;
};

/**
 * \brief           Configure the SADC resolution
 * \param[in]       resolution: SADC resolution setting
 */
__STATIC_INLINE void sadc_set_resolution(uint8_t resolution) {
    SADC->setting1.bit.cfg_sadc_bit = resolution;
    return;
};

/**
 * \brief           Get the SADC resolution
 * \return          SADC resolution
 */
__STATIC_INLINE uint32_t sadc_get_resolution(void) {
    return SADC->setting1.bit.cfg_sadc_bit;
};

/**
 * \brief           Configure the SADC oversample rate value
 * \param[in]       oversample_rate: SADC oversample rate setting
 */
__STATIC_INLINE void sadc_set_oversample_rate(uint8_t oversample_rate) {
    SADC->setting1.bit.cfg_sadc_osr = oversample_rate;
    return;
};

/**
 * \brief           Configure the SADC sample mode (one shot mode or timer mode)
 * \param[in]       sample_mode: SADC oversample rate setting
 */
__STATIC_INLINE void sadc_set_sample_mode(uint8_t sample_mode) {
    SADC->setting0.bit.cfg_sadc_smp_mode = sample_mode;
    return;
};

/**
 * \brief           Get the SADC sample mode
 * \return          SADC sample mode setting
 */
__STATIC_INLINE uint8_t sadc_get_sample_mode(void) {
    return SADC->setting0.bit.cfg_sadc_smp_mode;
};

/**
 * \brief           Select timer clock source in SADC sample timer mode
 * \param[in]       timer_source: SADC timer sample mode clock source
 */
__STATIC_INLINE void sadc_set_timer_sample_source(uint8_t timer_source) {
    SADC->setting0.bit.cfg_sadc_tmr_cksel = timer_source;
    return;
};

/**
 * \brief           Configure timer clock divisor (1~65535) in SADC sample timer mode
 * \param[in]       timer_divide: SADC timer divide
 */
__STATIC_INLINE void sadc_set_timer_divide(uint16_t timer_divide) {
    SADC->setting0.bit.cfg_sadc_tmr_ckdiv = timer_divide;
    return;
};

/**
 * \brief           Get the last SADC conversion channel
 * \return          SADC conversion channel
 */
__STATIC_INLINE uint32_t sadc_get_adc_channel(void) {
    return SADC->r0.bit.sadc_o_chx;
};

/**
 * \brief           Start sadc xdma start
 */
__STATIC_INLINE void sadc_xdma_start(void) {
    SADC->wdma_ctl0.bit.cfg_sadc_wdma_ctl0 = ENABLE;
    return;
};

/**
 * \brief           Get the XDMA buffer address
 * \return          xdma buffer address
 */
__STATIC_INLINE uint32_t sadc_get_xdma_start_address(void) {
    return SADC->wdma_set1;
};

/**
 * \brief           Get the number of SADC result write into WDMA since last SADC start
 * \return          number of SADC result
 */
__STATIC_INLINE uint32_t sadc_get_xdma_result_number(void) {
    return SADC->r1.bit.sadc_num_res;
};

/**
 * \brief           Configure the SADC to manual mode
 */
__STATIC_INLINE void sadc_manual_mode(void) {
    SADC->setting1.bit.cfg_sadc_tst = 0x08;
    return;
};

/**
 * \brief           Enable the SADC VGA
 */
__STATIC_INLINE void sadc_vga_enable(void) {
    SADC->control0.bit.cfg_sadc_vga_ena = ENABLE;
    return;
};

/**
 * \brief           Disable the SADC VGA
 */
__STATIC_INLINE void sadc_vga_disable(void) {
    SADC->control0.bit.cfg_sadc_vga_ena = DISABLE;
    return;
};

/**
 * \brief           Enable the SADC LDO
 */
__STATIC_INLINE void sadc_ldo_enable(void) {
    SADC->control0.bit.cfg_sadc_ldo_ena = ENABLE;
    return;
};

/**
 * \brief           Disable the SADC LDO
 */
__STATIC_INLINE void sadc_ldo_disable(void) {
    SADC->control0.bit.cfg_sadc_ldo_ena = DISABLE;
    return;
};

/**
 * \brief           Register sadc callback function
 * \param[in]       sadc_int_callback
 *                  sadc_comp_int_callback
 *                  sadc_txcomp_int_callback: user callback function 
 */
void sadc_register_int_callback(sadc_cb_fn sadc_int_callback);
void sadc_register_adcomp_int_callback(sadc_cb_fn sadc_comp_int_callback);
void sadc_register_txcomp_int_callback(sadc_cb_fn sadc_txcomp_int_callback);

/**
 * \brief           Config sadc xdma start address, segment size, block size
 * \param[in]       xdma_start_addr: xdma start address
 * \param[in]       xdma_seg_size: xdma segment size
 * \param[in]       xdma_blk_size: xdma block size
 */
void sadc_xdma_config(uint32_t xdma_start_addr, uint16_t xdma_seg_size,
                      uint16_t xdma_blk_size);

/**
 * \brief           Initinal SADC
 * \param[in]       p_config: config the sadc params
 * \param[in]       sadc_int_callback: config the interrupt service routine callback function
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t sadc_init(sadc_config_t* p_config, sadc_cb_fn sadc_int_callback);

/**
 * \brief           Enable sadc interrupt
 * \param[in]       int_mask: 1 mask corresponding bit interrupt, 0 unmask corresponding bit interrupt
 */
void sadc_int_enable(uint32_t int_mask);

/**
 * \brief           Disable the SADC interrupt
 */
void sadc_int_disable(void);

/**
 * \brief           Config the sadc channel
 * \param[in]       config_channel: channel configuration
 */
void sadc_channel_enable(sadc_channel_config_t* config_channel);

/**
 * \brief           Disable the sadc channel
 * \param[in]       ch_sel: sadc channel select
 */
void sadc_channel_disable(sadc_config_channel_t ch_sel);

/**
 * \brief           Disable SADC
 */
void sadc_disable(void);

/**
 * \brief           Enable SADC
 */
void sadc_enable(void);

/**
 * \brief           SADC start convert
 */
void sadc_start(void);

/**
 * \brief           SADC channel configuration, enable, and register the sadc
 *                   interrupt callback service routine function
 * \param[in]       res: SADC resolution configuration
 * \param[in]       os: SADC oversample configuration
 * \param[in]       sadc_int_callback: SADC interrupt callback service routine function configuration
 */
void sadc_config_enable(sadc_config_resolution_t res,
                        sadc_config_oversample_t os,
                        sadc_cb_fn sadc_int_callback);

/**
 * \brief           Trigger to read SADC channel
 * \param[in]       ch: SADC channel selecte
 * \return          Function status, STATUS_SUCCESS, STATUS_EBUSY
 */
uint32_t sadc_channel_read(sadc_input_ch_t ch);

/**
 * \brief           SADC resolution compensation
 * \param[in]       p_data: SADC channel Value
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t sadc_resolution_compensation(sadc_value_t* p_data);

/**
 * \brief           Get SADC convert state
 * \return          SADC convert state, SADC_CONVERT_IDLE, SADC_CONVERT_START, SADC_CONVERT_DONE
 */
sadc_convert_state_t sadc_convert_state_get(void);

/**
 * \brief           SADC Calibration Initialization to read SADC related MP Sector
 */
void sadc_calibration_init(void);

/**
 * \brief           SADC value calibration
 * \param[in]       cal_type: SADC calibration type
 * \param[in]       adc_val: SADC value before calibration
 * \return          SADC value after calibration
 */
uint32_t sadc_calibration(sadc_cal_type_t cal_type, sadc_value_t adc_val);

/**
 * \brief           Enable SADC Temperature
 */
void sadc_temperature_enable(void);

/**
 * \brief           Disable SADC Temperature
 */
void sadc_temperature_disable(void);

/**
 * \brief           SADC temperature value calibration
 * \param[in]       adc_val: SADC value before calibration
 * \return          SADC temperature value after calibration
 */
uint32_t sadc_temperature_calibration(sadc_value_t adc_val);

/**
 * \brief           Update SADC temperature calibration offset value
 * \param[in]       adc_val: SADC value for updating calibration offset
 */
void sadc_temperature_calibration_offset_update(sadc_value_t adc_val);

/**
 * \brief           SADC Compensation Initialization to enable SW timer for periodic compensation
 * \param[in]       xPeriodicTimeInSec: Periodic time in second for SADC compensation
 */
void sadc_compensation_init(uint32_t xPeriodicTimeInSec);

/**
 * \brief           SADC Compensation De-initialization to disable SW timer for periodic compensation
 */
void sadc_compensation_deinit(void);

/**
 * \brief           Update SADC compensation offset value
 * \param[in]       adc_val: SADC value for updating compensation offset
 */
void sadc_compensation_offset_update(sadc_value_t adc_val);

/**
 * \brief           SADC value compensation
 * \param[in]       adc_val: SADC value before compensation
 * \return          SADC value after compensation
 */
sadc_value_t sadc_compensation(sadc_value_t adc_val);

/*@}*/ /* end of RT58X_DRIVER SADC */

#ifdef __cplusplus
}
#endif

#endif /* End of SADC_H */
