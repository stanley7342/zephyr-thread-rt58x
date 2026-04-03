/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_commmon.h
 * \brief           hosal_commmon include file
 */

/*
 * This file is part of library_name.
 * Author:  
 */

#ifndef HOSAL_COMMON_H
#define HOSAL_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup HOSAL_COMMON Hosal common
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal common definitions, structures, and functions
 * @{
 */

#define HOSAL_SET_BIT(REG, BIT)   ((REG) |= (BIT))
#define HOSAL_CLEAR_BIT(REG, BIT) ((REG) &= ~(BIT))
#define HOSAL_READ_BIT(REG, BIT)  ((REG) & (BIT))
#define HOSAL_CLEAR_REG(REG)      ((REG) = (0x0))
#define HOSAL_WRITE_REG(REG, VAL) ((REG) = (VAL))
#define HOSAL_READ_REG(REG)       ((REG))
#define HOSAL_MODIFY_REG(REG, CLEARMASK, SETMASK)                              \
    HOSAL_WRITE_REG((REG),                                                     \
                    (((HOSAL_READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))
#define HOSAL_POSITION_VAL(VAL) (__CLZ(__RBIT(VAL)))

/*@}*/ /* end of RT58X_HOSAL HOSAL_COMMON */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_COMMON_H */
