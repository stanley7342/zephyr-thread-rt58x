/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_i2c_slave.c
 * \brief           Hosal i2c slave driver file
 */
/*
 * Author:          Kc.tseng
 */

#include "hosal_i2c_slave.h"

uint32_t hosal_i2c_slave_open(hosal_i2c_slave_mode_t* i2c_slave_client) {
    i2c_slave_mode_t cfg = {
        .i2c_slave_cb_func = i2c_slave_client->i2c_slave_cb_func,
        .i2c_bus_timeout_enable = i2c_slave_client->i2c_bus_timeout_enable,
        .i2c_bus_timeout = i2c_slave_client->i2c_bus_timeout,
        .i2c_slave_addr = i2c_slave_client->i2c_slave_addr,
    };

    return i2c_slave_open(&cfg);
}

uint32_t hosal_i2c_slave_close(void) { 
    return i2c_slave_close ();
}
