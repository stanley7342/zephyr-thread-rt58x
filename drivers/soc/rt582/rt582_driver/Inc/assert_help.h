/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            assert_help.h
 * \brief           assert_help include file
 */

/*
 * This file is part of library_name.
 * Author:     
 */

#ifndef ASSERT_H
#define ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef DEBUG

#include <stdint.h>
#include "stdio.h"

#define dprintf(msg, ...)                                                      \
    do {                                                                       \
        printf(msg, __VA_ARGS__);                                              \
    } while (0)
#define dprintf0(msg)                                                          \
    do {                                                                       \
        printf(msg);                                                           \
    } while (0)

#define assert_param(expr)                                                     \
    do {                                                                       \
        if (expr) {                                                            \
        } else {                                                               \
            printf("ASSERT Failed in File %s line %d \n", __FILE__, __LINE__); \
            while (1)                                                          \
                ;                                                              \
        }                                                                      \
    } while (0)

#else
#define assert_param(expr) ((void)0)
#define dprintf(msg, ...)  ((void)0)
#define dprintf0(msg)      ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* End of ASSERT_H */
