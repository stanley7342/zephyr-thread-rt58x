/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            system_mcu.c
 * \brief           system mcu file
 */
/*
 * This file is part of library_name.
 * Author:
 */

#include <stdint.h>
#include "mcu.h"

#include "mp_sector.h"

/**
 * \brief           Register bit write structure.
 */
typedef struct {
    uint32_t reg_addr;                          /*!< Register address to be written*/
    uint8_t start_bit;                          /*!< Register start bit to be written*/
    uint8_t bit_num;                            /*!< Register bit number to be written*/
    uint32_t reg_value;                         /*!< The value of register to be written*/
} reg_bit_write_t;


#ifndef SET_SYS_CLK
#define SET_SYS_CLK SYS_CLK_48MHZ
#endif

#if (SET_SYS_CLK == SYS_CLK_32MHZ)
#define XTAL (32000000UL) /* Oscillator frequency               */
#elif (SET_SYS_CLK == SYS_CLK_48MHZ)
#define XTAL (48000000UL) /* Oscillator frequency               */
#elif (SET_SYS_CLK == SYS_CLK_64MHZ)
#define XTAL (64000000UL) /* Oscillator frequency               */
#endif

uint32_t SystemFrequency = XTAL; /*!< System Clock Frequency (Core Clock)  */
uint32_t SystemCoreClock = XTAL; /*!< Processor Clock Frequency            */

#if (SET_PMU_MODE == PMU_LDO_MODE)
const reg_bit_write_t pmu_ap_init_table_ldo[] = {
    {0x402000C4, 24, 1, 0},
    {0x402000C4, 25, 1, 0},
    //    { 0x402000B0, 26, 1, 0 },
    {0x402000B0, 26, 1, 1},
};

const reg_bit_write_t pmu_mp_init_table_ldo[] = {
    {0x40200090, 16, 4, 10}, {0x40200090, 20, 4, 10}, {0x40200090, 24, 4, 10},
    {0x40200094, 0, 4, 0},   {0x40200094, 4, 4, 0},   {0x40200094, 8, 4, 0},
    {0x40200098, 0, 4, 7},   {0x40200098, 4, 4, 7},   {0x40200098, 8, 4, 7},
    {0x40200098, 16, 3, 0},  {0x40200098, 20, 3, 0},  {0x40200098, 24, 3, 0},
};

#elif (SET_PMU_MODE == PMU_DCDC_MODE)
const reg_bit_write_t pmu_ap_init_table_dcdc[] = {
    {0x402000C4, 24, 1, 0},
    {0x402000C4, 25, 1, 0},
    //    { 0x402000B0, 26, 1, 0 },
    {0x402000B0, 26, 1, 1},
};

const reg_bit_write_t pmu_mp_init_table_dcdc[] = {
    {0x40200090, 0, 4, 10},  {0x40200090, 4, 4, 10},  {0x40200090, 8, 4, 10},
    {0x40200090, 16, 4, 10}, {0x40200090, 20, 4, 10}, {0x40200090, 24, 4, 10},
    {0x40200094, 0, 4, 0},   {0x40200094, 4, 4, 0},   {0x40200094, 8, 4, 0},
    {0x40200098, 0, 4, 7},   {0x40200098, 4, 4, 7},   {0x40200098, 8, 4, 7},
    {0x40200098, 16, 3, 0},  {0x40200098, 20, 3, 0},  {0x40200098, 24, 3, 0},
};

#endif

void systemcoreclockupdate(void) /* Get Core Clock Frequency */
{
    SystemCoreClock = XTAL;
}

void systemfrequencyupdate(void) /* Update System clock Frequency */
{
    sys_clk_sel_t sys_clk_mode;

    sys_clk_mode = get_ahb_system_clk();

    if (sys_clk_mode == SYS_CLK_32MHZ) {
        SystemFrequency = 32000000;
    } else if (sys_clk_mode == SYS_CLK_48MHZ) {
        SystemFrequency = 48000000;
    } else if (sys_clk_mode == SYS_CLK_64MHZ) {
        SystemFrequency = 64000000;
    } else {
        SystemFrequency = 32000000;
    }
}

#if ((CHIP_VERSION == RT58X_MPA) || (CHIP_VERSION == RT58X_MPB))
void reg_bit_write(reg_bit_write_t reg_write_item) {
    uint32_t bit_max_value;
    uint32_t reg_value_buf;

    bit_max_value = ((1 << reg_write_item.bit_num) - 1);

    reg_value_buf = *((uint32_t*)reg_write_item.reg_addr);
    reg_value_buf &= (~(bit_max_value << reg_write_item.start_bit));
    reg_value_buf |= ((reg_write_item.reg_value & bit_max_value)
                      << reg_write_item.start_bit);
    *((uint32_t*)(reg_write_item.reg_addr)) = reg_value_buf;
}

void systempmuupdatedig(
    void) { /* Update PMU settings for digital configuration */
    PMU->pmu_xtal.bit.cfg_xtal_settle_time = 0x3F;
    PMU->pmu_xtal.bit.cfg_xtal_cap_sel = 0x191;
    SYSCTRL->sys_lowpower_ctrl = (SYSCTRL->sys_lowpower_ctrl
                                  & ~LOWPOWER_SRAM_DS_AUTO_MASK)
                                 | LOWPOWER_SRAM_DS_AUTO_ENABLE;
    PMU->pmu_xtal.bit.cfg_bypass_xtal_settle = 1;
    PMU->pmu_clk_ctrl.bit.cfg_chip_en_auto = 1;
    PMU->pmu_pwr_ctrl.bit.cfg_byp_xbuf_ldo = 1;

    SYSCTRL->gpio_aio_ctrl = ((SYSCTRL->gpio_aio_ctrl & ~FLASH_DRV_SEL_MASK)
                              | FLASH_DRV_SEL_SET);
}

void SystemPmuSetMode(pmu_mode_cfg_t pmu_mode) {
    if (pmu_mode == PMU_MODE_DCDC) {
        PMU->pmu_en_ctrl.bit.en_lldo_nm = 1;
        PMU->pmu_en_ctrl.bit.en_dcdc_nm = 1;
        PMU->pmu_en_ctrl.bit.en_lldo_nm = 0;
    } else if (pmu_mode == PMU_MODE_LDO) {
        PMU->pmu_en_ctrl.bit.en_lldo_nm = 1;
        PMU->pmu_en_ctrl.bit.en_dcdc_nm = 1;
        PMU->pmu_en_ctrl.bit.en_dcdc_nm = 0;
    }
}

#if (SET_PMU_MODE == PMU_LDO_MODE)
void systempmuupdateldo(void) { /* Update PMU settings in LDO mode */
    uint32_t reg_num = 0;
    reg_bit_write_t reg_write_item;
    uint32_t i = 0;

    pmu_other_ctrl_t pmu_other_ctrl_buf;
    pmu_en_ctrl_t pmu_en_ctrl_buf;

    pmu_other_ctrl_buf = PMU->pmu_other_ctrl;
    pmu_other_ctrl_buf.bit.lldo_sin = 2;
    pmu_other_ctrl_buf.bit.lldo_lout = 0;
    pmu_other_ctrl_buf.bit.lldo_ioc = 4;
    pmu_other_ctrl_buf.bit.ioldo_sin = 2;
    pmu_other_ctrl_buf.bit.ioldo_lout = 1;
    pmu_other_ctrl_buf.bit.ioldo_ioc = 6;
    pmu_other_ctrl_buf.bit.ioldo_ret_sin = 3;
    pmu_other_ctrl_buf.bit.ioldo_ret_lout = 0;
    PMU->pmu_other_ctrl = pmu_other_ctrl_buf;

    pmu_en_ctrl_buf = PMU->pmu_en_ctrl;
    pmu_en_ctrl_buf.bit.uvh_disable = 1;
    pmu_en_ctrl_buf.bit.en_bg1_nm = 0;
    PMU->pmu_en_ctrl = pmu_en_ctrl_buf;

    PMU->pmu_bod_sel.bit.por_vth = 3;
    PMU->pmu_bbpll.bit.uvl_vth = 3;

    PMU->pmu_rco32k.bit.rn_32k = 2047;
    PMU->pmu_rco32k.bit.pw_32k = 3;

    reg_num = sizeof(pmu_ap_init_table_ldo) / sizeof(reg_bit_write_t);

    for (i = 0; i < reg_num; i++) {
        reg_write_item = pmu_ap_init_table_ldo[i];
        Reg_Bit_Write(reg_write_item);
    }

    reg_num = sizeof(pmu_mp_init_table_ldo) / sizeof(reg_bit_write_t);

    for (i = 0; i < reg_num; i++) {
        reg_write_item = pmu_mp_init_table_ldo[i];
        Reg_Bit_Write(reg_write_item);
    }
}

#elif (SET_PMU_MODE == PMU_DCDC_MODE)
void systempmuupdatedcdc(void) { /* Update PMU settings in DCDC mode */
    uint32_t reg_num = 0;
    reg_bit_write_t reg_write_item;
    uint32_t i = 0;
    uint32_t chip_version = (get_chip_version() >> 4)
                            & 0x0F; //bit4~bit7 chip version

    pmu_other_ctrl_t pmu_other_ctrl_buf;
    pmu_en_ctrl_t pmu_en_ctrl_buf;
    pmu_dcdc_ctrl0_t pmu_dcdc_ctrl0_buf;
    pmu_dcdc_ctrl1_t pmu_dcdc_ctrl1_buf;

    pmu_other_ctrl_buf = PMU->pmu_other_ctrl;
    pmu_other_ctrl_buf.bit.lldo_sin = 2;
    pmu_other_ctrl_buf.bit.lldo_lout = 0;
    pmu_other_ctrl_buf.bit.lldo_ioc = 5;
    pmu_other_ctrl_buf.bit.ioldo_sin = 2;
    pmu_other_ctrl_buf.bit.ioldo_lout = 0;
    pmu_other_ctrl_buf.bit.ioldo_ioc = 6;
    pmu_other_ctrl_buf.bit.ioldo_ret_sin = 3;
    pmu_other_ctrl_buf.bit.ioldo_ret_lout = 0;
    //===
    pmu_other_ctrl_buf.bit.dcdc_sw_heavy = 0;
    pmu_other_ctrl_buf.bit.dcdc_auto_heavy = 1;

    PMU->pmu_other_ctrl = pmu_other_ctrl_buf;

    pmu_en_ctrl_buf = PMU->pmu_en_ctrl;
    pmu_en_ctrl_buf.bit.uvh_disable = 1;
    pmu_en_ctrl_buf.bit.en_bg1_nm = 0;
    PMU->pmu_en_ctrl = pmu_en_ctrl_buf;

    PMU->pmu_bod_sel.bit.por_vth = 0;

    if (chip_version == RT58X_MPB) {
        PMU->pmu_bbpll.bit.bbpll_hi = 1;
    } else {
        PMU->pmu_bbpll.bit.bbpll_hi = 3;
    }

    if (flash_get_deviceinfo() == RT582_FLASH_TYPE) {
        PMU->pmu_bod_sel.bit.apmu_test = 1;
    } else {
        PMU->pmu_bod_sel.bit.apmu_test = 3;
    }

    PMU->pmu_bbpll.bit.uvl_vth = 3;

#if (CHIP_VERSION == RT58X_MPA)
    PMU->pmu_rco32k.bit.rn_32k = 2047;
#elif (CHIP_VERSION == RT58X_MPB)
    PMU->pmu_rco32k.bit.rn_32k = 0;
#endif
    PMU->pmu_rco32k.bit.c_32k = 0;
#if (CHIP_VERSION == RT58X_MPA)
    PMU->pmu_rco32k.bit.pw_32k = 2;
#elif (CHIP_VERSION == RT58X_MPB)
    PMU->pmu_rco32k.bit.pw_32k = 0;
#endif

    pmu_dcdc_ctrl0_buf = PMU->pmu_dcdc_ctrl0;
    pmu_dcdc_ctrl0_buf.bit.dcdc_ppower_heavy = 0;
    pmu_dcdc_ctrl0_buf.bit.dcdc_en_comp_heavy = 1;
#if (CHIP_VERSION == RT58X_MPA)
    pmu_dcdc_ctrl0_buf.bit.dcdc_npower_heavy = 7;
#elif (CHIP_VERSION == RT58X_MPB)
    pmu_dcdc_ctrl0_buf.bit.dcdc_npower_heavy = 0;
#endif
    pmu_dcdc_ctrl0_buf.bit.dcdc_en_zcd_heavy = 0;
#if (CHIP_VERSION == RT58X_MPA)
    pmu_dcdc_ctrl0_buf.bit.dcdc_pdrive_heavy = 5;
#elif (CHIP_VERSION == RT58X_MPB)
    pmu_dcdc_ctrl0_buf.bit.dcdc_pdrive_heavy = 6;
#endif
    pmu_dcdc_ctrl0_buf.bit.dcdc_mg_heavy = 1;
    pmu_dcdc_ctrl0_buf.bit.dcdc_ndrive_heavy = 5;
    pmu_dcdc_ctrl0_buf.bit.dcdc_cm_heavy = 1;
    pmu_dcdc_ctrl0_buf.bit.dcdc_pw_heavy = 0;
    pmu_dcdc_ctrl0_buf.bit.dcdc_c_sc_heavy = 0;
    pmu_dcdc_ctrl0_buf.bit.dcdc_c_hg_heavy = 1;
#if (CHIP_VERSION == RT58X_MPA)
    pmu_dcdc_ctrl0_buf.bit.dcdc_pwmf_heavy = 8;
#elif (CHIP_VERSION == RT58X_MPB)
    pmu_dcdc_ctrl0_buf.bit.dcdc_pwmf_heavy = 7;
#endif
    pmu_dcdc_ctrl0_buf.bit.dcdc_os_pn_heavy = 1;
#if (CHIP_VERSION == RT58X_MPA)
    pmu_dcdc_ctrl0_buf.bit.dcdc_os_heavy = 3;
#elif (CHIP_VERSION == RT58X_MPB)
    pmu_dcdc_ctrl0_buf.bit.dcdc_os_heavy = 1;
#endif
    pmu_dcdc_ctrl0_buf.bit.dcdc_hg_heavy = 3;
#if (CHIP_VERSION == RT58X_MPA)
    pmu_dcdc_ctrl0_buf.bit.dcdc_dly_heavy = 2;
#elif (CHIP_VERSION == RT58X_MPB)
    pmu_dcdc_ctrl0_buf.bit.dcdc_dly_heavy = 0;
#endif
    PMU->pmu_dcdc_ctrl0 = pmu_dcdc_ctrl0_buf;

    pmu_dcdc_ctrl1_buf = PMU->pmu_dcdc_ctrl1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_ppower_light = 2;
    pmu_dcdc_ctrl1_buf.bit.dcdc_en_comp_light = 1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_npower_light = 7;
    pmu_dcdc_ctrl1_buf.bit.dcdc_en_zcd_light = 1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_pdrive_light = 3;
    pmu_dcdc_ctrl1_buf.bit.dcdc_mg_light = 1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_ndrive_light = 3;
    pmu_dcdc_ctrl1_buf.bit.dcdc_cm_light = 1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_pw_light = 0;
    pmu_dcdc_ctrl1_buf.bit.dcdc_c_sc_light = 0;
    pmu_dcdc_ctrl1_buf.bit.dcdc_c_hg_light = 1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_pwmf_light = 12;
    pmu_dcdc_ctrl1_buf.bit.dcdc_os_pn_light = 1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_os_light = 1;
    pmu_dcdc_ctrl1_buf.bit.dcdc_hg_light = 3;
    pmu_dcdc_ctrl1_buf.bit.dcdc_dly_light = 0;
    PMU->pmu_dcdc_ctrl1 = pmu_dcdc_ctrl1_buf;

    //===
    PMU->pmu_bbpll.bit.bbpll_byp_ldo = 1;
    PMU->pmu_bbpll.bit.bbpll_sel_vth = 1;
    //PMU->PMU_BBPLL.bit.BBPLL_HI = 1;
    //===

    SystemPmuSetMode(PMU_MODE_DCDC);

    reg_num = sizeof(pmu_ap_init_table_dcdc) / sizeof(reg_bit_write_t);

    for (i = 0; i < reg_num; i++) {
        reg_write_item = pmu_ap_init_table_dcdc[i];
        reg_bit_write(reg_write_item);
    }

    reg_num = sizeof(pmu_mp_init_table_dcdc) / sizeof(reg_bit_write_t);

    for (i = 0; i < reg_num; i++) {
        reg_write_item = pmu_mp_init_table_dcdc[i];

        reg_bit_write(reg_write_item);
    }
}

#endif
#endif

//#if (!MODULE_ENABLE(RCO40K_CALIBRATION_DISABLE))
void rco40k_analog_calibration(void) {
    uint32_t rco32k_c;
    uint32_t rco32k_result;

    PMU->pmu_cal32k_cfg0.bit.cfg_cal32k_en = 0;
    PMU->pmu_cal32k_cfg1.bit.cfg_32k_rc_sel = 0;

    PMU->pmu_rco32k.bit.rn_32k = 0;

    for (rco32k_c = 0; rco32k_c < 8; rco32k_c++) {
        PMU->pmu_rco32k.bit.c_32k = rco32k_c;
        PMU->pmu_cal32k_cfg1.bit.en_ck_cal32k = 1;
        PMU->pmu_cal32k_cfg1.bit.cfg_32k_est_mode = 0;
        PMU->pmu_cal32k_cfg1.bit.cfg_32k_est_time = 2;
        PMU->pmu_cal32k_cfg1.bit.cfg_32k_est_en = 1;
        delay_ms(1);
        PMU->pmu_cal32k_cfg1.bit.cfg_32k_est_en = 0;
        rco32k_result = PMU->pmu_cal32k_result0.bit.est_32k_result;
        if (rco32k_result >= 102915) {
            break;
        }
    }

    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_track_en = 1;
    PMU->pmu_cal32k_cfg0.bit.cfg_cal32k_target = 102400;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_lock_err = 5;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_avg_coarse = 1;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_avg_fine = 3;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_avg_lock = 3;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_dly = 2;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_fine_gain = 5;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_skip_coarse = 1;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_lock_gain = 5;
    PMU->pmu_cal32k_cfg1.bit.cfg_32k_rc_sel = 1;
    PMU->pmu_cal32k_cfg1.bit.cfg_cal32k_bound_mode = 0;
    PMU->pmu_cal32k_cfg0.bit.cfg_cal32k_en = 1;
}

//#endif

/**
 * \brief           Setup the microcontroller system.
 *                  Initialize the System.
 */
void SystemInit(void) {
    /*  This SystemInit is for boot used only,
     *  Please see system_cm3_mcu.s
     *  please don't call this function in other place.
     *  Remak:  .BSS section not initial yet.
     *   So you can not use any global variable.
     *  Also,  NO  race condition issue here...
     */
    /*set PMU XTAL. 2020/10/26 add  --- set cfg_xtal_settle_time*/
    PMU->pmu_xtal.reg = (PMU->pmu_xtal.reg & ~0xFF) | 0x3F;

    set_sram_shutdown_sleep(0);

#if (CHIP_VERSION == RT58X_MPB)
    /*change BBPLL setting 2022/03/25 for MPB */
    PMU->pmu_bbpll.bit.bbpll_sel_vth = 1;
#endif

    /*enable flash QSPI mode.*/
    flash_enable_qe();

#if ((CHIP_VERSION == RT58X_MPA) || (CHIP_VERSION == RT58X_MPB))
    systempmuupdatedig();

#if (SET_PMU_MODE == PMU_LDO_MODE)
    systempmuupdateldo();
#elif (SET_PMU_MODE == PMU_DCDC_MODE)
    systempmuupdatedcdc();
#endif
#endif

#if !CONFIG_HOSAL_SOC_DISABLE_MP_SECTOR_INIT
    mpsectorinit();
#endif

#if (SET_SYS_CLK == SYS_CLK_32MHZ)
    change_ahb_system_clk(SYS_32MHZ_CLK);
#elif (SET_SYS_CLK == SYS_CLK_48MHZ)
    change_ahb_system_clk(SYS_48MHZ_CLK);
#elif (SET_SYS_CLK == SYS_CLK_64MHZ)
    change_ahb_system_clk(SYS_64MHZ_CLK);
#endif

    systemcoreclockupdate();

    sys_set_retention_reg(6, 7);

#if (CHIP_TYPE < RT584)
    {
        /*Enable dmaClkGate to save DMA power for each channel*/
        dma_t* DMA;

        DMA = (dma_t*)DMA0_CH0_BASE;
        DMA->dma_enable |= DMA_DMACLK_GATE;
        DMA = (dma_t*)DMA0_CH1_BASE;
        DMA->dma_enable |= DMA_DMACLK_GATE;
        DMA = (dma_t*)DMA0_CH2_BASE;
        DMA->dma_enable |= DMA_DMACLK_GATE;
        DMA = (dma_t*)DMA0_CH3_BASE;
        DMA->dma_enable |= DMA_DMACLK_GATE;
    }
#endif

    rco40k_analog_calibration();

#if defined(CONFIG_EXTRCO32K_ENABLE)
    set_ext32k_pin(
        EXT32K_GPIO7); /* externl slow clock 32.768khz, select input gpio pin, gpio0~gpio7 (0~7) */
    set_slow_clock_source(
        EXT32K_GPIO_ENABLE); /* set slow clock(32.768khz) source form gpio */
#endif

    return;
}
