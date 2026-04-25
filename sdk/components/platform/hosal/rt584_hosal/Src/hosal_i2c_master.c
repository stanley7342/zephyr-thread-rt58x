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

#include "hosal_i2c_master.h"

uint32_t hosal_i2c_preinit(uint32_t master_id) {
    uint32_t rval;

    rval = i2c_preinit(master_id);

    return rval;
}

uint32_t hosal_i2c_init(uint32_t master_id, uint32_t i2c_speed) {
    uint32_t rval;

    rval = i2c_master_init(master_id, i2c_speed);

    return rval;
}

uint32_t hosal_i2c_write(uint32_t master_id, hosal_i2c_master_mode_t* slave,
                         uint8_t* data, uint32_t len) {
    i2c_master_mode_t drv_cfg = {
        .bFlag_16bits = slave->bFlag_16bits,
        .dev_addr = slave->dev_addr,
        .reg_addr = slave->reg_addr,
        .endproc_cb = slave->i2c_usr_isr,
    };

    uint32_t rval;

    rval = i2c_master_write(master_id, &drv_cfg, data, len);

    return rval;
}

uint32_t hosal_i2c_read(uint32_t master_id, hosal_i2c_master_mode_t* slave,
                        uint8_t* data, uint32_t len) {
    i2c_master_mode_t drv_cfg = {
        .bFlag_16bits = slave->bFlag_16bits,
        .dev_addr = slave->dev_addr,
        .reg_addr = slave->reg_addr,
        .endproc_cb = slave->i2c_usr_isr,
    };

    uint32_t rval;

    rval = i2c_master_read(master_id, &drv_cfg, data, len);

    return rval;
}
