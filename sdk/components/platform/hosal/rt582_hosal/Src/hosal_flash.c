/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_flash.c
 * \brief           Hosal Flash driver file
 */

/*
 * This file is part of library_name.
 * Author:         ives.lee 
 */
#include "stdio.h"
#include <stdint.h>
#include "mcu.h"
#include "flashctl.h"
#include "hosal_flash.h"
#include "hosal_status.h"



int hosal_flash_enable_qe(void) {
    flash_enable_qe();
    return HOSAL_STATUS_SUCCESS;
}

int hosal_flash_init(void) {
    return HOSAL_STATUS_SUCCESS;
}

int hosal_flash_read(int ctl, uint32_t address, uint8_t* buf) {

    uint32_t  status = 0, buf_address;

    if (flash_check_busy()) {

        return HOSAL_STATUS_EBUSY;
    }

    switch (ctl) {
        case HOSAL_FLASH_READ_BYTE:
            buf[0] = flash_read_byte(address);
            return HOSAL_STATUS_SUCCESS;
        case HOSAL_FLASH_READ_PAGE:
            return flash_read_page_syncmode((uint32_t)buf, address);
        case HOSAL_FLASH_SECURITY_READ:
            return flash_read_sec_register((uint32_t)buf, address); 
        default:
            return HOSAL_STATUS_INVALID_REQUEST;
    }
}

int hosal_flash_read_n_bytes(uint32_t address,uint8_t* buf,uint32_t len) {

    uint32_t  status = HOSAL_STATUS_SUCCESS;

    if (flash_check_busy()) {

        return HOSAL_STATUS_EBUSY;
    }

    
    status = flash_read_n_bytes(address,(uint32_t)buf,len);

    return status;
}

int hosal_flash_write(int ctl, uint32_t address, uint8_t* buf) {

    uint32_t  status = HOSAL_STATUS_SUCCESS;

    if (flash_check_busy()) {
        
        return HOSAL_STATUS_EBUSY;
    }

    switch (ctl) {
        case HOSAL_FLASH_WRITE_BYTE:
            return flash_write_byte(address, buf[0]);
        case HOSAL_FLASH_WRITE_PAGE:
            return flash_write_page((uint32_t)buf, address);
        case HOSAL_FLASH_SECURITY_WRITE:
            return flash_write_sec_register(address, (uint32_t)buf);
        default:
            return HOSAL_STATUS_INVALID_REQUEST;
    }
}

int hosal_flash_write_n_bytes(uint32_t address,uint8_t* buf,uint32_t len) {

    uint32_t  status = HOSAL_STATUS_SUCCESS;

    if (flash_check_busy()) {

        return HOSAL_STATUS_EBUSY;
    }


    return flash_write_n_bytes(address, (uint32_t)buf, len);
}



int hosal_flash_erase(int ctl, uint32_t address) {
    
     uint32_t len = 0,ret=0;

    if (flash_check_busy()) {

        return HOSAL_STATUS_EBUSY;
    }


    switch (ctl) {
        case HOSAL_FLASH_ERASE_PAGE: //Only Support 512k flash
            return flash_erase(FLASH_ERASE_PAGE, address);
        case HOSAL_FLASH_ERASE_SECTOR:
            return flash_erase(FLASH_ERASE_SECTOR, address);
        case HOSAL_FLASH_ERASE_32K_SECTOR:
            return flash_erase(FLASH_ERASE_32K, address);
        case HOSAL_FLASH_ERASE_64K_SECTOR:
            return flash_erase(FLASH_ERASE_64K, address);
        case HOSAL_FLASH_ERASE_CHIP:
            return HOSAL_STATUS_INVALID_REQUEST;
        default:
            return HOSAL_STATUS_INVALID_REQUEST;
    }
}


int hosal_flash_ioctrl(int ctl, void *p_arg) {

    uint32_t  status = HOSAL_STATUS_SUCCESS;

    switch (ctl) {

    case HOSAL_FLASH_ENABLE_SUSPEND:

        flash_enable_suspend();

        status = HOSAL_STATUS_SUCCESS;
        break;

    case HOSAL_FLASH_DISABLE_SUSPEND:

        flash_disable_suspend();

        status = HOSAL_STATUS_SUCCESS;
        break;

    case HOSAL_FLASH_CACHE:

        flush_cache();

        status = HOSAL_STATUS_SUCCESS;

        break;


    case HOSAL_FLASH_BUSY:
        
        return flash_check_busy();

        break;

    case HOSAL_FLASH_GET_INFO:

        *(hosal_flash_size_t*)p_arg = flash_get_deviceinfo();

        return HOSAL_STATUS_SUCCESS;
        break;
    default :
        HOSAL_STATUS_INVALID_REQUEST;

    }

    return (int)status;
}


int hosal_flash_read_id(uint32_t length, uint8_t* buf) {


    flash_get_unique_id((uint32_t)buf, length);    

    return HOSAL_STATUS_SUCCESS;
 
}
