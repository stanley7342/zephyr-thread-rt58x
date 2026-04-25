/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_sadc.h
 * \brief           Hosal SADC driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_SADC_H
#define HOSAL_SADC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "sadc.h"



/**
 * \defgroup        HOSAL_SADC Hosal sadc
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal sadc definitions, structures, and functions
 * @{
 */

/**
 * \brief           Hosal sadc value
 */
typedef int32_t hosal_sadc_value_t;

/**
 * \brief           Hosal sadc convert status types
 */
#define HOSAL_SADC_CONVERT_IDLE  SADC_CONVERT_IDLE
#define HOSAL_SADC_CONVERT_START SADC_CONVERT_START
#define HOSAL_SADC_CONVERT_DONE  SADC_CONVERT_DONE

/**
 * \brief           Hosal sadc resolution bit setting
 */
#define HOSAL_SADC_RES_8BIT  SADC_RES_8BIT      /*!< 8 bit resolution. */
#define HOSAL_SADC_RES_10BIT SADC_RES_10BIT     /*!< 10 bit resolution. */
#define HOSAL_SADC_RES_12BIT SADC_RES_12BIT     /*!< 12 bit resolution. */
#define HOSAL_SADC_RES_14BIT SADC_RES_14BIT     /*!< 14 bit resolution. */

/**
 * \brief           Hosal sadc oversampleing rate setting
 */
#define HOSAL_SADC_OVERSAMPLE_0   SADC_OVERSAMPLE_0     /*!< No oversampling*/
#define HOSAL_SADC_OVERSAMPLE_2   SADC_OVERSAMPLE_2     /*!< Oversampling ratio multiple 2*/
#define HOSAL_SADC_OVERSAMPLE_4   SADC_OVERSAMPLE_4     /*!< Oversampling ratio multiple 4*/
#define HOSAL_SADC_OVERSAMPLE_8   SADC_OVERSAMPLE_8     /*!< Oversampling ratio multiple 8*/
#define HOSAL_SADC_OVERSAMPLE_16  SADC_OVERSAMPLE_16    /*!< Oversampling ratio multiple 16*/
#define HOSAL_SADC_OVERSAMPLE_32  SADC_OVERSAMPLE_32    /*!< Oversampling ratio multiple 32*/
#define HOSAL_SADC_OVERSAMPLE_64  SADC_OVERSAMPLE_64    /*!< Oversampling ratio multiple 64*/
#define HOSAL_SADC_OVERSAMPLE_128 SADC_OVERSAMPLE_128   /*!< Oversampling ratio multiple 128*/
#define HOSAL_SADC_OVERSAMPLE_256 SADC_OVERSAMPLE_256   /*!< Oversampling ratio multiple 256*/

/**
 * \brief           Hosal sadc channel input selection of the analog-to-digital converter.
 */         
#define HOSAL_SADC_CH_AIN0          SADC_CH_AIN0            /*!< Input AIO0. */
#define HOSAL_SADC_CH_AIN1          SADC_CH_AIN1            /*!< Input AIO1. */
#define HOSAL_SADC_CH_AIN2          SADC_CH_AIN2            /*!< Input AIO2. */
#define HOSAL_SADC_CH_AIN3          SADC_CH_AIN3            /*!< Input AIO3. */
#define HOSAL_SADC_CH_AIN4          SADC_CH_AIN4            /*!< Input AIO4. */
#define HOSAL_SADC_CH_AIN5          SADC_CH_AIN5            /*!< Input AIO5. */
#define HOSAL_SADC_CH_AIN6          SADC_CH_AIN6            /*!< Input AIO6. */
#define HOSAL_SADC_CH_AIN7          SADC_CH_AIN7            /*!< Input AIO7. */
#define HOSAL_SADC_CH_TEMPERATURE   SADC_CH_TEMPERATURE     /*!< Input TEMPERATURE. */
#define HOSAL_SADC_CH_VBAT          SADC_CH_VBAT            /*!< Input VBAT. */
#define HOSAL_SADC_CH_NC            SADC_CH_NC              /*!< Input No Connect. */

/**
 * \brief           Hosal sadc callback types
 */
#define HOSAL_SADC_CB_DONE   SADC_CB_DONE                   /*!< CB generated when the buffer is filled with samples. */
#define HOSAL_SADC_CB_SAMPLE SADC_CB_SAMPLE                 /*!< CB generated when the requested channel is sampled. */

/**
 * \brief           Hosal sadc analog-to-digital converter driver DONE cb
 */
typedef struct {
    hosal_sadc_value_t* p_buffer;               /*!< Pointer to buffer with converted samples. */
    uint16_t size;                              /*!< Number of samples in the buffer. */
} hosal_sadc_done_cb_t;

/**
 * \brief           Hosal sadc analog-to-digital converter driver raw SAMPLE cb
 */
typedef struct {
    hosal_sadc_value_t conversion_value;        /*!< Converted sample. */
    hosal_sadc_value_t compensation_value;      /*!< Compensation sample. */
    hosal_sadc_value_t calibration_value;       /*!< Calibration sample. */
} hosal_sadc_raw_cb_t;

/**
 * \brief           Hosal sadc analog-to-digital converter driver SAMPLE cb
 */
typedef struct {
    sadc_value_t value;                         /*!< Converted sample. */
    uint32_t channel;                           /*!< Converted channel. */
} hosal_sadc_sample_cb_t;

/**
 * \brief           Hosal sadc analog-to-digital converter driver cb
 */
typedef struct {
    uint32_t type;                              /*!< CB type. */
    hosal_sadc_raw_cb_t raw;

    union {
        hosal_sadc_done_cb_t done;              /*!< Data for DONE cb. */
        hosal_sadc_sample_cb_t sample;          /*!< Data for SAMPLE cb. */
    } data;
} hosal_sadc_cb_t;

/**
 * \brief           Sadc config structure.
 */
typedef struct {
    uint32_t resolution;                        /*!< sadc resolution bits. */
    uint32_t oversample;                        /*!< sadc oversample rate. */

} hosal_sadc_config_t;

/**
 * \brief           Sadc read channel config structure.
 */
typedef struct {
    uint32_t channel;                           /*!< sadc channel. */

} hosal_sadc_channel_config_t;

/**
 * \brief           Sadc interrupt service routine callback for user application.
 * \param[in]       p_cb: CB
 */
typedef void (*hosal_sadc_cb_fn)(hosal_sadc_cb_t* p_cb);

/**
 * \brief           Sadc AIO Enable
 * \param[in]       aio_num: Aio number
 */
void hosal_sadc_aio_disable(uint8_t aio_num);

/**
 * \brief           Sadc AIO Disnable
 * \param[in]       aio_num: Aio number
 */
void hosal_sadc_aio_enable(uint8_t aio_num);

/**
 * \brief hosal     Sadc channel configuration, enable, and register the sadc interrupt callback service routine function
 * \param[in]       SADC: configuration
 * \param[in]       sadc_int_callback: SADC interrupt callback service routine function configuration
 * \return None
 */
void hosal_sadc_config_enable(hosal_sadc_config_t sadc_cfg, void* sadc_int_callback);

/**
 * \brief           Trigger to read SADC channel
 * \param[in]       ch: SADC channel assignment
 *                      \arg SADC_CH_AIN0 Read SADC channel AIO0
 *                      \arg SADC_CH_AIN1 Read SADC channel AIO1
 *                      \arg SADC_CH_AIN2 Read SADC channel AIO2
 *                      \arg SADC_CH_AIN3 Read SADC channel AIO3
 *                      \arg SADC_CH_AIN4 Read SADC channel AIO4
 *                      \arg SADC_CH_AIN5 Read SADC channel AIO5
 *                      \arg SADC_CH_AIN6 Read SADC channel AIO6
 *                      \arg SADC_CH_AIN7 Read SADC channel AIO7
 *                      \arg SADC_CH_VBAT Read SADC channel VBAT
 * \return          return SADC channel read status
 *                      \arg STATUS_EBUSY Read SADC channel is busy and invalid
 *                      \arg STATUS_SUCCESS Read SADC channel is successful
 */
uint32_t hosal_sadc_channel_read(hosal_sadc_channel_config_t ch);

/*@}*/ /* end of RT584_HOSAL HOSAL_SADC */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_SADC_H */
