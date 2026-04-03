/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_i2c_master.c
 * \brief           Hosal i2c master driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "mcu.h"
#include "hosal_i2c_master.h"

uint32_t hosal_i2c_preinit(uint32_t master_id) {
    return i2c_preinit();
}

uint32_t hosal_i2c_init(uint32_t i2c_master_port, uint32_t i2c_speed) {
    return i2c_init(i2c_speed);
}

uint32_t hosal_i2c_write(uint32_t i2c_master_port, void* slave, uint8_t* data,
                         uint32_t len) {
    hosal_i2c_master_mode_t* hosal_cfg = (hosal_i2c_master_mode_t*)slave;
    i2c_master_mode_t drv_cfg = {
        .bFlag_16bits = hosal_cfg->bFlag_16bits,
        .dev_addr = hosal_cfg->dev_addr,
        .reg_addr = hosal_cfg->reg_addr,
    };
    return i2c_write(&drv_cfg, data, len, hosal_cfg->i2c_usr_isr);
}

uint32_t hosal_i2c_read(uint32_t i2c_master_port,
                        hosal_i2c_master_mode_t* slave, uint8_t* data,
                        uint32_t len) {
    i2c_master_mode_t drv_cfg = {
        .bFlag_16bits = slave->bFlag_16bits,
        .dev_addr = slave->dev_addr,
        .reg_addr = slave->reg_addr,
    };
    return i2c_read(&drv_cfg, data, len, slave->i2c_usr_isr);
}
