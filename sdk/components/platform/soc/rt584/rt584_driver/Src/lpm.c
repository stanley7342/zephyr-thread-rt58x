/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            lpm.c
 * \brief           low power mode driver 
 */
/*
 * This file is part of library_name.
 * Author: ives.lee
 */


#include "mcu.h"
#include "lpm.h"
#include "sysctrl.h"
#include "gpio.h"
#include "aux_comp.h"
#include "dpd.h"
#include "dwt.h"

static volatile  uint32_t low_power_mask_status = LOW_POWER_NO_MASK;
static volatile  uint32_t comm_subsystem_wakeup_mask_status = COMM_SUBSYS_WAKEUP_NO_MASK;
static volatile  low_power_level_cfg_t low_power_level = LOW_POWER_LEVEL_NORMAL;
static volatile  low_power_wakeup_cfg_t low_power_wakeup = LOW_POWER_WAKEUP_NULL;
static volatile  uint32_t low_power_wakeup_update = 0;
static volatile  uint32_t low_power_wakeup_uart = 0;
static volatile  uint32_t low_power_wakeup_gpio = 0;
static volatile  uint32_t low_power_timer_pwm = 0;
static volatile  uint32_t low_power_aux_level = 0;
static volatile  uint32_t low_power_bod_level = 0;

#define LPM_SRAM0_RETAIN 0x1F

void lpm_init() {
    low_power_mask_status = LOW_POWER_NO_MASK;
    comm_subsystem_wakeup_mask_status = COMM_SUBSYS_WAKEUP_NO_MASK;
    low_power_level = LOW_POWER_LEVEL_NORMAL;
    low_power_wakeup = LOW_POWER_WAKEUP_NULL;
    low_power_wakeup_update = 0;
    low_power_wakeup_uart = 0;
    low_power_wakeup_gpio = 0;
    low_power_timer_pwm = 0;
    low_power_aux_level = 0;
    low_power_bod_level = 0;
}

void lpm_low_power_mask(uint32_t mask) {
    low_power_mask_status |= mask;
}

void lpm_low_power_unmask(uint32_t unmask) {
    low_power_mask_status &= (~unmask);
}

uint32_t lpm_get_low_power_mask_status(void) {
    return low_power_mask_status;
}

uint32_t lpm_get_low_power_level(void) {
    return low_power_level;
}

void lpm_comm_subsystem_wakeup_mask(uint32_t mask) {
    comm_subsystem_wakeup_mask_status |= mask;
}

void lpm_comm_subsystem_wakeup_unmask(uint32_t unmask) {
    comm_subsystem_wakeup_mask_status &= (~unmask);
}

void lpm_comm_subsystem_check_system_ready(void) {
    uint32_t status;
    do {
        status = COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO;

    } while (((status & 0x01) != 1));
}

void lpm_comm_subsystem_disable_wait_32k_done(void) {
    //Reserved
}
static uint32_t lpm_comm_subsystem_power_status(void)
{
    return  ((COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO & 0x6) >> 1);
}
/**
* \brief            Check communication subsystem slee mode
*/
void lpm_comm_subsystem_check_system_deepsleep(void) {
    uint32_t status;
    do {
        status = ((COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO & 0x6) >> 1);
    } while ((status != 1UL));
}

/**
* \brief            Check communication subsystem slee mode
*/
void lpm_comm_subsystem_check_sleep_mode(uint32_t mode) {
    uint32_t status;

    do {
        status = ((COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO & 0x6) >> 1);
    } while ((status != mode));
}

uint32_t lpm_get_comm_subsystem_wakeup_mask_status(void) {
    return comm_subsystem_wakeup_mask_status;
}

void lpm_set_low_power_level(low_power_level_cfg_t low_power_level_cfg) {
    low_power_level = low_power_level_cfg;
    low_power_wakeup_update = 1;
}

void lpm_enable_low_power_wakeup(low_power_wakeup_cfg_t low_power_wakeup_cfg) {
    uint32_t wakeup_source = 0;
    uint32_t wakeup_source_index = 0;

    wakeup_source = (low_power_wakeup_cfg & 0xFFFF);

    if (wakeup_source == LOW_POWER_WAKEUP_GPIO) {
        wakeup_source_index = (low_power_wakeup_cfg >> 16);
        low_power_wakeup_gpio |= (1 << wakeup_source_index);
    } else if (wakeup_source == LOW_POWER_WAKEUP_UART_RX) {
        wakeup_source_index = (low_power_wakeup_cfg >> 16);
        low_power_wakeup_uart |= (1 << wakeup_source_index);
    } else if (wakeup_source == LOW_POWER_WAKEUP_UART_DATA) {
        wakeup_source_index = (low_power_wakeup_cfg >> 16);
        low_power_wakeup_uart |= (1 << wakeup_source_index);
    } else {
        low_power_wakeup |= wakeup_source;
    }

    low_power_wakeup_update = 1;
}

void lpm_disable_low_power_wakeup(low_power_wakeup_cfg_t low_power_wakeup_cfg) {
    uint32_t wakeup_source = 0;
    uint32_t wakeup_source_index = 0;

    wakeup_source = (low_power_wakeup_cfg & 0xFFFF);

    if (wakeup_source == LOW_POWER_WAKEUP_GPIO) {
        wakeup_source_index = (low_power_wakeup_cfg >> 16);
        low_power_wakeup_gpio &= (~(1 << wakeup_source_index));
    } else if (wakeup_source == LOW_POWER_WAKEUP_UART_RX) {
        wakeup_source_index = (low_power_wakeup_cfg >> 16);
        low_power_wakeup_uart &= (~(1 << wakeup_source_index));
    } else if (wakeup_source == LOW_POWER_WAKEUP_UART_DATA) {
        wakeup_source_index = (low_power_wakeup_cfg >> 16);
        low_power_wakeup_uart &= (~(1 << wakeup_source_index));
    } else {
        low_power_wakeup &= ~wakeup_source;
    }

    low_power_wakeup_update = 1;
}

void lpm_enable_timer_pwm(void) {
    low_power_timer_pwm = 1;
}

void lpm_disable_timer_pwm(void) {
    low_power_timer_pwm = 0;
}

void lpm_enable_aux_level(void) {
    low_power_aux_level = 1;
}

void lpm_disable_aux_level(void) {
    low_power_aux_level = 0;
}

void lpm_enable_bod_level(void) {
    low_power_bod_level = 1;
}

void lpm_disable_bod_level(void) {
    low_power_bod_level = 0;
}

/**
* \brief            Set low power mode wake up source
*/
void lpm_set_platform_low_power_wakeup(low_power_platform_enter_mode_cfg_t platform_low_power_mode) {
    do {
        if (low_power_wakeup_update == 0) {
            break;
        }

        low_power_wakeup_update = 0;

        if (platform_low_power_mode == LOW_POWER_PLATFORM_ENTER_SLEEP) {
            lpm_set_peripheral_power_off(MCU_PERI2_PWR_OFF_SLEEP, ENABLE);
            lpm_set_peripheral_power_off(MCU_PERI3_PWR_OFF_SLEEP, ENABLE);
            /* UART0 sleep wake enable selection in Sleep */
            if (low_power_wakeup_uart & (1 << 0)) {
                UART0->wake_sleep_en = 1;
            } else if (low_power_wakeup_uart & (1 << 3)) {
                lpm_set_peripheral_power_off(MCU_PERI3_PWR_OFF_SLEEP, DISABLE);  //Disable Power off in sleep
                UART0->wake_sleep_en = 1;
            } else {
                UART0->wake_sleep_en = 0;
            }

            /* UART1 sleep wake enable selection in Sleep */
            if (low_power_wakeup_uart & (1 << 1) || low_power_wakeup_uart & (1 << 4)) {
                lpm_set_peripheral_power_off(MCU_PERI3_PWR_OFF_SLEEP, DISABLE);
                UART1->wake_sleep_en = 1;
            } else {
                UART1->wake_sleep_en = 0;
            }

            /* UART2 sleep wake enable selection in Sleep */
            if (low_power_wakeup_uart & (1 << 2) || low_power_wakeup_uart & (1 << 5)) {
               
             lpm_set_peripheral_power_off(MCU_PERI2_PWR_OFF_SLEEP, DISABLE);
             lpm_set_peripheral_power_off(MCU_PERI3_PWR_OFF_SLEEP, DISABLE);  //Disable Power off in sleep
                UART2->wake_sleep_en = 1;
            } else {
                UART2->wake_sleep_en = 0;
            }

            if ( low_power_timer_pwm == 1) {
                lpm_set_peripheral_power_off(MCU_PERI3_PWR_OFF_SLEEP, DISABLE);  //Disable Power off in sleep
            }
        } else if (platform_low_power_mode == LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP) {

            SYSCTRL->sram_lowpower_3.bit.cfg_ds_rco32k_off = 1;  //Disable RCO32K in Deepsleep Mode

            if (low_power_wakeup & LOW_POWER_WAKEUP_RTC_TIMER) {
                SYSCTRL->sram_lowpower_3.bit.cfg_ds_rco32k_off = 0;
                SYSCTRL->sys_clk_ctrl2.bit.en_ck32_rtc = 1;
            }

            if (low_power_wakeup & LOW_POWER_WAKEUP_AUX_COMP) {
                SYSCTRL->sram_lowpower_3.bit.cfg_ds_rco32k_off = 0;
                if ( low_power_aux_level == 1 ) {
                    SYSCTRL->sys_clk_ctrl2.bit.en_ck32_auxcomp = 0;
                } else {
                    SYSCTRL->sys_clk_ctrl2.bit.en_ck32_auxcomp = 1;
                }
                AUX_COMP->comp_dig_ctrl0.bit.comp_en_ds = 1;
            }

            if (low_power_wakeup & LOW_POWER_WAKEUP_BOD_COMP) {
                SYSCTRL->sram_lowpower_3.bit.cfg_ds_rco32k_off = 0;
                if ( low_power_bod_level == 1 ) {
                    SYSCTRL->sys_clk_ctrl2.bit.en_ck32_bodcomp = 0;
                } else {
                    SYSCTRL->sys_clk_ctrl2.bit.en_ck32_bodcomp = 1;
                }
                BOD_COMP->comp_dig_ctrl0.bit.comp_en_ds = 1;
            }
        } else if (platform_low_power_mode == LOW_POWER_PLATFORM_ENTER_POWER_DOWN_MODE) {
        }
    } while (0);
}

void lpm_set_sram_normal_shutdown(uint32_t value) {
    /* SRAM shut-down control in Normal Mode, set the corresponding bit0~bit4 to shut-down SRAM0~SRAM4 in Normal Mode */
    set_sram_shutdown_normal(value);
}

void lpm_set_sram_sleep_deepsleep_shutdown(uint32_t value) {
    if (low_power_level == LPM_SLEEP) {
        set_sram_shutdown_sleep(value);
    } else if (low_power_level == LPM_DEEP_SLEEP) {
        set_sram_shutdown_deepsleep(value);
    } else if (low_power_level == LPM_POWER_DOWN) {
        set_sram_shutdown_deepsleep(value);
    }
}

void lpm_set_peripheral_power_off(uint32_t peripwroffctlbit, uint32_t isenabled)
{
    set_peripheral_power_off(peripwroffctlbit, isenabled);
}
/**
* @brief disable sram in sleep and deep sleep mode
*/
void lpm_set_gpio_deepsleep_wakeup_invert(uint32_t value)
{
    set_deepsleep_wakeup_invert(value);                                /* Set the corresponding bit0~bit31 to invert the GPIO0~GPIO31 for wakeup in DeepSleep Mode */
}
/**
* @brief disable sram in sleep and deep sleep mode
*/
void lpm_set_gpio_deepsleep_wakeup_invert_ex(uint8_t num, uint32_t value)    /* Set the corresponding bit0~bit31 to invert the GPIO0~GPIO31 for wakeup in DeepSleep Mode */
{
    gpio_setup_deep_sleep_io(num, value);
}
/**
* @brief disable sram in sleep and deep sleep mode
*/
void lpm_set_gpio_powerdown_wakeup_invert_ex(uint8_t num, uint32_t value)    /* Set the corresponding bit0~bit31 to invert the GPIO0~GPIO31 for wakeup in DeepSleep Mode */
{
    gpio_setup_deep_powerdown_io(num, value);
}

void lpm_comm_subsystem_power_status_check(commumication_subsystem_pwr_mode_cfg_t mode) {
    uint32_t status;

    do {
        status = ((COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO & 0x6) >> 1);
    } while ((status != mode));
}

void lpm_sub_system_low_power_mode(commumication_subsystem_pwr_mode_cfg_t mode) {

    if (mode == COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP) {
        /* set platform subsystem entering sleep mode*/
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_HOSTMODE;
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_SLEEP;
    } else if (mode == COMMUMICATION_SUBSYSTEM_PWR_STATE_DEEP_SLEEP) {
        if( (COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_TX_INFO >> 16) & 0xFFF ) {
            //have fw
        } else {
            lpm_sub_system_low_power_mode(COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP);
        }

        /* check subsystem enter sleep mode*/
        if (lpm_comm_subsystem_power_status() == COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP ||
                lpm_comm_subsystem_power_status() == COMMUMICATION_SUBSYSTEM_PWR_STATE_DEEP_SLEEP)
        {
            //sub system deep sleep mode;
        }
        else
        {
            lpm_comm_subsystem_power_status_check(COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP);
        }
        /* set platform subsystem entering deep sleep mode*/
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_HOSTMODE;
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_DEEPSLEEP;
    } else {
        /* set platform subsystem entering normal mode*/
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_HOSTMODE;
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_RESET;
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_DIS_HOSTMODE;
        lpm_comm_subsystem_check_system_ready();
    }

    lpm_comm_subsystem_power_status_check(mode);
}
/**
* \brief set mcu deep sleep ski isp fas boot
*/
void lpm_deep_sleep_wakeup_fastboot()
{
    dpd_set_deepsleep_wakeup_fast_boot();
}

void lpm_deep_sleep_set_rco_settle_time()
{
    if (sys_slow_clk_mode() != RCO32K_MODE)
    {

       
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_mv_settle_time = 12;   //mv  settle time = 400us (400us for 1.8v, 3.3v 200us)
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_lv_settle_time = 5;    //lv  settle time = 62.5us
        
        PMU_CTRL->pmu_soc_pmu_xtal1.bit.cfg_xtal_settle_time = 31;  //1ms
        
        PMU_CTRL->pmu_rvd0.bit.cfg_xtal_fast_time = 15;             //0.5ms
        
        PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_pwrx_settle_time = 2;   
    }
}


void lpm_enter_low_power_mode(void) {

    if (low_power_mask_status == LOW_POWER_NO_MASK) {
        if (low_power_level == LPM_SLEEP) {
            /* set platform system wakeup source when entering sleep mode*/
            lpm_set_platform_low_power_wakeup(LOW_POWER_PLATFORM_ENTER_SLEEP);
            /* platform system enter sleep mode */
            SYSCTRL->sys_power_state.bit.cfg_set_lowpower = LOW_POWER_PLATFORM_ENTER_SLEEP;
            __WFI();
        } else if (low_power_level == LPM_DEEP_SLEEP) {
        	
        	lpm_deep_sleep_set_rco_settle_time();
        	lpm_deep_sleep_wakeup_fastboot();
            /* set platform system wakeup source when entering deep sleep mode*/
            lpm_set_platform_low_power_wakeup(LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP);
            /* platform system enter deep sleep mode */
            while(1)
            {
              SYSCTRL->sys_power_state.bit.cfg_set_lowpower = LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP;	
              __WFI();    
            }
        } else if (low_power_level == LPM_POWER_DOWN) {
            /* set platform system wakeup source when entering power down mode*/
            lpm_set_platform_low_power_wakeup(LOW_POWER_PLATFORM_ENTER_POWER_DOWN_MODE);
            /* platform system enter power down mode */
            SYSCTRL->sys_power_state.bit.cfg_set_lowpower = LOW_POWER_PLATFORM_ENTER_POWER_DOWN_MODE;
            __WFI();
        }
        SYSCTRL->sys_power_state.bit.cfg_set_lowpower  = LOW_POWER_PLATFORM_NORMAL;
    }
}
