/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           XDMA_reg.h
 * \brief          XDMA register definition header file
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#ifndef XDMA_REG_H
#define XDMA_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \brief          Xdma total register 
 */
typedef struct {
    __IO uint32_t xdma_ctl0;                    /*!< offset:0x00 */
    __O  uint32_t xdma_ctl1;                    /*!< offset:0x04 */
} xdma_t;

#define XDMA_ENABLE            (1UL<<0)
#define XDMA_RESET             (1UL<<0)

#ifdef __cplusplus
}
#endif

#endif /* End of XDMA_REG_H */
