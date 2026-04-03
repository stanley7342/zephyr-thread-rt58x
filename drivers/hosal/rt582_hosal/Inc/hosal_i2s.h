/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_i2s.h
 * \brief           Hosal I2S driver header file
 */
/*
 * Author:          Kc.tseng
 */

#ifndef HOSAL_I2S_H
#define HOSAL_I2S_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \defgroup HOSAL_I2S Hosal i2s
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal i2s definitions, structures, and functions
 * @{
 */


/**
 * \brief           Hosal I2S Simple Rate mapping table
 */
#define HOSAL_I2S_SR_48K I2S_SR_48K             /*!< I2S_WCK 48K */
#define HOSAL_I2S_SR_32K I2S_SR_32K             /*!< I2S_WCK 32K */
#define HOSAL_I2S_SR_16K I2S_SR_16K             /*!< I2S_WCK 16K */
#define HOSAL_I2S_SR_8K I2S_SR_8K               /*!< I2S_WCK 8K */
#define I2S_SR_MAX                              /*!< I2S_WCK MAX */

/**
 * \brief           Hosal I2S internal mclk setting
 */
#define HOSAL_I2S_IMCLK_12P288M I2S_IMCLK_12P288M   /*!< PLL_CK = 32M, I2S_WCK = 48K */
#define HOSAL_I2S_IMCLK_8P192M I2S_IMCLK_8P192M     /*!< PLL_CK = 32M, I2S_WCK = 8/16/32K */
#define HOSAL_I2S_IMCLK_24P576M I2S_IMCLK_24P576M   /*!< PLL_CK = 64M, I2S_WCK = 48K */
#define HOSAL_I2S_IMCLK_16P384M I2S_IMCLK_16P384M   /*!< PLL_CK = 64M, I2S_WCK = 64K */
#define HOSAL_I2S_IMCLK_MAX I2S_IMCLK_MAX           /*!< imclk selection max */

/**
 * \brief           Hosal I2S channel mode, Stereo / Left channel only / Right channel only
 */
#define HOSAL_I2S_CH_STEREO I2S_CH_STEREO           /*!< I2S stereo */
#define HOSAL_I2S_CH_MONO_L I2S_CH_MONO_L           /*!< I2S left only */
#define HOSAL_I2S_CH_MONO_R I2S_CH_MONO_R           /*!< I2S right only */
#define HOSAL_I2S_CH_MAX I2S_CH_MAX                 /*!< I2S channel_max */

/**
 * \brief            Hosal I2S format table
 */
#define HOSAL_I2S_FMT_LJ I2S_FMT_LJ                 /*!< I2S left justified format */
#define HOSAL_I2S_FMT_RJ I2S_FMT_RJ                 /*!< I2S right justified format */
#define HOSAL_I2S_FMT_I2S I2S_FMT_I2S               /*!< I2S standard format */
#define HOSAL_I2S_FMT_MAX I2S_FMT_MAX               /*!< I2S format_max */

/**
 * \brief           Hosal I2S MCLK selection index table
 */
#define HOSAL_I2S_MCLK_ISEL_0 I2S_MCLK_ISEL_0       /*!< mclk selection index 0 */
#define HOSAL_I2S_MCLK_ISEL_1 I2S_MCLK_ISEL_1       /*!< mclk selection index 1 */
#define HOSAL_I2S_MCLK_ISEL_2 I2S_MCLK_ISEL_2       /*!< mclk selection index 2 */
#define HOSAL_I2S_MCLK_ISEL_3 I2S_MCLK_ISEL_3       /*!< mclk selection index 3 */
#define HOSAL_I2S_MCLK_ISEL_4 I2S_MCLK_ISEL_4       /*!< mclk selection index 4 */
#define HOSAL_I2S_MCLK_ISEL_5 I2S_MCLK_ISEL_5       /*!< mclk selection index 5 */
#define HOSAL_I2S_MCLK_ISEL_MAX I2S_MCLK_ISEL_MAX   /*!< mclk selection index max */

/**
 * \brief           Hosal I2S MCLK division table
 */
#define HOSAL_I2S_MCLK_DIV_1 I2S_MCLK_DIV_1         /*!< mclk division 1 */
#define HOSAL_I2S_MCLK_DIV_2 I2S_MCLK_DIV_2         /*!< mclk division 2 */
#define HOSAL_I2S_MCLK_DIV_4 I2S_MCLK_DIV_4         /*!< mclk division 4 */
#define HOSAL_I2S_MCLK_DIV_8 I2S_MCLK_DIV_8         /*!< mclk division 8 */
#define HOSAL_I2S_MCLK_DIV_16 I2S_MCLK_DIV_16       /*!< mclk division 16 */
#define HOSAL_I2S_MCLK_DIV_32 I2S_MCLK_DIV_32       /*!< mclk division 32 */
#define HOSAL_I2S_MCLK_DIV_MAX I2S_MCLK_DIV_MAX     /*!< mclk division max */

/**
 * \brief           Hosal I2S BCLK OSR setting table
 */
#define HOSAL_I2S_CFG_BCK_OSR_2 I2S_CFG_BCK_OSR_2       /*!< bclk osr 2 */
#define HOSAL_I2S_CFG_BCK_OSR_4 I2S_CFG_BCK_OSR_4       /*!< bclk osr 4 */
#define HOSAL_I2S_CFG_BCK_OSR_8 I2S_CFG_BCK_OSR_8       /*!< bclk osr 8 */
#define HOSAL_I2S_CFG_BCK_OSR_MAX I2S_CFG_BCK_OSR_MAX   /*!< bclk osr max */

/**
 * \brief           I2S TRX mode setting table
 */
#define HOSAL_I2S_TRX_MODE_TXRX I2S_TRX_MODE_TXRX   /*!< TRX */
#define HOSAL_I2S_TRX_MODE_TX I2S_TRX_MODE_TX       /*!< TX */
#define HOSAL_I2S_TRX_MODE_RX I2S_TRX_MODE_RX       /*!< RX */
#define HOSAL_I2S_TRX_MODE_MAX I2S_TRX_MODE_MAX     /*!< TRX max */

/**
 * \brief           Hosal I2S date depth
 */
#define HOSAL_I2S_CFG_WID_16 I2S_CFG_WID_16     /*!< data width 16 */
#define HOSAL_I2S_CFG_WID_24 I2S_CFG_WID_24     /*!< data width 24 */
#define HOSAL_I2S_CFG_WID_32 I2S_CFG_WID_32     /*!< data width 32 */
#define HOSAL_I2S_CFG_WID_MAX I2S_CFG_WID_MAX   /*!< data width max */

/**
 * \brief           Hosal I2S BLCK / WRCK ratio
 */
#define HOSAL_I2S_BCK_RATIO_32 I2S_BCK_RATIO_32,    /*!< blck / wrck ratio 32 */
#define HOSAL_I2S_BCK_RATIO_48 I2S_BCK_RATIO_48     /*!< blck / wrck ratio 48 */
#define HOSAL_I2S_BCK_RATIO_64 I2S_BCK_RATIO_64     /*!< blck / wrck ratio 64 */
#define HOSAL_I2S_BCK_RATIO_MAX I2S_BCK_RATIO_MAX   /*!< blck / wrck ratio max */

/**
 * \brief           Hosal I2S callback data type
 */

#define HOSAL_I2S_CB_WDMA I2S_CB_WDMA           /*!< CB generated when the buffer is filled with samples. */
#define HOSAL_I2S_CB_RDMA I2S_CB_RDMA           /*!< CB generated when the requested channel is sampled. */


/**
 * \brief           Hosal I2S callback structure
 */
typedef struct {
    uint32_t type;                   /*!< i2s type */
    uint16_t blk_size;                          /*!< xdma blockk size */
    uint16_t seg_size;                          /*!< xdma segment size */
} hosal_i2s_cb_t;

/**
 * \brief           Hosal I2S xDMA configurarions structure
 */
typedef struct {
    uint32_t i2s_xdma_start_addr;               /*!< xDMA  start address */
    uint32_t i2s_fw_access_addr;                /*!< Firmware access address */
    uint16_t i2s_xdma_seg_size;                 /*!< xDMA Segment size */
    uint16_t i2s_xdma_blk_size;                 /*!< xDMA Block size */
    uint8_t  i2s_xdma_seg_blk_ratio;            /*!< xDMA Segment and Block ratio */
} hosal_i2s_xdma_ctrl_ptr_t, hosal_i2s_rdma_ctrl_ptr_t, hosal_i2s_wdma_ctrl_ptr_t;

/**
 * \brief           I2S configurarions structure
 */
typedef struct {
    hosal_i2s_rdma_ctrl_ptr_t*  rdma_config;    /*!< rdma config */
    hosal_i2s_wdma_ctrl_ptr_t*  wdma_config;    /*!< wdma config */
    uint32_t                    sr;             /*!< sample rate */
    uint32_t                    ch;             /*!< channel format */
    uint32_t                    trx_mode;       /*!< TRX mode */
    uint32_t                    fmt;            /*!< data format */
    uint32_t                    width;          /*!< data width */
    uint32_t                    bck_ratio;      /*!< bck / wrck ratio */
    uint32_t                    mck_div;        /*!< mclk division */
    uint32_t                    bck_osr;        /*!< bclk osr setting */
    uint32_t                    imck_rate;      /*!< internal mclk setting */
} hosal_i2s_para_set_t;

/**
 * \brief           Hosal I2S buffer pointer infomation
 */
typedef struct {
    uint8_t     flag;                           /*!< data complete flag */
    uint16_t    size;                           /*!< new data */
    uint32_t    index;                          /*!< total data */
    uint16_t    offset;                         /*!< index - size */
    uint32_t    address;                        /*!< data address */
} hosal_i2s_buff_ptr_t;


/**
 * \brief           i2S interrupt service routine callback for user application.
 * \param[in]       p_cb: the reason of rtc alarm routine trigger
 */
typedef void (*hosal_i2s_cb_fn)(hosal_i2s_cb_t* p_cb);

/**
 * \brief           Get i2s rdma access address
 * \param[out]      rdma_address: the pointer to get I2S rdma access address
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_i2s_get_rdma_access_pos(uint32_t* rdma_address);

/**
 * \brief           Get i2s wdma access address
 * \param[out]      wdma_address: the pointer to get I2S wdma access address
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_i2s_get_wdma_access_pos(uint32_t* wdma_address);

/**
 * \brief           Register i2s callback function
 * \param[in]       i2s_usr_callback: user callback function
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_i2s_callback_register(void* i2s_usr_callback);

/**
 * \brief           The initial config function according to the i2s_para to set i2s registers
 * \param[in]       i2s_para: i2s parameter
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_i2s_init(hosal_i2s_para_set_t* i2s_para);

/**
 * \brief           Uninitial i2s
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_i2s_uninit(void);

/**
 * \brief           Open i2s loopback
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_i2s_loopback_open(void);


/**
 * \brief           Start i2s, and enable xdma
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_i2s_start(hosal_i2s_para_set_t* i2s_para);

/**
 * \brief           Stop i2s
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t hosal_i2s_stop(void);

/*@}*/ /* end of RT58X_HOSAL HOSAL_I2S */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_I2S_H */
