/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           status.h
 * \brief          status header file
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#ifndef STATUS_H
#define STATUS_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \defgroup        STATUS Status
 * \ingroup         RT584_DRIVER
 * \brief           Define Status definitions, structures, and functions
 * @{
 */

/**
 * \brief           Define return Status
 */
#define STATUS_SUCCESS            (0UL)         /*!< Success */
#define STATUS_INVALID_PARAM      (1UL)         /*!< Invalid Parameter. */
#define STATUS_INVALID_REQUEST    (2UL)         /*!< Invalid Request. */
#define STATUS_EBUSY              (3UL)         /*!< Device is busy now. */
#define STATUS_NO_INIT            (4UL)         /*!< Device should be init first. */
#define STATUS_ERROR              (5UL)         /*!< ERROR */
#define STATUS_TIMEOUT            (6UL)         /*!< TIMEOUT */
#define STATUS_LOW_VOLTAGE        (7UL)         /*!< STATUS_LOW_VOLTAGE */
#define STATUS_VERIFY_FAIL        (8UL)         /*!< VERIFY_FAIL */ 
#define STATUS_PROTECTED          (9UL)         /*!< PROTECTED */ 
#define STATUS_REGION_CONFLICT    (10UL)        /*!< CONFLICT */ 
#define STATUS_UNSUPPORTED        (11UL)        /*!< UNSUPPORTED */ 
/*@}*/ /* end of RT584_DRIVER STATUS */

#ifdef __cplusplus
}
#endif

#endif /* End of STATUS_H */
