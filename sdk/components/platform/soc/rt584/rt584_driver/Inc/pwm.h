/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            pwm.h
 * \brief           pwm header file
 */
/*
 * This file is part of library_name.
 * Author:
 */
#ifndef PWM_H
#define PWM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "mcu.h"



/**
 * \defgroup        PWM Pwm
 * \ingroup         RT584_DRIVER
 * \brief           Define Pwm definitions, structures, and functions
 * @{
 */


/**
 * \brief           PWM ID mapping.
 */
typedef enum {
    PWM_ID_0=0,                                   /*!< PWM ID 0 */
    PWM_ID_1,                                   /*!< PWM ID 1 */
    PWM_ID_2,                                   /*!< PWM ID 2 */
    PWM_ID_3,                                   /*!< PWM ID 3 */
    PWM_ID_4,                                   /*!< PWM ID 4 */
    PWM_ID_MAX,                                 /*!< MAX PWM ID  */
} pwm_id_t;

/**
 * \brief           PWM Clock division table.
 */
typedef enum {
    PWM_CLK_DIV_1 = 0,
    PWM_CLK_DIV_2,
    PWM_CLK_DIV_4,
    PWM_CLK_DIV_8,
    PWM_CLK_DIV_16,
    PWM_CLK_DIV_32,
    PWM_CLK_DIV_64,
    PWM_CLK_DIV_128,
    PWM_CLK_DIV_256,
} pwm_clk_div_t;

/**
 * \brief           PWM Sequence order table.
 *                  Order_0: S0  /  Order_1: S1  /  Order_2: S0S1  /  Order_3: S1S0
 */
typedef enum {
    PWM_SEQ_ORDER_R = 0,
    PWM_SEQ_ORDER_T,
    PWM_SEQ_ORDER_MAX,
} pwm_seq_order_t;

/**
 * \brief           PWM sequence selection table.
 */
typedef enum {
    PWM_SEQ_NUM_1,
    PWM_SEQ_NUM_2,
} pwm_seq_num_t;

/**
 * \brief           PWM sequence playmode table.
 */
typedef enum {
    PWM_SEQ_MODE_NONCONTINUOUS,
    PWM_SEQ_MODE_CONTINUOUS,
} pwm_seq_mode_t;

/**
 * \brief           PWM trigger source table.
 */
typedef enum {
    PWM_TRIGGER_SRC_PWM0 = 0,
    PWM_TRIGGER_SRC_PWM1,
    PWM_TRIGGER_SRC_PWM2,
    PWM_TRIGGER_SRC_PWM3,
    PWM_TRIGGER_SRC_PWM4,
    PWM_TRIGGER_SRC_SELF = 5,
} pwm_trigger_src_t;

/**
 * \brief           PWM DMA sample format table.
 */
typedef enum {
    PWM_DMA_SMP_FMT_0 = 0,
    PWM_DMA_SMP_FMT_1,
} pwm_dma_smp_fmt_t;

/**
 * \brief           PWM counter mode table.
 *                  UP: Up mode / UD: Up-Down mode
 */
typedef enum {
    PWM_COUNTER_MODE_UP = 0,
    PWM_COUNTER_MODE_UD,
} pwm_counter_mode_t;

/**
 * \brief           PWM DMA auto table.
 */
typedef enum {
    PWM_DMA_AUTO_DISABLE = 0,
    PWM_DMA_AUTO_ENABLE,
} pwm_dma_auto_t;

/**
 * \brief           PWM Phase table.
 */
typedef enum {
    PWM_PHASE_POSITIVE = 0,
    PWM_PHASE_NEGATIVE,
} pwm_phase_t;

/**
 * \brief           PWM Phase table.
 */
typedef enum {
    PWM_WAVE_INVERT_DISABLE = 0,
    PWM_WAVE_INVERT_ENABLE,
} pwm_invert_t;

/**
 * \brief           PWM clock and rdma enable, interrupt mask, clear and enable value. 
 */
#define PWM_ENABLE_PWM  (0x01UL << PWM_CFG0_PWM_ENA_SHFT)
#define PWM_ENABLE_CLK  (0x01UL << PWM_CFG0_CK_ENA_SHFT)
#define PWM_RESET       (0x01UL << PWM_CFG0_PWM_RST_SHFT)
#define PWM_RDMA_ENABLE (0x01UL << PWM_CFG0_PWM_RDMA0_CTL0_SHFT)
#define PWM_RDMA_RESET  (0x01UL << PWM_CFG0_PWM_RDMA0_CTL1_SHFT)

#define PWM_RDMA0_INT_CLR      (0x01UL << PWM_RDMA0_INT_CLR_SHFT)
#define PWM_RDMA0_ERR_INT_CLR  (0x01UL << PWM_RDMA0_ERR_INT_CLR_SHFT)
#define PWM_RDMA1_INT_CLR      (0x01UL << PWM_RDMA1_INT_CLR_SHFT)
#define PWM_RDMA1_ERR_INT_CLR  (0x01UL << PWM_RDMA1_ERR_INT_CLR_SHFT)
#define PWM_RSEQ_DONE_INT_CLR  (0x01UL << PWM_RSEQ_DONE_INT_CLR_SHFT)
#define PWM_TSEQ_DONE_INT_CLR  (0x01UL << PWM_TSEQ_DONE_INT_CLR_SHFT)
#define PWM_TRSEQ_DONE_INT_CLR (0x01UL << PWM_TRSEQ_DONE_INT_CLR_SHFT)

#define PWM_RDMA0_INT_MASK_ENABLE      (0x01UL << PWM_RDMA0_INT_MASK_SHFT)
#define PWM_RDMA0_ERR_INT_MASK_ENABLE  (0x01UL << PWM_RDMA0_ERR_INT_MASK_SHFT)
#define PWM_RDMA1_INT_MASK_ENABLE      (0x01UL << PWM_RDMA1_INT_MASK_SHFT)
#define PWM_RDMA1_ERR_INT_MASK_ENABLE  (0x01UL << PWM_RDMA1_ERR_INT_MASK_SHFT)
#define PWM_RSEQ_DONE_INT_MASK_ENABLE  (0x01UL << PWM_RSEQ_DONE_INT_MASK_SHFT)
#define PWM_TSEQ_DONE_INT_MASK_ENABLE  (0x01UL << PWM_TSEQ_DONE_INT_MASK_SHFT)
#define PWM_TRSEQ_DONE_INT_MASK_ENABLE (0x01UL << PWM_TRSEQ_DONE_INT_MASK_SHFT)

/**
 * \brief           Defein PWM MAX number and ducy max value
 */
#define PWM_MAX_NUMBER       5
#define PWM_DUTY_MAX_PERCENT 100

/**
 * \brief           PWM default Paramater 
 */
#define PWM_DEFAULT_CNT_END_VAL      0                        /*!< default Count end Value using format0 >*/
#define PWM_DEFAULT_ELEMENT_NUM      1                        /*!< default element number>*/
#define PWM_FMT0_DEFAULT_ELEMENT_NUM 2                        /*!< default forma0 defeault element number>*/
#define PWM_DEFAULT_REPEAT_NUM       0                        /*!< defaultpwm Repeat number>*/
#define PWM_DEFAULT_DLY_NUM          0                        /*!< defaultpwm delay number>*/
#define PWM_DEFAULT_PLAY_CNT         0                        /*!< defaultpwm play counter 0:is infinite>*/

#define PWM_RDMA0_SET_DEFAULT 0                               /*!< defaultCount end Value using format0>*/
#define PWM_RDMA1_SET_DEFAULT 0                               /*!< defaultCount end Value using format0>*/

#define PWM_COUNT_END_VALUE_MAX 32767                         /*!< defaultCount end Value max value>*/
#define PWM_COUNT_END_VALUE_MIN 3                             /*!< defaultCount end Value min value>*/

/**
 *\brief            Pwm set0 control register struct
 */
typedef union pwm_set_s_ {
    struct pwm_set_b_ {
        uint32_t cfg_seq_order    : 1;
        uint32_t cfg_seq_two_sel  : 1;
        uint32_t cfg_seq_mode     : 1;
        uint32_t cfg_pwm_dma_fmt  : 1;
        uint32_t cfg_pwm_cnt_mode : 1;
        uint32_t cfg_pwm_cnt_trig : 1;
        uint32_t cfg_dma_auto     : 1;
        uint32_t RVD1             : 1;
        uint32_t cfg_ck_div       : 4;
        uint32_t cfg_pwm_ena_trig : 3;
        uint32_t RVD2             : 1;
        uint32_t RVD3             : 4;
        uint32_t RVD4             : 12;
    } bit;

    uint32_t Reg;
} pwm_set_t;

/**
 *\brief            Pwm format 1 register struct
 */
typedef union pwm_fmt1_s {
    struct pwm_fmt1_b {
        uint32_t cunt_end : 16;
        uint32_t phase    : 1;
        uint32_t Thd      : 15;
    } bit;

    uint32_t Reg;
} pwm_fmt1_t;

/**
 *\brief            Pwm format 0 register struct
 */
typedef union pwm_fmt0_s {
    struct pwm_fmt0_b {
        uint32_t phase1 : 1;
        uint32_t Thd1   : 15;
        uint32_t phase2 : 1;
        uint32_t Thd2   : 15;
    } bit;

    uint32_t Reg;
} pwm_fmt0_t;

/**
 *\brief            Pwm interrupt struct
 */
typedef union pwm_int_s {
    struct pwm_int_b {
        uint32_t    rdma0_int           : 1;
        uint32_t    rdma0_err_int       : 1;
        uint32_t    rdma1_int           : 1;
        uint32_t    rdma1_err_int       : 1;
        uint32_t    rseq_done_int       : 1;
        uint32_t    tseq_done_int       : 1;
        uint32_t    rtseq_done_int      : 1;
        uint32_t    reserved            : 25;
    } bit;

    uint32_t Reg;
} pwm_int_t;

/**
 * \brief           Convert THD_Value / End_Value / PHA_Value into a 32-bit data
 *                  Mode0: val1=THD1, val2=THD2
 *                  Mode1: val0=PHA, val1=THD, val2=end
 */
#define PWM_FILL_SAMPLE_DATA_MODE0(val0,val1,val2)  ((val0 << 31) | (val2 << 16) | (val0 << 15) | (val1))
#define PWM_FILL_SAMPLE_DATA_MODE1(val0,val1,val2)  ((val2 << 16) | (val0 << 15) | (val1))

/**
 * \brief           pwm data mode struct
 */
typedef struct {
    uint32_t rdma_addr;                        /*!< xDMA start address configurations for PWM sequence controller.> */
    uint16_t element_num;                      /*!< Element number configurationsfor PWM sequence controller.> */
    uint16_t repeat_num;                       /*!< Repeat number configurationsof each element for PWM sequence controller.> */
    uint16_t delay_num;                        /*!< Delay number configurations after PWM sequence is play finish for PWM sequence controller.> */
} pwm_data_mode1_t;

/**
 * \brief           pwm interrupt call back function typedef.
 */
typedef void (*pwm_callback_t)(uint32_t id, void* p_arg);

/**
 * \brief           Structure for each RDMA configurations
 */
typedef struct {
    uint32_t rdma_addr;                         /*!< xDMA start address configurationsfor PWM sequence controller.> */
    uint16_t element_num;                       /*!< Element number configurationsfor PWM sequence controller. >*/
    uint16_t repeat_num;                        /*!< Repeat number configurationsof each element for PWM sequence controller.> */
    uint16_t delay_num;                         /*!< Delay number configurations after PWM sequence is play finish for PWM sequence controller.> */
} pwm_seq_para_t;

/**
 * \brief           Structure for each PWM configurations
 */
typedef struct {
    pwm_seq_para_t pwm_seq0;                    /*!< Handle of PWM sequence controller configurations for R-SEQ. */
    pwm_seq_para_t pwm_seq1;                    /*!< Handle of PWM sequence controller configurations for T-SEQ. */
    uint16_t pwm_play_cnt;                      /*!< PWM play amount configuration. */
    uint16_t pwm_count_end_val;                 /*!< PWM counter end value configuration. */
    pwm_seq_order_t pwm_seq_order;              /*!< PWM sequence play order configuration. */
    pwm_trigger_src_t pwm_triggered_src;        /*!< PWM play trigger source configuration. */
    pwm_seq_num_t pwm_seq_num;                  /*!< PWM sequence number configuration. */
    pwm_id_t pwm_id;                            /*!< PWM ID designation. */
    pwm_clk_div_t pwm_clk_div;                  /*!< PWM gated clock divider value configuration. */
    pwm_counter_mode_t pwm_counter_mode;        /*!< PWM counter mode configuration. */
    pwm_dma_smp_fmt_t pwm_dma_smp_fmt;          /*!< PWM DMA sample format configuration. */
    pwm_seq_mode_t pwm_seq_mode;                /*!< PWM sequence play mode configuration. */
} pwm_seq_para_head_t;

/**
 * \brief           Structure for each PWM configurations
 */
typedef struct {
    pwm_id_t id;                                /*!< PWM ID designation >*/
    pwm_seq_num_t seq_num;                      /*!< PWM sequence number configuration.> */
    pwm_clk_div_t clk_div;                      /*!< PWM gated clock divider value configuration.> */
    pwm_phase_t phase;                          /*!< PWM phase enum configuration.> */
    pwm_counter_mode_t counter_mode;            /*!< PWM counter mode configuration.> */
    pwm_dma_smp_fmt_t dma_smp_fmt;              /*!< PWM DMA sample format configuration.> */
    pwm_seq_mode_t seq_mode;                     /*!< PWM sequence play mode configuration.> */
    pwm_trigger_src_t triggered_src;            /*!< PWM play trigger source configuration.> */
    pwm_seq_order_t seq_order_1st;              /*!< PWM sequence play order configuration.> */
    pwm_seq_order_t seq_order_2nd;              /*!< PWM sequence play order configuration.> */
    uint16_t play_cnt;                          /*!< PWM play amount configuration.> */
    uint16_t count_end_val;                     /*!< PWM counter end value configuration.> */
    pwm_seq_para_t rseq;                        /*!< Handle of PWM sequence controller configurations for R-SEQ.> */
    pwm_seq_para_t tseq;                        /*!< Handle of PWM sequence controller configurations for T-SEQ.> */
    uint32_t frequency;                         /*!< pwm frequency value > */
    uint32_t pin_out;                           /*!< pwm pin output .> */
    pwm_seq_order_t seq_order;                  /*!< PWM sequence play order configuration. */
    pwm_invert_t    invert;                     /*!< PWM output wave inter. */
} pwm_config_t;

/**
 * \brief           Function to initinal formation 1 function
 * \param[in]       id: pwm
 * \param[in]       freqency: pwm frequency
 * \retval          STATUS_SUCCESS config pwm irq,clock,rdma registers is vaild
 * \retval          STATUS_INVALID_PARAM config pwm irq,clock,rdma registers is invaild
 */
uint32_t pwm_init_fmt1(uint32_t id, uint32_t freqency);

/**
 * \brief           Function to initinal formation 0 function
 * \param[in]       id: pwm
 * \param[in]       freqency: pwm frequency
 * \param[in]       count_end_value: pwm counter end value
 * \retval          STATUS_SUCCESS config pwm irq,clock,rdma registers is vaild
 * \retval          STATUS_INVALID_PARAM config pwm irq,clock,rdma registers is invaild
 */
uint32_t pwm_init_fmt0(uint32_t id, uint32_t freqency,
                       uint32_t count_end_value);
/**
 * \brief           Get pmw freqnecy
 * \param[in]       id: pwm 
 * \param[in]       freqency: pwm frequency
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_get_frequency(uint32_t id, uint32_t* get_frequency);

/**
 * \brief           Set pmw freqnecy
 * \param[in]       id: pwm 
 * \param[in]       freqency: pwm frequency
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_frequency(uint32_t id, uint32_t freqency);

/**
 * \brief           Set pmw clock divider
 * \param[in]       id: pwm 
 * \param[in]       pwm_clk_div: pwm clock dividere enum
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_clock_divider(pwm_id_t id, pwm_clk_div_t pwm_clk_div);

/**
 * \brief           Get pmw phase
 * \param[in]       id: pwm 
 * \param[in]       get_phase: pwm phase
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_get_pahse(uint32_t id, uint32_t* get_phase);

/**
 * \brief           Set pmw phase
 * \param[in]       id: pwm 
 * \param[in]       pwm_phase_t: pwm phase
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_pahse(uint32_t id, pwm_phase_t phase);

/**
 * \brief           Get pmw count
 * \param[in]       id: pwm 
 * \param[in]       full_count: pwm full count
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_get_count(uint32_t id, uint32_t* full_count);

/**
 * \brief           Set pmw count
 * \param[in]       id: pwm 
 * \param[in]       counter_mode: pwm counter mode
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_counter_mode(uint32_t id, pwm_counter_mode_t counter_mode);

/**
 * \brief           Set pmw count end value
 * \param[in]       id: pwm 
 * \param[in]       counter_end_value: pwm counter end value
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_counter_end_value(uint32_t id, uint32_t counter_end_value);

/**
 * \brief           Set pmw dma format mode
 * \param[in]       id: pwm 
 * \param[in]       format: pwm dma format mode
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_dma_format(uint32_t id, pwm_dma_smp_fmt_t format);

/**
 * \brief           Set pmw format 1 duty
 * \param[in]       id: pwm 
 * \param[in]       duty: pwm duty
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_fmt1_duty(uint32_t id, uint8_t duty);

/**
 * \brief           Set pmw format 0 duty
 * \param[in]       id: pwm 
 * \param[in]       duty: pwm duty
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_fmt0_duty(uint32_t id, uint8_t duty);

/**
 * \brief           Set pmw format 1 counter
 * \param[in]       id: pwm 
 * \param[in]       count: pwm count value
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_fmt1_count(uint32_t id, uint32_t count);

/**
 * \brief           Set pmw format 0 counter
 * \param[in]       id: pwm 
 * \param[in]       count: pwm count value
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_fmt0_count(uint32_t id, uint32_t count);

/**
 * \brief           Pmw multi element initinal counter
 * \param[in]       pwm_cfg: pwm config paramater
 * \param[in]       freqency: pwm freqency
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_multi_init(pwm_config_t pwm_cfg, uint32_t freqency);

/**
 * \brief           Pmw multi element format 1 duty
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       element: pwm element
 * \param[in]       duty: pwm duty value
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_multi_fmt1_duty(uint32_t id, pwm_seq_order_t seq_order,
                             uint32_t element, uint8_t duty);
/**
 * \brief           Pmw multi element format 0 duty
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       element: pwm element
 * \param[in]       thd1_duty: pwm duty value
 * \param[in]       thd2_duty: pwm duty value
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_multi_fmt0_duty(uint32_t id, pwm_seq_order_t seq_order,
                             uint32_t element, uint8_t thd1_duty,
                             uint8_t thd2_duty);

/**
 * \brief           Pmw multi element format 1 count
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       element: pwm element
 * \param[in]       count: pwm count
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_multi_fmt1_count(uint32_t id, pwm_seq_order_t seq_order,
                              uint32_t element, uint32_t count);

/**
 * \brief           Pmw multi element format 0 count
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       element: pwm element
 * \param[in]       thd1_Count: pwm count
 * \param[in]       thd2_count: pwm count
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_multi_fmt0_count(uint32_t id, pwm_seq_order_t seq_order,
                              uint32_t element, uint32_t thd1_Count,
                              uint32_t thd2_count);

/**
 * \brief           Pmw set repeat number value
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       repeat_number: pwm repeat number
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_repeat_number(uint32_t id, pwm_seq_order_t seq_order,
                               uint32_t repeat_number);

/**
 * \brief           Pmw get repeat number value
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       repeat_number: pwm repeat number
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_get_repeat_number(uint32_t id, pwm_seq_order_t seq_order,
                               uint32_t* get_repeat_number);

/**
 * \brief           Pmw set delay number value
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       dly_number: pwm delay number
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_delay_number(uint32_t id, pwm_seq_order_t seq_order,
                              uint32_t dly_number);

/**
 * \brief           Pmw get delay number value
 * \param[in]       id: pwm id
 * \param[in]       seq_order: pwm sequence order
 * \param[in]       get_delay_number: pwm delay number
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_get_delay_number(uint32_t id, pwm_seq_order_t seq_order,
                              uint32_t* get_delay_number);

/**
 * \brief           Pmw set dma element number value
 * \param[in]       id: pwm id
 * \param[in]       element: pwm element value 
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_dma_element(uint32_t id, uint32_t element);

/**
 * \brief           Pmw get dma element number value
 * \param[in]       id: pwm id
 * \param[in]       get_element: pwm element value 
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_get_dma_element(uint32_t id, uint32_t* get_element);

/**
 * \brief           Pmw set wave invert 
 * \param[in]       id: pwm id
 * \param[in]       invert: pwm wave invert disable or enable 
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_set_inveter(uint32_t id, uint32_t invert);

/**
 * \brief           Pmw get wave invert
 * \param[in]       id: pwm id
 * \param[in]       get_invert: pwm element value 
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_get_inveter(uint32_t id, uint32_t *get_invert);

/**
 * \brief           Pmw start function value
 * \param[in]       id: pwm id
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_start(uint32_t id);

/**
 * \brief           Pmw stop function value
 * \param[in]       id: pwm id
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_stop(uint32_t id);

/**
 * \brief           pwm_register_callback_function
 * \param[in]       id: pwm id
 * \param[in]       cb_fun: pwm call back function
 * \retval          STATUS_SUCCESS
 * \retval          STATUS_INVALID_PARAM
 */
uint32_t pwm_register_callback_function(uint32_t id, pwm_callback_t cb_fun);

/*@}*/ /* end of RT584_DRIVER PWM */

#ifdef __cplusplus
}
#endif

#endif /* End of PWM_H */
