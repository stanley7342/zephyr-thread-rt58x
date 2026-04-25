/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            flash control.c
 * \brief           flash control  driver 
 */
/*
 * This file is part of library_name.
 * Author: ives.lee
 */
#include <stdio.h>
#include "mcu.h"
#include "flashctl.h"
#include "status.h"
#include "sysctrl.h"


uint32_t flash_check_address(uint32_t flash_address, uint32_t length) {

    uint16_t flash_size_id;

    //get flash size id
    flash_size_id = ((flash_get_deviceinfo() >> FLASH_SIZE_ID_SHIFT) & 0xFF);

    if ((flash_address < BOOT_LOADER_END_PROTECT_ADDR)
        || ((flash_address + (length - 1)) >= FLASH_END_ADDR(flash_size_id))) {
        return STATUS_INVALID_PARAM;
    }

    return STATUS_SUCCESS;
}

uint32_t flash_get_deviceinfo(void) { return (FLASH->flash_info & 0x00FFFFFF); }

flash_size_t flash_size(void) {
    uint32_t flash_size_id;

    flash_size_id = ((flash_get_deviceinfo() >> FLASH_SIZE_ID_SHIFT) & 0xFF);

    if (flash_size_id == FLASH_512K) {
        return FLASH_512K;
    } else if (flash_size_id == FLASH_1024K) {
        return FLASH_1024K;
    } else if (flash_size_id == FLASH_2048K) {
        return FLASH_2048K;
    } else if (flash_size_id == FLASH_4096K) {
        return FLASH_4096K;
    } else {
        return FLASH_NOT_SUPPORT;
    }
}

uint32_t flash_read_page_ext(uint32_t buf_addr, uint32_t read_page_addr) {

    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    enter_critical_section();
    FLASH->command = CMD_READPAGE;
    FLASH->flash_addr = read_page_addr;
    FLASH->mem_addr = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
    leave_critical_section();

    while (flash_check_busy()) {}

    return STATUS_SUCCESS; /*remember to wait flash to finish read outside the caller*/
}

uint32_t flash_read_page(uint32_t buf_addr, uint32_t read_page_addr) {


    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    enter_critical_section();
    FLASH->command = CMD_READPAGE;
    FLASH->flash_addr = read_page_addr;
    FLASH->mem_addr = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
    leave_critical_section();

    while (flash_check_busy()) {}

    return STATUS_SUCCESS; /*remember to wait flash to finish read outside the caller*/
}

uint32_t flash_read_page_syncmode(uint32_t buf_addr, uint32_t read_page_addr) {


    if (flash_check_busy()) {
        return STATUS_EBUSY; /*flash busy.. please call this function again*/
    }

    enter_critical_section();
    FLASH->command = CMD_READPAGE;
    FLASH->flash_addr = read_page_addr;
    FLASH->mem_addr = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
    leave_critical_section();

    while (flash_check_busy()) {}

    return STATUS_SUCCESS; /*all data in register buffer now*/
}

uint8_t flash_read_byte(uint32_t read_addr) {

    /*this is not a good idea to block function here....*/
    if (flash_check_busy()) {
        return STATUS_EBUSY; /*flash busy.. please call this function again*/
    }

    enter_critical_section();
    FLASH->command = CMD_READBYTE;
    FLASH->flash_addr = read_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;

    leave_critical_section();
    
    while (flash_check_busy()) {}

    return FLASH->flash_data >> 8;
}

uint32_t flash_read_byte_check_addr(uint32_t* buf_addr, uint32_t read_addr) {


    if (flash_check_busy()) {
        return STATUS_EBUSY; /*flash busy.. please call this function again*/
    }

    enter_critical_section();

    FLASH->command =  CMD_READBYTE;
    FLASH->flash_addr = read_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
    leave_critical_section();

    while (flash_check_busy()) {;}

    *buf_addr = (FLASH->flash_data >> 8) & 0xFF;

    return STATUS_SUCCESS;
}

uint32_t flash_erase(flash_erase_mode_t mode, uint32_t flash_addr) {

    
    /* Calculate erase length based on mode */
    uint32_t erase_len = LENGTH_4KB;  /* default sector size */
    switch (mode) {
        case FLASH_ERASE_PAGE:   erase_len = LENGTH_PAGE; break;
        case FLASH_ERASE_SECTOR: erase_len = LENGTH_4KB; break;
        case FLASH_ERASE_32K:    erase_len = LENGTH_32KB; break;
        case FLASH_ERASE_64K:    erase_len = LENGTH_64KB; break;
        case FLASH_ERASE_SECURE: erase_len = LENGTH_PAGE; break;
        default: break;
    }


    if (mode > FLASH_ERASE_SECURE) {
        return STATUS_INVALID_PARAM;
    }

    /* For Safety reason, we don't implement
     * erase chip command here. */
    switch (mode) {
        case FLASH_ERASE_PAGE: {
            if ((flash_get_deviceinfo() & 0xFF) != PUYA_MANU_ID) {
                return STATUS_INVALID_PARAM; //invalid flash id
            }

            if (flash_check_address(flash_addr, LENGTH_PAGE)
                == STATUS_INVALID_PARAM) {
                return STATUS_INVALID_PARAM; //invalid addres range
            }

            FLASH->command = CMD_ERASEPAGE;
            break;
        }
        case FLASH_ERASE_SECTOR: {
            if (flash_check_address(flash_addr, LENGTH_4KB)
                == STATUS_INVALID_PARAM) {
                return STATUS_INVALID_PARAM; //invalid addres range
            }

            FLASH->command = CMD_ERASESECTOR;
            break;
        }
        case FLASH_ERASE_32K: {
            if (flash_check_address(flash_addr, LENGTH_32KB)
                == STATUS_INVALID_PARAM) {
                return STATUS_INVALID_PARAM; //invalid addres range
            }

            FLASH->command = CMD_ERASE_BL32K;
            break;
        }
        case FLASH_ERASE_64K: {
            if (flash_check_address(flash_addr, LENGTH_64KB)
                == STATUS_INVALID_PARAM) {
                return STATUS_INVALID_PARAM; //invalid addres range
            }

            FLASH->command = CMD_ERASE_BL64K;
            break;
        }
        case FLASH_ERASE_SECURE: {
            /*This is special command for erase secure register*/
            FLASH->command = CMD_ERASE_SEC_PAGE;
            break;
        }
        default: return STATUS_INVALID_PARAM;
    }

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }


    enter_critical_section();
    FLASH->flash_addr = flash_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
	
    while (flash_check_busy()) {}
	leave_critical_section();

    return STATUS_SUCCESS;
}

void flash_set_timing(flash_timing_mode_t* timing_cfg) {
    FLASH->dpd = timing_cfg->deep_pd_timing;
    FLASH->rdpd = timing_cfg->deep_rpd_timing;
    FLASH->suspend = timing_cfg->suspend_timing;
    FLASH->resume = timing_cfg->resume_timing;
    return;
}

 uint32_t flash_write_page(uint32_t buf_addr, uint32_t write_page_addr) {


    if (flash_check_address(write_page_addr, LENGTH_PAGE)
        == STATUS_INVALID_PARAM) {
        return STATUS_INVALID_PARAM; //invalid addres range
    }

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }


    enter_critical_section();
    
    FLASH->command = CMD_WRITEPAGE;
    FLASH->flash_addr = write_page_addr;
    FLASH->mem_addr = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;

    while (flash_check_busy()) {}
    
	leave_critical_section();

    return STATUS_SUCCESS;
}

uint32_t flash_write_n_bytes(uint32_t write_flash_addr, uint32_t data_buf_addr,uint32_t data_len) {
    

    uint32_t page_base;
    uint32_t offset;
    uint32_t bytes_in_page;
    uint32_t current_flash_addr;
    uint32_t current_buf_index = 0;
    uint32_t bytes_remaining;
    uint32_t full_pages;
    uint32_t i, j;

    uint8_t *buf_addr = (uint8_t *)data_buf_addr;

    /* page temp buffer: controller mem_addr 4-byte aligned */
    uint8_t temp_page_buffer[LENGTH_PAGE] __attribute__((aligned(4)));

    if (data_len == 0 || buf_addr == NULL) {
        return STATUS_INVALID_PARAM;
    }

    /* check address */
    if (flash_check_address(write_flash_addr, data_len) == STATUS_INVALID_PARAM) {
        return STATUS_INVALID_PARAM;
    }

    bytes_remaining = data_len;
    current_flash_addr = write_flash_addr;

    /* =========================================================
     * A) First：process non-aligment read page merge
     * ========================================================= */
    page_base = current_flash_addr & ~(LENGTH_PAGE - 1u);
    offset    = current_flash_addr - page_base;

    if (offset != 0u) {
        bytes_in_page = LENGTH_PAGE - offset;
        if (bytes_in_page > bytes_remaining) {
            bytes_in_page = bytes_remaining;
        }

        /* check write address */
        if (flash_check_address(page_base, LENGTH_PAGE) == STATUS_INVALID_PARAM) {
            return STATUS_INVALID_PARAM;
        }

        /* 1) read page */
        {
            volatile uint8_t *flash_ptr = (volatile uint8_t *)page_base;
            for (i = 0; i < LENGTH_PAGE; i++) {
                temp_page_buffer[i] = flash_ptr[i];
            }
        }

        /* 2) merge：overwirte offset bytes_in_page */
        for (i = 0; i < bytes_in_page; i++) {
            temp_page_buffer[offset + i] = buf_addr[current_buf_index + i];
        }
        

        /* 3) Page write */
        enter_critical_section();

        FLASH->command    = CMD_WRITEPAGE;
        FLASH->flash_addr = page_base;
        FLASH->mem_addr   = (uint32_t)temp_page_buffer;
        FLASH->pattern    = FLASH_UNLOCK_PATTER;
        FLASH->start      = STARTBIT;

        while (flash_check_busy()) {}

        leave_critical_section();

        current_buf_index  += bytes_in_page;
        bytes_remaining    -= bytes_in_page;
        current_flash_addr  = page_base + LENGTH_PAGE;

        if (bytes_remaining == 0u) {
            return STATUS_SUCCESS;
        }
    }

    /* =========================================================
     * B) page write
     * ========================================================= */
    full_pages = bytes_remaining / LENGTH_PAGE;

    for (i = 0; i < full_pages; i++) {

        if (flash_check_address(current_flash_addr, LENGTH_PAGE) == STATUS_INVALID_PARAM) {
            return STATUS_INVALID_PARAM;
        }

        uint32_t src_addr = (uint32_t)(&buf_addr[current_buf_index]);

        /*   */
        if (src_addr & 0x3u) {
            for (j = 0; j < LENGTH_PAGE; j++) {
                temp_page_buffer[j] = buf_addr[current_buf_index + j];
            }
            src_addr = (uint32_t)temp_page_buffer;
        }

        enter_critical_section();

        FLASH->command    = CMD_WRITEPAGE;
        FLASH->flash_addr = current_flash_addr;
        FLASH->mem_addr   = src_addr;
        FLASH->pattern    = FLASH_UNLOCK_PATTER;
        FLASH->start      = STARTBIT;

        while (flash_check_busy()) {}

        leave_critical_section();

        current_flash_addr += LENGTH_PAGE;
        current_buf_index  += LENGTH_PAGE;
        bytes_remaining    -= LENGTH_PAGE;
    }

    /* =========================================================
     * C) last page：read merge）
     * ========================================================= */
    if (bytes_remaining > 0u) {

        /*  */
        if (flash_check_address(current_flash_addr, LENGTH_PAGE) == STATUS_INVALID_PARAM) {
            return STATUS_INVALID_PARAM;
        }

        /* 1) read page */
        {
            volatile uint8_t *flash_ptr = (volatile uint8_t *)current_flash_addr;
            for (i = 0; i < LENGTH_PAGE; i++) {
                temp_page_buffer[i] = flash_ptr[i];
            }
        }

        /* 2) merge：overwrite bytes_remaining*/
        for (i = 0; i < bytes_remaining; i++) {
            temp_page_buffer[i] = buf_addr[current_buf_index + i];
        }

        /* 3) Page write */
        enter_critical_section();

        FLASH->command    = CMD_WRITEPAGE;
        FLASH->flash_addr = current_flash_addr;
        FLASH->mem_addr   = (uint32_t)temp_page_buffer;
        FLASH->pattern    = FLASH_UNLOCK_PATTER;
        FLASH->start      = STARTBIT;

        while (flash_check_busy()) {}

        leave_critical_section();
    }

    return STATUS_SUCCESS;
}

uint32_t flash_read_n_bytes(uint32_t read_flash_addr, uint32_t data_buf_addr, uint32_t data_len) {
    
    if (data_len == 0) {
        return STATUS_INVALID_PARAM;
    }
    
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    //  static aligned，avoid stack overflow
    static __attribute__((aligned(4))) uint8_t page_buffer[LENGTH_PAGE];
    
    uint8_t *user_buf = (uint8_t *)data_buf_addr;
    uint32_t bytes_remaining = data_len;
    uint32_t current_flash_addr = read_flash_addr;
    uint32_t buf_offset = 0;
    
    while (bytes_remaining > 0) {
        //  page boundary (256 bytes)
        uint32_t aligned_page_addr = (current_flash_addr & ~(LENGTH_PAGE - 1));
        
        // cal offset and copy size
        uint32_t offset_in_page = current_flash_addr - aligned_page_addr;
        uint32_t bytes_to_copy = LENGTH_PAGE - offset_in_page;
        if (bytes_to_copy > bytes_remaining) {
            bytes_to_copy = bytes_remaining;
        }
        
        // aligned page
        enter_critical_section();
        FLASH->command = CMD_READPAGE;
        FLASH->flash_addr = aligned_page_addr;  //  256-byte aligned
        FLASH->mem_addr = (uint32_t)page_buffer;  //  4-byte aligned
        FLASH->pattern = FLASH_UNLOCK_PATTER;
        FLASH->start = STARTBIT;
        leave_critical_section();
        
        while (flash_check_busy()) {}
        
        // copy need size data
        for (uint32_t i = 0; i < bytes_to_copy; i++) {
            user_buf[buf_offset++] = page_buffer[offset_in_page + i];
        }
        
        bytes_remaining -= bytes_to_copy;
        current_flash_addr += bytes_to_copy;
    }

    return STATUS_SUCCESS;
}


uint32_t flash_write_byte(uint32_t write_addr, uint8_t singlebyte) {


    if (flash_check_address(write_addr, LENGTH_BYTE) == STATUS_INVALID_PARAM) {
        return STATUS_INVALID_PARAM; //invalid addres range
    }

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }
    

    enter_critical_section();
    FLASH->command = CMD_WRITEBYTE;
    FLASH->flash_addr = write_addr;
    FLASH->flash_data = singlebyte;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
	leave_critical_section();
    while (flash_check_busy()) {} //wait busy finish

	
    return STATUS_SUCCESS;
}

uint32_t flash_verify_page(uint32_t read_page_addr) {

    if (flash_check_address(read_page_addr, LENGTH_PAGE)
        == STATUS_INVALID_PARAM) {
        return STATUS_INVALID_PARAM; //invalid addres range
    }

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    enter_critical_section();
    FLASH->command = CMD_READVERIFY;
    FLASH->flash_addr = read_page_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
	leave_critical_section();
    while (flash_check_busy()) {}

    return STATUS_SUCCESS;
}

uint32_t flash_get_status_reg(flash_status_t* status) {

    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    if ((status->require_mode) & FLASH_STATUS_RW1) {
        enter_critical_section();
        FLASH->command = CMD_READ_STATUS1;
        FLASH->pattern = FLASH_UNLOCK_PATTER;
        FLASH->start = STARTBIT;
        leave_critical_section();

        /*this check_busy is very short... it just send command then to receive data*/
        while (flash_check_busy()) {}
        status->status1 = (uint8_t)((FLASH->flash_data) >> 8);
    }

    if (status->require_mode & FLASH_STATUS_RW2) {
          
        enter_critical_section(); 
        FLASH->command = CMD_READ_STATUS2;
        FLASH->pattern = FLASH_UNLOCK_PATTER;
        FLASH->start = STARTBIT;
		leave_critical_section();
        while (flash_check_busy()) {}
        status->status2 = (uint8_t)((FLASH->flash_data) >> 8);
    }

    /*2022/01/18: GD does NOT have status bytes3.*/

    return STATUS_SUCCESS;
}

uint32_t flash_set_status_reg(const flash_status_t* status) {

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }
    /*
     * 2022/01/18: GD only have status bytes1 and bytes2.
     * GD only support command 0x01. So if you want to write
     */
    if (status->require_mode == FLASH_STATUS_RW1_2) {

        enter_critical_section();

        /*GD write status2 must two bytes */
        FLASH->command = CMD_WRITE_STATUS;
        FLASH->status = (uint32_t)(status->status1)
                        | (uint32_t)((status->status2) << 8);
        FLASH->pattern = FLASH_UNLOCK_PATTER;
        FLASH->start = STARTBIT;
		leave_critical_section();
        while (flash_check_busy()) {}
    } else if (status->require_mode == FLASH_STATUS_RW1) {
    	
    	enter_critical_section();
        FLASH->command = CMD_WRITE_STATUS1;
        FLASH->status = (status->status1);
        FLASH->pattern = FLASH_UNLOCK_PATTER;
        FLASH->start = STARTBIT;
		leave_critical_section();
        while (flash_check_busy()) {}
    }

    return STATUS_SUCCESS;
}

uint32_t flash_write_sec_register(uint32_t buf_addr, uint32_t write_reg_addr) {
    uint32_t addr;
    /*first we should check write_reg_addr*/
    addr = write_reg_addr >> 12;

    if ((addr > 3) || (write_reg_addr & 0xFF)) {
        /*only support 3 secureity register.*/
        /*We need secure register write to be 256 bytes alignment*/
        return STATUS_INVALID_PARAM;
    }

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }


    enter_critical_section();
    FLASH->command = CMD_WRITE_SEC_PAGE;
    FLASH->flash_addr = write_reg_addr;
    FLASH->mem_addr = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
	leave_critical_section();
	while (flash_check_busy()) {}
    return STATUS_SUCCESS;
}

uint32_t flash_read_sec_register(uint32_t buf_addr, uint32_t read_reg_addr) {
    uint32_t addr;
    /*first we should check read_reg_addr*/
    addr = read_reg_addr >> 12;

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    //if((addr>3)|| (read_reg_addr & 0xFF)) {
    if (addr > 3) {
        /*We need secure register read to be 256 bytes alignment*/
        return STATUS_INVALID_PARAM;
    }

    enter_critical_section();
    
    FLASH->command = CMD_READ_SEC_PAGE;
    FLASH->flash_addr = read_reg_addr;
    FLASH->mem_addr = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
	leave_critical_section();
    while (flash_check_busy()) {}

    return STATUS_SUCCESS;
}

uint32_t flash_get_unique_id(uint32_t flash_id_buf_addr, uint32_t buf_length) {
    uint32_t i;
    uint8_t temp[16], *ptr;

    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    /*
     * Notice: we don't check flash_id_buf_addr value here..
     * it should be correct address in SRAM!
     */
    if (buf_length == 0) {
        return STATUS_INVALID_PARAM;
    } else if (buf_length > 16) {
        buf_length = 16;
    }

    enter_critical_section();
    
    FLASH->command = CMD_READUID;
    FLASH->page_read_word = 0xF;
    FLASH->mem_addr = (uint32_t)temp;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
	leave_critical_section();
    ptr = (uint8_t*)flash_id_buf_addr; /*set address*/

    while (flash_check_busy()) {}

    FLASH->page_read_word = 0xFF; /*restore read one page length by default*/

    /*move unique number from stack to assign buffer*/
    for (i = 0; i < buf_length; i++) {
        ptr[i] = temp[i];
    }

    return STATUS_SUCCESS;
}


void flash_timing_init(void) {
    uint32_t clk_mode, sys_clk;
    uint16_t tdp, tres, tsus, trs, flash_type_id;

    flash_timing_mode_t flash_timing;
    /*change AHB clock also need change flash timing.*/
    flash_type_id = flash_get_deviceinfo() & FLASH_TYPE_ID_MAKS;

    clk_mode = get_ahb_system_clk();

    sys_clk = 0;
    /*check flash type to adjust flash timing*/
    if (flash_type_id == GDWQ_ID) {
        tdp = GDWQ_FLASH_TDP;
        tres = GDWQ_FLASH_TRES1;
        tsus = GDWQ_FLASH_TSUS;
        trs = GDWQ_FLASH_TRS;
    }

    if (flash_type_id == GDLQ_ID) {
        tdp = GDLQ_FLASH_TDP;
        tres = GDLQ_FLASH_TRES1;
        tsus = GDLQ_FLASH_TSUS;
        trs = GDLQ_FLASH_TRS;
    }

    if (flash_type_id == PUYA_ID) {
        tdp = PUYA_FLASH_TDP;
        tres = PUYA_FLASH_TRES1;
        tsus = PUYA_FLASH_TSUS;
        trs = PUYA_FLASH_TRS;
    }

    if (clk_mode == SYS_32MHZ_CLK)
    {
        sys_clk = 32;
    }
    else if (clk_mode == SYS_48MHZ_CLK)
    {
        sys_clk = 48;
    }
    else if (clk_mode == SYS_64MHZ_CLK)
    {
        sys_clk = 64;
    }
    else
    {
        sys_clk = 32;
    }

    flash_timing.deep_pd_timing = tdp * sys_clk + 2;
    flash_timing.deep_rpd_timing = tres * sys_clk + 2;
    flash_timing.suspend_timing = tsus * sys_clk + 2;
    flash_timing.resume_timing = trs * sys_clk + 2;

    //for FPGA Verify
    flash_set_timing(&flash_timing);
}

/** 
 * \brief  Read otp secure register
 */
uint32_t flash_read_otp_sec_register(uint32_t buf_addr,
                                     uint32_t read_reg_addr) {
    uint32_t addr;

    /*first we should check read_reg_addr*/
    addr = read_reg_addr >> 12;

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy()) {
        return STATUS_EBUSY;
    }

    //if((addr>3)|| (read_reg_addr & 0xFF)) {
    if (addr > 3) {
        /*We need secure register read to be 256 bytes alignment*/
        return STATUS_INVALID_PARAM;
    }

    FLASH->command = CMD_READ_SEC_PAGE;
    FLASH->flash_addr = read_reg_addr;
    FLASH->mem_addr = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;

    while (flash_check_busy()) {}

    return STATUS_SUCCESS;
}

/** 
 * \brief  Read otp secure page
 */
uint32_t flash_read_otp_sec_page(uint32_t buf_addr) {
    switch (flash_get_deviceinfo()) //check flash device
    {
        case RT581_FLASH_TYPE: //0x2000
        case FLASH_512K_TYPE:  //0x2000
        case FLASH_4MB_TYPE: //0x2000
            if (flash_read_otp_sec_register((uint32_t)buf_addr,
                                            FLASH_SECREG_R2_P0)) {
                return STATUS_INVALID_PARAM;
            }
            break;

        case FLASH_1MB_TYPE: //0x0000
        case FLASH_2MB_TYPE: //0x0000
            if (flash_read_otp_sec_register((uint32_t)buf_addr,
                                            FLASH_SECREG_R0_P0)) {
                return STATUS_INVALID_PARAM;
            }
    }

    return STATUS_SUCCESS;
}

/**
* @brief Enable flash Suspend fucntion
*/
void flash_enable_suspend(void)
{
    FLASH->control_set = FLASH->control_set | 0x200;
}

/**
*@brief Disable flash Suspend fucntion
*/
void flash_disable_suspend(void)
{
    FLASH->control_set = FLASH->control_set & ~(0x200);
}

/**
*@brief get flash control register value
*/
uint32_t flash_get_control_reg(void)
{
    return FLASH->control_set;
}

/**
* @brief Flash_Erase: erase flash address data
*/
uint32_t flash_erase_mpsector()
{

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy())
    {
        return  STATUS_EBUSY;
    }

    enter_critical_section();
    FLASH->command =  CMD_ERASESECTOR;

#if defined(CONFIG_FLASHCTRL_SECURE_EN)

    if (flash_size() == FLASH_1024K)
    {
        FLASH->flash_addr = (0x000FF000 + FLASH_SECURE_MODE_BASE_ADDR);
    }
    else if (flash_size() == FLASH_2048K)
    {
        FLASH->flash_addr = (0x001FF000 + FLASH_SECURE_MODE_BASE_ADDR);
    }
    else if (flash_size() == FLASH_4096K)
    {
        FLASH->flash_addr = (0x003FF000 + FLASH_SECURE_MODE_BASE_ADDR);
    }
#else
    if (flash_size() == FLASH_1024K)
    {
        FLASH->flash_addr = 0x000FF000;
    }
    else if (flash_size() == FLASH_2048K)
    {
        FLASH->flash_addr = 0x001FF000;
    }
    else if (flash_size() == FLASH_4096K)
    {
        FLASH->flash_addr = 0x003FF000;
    }
#endif

    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;

    while (flash_check_busy()) {;}

    leave_critical_section();

    return STATUS_SUCCESS;
}
/**
* @brief flash write one page (256bytes) data
*/
uint32_t flash_write_mp_sector(uint32_t buf_addr, uint32_t write_page_addr)
{
    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy())
    {
        return  STATUS_EBUSY;
    }

    if (flash_size() == FLASH_1024K)
    {

#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_page_addr < 0x100FF000) || (write_page_addr > 0x10100000))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_page_addr < 0x000FF000) || (write_page_addr > 0x00100000))
        {
            return STATUS_INVALID_PARAM;
        }
#endif

    }
    else if (flash_size() == FLASH_2048K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_page_addr < 0x101FF000) || (write_page_addr > 0x10200000))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_page_addr < 0x001FF000) || (write_page_addr > 0x00200000))
        {
            return STATUS_INVALID_PARAM;
        }
#endif
    }
    else if (flash_size() == FLASH_4096K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_page_addr < 0x103FF000) || (write_page_addr > 0x10400000))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_page_addr < 0x003FF000) || (write_page_addr > 0x00400000))
        {
            return STATUS_INVALID_PARAM;
        }
#endif
    }


    enter_critical_section();

    FLASH->command =  CMD_WRITEPAGE;
    FLASH->flash_addr = write_page_addr;
    FLASH->mem_addr  = buf_addr;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;

    while (flash_check_busy()) {;}

    leave_critical_section();

    return STATUS_SUCCESS;
}

/**
* @brief write one byte data into flash.
*/
uint32_t flash_write_mpsector_txpwrcfgbyte(uint32_t write_addr, uint8_t singlebyte)
{

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy())
    {
        return  STATUS_EBUSY;
    }

    if (flash_size() == FLASH_1024K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_addr < 0x100FFFD8) || (write_addr > 0x100FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_addr < 0x000FFFD8) || (write_addr > 0x000FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#endif

    }
    else if (flash_size() == FLASH_2048K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_addr < 0x101FFFD8) || (write_addr > 0x101FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_addr < 0x001FFFD8) || (write_addr > 0x001FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#endif
    }
    else if (flash_size() == FLASH_4096K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_addr < 0x103FFFD8) || (write_addr > 0x103FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_addr < 0x003FFFD8) || (write_addr > 0x003FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#endif
    }


    enter_critical_section();

    FLASH->command =  CMD_WRITEBYTE;
    FLASH->flash_addr = write_addr;
    FLASH->flash_data = singlebyte;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
    while (flash_check_busy()) {;}
    leave_critical_section();

    return STATUS_SUCCESS;
}
/**
* @brief write one byte data into flash.
*/
uint32_t flash_write_mpsector_rftrimbyte(uint32_t write_addr, uint8_t singlebyte)
{

    /*2022/04/28 add, Device busy. try again.*/
    if (flash_check_busy())
    {
        return  STATUS_EBUSY;
    }

    if (flash_size() == FLASH_1024K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_addr < 0x100FF219) || (write_addr > 0x100FF22A))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_addr < 0x000FFFD8) || (write_addr > 0x000FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#endif

    }
    else if (flash_size() == FLASH_2048K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_addr < 0x101FF219) || (write_addr > 0x101FF22A))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_addr < 0x001FFFD8) || (write_addr > 0x001FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#endif
    }
    else if (flash_size() == FLASH_4096K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)

        if ((write_addr < 0x103FF219) || (write_addr > 0x103FF22A))
        {
            return STATUS_INVALID_PARAM;
        }
#else
        if ((write_addr < 0x003FFFD8) || (write_addr > 0x003FFFDF))
        {
            return STATUS_INVALID_PARAM;
        }
#endif
    }


    enter_critical_section();

    FLASH->command =  CMD_WRITEBYTE;
    FLASH->flash_addr = write_addr;
    FLASH->flash_data = singlebyte;
    FLASH->pattern = FLASH_UNLOCK_PATTER;
    FLASH->start = STARTBIT;
    while (flash_check_busy()) {;}
    leave_critical_section();

    return STATUS_SUCCESS;
}

