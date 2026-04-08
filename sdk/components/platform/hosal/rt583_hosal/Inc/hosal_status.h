/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_status.h
 * \brief           Hosal status header file
 */
/*
 * This file is part of library_name.
 * Author:
 */

#ifndef HOSAL_STATUS_H
#define HOSAL_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \defgroup HOSAL_STATUS Hosal status
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal status definitions, structures, and functions
 * @{
 */


/**
 * \brief           Define return Status
 */
#define HOSAL_STATUS_SUCCESS         STATUS_SUCCESS
#define HOSAL_STATUS_INVALID_PARAM   STATUS_INVALID_PARAM
#define HOSAL_STATUS_INVALID_REQUEST STATUS_INVALID_REQUEST
#define HOSAL_STATUS_EBUSY           STATUS_EBUSY
#define HOSAL_STATUS_NO_INIT         STATUS_NO_INIT
#define HOSAL_STATUS_ERROR           STATUS_ERROR
#define HOSAL_STATUS_TIMEOUT         STATUS_TIMEOUT

/*@}*/ /* end of RT58X_HOSAL HOSAL_STATUS */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_STATUS_H */
