/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            sadc.c
 * \brief           sadc driver file
 */
/*
 * Author:          Kc.tseng
 */


#include "sadc.h"
#include "sysctrl.h"
#include "aux_comp.h"
#include "mp_sector.h"

static sadc_proc_cb sadc_reg_handler = NULL;
static uint32_t sadc_xdma_single_mode = DISABLE;
static sadc_convert_state_t sadc_convert_state = SADC_CONVERT_IDLE;
static sadc_convert_state_t sadc_ch_read_convert_state = SADC_CONVERT_IDLE;
//static int32_t sadc_compensation_offset = 0;
//static int32_t sadc_temperature_calibration_offset = 0;

static sadc_input_ch_t  sadc_convert_ch = SADC_CH_NC;
static sadc_value_t sadc_ch_value;

static sadc_config_resolution_t sadc_config_res = SADC_RES_12BIT;
static sadc_config_oversample_t sadc_config_os = SADC_OVERSAMPLE_256;
static sadc_proc_cb sadc_config_int_callback = NULL;

sadc_channel_config_t sadc_ch_init[] = {
    {SADC_CHANNEL_0, SADC_AIN_0,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_1,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_2,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_3,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_4,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_5,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_6,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_7,       SADC_AIN_DISABLED4, SADC_GAIN_AIO,  SADC_PULL_AIO,  SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_TEMPERATURE, SADC_TEMPERATURE,   SADC_GAIN_TEMP, SADC_PULL_TEMP, SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_VBAT,        SADC_AIN_DISABLED4, SADC_GAIN_VBAT, SADC_PULL_VBAT, SADC_TACQ_EDLY_TIME_4US, SADC_TACQ_EDLY_TIME_2US, SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
};


void sadc_analog_aio_init(void) {
    SADC->sadc_ana_set0.bit.aux_adc_debug = 0;
    SADC->sadc_ana_set0.bit.aux_adc_mode = 0;
    SADC->sadc_ana_set0.bit.aux_adc_outputstb = 0;
    SADC->sadc_ana_set0.bit.aux_adc_ospn = 0;
    SADC->sadc_ana_set0.bit.aux_adc_clk_sel = 0;
    SADC->sadc_ana_set0.bit.aux_adc_mcap = 3;
    SADC->sadc_ana_set0.bit.aux_adc_mdly = 2;
    SADC->sadc_ana_set0.bit.aux_adc_sel_duty = 2;
    SADC->sadc_ana_set0.bit.aux_adc_os = 0;
    SADC->sadc_ana_set0.bit.aux_adc_br = 15;
    SADC->sadc_ana_set0.bit.aux_adc_pw = 0;
    SADC->sadc_ana_set0.bit.aux_adc_stb_bit = 0;
    SADC->sadc_ana_set0.bit.aux_pw = 2;

    SADC->sadc_ana_set1.bit.aux_vga_cmsel = 0;
    SADC->sadc_ana_set1.bit.aux_vga_comp = 1;
    SADC->sadc_ana_set1.bit.aux_vga_sin = 0;
    SADC->sadc_ana_set1.bit.aux_vga_lout = 0;
    SADC->sadc_ana_set1.bit.aux_vga_sw_vdd = 0;
    SADC->sadc_ana_set1.bit.aux_vga_vldo = 2;
    SADC->sadc_ana_set1.bit.aux_vga_acm = 15;
    SADC->sadc_ana_set1.bit.aux_vga_pw = 31;
    SADC->sadc_ana_set1.bit.aux_dc_adj = 3;
    SADC->sadc_ana_set1.bit.aux_test_mode = 0;
    SADC->sadc_ana_set1.bit.cfg_en_clkaux = 1;
    SADC->sadc_ana_set1.bit.aux_vga_test_aio_en = 0;
    AUX_COMP->comp_ana_ctrl.bit.comp_en_start = 1;
}

void sadc_analog_vbat_init(void) {
    SADC->sadc_ana_set0.bit.aux_adc_debug = 0;
    SADC->sadc_ana_set0.bit.aux_adc_mode = 0;
    SADC->sadc_ana_set0.bit.aux_adc_outputstb = 0;
    SADC->sadc_ana_set0.bit.aux_adc_ospn = 0;
    SADC->sadc_ana_set0.bit.aux_adc_clk_sel = 0;
    SADC->sadc_ana_set0.bit.aux_adc_mcap = 3;
    SADC->sadc_ana_set0.bit.aux_adc_mdly = 2;
    SADC->sadc_ana_set0.bit.aux_adc_sel_duty = 2;
    SADC->sadc_ana_set0.bit.aux_adc_os = 0;
    SADC->sadc_ana_set0.bit.aux_adc_br = 15;
    SADC->sadc_ana_set0.bit.aux_adc_pw = 0;
    SADC->sadc_ana_set0.bit.aux_adc_stb_bit = 0;
    SADC->sadc_ana_set0.bit.aux_pw = 2;

    SADC->sadc_ana_set1.bit.aux_vga_cmsel = 0;
    SADC->sadc_ana_set1.bit.aux_vga_comp = 1;
    SADC->sadc_ana_set1.bit.aux_vga_sin = 0;
    SADC->sadc_ana_set1.bit.aux_vga_lout = 0;
    SADC->sadc_ana_set1.bit.aux_vga_sw_vdd = 0;
    SADC->sadc_ana_set1.bit.aux_vga_vldo = 2;
    SADC->sadc_ana_set1.bit.aux_vga_acm = 15;
    SADC->sadc_ana_set1.bit.aux_vga_pw = 28;
    SADC->sadc_ana_set1.bit.aux_dc_adj = 0;
    SADC->sadc_ana_set1.bit.aux_test_mode = 0;
    SADC->sadc_ana_set1.bit.cfg_en_clkaux = 1;
    SADC->sadc_ana_set1.bit.aux_vga_test_aio_en = 0;
    AUX_COMP->comp_ana_ctrl.bit.comp_en_start = 1;
}

void sadc_analog_temp_init(void) {
    SADC->sadc_ana_set0.bit.aux_adc_debug = 0;
    SADC->sadc_ana_set0.bit.aux_adc_mode = 0;
    SADC->sadc_ana_set0.bit.aux_adc_outputstb = 0;
    SADC->sadc_ana_set0.bit.aux_adc_ospn = 0;
    SADC->sadc_ana_set0.bit.aux_adc_clk_sel = 0;
    SADC->sadc_ana_set0.bit.aux_adc_mcap = 3;
    SADC->sadc_ana_set0.bit.aux_adc_mdly = 2;
    SADC->sadc_ana_set0.bit.aux_adc_sel_duty = 2;
    SADC->sadc_ana_set0.bit.aux_adc_os = 0;
    SADC->sadc_ana_set0.bit.aux_adc_br = 15;
    SADC->sadc_ana_set0.bit.aux_adc_pw = 0;
    SADC->sadc_ana_set0.bit.aux_adc_stb_bit = 0;
    SADC->sadc_ana_set0.bit.aux_pw = 2;

    SADC->sadc_ana_set1.bit.aux_vga_cmsel = 0;
    SADC->sadc_ana_set1.bit.aux_vga_comp = 1;
    SADC->sadc_ana_set1.bit.aux_vga_sin = 0;
    SADC->sadc_ana_set1.bit.aux_vga_lout = 0;
    SADC->sadc_ana_set1.bit.aux_vga_sw_vdd = 0;
    SADC->sadc_ana_set1.bit.aux_vga_vldo = 2;
    SADC->sadc_ana_set1.bit.aux_vga_acm = 15;
    SADC->sadc_ana_set1.bit.aux_vga_pw = 31;
    SADC->sadc_ana_set1.bit.aux_dc_adj = 3;
    SADC->sadc_ana_set1.bit.aux_test_mode = 0;
    SADC->sadc_ana_set1.bit.cfg_en_clkaux = 1;
    SADC->sadc_ana_set1.bit.aux_vga_test_aio_en = 0;
    AUX_COMP->comp_ana_ctrl.bit.comp_en_start = 1;


    PMU_CTRL->soc_ts.bit.ts_vx = 5;
    PMU_CTRL->soc_ts.bit.ts_s = 4;
    PMU_CTRL->soc_ts.bit.ts_en = 1;
    PMU_CTRL->soc_ts.bit.ts_rst = 0;
    PMU_CTRL->soc_ts.bit.ts_rst = 1;
    PMU_CTRL->soc_ts.bit.ts_rst = 0;
    PMU_CTRL->soc_ts.bit.ts_clk_en = 1;
    PMU_CTRL->soc_ts.bit.ts_clk_sel = 1;
}


void sadc_register_int_callback(sadc_proc_cb sadc_int_callback) {
    sadc_reg_handler = sadc_int_callback;
    return;
}

void sadc_int_enable(uint32_t int_mask) { 
    SADC->sadc_int_clear.reg = SADC_INT_CLEAR_ALL;
    SADC->sadc_int_mask.reg = int_mask;
    
    return;
}

void sadc_int_disable(void) {
    SADC->sadc_int_mask.reg = SADC_INT_DISABLE_ALL;
    SADC->sadc_int_clear.reg = SADC_INT_CLEAR_ALL;
    return;
}


void sadc_aio_disable(uint8_t aio_num) {
    SYSCTRL->gpio_aio_ctrl.bit.gpio_en_aio &= ~(0x01 << aio_num);
    return;
}

void sadc_aio_enable(uint8_t aio_num) {
    SYSCTRL->gpio_aio_ctrl.bit.gpio_en_aio |= (0x01 << aio_num);
    return;
}

void sadc_xdma_config(uint32_t xdma_start_addr,
                      uint16_t xdma_seg_size,
                      uint16_t xdma_blk_size) {
    /*Reset XDMA*/
    SADC->sadc_wdma_ctl1.bit.cfg_sadc_wdma_ctl1 = ENABLE;

    /*Clear XDMA IRQ*/
    SADC->sadc_int_clear.bit.wdma = ENABLE;
    SADC->sadc_int_clear.bit.wdma_error = ENABLE;

    /*Enable XDMA IRQ*/
    SADC->sadc_int_mask.bit.wdma = 0;
    SADC->sadc_int_mask.bit.wdma_error = 0;


    /*Set XDMA buffer address*/
    SADC->sadc_wdma_set1 = xdma_start_addr;

    /*Set XDMA buffer size*/
    SADC->sadc_wdma_set0.bit.cfg_sadc_seg_size = xdma_seg_size;
    SADC->sadc_wdma_set0.bit.cfg_sadc_blk_size = xdma_blk_size;

    /*Start XDMA for memory access*/
    SADC->sadc_wdma_ctl0.bit.cfg_sadc_wdma_ctl0 = ENABLE;

    return;
}

uint32_t sadc_resolution_compensation(sadc_value_t *p_data) {
    uint32_t compensation_bit = 0;

    if (p_data == NULL) {
        return STATUS_INVALID_PARAM;
    }

    switch (SADC->sadc_set1.bit.cfg_sadc_bit) {
    case SADC_RES_8BIT:
        compensation_bit = 6;
        break;

    case SADC_RES_10BIT:
        compensation_bit = 4;
        break;

    case SADC_RES_12BIT:
        compensation_bit = 2;
        break;

    case SADC_RES_14BIT:
        break;

    default:
        break;
    }

    (*p_data) >>= compensation_bit;

    return STATUS_SUCCESS;
}

/**
* @brief Sadc resolution compensation
*/
uint32_t sadc_resolution_compensation_temp(sadc_value_t *p_data)
{
    if (p_data == NULL)
    {
        return STATUS_INVALID_PARAM;
    }

    (*p_data) >>= 2;

    return STATUS_SUCCESS;
}

void sadc_calibration_init(void) {
    mpcaladcinit();
}

uint32_t sadc_calibration(sadc_cal_type_t cal_type, sadc_value_t *adc_val) {
    uint32_t read_status;
    mp_cal_adc_t mp_cal_adc;
    mp_cal_temp_adc_t mp_cal_temp_adc;
    int32_t cal_vol = 0;

    if (cal_type == SADC_CALIBRATION_VBAT) {

        read_status = mpcalvbatadcread(&mp_cal_adc);

        if (read_status == STATUS_SUCCESS) {
            float m, k;
            uint32_t adc_1, adc_2;

            adc_1 = mp_cal_adc.adc_1 << 2;
            adc_2 = mp_cal_adc.adc_2 << 2;

            m = (float)(3600 - 1800) / (float)(adc_2 - adc_1);
            k = (float)1800 - (float)(m * adc_1);

            *adc_val = *adc_val * m + k;

        } else {
            *adc_val = *adc_val;
        }

    } else if (cal_type == SADC_CALIBRATION_AIO) {

        read_status = mpcalaioadcread(&mp_cal_adc);

        if (read_status == STATUS_SUCCESS) {
            float m, k;
            uint32_t adc_1, adc_2;

            adc_1 = mp_cal_adc.adc_1 << 2;
            adc_2 = mp_cal_adc.adc_2 << 2;

            m = (float)3600 / (float)(adc_2 - adc_1);
            k = - (float)(m * adc_1);

            *adc_val = *adc_val * m + k;
        } else {
            *adc_val = *adc_val;
        }

    } else if (cal_type == SADC_CALIBRATION_TEMP) {
        read_status = mpcaltempadcread(&mp_cal_temp_adc);
        if (read_status == STATUS_SUCCESS) {
            *adc_val = (float)(*adc_val - mp_cal_temp_adc.adc_1) / 9.3 + 25;
        } else {
            *adc_val = *adc_val;
        }

    } else {
        read_status = STATUS_INVALID_PARAM;
    }

    return read_status;
}

void sadc_channel_enable(sadc_channel_config_t *config_channel) {
    volatile sadc_pnsel_ch_t *sadc_pnsel_ch;
    volatile sadc_set_ch_t *sadc_set_ch;
    volatile sadc_thd_ch_t *sadc_thd_ch;
    sadc_config_input_t ch;


    sadc_pnsel_ch = &(SADC->sadc_pnsel_ch0);
    sadc_set_ch = &(SADC->sadc_set_ch0);
    sadc_thd_ch = &(SADC->sadc_thd_ch0);


    //Ain0-2 = GPIO21-23, Ain4-7  = GPIO28-31
    if ( (config_channel->pi_sel < SADC_AIN_8) || (config_channel->ni_sel < SADC_AIN_8)) {
        if((config_channel->pi_sel < SADC_AIN_8)) {
            ch = config_channel->pi_sel;
        } else {
            ch = config_channel->ni_sel;
        }

        /* GPIO corresponding to AIO need no pull */
        if( ch <= SADC_AIN_2 ) {
            pin_set_pullopt( (ch + 21), PULL_NONE);
        } else {
            pin_set_pullopt( ((ch - 4) + 28), PULL_NONE);
        }
        /* enable AIO */
        sadc_aio_enable(ch);
    }

    sadc_pnsel_ch->bit.cfg_sadc_psel_ch = config_channel->pi_sel;
    sadc_pnsel_ch->bit.cfg_sadc_nsel_ch = config_channel->ni_sel;
    sadc_pnsel_ch->bit.cfg_sadc_gain_ch = config_channel->gain;
    sadc_pnsel_ch->bit.cfg_sadc_pull_ch = config_channel->pull;
    sadc_pnsel_ch->bit.cfg_sadc_ref_in_ch  = 1;

    sadc_pnsel_ch->bit.cfg_sadc_tacq_ch = config_channel->tacq;
    sadc_pnsel_ch->bit.cfg_sadc_edly_ch = config_channel->edly;

    sadc_set_ch->bit.cfg_sadc_burst_ch = config_channel->burst;


    sadc_thd_ch->bit.cfg_sadc_lthd_ch = config_channel->low_thd;
    sadc_thd_ch->bit.cfg_sadc_hthd_ch = config_channel->high_thd;

    /*
    sadc_pnsel_ch->bit.CFG_SADC_PSEL_CH = 7;
    sadc_pnsel_ch->bit.CFG_SADC_NSEL_CH = 15;
    sadc_pnsel_ch->bit.CFG_SADC_GAIN_CH = 2;
    sadc_pnsel_ch->bit.CFG_SADC_REF_IN_CH  = 1;
    sadc_pnsel_ch->bit.CFG_SADC_PULL_CH = 0;
    */

    return;
}

void sadc_channel_disable(sadc_config_channel_t ch_sel) {
    volatile sadc_pnsel_ch_t *sadc_pnsel_ch;
    volatile sadc_set_ch_t *sadc_set_ch;
    volatile sadc_thd_ch_t *sadc_thd_ch;

    sadc_pnsel_ch = &(SADC->sadc_pnsel_ch0);
    sadc_set_ch = &(SADC->sadc_set_ch0);
    sadc_thd_ch = &(SADC->sadc_thd_ch0);

    sadc_pnsel_ch->reg = SADC_PNSEL_CH_REG_DEFAULT;
    sadc_set_ch->reg = SADC_SET_CH_REG_DEFAULT;
    sadc_thd_ch->reg = SADC_THD_CH_REG_DEFAULT;

    return;
}

void sadc_vbat_enable(sadc_channel_config_t* config_channel) {
    volatile sadc_pnsel_ch_t *sadc_pnsel_ch;
    volatile sadc_set_ch_t *sadc_set_ch;
    volatile sadc_thd_ch_t *sadc_thd_ch;

    sadc_pnsel_ch = &(SADC->sadc_pnsel_ch0);
    sadc_set_ch = &(SADC->sadc_set_ch0);
    sadc_thd_ch = &(SADC->sadc_thd_ch0);

    sadc_pnsel_ch->bit.cfg_sadc_psel_ch = config_channel->pi_sel;
    sadc_pnsel_ch->bit.cfg_sadc_nsel_ch = config_channel->ni_sel;
    sadc_pnsel_ch->bit.cfg_sadc_gain_ch = config_channel->gain;
    sadc_pnsel_ch->bit.cfg_sadc_pull_ch = config_channel->pull;
    sadc_pnsel_ch->bit.cfg_sadc_ref_in_ch  = 1;

    sadc_pnsel_ch->bit.cfg_sadc_tacq_ch = config_channel->tacq;
    sadc_pnsel_ch->bit.cfg_sadc_edly_ch = config_channel->edly;

    sadc_set_ch->bit.cfg_sadc_burst_ch = config_channel->burst;

    sadc_thd_ch->bit.cfg_sadc_lthd_ch = config_channel->low_thd;
    sadc_thd_ch->bit.cfg_sadc_hthd_ch = config_channel->high_thd;

    /*
    sadc_pnsel_ch->bit.CFG_SADC_PSEL_CH = 10;
    sadc_pnsel_ch->bit.CFG_SADC_NSEL_CH = 15;
    sadc_pnsel_ch->bit.CFG_SADC_GAIN_CH = 7;
    sadc_pnsel_ch->bit.CFG_SADC_REF_IN_CH  = 1;
    sadc_pnsel_ch->bit.CFG_SADC_PULL_CH = 3;
    */
    return;
}

void sadc_vbat_disable(void) {
    volatile sadc_pnsel_ch_t *sadc_pnsel_ch;
    volatile sadc_set_ch_t *sadc_set_ch;
    volatile sadc_thd_ch_t *sadc_thd_ch;

    sadc_pnsel_ch = &(SADC->sadc_pnsel_ch0);
    sadc_set_ch = &(SADC->sadc_set_ch0);
    sadc_thd_ch = &(SADC->sadc_thd_ch0);

    sadc_pnsel_ch->reg = SADC_PNSEL_CH_REG_DEFAULT;
    sadc_set_ch->reg = SADC_SET_CH_REG_DEFAULT;
    sadc_thd_ch->reg = SADC_THD_CH_REG_DEFAULT;

    return;
}

void sadc_temp_enable(sadc_channel_config_t* config_channel) {
    volatile sadc_pnsel_ch_t *sadc_pnsel_ch;
    volatile sadc_set_ch_t *sadc_set_ch;
    volatile sadc_thd_ch_t *sadc_thd_ch;

    sadc_pnsel_ch = &(SADC->sadc_pnsel_ch0);
    sadc_set_ch = &(SADC->sadc_set_ch0);
    sadc_thd_ch = &(SADC->sadc_thd_ch0);

    sadc_pnsel_ch->bit.cfg_sadc_psel_ch = config_channel->pi_sel;
    sadc_pnsel_ch->bit.cfg_sadc_nsel_ch = config_channel->ni_sel;
    sadc_pnsel_ch->bit.cfg_sadc_gain_ch = config_channel->gain;
    sadc_pnsel_ch->bit.cfg_sadc_pull_ch = config_channel->pull;
    sadc_pnsel_ch->bit.cfg_sadc_ref_in_ch  = 0;

    sadc_pnsel_ch->bit.cfg_sadc_tacq_ch = config_channel->tacq;
    sadc_pnsel_ch->bit.cfg_sadc_edly_ch = config_channel->edly;

    sadc_set_ch->bit.cfg_sadc_burst_ch = config_channel->burst;


    sadc_thd_ch->bit.cfg_sadc_lthd_ch = config_channel->low_thd;
    sadc_thd_ch->bit.cfg_sadc_hthd_ch = config_channel->high_thd;

    /*
    sadc_pnsel_ch->bit.CFG_SADC_PSEL_CH = 8;
    sadc_pnsel_ch->bit.CFG_SADC_NSEL_CH = 8;
    sadc_pnsel_ch->bit.CFG_SADC_GAIN_CH = 3;
    sadc_pnsel_ch->bit.CFG_SADC_PULL_CH = 0;
    sadc_pnsel_ch->bit.CFG_SADC_REF_IN_CH  = 0;
    */
    return;
}

void sadc_temp_disable(void) {
    volatile sadc_pnsel_ch_t *sadc_pnsel_ch;
    volatile sadc_set_ch_t *sadc_set_ch;
    volatile sadc_thd_ch_t *sadc_thd_ch;

    sadc_pnsel_ch = &(SADC->sadc_pnsel_ch0);
    sadc_set_ch = &(SADC->sadc_set_ch0);
    sadc_thd_ch = &(SADC->sadc_thd_ch0);

    sadc_pnsel_ch->reg = SADC_PNSEL_CH_REG_DEFAULT;
    sadc_set_ch->reg = SADC_SET_CH_REG_DEFAULT;
    sadc_thd_ch->reg = SADC_THD_CH_REG_DEFAULT;

    return;
}

uint32_t sadc_init(sadc_config_t *p_config, sadc_proc_cb sadc_int_callback) {
    if (p_config == NULL) {
        return STATUS_INVALID_PARAM;
    }

    sadc_xdma_single_mode = DISABLE;
    sadc_convert_state = SADC_CONVERT_IDLE;

    SADC->sadc_ctl1.bit.cfg_sadc_rst = ENABLE;  /*Reset SADC*/

    sadc_analog_aio_init();
    //Sadc_Analog_Vbat_Init();

    /* Set SADC resolution bit */
    SADC->sadc_set1.bit.cfg_sadc_bit = p_config->sadc_resolution;
    /* Set SADC oversample rate */
    SADC->sadc_set1.bit.cfg_sadc_osr = p_config->sadc_oversample;

    if (p_config->sadc_xdma.enable == ENABLE) {
        sadc_xdma_config(p_config->sadc_xdma.start_addr, p_config->sadc_xdma.seg_size, p_config->sadc_xdma.blk_size);

        if (p_config->sadc_xdma.blk_size == 0) {
            sadc_xdma_single_mode = ENABLE;
        }
    }

    if (sadc_int_callback != NULL) {
        sadc_register_int_callback(sadc_int_callback);
    }
    sadc_int_enable(p_config->sadc_int_mask.reg);

    SADC->sadc_set0.bit.cfg_sadc_smp_mode = p_config->sadc_sample_mode;

    /*Sample rate depends on timer rate*/
    if (p_config->sadc_sample_mode == SADC_SAMPLE_TIMER) {
        SADC->sadc_set0.bit.cfg_sadc_tmr_cksel =
            p_config->sadc_timer.timer_clk_src;
        SADC->sadc_set0.bit.cfg_sadc_tmr_ckdiv =
            p_config->sadc_timer.timer_clk_div;
    }

    //for analog test
    SADC->sadc_set1.bit.cfg_sadc_tst = 12;
    SADC->sadc_set1.bit.cfg_sadc_chx_sel = 0;
    SADC->sadc_ctl0.bit.cfg_sadc_ck_free = 1;

#if (SADC_TEST_MODE == 1)
    SADC->sadc_set1.bit.cfg_sadc_tst = ENABLE;
    SADC->sadc_set1.bit.cfg_sadc_val_tst = SADC_TEST_VALUE;
#elif (SADC_CALIBRATION_VALUE != 0)
    SADC->sadc_set1.bit.cfg_sadc_val_tst = SADC_CALIBRATION_VALUE;
#endif

    return STATUS_SUCCESS;
}

void sadc_config_enable(sadc_config_resolution_t res, sadc_config_oversample_t os, sadc_proc_cb sadc_int_callback) {
    sadc_config_t p_sadc_config;

    sadc_calibration_init();

    //=== Sadc config backup ===
    sadc_config_res = res;
    sadc_config_os = os;
    sadc_config_int_callback = sadc_int_callback;

    //=== Sadc_Config(&p_sadc_config); start ===
    p_sadc_config.sadc_int_mask.bit.done = 1;                         /*Set SADC interrupt mask*/
    p_sadc_config.sadc_int_mask.bit.monitor_low = 1;
    p_sadc_config.sadc_int_mask.bit.monitor_high = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.monitor_low = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.valid = 0;
    p_sadc_config.sadc_int_mask.bit.wdma = 1;
    p_sadc_config.sadc_int_mask.bit.wdma_error = 1;

    p_sadc_config.sadc_resolution = res;                              /*Set SADC resolution bit*/

    p_sadc_config.sadc_oversample = os;                               /*Set SADC oversample rate*/

    p_sadc_config.sadc_xdma.enable = DISABLE;
    p_sadc_config.sadc_xdma.start_addr = (uint32_t)&sadc_ch_value;
    p_sadc_config.sadc_xdma.seg_size = 1;
    p_sadc_config.sadc_xdma.blk_size = 0;

    p_sadc_config.sadc_sample_mode = SADC_SAMPLE_START;               /*Sample rate depends on start trigger*/
    //p_sadc_config.sadc_sample_mode = SADC_SAMPLE_TIMER;
    //=== Sadc_Config(&p_sadc_config); end ===

    sadc_init(&p_sadc_config, sadc_int_callback);

    sadc_enable();       /*Enable SADC*/
}

uint32_t sadc_vbat_init(sadc_config_t *p_config, sadc_proc_cb sadc_int_callback) {
    if (p_config == NULL) {
        return STATUS_INVALID_PARAM;
    }

    sadc_xdma_single_mode = DISABLE;
    sadc_convert_state = SADC_CONVERT_IDLE;

    SADC->sadc_ctl1.bit.cfg_sadc_rst = ENABLE; /*Reset SADC*/

    sadc_analog_vbat_init();

    /* Set SADC resolution bit */
    SADC->sadc_set1.bit.cfg_sadc_bit = p_config->sadc_resolution;
    /* Set SADC oversample rate */
    SADC->sadc_set1.bit.cfg_sadc_osr = p_config->sadc_oversample;

    if (p_config->sadc_xdma.enable == ENABLE) {
        sadc_xdma_config(p_config->sadc_xdma.start_addr, p_config->sadc_xdma.seg_size, p_config->sadc_xdma.blk_size);

        if (p_config->sadc_xdma.blk_size == 0) {
            sadc_xdma_single_mode = ENABLE;
        }
    }

    if (sadc_int_callback != NULL) {
        sadc_register_int_callback(sadc_int_callback);
    }
    sadc_int_enable(p_config->sadc_int_mask.reg);

    SADC->sadc_set0.bit.cfg_sadc_smp_mode = p_config->sadc_sample_mode;

    /*Sample rate depends on timer rate*/
    if (p_config->sadc_sample_mode == SADC_SAMPLE_TIMER) {
        SADC->sadc_set0.bit.cfg_sadc_tmr_cksel =
            p_config->sadc_timer.timer_clk_src;
        SADC->sadc_set0.bit.cfg_sadc_tmr_ckdiv =
            p_config->sadc_timer.timer_clk_div;
    }

    //for analog test
    //SADC->SADC_SET1.bit.CFG_SADC_TST = 12;
    SADC->sadc_set1.bit.cfg_sadc_tst = 8;
    SADC->sadc_set1.bit.cfg_sadc_chx_sel = 0;
    SADC->sadc_ctl0.bit.cfg_sadc_ck_free = 1;

    return STATUS_SUCCESS;
}

void sadc_vbat_config_enable(sadc_config_resolution_t res, sadc_config_oversample_t os, sadc_proc_cb sadc_int_callback) {
    sadc_config_t p_sadc_config;

    sadc_calibration_init();

    //=== Sadc config backup ===
    sadc_config_res = res;
    sadc_config_os = os;
    sadc_config_int_callback = sadc_int_callback;

    //=== Sadc_Config(&p_sadc_config); start ===
    p_sadc_config.sadc_int_mask.bit.done = 1;                         /*Set SADC interrupt mask*/
    p_sadc_config.sadc_int_mask.bit.mode_done = 1;
    p_sadc_config.sadc_int_mask.bit.monitor_high = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.monitor_low = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.valid = 0;
    p_sadc_config.sadc_int_mask.bit.wdma = 1;
    p_sadc_config.sadc_int_mask.bit.wdma_error = 1;

    p_sadc_config.sadc_resolution = res;                              /*Set SADC resolution bit*/

    p_sadc_config.sadc_oversample = os;                               /*Set SADC oversample rate*/

    p_sadc_config.sadc_xdma.enable = DISABLE;
    p_sadc_config.sadc_xdma.start_addr = (uint32_t)&sadc_ch_value;
    p_sadc_config.sadc_xdma.seg_size = 1;
    p_sadc_config.sadc_xdma.blk_size = 0;

    p_sadc_config.sadc_sample_mode = SADC_SAMPLE_START;               /*Sample rate depends on start trigger*/
    //p_sadc_config.sadc_sample_mode = SADC_SAMPLE_TIMER;
    //=== Sadc_Config(&p_sadc_config); end ===

    sadc_vbat_init(&p_sadc_config, sadc_int_callback);

    sadc_enable();       /*Enable SADC*/
}


uint32_t sadc_temp_init(sadc_config_t *p_config, sadc_proc_cb sadc_int_callback) {
    if (p_config == NULL) {
        return STATUS_INVALID_PARAM;
    }

    sadc_xdma_single_mode = DISABLE;
    sadc_convert_state = SADC_CONVERT_IDLE;

    SADC->sadc_ctl1.bit.cfg_sadc_rst = ENABLE; /*Reset SADC*/

    sadc_analog_temp_init();

    /* Set SADC resolution bit */
    SADC->sadc_set1.bit.cfg_sadc_bit = p_config->sadc_resolution;
    /* Set SADC oversample rate */
    SADC->sadc_set1.bit.cfg_sadc_osr = p_config->sadc_oversample;

    if (p_config->sadc_xdma.enable == ENABLE) {
        sadc_xdma_config(p_config->sadc_xdma.start_addr, p_config->sadc_xdma.seg_size, p_config->sadc_xdma.blk_size);

        if (p_config->sadc_xdma.blk_size == 0) {
            sadc_xdma_single_mode = ENABLE;
        }
    }

    if (sadc_int_callback != NULL) {
        sadc_register_int_callback(sadc_int_callback);
    }
    sadc_int_enable(p_config->sadc_int_mask.reg);

    SADC->sadc_set0.bit.cfg_sadc_smp_mode = p_config->sadc_sample_mode;

    /*Sample rate depends on timer rate*/
    if (p_config->sadc_sample_mode == SADC_SAMPLE_TIMER) {
        SADC->sadc_set0.bit.cfg_sadc_tmr_cksel = p_config->sadc_timer.timer_clk_src;
        SADC->sadc_set0.bit.cfg_sadc_tmr_ckdiv = p_config->sadc_timer.timer_clk_div;
    }

    //for analog test
    //SADC->SADC_SET1.bit.CFG_SADC_TST = 12;
    SADC->sadc_set1.bit.cfg_sadc_tst = 8;
    SADC->sadc_set1.bit.cfg_sadc_chx_sel = 0;
    SADC->sadc_ctl0.bit.cfg_sadc_ck_free = 1;

    return STATUS_SUCCESS;
}

void sadc_temp_config_enable(sadc_config_resolution_t res, sadc_config_oversample_t os, sadc_proc_cb sadc_int_callback) {
    sadc_config_t p_sadc_config;

    sadc_calibration_init();

    //=== Sadc config backup ===
    sadc_config_res = res;
    sadc_config_os = os;
    sadc_config_int_callback = sadc_int_callback;

    //=== Sadc_Config(&p_sadc_config); start ===
    p_sadc_config.sadc_int_mask.bit.done = 1;                         /*Set SADC interrupt mask*/
    p_sadc_config.sadc_int_mask.bit.mode_done = 1;
    p_sadc_config.sadc_int_mask.bit.monitor_high = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.monitor_low = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.valid = 0;
    p_sadc_config.sadc_int_mask.bit.wdma = 1;
    p_sadc_config.sadc_int_mask.bit.wdma_error = 1;

    p_sadc_config.sadc_resolution = res;                              /*Set SADC resolution bit*/

    p_sadc_config.sadc_oversample = os;                               /*Set SADC oversample rate*/

    p_sadc_config.sadc_xdma.enable = DISABLE;
    p_sadc_config.sadc_xdma.start_addr = (uint32_t)&sadc_ch_value;
    p_sadc_config.sadc_xdma.seg_size = 1;
    p_sadc_config.sadc_xdma.blk_size = 0;

    p_sadc_config.sadc_sample_mode = SADC_SAMPLE_START;               /*Sample rate depends on start trigger*/
    //p_sadc_config.sadc_sample_mode = SADC_SAMPLE_TIMER;
    //=== Sadc_Config(&p_sadc_config); end ===

    sadc_temp_init(&p_sadc_config, sadc_int_callback);

    sadc_enable();       /*Enable SADC*/
}

void sadc_disable(void) {

    sadc_int_disable();
    sadc_register_int_callback(NULL);

    SADC->sadc_ctl0.bit.cfg_sadc_ena = DISABLE;
    SADC->sadc_ctl0.bit.cfg_sadc_ldo_ena = DISABLE;
    SADC->sadc_ctl0.bit.cfg_sadc_vga_ena = DISABLE;

    return;
}

void sadc_enable(void) {
    SADC->sadc_ctl0.bit.cfg_sadc_ena = ENABLE;
    SADC->sadc_ctl0.bit.cfg_sadc_ldo_ena = ENABLE;
    SADC->sadc_ctl0.bit.cfg_sadc_vga_ena = ENABLE;
    return;
}

void sadc_start(void) {
    if (sadc_xdma_single_mode == ENABLE) {
        SADC->sadc_wdma_ctl0.bit.cfg_sadc_wdma_ctl0 = ENABLE;
    }

    sadc_convert_state = SADC_CONVERT_START;

    SADC->sadc_ctl2.bit.cfg_sadc_start = ENABLE;

    return;
}

sadc_convert_state_t sadc_convert_state_get(void) {
    return sadc_convert_state;
}

uint32_t sadc_channel_read(sadc_input_ch_t ch) {
    uint32_t read_status;

    enter_critical_section();

    if (sadc_ch_read_convert_state != SADC_CONVERT_START) {
        sadc_ch_read_convert_state = SADC_CONVERT_START;

        leave_critical_section();

        sadc_convert_ch = ch;
        sadc_config_enable(sadc_config_res, sadc_config_os, sadc_config_int_callback);
        sadc_channel_enable(&sadc_ch_init[ch]);
        delay_ms(10);
        sadc_start();        /*Start to trigger SADC*/

        read_status = STATUS_SUCCESS;
    } else {
        leave_critical_section();

        read_status = STATUS_EBUSY;
    }

    return read_status;
}

uint32_t sadc_vbat_read(void) {
    uint32_t read_status;

    enter_critical_section();

    if (sadc_ch_read_convert_state != SADC_CONVERT_START) {
        sadc_ch_read_convert_state = SADC_CONVERT_START;

        leave_critical_section();

        sadc_convert_ch = SADC_CH_VBAT;
        sadc_vbat_config_enable(sadc_config_res, sadc_config_os, sadc_config_int_callback);
        sadc_vbat_enable(&sadc_ch_init[SADC_CH_VBAT]);
        delay_ms(10);
        sadc_start();        /*Start to trigger SADC*/

        read_status = STATUS_SUCCESS;
    } else {
        leave_critical_section();

        read_status = STATUS_EBUSY;
    }

    return read_status;
}

uint32_t sadc_temp_read(void) {
    uint32_t read_status;

    enter_critical_section();

    if (sadc_ch_read_convert_state != SADC_CONVERT_START) {
        sadc_ch_read_convert_state = SADC_CONVERT_START;

        leave_critical_section();

        sadc_convert_ch = SADC_CH_TEMPERATURE;
        //callback may change
        sadc_temp_config_enable(sadc_config_res, sadc_config_os, sadc_config_int_callback);
        sadc_temp_enable(&sadc_ch_init[SADC_CH_TEMPERATURE]);
        delay_ms(10);
        sadc_start();        /*Start to trigger SADC*/

        read_status = STATUS_SUCCESS;
    } else {
        leave_critical_section();

        read_status = STATUS_EBUSY;
    }

    return read_status;
}

int sadc_voltage_result(sadc_value_t sadc_value)
{
    int value, thousund_val, hundred_val;
    int ten_val __attribute__((unused));
    int unit_val __attribute__((unused));
    int value_max, value_min;

    thousund_val = 0;
    hundred_val = 0;
    value = 0;


    thousund_val = (sadc_value / 1000) % 10;
    hundred_val = (sadc_value / 100) % 10;

    value_max = (thousund_val * 1000) + (hundred_val * 100) + 51;
    value_min = (thousund_val * 1000) + (hundred_val * 100) - 49;

    if (sadc_value <= 0) {
        value = 0;
    } else if ((sadc_value < value_max) && (sadc_value >= value_min))
    {
        value = (thousund_val * 1000) + (hundred_val * 100) ; // + (ten_val * 10);
    } else if ( sadc_value >= value_max) {
        value = (thousund_val * 1000) + ((hundred_val + 1) * 100);
    } else if ( sadc_value < value_min) {
        value = (thousund_val * 1000) + ((hundred_val - 1) * 100);
    }

    return value;
}
                    
/**
 * \brief           SADC interrupt handler
 */
void sadc_handler(void) {
    sadc_cb_t cb;
    sadc_int_t reg_sadc_int_status;
    sadc_value_t  sadc_value;
    sadc_cal_type_t cal_type = 0xFF;

    reg_sadc_int_status.reg = SADC->sadc_int_status.reg;
    SADC->sadc_int_clear.reg = reg_sadc_int_status.reg;

    if (reg_sadc_int_status.reg != 0) {
        if (reg_sadc_int_status.bit.done == 1) {
        }

        if (reg_sadc_int_status.bit.mode_done == 1) {
            if (SADC->sadc_set0.bit.cfg_sadc_smp_mode == SADC_SAMPLE_START) {
                sadc_convert_state = SADC_CONVERT_DONE;
            }
        }

        if (reg_sadc_int_status.bit.valid == 1) {
            cb.type = SADC_CB_SAMPLE;
            sadc_value = SADC->sadc_r0.bit.sadc_o;

            /* Need to compensation and calibration */
            if (sadc_convert_ch <= SADC_CH_AIN7)
            {
                cal_type = SADC_CALIBRATION_AIO;
            }
            else if (sadc_convert_ch == SADC_CH_VBAT)
            {
                cal_type = SADC_CALIBRATION_VBAT;
            }
            else if (sadc_convert_ch == SADC_CH_TEMPERATURE)
            {
                cal_type = SADC_CALIBRATION_TEMP;
                sadc_resolution_compensation_temp(&sadc_value);
                cb.raw.conversion_value = sadc_value;
            }
            sadc_calibration(cal_type, &sadc_value);
            cb.raw.calibration_value = sadc_value;

            if (sadc_convert_ch <= SADC_CH_AIN7)
            {
                if ( (int)sadc_value < 0 )
                {
                    sadc_value = 0;
                }
            }
            else if (sadc_convert_ch == SADC_CH_VBAT)
            {
                if ( (int)sadc_value < 0 )
                {
                    sadc_value = 0;
                }
            }

            cb.data.sample.value = sadc_value;
            cb.data.sample.channel = sadc_convert_ch;
            sadc_reg_handler(&cb);

            sadc_ch_read_convert_state = SADC_CONVERT_DONE;
        }

        if (reg_sadc_int_status.bit.wdma == 1) {
        }

        if (reg_sadc_int_status.bit.wdma_error == 1) {
        }
    }
}
