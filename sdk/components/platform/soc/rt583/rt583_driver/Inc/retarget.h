/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            retarget.h
 * \brief           retarget retarget driver header file
 */
/*
 * This file is part of library_name.
 * Author:
 */
#ifndef RETARGET_H
#define RETARGET_H

#ifdef __cplusplus
extern "C" {
#endif


#include "uart_drv.h"

/**
 * \defgroup UART_RETARGET Uart retarget
 * \ingroup RT58X_DRIVER
 * \brief  Define Uart retarget definitions, structures, and functions
 * @{
 */

/**
 *  \brief Initialize debug console
 *  This function configures and enable the debug console (UART0)
 *  So user appplication can call some stdout function like, printf(...) or getchar()
 *  for uart0
 *  The uart0  will be configured as 8bit mode, NONE parity, and baudrate is set by
 *  the first parameter of this function.
 *  Please Note: baudrate is defined in uart_drv.h!
 *  (If you want to set the baudrate to 115200, baudrate should be use the define "UART_BAUDRATE_115200", not 115200)
 *  \param[in] baudrate       identifier for uart0 baudrate
 *  \return
 *  \retval   STATUS_SUCCESS on success, or  error.
*/
uint32_t console_drv_init(uart_baudrate_t baudrate);

/**
 *  \brief check uart0 rx buffer is empty or not
 *  Call this function to check if there is any data in rx buffer
 *  Please notice this function is designed to check uart0 RX buffer.
 *  If only reflects "the current data" in uart0 rx buffer when called this function.
 *  But if there are multiple tasks reading the RX buffer "at the same time", it is
 *  possible that when task to read RX buffer, there is empty in RX buffer.
 *  \param     NONE
 *  \retval   return the number character in uart0 rx buffer.
*/
uint32_t check_uart0_rx_buffer_data_num(void);

/**
 *  \brief terminate console output message if system want to enter sleep mode
 *  Call this function to terminate the output message if there are data in output queue.
 *  Any message in output queue will be clear.
 *  Please call this function before system to enter sleep mode or waiting all message outputed.
 *  \param      NONE
 *  \retval     NONE.
*/
void console_sleep(void);

/*@}*/ /* end of RT58X_DRIVER RETARGET */

#ifdef __cplusplus
}
#endif

#endif /* End of RETARGET_H */
