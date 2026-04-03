/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            status.h
 * \brief           Status header file
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
 * \defgroup STATUS Status
 * \ingroup RT58X_DRIVER
 * \brief  Define Status definitions
 * @{
 */

/**
 * \brief           Define return Status
 */
#define STATUS_SUCCESS         (0UL)            /*!< */
#define STATUS_INVALID_PARAM   (1UL)            /*!< Invalid Parameter. */
#define STATUS_INVALID_REQUEST (2UL)            /*!< Invalid Request. */
#define STATUS_EBUSY           (3UL)            /*!< Device is busy now. */
#define STATUS_NO_INIT         (4UL)            /*!< Device should be init first. */
#define STATUS_ERROR           (5UL)            /*!< ERROR */
#define STATUS_TIMEOUT         (6UL)            /*!< TIMEOUT */

/*@}*/ /* end of RT58X_DRIVER STATUS */

#ifdef __cplusplus
}
#endif

#endif /* End of STATUS_H */
