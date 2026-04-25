/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           irm.h
 * \brief          IRM driver header file
 */
/*
 * Author:         Kc.tseng
 */


#ifndef IRM_H
#define IRM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <string.h>
#include "mcu.h"



/**
 * \defgroup        IRM Irm
 * \ingroup         RT584_DRIVER
 * \brief           Define Irm definitions, structures, and functions
 * @{
 */


/**
 *  \brief          IRM normal definitions
 */
#define IR_ENV_INI_HIGH (BIT0)
#define IR_ENV_INI_LOW (0)

#define IR_MODE_MASK ((~BIT1))
#define IR_OUT_MODE_MASK ((~(BIT2|BIT3)))

#define IR_BUFFER_MAX_LENGTH (2*8*32)           /*!< 2(high+low)*8bits*32, i.e, 32bytes */
#define IR_BUFFER_FIRST_FILLIN_LENGTH (16)
#define IR_BUFFER_INT_LENGTH (4)                /*!< HALF OF IR_BUFFER_FIRST_FILLIN_LENGTH */

#define IR_EVELOPE_HIGH (1)
#define IR_EVELOPE_LOW (0)

#define ENV_START_INT 1
#define ENV_LAST_INT  2
#define FIFO_UFL_INT  4

#define IR_ENV_START_INT_EN 1
#define IR_ENV_LAST_INT_EN  2
#define IR_FIFO_UFL_INT_EN  4
#define IR_ALL_INT_EN (IR_ENV_START_INT_EN|IR_ENV_LAST_INT_EN|IR_FIFO_UFL_INT_EN)

/**
 *  \brief          NEC Definitions
 */
#define NEC_CARRIER_LOW_CNT (1)
#define NEC_CARRIER_HIGH_CNT (0)
#define NEC_CARRIER_BASEMENT_CNT (281)

#define NEC_OFFSET (1)

#if 1
#define NEC_START_HIGHT (342+NEC_OFFSET)        /*!< 9000us / 26.34375 us = 341.6 */
#define NEC_START_LOW (171+NEC_OFFSET)          /*!< 4500us / 26.34375 us = 170.8 */
#define NEC_ONE_HIGHT (21+NEC_OFFSET)           /*!< 560us / 26.34375 us = 21.25 */
#define NEC_ONE_LOW (64+NEC_OFFSET)             /*!< 1690us / 26.34375 us = 64.1 */
#define NEC_ZERO_HIGHT (21+NEC_OFFSET)          /*!< 560us / 26.34375 us = 21.25 */
#define NEC_ZERO_LOW (21+NEC_OFFSET)            /*!< 560us / 26.34375 us = 21.25 */
#define NEC_END_BUST (21+NEC_OFFSET)            /*!< 560us / 26.34375 us = 21.25 */
#define NEC_REPEAT_LOW (85+NEC_OFFSET)          /*!< 2250us / 26.34375 us = 85.40 */
#define NEC_110MS (4176+NEC_OFFSET)             /*!< (4176)//110000us / 26.34375 us = 4175.5 */
#else
#define NEC_START_HIGHT (360+NEC_OFFSET)        /*!< 9000us / 25 us = 360 */
#define NEC_START_LOW (144+NEC_OFFSET)          /*!< 4500us / 25 us = 180 */
#define NEC_ONE_HIGHT (22+NEC_OFFSET)           /*!< 560us / 25 us = 22.4 */
#define NEC_ONE_LOW (67+NEC_OFFSET)             /*!< 1690us / 25 us = 67.6 */
#define NEC_ZERO_HIGHT (22+NEC_OFFSET)          /*!< 560us / 25 us = 22.4 */
#define NEC_ZERO_LOW (22+NEC_OFFSET)            /*!< 560us / 25 us = 22.4 */
#define NEC_END_BUST (22+NEC_OFFSET)            /*!< 560us / 25 us = 22.4 */
#define NEC_REPEAT_LOW (90+NEC_OFFSET)          /*!< 2250us / 25 us = 90 */
#define NEC_110MS (4400+NEC_OFFSET)             /*!< 110000us / 25 us = 3520 */
#endif

/**
 *  \brief          SIRC Definitions
 */
#define SIRC_CARRIER_LOW_CNT (1)
#define SIRC_CARRIER_HIGH_CNT (0)
#define SIRC_CARRIER_BASEMENT_CNT (267)
#define SIRC_OFFSET (-1)
#define SIRC_START_HIGHT (96+SIRC_OFFSET)       /*!< 2400us / 25 us = 96 */
#define SIRC_START_LOW (24+SIRC_OFFSET)         /*!< 600us / 25 us = 24 */
#define SIRC_ONE_HIGHT (48+SIRC_OFFSET)         /*!< 1200us / 25 us = 48 */
#define SIRC_ONE_LOW (SIRC_START_LOW)
#define SIRC_ZERO_HIGHT (SIRC_START_LOW)        /*!< 600us / 25 us = 24 */
#define SIRC_ZERO_LOW (SIRC_START_LOW)
#define SIRC_END_LOW (400+SIRC_START_LOW)       /*!< 45000us+600us / 25 us = 1800+24 */

/**
 *  \brief          RC6 Definitions
 */
#define RC6_CARRIER_LOW_CNT (1)
#define RC6_CARRIER_HIGH_CNT (0)
#define RC6_CARRIER_BASEMENT_CNT (296)
#define RC6_OFFSET (0)
#define RC6_START_HIGHT (96+RC6_OFFSET)         /*!< 2666us = t*6 = 1/36000*16*6 */
#define RC6_START_LOW (32+RC6_OFFSET)           /*!< 889us = t*62 = 1/36000*16*2 */
#define RC6_TR_HIGHT (32+RC6_OFFSET)            /*!< 889us = t*2 = 1/36000*16*2 */
#define RC6_TR_LOW (32+RC6_OFFSET)              /*!< 889us = t*2 = 1/36000*16*2 */

#define RC6_ONE_HIGHT (16+RC6_OFFSET)           /*!< 444us = t = 1/36000*16 */
#define RC6_ONE_LOW (RC6_ONE_HIGHT)             /*!< 444us = t = 1/36000*16 */
#define RC6_ZERO_HIGHT (RC6_ONE_HIGHT)          /*!< 444us = t = 1/36000*16 */
#define RC6_ZERO_LOW (RC6_ONE_HIGHT)            /*!< 444us = t = 1/36000*16 */
#define RC6_END_LOW (RC6_START_HIGHT)           /*!< 2666us = t*6 = 1/36000*16*6 */

/**
 * \brief           IRM operaion mode
 */
typedef enum ir_mode {
    NORMAL_MODE,                                /*!< IR normal mode */
    AUTO_MODE                                   /*!< IR auto mode */
} ir_mode_t;

/**
 * \brief           IRM output logic function
 */
typedef enum ir_out_mode {
    AND,                                        /*!< envelope AND carrier */
    OR,                                         /*!< envelope OR carrier */
    NAND,                                       /*!< envelope NAND carrier */
    NOR                                         /*!< envelope NOR carrier */
} ir_out_mode_t;

/**
 * \brief           IRM protocol
 */
typedef enum ir_potocol {
    RC5,                                        /*!< RC5 protocol */
    RC6,                                        /*!< RC6 protocol */
    RC_MN,                                      /*!< RC_MN protocol */
    NEC,                                        /*!< NEC protocol */
    SIRC_12bits = 5,                            /*!< SIRC_12 protocol */
    SIRC_15bits = 8                             /*!< SIRC_15 protocol */
} ir_potocol_t;

/**
 * \brief           IRM fifo
 */
typedef union {
    uint32_t u32;                               /*!< 32bits info */
    struct ir_fifo {
        uint16_t env_cnt;                       /*!< env count */
        uint8_t  envelope: 1;                   /*!< envelope high/low */
        uint8_t  env_int: 1;                    /*!< env interrupt */
        uint8_t  env_last: 1;                   /*!< last envelope */
        uint16_t reserve: 13;                   /*!< resvered bits */
    } bf;
} ir_fifo_t;

/**
 * \brief           IRM buffer
 */
typedef struct ir_buffer {
    ir_fifo_t *buf ;                            /*!<  */
    uint16_t bufsize_mask;                      /*!< env count */
    volatile uint32_t wr_idx;                   /*!< write idx */
    volatile uint32_t rd_idx;                   /*!< read idx */
    uint32_t total_usage_cnt;                   /*!< total count */
} ir_buffer_t;

/**
 * \brief           IRM state
 */
typedef enum ir_state {
    IDLE,                                       /*!< IR idle state */
    TRANSMITTING,                               /*!< IR transmitting state */
    DONE                                        /*!< IR done state */
} ir_state_t;

/**
 * \brief           IRM finish routine callback for user application.
 * \param[in]       status: IRM transfer status.
 * \details         This callback function is still running in interrupt mode, so this function
 *                  should be as short as possible. It can NOT call any block function in this
 *                  callback service routine.
 *                  This function will be called when IRM finished the transfer envelope, or there
 *                  is no envelope in fifo during the transfer (error case).
 */
typedef void (* irm_proc_cb)(uint32_t status);

/**
 * \brief           IRM configuration parameter
 */
typedef struct {
    irm_proc_cb   irm_cb_func;                  /*!< irm callback function */
    ir_mode_t     op_mode;                      /*!< irm opration mode */
    ir_out_mode_t ir_out_mode;                  /*!< irm output mode */
    uint32_t      irm_int_en;                   /*!< irm interrupt enable */
} irm_mode_t;


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
uint32_t ir_carrier_config(uint8_t carrier_high_cnt, uint8_t carrier_low_cnt, uint16_t carrier_base_cnt);

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
uint32_t irm_open(irm_mode_t *irm_cfg);

/**
 * \brief           Send NEC protocol
 * \param[in]       cmd: command
 * \param[in]       address: address
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to send NEC protocol data.
 */
uint32_t ir_nec_encoder(uint16_t cmd, uint16_t address);

/**
 * \brief           Send NEC protocol repeat command
 * \details
 *                  Call this function to send NEC protocol repeat command.
 */
uint32_t ir_nec_repeat_encoder(void);

/**
 * \brief           Send SIRC protocol
 * \param[in]       cmd: command
 * \param[in]       address: address
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to send SIRC protocol data.
 */
uint32_t ir_sirc_encoder(uint8_t cmd, uint8_t address, uint8_t address_length);

/**
 * \brief           Send RC6 protocol
 * \param[in]       cmd: command
 * \param[in]       address: address
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to send RC6 protocol data.
 */
uint32_t ir_rc6_encoder(uint16_t cmd, uint8_t address, uint8_t RC6_toggle);

/**
 * \brief           Set irm fifo first
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to set irm fifo data.
 */
uint32_t ir_buffer_fill_in(void);

/**
 * \brief           Set irm fifo
 * \return          STATUS_SUCCESS
 * \details
 *                  Call this function to set irm fifo data.
 */
uint32_t ir_buffer_repeat_fill_in(void);

/**
 * \brief           Enable irm
 * \return          STATUS_SUCCESS
 */
uint32_t ir_enable(void);

/**
 * \brief           Disable irm
 * \return          STATUS_SUCCESS
 */
uint32_t ir_disable(void);

/**
 * \brief           Start irm
 * \return          STATUS_SUCCESS
 */
uint32_t ir_start(void);

/**
 * \brief           Stop irm
 * \return          STATUS_SUCCESS
 */
uint32_t ir_stop(void);

/**
 * \brief           Get ir fifo level
 * \param[out]      fifo_lvl: the pointer to get fifo level([4:0])
 * \return          STATUS_SUCCESS
 */
uint32_t ir_fifo_level(uint32_t *fifo_lvl);

/**
 * \brief           Get ir fifo empty
 * \param[out]      empty: the pointer to get fifo empty
 *                         0: Fifo not empty, 1: Fifo empty
 * \return          STATUS_SUCCESS
 */
uint32_t ir_fifo_empty(uint32_t *empty);

/**
 * \brief           Get ir fifo Full
 * \param[out]      full: the pointer to get fifo full
 *                        0: Fifo not full, 1: Fifo full
 * \return          STATUS_SUCCESS
 */
uint32_t ir_fifo_full(uint32_t *full);

/*@}*/ /* end of RT584_DRIVER IRM */

#ifdef __cplusplus
}
#endif

#endif /* End of IRM_H */
