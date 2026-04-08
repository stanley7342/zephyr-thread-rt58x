/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            remap_reg.h
 * \brief           Remap register definition header file
 */
/*
 * This file is part of library_name.
 * Author:
 */
#ifndef REMAP_REG_H
#define REMAP_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
    __IO uint32_t RESERVED1;                    /*!< 0x00 */
    __IO uint32_t RESERVED2;                    /*!< 0x04 */
    __IO uint32_t RESERVED3;                    /*!< 0x08 */
    __IO uint32_t RESERVED4;                    /*!< 0x0C */
    __IO uint32_t sw_irq_set;                   /*!< 0x10 */
    __IO uint32_t sw_irq_clr;                   /*!< 0x14 */
    __IO uint32_t sw_irq_en_set;                /*!< 0x18 */
    __IO uint32_t sw_irq_en_clr;                /*!< 0x1C */
} remap_t;

#ifdef __cplusplus
}
#endif

#endif /* End of GPIO_H */
