/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            cachectl_reg.h
 * \brief           cachectl_reg.h include file
 */

/*
 * This file is part of library_name.
 * Author:     
 */
#ifndef CACHECTL_REG_H
#define CACHECTL_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef union cache_ccr_s {
    struct cache_ccr_b {
        uint32_t CACHE_EN        : 1;
        uint32_t CACHE_WAY_1_EN  : 1;
        uint32_t RESERVED2       : 6;
        uint32_t CACHE_WAY_0_CLR : 1;
        uint32_t CACHE_WAY_1_CLR : 1;
        uint32_t RESERVED10      : 22;
    } bit;

    uint32_t reg;
} cache_ccr_t;

typedef struct {
    __IO cache_ccr_t CCR; /*0x00*/
} cachectl_t;


#ifdef __cplusplus
}
#endif

#endif /* End of CACHECTL_REG_H */
