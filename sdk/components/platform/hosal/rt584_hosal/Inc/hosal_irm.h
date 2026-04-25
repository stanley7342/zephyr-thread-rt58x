/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           hosal_irm.h
 * \brief          Hosal IRM driver header file
 */
/*
 * Author:         Kc.tseng
 */


#ifndef HOSAL_IRM_H
#define HOSAL_IRM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "irm.h"



/**
 * \defgroup        HOSAL_IRM Hosal irm
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal irm definitions, structures, and functions
 * @{
 */

/**
 *  \brief          NEC Definitions
 */
#define HOSAL_NEC_CARRIER_LOW_CNT       NEC_CARRIER_LOW_CNT
#define HOSAL_NEC_CARRIER_HIGH_CNT      NEC_CARRIER_HIGH_CNT
#define HOSAL_NEC_CARRIER_BASEMENT_CNT  NEC_CARRIER_BASEMENT_CNT

/**
 *  \brief          SIRC Definitions
 */
#define HOSAL_SIRC_CARRIER_LOW_CNT          SIRC_CARRIER_LOW_CNT
#define HOSAL_SIRC_CARRIER_HIGH_CNT         SIRC_CARRIER_HIGH_CNT
#define HOSAL_SIRC_CARRIER_BASEMENT_CNT     SIRC_CARRIER_BASEMENT_CNT

/**
 *  \brief          RC6 Definitions
 */
#define HOSAL_RC6_CARRIER_LOW_CNT       RC6_CARRIER_LOW_CNT
#define HOSAL_RC6_CARRIER_HIGH_CNT      RC6_CARRIER_HIGH_CNT
#define HOSAL_RC6_CARRIER_BASEMENT_CNT  RC6_CARRIER_BASEMENT_CNT

/**
 * \brief           IRM operaion mode
 */

#define HOSAL_NORMAL_MODE NORMAL_MODE           /*!< IR normal mode */
#define HOSAL_AUTO_MODE AUTO_MODE               /*!< IR auto mode */

/**
 * \brief           IRM output logic function
 */
#define HOSAL_AND   AND                         /*!< envelope AND carrier */
#define HOSAL_OR    OR                          /*!< envelope OR carrier */
#define HOSAL_NAND  NAND                        /*!< envelope NAND carrier */
#define HOSAL_NOR   NOR                         /*!< envelope NOR carrier */

/**
 * \brief           IRM protocol
 */
#define HOSAL_SIRC_12bits SIRC_12bits           /*!< SIRC_12 protocol */
#define HOSAL_SIRC_15bits SIRC_15bits           /*!< SIRC_15 protocol */


/**
 * \brief           IRM configuration parameter
 */
typedef struct {
    irm_proc_cb irm_cb_func;                    /*!< irm callback function */
    uint32_t    op_mode;                        /*!< irm opration mode */
    uint32_t    ir_out_mode;                    /*!< irm output mode */
    uint32_t    irm_int_en;                     /*!< irm interrupt enable */
} hosal_irm_mode_t;

__STATIC_INLINE uint8_t hosal_onebyte_bitreverse(uint8_t in_byte) {
    uint8_t out_byte;
    out_byte = in_byte;
    out_byte = (((out_byte & 0xaa) >> 1) | ((out_byte & 0x55) << 1));
    out_byte = (((out_byte & 0xcc) >> 2) | ((out_byte & 0x33) << 2));
    out_byte = ((out_byte >> 4) | (out_byte << 4));

    return out_byte;
}

/**
 * \brief           Set IRM carrier
 * \param[in]       carrier_high_cnt: 
 * \param[in]       carrier_low_cnt: 
 * \param[in]       carrier_base_cnt: 
 *                  carrier frequercy = irm_sclk(32M) / 
 *                  (carrier_base_cnt * (carrier_high_cnt + carrier_low_cnt + 2))
 *                  duty cycle = (carrier_high_cnt + 1) / 
 *                  (carrier_high_cnt + carrier_low_cnt + 2)
 * \return          STATUS_SUCCESS
 * \details
 *       Call this function to setup IRM carrier frequency and duty cycle.
 */
uint32_t hosal_ir_carrier_config(uint8_t carrier_high_cnt, uint8_t carrier_low_cnt, uint16_t carrier_base_cnt);

/**
 * \brief           Set IRM initialize
 * \param[in]       irm_cfg: irm config
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to initail IRM, the whole IRM
 *                  driver is interrupt-driven, all IRM response are processing
 *                  in user "irm_cb_func" callback function. Please Notice the
 *                  the function "irm_cb_func" is in ISR context, so finish the
 *                  task ASAP. You can NOT block the function!
 */
uint32_t hosal_irm_open(hosal_irm_mode_t *irm_cfg);

/**
 * \brief           Send NEC protocol
 * \param[in]       cmd: command
 * \param[in]       address: address
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to send NEC protocol data.
 */
uint32_t hosal_ir_nec_encoder(uint16_t cmd, uint16_t address);

/**
 * \brief           Send NEC protocol repeat command
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to send NEC protocol repeat command.
 */
uint32_t hosal_ir_nec_repeat_encoder(void);

/**
 * \brief           Send SIRC protocol
 * \param[in]       cmd: command
 * \param[in]       address: address
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to send SIRC protocol data.
 */
uint32_t hosal_ir_sirc_encoder(uint8_t cmd, uint8_t address, uint8_t address_length);

/**
 * \brief           Send RC6 protocol
 * \param[in]       cmd: command
 * \param[in]       address: address
 * \param[in]       rc6_toggle: toggle or not
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to send RC6 protocol data.
 */
uint32_t hosal_ir_rc6_encoder(uint16_t cmd, uint8_t address, uint8_t rc6_toggle);

/**
 * \brief           Set irm fifo first
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to set irm fifo data.
 */
uint32_t hosal_ir_buffer_fill_in(void);

/**
 * \brief           Set irm fifo
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to set irm fifo data.
 */
uint32_t hosal_ir_buffer_repeat_fill_in(void);

/**
 * \brief           Enable irm
 * \return          STATUS_SUCCESS
 */
uint32_t hosal_ir_enable(void);

/**
 * \brief           Disable irm
 * \return          STATUS_SUCCESS
 */
uint32_t hosal_ir_disable(void);

/**
 * \brief           Start irm
 * \return          STATUS_SUCCESS
 */
uint32_t hosal_ir_start(void);

/**
 * \brief           Stop irm
 * \return          STATUS_SUCCESS
 */
uint32_t hosal_ir_stop(void);

/**
 * \brief           Get ir fifo level
 * \param[out]      fifo_lvl: the pointer to get fifo level([4:0])
 * \return          STATUS_SUCCESS
 */
uint32_t hosal_ir_fifo_level(uint32_t *fifo_lvl);

/**
 * \brief           Get ir fifo empty
 * \param[out]      empty: the pointer to get fifo empty
 *                         0: Fifo not empty, 1: Fifo empty
 * \return          STATUS_SUCCESS
 */
uint32_t hosal_ir_fifo_empty(uint32_t *empty);

/**
 * \brief           Get ir fifo Full
 * \param[out]      full: the pointer to get fifo full
 *                        0: Fifo not full, 1: Fifo full
 * \return          STATUS_SUCCESS
 */
uint32_t hosal_ir_fifo_full(uint32_t *full);

/*@}*/ /* end of RT584_HOSAL HOSAL_IRM */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_IRM_H */
