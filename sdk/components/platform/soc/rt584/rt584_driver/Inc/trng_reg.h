/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           trng_reg.h
 * \brief          true random number generator register definition header file
 */
/*
 * This file is part of library_name.
 * Author:
 */


#ifndef TRNG_REG_H
#define TRNG_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \brief           Irq priority definitions.
 */
typedef struct trng_ctrl_struct {

    __IO uint32_t trng0;                        /*!< offset:0x00 */
    __IO uint32_t trng1;                        /*!< offset:0x04 */
    __I  uint32_t trng2;                        /*!< offset:0x08 */
    __IO uint32_t trng3;                        /*!< offset:0x0C */

} trng_t;
/**
 * \brief           Irq priority definitions.
 */
#define  TRNG_ENABLE          (1UL << 0)
#define  TRNG_INTR_CLEAR      (1UL << 1)

#define  TRNG_SEL             (1UL << 0)
#define  TRNG_INTR_ENABLE     (1UL << 1)

#define  TRNG_BUSY            (1UL << 0)
#define  TRNG_INTR_STATUS     (1UL << 1)

#ifdef __cplusplus
}
#endif

#endif /* End of TRNG_REG_H */
