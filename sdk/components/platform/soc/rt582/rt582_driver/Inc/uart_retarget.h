/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/** @file uart_retarget.h
 * @license
 * @description
 */

#ifndef UART_RETARGET_H
#define UART_RETARGET_H

#ifdef __cplusplus
extern "C" {
#endif
#include "project_config.h"
#include "stdint.h"


/**
 * \defgroup UART_RETARGET Uart retarget
 * \ingroup RT58X_DRIVER
 * \brief  Define Uart retarget definitions, structures, and functions
 * @{
 */


/**
 * \brief           uart retarget callback function typedef define
 */
typedef int (*cb_stdin_str_t)(char* pBuf, int length);

/**
 * \brief           the description of uart retarget
 */
typedef struct retarget_desc {
    uint32_t uart_id;

    // If you need to retarget getc or read, must assigne the callback
    cb_stdin_str_t pf_stdin_str;
} uart_retarget_desc_t;

/**
 *  \brief           initialize uart retarget for printf
 *
 *  \param [in]       uart_retarget_desc_t     assign the user description of retarget
 *  \return           0: ok, other: fail
 */
int uart_retarget_init(uart_retarget_desc_t* pt_retarget_desc);

/**
 *  \brief           stdout for bsp stdout
 *
 *  \param [in]      ch output ch
 *  \return         0: ok, other: fail
 */
int uart_retarget_stdout_char(int ch);

/**
 *  \brief           stdout string for bsp stdout
 *
 *  \param [in]      str char point
 *  \param [in]      length output data length
 *  \return         0: ok, other: fail
 */
int uart_retarget_stdout_string(char* str, int length);

/*@}*/ /* end of RT58X_DRIVER UART_RETARGET */

#ifdef __cplusplus
}
#endif

#endif /* End of UART_RETARGET_H */
