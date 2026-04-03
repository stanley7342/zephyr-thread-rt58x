/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            lpm.c
 * \brief           low power mode driver file
 */

/*
 * This file is part of library_name.
 * Author: ives.lee
 */

#include "comm_subsystem_drv.h"
#include "mcu.h"
#include "rf_mcu_ahb.h"
#include "lpm.h"

static volatile uint32_t low_power_mask_status;
static volatile uint32_t comm_subsystem_wakeup_mask_status;
static volatile low_power_level_cfg_t low_power_level;
static volatile low_power_wakeup_cfg_t low_power_wakeup;
static volatile uint32_t low_power_wakeup_update;
static volatile uint32_t low_power_wakeup_uart;
static volatile uint32_t low_power_wakeup_gpio;

#define LPM_SRAM0_RETAIN 0x1E

/**
 * \brief            low power mode intinal
 */
void lpm_init(void) {
    low_power_mask_status = LOW_POWER_NO_MASK;
    comm_subsystem_wakeup_mask_status = COMM_SUBSYS_WAKEUP_NO_MASK;
    low_power_level = LOW_POWER_LEVEL_NORMAL;
    low_power_wakeup = LOW_POWER_WAKEUP_NULL;
    low_power_wakeup_update = 0;
    low_power_wakeup_uart = 0;
    low_power_wakeup_gpio = 0;
}
/**
 * \brief            set low power status mask 
 */
void lpm_low_power_mask(uint32_t mask) { low_power_mask_status |= mask; }
/**
 * \brief            set low power status unmask 
 */
void lpm_low_power_unmask(uint32_t unmask) {
    low_power_mask_status &= (~unmask);
}
/**
 * \brief            get low power mask status  
 */
uint32_t lpm_get_low_power_mask_status(void) { return low_power_mask_status; }
/**
 * \brief            set low power sub system wake up status mask   
 */
void lpm_comm_subsystem_wakeup_mask(uint32_t mask) {
    comm_subsystem_wakeup_mask_status |= mask;
}
/**
 * \brief            set low power sub system wake up status unmask   
 */
void lpm_comm_subsystem_wakeup_unmask(uint32_t unmask) {
    comm_subsystem_wakeup_mask_status &= (~unmask);
}
/**
 * \brief            check sub system ready status   
 */
void lpm_comm_subsystem_check_system_ready(void) {
    uint32_t status;
    do {
        status = COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO;
    } while (((status & 0x01) != 1));
}
/**
 * \brief            wait sub system disable 32k done   
 */
void lpm_comm_subsystem_disable_wait_32k_done(void) {
    uint8_t reg_buf[4];

    RfMcu_MemoryGetAhb(SUBSYSTEM_CFG_WAIT_32K_DONE, reg_buf, 4);
    /* Clear Communication System Register 0x418 Bit26 */
    reg_buf[3] &= ~SUBSYSTEM_CFG_WAIT_32K_DONE_DISABLE; 
    RfMcu_MemorySetAhb(SUBSYSTEM_CFG_WAIT_32K_DONE, reg_buf, 4);
}
/**
 * \brief            wait sub system disable ldo mode   
 */
void lpm_comm_subsystem_disable_ldo_mode(void) {
    uint8_t reg_buf[4];
    RfMcu_MemoryGetAhb(SUBSYSTEM_CFG_PMU_MODE, reg_buf, 4);
    reg_buf[0] &= ~SUBSYSTEM_CFG_LDO_MODE_DISABLE;
    RfMcu_MemorySetAhb(SUBSYSTEM_CFG_PMU_MODE, reg_buf, 4);
}
/**
 * \brief           lpm_comm_subsystem_check_system_deepsleep
 */
void lpm_comm_subsystem_check_system_deepsleep(void) {
    uint32_t status;
    do {
        status = ((COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO & 0x6) >> 1);
    } while ((status != 1UL));
}
/**
 * \brief           set low power sub system wake up status unmask
 */
uint32_t lpm_get_comm_subsystem_wakeup_mask_status(void) {
    return comm_subsystem_wakeup_mask_status;
}
/**
 * \brief            low power mode subsystem srame deep sleep intintal
 */
void lpm_comm_subsystem_sram_deep_sleep_init(void) {
    /* SRAM shut-down control in Low Power Mode, set the corresponding bit0~bit4 to shut-down SRAM0~SRAM4 in Sleep/DeepSleep Mode */
    Comm_Subsystem_Sram_Deep_Sleep_Init(); 
}
/**
 * \brief            set low power mode level
 */
void lpm_set_low_power_level(low_power_level_cfg_t low_power_level_cfg) {
    low_power_level = low_power_level_cfg;
    low_power_wakeup_update = 1;
}
/**
 * \brief            set low power mode level
 */
uint32_t lpm_get_low_power_level(void) {
    return low_power_level;
}
/**
 * \brief            enable low power mode wake up
 */
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
    } else {
        low_power_wakeup |= wakeup_source;
    }

    low_power_wakeup_update = 1;
}
/**
 * \brief            disable low power mode wake up
 */
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
    } else {
        low_power_wakeup &= ~wakeup_source;
    }

    low_power_wakeup_update = 1;
}
void lpm_comm_subsystem_power_status_check(commumication_subsystem_pwr_mode_cfg_t mode) {
    uint32_t status;

    do {
        status = ((COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_INFO & 0x6) >> 1);
        
    } while ((status != mode));
}

/**
 * \brief            lpm_set_subsystem_low_power
 */
void lpm_sub_system_low_power_mode(commumication_subsystem_pwr_mode_cfg_t mode) {

    if (mode == COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP) {
        /* set platform subsystem entering sleep mode*/
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_HOSTMODE;
        COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_SLEEP;
    } else if (mode == COMMUMICATION_SUBSYSTEM_PWR_STATE_DEEP_SLEEP) {
        /* set platform subsystem entering deep sleep mode*/
        if( (COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_TX_INFO >> 16) & 0xFFF) {
            /* have fw */
        } else {
            lpm_sub_system_low_power_mode(COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP);
        }

        lpm_comm_subsystem_disable_wait_32k_done();
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
 * \brief            lpm_set_platform_low_power_wakeup
 */
void lpm_set_platform_low_power_wakeup(
    low_power_platform_enter_mode_cfg_t platform_low_power_mode) {
    do {
        if (low_power_wakeup_update == 0) {
            break;
        }

        low_power_wakeup_update = 0;

        if (platform_low_power_mode == LOW_POWER_PLATFORM_ENTER_SLEEP) {
            /* HCLK sleep selection in Sleep */
            if ((low_power_wakeup & LOW_POWER_WAKEUP_RTC_TIMER)
                || (low_power_wakeup & LOW_POWER_WAKEUP_COMPARATOR)
                || (low_power_wakeup & LOW_POWER_WAKEUP_SLOW_TIMER)
                || (low_power_wakeup_gpio != 0)) {
                SYSCTRL->sys_clk_ctrl |= HCLK_SLEEP_RUN_IN_32K;
            } else {
                SYSCTRL->sys_clk_ctrl &= (~HCLK_SLEEP_RUN_IN_32K);
            }

            /* AUX comparator enable selection in Sleep */
            if (low_power_wakeup & LOW_POWER_WAKEUP_COMPARATOR) {
                PMU->pmu_comp0.bit.aux_comp_en_sp = 1;
            } else {
                PMU->pmu_comp0.bit.aux_comp_en_sp = 0;
            }

            /* UART0 sleep wake enable selection in Sleep */
            if (low_power_wakeup_uart & (1 << 0)) {
                SYSCTRL->sys_lowpower_ctrl |= UART0_SLEEP_WAKE_EN_MASK;
            } else {
                SYSCTRL->sys_lowpower_ctrl &= (~UART0_SLEEP_WAKE_EN_MASK);
            }

            /* UART1 sleep wake enable selection in Sleep */
            if (low_power_wakeup_uart & (1 << 1)) {
                SYSCTRL->sys_lowpower_ctrl |= UART1_SLEEP_WAKE_EN_MASK;
            } else {
                SYSCTRL->sys_lowpower_ctrl &= (~UART1_SLEEP_WAKE_EN_MASK);
            }

            /* UART2 sleep wake enable selection in Sleep */
            if (low_power_wakeup_uart & (1 << 2)) {
                SYSCTRL->sys_lowpower_ctrl |= UART2_SLEEP_WAKE_EN_MASK;
            } else {
                SYSCTRL->sys_lowpower_ctrl &= (~UART2_SLEEP_WAKE_EN_MASK);
            }
        } else if (platform_low_power_mode
                   == LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP) {
            if (low_power_wakeup & LOW_POWER_WAKEUP_RTC_TIMER) {
                /* HCLK sleep selection in Sleep, HCLK is forced to 32KHz in Sleep */
                SYSCTRL->sys_clk_ctrl |= HCLK_SLEEP_RUN_IN_32K;
                /* Enable RTC PCLK in Sleep, this allows RTC wakeup in DeepSleep */
                SYSCTRL->sys_clk_ctrl |= RTC_PCLK_DS_MASK;
                /* Enable 32KHz in DeepSleep mode, this allows RTC to work in DeepSleep */
                PMU->pmu_pwr_ctrl.bit.cfg_ds_32k_off = 0;
            } else {
                /* HCLK sleep selection in Sleep, HCLK is disabled in Sleep */
                SYSCTRL->sys_clk_ctrl &= (~HCLK_SLEEP_RUN_IN_32K);
                /* Disable RTC PCLK in Sleep */
                SYSCTRL->sys_clk_ctrl &= (~RTC_PCLK_DS_MASK);
                /* Disable 32KHz in DeepSleep mode */
                PMU->pmu_pwr_ctrl.bit.cfg_ds_32k_off = 1;
            }
            /* Set the corresponding bits to enable the wakeup of GPIOx in Deep Sleep */
            set_deepsleep_wakeup_pin(low_power_wakeup_gpio); 

            if (low_power_wakeup & LOW_POWER_WAKEUP_COMPARATOR) {
                /* AUX comparator enable selection in Sleep, AUX comparator enable in DeepSleep */
                PMU->pmu_comp0.bit.aux_comp_en_ds = 1;
                /* AUX comparator wakeup enable selection in Sleep, AUX comparator wakeup enable in DeepSleep */
                PMU->pmu_comp0.bit.aux_comp_ds_wakeup_en = 1;
            } else {
                /* AUX comparator enable selection in Sleep, AUX comparator disable in DeepSleep */
                PMU->pmu_comp0.bit.aux_comp_en_ds = 0;
                /* AUX comparator wakeup enable selection in Sleep, AUX comparator wakeup disable in DeepSleep */
                PMU->pmu_comp0.bit.aux_comp_ds_wakeup_en = 0;
            }
        }

    } while (0);
}
/**
 * \brief            set gpio deep sleep wake up intert
 */
void lpm_set_gpio_deepsleep_wakeup_invert(uint32_t value) {
    /* Set the corresponding bit0~bit31 to invert the GPIO0~GPIO31 for wakeup in DeepSleep Mode */
    set_deepsleep_wakeup_invert(value); 
}
/**
 * \brief            set comparator  deep sleep wake up intert
 */
void lpm_set_comparator_deepsleep_wakeup_invert(uint32_t value) {
    /* Invert the Aux Comparator Output for wakeup in DeepSleep Mode */
    PMU->pmu_comp0.bit.aux_comp_ds_inv = value; 
}
/**
 * \brief            set sram normal shutdown
 */
void lpm_set_sram_normal_shutdown(uint32_t value) {
    /* SRAM shut-down control in Normal Mode, set the corresponding bit0~bit4 to shut-down SRAM0~SRAM4 in Normal Mode */
    set_sram_shutdown_normal(value);
}
/**
 * \brief            set sram sleep/deepsleep shutdown
 */
void lpm_set_sram_sleep_deepsleep_shutdown(uint32_t value) {
    /* SRAM shut-down control in Low Power Mode, set the corresponding bit0~bit4 to shut-down SRAM0~SRAM4 in Sleep/DeepSleep Mode */
    set_sram_shutdown_sleep(value); 
}
/**
 * \brief            enter low power mode
 */
void lpm_enter_low_power_mode(void) {
    pmu_dcdc_ctrl1_t pmu_cdcd_ctrl1_buf;

    if (low_power_mask_status == LOW_POWER_NO_MASK) {
        pmu_cdcd_ctrl1_buf.reg = PMU->pmu_dcdc_ctrl1.reg;

        PMU->pmu_dcdc_ctrl1.bit.dcdc_en_comp_light = 0;
        PMU->pmu_dcdc_ctrl1.bit.dcdc_mg_light = 0;

        if (low_power_level == LPM_DEEP_SLEEP) {
            /* enable host mode */
            COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_HOSTMODE;
            /* communication system host control reset */
            COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_RESET; 
            lpm_comm_subsystem_check_system_ready();
            lpm_comm_subsystem_disable_wait_32k_done();
            /* communication system enter deep sleep mode */
            COMM_SUBSYSTEM_AHB->COMM_SUBSYSTEM_HOST |= COMMUMICATION_SUBSYSTEM_DEEPSLEEP;
            lpm_comm_subsystem_check_system_deepsleep();
            lpm_set_sram_sleep_deepsleep_shutdown(LPM_SRAM0_RETAIN);
            sys_set_retention_reg(6, 0);
            /* set platform system wakeup source when entering deep sleep mode*/
            lpm_set_platform_low_power_wakeup(LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP); 
            /* platform system enter deep sleep mode */
            SYSCTRL->power_state_ctrl = LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP;
            __WFI();
        } else if (low_power_level == LPM_SLEEP) {
            /* set platform system wakeup source when entering sleep mode*/
            lpm_set_platform_low_power_wakeup(LOW_POWER_PLATFORM_ENTER_SLEEP); 
            /* platform system enter sleep mode */
            SYSCTRL->power_state_ctrl = LOW_POWER_PLATFORM_ENTER_SLEEP;
            __WFI();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
        }

        PMU->pmu_dcdc_ctrl1.bit.dcdc_en_comp_light =
            pmu_cdcd_ctrl1_buf.bit.dcdc_en_comp_light;
        PMU->pmu_dcdc_ctrl1.bit.dcdc_mg_light =
            pmu_cdcd_ctrl1_buf.bit.dcdc_mg_light;
    }
}
