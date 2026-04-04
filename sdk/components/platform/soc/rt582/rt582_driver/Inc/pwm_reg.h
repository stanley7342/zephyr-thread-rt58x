/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            pwm_reg.h
 * \brief           pwm_reg.h include file
 */
/*
 * This file is part of library_name.
 * Author:
 */

#ifndef PWM_REG_H
#define PWM_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief           Pwm control 0 related define, at offet 0x00
 */
#define PWM_CFG0_PWM_ENA_SHFT 0
#define PWM_CFG0_PWM_ENA_MASK (0x01UL << PWM_CFG0_PWM_ENA_SHFT)
#define PWM_CFG0_CK_ENA_SHFT  1
#define PWM_CFG0_CK_ENA_MASK  (0x01UL << PWM_CFG0_CK_ENA_SHFT)

/**
 * \brief           Pwm control 1 related define, at offet 0x04
 */
#define PWM_CFG0_PWM_RST_SHFT 0
#define PWM_CFG0_PWM_RST_MASK (0x00000001UL << PWM_CFG0_PWM_RST_SHFT)
#define PWM_CFG0_CK_UPD_SHFT  8
#define PWM_CFG0_CK_UPD_MASK  (0x00000001UL << PWM_CFG0_CK_UPD_SHFT)

/**
 * \brief           Pwm setetting 0 related define, at offet 0x08
 */
#define PWM_CFG0_SEQ_ORDER_SHFT    0
#define PWM_CFG0_SEQ_ORDER_MASK    (0x00000003UL << PWM_CFG0_SEQ_ORDER_SHFT)
#define PWM_CFG0_SEQ_NUM_SEL_SHFT  1
#define PWM_CFG0_SEQ_NUM_SEL_MASK  (0x00000001UL << PWM_CFG0_SEQ_NUM_SEL_SHFT)
#define PWM_CFG0_SEQ_MODE_SHFT     2
#define PWM_CFG0_SEQ_MODE_MASK     (0x00000001UL << PWM_CFG0_SEQ_MODE_SHFT)
#define PWM_CFG0_PWM_DMA_FMT_SHFT  3
#define PWM_CFG0_PWM_DMA_FMT_MASK  (0x00000001UL << PWM_CFG0_PWM_DMA_FMT_SHFT)
#define PWM_CFG0_PWM_CNT_MODE_SHFT 4
#define PWM_CFG0_PWM_CNT_MODE_MASK (0x00000001UL << PWM_CFG0_PWM_CNT_MODE_SHFT)
#define PWM_CFG0_PWM_CNT_TRIG_SHFT 5
#define PWM_CFG0_PWM_CNT_TRIG_MASK (0x00000001UL << PWM_CFG0_PWM_CNT_TRIG_SHFT)
#define PWM_CFG0_SEQ_DMA_AUTO_SHFT 6
#define PWM_CFG0_SEQ_DMA_AUTO_MASK (0x00000001UL << PWM_CFG0_SEQ_DMA_AUTO_SHFT)
#define PWM_CFG0_CK_DIV_SHFT       8
#define PWM_CFG0_CK_DIV_MASK       (0x0000000FUL << PWM_CFG0_CK_DIV_SHFT)
#define PWM_CFG0_PWM_ENA_TRIG_SHFT 12
#define PWM_CFG0_PWM_ENA_TRIG_MASK (0x00000007UL << PWM_CFG0_PWM_ENA_TRIG_SHFT)

/**
 * \brief           Pwm setetting 1 related define, at offet 0x0C
 */
#define PWM_CFG0_PWM_CNT_END_SHFT 0
#define PWM_CFG0_PWM_CNT_END_MASK (0x00007FFFUL << PWM_CFG0_PWM_CNT_END_SHFT)

/**
 * \brief           Pwm setetting 2 related define, at offet 0x10
 */
#define PWM_CFG0_SEQX_PCNT_SHFT 0
#define PWM_CFG0_SEQX_PCNT_MASK (0x0000FFFFUL << PWM_CFG0_SEQX_PCNT_SHFT)

/**
 * \brief           Pwm setetting 3 related define, at offet 0x14
 */
#define PWM_CFG0_SEQ0_NUM_SHFT 0
#define PWM_CFG0_SEQ0_NUM_MASK (0x0000FFFFUL << PWM_CFG0_SEQ0_NUM_SHFT)

/**
 * \brief           Pwm setetting 4 related define, at offet 0x18
 */
#define PWM_CFG0_SEQ0_RPT_SHFT 0
#define PWM_CFG0_SEQ0_RPT_MASK (0x00FFFFFFUL << PWM_CFG0_SEQ0_RPT_SHFT)

/**
 * \brief           Pwm setetting 5 related define, at offet 0x1C
 */
#define PWM_CFG0_SEQ0_DLY_SHFT 0
#define PWM_CFG0_SEQ0_DLY_MASK (0x00FFFFFFUL << PWM_CFG0_SEQ0_DLY_SHFT)

/**
 * \brief           Pwm setetting 5 related define, at offet 0x20
 */
#define PWM_CFG0_SEQ1_NUM_SHFT 0
#define PWM_CFG0_SEQ1_NUM_MASK (0x0000FFFFUL << PWM_CFG0_SEQ1_NUM_SHFT)

/**
 * \brief           Pwm setetting 6 related define, at offet 0x24
 */
#define PWM_CFG0_SEQ1_RPT_SHFT 0
#define PWM_CFG0_SEQ1_RPT_MASK (0x00FFFFFFUL << PWM_CFG0_SEQ1_RPT_SHFT)

/**
 * \brief           Pwm setetting 7 related define, at offet 0x28
 */
#define PWM_CFG0_SEQ1_DLY_SHFT 0
#define PWM_CFG0_SEQ1_DLY_MASK (0x00FFFFFFUL << PWM_CFG0_SEQ1_DLY_SHFT)

/**
 * \brief           Pwm setetting 8 related define, at offet 0x2C
 */
#define PWM_CFG0_PWM_RDMA0_CTL0_SHFT 0
#define PWM_CFG0_PWM_RDMA0_CTL0_MASK                                           \
    (0x00000001UL << PWM_CFG0_PWM_RDMA0_CTL0_SHFT)

/**
 * \brief           Pwm rdma0 control 1 related define, at offet 0x44
 */
#define PWM_CFG0_PWM_RDMA0_CTL1_SHFT 0
#define PWM_CFG0_PWM_RDMA0_CTL1_MASK                                           \
    (0x00000001UL << PWM_CFG0_PWM_RDMA0_CTL1_SHFT)

/**
 * \brief           Pwm rdma0 set 0 related define, at offet 0x48
 */
#define PWM_CFG0_PWM_RDMA0_SEG_SHFT 0
#define PWM_CFG0_PWM_RDMA0_SEG_MASK                                            \
    (0x0000FFFFUL << PWM_CFG0_PWM_RDMA0_SEG_SHFT)
#define PWM_CFG0_PWM_RDMA0_BLK_SHFT 16
#define PWM_CFG0_PWM_RDMA0_BLK_MASK                                            \
    (0x0000FFFFUL << PWM_CFG0_PWM_RDMA0_BLK_SHFT)

/**
 * \brief           Pwm rdma0 set 1 related define, at offet 0x4C
 */
#define PWM_CFG0_PWM_RDMA0_SET1_SHFT 0
#define PWM_CFG0_PWM_RDMA0_SET1_MASK                                           \
    (0xFFFFFFFFUL << PWM_CFG0_PWM_RDMA0_SET1_SHFT)

/**
 * \brief           Pwm rdma0 r0 related define, at offet 0x58
 */
#define PWM_CFG0_PWM_RDMA0_R0_SHFT 0
#define PWM_CFG0_PWM_RDMA0_R0_MASK (0xFFFFFFFFUL << PWM_CFG0_PWM_RDMA0_R0_SHFT)

/**
 * \brief           Pwm rdma0 r1 related define, at offet 0x5C
 */
#define PWM_CFG0_PWM_RDMA0_R1_SHFT 0
#define PWM_CFG0_PWM_RDMA0_R1_MASK (0xFFFFFFFFUL << PWM_CFG0_PWM_RDMA0_R1_SHFT)

/**
 * \brief           Pwm rdma1 control 0 related define, at offet 0x60
 */
#define PWM_CFG0_PWM_RDMA1_CTL0_SHFT 0
#define PWM_CFG0_PWM_RDMA1_CTL0_MASK                                           \
    (0x0000000FUL << PWM_CFG0_PWM_RDMA1_CTL0_SHFT)

/**
 * \brief           Pwm rdma1 control 1 related define, at offet 0x64
 */
#define PWM_CFG0_PWM_RDMA1_CTL1_SHFT 0
#define PWM_CFG0_PWM_RDMA1_CTL1_MASK                                           \
    (0x00000001UL << PWM_CFG0_PWM_RDMA1_CTL1_SHFT)

/**
 * \brief           Pwm rdma1 set 0 related define, at offet 0x68
 */
#define PWM_CFG0_PWM_RDMA1_SEG_SHFT 0
#define PWM_CFG0_PWM_RDMA1_SEG_MASK                                            \
    (0x0000FFFFUL << PWM_CFG0_PWM_RDMA1_SEG_SHFT)
#define PWM_CFG0_PWM_RDMA1_BLK_SHFT 16
#define PWM_CFG0_PWM_RDMA1_BLK_MASK                                            \
    (0x0000FFFFUL << PWM_CFG0_PWM_RDMA1_BLK_SHFT)

// offset: 0x6C[31:0]
/**
 * \brief           Pwm rdma1 set 1 related define, at offet 0x6C
 */
#define PWM_CFG0_PWM_RDMA1_SET1_SHFT 0
#define PWM_CFG0_PWM_RDMA1_SET1_MASK                                           \
    (0xFFFFFFFFUL << PWM_CFG0_PWM_RDMA1_SET1_SHFT)

/**
 * \brief           Pwm rdma1 r0 related define, at offet 0x78
 */
#define PWM_CFG0_PWM_RDMA1_R0_SHFT 0
#define PWM_CFG0_PWM_RDMA1_R0_MASK (0xFFFFFFFFUL << PWM_CFG0_PWM_RDMA1_R0_SHFT)

/**
 * \brief           Pwm rdma1 r1 related define, at offet 0x7C
 */
#define PWM_CFG0_PWM_RDMA1_R1_SHFT 0
#define PWM_CFG0_PWM_RDMA1_R1_MASK (0xFFFFFFFFUL << PWM_CFG0_PWM_RDMA1_R1_SHFT)

/**
 * \brief           Pwm interrupt clear related define, at offet 0xA0
 */
#define PWM_RDMA0_INT_CLR_SHFT      0
#define PWM_RDMA0_INT_CLR_MASK      (0x01UL << PWM_RDMA0_INT_CLR_SHFT)
#define PWM_RDMA0_ERR_INT_CLR_SHFT  1
#define PWM_RDMA0_ERR_INT_CLR_MASK  (0x01UL << PWM_RDMA0_ERR_INT_CLR_SHFT)
#define PWM_RDMA1_INT_CLR_SHFT      2
#define PWM_RDMA1_INT_CLR_MASK      (0x01UL << PWM_RDMA1_INT_CLR_SHFT)
#define PWM_RDMA1_ERR_INT_CLR_SHFT  3
#define PWM_RDMA1_ERR_INT_CLR_MASK  (0x01UL << PWM_RDMA1_ERR_INT_CLR_SHFT)
#define PWM_RSEQ_DONE_INT_CLR_SHFT  4
#define PWM_RSEQ_DONE_INT_CLR_MASK  (0x01UL << PWM_RSEQ_DONE_INT_CLR_SHFT)
#define PWM_TSEQ_DONE_INT_CLR_SHFT  5
#define PWM_TSEQ_DONE_INT_CLR_MASK  (0x01UL << PWM_TSEQ_DONE_INT_CLR_MASK)
#define PWM_TRSEQ_DONE_INT_CLR_SHFT 6
#define PWM_TRSEQ_DONE_INT_CLR_MASK (0x01UL << PWM_TRSEQ_DONE_INT_CLR_SHFT)

/**
 * \brief           Pwm interrupt mask related define, at offet 0xA4
 */
#define PWM_RDMA0_INT_MASK_SHFT      0
#define PWM_RDMA0_INT_MASK_MASK      (0x01UL << PWM_RDMA0_INT_MASK_SHFT)
#define PWM_RDMA0_ERR_INT_MASK_SHFT  1
#define PWM_RDMA0_ERR_INT_MASK_MASK  (0x01UL << PWM_RDMA0_ERR_INT_MASK_SHFT)
#define PWM_RDMA1_INT_MASK_SHFT      2
#define PWM_RDMA1_INT_MASK_MASK      (0x01UL << PWM_RDMA1_INT_MASK_SHFT)
#define PWM_RDMA1_ERR_INT_MASK_SHFT  3
#define PWM_RDMA1_ERR_INT_MASK_MASK  (0x01UL << PWM_RDMA1_ERR_INT_MASK_SHFT)
#define PWM_RSEQ_DONE_INT_MASK_SHFT  4
#define PWM_RSEQ_DONE_INT_MASK_MASK  (0x01UL << PWM_RSEQ_DONE_INT_MASK_SHFT)
#define PWM_TSEQ_DONE_INT_MASK_SHFT  5
#define PWM_TSEQ_DONE_INT_MASK_MASK  (0x01UL << PWM_TSEQ_DONE_INT_MASK_SHFT)
#define PWM_TRSEQ_DONE_INT_MASK_SHFT 6
#define PWM_TRSEQ_DONE_INT_MASK_MASK (0x01UL << PWM_TRSEQ_DONE_INT_MASK_SHFT)

/**
 * \brief           Pwm interrupt status related define, at offet 0xA8
 */
#define PWM_RDMA0_STATUS_INT_SHFT      0
#define PWM_RDMA0_STATUS_INT_MASK      (0x01UL << PWM_RDMA0_STATUS_INT_SHFT)
#define PWM_RDMA0_STATUS_ERR_INT_SHFT  1
#define PWM_RDMA0_STATUS_ERR_INT_MASK  (0x01UL << PWM_RDMA0_STATUS_ERR_INT_SHFT)
#define PWM_RDMA1_STATUS_INT_SHFT      2
#define PWM_RDMA1_STATUS_INT_MASK      (0x01UL << PWM_RDMA1_STATUS_INT_SHFT)
#define PWM_RDMA1_STATUS_ERR_INT_SHFT  3
#define PWM_RDMA1_STATUS_ERR_INT_MASK  (0x01UL << PWM_RDMA1_STATUS_ERR_INT_SHFT)
#define PWM_RSEQ_DONE_STATUS_INT_SHFT  4
#define PWM_RSEQ_DONE_STATUS_INT_MASK  (0x01UL << PWM_RSEQ_DONE_STATUS_INT_SHFT)
#define PWM_TSEQ_DONE_STATUS_INT_SHFT  5
#define PWM_TSEQ_DONE_STATUS_INT_MASK  (0x01UL << PWM_TSEQ_DONE_STATUS_INT_SHFT)
#define PWM_TRSEQ_DONE_STATUS_INT_SHFT 6
#define PWM_TRSEQ_DONE_STATUS_INT_MASK (0x01UL << PWM_TRSEQ_DONE_STATUS_INT_SHFT)

typedef struct {
    __IO uint32_t pwm_ctl0;       //offset: 0x00
    __IO uint32_t pwm_ctl1;       //offset: 0x04
    __IO uint32_t pwm_set0;       //offset: 0x08
    __IO uint32_t pwm_set1;       //offset: 0x0C
    __IO uint32_t pwm_set2;       //offset: 0x10
    __IO uint32_t pwm_set3;       //offset: 0x14
    __IO uint32_t pwm_set4;       //offset: 0x18
    __IO uint32_t pwm_set5;       //offset: 0x1C
    __IO uint32_t pwm_set6;       //offset: 0x20
    __IO uint32_t pwm_set7;       //offset: 0x24
    __IO uint32_t pwm_set8;       //offset: 0x28
    __IO uint32_t pwm_rsvd_0;     //offset: 0x2C
    __IO uint32_t pwm_rsvd_1;     //offset: 0x30
    __IO uint32_t pwm_rsvd_2;     //offset: 0x34
    __IO uint32_t pwm_rsvd_3;     //offset: 0x38
    __IO uint32_t pwm_rsvd_4;     //offset: 0x3C
    __IO uint32_t pwm_rdma0_ctl0; //offset: 0x40
    __IO uint32_t pwm_rdma0_ctl1; //offset: 0x44
    __IO uint32_t pwm_rdma0_set0; //offset: 0x48
    __IO uint32_t pwm_rdma0_set1; //offset: 0x4C
    __IO uint32_t pwm_rsvd_5;     //offset: 0x50
    __IO uint32_t pwm_rsvd_6;     //offset: 0x54
    __I uint32_t pwm_rdma0_r0;    //offset: 0x58
    __I uint32_t pwm_rdma0_r1;    //offset: 0x5C
    __IO uint32_t pwm_rdma1_ctl0; //offset: 0x60
    __IO uint32_t pwm_rdma1_ctl1; //offset: 0x64
    __IO uint32_t pwm_rdma1_set0; //offset: 0x68
    __IO uint32_t pwm_rdma1_set1; //offset: 0x6C
    __IO uint32_t pwm_rsvd_7;     //offset: 0x70
    __IO uint32_t pwm_rsvd_8;     //offset: 0x74
    __I uint32_t pwm_rdma1_r0;    //offset: 0x78
    __I uint32_t pwm_rdma1_r1;    //offset: 0x7C
    __IO uint32_t pwm_rsvd_9;     //offset: 0x80
    __IO uint32_t pwm_rsvd_10;    //offset: 0x84
    __IO uint32_t pwm_rsvd_11;    //offset: 0x88
    __IO uint32_t pwm_rsvd_12;    //offset: 0x8C
    __IO uint32_t pwm_rsvd_13;    //offset: 0x90
    __IO uint32_t pwm_rsvd_14;    //offset: 0x94
    __IO uint32_t pwm_rsvd_15;    //offset: 0x98
    __IO uint32_t pwm_rsvd_16;    //offset: 0x9C
    __IO uint32_t pwm_int_clear;  //offset: 0xA0
    __IO uint32_t pwm_int_mask;   //offset: 0xA4
    __I uint32_t pwm_int_status;  //offset: 0xA8

} pwm_t;

#ifdef __cplusplus
}
#endif

#endif /* End of PWM_REG_H */

