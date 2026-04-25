/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           sysctrl.c
 * \brief          system control driver 
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#include <stdio.h>
#include "sysctrl.h"
#include "gpio.h"

void delay_us(volatile uint32_t us) {

    volatile uint32_t  Delay = 0, sys_clk;
    volatile uint32_t clk_mode = get_ahb_system_clk();
    
    sys_clk = (clk_mode + 2) << 4;
    Delay = (us * sys_clk ) / 10;

    do
    {
        __NOP();
        __NOP();
        __NOP();
        __NOP();

    } while (Delay--);
}

void delay_ms(volatile uint32_t ms) {

    while (ms--)
    {
       delay_us(1000);
    }
}

void pin_set_out_mode_ex(uint32_t pin_number, uint32_t mode) {

    uint32_t base, reg_shift;
    uint32_t index;
    uint32_t temp_value;

    /*RT584 mode become 4 bits.. */
    if (pin_number > 32) {
        return; /*Invalid setting mode.*/
    }

    index = (pin_number / 4);
    base = (pin_number % 4);
    reg_shift = (base << 3);

    temp_value = (SYSCTRL->sys_gpio_omux[index] & ~(0x3F << reg_shift));
    temp_value = temp_value | (mode << reg_shift);
    SYSCTRL->sys_gpio_omux[index] = temp_value;

    return;
}

void pin_set_in_mode_ex(uint32_t pin_number, uint32_t mode) {
    uint32_t reg_shift, enable;
    uint32_t index;
    uint32_t temp_value;
    
    /*RT584 mode become 4 bits..*/
    if (pin_number > 32) {
        return; /*Invalid setting mode.*/
    }

    index = 0;
    reg_shift = 0;

    index = (mode >> 28 & 0x07);
    reg_shift = (mode & 0x18);
    enable = ((mode & 0x18) + 7);

    gpio_input_enable(pin_number);

    /*
    SYSCTRL->sys_gpio_imux[index] = (SYSCTRL->sys_gpio_imux[index]
                                     & ~(0x3F << reg_shift));
    SYSCTRL->sys_gpio_imux[index] = (SYSCTRL->sys_gpio_imux[index]
                                     | (pin_number << reg_shift));
    SYSCTRL->sys_gpio_imux[index] = (SYSCTRL->sys_gpio_imux[index]
                                     | (1 << enable));
    */ 
    temp_value = (SYSCTRL->sys_gpio_imux[index]&~(0x3F<<reg_shift));

    temp_value = (temp_value|(pin_number << reg_shift));
            
    temp_value = (temp_value|(1<< enable));

    SYSCTRL->sys_gpio_imux[index] = temp_value;
    return;
}

void pin_set_mode(uint32_t pin_number, uint32_t mode) {
    /*
     * avoid to set pin 10 & 11 these two is SWD/SWI
     */
    if ((pin_number >= 32) || (pin_number == 10) || (pin_number == 11)
        || (mode >= MODE_MAX)) {
        return; /*Invalid setting mode.*/
    }
    switch (mode) {
        case MODE_GPIO:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            break;

        case MODE_UART0_TX:
            pin_set_out_mode_ex(pin_number, MODE_UART0_TX_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            /* Tx idle is High */
            gpio_pin_set(pin_number);
            break;

        case MODE_UART0_RX:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_UART0_RX_REG_VALUE);
            break;

        case MODE_UART1_TX:
            pin_set_out_mode_ex(pin_number, MODE_UART1_TX_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            /* Tx idle is High */
            gpio_pin_set(pin_number);
            break;

        case MODE_UART1_RX:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_UART1_RX_REG_VALUE);
            break;

        case MODE_UART1_RTSN:
            pin_set_out_mode_ex(pin_number, MODE_UART1_RTSN_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_UART1_CTSN:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_UART1_CTSN_REG_VALUE);
            break;

        case MODE_UART2_TX:
            pin_set_out_mode_ex(pin_number, MODE_UART2_TX_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            /* Tx idle is High */
            gpio_pin_set(pin_number);
            break;

        case MODE_UART2_RX:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_UART2_RX_REG_VALUE);
            break;

        case MODE_UART2_RTSN:
            pin_set_out_mode_ex(pin_number, MODE_UART2_RTSN_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_UART2_CTSN:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_UART2_CTSN_REG_VALUE);
            break;

        case MODE_PWM0:
            pin_set_out_mode_ex(pin_number, MODE_PWM0_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_PWM1:
            pin_set_out_mode_ex(pin_number, MODE_PWM1_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_PWM2:
            pin_set_out_mode_ex(pin_number, MODE_PWM2_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_PWM3:
            pin_set_out_mode_ex(pin_number, MODE_PWM3_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_PWM4:
            pin_set_out_mode_ex(pin_number, MODE_PWM4_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_IRM:
            pin_set_out_mode_ex(pin_number, MODE_IRM_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            /* Tx idle is High */
            gpio_pin_set(pin_number);
            break;

        case MODE_I2CM0_SCL:
            pin_set_out_mode_ex(pin_number, MODE_I2CM0_SCL_OUT_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_I2CM0_SCL_REG_VALUE);
            break;

        case MODE_I2CM0_SDA:
            pin_set_out_mode_ex(pin_number, MODE_I2CM0_SDA_OUT_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_I2CM0_SDA_REG_VALUE);
            break;

        case MODE_I2CM1_SCL:
            pin_set_out_mode_ex(pin_number, MODE_I2CM1_SCL_OUT_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_I2CM1_SCL_REG_VALUE);
            break;

        case MODE_I2CM1_SDA:
            pin_set_out_mode_ex(pin_number, MODE_I2CM1_SDA_OUT_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_I2CM1_SDA_REG_VALUE);
            break;

        case MODE_I2CS_SCL:
            pin_set_out_mode_ex(pin_number, MODE_I2CS_SCL_OUT_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_I2CS_SCL_REG_VALUE);
            break;

        case MODE_I2CS_SDA:
            pin_set_out_mode_ex(pin_number, MODE_I2CS_SDA_OUT_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_I2CS_SDA_REG_VALUE);
            break;

        case MODE_SPI0_MASTER_SCLK:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SCLK_OUT_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI0_MASTER_CSN0:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_CSN0_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI0_MASTER_MOSI:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT0_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI0_MASTER_MISO:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA1_REG_VALUE);
            break;

        case MODE_SPI0_MASTER_SDATA0:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT0_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA0_REG_VALUE);
            break;

        case MODE_SPI0_MASTER_SDATA1:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT1_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA1_REG_VALUE);
            break;

        case MODE_SPI0_MASTER_SDATA2:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT2_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA2_REG_VALUE);
            break;

        case MODE_SPI0_MASTER_SDATA3:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT3_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA3_REG_VALUE);
            break;

        case MODE_SPI0_SLAVE_SCLK:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SCLK_REG_VALUE);
            break;

        case MODE_SPI0_SLAVE_CSN0:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_CSN_REG_VALUE);
            break;

        case MODE_SPI0_SLAVE_MOSI:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA0_REG_VALUE);
            break;

        case MODE_SPI0_SLAVE_MISO:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT1_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI1_MASTER_SCLK:
            pin_set_out_mode_ex(pin_number, MODE_SPI1_SCLK_OUT_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI1_MASTER_CSN0:
            pin_set_out_mode_ex(pin_number, MODE_SPI1_CSN0_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI1_MASTER_MOSI:
            pin_set_out_mode_ex(pin_number, MODE_SPI1_SDATA_OUT0_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI1_MASTER_MISO:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI1_SDATA1_REG_VALUE);
            break;

        case MODE_SPI1_MASTER_SDATA0:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT0_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA0_REG_VALUE);
            break;

        case MODE_SPI1_MASTER_SDATA1:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT1_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA1_REG_VALUE);
            break;

        case MODE_SPI1_MASTER_SDATA2:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT2_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA2_REG_VALUE);
            break;

        case MODE_SPI1_MASTER_SDATA3:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_SDATA_OUT3_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_SDATA3_REG_VALUE);
            break;

        case MODE_SPI1_SLAVE_SCLK:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI1_SCLK_REG_VALUE);
            break;

        case MODE_SPI1_SLAVE_CSN0:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI1_CSN_REG_VALUE);
            break;

        case MODE_SPI1_SLAVE_MOSI:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI1_SDATA0_REG_VALUE);
            break;

        case MODE_SPI1_SLAVE_MISO:
            pin_set_out_mode_ex(pin_number, MODE_SPI1_SDATA_OUT1_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI0_MASTER_CSN1:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_CSN1_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI0_MASTER_CSN2:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_CSN2_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI0_MASTER_CSN3:
            pin_set_out_mode_ex(pin_number, MODE_SPI0_CSN3_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI1_MASTER_CSN1:
            pin_set_out_mode_ex(pin_number, MODE_SPI1_CSN1_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI1_MASTER_CSN2:
            pin_set_out_mode_ex(pin_number, MODE_SPI1_CSN2_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI1_MASTER_CSN3:
            pin_set_out_mode_ex(pin_number, MODE_SPI1_CSN3_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SPI0_SLAVE_CSN1:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_CSN_REG_VALUE);
            break;

        case MODE_SPI0_SLAVE_CSN2:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_CSN_REG_VALUE);
            break;

        case MODE_SPI0_SLAVE_CSN3:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI0_CSN_REG_VALUE);
            break;

        case MODE_SPI1_SLAVE_CSN1:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI1_CSN_REG_VALUE);
            break;

        case MODE_SPI1_SLAVE_CSN2:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI1_CSN_REG_VALUE);
            break;

        case MODE_SPI1_SLAVE_CSN3:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_QSPI1_CSN_REG_VALUE);
            break;

        case MODE_I2S_BCK:
            pin_set_out_mode_ex(pin_number, MODE_I2S_BCK_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_I2S_WCK:
            pin_set_out_mode_ex(pin_number, MODE_I2S_WCK_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_I2S_SDO:
            pin_set_out_mode_ex(pin_number, MODE_I2S_SDO_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_I2S_SDI:
            pin_set_out_mode_ex(pin_number, MODE_GPIO_REG_VALUE);
            pin_set_in_mode_ex(pin_number, MODE_I2S_SDI_REG_VALUE);
            break;

        case MODE_I2S_MCLK:
            pin_set_out_mode_ex(pin_number, MODE_I2S_MCLK_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_SWD:
            pin_set_out_mode_ex(pin_number, MODE_SWD_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG0:
            pin_set_out_mode_ex(pin_number, MODE_DBG0_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG1:
            pin_set_out_mode_ex(pin_number, MODE_DBG1_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG2:
            pin_set_out_mode_ex(pin_number, MODE_DBG2_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG3:
            pin_set_out_mode_ex(pin_number, MODE_DBG3_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG4:
            pin_set_out_mode_ex(pin_number, MODE_DBG4_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG5:
            pin_set_out_mode_ex(pin_number, MODE_DBG5_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG6:
            pin_set_out_mode_ex(pin_number, MODE_DBG6_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG7:
            pin_set_out_mode_ex(pin_number, MODE_DBG7_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG8:
            pin_set_out_mode_ex(pin_number, MODE_DBG8_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBG9:
            pin_set_out_mode_ex(pin_number, MODE_DBG9_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBGA:
            pin_set_out_mode_ex(pin_number, MODE_DBGA_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBGB:
            pin_set_out_mode_ex(pin_number, MODE_DBGB_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBGC:
            pin_set_out_mode_ex(pin_number, MODE_DBGC_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBGD:
            pin_set_out_mode_ex(pin_number, MODE_DBGD_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBGE:
            pin_set_out_mode_ex(pin_number, MODE_DBGE_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;

        case MODE_DBGF:
            pin_set_out_mode_ex(pin_number, MODE_DBGF_REG_VALUE);
            pin_set_pullopt(pin_number, PULL_NONE);
            gpio_cfg_output(pin_number);
            break;
    }
}





uint32_t pin_get_mode(uint32_t pin_number)
{
    if (pin_number >= 32)
    {
        return 0xF;     /*Invalid setting mode.*/
    }

    return 0;
}

void pin_enable_schmitt(uint32_t pin_number) {
    SYSCTRL->gpio_en_schmitt |= 0x1 << pin_number;
}

void pin_disable_schmitt(uint32_t pin_number) {
    SYSCTRL->gpio_en_schmitt &= ~(0x1 << pin_number);
}

void pin_enable_filter(uint32_t pin_number) {
    SYSCTRL->gpio_en_filter |= 0x1 << pin_number;
}

void pin_disable_filter(uint32_t pin_number) {
    SYSCTRL->gpio_en_filter &= ~(0x1 << pin_number);
}

void enable_perclk(uint32_t clock) {
    if ((clock > GPIO_32K_CLK)) {
        return;     /*Invalid setting mode.*/
    }

    SYSCTRL->sys_clk_ctrl1.reg |= (1 << clock) ;
}

void disable_perclk(uint32_t clock) {
    if ((clock > GPIO_32K_CLK)) {
        return;     /*Invalid setting mode.*/
    }

    SYSCTRL->sys_clk_ctrl1.reg  &= ~(1 << clock);
}

void pin_set_pullopt(uint32_t pin_number, uint32_t mode) {
    uint32_t reg, base, mask_offset, mask;

    if ((pin_number >= 32) || (mode > 7)) {
        return;     /*Invalid setting mode.*/
    }

    base = PULLOPT_BASE + (pin_number >> 3) * 4;
    mask_offset = (pin_number & 0x7) << 2;
    mask = 0xF << mask_offset;

    /*pin mux setting is share resource.*/
    reg = *((volatile unsigned int *) base);
    reg = reg & ~mask;
    reg = reg | (mode << mask_offset);

    *((volatile unsigned int *)base) =  reg;

    return;
}

void pin_set_drvopt(uint32_t pin_number, uint32_t mode) {
    uint32_t reg, base, mask_offset, mask;

    if ((pin_number >= 32) || (mode > 3)) {
        return;     /*Invalid setting mode.*/
    }

    base = DRV_BASE + (pin_number >> 4) * 4;
    mask_offset = (pin_number & 0xF) << 1;
    mask = 0x3 << mask_offset;

    /*pin mux setting is share resource.*/
    reg = *((volatile unsigned int *) base);
    reg = reg & ~mask;
    reg = reg | (mode << mask_offset);

    *((volatile unsigned int *)base) =  reg;

    return;
}

void enable_pin_opendrain(uint32_t pin_number) {
    uint32_t base, mask, reg;

    if (pin_number >= 32) {
        return;     /*Invalid setting mode.*/
    }

    base = OD_BASE ;
    mask = 1 << pin_number ;

    /*pin mux setting is share resource.*/
    reg = *((volatile unsigned int *) base);
    reg = reg | mask;
    *((volatile unsigned int *)base) =  reg;

    return;
}

void disable_pin_opendrain(uint32_t pin_number) {
    uint32_t base, mask, reg;

    if (pin_number >= 32) {
        return;     /*Invalid setting mode.*/
    }

    base = OD_BASE ;
    mask = ~(1 << pin_number);

    /*pin mux setting is share resource.*/
    reg = *((volatile unsigned int *) base);
    reg = reg & mask;
    *((volatile unsigned int *)base) =  reg;

    return;
}

sys_clk_sel_t pll_unlock_check(void)
{
    sys_clk_sel_t sys_clk_mode;
    uint32_t clk_sel;

    clk_sel = (SYSCTRL->sys_clk_ctrl.reg & 0x03);

    if (clk_sel == HCLK_SEL_PLL)
    {
        sys_clk_mode = get_ahb_system_clk(); /*get mcu system clock mode.*/
    }
    else
    {
        return clk_sel;
    }

    if ((sys_clk_mode == SYS_48MHZ_CLK) || (sys_clk_mode == SYS_64MHZ_CLK))   /*PLL 48Mhz/64Mhz unlock check*/
    {

        if (  ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_4)) ||
                ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_7))
           )
        {
            enter_critical_section();

            change_ahb_system_clk(SYS_32MHZ_CLK);

            leave_critical_section();

            sys_clk_mode = get_ahb_system_clk();
        }
        else  if (((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0)) || ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3)))
        {
            enter_critical_section();

            change_ahb_system_clk(sys_clk_mode);

            leave_critical_section();

            sys_clk_mode = get_ahb_system_clk();
        }
        else
        {
            /*PLL Stable*/
        }
    }

    return sys_clk_mode;
}

uint32_t pll_status_check(void)
{

    if (  ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_4)) ||
            ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() == PLL_LOCK_BANK_VCO_7))
       )
    {
        return STATUS_INVALID_REQUEST;
    }
    else  if (((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_0) && (PLL_BANK_VCO_STATUS() != PLL_LOCK_BANK_VCO_4)) ||
              ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() != PLL_LOCK_BANK_VCO_7))
             )
    {
        return STATUS_EBUSY;
    }
    else  if (PLL_LOCK_STATUS() == PLL_LOCK_DOWN)
    {
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_REQUEST;
}

uint32_t change_ahb_system_clk(sys_clk_sel_t sys_clk_mode)
{

    uint32_t i = 0, j=0;
    uint32_t pll_clk = 0, bank_vco=0, clk_status=0;

    if (sys_clk_mode > SYS_CLK_MAX)                                         /*Invalid parameter*/
    {
        return STATUS_ERROR;
    }

    SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg & ~MCU_HCLK_SEL_MASK);  /*Set MCU clock source to default 32MHZ*/
    SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg & ~(MCU_BBPLL_CLK_MASK | MCU_BBPLL_ENABLE)); /*disable pll*/

    if ((sys_clk_mode == SYS_48MHZ_CLK) || (sys_clk_mode == SYS_64MHZ_CLK))        /*Set PLL to 48MHz*/
    {
        PMU_CTRL->soc_bbpll1.bit.bbpll_ini_bank = 0;
        PMU_CTRL->soc_bbpll0.bit.bbpll_manubank = 0;
        pll_clk = (sys_clk_mode-SYS_CLK_OFFSET);

        SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg & ~MCU_BBPLL_CLK_MASK) | (pll_clk << MCU_BBPLL_CLK_SHIFT);

        for (j = 0; j < PLL_CHECK_COUNT; j++)
        {
            SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg | MCU_BBPLL_ENABLE);      /*config BASEBAND_PLL_ENABLE*/
            /*
            * need delay 200 us.
            * If code is not in cache, it will take more time for preload data.
            */
            for (i = 0; i < PLL_WAIT_PERIOD; i++)
            {
                __NOP();
            }

            if (PLL_LOCK_STATUS() == PLL_LOCK_DOWN)            //PLL lock down
            {
                SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg & ~MCU_HCLK_SEL_MASK) | MCU_HCLK_SEL_PLL; //switch pll clock outpu
                break;
            }
            else if ((PLL_VIBIT_STATUS() == PLL_LOCK_VIBIT_3) && (PLL_BANK_VCO_STATUS() < PLL_LOCK_BANK_VCO_7))
            {
                bank_vco = PLL_BANK_VCO_STATUS();
                PMU_CTRL->soc_bbpll1.bit.bbpll_ini_bank = (bank_vco + 1); //next bank
                PMU_CTRL->soc_bbpll0.bit.bbpll_manubank = 1;
            }
            else
            {
                SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg & MCU_BBPLL_DISABLE);            //baseband pll disable

                //delay > 32us
                for (i = 0; i < PLL_DELAY_PERIOD; i++)
                {
                    __NOP();
                }

                clk_status = pll_status_check();

                if (clk_status == STATUS_INVALID_REQUEST)
                {
                    break;
                }
            }
        }
    }
    else
    {
      //default 32M Clock
    }

    //Add pll status check lock success, config to pll clock
    if ((j == PLL_CHECK_COUNT) && (clk_status == STATUS_SUCCESS))
    {
        SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg | MCU_BBPLL_ENABLE);      /*config BASEBAND_PLL_ENABLE*/

        /*baseband pll enable wait delay time 200us*/
        for (i = 0; i < PLL_WAIT_PERIOD; i++)
        {
            __NOP();
        }

        SYSCTRL->sys_clk_ctrl.reg = (SYSCTRL->sys_clk_ctrl.reg & ~MCU_HCLK_SEL_MASK) | MCU_HCLK_SEL_PLL;
    }

    return STATUS_SUCCESS;
}

uint32_t get_ahb_system_clk(void) {
    
    uint32_t clk_sel = 0, clk_mode = 0;

    clk_sel = (SYSCTRL->sys_clk_ctrl.reg & 0x03);

    if (clk_sel == HCLK_SEL_PLL)
    {
        clk_mode = ((((SYSCTRL->sys_clk_ctrl.reg & MCU_BBPLL_CLK_MASK) >> MCU_BBPLL_CLK_SHIFT)) + SYS_CLK_OFFSET);
    }
    else
    {
        clk_mode = (SYSCTRL->sys_clk_ctrl.reg & 0x03);
    }

    return clk_mode;
}

uint32_t change_peri_clk(perclk_clk_sel_t sys_clk_mode) {
    if (sys_clk_mode == PERCLK_SEL_RCO1M) {
        PMU_CTRL->soc_pmu_rco1m.bit.en_rco_1m = 1;
    }

    SYSCTRL->sys_clk_ctrl.bit.per_clk_sel = sys_clk_mode;

    return STATUS_SUCCESS;
}

uint32_t get_peri_clk(void) {
    uint32_t clk_mode;

    clk_mode = SYSCTRL->sys_clk_ctrl.bit.per_clk_sel;

    return clk_mode;
}
/**
* \brief set sloc clock source
*/
void slow_clock_calibration(slow_clock_select_t rco_select)
{
    if (rco_select == RCO32K_SELECT)
    {

        PMU_CTRL->pmu_osc32k.bit.tune_fine_rco_32k = 88;
        PMU_CTRL->pmu_osc32k.bit.tune_coarse_rco_32k = 3;
        PMU_CTRL->pmu_osc32k.bit.pw_buf_rco_32k = 3;
        PMU_CTRL->pmu_osc32k.bit.pw_rco_32k = 15;
        PMU_CTRL->pmu_osc32k.bit.rco_32k_sel = 1;
        SYSCTRL->sys_clk_ctrl2.bit.en_rco32k_div2 = 0;

        //rco
        if (get_peri_clk() == PERCLK_SEL_16M)
        {
            RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_target = 64000; ////per_clk = 16mhz is  64000'd
        }
        else
        {
            RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_target = 128000; //per_clk = 32mhz is  128000'd
        }

        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_lock_err = 0x20;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_avg_coarse = 1;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_avg_fine = 2;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_avg_lock = 2;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_dly = 0;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_fine_gain = 10;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_lock_gain = 10;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_track_en = 1;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_skip_coarse = 1;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_bound_mode = 0;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_32k_rc_sel = 1;
        RCO32K_CAL->cal32k_cfg1.bit.en_ck_cal32k = 1;
        RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_en = 1;

        //offset 0x500060bc
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_mv_settle_time = 12;   //mv  settle time = 400us (400us for 1.8v, 3.3v 200us)
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_lv_settle_time = 5;    //lv  settle time = 62.5us
        //offset 0x50006024
        PMU_CTRL->pmu_soc_pmu_xtal1.bit.cfg_xtal_settle_time = 31;  //1ms
        //offset 0x50006040
        PMU_CTRL->pmu_rvd0.bit.cfg_xtal_fast_time = 15;             //0.5ms
        //offset 0x500060bc
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_pwrx_settle_time = 2;

    }
    else if (rco_select == RCO16K_SELECT)
    {
        PMU_CTRL->pmu_osc32k.bit.tune_fine_rco_32k = 88;
        PMU_CTRL->pmu_osc32k.bit.tune_coarse_rco_32k = 3;
        PMU_CTRL->pmu_osc32k.bit.pw_buf_rco_32k = 3;
        PMU_CTRL->pmu_osc32k.bit.pw_rco_32k = 0;
        PMU_CTRL->pmu_osc32k.bit.rco_32k_sel = 1;
        SYSCTRL->sys_clk_ctrl2.bit.en_ck_div_32k = 1;
        SYSCTRL->sys_clk_ctrl2.bit.en_rco32k_div2 = 1;

        if (get_peri_clk() == PERCLK_SEL_16M)
        {
            RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_target = 64000; //per_clk = 16mhz is  64000'd
        }
        else
        {
            RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_target = 128000; //per_clk = 32mhz is  128000'd
        }


        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_lock_err = 0x20;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_avg_coarse = 1;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_avg_fine = 2;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_avg_lock = 2;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_dly = 0;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_fine_gain = 10;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_lock_gain = 10;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_track_en = 1;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_skip_coarse = 1;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_cal32k_bound_mode = 0;
        RCO32K_CAL->cal32k_cfg1.bit.cfg_32k_rc_sel = 1;
        RCO32K_CAL->cal32k_cfg1.bit.en_ck_cal32k = 1;
        RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_en = 1;

        //offset 0x500060bc
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_mv_settle_time = 6;    //mv  settle time = 400us (400us for 1.8v, 3.3v 200us)
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_lv_settle_time = 2;    //lv  settle time = 150us
        //offset 0x50006024
        PMU_CTRL->pmu_soc_pmu_xtal1.bit.cfg_xtal_settle_time = 15;   //slow clk 10k, 1ms
        //offset 0x50006040
        PMU_CTRL->pmu_rvd0.bit.cfg_xtal_fast_time = 7;              //slow clk 10k, 0.5ms
        //offset 0x500060bc
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_pwrx_settle_time = 1;
    }
    else if (rco_select == RCO1M_SELECT)
    {
        PMU_CTRL->soc_pmu_rco1m.bit.tune_fine_rco_1m = 70;
        PMU_CTRL->soc_pmu_rco1m.bit.tune_coarse_rco_1m = 11;
        PMU_CTRL->soc_pmu_rco1m.bit.pw_rco_1m = 1;
        PMU_CTRL->soc_pmu_rco1m.bit.test_rco_1m = 0;
        PMU_CTRL->soc_pmu_rco1m.bit.en_rco_1m = 1;      //1:rco1m enable, 0 : rco1m disable
        //rco1m
        if (get_peri_clk() == PERCLK_SEL_16M)
        {
            RCO1M_CAL->cal1m_cfg0.bit.cfg_cal_target  = 0x115c7; //hper_clk = 16mhz is 115c7'h"
        }
        else
        {
            RCO1M_CAL->cal1m_cfg0.bit.cfg_cal_target = 0x22b8e; //per_clk = 32mhz is 0x22b8e'
        }

        //per_clk = 16mhz is 115c7'h
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_lock_err = 0x20;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_avg_coarse = 1;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_avg_fine = 2;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_avg_lock = 2;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_dly = 0;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_fine_gain = 10;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_lock_gain = 10;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_track_en = 1;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_skip_coarse = 1;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_cal_bound_mode = 0;
        RCO1M_CAL->cal1m_cfg1.bit.cfg_tune_rco_sel = 1;
        RCO1M_CAL->cal1m_cfg1.bit.en_ck_cal = 1;
        RCO1M_CAL->cal1m_cfg0.bit.cfg_cal_en = 1;
    }

    delay_ms(2);
}

void set_slow_clock_source(uint32_t mode)
{
    if (mode != 3)
    {
        return;
    }

    SYSCTRL->sys_clk_ctrl2.bit.en_ck_div_32k = 1;
    SYSCTRL->sys_clk_ctrl.bit.slow_clk_sel = 2;
    SYSCTRL->sys_clk_ctrl.bit.slow_clk_sel = mode;
    delay_us(100);
    SYSCTRL->sys_clk_ctrl2.bit.en_ck_div_32k = 0;

}

void set_ext32k_pin(uint32_t pin_number)
{
    SYSCTRL->sys_clk_ctrl1.bit.ext_slow_clk_sel = pin_number;
    SYSCTRL->sys_clk_ctrl1.bit.ext_slow_clk_en = 1;
    return;
}


