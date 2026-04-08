/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_pwm.h
 * \brief           hosal_pwm include file
 */

/*
 * This file is part of library_name.
 * Author:         ives.lee
 */
#ifndef HOSAL_PWM_H
#define HOSAL_PWM_H

#include "pwm.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup HOSAL_PWM Hosal pwm
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal pwm definitions, structures, and functions
 * @{
 */

/**
 * \brief           hosal pwm iocontrl const defind
 */
#define HOSAL_PWM_SET_COUNT_MODE      1
#define HOSAL_PWM_SET_FRQUENCY        2
#define HOSAL_PWM_SET_DELAY_NUMBER    3
#define HOSAL_PWM_SET_REPEAT_NUMBER   4
#define HOSAL_PWM_SET_PLAY_NUMBER     5
#define HOSAL_PWM_SET_TSEQ_ADDRESS    6
#define HOSAL_PWM_SET_RSEQ_ADDRESS    7
#define HOSAL_PWM_GET_COUNT_MODE      8
#define HOSAL_PWM_GET_FRQUENCY        9
#define HOSAL_PWM_GET_DUTY            10
#define HOSAL_PWM_GET_COUNT           11
#define HOSAL_PWM_GET_DELAY_NUMBER    12
#define HOSAL_PWM_GET_REPEAT_NUMBER   13
#define HOSAL_PWM_GET_PLAY_NUMBER     14
#define HOSAL_PWM_GET_TSEQ_ADDRESS    15
#define HOSAL_PWM_GET_RSEQ_ADDRESS    16
#define HOSAL_PWM_SET_CLOCK_DIVIDER   17
#define HOSAL_PWM_GET_PHASE           18
#define HOSAL_PWM_SET_PHASE           19
#define HOSAL_PWM_SET_COUNT_END_VALUE 20
#define HOSAL_PWM_SET_DMA_FORMAT      21
#define HOSAL_PWM_GET_INVERT          22
#define HOSAL_PWM_SET_INVERT          23 
#define HOSAL_PWM_DISABLE_INTERRUPT   24
#define HOSAL_PWM_ENABLE_INTERRUPT    25
#define HOSAL_PWM_REGISTER_CALLBACK   26

/**
 * \brief           hosal pwm config struce defined
 */
#define HOSAL_PWM_ID_0                         PWM_ID_0
#define HOSAL_PWM_ID_1                         PWM_ID_1
#define HOSAL_PWM_ID_2                         PWM_ID_2
#define HOSAL_PWM_ID_3                         PWM_ID_3
#define HOSAL_PWM_ID_4                         PWM_ID_4

/**
 * \brief           PWM Clock division table.
 */
 #define HOSAL_PWM_CLK_DIV_1    PWM_CLK_DIV_1
 #define HOSAL_PWM_CLK_DIV_2    PWM_CLK_DIV_2
 #define HOSAL_PWM_CLK_DIV_4    PWM_CLK_DIV_4
 #define HOSAL_PWM_CLK_DIV_8    PWM_CLK_DIV_8
 #define HOSAL_PWM_CLK_DIV_16   PWM_CLK_DIV_16
 #define HOSAL_PWM_CLK_DIV_32   PWM_CLK_DIV_32
 #define HOSAL_PWM_CLK_DIV_64   PWM_CLK_DIV_64
 #define HOSAL_PWM_CLK_DIV_128  PWM_CLK_DIV_128
 #define HOSAL_PWM_CLK_DIV_256  PWM_CLK_DIV_256


/**
 * \brief           PWM Sequence order table.
 *                  Order_0: S0  /  Order_1: S1  /  Order_2: S0S1  /  Order_3: S1S0
 */
#define HOSAL_PWM_SEQ_ORDER_R PWM_SEQ_ORDER_R
#define HOSAL_PWM_SEQ_ORDER_T PWM_SEQ_ORDER_T

/**
 * \brief           PWM sequence selection table.
 */
#define HOSAL_PWM_SEQ_NUM_1 PWM_SEQ_NUM_1
#define HOSAL_PWM_SEQ_NUM_2 PWM_SEQ_NUM_2


/**
 * \brief           PWM sequence playmode table.
 */
#define HOSAL_PWM_SEQ_MODE_NONCONTINUOUS PWM_SEQ_MODE_NONCONTINUOUS
#define HOSAL_PWM_SEQ_MODE_CONTINUOUS    PWM_SEQ_MODE_CONTINUOUS


/**
 * \brief           PWM trigger source table.
 */
#define HOSAL_PWM_TRIGGER_SRC_PWM0 PWM_TRIGGER_SRC_PWM0
#define HOSAL_PWM_TRIGGER_SRC_PWM1 PWM_TRIGGER_SRC_PWM1
#define HOSAL_PWM_TRIGGER_SRC_PWM2 PWM_TRIGGER_SRC_PWM2
#define HOSAL_PWM_TRIGGER_SRC_PWM3 PWM_TRIGGER_SRC_PWM3
#define HOSAL_PWM_TRIGGER_SRC_PWM4 PWM_TRIGGER_SRC_PWM4
#define HOSAL_PWM_TRIGGER_SRC_SELF PWM_TRIGGER_SRC_SELF


#define HOSAL_PWM_DMA_SMP_FMT_0 PWM_DMA_SMP_FMT_0
#define HOSAL_PWM_DMA_SMP_FMT_1 PWM_DMA_SMP_FMT_1


/**
 * \brief           PWM counter mode table.
 *                  UP: Up mode / UD: Up-Down mode
 */
#define HOSAL_PWM_COUNTER_MODE_UP PWM_COUNTER_MODE_UP
#define HOSAL_PWM_COUNTER_MODE_UD PWM_COUNTER_MODE_UD

/**
 * \brief           pwm output wave invert
 */
#define HOSAL_PWM_WAVE_INVERT_DISABLE PWM_WAVE_INVERT_DISABLE
#define HOSAL_PWM_WAVE_INVERT_ENABLE  PWM_WAVE_INVERT_ENABLE

/**
 * \brief           hosal pwm config struce defined
 */
typedef struct {
    pwm_config_t config; /*!< pwm config struct*/
} hosal_pwm_dev_t;

/**
 * \brief           pwm pin config function.
 * \param[in]      id            pwm identifier
 * \param[in]      pin_number    pwm pin number.
 * \return           function status
 */
int hosal_pwm_pin_conifg(uint32_t id, uint32_t pin_number);

/**
 * \brief           pwm format1 initinalfunction.
 * \param[in]      hosal_pwm_dev_t   pwm device config struct
 * \return             function status
 */
int hosal_pwm_init_fmt1(hosal_pwm_dev_t* dev);

/**
 * \brief           pwm format0 initinalfunction.
 * \param[in]       hosal_pwm_dev_t   pwm device config struct
 * \return            function status
 */
int hosal_pwm_init_fmt0(hosal_pwm_dev_t* dev);

/**
 * \brief           pwm format1 duty function.
 * \param[in]    id   pwm identifier
 * \param[in]    duty   pwm duty value (0~100)
 * \return           function status
 */
int hosal_pwm_fmt1_duty(uint32_t id, uint8_t duty);

/**
 * \brief           pwm format0 duty function.
 * \param[in]    id   pwm identifier
 * \param[in]    duty   pwm duty value (0~100)
 * \return           function status
 */
int hosal_pwm_fmt0_duty(uint32_t id, uint8_t duty);

/**
 * \brief           pwm format1 count function.
 * \param[in]    id                pwm identifier
 * \param[in]    count             pwm count value
 * \return           function status
 */
int hosal_pwm_fmt1_count(uint32_t id, uint32_t count);

/**
 * \brief           pwm format0 duty function.
 * \param[in]    id                pwm identifier
 * \param[in]    count             pwm count value
 * \return           function status
 */
int hosal_pwm_fmt0_count(uint32_t id, uint32_t count);

/**
 * \brief           pwm multi element intinal function.
 * \param[in]    hosal_pwm_dev_t   pwm device config struct
 * \return           function status
 */
int hosal_pwm_multi_init(hosal_pwm_dev_t* dev);

/**
 * \brief           pwm format1 duty function.
 * \param[in]    id                 pwm identifier
 * \param[in]    hosal_pwm_dev_t    pwm device struct
 * \param[in]    element            pwm element value
 * \param[in]    duty               pwm  value
 * \return           function status
 */
int hosal_pwm_multi_fmt1_duty(uint32_t id, hosal_pwm_dev_t* dev,
                                 uint32_t element, uint8_t duty);

/**
 * \brief           pwm format0 duty function.
 * \param[in]    id                  pwm identifier
 * \param[in]    hosal_pwm_dev_t     pwm device struct
 * \param[in]    element             pwm element value
 * \param[in]    thd1_duty           pwm  thread 1 duty value
 * \param[in]    thd2_duty           pwm  thread 2 duty value
 * \return              function status
 */
int hosal_pwm_multi_fmt0_duty(uint32_t id, hosal_pwm_dev_t* dev,
                                 uint32_t element, uint8_t thd1_duty,
                                 uint8_t thd2_duty);

/**
 * \brief           pwm format0 count function.
 * \param[in]    id                 pwm identifier
 * \param[in]    hosal_pwm_dev_t    pwm device struct
 * \param[in]    element            pwm element value
 * \param[in]    count              pwm  count value
 * \return            function status 
 */
int hosal_pwm_multi_fmt1_count(uint32_t id, hosal_pwm_dev_t* dev,
                                  uint32_t element, uint32_t count);

/**
 * \brief           pwm format0 count function.
 * \param[in]    id                 pwm identifier
 * \param[in]    hosal_pwm_dev_t    pwm device struct
 * \param[in]    element            pwm element value
 * \param[in]    count              pwm  count value
 * \return             function status
 */
int hosal_pwm_multi_fmt0_count(uint32_t id, hosal_pwm_dev_t* dev,
                                  uint32_t element, uint32_t thd1_Count,
                                  uint32_t thd2_count);

/**
 * \brief           pwm stop function.
 * \param[in]       id pwm identifier
 * \return            function status
 */
int hosal_pwm_stop(uint32_t id);

/**
 * \brief           pwm start function.
 * \param[in]       id pwm identifier
 * \return            function status
 */
int hosal_pwm_start(uint32_t id);

/**
 * \brief           pwm paramater function.
 * \param[in]       id   pwm identifier
 * \param[in]       ctl  pwm control command
 * \param[in]       p_arg pwm paramater
 * \return          function status
 */
int hosal_pwm_ioctl(hosal_pwm_dev_t* dev, int ctl, void* p_arg);

/*@}*/ /* end of RT58X_HOSAL HOSAL_PWM */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_PWM_H */
