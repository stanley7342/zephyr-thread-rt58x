/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 */

/**
 * \file            hosal_flash.h
 * \brief           hosal_flash include file
 */

/*
 * This file is part of library_name.
 * Author:  ives.lee 
 */
#ifndef HOSAL_FLASH_H
#define HOSAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "flashctl.h"

/* Flash read command */
#define HOSAL_FLASH_READ_BYTE                0x00
#define HOSAL_FLASH_READ_PAGE                0x01
#define HOSAL_FLASH_SECURITY_READ            0x02
#define HOSAL_FLASH_READ_ID                  0x04

/* Flash write command */
#define HOSAL_FLASH_WRITE_BYTE               0x10
#define HOSAL_FLASH_WRITE_PAGE               0x11
#define HOSAL_FLASH_SECURITY_WRITE           0x12

/* Flash erase command */
#define HOSAL_FLASH_ERASE_PAGE               0x20
#define HOSAL_FLASH_ERASE_SECTOR             0x21
#define HOSAL_FLASH_ERASE_32K_SECTOR         0x22
#define HOSAL_FLASH_ERASE_64K_SECTOR         0x23
#define HOSAL_FLASH_ERASE_CHIP               0x24
#define HOSAL_FLASH_ERASE_SECURITY_PAGE      0x25

/* Flash io control command */
#define HOSAL_FLASH_ENABLE_SUSPEND           0x30
#define HOSAL_FLASH_DISABLE_SUSPEND          0x31
#define HOSAL_FLASH_CACHE                    0x32
#define HOSAL_FLASH_BUSY                     0x33
#define HOSAL_FLASH_GET_INFO                 0x34
#define HOSAL_FLASH_ENABLE_VERIFY            0x35  /* Enable write verification    */
#define HOSAL_FLASH_DISABLE_VERIFY           0x36  /* Disable write verification   */
#define HOSAL_FLASH_LVD_ENABLE               0x37  /* low voltage detect           */
#define HOSAL_FLASH_GET_PROTECTION_INFO      0x40  /* Get protection configuration */
#define HOSAL_FLASH_BP_REMOVE_ALL            0x41  
#define HOSAL_FLASH_SET_ALLOWED_REGION       0x42  
#define HOSAL_FLASH_CLEAR_ALLOWED_REGION     0x43  
#define HOSAL_FLASH_GET_PROTECTION_CONFIG    0x44  
#define HOSAL_FLASH_GET_STATUS_SR12          0x45
/* Flash size definitions */
typedef enum {
    HOSAL_FLASH_NOT_SUPPORT = 0x00,
    HOSAL_FLASH_512K  = 0x13,
    HOSAL_FLASH_1024K = 0x14,
    HOSAL_FLASH_2048K = 0x15,
    HOSAL_FLASH_4096K = 0x16,
} hosal_flash_size_t;

typedef struct {
    uint32_t start;
    uint32_t end;
} hosal_flash_allowed_region_t;

/* ?Y?A?Q?? SR1/SR2 ?]?? HOSAL */
typedef struct {
    uint8_t sr1;
    uint8_t sr2;
} hosal_flash_sr12_t;

/**
 * \brief           enable flash
 * \param[in]       NONE
 * \return           function status
 */
int hosal_flash_enable_qe(void);

/**
 * \brief           hosal_flash_init
 * \param[in]       NONE
 * \return          function status
 */
int hosal_flash_init(void);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_read(int ctl, uint32_t address, uint8_t *buf);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_write(int ctl, uint32_t address, uint8_t *buf);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \return          function status
 */
int hosal_flash_erase(int ctl, uint32_t address);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_ioctrl(int ctl, void *p_arg);

/**
 * \brief           flash io control function
 * \param[in]       length: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_read_id(uint32_t length, uint8_t* buf);
/**
 * \brief Write data to flash memory.
 *
 * Writes a continuous block of data to flash starting from the given address.
 * The flash area must be erased before writing.
 *
 * \param[in] address  Flash start address.
 * \param[in] buf      Pointer to data buffer.
 * \param[in] len      Number of bytes to write.
 *
 * \retval STATUS_SUCCESS         Write successful.
 * \retval STATUS_INVALID_PARAM   Invalid parameter.
 * \retval STATUS_EBUSY           Flash is busy.
 */
int hosal_flash_write_n_bytes(uint32_t address,uint8_t* buf,uint32_t len);

/**
 * \brief read data frm flash memory.
 *
 *
 * \param[in] address  Flash start address.
 * \param[in] buf      Pointer to data buffer.
 * \param[in] len      Number of bytes to read.
 *
 * \retval STATUS_SUCCESS         Read successful.
 * \retval STATUS_INVALID_PARAM   Invalid parameter.
 * \retval STATUS_EBUSY           Flash is busy.
 */
int hosal_flash_read_n_bytes(uint32_t address,uint8_t* buf,uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* HOSAL_FLASH_H */