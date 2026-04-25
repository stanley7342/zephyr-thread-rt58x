/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           system_mcu.c
 * \brief          system mcu 
 */
/*
 * This file is part of library_name.
 * Author: ives.lee
 */

#include "sysctrl.h"
#include "system_mcu.h"
#include "flashctl.h"
#include "sysfun.h"
#include "mp_sector.h"
#include "gpio.h"
#if defined (__ARM_FEATURE_CMSE) &&  (__ARM_FEATURE_CMSE == 3U)
#include "partition.h"
#endif

/**
 * \brief           Define clocks
 */
#ifndef SET_SYS_CLK
#define SET_SYS_CLK    SYS_CLK_48MHZ
#endif

#if (SET_SYS_CLK == SYS_CLK_32MHZ)
#define XTAL    (32000000UL)                    /*!< Oscillator frequency */
#elif (SET_SYS_CLK == SYS_CLK_48MHZ)
#define XTAL    (48000000UL)                    /*!< Oscillator frequency */
#elif (SET_SYS_CLK == SYS_CLK_64MHZ)
#define XTAL    (64000000UL)                    /*!< Oscillator frequency */
#endif

#define  SYSTEM_CLOCK    (XTAL)

/**
 * \brief           Exception / Interrupt Vector table
 */
extern const VECTOR_TABLE_Type __VECTOR_TABLE[64];

/**
 * \brief           System Core Clock Variable
 */
uint32_t SystemCoreClock = SYSTEM_CLOCK;        /*!< System Core Clock Frequency */
uint32_t SystemFrequency = SYSTEM_CLOCK;

void systemcoreclockupdate (void) {
    SystemCoreClock = SYSTEM_CLOCK;
    SystemFrequency = SYSTEM_CLOCK;
}

/**
 * \brief           System pmu update dcdc parameter
 */


void systempmuupdatedcdc() 
{
    uint32_t chip_id = SYSCTRL->soc_chip_info.bit.chip_id;
    
    //offset:608c
    PMU_CTRL->soc_pmu_rco1m.bit.tune_fine_rco_1m = 70;
    PMU_CTRL->soc_pmu_rco1m.bit.tune_coarse_rco_1m = 11;
    PMU_CTRL->soc_pmu_rco1m.bit.pw_rco_1m = 1;
    PMU_CTRL->soc_pmu_rco1m.bit.test_rco_1m = 0;
    PMU_CTRL->soc_pmu_rco1m.bit.en_rco_1m = 0;      //1:rco1m enable, 0 : rco1m disable

    //offset:609c
    PMU_CTRL->pmu_soc_pmu_timing.bit.force_dcdc_soc_pmu =   1;// sub system pmu modecontrol by cm33

    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_ppower_normal    =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_en_comp_normal   =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_npower_normal    =   0x06;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_en_zcd_normal    =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_pdrive_normal    =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_mg_normal        =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_ndrive_normal    =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_en_cm_normal     =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_pw_normal        =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_c_hg_normal      =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_pwmf_normal      =   0x0E;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_c_sc_normal      =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_os_pn_normal     =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_os_normal        =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_hg_normal        =   0x03;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_dly_normal       =   0x00;
    //offset:60a8
    PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_normal =    0x0;
    //offset:60a0
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_ppower_heavy      =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_en_comp_heavy     =   0x1;
    //0ffset:60a0

    if(chip_id==0x0584)
    {
        #if defined(CONFIG_RF_POWER_14DBM)
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x2;
        #elif defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x0;
        #else
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x2;        //default 14dbm
        #endif
    }
    else  //rt584h/rt584l support 0dam/10dbm/20dbm
    {
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x0;
    }

    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_en_zcd_heavy      =   0x1;

    if(chip_id==0x0584)
    {  

        #if defined(CONFIG_RF_POWER_14DBM)
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x6;
        #elif defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x1;
        #else
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x6;        //default 14dbm
        #endif
    }
    else  //rt584h/rt584l support 0dam/10dbm/20dbm
    {
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x1;
    }
   
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_mg_heavy          =   0x1;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_ndrive_heavy      =   0x2;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_en_cm_heavy       =   0x1;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pw_heavy          =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_c_hg_heavy        =   0x1;

    if(chip_id==0x0584)
    {

        #if defined(CONFIG_RF_POWER_14DBM)
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0C;
        #elif defined(CONFIG_RF_POWER_0DBM)
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x08;
        #elif defined(CONFIG_RF_POWER_20DBM)
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0E;
        #else
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0C;       //default 14dbm
        #endif
    }
    else if(chip_id==0x1584) //rt584l
    {
        #if defined(CONFIG_RF_POWER_0DBM)
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x08;
        #elif defined(CONFIG_RF_POWER_10DBM)
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0C;
        #else
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0C;       //default 10dbm
        #endif
    }
    else if(chip_id==0x3584)  //rt584h
    {
        #if defined(CONFIG_RF_POWER_0DBM)
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x08;
        #elif defined(CONFIG_RF_POWER_20DBM)
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0E;
        #else
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0E;       //default 20dbm
        #endif
    }  

    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_c_sc_heavy        =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_os_pn_heavy       =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_os_heavy          =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_hg_heavy          =   0x3;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_dly_heavy         =   0x0;
    //offset:60a8
    PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_heavy   =   0x0;
    //offset:60a4
    if(chip_id==0x0584)
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ppower_light      =   0x3;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ppower_light      =   0x0;
    }
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_en_comp_light     =   0x1;

    if(chip_id==0x0584)
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_npower_light      =   0x3;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_npower_light     =   0x0;
    }    
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_en_zcd_light      =   0x1;

    if(chip_id==0x0584)
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pdrive_light  =   0x7;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pdrive_light   =   0x5;
    }  

    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_mg_light          =   0x1;

    if(chip_id==0x0584)
    { 
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ndrive_light    =   0x7;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ndrive_light    =   0x5;
    }  
   
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_en_cm_light       =   0x1;

    if(chip_id==0x0584)
    { 
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pw_light        =  0x5;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pw_light     =  0x3;
    }       
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_c_hg_light        =   0x1;
    
    if(chip_id==0x0584)
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pwmf_light        =   0xE;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pwmf_light        =   0xF;
    }    
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_c_sc_light        =   0x0;
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_os_pn_light       =   0x0;

    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_os_light          =  0x0;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_os_light          =  0x3;
    }      
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_hg_light          =   0x3;
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_dly_light         =   0x0;
    //offset:60a8
    if(chip_id==0x0584)
    { 
        PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_light   =   0x0;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_light   =  0x3;
    }   
    
    //offset:60ac
    PMU_CTRL->pmu_ldo_ctrl.bit.dcdc_ioc                 =   0x01;

    PMU_CTRL->pmu_ldo_ctrl.bit.dcdc_rup_en              =  0x01;
    //offset:60ac
    PMU_CTRL->pmu_ldo_ctrl.bit.ldodig_sin               =   0x00;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldodig_lout              =   0x01;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldodig_ioc_nm            =   0x01;

    PMU_CTRL->pmu_ldo_ctrl.bit.ldomv_sin                =   0x00;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldomv_lout               =   0x01;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldomv_ioc_nm             =   0x01;

    //offset:60b8
    PMU_CTRL->pmu_rfldo.bit.ldoana_lout                 =   0x01;
    PMU_CTRL->pmu_rfldo.bit.ldoana_ioc_nm               =   0x01;

    //offset:6020
    PMU_CTRL->pmu_soc_pmu_xtal0.bit.xosc_lpf_c          =   0x03;
    PMU_CTRL->pmu_soc_pmu_xtal0.bit.xosc_lpf_r          =   0x01;
    //
    //offset:60b0
    //PMU_CTRL->pmu_en_control.bit.en_ldomv_nm = 1;
    //PMU_CTRL->pmu_en_control.bit.en_dcdc_nm = 1;
    //PMU_CTRL->pmu_en_control.bit.en_ldomv_nm = 0;
    sys_pmu_setmode(SET_PMU_MODE);
    //
    //offset:6098
    PMU_CTRL->pmu_core_vosel.bit.sldo_vosel_sp = 8; // (8~10),
    //
    // offset:6090
    PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_normal = 0x0A;

    if(chip_id==0x0584)
    {

        #if defined(CONFIG_RF_POWER_14DBM)
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = 0x17;
        #elif defined(CONFIG_RF_POWER_0DBM) ||defined(CONFIG_RF_POWER_20DBM)
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = 0x0A;
        #else
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = 0x17;               //default 14dbm
        #endif
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = 0xA;

    }

    PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_light = 0x0A;

    //offset:6094
    PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_normal = 0x0A;

    if(chip_id==0x0584)
    {
        #if defined(CONFIG_RF_POWER_14DBM)
            //offset:6094
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0x17;
        #elif defined(CONFIG_RF_POWER_0DBM) ||defined(CONFIG_RF_POWER_20DBM)
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0x0A;
        #else
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0x17;             //default 14dbm
        #endif
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0xA;
    }

    //offset:6094
    PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_light = 0x0A;

    //offset:60b8
    PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_normal = 0x09;
    PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_heavy = 0x0A;
    //offset:6098
    PMU_CTRL->pmu_core_vosel.bit.ldodig_vosel = 0xA;
    //offset:6024
    PMU_CTRL->pmu_soc_pmu_xtal1.bit.xosc_cap_ini = 29;
}

/**
 * \brief           Calibration rco 1m / rco 32k
 */
void rco1m_and_rco32k_calibration() {

#if defined(CONFIG_RCO32K_ENABLE)
    PMU_CTRL->pmu_osc32k.bit.tune_fine_rco_32k = 88;
    PMU_CTRL->pmu_osc32k.bit.tune_coarse_rco_32k = 3;
    PMU_CTRL->pmu_osc32k.bit.pw_buf_rco_32k = 3;
    PMU_CTRL->pmu_osc32k.bit.pw_rco_32k = 15;
    PMU_CTRL->pmu_osc32k.bit.rco_32k_sel = 1;
    SYSCTRL->sys_clk_ctrl2.bit.en_rco32k_div2 = 0;

    //RCO
    RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_target = 128000; //per_clk = 32mhz is  128000'd
    //per_clk = 16mhz is  64000'd
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

    //offset 0x500060BC
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_mv_settle_time = 12;   //mv  settle time = 400us (400us for 1.8v, 3.3v 200us)
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_lv_settle_time = 5;    //lv  settle time = 150us
    //offset 0x50006024
    PMU_CTRL->pmu_soc_pmu_xtal1.bit.cfg_xtal_settle_time = 31;  //1ms
    //offset 0x50006040
    PMU_CTRL->pmu_rvd0.bit.cfg_xtal_fast_time = 15;             //0.5ms
    //offset 0x500060bc
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_pwrx_settle_time = 2;


#elif defined(CONFIG_RCO16K_ENABLE)
    //32K DIV 2
    PMU_CTRL->pmu_osc32k.bit.tune_fine_rco_32k = 88;
    PMU_CTRL->pmu_osc32k.bit.tune_coarse_rco_32k = 3;
    PMU_CTRL->pmu_osc32k.bit.pw_buf_rco_32k = 3;
    PMU_CTRL->pmu_osc32k.bit.pw_rco_32k = 0;
    PMU_CTRL->pmu_osc32k.bit.rco_32k_sel = 1;
    SYSCTRL->sys_clk_ctrl2.bit.en_ck_div_32k = 1;
    SYSCTRL->sys_clk_ctrl2.bit.en_rco32k_div2 = 1;

    RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_target = 128000; //per_clk = 32mhz, target ~= 20KHz is 204800'd
    //per_clk = 16mhz, target ~= 20khz is 102400'd
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

    //offset 0x500060BC
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_mv_settle_time = 6;    //mv  settle time = 400us (400us for 1.8V, 3.3V 200us)
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_lv_settle_time = 2;    //lv  settle time = 150
    //offset 0x50006024
    PMU_CTRL->pmu_soc_pmu_xtal1.bit.cfg_xtal_settle_time = 15;   //slow clk 10k, 1ms
    //offset 0x50006040
    PMU_CTRL->pmu_rvd0.bit.cfg_xtal_fast_time = 7;              //slow clk 10k, 0.5ms
    //offset 0x500060bc
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_pwrx_settle_time = 1;

    RTC->rtc_clock_div = 0x100000;
#else
    //default RCO32K
    PMU_CTRL->pmu_osc32k.bit.tune_fine_rco_32k = 88;
    PMU_CTRL->pmu_osc32k.bit.tune_coarse_rco_32k = 3;
    PMU_CTRL->pmu_osc32k.bit.pw_buf_rco_32k = 3;
    PMU_CTRL->pmu_osc32k.bit.pw_rco_32k = 15;
    PMU_CTRL->pmu_osc32k.bit.rco_32k_sel = 1;
    SYSCTRL->sys_clk_ctrl2.bit.en_rco32k_div2 = 0;

    //RCO
    RCO32K_CAL->cal32k_cfg0.bit.cfg_cal32k_target = 128000; //per_clk = 32mhz is  128000'd
    //per_clk = 16mhz is  64000'd
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


#endif

    //Offset:608C
    PMU_CTRL->soc_pmu_rco1m.bit.tune_fine_rco_1m = 70;
    PMU_CTRL->soc_pmu_rco1m.bit.tune_coarse_rco_1m = 11;
    PMU_CTRL->soc_pmu_rco1m.bit.pw_rco_1m = 1;
    PMU_CTRL->soc_pmu_rco1m.bit.test_rco_1m = 0;
    PMU_CTRL->soc_pmu_rco1m.bit.en_rco_1m = 0;      //1:rco1m enable, 0 : rco1m disable

    //RCO1M
    RCO1M_CAL->cal1m_cfg0.bit.cfg_cal_target = 0x22b8e; //per_clk = 32mhz is 0x22B8E' hper_clk = 16MHz is 115C7'h"
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


    SYSCTRL->sram_lowpower_0.reg = 0xffffffff;

    PMU_CTRL->pmu_bg_control.bit.pmu_res_dis = 1;

    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_bypass_mv_settle = 1;
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_bypass_lv_settle = 1;
    PMU_CTRL->pmu_soc_pmu_xtal1.bit.cfg_bypass_xtal_settle = 0;
}

void SystemPmuUpdateDcdcTxPwrLvl(txpower_default_cfg_t txpwrlevel)
{
    uint16_t chip_id = SYSCTRL->soc_chip_info.bit.chip_id;
    //Offset:608C
    //PMU_CTRL->soc_pmu_rco1m.bit.TUNE_FINE_RCO_1M = 70;
    //PMU_CTRL->soc_pmu_rco1m.bit.TUNE_COARSE_RCO_1M = 11;
    //PMU_CTRL->soc_pmu_rco1m.bit.PW_RCO_1M = 1;
    //PMU_CTRL->soc_pmu_rco1m.bit.TEST_RCO_1M = 0;
    //PMU_CTRL->soc_pmu_rco1m.bit.EN_RCO_1M = 0;      //1:rco1m enable, 0 : rco1m disable

    //offset:609c
    PMU_CTRL->pmu_soc_pmu_timing.bit.force_dcdc_soc_pmu =   1;// sub system pmu modecontrol by cm33

    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_ppower_normal    =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_en_comp_normal   =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_npower_normal    =   0x06;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_en_zcd_normal    =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_pdrive_normal    =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_mg_normal        =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_ndrive_normal    =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_en_cm_normal     =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_pw_normal        =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_c_hg_normal      =   0x01;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_pwmf_normal      =   0x0E;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_c_sc_normal      =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_os_pn_normal     =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_os_normal        =   0x00;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_hg_normal        =   0x03;
    PMU_CTRL->pmu_dcdc_normal.bit.dcdc_dly_normal       =   0x00;
    //offset:60a8
    PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_normal =    0x0;
    //offset:60a0
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_ppower_heavy      =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_en_comp_heavy     =   0x1;
    //0ffset:60a0


    if(chip_id==0x0584)
    {
        if (txpwrlevel == TX_POWER_14DBM_DEF)  {

            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x2;
        }
        else if ((txpwrlevel == TX_POWER_0DBM_DEF) || (txpwrlevel == TX_POWER_20DBM_DEF))  {
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x0;
        }
        else {//default power 
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x2;        //default 14dbm
        }
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_npower_heavy      =   0x0;
    }


    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_en_zcd_heavy      =   0x1;


    if(chip_id==0x0584)
    {  

        if (txpwrlevel == TX_POWER_14DBM_DEF) {
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x6;
        }
        else if ((txpwrlevel == TX_POWER_0DBM_DEF) || (txpwrlevel == TX_POWER_20DBM_DEF)) {
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x1;
        }
        else  {//default power
        
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x6;        //default 14dbm
        }

    }
    else //rt584h/rt584l
    {

        PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pdrive_heavy      =   0x1;

    }



    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_mg_heavy          =   0x1;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_ndrive_heavy      =   0x2;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_en_cm_heavy       =   0x1;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pw_heavy          =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_c_hg_heavy        =   0x1;

    if(chip_id==0x0584)
    {  

        if (txpwrlevel == TX_POWER_14DBM_DEF){
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0c;
        }
        else if (txpwrlevel == TX_POWER_0DBM_DEF){
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x08;
        }
        else if (txpwrlevel == TX_POWER_20DBM_DEF){
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0E;
        }
        else{ //default power
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0C;       //default 14dbm
        }
    }
    else if(chip_id==0x1584)
    {
            if (txpwrlevel == TX_POWER_0DBM_DEF) {
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x08;
            }
            else if (txpwrlevel == TX_POWER_10DBM_DEF) {
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0C;
            }
            else{
            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0C;       //default 10dbm
            }
    }
    else if(chip_id==0x3584)  //rt584h
    {
        if (txpwrlevel == TX_POWER_0DBM_DEF) {
          PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x08;
        }
        else if (txpwrlevel == TX_POWER_20DBM_DEF) {
          PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0E;
        }
        else{

            PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_pwmf_heavy        =   0x0E;       //default 20dbm
          }
    }  


    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_c_sc_heavy        =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_os_pn_heavy       =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_os_heavy          =   0x0;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_hg_heavy          =   0x3;
    PMU_CTRL->pmu_dcdc_heavy.bit.dcdc_dly_heavy         =   0x0;
    //offset:60a8
    PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_heavy   =   0x0;
    //offset:60a4
    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ppower_light  =   0x3;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ppower_light  =   0x0;
    }   
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_en_comp_light     =   0x1;

    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_npower_light  =  0x3;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_npower_light  =  0x0;
    }      
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_en_zcd_light      =   0x1;

    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pdrive_light  =   0x7;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pdrive_light   =   0x5;
    }    

    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_mg_light          =   0x1;

    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ndrive_light   =   0x7;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_ndrive_light   =   0x5;
    }      
   
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_en_cm_light       =   0x1;

    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pw_light        =  0x5;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pw_light    =  0x3;
    }        
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_c_hg_light        =   0x1;

    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pwmf_light        =   0xE;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_pwmf_light        =   0xF;
    }     
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_c_sc_light        =   0x0;
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_os_pn_light       =   0x0;

    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_os_light          =  0x0;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_light.bit.dcdc_os_light          =  0x3;
    }      
    
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_hg_light          =   0x3;
    PMU_CTRL->pmu_dcdc_light.bit.dcdc_dly_light         =   0x0;
    //offset:60a8
    if(chip_id==0x0584)
    {  
        PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_light   =   0x0;
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_reserved.bit.dcdc_pw_dig_light  =  0x3;
    }    
    
    //offset:60ac
    PMU_CTRL->pmu_ldo_ctrl.bit.dcdc_ioc                 =   0x01;

    PMU_CTRL->pmu_ldo_ctrl.bit.dcdc_rup_en              =  0x01;
    //offset:60ac
    PMU_CTRL->pmu_ldo_ctrl.bit.ldodig_sin               =   0x00;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldodig_lout              =   0x01;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldodig_ioc_nm            =   0x01;

    PMU_CTRL->pmu_ldo_ctrl.bit.ldomv_sin                =   0x00;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldomv_lout               =   0x01;
    PMU_CTRL->pmu_ldo_ctrl.bit.ldomv_ioc_nm             =   0x01;

    //offset:60b8
    PMU_CTRL->pmu_rfldo.bit.ldoana_lout                 =   0x01;
    PMU_CTRL->pmu_rfldo.bit.ldoana_ioc_nm               =   0x01;

    //offset:6020
    PMU_CTRL->pmu_soc_pmu_xtal0.bit.xosc_lpf_c          =   0x03;
    PMU_CTRL->pmu_soc_pmu_xtal0.bit.xosc_lpf_r          =   0x01;
    //
    //offset:60b0
    //PMU_CTRL->pmu_en_control.bit.en_ldomv_nm = 1;
    //PMU_CTRL->pmu_en_control.bit.en_dcdc_nm = 1;
    //PMU_CTRL->pmu_en_control.bit.en_ldomv_nm = 0;
    sys_pmu_setmode(SET_PMU_MODE);
    //
    //offset:6098
    PMU_CTRL->pmu_core_vosel.bit.sldo_vosel_sp = 8; // (8~10),
    //
    // offset:6090
    PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_normal = 0x0A;


    if(chip_id==0x0584)
    {  
        if (txpwrlevel == TX_POWER_14DBM_DEF) {
            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = 0x17;
        }
        else if ((txpwrlevel == TX_POWER_0DBM_DEF) || (txpwrlevel == TX_POWER_20DBM_DEF)) {
            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = 0x0A;
        }
        else { //default power
        
            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = 0x17;               //default 14dbm
        }
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy          =  0xA;
    } 




    PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_light = 0x0A;

    //offset:6094
    PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_normal = 0x0A;

    if(chip_id==0x0584)
    {
        if (txpwrlevel == TX_POWER_14DBM_DEF) {
        
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0x17;
        }
        else if ((txpwrlevel == TX_POWER_0DBM_DEF) || (txpwrlevel == TX_POWER_20DBM_DEF)) { 
        
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0x0A;
        }
        else { //default power
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0x17;             //default 14dbm
        }
    }
    else //rt584h/rt584l
    {
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = 0x0A;
    } 


    //offset:6094
    PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_light = 0x0A;

    //offset:60b8
    PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_normal = 0x09;
    PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_heavy = 0x0A;
    //offset:6098
    PMU_CTRL->pmu_core_vosel.bit.ldodig_vosel = 0xa;
    //offset:6024
    PMU_CTRL->pmu_soc_pmu_xtal1.bit.xosc_cap_ini = 29;


    //low power config
    SYSCTRL->sram_lowpower_0.reg = 0xFFFFFFFF;

    PMU_CTRL->pmu_bg_control.bit.pmu_res_dis = 1;

    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_bypass_mv_settle = 1;
    PMU_CTRL->pmu_soc_pmu_timing.bit.cfg_bypass_lv_settle = 1;
    PMU_CTRL->pmu_soc_pmu_xtal1.bit.cfg_bypass_xtal_settle = 0;

    //Set RF Tx Power config
    set_sys_txpower_default(txpwrlevel);
}

void systeminit (void) {

    #if defined(CONFIG_RF_POWER_14DBM) || defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
    #elif defined(CONFIG_BASIC_EXAMPLE) || defined(CONFIG_HELLOWORLD)
    #else
        txpower_default_cfg_t txpwrlevel = sys_txpower_getdefault();
    #endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    //  uint32_t blk_cfg, blk_max, blk_size, blk_cnt;
#endif

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
    SCB->VTOR = (uint32_t) & (__VECTOR_TABLE[0]);
#endif

#if defined (__FPU_USED) && (__FPU_USED == 1U)
    /* Coprocessor Access Control Register. It's banked for secure state and non-seure state */
    SCB->CPACR |= ((3U << 10U * 2U) |         /* enable CP10 Full Access */
                   (3U << 11U * 2U)  );       /* enable CP11 Full Access */

    /*Notice: CPACR Secure state address 0xE000ED88.  CPACR_NS is 0xE002ED88
     *   Secure software can also define whether non-secure software
     *   can access ecah of the coprocessor using a register called NSACR
     *   Non-secure Access Control Register.
     */

#endif


#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)

    /* Enable BusFault, UsageFault, MemManageFault and SecureFault to ease diagnostic */
    SCB->SHCSR |= (SCB_SHCSR_USGFAULTENA_Msk  |
                   SCB_SHCSR_BUSFAULTENA_Msk  |
                   SCB_SHCSR_MEMFAULTENA_Msk  |
                   SCB_SHCSR_SECUREFAULTENA_Msk);

    /* BFSR register setting to enable precise errors */
    SCB->CFSR |= SCB_CFSR_PRECISERR_Msk;

	TZ_SAU_Setup();
	
#endif

#if defined(CONFIG_RF_POWER_14DBM) || defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
    systempmuupdatedcdc();
#elif defined(CONFIG_BASIC_EXAMPLE) || defined(CONFIG_HELLOWORLD)
    systempmuupdatedcdc();
#else
    SystemPmuUpdateDcdcTxPwrLvl(txpwrlevel);
#endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)

#if defined(CONFIG_FLASHCTRL_SECURE_EN)
    /*set flash timing.*/
    flash_timing_init();

    /*enable flash 4 bits mode*/
    flash_enable_qe();

    rco1m_and_rco32k_calibration();
#endif

#if defined(CONFIG_SYSCTRL_SECURE_EN)

#if (SET_SYS_CLK == SYS_CLK_32MHZ)

    change_ahb_system_clk(SYS_32MHZ_CLK);
    change_peri_clk(PERCLK_SEL_32M);

#elif (SET_SYS_CLK == SYS_CLK_48MHZ)

    change_ahb_system_clk(SYS_48MHZ_CLK);
    change_peri_clk(PERCLK_SEL_32M);

#elif (SET_SYS_CLK == SYS_CLK_64MHZ)

    change_ahb_system_clk(SYS_64MHZ_CLK);
    change_peri_clk(PERCLK_SEL_32M);
    
#endif

#endif

     //slow_clock_calibration(RCO32K_ENABLE);
#else

#if defined(CONFIG_SYSCTRL_SECURE_EN)

#if defined(CONFIG_FLASHCTRL_SECURE_EN)
    /*set flash timing.*/
    flash_timing_init();

    /*enable flash 4 bits mode*/
    flash_enable_qe();

    rco1m_and_rco32k_calibration();

    
#endif

#if (SET_SYS_CLK == SYS_CLK_32MHZ)
   
    change_ahb_system_clk(SYS_32MHZ_CLK);
    change_peri_clk(PERCLK_SEL_32M);

#elif (SET_SYS_CLK == SYS_CLK_48MHZ)

    change_ahb_system_clk(SYS_48MHZ_CLK);
    change_peri_clk(PERCLK_SEL_32M);

#elif (SET_SYS_CLK == SYS_CLK_64MHZ)

    change_ahb_system_clk(SYS_64MHZ_CLK);
    change_peri_clk(PERCLK_SEL_32M);
#endif

#endif

#endif

#if !CONFIG_HOSAL_SOC_DISABLE_MP_SECTOR_INIT
        mpsectorinit();
#endif

    SystemCoreClock = SYSTEM_CLOCK;

#if defined(CONFIG_EXTRCO32K_ENABLE)
    set_ext32k_pin(GPIO5);
    set_slow_clock_source(EXT_GPIO_RCO32K);
#endif

}

//#if  (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
///*This interrupt must be secure world.*/

///*Debug used*/
//void Sec_Ctrl_Handler(void)
//{
//    uint32_t status;

//    status = SEC_CTRL->SEC_INT_STATUS.reg;
//    SEC_CTRL->SEC_INT_CLR.reg = status;
//    status = SEC_CTRL->SEC_INT_STATUS.reg;         /*ensure the clear.*/
//}

//#endif

