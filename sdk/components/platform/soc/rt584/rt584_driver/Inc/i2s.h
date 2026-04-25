/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           i2s.h
 * \brief          I2S driver header file
 */
/*
 * Author:         Kc.tseng
 */


#ifndef I2S_H
#define I2S_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"


/**
 * \defgroup        I2S I2S
 * \ingroup         RT584_DRIVER
 * \brief           Define I2S definitions, structures, and functions
 * @{
 */

/**
 * \brief           I2S XDMA related
 */
#define I2S_RDMA_ENABLE              (0x01UL << I2S_RDMA_ENABLE_SHFT)
#define I2S_RDMA_RESET               (0x01UL << I2S_RDMA_RESET_SHFT)
#define I2S_WDMA_ENABLE              (0x01UL << I2S_WDMA_ENABLE_SHFT)
#define I2S_WDMA_RESET               (0x01UL << I2S_WDMA_RESET_SHFT)
#define I2S_RDMA_IRQ_CLR             (0x01UL << I2S_RDMA_IRQ_CLR_SHFT)
#define I2S_RDMA_ERR_IRQ_CLR         (0x01UL << I2S_RDMA_ERR_IRQ_CLR_SHFT)
#define I2S_WDMA_IRQ_CLR             (0x01UL << I2S_WDMA_IRQ_CLR_SHFT)
#define I2S_WDMA_ERR_IRQ_CLR         (0x01UL << I2S_WDMA_ERR_IRQ_CLR_SHFT)
#define I2S_RDMA_IRQ_MASK_ENABLE     (0x01UL << I2S_RDMA_IRQ_MASK_SHFT)
#define I2S_RDMA_ERR_IRQ_MASK_ENABLE (0x01UL << I2S_RDMA_ERR_IRQ_MASK_SHFT)
#define I2S_WDMA_IRQ_MASK_ENABLE     (0x01UL << I2S_WDMA_IRQ_MASK_SHFT)
#define I2S_WDMA_ERR_IRQ_MASK_ENABLE (0x01UL << I2S_WDMA_ERR_IRQ_MASK_SHFT)

/**
 *  \brief          I2S-XDMA type, RDMA or WDMA.
 */
typedef enum {
    I2S_XDMA_RDMA = 0,                          /*!< I2S rdma */
    I2S_XDMA_WDMA,                              /*!< I2S wdma */
    I2S_XDMA_MAX,                               /*!< I2S xdma_max */
} i2s_xdma_t;

/**
 * \brief           I2S Simple Rate mapping table
 */
typedef enum {
    I2S_SR_48K,                                 /*!< I2S_WCK 48K */
    I2S_SR_32K,                                 /*!< I2S_WCK 32K */
    I2S_SR_16K,                                 /*!< I2S_WCK 16K */
    I2S_SR_8K,                                  /*!< I2S_WCK 8K */
    I2S_SR_MAX,                                 /*!< I2S_WCK MAX */
} i2s_sample_rate_t;

/**
 * \brief           I2S internal mclk setting
 */
typedef enum {
    I2S_IMCLK_12P288M,                          /*!< PLL_CK = 32M, I2S_WCK = 48K */
    I2S_IMCLK_8P192M,                           /*!< PLL_CK = 32M, I2S_WCK = 8/16/32K */
    I2S_IMCLK_24P576M,                          /*!< PLL_CK = 64M, I2S_WCK = 48K */
    I2S_IMCLK_16P384M,                          /*!< PLL_CK = 64M, I2S_WCK = 64K */
    I2S_IMCLK_MAX,                              /*!< imclk selection max */
} i2s_cfg_imck_rate_t;

/**
 *  \brief          I2S channel mode, Stereo / Left channel only / Right channel only
 */
typedef enum {
    I2S_CH_STEREO,                              /*!< I2S stereo */
    I2S_CH_MONO_L,                              /*!< I2S left only */
    I2S_CH_MONO_R,                              /*!< I2S right only */
    I2S_CH_MAX,                                 /*!< I2S channel_max */
} i2s_ch_format_t;

/**
 *  \brief          I2S format table, standard I2S mode / Left Justified mode / Right Justified mode
 */
typedef enum {
    I2S_FMT_LJ = 0,                             /*!< I2S left justified format */
    I2S_FMT_RJ,                                 /*!< I2S right justified format */
    I2S_FMT_I2S,                                /*!< I2S standard format */
    I2S_FMT_MAX,                                /*!< I2S format_max */
} i2s_fmt_t;

/**
 *  \brief          MCLK selection index table
 */
typedef enum {
    I2S_MCLK_ISEL_0 = 0,                        /*!< mclk selection index 0 */
    I2S_MCLK_ISEL_1,                            /*!< mclk selection index 1 */
    I2S_MCLK_ISEL_2,                            /*!< mclk selection index 2 */
    I2S_MCLK_ISEL_3,                            /*!< mclk selection index 3 */
    I2S_MCLK_ISEL_4,                            /*!< mclk selection index 4 */
    I2S_MCLK_ISEL_5,                            /*!< mclk selection index 5 */
    I2S_MCLK_ISEL_MAX,                          /*!< mclk selection index max */
} i2s_mclk_isel_t;

/**
 * \brief           I2S MCLK division table
 */
typedef enum {
    I2S_MCLK_DIV_1 = 0,                         /*!< mclk division 1 */
    I2S_MCLK_DIV_2,                             /*!< mclk division 2 */
    I2S_MCLK_DIV_4,                             /*!< mclk division 4 */
    I2S_MCLK_DIV_8,                             /*!< mclk division 8 */
    I2S_MCLK_DIV_16,                            /*!< mclk division 16 */
    I2S_MCLK_DIV_32,                            /*!< mclk division 32 */
    I2S_MCLK_DIV_MAX,                           /*!< mclk division max */
} i2s_mclk_div_t;

/**
 * \brief           I2S BCLK OSR setting table
 */
typedef enum {
    I2S_CFG_BCK_OSR_2 = 0,                      /*!< bclk osr 2 */
    I2S_CFG_BCK_OSR_4,                          /*!< bclk osr 4 */
    I2S_CFG_BCK_OSR_8,                          /*!< bclk osr 8 */
    I2S_CFG_BCK_OSR_MAX,                        /*!< bclk osr max */
} i2s_cfg_bck_osr_t;
/**
 * \brief           I2S TRX mode setting table
 */
typedef enum {
    I2S_TRX_MODE_TXRX = 0,                      /*!< TRX */
    I2S_TRX_MODE_TX,                            /*!< TX */
    I2S_TRX_MODE_RX,                            /*!< RX */
    I2S_TRX_MODE_MAX,                           /*!< TRX max */
} i2s_cfg_i2s_mode_t, i2s_trx_mode_t;

/**
 * \brief           I2S date depth
 */
typedef enum {
    I2S_CFG_WID_16 = 0,                         /*!< data width 16 */
    I2S_CFG_WID_24,                             /*!< data width 24 */
    I2S_CFG_WID_32,                             /*!< data width 32 */
    I2S_CFG_WID_MAX,                            /*!< data width max */
} i2s_cfg_txd_wid_t, i2s_cfg_rxd_wid_t, i2s_bck_width_t;

/**
 * \brief           I2S BLCK / WRCK ratio
 */
typedef enum {
    I2S_BCK_RATIO_32 = 0,                       /*!< blck / wrck ratio 32 */
    I2S_BCK_RATIO_48,                           /*!< blck / wrck ratio 48 */
    I2S_BCK_RATIO_64,                           /*!< blck / wrck ratio 64 */
    I2S_BCK_RATIO_MAX,                          /*!< blck / wrck ratio max */
} i2s_cfg_bck_ratio_t;

/**
 * \brief           I2S callback data type
 */
typedef enum {
    I2S_CB_WDMA,                                /*!< CB generated when the buffer is filled with samples. */
    I2S_CB_RDMA,                                /*!< CB generated when the requested channel is sampled. */
} i2s_cb_type_t;


/**
 * \brief           I2S xDMA configurarions structure
 */
typedef struct {
    uint32_t i2s_xdma_start_addr;               /*!< xDMA  start address */
    uint32_t i2s_fw_access_addr;                /*!< Firmware access address */
    uint16_t i2s_xdma_seg_size;                 /*!< xDMA Segment size */
    uint16_t i2s_xdma_blk_size;                 /*!< xDMA Block size */
    uint8_t i2s_xdma_seg_blk_ratio;             /*!< xDMA Segment and Block ratio */
} i2s_xdma_ctrl_ptr_t, i2s_rdma_ctrl_ptr_t, i2s_wdma_ctrl_ptr_t;

/**
 * \brief           I2S callback structure
 */
typedef struct {
    i2s_cb_type_t type;                         /*!< i2s type */
    uint16_t blk_size;                          /*!< xdma blockk size */
    uint16_t seg_size;                          /*!< xdma segment size */
} i2s_cb_t;

/**
 * \brief           i2S interrupt service routine callback for user application.
 * \param[in]       p_cb: the reason of rtc alarm routine trigger
 */
typedef void (*i2s_cb_fn)(i2s_cb_t* p_cb);

/**
 * \brief           I2S configurarions structure
 */
typedef struct {
    i2s_rdma_ctrl_ptr_t* rdma_config;           /*!< rdma config */
    i2s_wdma_ctrl_ptr_t* wdma_config;           /*!< wdma config */
    i2s_sample_rate_t    sr;                    /*!< sample rate */
    i2s_ch_format_t      ch;                    /*!< channel format */
    i2s_trx_mode_t       trx_mode;              /*!< TRX mode */
    i2s_fmt_t            fmt;                   /*!< data format */
    i2s_cfg_txd_wid_t    width;                 /*!< data width */
    i2s_cfg_bck_ratio_t  bck_ratio;             /*!< bck / wrck ratio */
    i2s_mclk_div_t       mck_div;               /*!< mclk division */
    i2s_cfg_bck_osr_t    bck_osr;               /*!< bclk osr setting */
    i2s_cfg_imck_rate_t  imck_rate;             /*!< internal mclk setting */
} i2s_para_set_t;

/**
 * \brief           Get i2s rdma segment size
 * \param[out]      rdma_seg_size: the pointer to get I2S rdma segment size
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2s_get_rdma_seg_size(uint16_t* rdma_seg_size) {
    *rdma_seg_size = (I2S_MASTER->rdma_set0 & 0xFFFF);

    return STATUS_SUCCESS;
};

/**
 * \brief           Get i2s rdma block size
 * \param[out]      rdma_blk_size: the pointer to get I2S rdma block size
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2s_get_rdma_blk_size(uint16_t* rdma_blk_size) {
    *rdma_blk_size = ((I2S_MASTER->rdma_set0 >> 16) & 0xFFFF);

    return STATUS_SUCCESS;
};

/**
 * \brief           Get i2s wdma segment size
 * \param[out]      wdma_seg_size: the pointer to get I2S wdma segment size
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2s_get_wdma_seg_size(uint16_t* wdma_seg_size) {
    *wdma_seg_size = (I2S_MASTER->wdma_set0 & 0xFFFF);

    return STATUS_SUCCESS;
};

/**
 * \brief           Get i2s wdma block size
 * \param[out]      wdma_blk_size: the pointer to get I2S wdma block size
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2s_get_wdma_blk_size(uint16_t* wdma_blk_size) {
    *wdma_blk_size = ((I2S_MASTER->wdma_set0 >> 16) & 0xFFFF);

    return STATUS_SUCCESS;
};

/**
 * \brief           Get i2s rdma access address
 * \param[out]      rdma_address: the pointer to get I2S rdma access address
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2s_get_rdma_access_pos(uint32_t* rdma_address) {
    *rdma_address = I2S_MASTER->rdma_r0;

    return STATUS_SUCCESS;
};

/**
 * \brief           Get i2s wdma access address
 * \param[out]      wdma_address: the pointer to get I2S wdma access address
 * \return          Function status, STATUS_SUCCESS
 */
__STATIC_INLINE uint32_t i2s_get_wdma_access_pos(uint32_t* wdma_address) {
    *wdma_address = I2S_MASTER->wdma_r0;
    
    return STATUS_SUCCESS;
};

/**
 * \brief           Register i2s callback function
 * \param[in]       i2s_cb: user callback function
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t i2s_callback_register(i2s_cb_fn i2s_cb);

/**
 * \brief           Set I2S initialize
 * \param[in]       i2s_para: Specifies the I2S parameter.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t i2s_init(i2s_para_set_t *i2s_para);

/**
 * \brief           Set I2S uninitialize
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t i2s_uninit(void);

/**
 * \brief           Set I2S loopback open
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t i2s_loopback_open(void);

/**
 * \brief           Start I2S
 * \param[in]       i2s_para: Specifies the I2S parameter.
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t i2s_start(i2s_para_set_t *i2s_para);

/**
 * \brief           Stop I2S
 * \return          Function status, STATUS_SUCCESS
 */
uint32_t i2s_stop(void);

/*@}*/ /* end of RT584_DRIVER I2S */

#ifdef __cplusplus
}
#endif

#endif      /* end of I2S_H */

