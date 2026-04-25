/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 */

/**
 * \file            hosal_flash.c
 * \brief           HOSAL Flash driver (thin wrapper over flashctl)
 */

#include <stdint.h>
#include <stdbool.h>
#include "mcu.h"
#include "flashctl.h"
#include "hosal_flash.h"
#include "hosal_status.h"



int hosal_flash_enable_qe(void)
{
    flash_enable_qe();
    return STATUS_SUCCESS;
}

int hosal_flash_init(void)
{
    return STATUS_SUCCESS;
}

int hosal_flash_read(int ctl, uint32_t address, uint8_t *buf)
{
    uint32_t rc;
    
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    switch (ctl) {
    case HOSAL_FLASH_READ_BYTE:
        if (!buf) return STATUS_INVALID_PARAM;
        buf[0] = flash_read_byte(address);
        return STATUS_SUCCESS;

    case HOSAL_FLASH_READ_PAGE:
        if (!buf) return STATUS_INVALID_PARAM;
        rc = flash_read_page_syncmode((uint32_t)buf, address);
        return (int)rc;

    case HOSAL_FLASH_SECURITY_READ:
        if (!buf) return STATUS_INVALID_PARAM;
        rc = flash_read_sec_register((uint32_t)buf, address);
        return (int)rc;

    default:
        return HOSAL_STATUS_INVALID_REQUEST;
    }
}

int hosal_flash_write(int ctl, uint32_t address, uint8_t *buf)
{
    uint32_t rc;
    
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    switch (ctl) {
    case HOSAL_FLASH_WRITE_BYTE:
        if (!buf) return STATUS_INVALID_PARAM;
        rc = flash_write_byte(address, buf[0]);
        return (int)rc;

    case HOSAL_FLASH_WRITE_PAGE:
        if (!buf) return STATUS_INVALID_PARAM;
        rc = flash_write_page((uint32_t)buf, address);
        return (int)rc;

    case HOSAL_FLASH_SECURITY_WRITE:
        if (!buf) return STATUS_INVALID_PARAM;
        rc = flash_write_sec_register((uint32_t)buf, address);
        return (int)rc;

    default:
        return HOSAL_STATUS_INVALID_REQUEST;
    }
}

int hosal_flash_write_n_bytes(uint32_t address, uint8_t *buf, uint32_t len)
{
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }
    if (!buf || len == 0u) {
        return STATUS_INVALID_PARAM;
    }

    uint32_t rc = flash_write_n_bytes(address, (uint32_t)buf, len);
    return (int)rc;
}

int hosal_flash_read_n_bytes(uint32_t address, uint8_t *buf, uint32_t len)
{
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }
    if (!buf || len == 0u) {
        return STATUS_INVALID_PARAM;
    }

    uint32_t rc = flash_read_n_bytes(address, (uint32_t)buf, len);
    return (int)rc;
}

int hosal_flash_erase(int ctl, uint32_t address)
{
    uint32_t rc;
    flash_erase_mode_t mode;
    
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    switch (ctl) {
    case HOSAL_FLASH_ERASE_PAGE:
        mode = FLASH_ERASE_PAGE;
        break;
    case HOSAL_FLASH_ERASE_SECTOR:
        mode = FLASH_ERASE_SECTOR;
        break;
    case HOSAL_FLASH_ERASE_32K_SECTOR:
        mode = FLASH_ERASE_32K;
        break;
    case HOSAL_FLASH_ERASE_64K_SECTOR:
        mode = FLASH_ERASE_64K;
        break;
    case HOSAL_FLASH_ERASE_SECURITY_PAGE:
        mode = FLASH_ERASE_SECURE;
        break;
    case HOSAL_FLASH_ERASE_CHIP:
    default:
        return HOSAL_STATUS_INVALID_REQUEST;
    }

    rc = flash_erase(mode, address);
    return (int)rc;
}

int hosal_flash_ioctrl(int ctl, void *p_arg)
{
    uint32_t rc;
    
    switch (ctl) {
    case HOSAL_FLASH_ENABLE_SUSPEND:
        flash_enable_suspend();
        return STATUS_SUCCESS;

    case HOSAL_FLASH_DISABLE_SUSPEND:
        flash_disable_suspend();
        return STATUS_SUCCESS;

    case HOSAL_FLASH_CACHE:
        flush_cache();
        return STATUS_SUCCESS;

    case HOSAL_FLASH_BUSY:
        return (int)flash_check_busy();

    case HOSAL_FLASH_GET_INFO:
        if (!p_arg) return STATUS_INVALID_PARAM;
        *(hosal_flash_size_t *)p_arg = (hosal_flash_size_t)flash_size();
        return STATUS_SUCCESS;

    default:
        return HOSAL_STATUS_UNSUPPORTED;
    }
}

int hosal_flash_read_id(uint32_t length, uint8_t *buf)
{
    if (!buf || length == 0u) return STATUS_INVALID_PARAM;
    uint32_t rc = flash_get_unique_id((uint32_t)buf, length);
    return (int)rc;

}

