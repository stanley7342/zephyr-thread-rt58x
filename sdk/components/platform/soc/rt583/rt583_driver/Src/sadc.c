/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            sadc.c
 * \brief           SADC driver file
 */
/*
 * Author:          Kc.tseng
 */

#include <stdio.h>
#include <string.h>

#include "mcu.h"
#include "mp_sector.h"

#include "FreeRTOS.h"
#include "timers.h"

/**
 * \brief           Sadc test mode value define
 */
#ifdef CONFIG_SADC_TEST_MODE
#define SADC_TEST_VALUE 0x5AF
#endif

/**
 * \brief           Sadc channel setting value
 */
/* VGA gain selection, [3:0] +3dB/step, [5:4] +6dB/step */
#define SADC_GAIN_AIN 0x14
/* Channel pull selection, [0] P-CH pull high, [1] P-CH pull low, [2] N-CH pull high, [3] N-CH pull low */
#define SADC_PULL_AIN 0x00
/* VGA gain selection, [3:0] +3dB/step, [5:4] +6dB/step */
#define SADC_GAIN_VBAT 0x14
/* Channel pull selection, [0] P-CH pull high, [1] P-CH pull low, [2] N-CH pull high, [3] N-CH pull low */
#define SADC_PULL_VBAT 0x03
/* VGA gain selection, [3:0] +3dB/step, [5:4] +6dB/step */
#define SADC_GAIN_VCM 0x14
/* Channel pull selection, [0] P-CH pull high, [1] P-CH pull low, [2] N-CH pull high, [3] N-CH pull low */
#define SADC_PULL_VCM 0x00
/* VGA gain selection, [3:0] +3dB/step, [5:4] +6dB/step */
#define SADC_GAIN_TEMPERATURE 0x14
/* Channel pull selection, [0] P-CH pull high, [1] P-CH pull low, [2] N-CH pull high, [3] N-CH pull low */
#define SADC_PULL_TEMPERATURE 0x00
/* Channel pull selection, P-CH pull low, N-CH no pull*/
#define SADC_PULL_OVADC 0x02

/**
 * \brief           Sadc temperature calibration setting
 */
#define TEMPERATURE_CALIBRATION_CONST 111820
#define TEMPERATURE_CALIBRATION_RATIO 1000

static sadc_cb_fn sadc_reg_handler = NULL;
static sadc_cb_fn sadc_adcomp_handler = NULL;
static sadc_cb_fn sadc_txcomp_handler = NULL;
static uint32_t sadc_xdma_single_mode = DISABLE;
static sadc_convert_state_t sadc_convert_state = SADC_CONVERT_IDLE;
static sadc_convert_state_t sadc_ch_read_convert_state = SADC_CONVERT_IDLE;
static int32_t sadc_compensation_offset = 0;
static int32_t sadc_temperature_calibration_offset = 0;

static int32_t cvbat = 0;
static int32_t c0vadc = 0;
static int32_t vbatadc = 0;

static sadc_input_ch_t sadc_convert_ch = SADC_CH_NC;
static sadc_value_t sadc_ch_value;

sadc_channel_config_t sadc_ch_init[] = {
    {SADC_CHANNEL_0, SADC_AIN_0, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_1, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_2, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_3, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_4, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_5, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_6, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_AIN_7, SADC_AIN_DISABLED, SADC_GAIN_AIN,
     SADC_PULL_AIN, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_VBAT, SADC_AIN_DISABLED, SADC_GAIN_VBAT,
     SADC_PULL_VBAT, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_VBAT, SADC_AIN_DISABLED, SADC_GAIN_VBAT,
     SADC_PULL_VBAT, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_TEMPERATURE, SADC_TEMPERATURE, SADC_GAIN_TEMPERATURE,
     SADC_PULL_TEMPERATURE, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT},
    {SADC_CHANNEL_0, SADC_VBAT, SADC_VBAT, SADC_GAIN_VCM, SADC_PULL_VCM,
     SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US, SADC_BURST_ENABLE,
     SADC_MONITOR_LOW_THD_DEFAULT, SADC_MONITOR_HIGH_THD_DEFAULT},
    /*0V ADC Check for temperature sensor*/
    {SADC_CHANNEL_0, SADC_VBAT_0VADC, SADC_AIN_DISABLED, SADC_GAIN_VBAT,
     SADC_PULL_OVADC, SADC_TACQ_EDLY_TIME_16US, SADC_TACQ_EDLY_TIME_16US,
     SADC_BURST_ENABLE, SADC_MONITOR_LOW_THD_DEFAULT,
     SADC_MONITOR_HIGH_THD_DEFAULT}};

TimerHandle_t sadc_comp_timer_handle = NULL;

static sadc_config_resolution_t sadc_config_res = SADC_RES_12BIT;
static sadc_config_oversample_t sadc_config_os = SADC_OVERSAMPLE_256;
static sadc_cb_fn sadc_config_int_callback = NULL;

/**
 * \brief           SADC analog initinal
 */
void sadc_analog_init(void) {
    SADC->ana_set0.bit.cfg_aux_ana_set0 = 0x7F708;

    SADC->ana_set1.bit.cfg_aux_cmsel = 0;
    SADC->ana_set1.bit.cfg_aux_cmsel = 5;
    SADC->ana_set1.bit.cfg_aux_cmsel = 0;
    SADC->ana_set1.bit.cfg_aux_cmsel = 1;

    SADC->ana_set1.bit.cfg_aux_comp = 3;
    SADC->ana_set1.bit.cfg_aux_adc_outputstb = 0;
    SADC->ana_set1.bit.cfg_aux_test_mode = 0;
    SADC->ana_set1.bit.cfg_aux_vldo = 3;
    SADC->ana_set1.bit.cfg_aux_clk_sel = 0;

    SADC->ana_set1.bit.cfg_aux_pw = 0;
    SADC->ana_set1.bit.cfg_aux_pw = 36;
    SADC->ana_set1.bit.cfg_aux_pw = 0;
    SADC->ana_set1.bit.cfg_aux_pw = 36;

    SADC->ana_set1.bit.cfg_en_clkaux = ENABLE;
}

void sadc_register_int_callback(sadc_cb_fn sadc_int_callback) {
    sadc_reg_handler = sadc_int_callback;

    return;
}

void sadc_register_adcomp_int_callback(sadc_cb_fn sadc_comp_int_callback) {
    sadc_adcomp_handler = sadc_comp_int_callback;

    return;
}

void sadc_register_txcomp_int_callback(sadc_cb_fn sadc_txcomp_int_callback) {
    sadc_txcomp_handler = sadc_txcomp_int_callback;

    return;
}

void sadc_xdma_config(uint32_t xdma_start_addr, uint16_t xdma_seg_size,
                      uint16_t xdma_blk_size) {
    /* Reset XDMA */
    SADC->wdma_ctl1.bit.cfg_sadc_wdma_ctl1 = ENABLE;

    /* Clear XDMA IRQ */
    SADC->int_clear.bit.wdma = ENABLE;
    SADC->int_clear.bit.wdma_error = ENABLE;

    /* Enable XDMA IRQ */
    /*
    SADC->SADC_INT_MASK.bit.WDMA = 0;
    SADC->SADC_INT_MASK.bit.WDMA_ERROR = 0;
    */

    /* Set XDMA buffer address */
    SADC->wdma_set1 = xdma_start_addr;

    /* Set XDMA buffer size */
    SADC->wdma_set0.bit.cfg_sadc_seg_size = xdma_seg_size;
    SADC->wdma_set0.bit.cfg_sadc_blk_size = xdma_blk_size;

    /* Start XDMA for memory access */
    sadc_xdma_start();

    return;
}

uint32_t sadc_init(sadc_config_t* p_config, sadc_cb_fn sadc_int_callback) {
    if (p_config == NULL) {
        return STATUS_INVALID_PARAM;
    }

    sadc_xdma_single_mode = DISABLE;
    sadc_convert_state = SADC_CONVERT_IDLE;

    sadc_reset(); /*Reset SADC*/

    sadc_analog_init();

    sadc_set_resolution(p_config->sadc_resolution); /*Set SADC resolution bit*/

    sadc_set_oversample_rate(
        p_config->sadc_oversample); /*Set SADC oversample rate*/

    if (p_config->sadc_xdma.enable == ENABLE) {
        sadc_xdma_config(p_config->sadc_xdma.start_addr,
                         p_config->sadc_xdma.seg_size,
                         p_config->sadc_xdma.blk_size);

        if (p_config->sadc_xdma.blk_size == 0) {
            sadc_xdma_single_mode = ENABLE;
        }
    }

    if (sadc_int_callback != NULL) {
        sadc_register_int_callback(sadc_int_callback);
    }
    sadc_int_enable(p_config->sadc_int_mask.reg);

    sadc_set_sample_mode(
        p_config->sadc_sample_mode); /*Sample rate depends on timer rate*/
    if (p_config->sadc_sample_mode == SADC_SAMPLE_TIMER) {
        sadc_set_timer_sample_source(p_config->sadc_timer.timer_clk_src);
        sadc_set_timer_divide(p_config->sadc_timer.timer_clk_div);
    }

    sadc_manual_mode();
    sadc_vga_enable();
    sadc_ldo_enable();

    /* Delay 1ms for VGA settle */
    delay_ms(1);

#ifdef CONFIG_SADC_TEST_MODE
    SADC->setting1.bit.cfg_sadc_tst = 1;
    SADC->setting1.bit.cfg_sadc_val_tst = SADC_TEST_VALUE;
    printf("SADC_TEST_VALUE %x\r\n", SADC_TEST_VALUE);
#endif
#ifdef CONFIG_SADC_CALIBRATION_VALUE
    SADC->setting1.bit.cfg_sadc_val_tst = CONFIG_SADC_CALIBRATION_VALUE;
#endif

    return STATUS_SUCCESS;
}

void sadc_int_enable(uint32_t int_mask) {
    SADC->int_clear.reg = SADC_INT_CLEAR_ALL;
    SADC->int_mask.reg = int_mask;
    return;
}

void sadc_int_disable(void) {
    SADC->int_mask.reg = SADC_INT_DISABLE_ALL;
    SADC->int_clear.reg = SADC_INT_CLEAR_ALL;
    return;
}

void sadc_channel_enable(sadc_channel_config_t* config_channel) {
    volatile sadc_pnsel_ch_t* sadc_pnsel_ch;
    volatile sadc_set_ch_t* sadc_set_ch;
    volatile sadc_thd_ch_t* sadc_thd_ch;
    uint32_t gpio_pull_ctrl_bit = 0;

    sadc_pnsel_ch = &(SADC->pnsel_ch0)
                    + (config_channel->ch_sel * SADC_CH_REG_OFFSET);
    sadc_set_ch = &(SADC->set_ch0)
                  + (config_channel->ch_sel * SADC_CH_REG_OFFSET);
    sadc_thd_ch = &(SADC->thd_ch0)
                  + (config_channel->ch_sel * SADC_CH_REG_OFFSET);

    if (config_channel->pi_sel < SADC_AIN_8) {
        gpio_pull_ctrl_bit = config_channel->pi_sel * 4;
        /* no pull */
        SYSCTRL->gpio_pull_ctrl[3] &= (~(0x00000007 << gpio_pull_ctrl_bit));
        /* enable AIO */
        SYSCTRL->gpio_aio_ctrl |= (0x00000001 << config_channel->pi_sel);
    }

    if (config_channel->ni_sel < SADC_AIN_8) {
        gpio_pull_ctrl_bit = config_channel->ni_sel * 4;
        /* no pull */
        SYSCTRL->gpio_pull_ctrl[3] &= (~(0x00000007 << gpio_pull_ctrl_bit));

        /* enable AIO */
        SYSCTRL->gpio_aio_ctrl |= (0x00000001 << config_channel->ni_sel);
    }

    sadc_pnsel_ch->bit.cfg_sadc_psel_ch = config_channel->pi_sel;
    sadc_pnsel_ch->bit.cfg_sadc_nsel_ch = config_channel->ni_sel;
    sadc_pnsel_ch->bit.cfg_sadc_gain_ch = config_channel->gain;

    /* N channel = P channel = Vbat, VCM ADC */
    if ((config_channel->pi_sel == SADC_VBAT)
        && (config_channel->ni_sel == SADC_VBAT)) {
        /* N/P channel = adc_ldo */
        sadc_pnsel_ch->bit.cfg_sadc_ref_in_ch = 3;
    } else if ((config_channel->pi_sel == SADC_TEMPERATURE)
               && (config_channel->ni_sel == SADC_TEMPERATURE)) {
        /* N channel = P channel = Temperature Sensor */
        /* adc ref not from ADC_LDO output */
        sadc_pnsel_ch->bit.cfg_sadc_ref_in_ch = 0;
        sadc_temperature_enable();
    } else {
        /* adc ref from ADC_LDO output */
        sadc_pnsel_ch->bit.cfg_sadc_ref_in_ch = 1;
    }

    sadc_pnsel_ch->bit.cfg_sadc_pull_ch = config_channel->pull;
    sadc_pnsel_ch->bit.cfg_sadc_tacq_ch = config_channel->tacq;
    sadc_pnsel_ch->bit.cfg_sadc_edly_ch = config_channel->edly;

    sadc_set_ch->bit.cfg_sadc_burst_ch = config_channel->burst;

    sadc_thd_ch->bit.cfg_sadc_lthd_ch = config_channel->low_thd;
    sadc_thd_ch->bit.cfg_sadc_hthd_ch = config_channel->high_thd;

    return;
}

void sadc_channel_disable(sadc_config_channel_t ch_sel) {
    volatile sadc_pnsel_ch_t* sadc_pnsel_ch;
    volatile sadc_set_ch_t* sadc_set_ch;
    volatile sadc_thd_ch_t* sadc_thd_ch;

    sadc_pnsel_ch = &(SADC->pnsel_ch0) + (ch_sel * SADC_CH_REG_OFFSET);
    sadc_set_ch = &(SADC->set_ch0) + (ch_sel * SADC_CH_REG_OFFSET);
    sadc_thd_ch = &(SADC->thd_ch0) + (ch_sel * SADC_CH_REG_OFFSET);

    sadc_pnsel_ch->reg = SADC_PNSEL_CH_REG_DEFAULT;
    sadc_set_ch->reg = SADC_SET_CH_REG_DEFAULT;
    sadc_thd_ch->reg = SADC_THD_CH_REG_DEFAULT;

    return;
}

void sadc_disable(void) {
    SADC->control0.bit.cfg_sadc_ena = DISABLE;
    SADC->control0.bit.cfg_sadc_ldo_ena = DISABLE;
    SADC->control0.bit.cfg_sadc_vga_ena = DISABLE;

    sadc_int_disable();
    sadc_register_int_callback(NULL);

    return;
}

void sadc_enable(void) {

    SADC->control0.bit.cfg_sadc_ena = ENABLE;

    return;
}

void sadc_start(void) {
    if (sadc_xdma_single_mode == ENABLE) {
        /* Start SADC xdma */
        SADC->wdma_ctl0.bit.cfg_sadc_wdma_ctl0 = ENABLE;
    }

    sadc_convert_state = SADC_CONVERT_START;

    /* Start to trigger SADC */
    SADC->control2.bit.cfg_sadc_start = ENABLE;

    return;
}

void sadc_config_enable(sadc_config_resolution_t res,
                        sadc_config_oversample_t os,
                        sadc_cb_fn sadc_int_callback) {
    sadc_config_t p_sadc_config;

    sadc_calibration_init();

    //=== Sadc config backup ===
    sadc_config_res = res;
    sadc_config_os = os;
    sadc_config_int_callback = sadc_int_callback;

    //=== Sadc_Config(&p_sadc_config); start ===
    p_sadc_config.sadc_int_mask.bit.done = 1; /*Set SADC interrupt mask*/
    p_sadc_config.sadc_int_mask.bit.mode_done = 1;
    p_sadc_config.sadc_int_mask.bit.monitor_high = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.monitor_low = 0x3FF;
    p_sadc_config.sadc_int_mask.bit.valid = 0;
    p_sadc_config.sadc_int_mask.bit.wdma = 1;
    p_sadc_config.sadc_int_mask.bit.wdma_error = 1;

    p_sadc_config.sadc_resolution = res; /*Set SADC resolution bit*/

    p_sadc_config.sadc_oversample = os; /*Set SADC oversample rate*/

    p_sadc_config.sadc_xdma.enable = DISABLE;
    p_sadc_config.sadc_xdma.start_addr = (uint32_t)&sadc_ch_value;
    p_sadc_config.sadc_xdma.seg_size = 1;
    p_sadc_config.sadc_xdma.blk_size = 0;

    p_sadc_config.sadc_sample_mode =
        SADC_SAMPLE_START; /*Sample rate depends on start trigger*/
    //=== Sadc_Config(&p_sadc_config); end ===

    sadc_init(&p_sadc_config, sadc_int_callback);

    sadc_enable(); /*Enable SADC*/
}

uint32_t sadc_channel_read(sadc_input_ch_t ch) {
    uint32_t read_status;

    enter_critical_section();

    if (sadc_ch_read_convert_state != SADC_CONVERT_START) {
        sadc_ch_read_convert_state = SADC_CONVERT_START;

        leave_critical_section();

        lpm_low_power_mask(LOW_POWER_MASK_BIT_TASK_ADC);
        sadc_convert_ch = ch;
        sadc_config_enable(sadc_config_res, sadc_config_os,
                           sadc_config_int_callback);
        sadc_channel_enable(&sadc_ch_init[ch]);
        sadc_start(); /*Start to trigger SADC*/

        read_status = STATUS_SUCCESS;

    } else {
        leave_critical_section();
        read_status = STATUS_EBUSY;
    }

    return read_status;
}

uint32_t sadc_resolution_compensation(sadc_value_t* p_data) {
    uint32_t compensation_bit = 0;

    if (p_data == NULL) {
        return STATUS_INVALID_PARAM;
    }

    switch (sadc_get_resolution()) {
        case SADC_RES_8BIT: {
            compensation_bit = 6;
            break;
        }

        case SADC_RES_10BIT: {
            compensation_bit = 4;
            break;
        }

        case SADC_RES_12BIT: compensation_bit = 2; break;

        case SADC_RES_14BIT: break;

        default: break;
    }

    (*p_data) >>= compensation_bit;

    return STATUS_SUCCESS;
}

sadc_convert_state_t sadc_convert_state_get(void) { return sadc_convert_state; }

void sadc_calibration_init(void) { mpcaladcinit(); }

uint32_t sadc_calibration(sadc_cal_type_t cal_type, sadc_value_t adc_val) {
    uint32_t read_status;
    mp_cal_adc_t mp_cal_adc;
    int32_t cal_vol = 0;

    if (cal_type == SADC_CALIBRATION_VBAT) {
        read_status = mpcalvbatadcread(&mp_cal_adc);
    } else if (cal_type == SADC_CALIBRATION_AIO) {
        read_status = mpcalaioadcread(&mp_cal_adc);
    } else {
        read_status = STATUS_INVALID_PARAM;
    }

    if (read_status == STATUS_SUCCESS) {
        cal_vol = (adc_val - mp_cal_adc.adc_1);
        cal_vol *= (mp_cal_adc.voltage_2 - mp_cal_adc.voltage_1);
        cal_vol /= (mp_cal_adc.adc_2 - mp_cal_adc.adc_1);
        cal_vol += mp_cal_adc.voltage_1;
    }

    return cal_vol;
}

void sadc_temperature_enable(void) {
    PMU->pmu_ts.bit.ts_vx = 4;
    PMU->pmu_ts.bit.ts_s = 5;
    PMU->pmu_ts.bit.ts_en = ENABLE;
    delay_ms(1);
    PMU->pmu_ts.bit.ts_clk_sel = 1;
    PMU->pmu_ts.bit.ts_clk_en = ENABLE;
    delay_ms(1);
    PMU->pmu_ts.bit.ts_rst = ENABLE;
    delay_ms(1);
    PMU->pmu_ts.bit.ts_rst = ENABLE;
    delay_ms(5);
}

void sadc_temperature_disable(void) {
    PMU->pmu_ts.bit.ts_en = DISABLE;
    PMU->pmu_ts.bit.ts_clk_en = DISABLE;
}

uint32_t sadc_temperature_calibration(sadc_value_t adc_val) {
    int32_t cal_vol = 0;
    int32_t cal_m, cal_v, cal_k;
    int32_t cts = adc_val;

    sadc_temperature_calibration_offset_update(adc_val);

    if (sadc_temperature_calibration_offset != 0) {
        cal_m = vbatadc >> 1;
        cal_m = (cal_m * TEMPERATURE_CALIBRATION_RATIO) / (cvbat - c0vadc);
        cal_k = 0 - ((cal_m * c0vadc) / TEMPERATURE_CALIBRATION_RATIO);
        cal_v = (cal_m * cts) / TEMPERATURE_CALIBRATION_RATIO;
        cal_v = (cal_v + cal_k);
        cal_vol = (cal_v * TEMPERATURE_CALIBRATION_CONST)
                  / TEMPERATURE_CALIBRATION_RATIO;
        cal_vol = cal_vol / TEMPERATURE_CALIBRATION_RATIO;
        cal_vol = cal_vol + sadc_temperature_calibration_offset;
    }

    return cal_vol;
}

void sadc_temperature_calibration_offset_update(sadc_value_t adc_val) {
    uint32_t read_status;

    mp_temp_k_t mp_cal_tempk;

    read_status = mpcalktread(&mp_cal_tempk);

    if (read_status == STATUS_SUCCESS) {
        sadc_temperature_calibration_offset = mp_cal_tempk.ktvalue * (-1);
    } else {
        sadc_temperature_calibration_offset = 0;
    }
}

void sadc_compensation_int_handler(sadc_cb_t* p_cb) {
    uint32_t sadc_comp_input;
    sadc_value_t sadc_comp_value;

    if (p_cb->type == SADC_CB_SAMPLE) {
        sadc_comp_input = p_cb->data.sample.channel;
        sadc_comp_value = p_cb->data.sample.value;

        if (sadc_comp_input == SADC_COMP_VCM) {
#ifdef CONFIG_SADC_COMP_DEBUG
            gpio_pin_toggle(0);
            printf("\nVCM ADC = %d\n", sadc_comp_value);
#endif

            sadc_compensation_offset_update(sadc_comp_value);
        }
    }
}

void sadc_compensation_periodic_callback(TimerHandle_t pxTimer) {
    /* Optionally do something if the pxTimer parameter is NULL. */
    configASSERT(pxTimer);
    sadc_channel_read(SADC_COMP_VCM);
}

void sadc_compensation_init(uint32_t xPeriodicTimeInSec) {
    sadc_register_adcomp_int_callback(sadc_compensation_int_handler);

    sadc_comp_timer_handle = xTimerCreate(
        "Timer", pdMS_TO_TICKS(xPeriodicTimeInSec * 1000), pdTRUE, (void*)0,
        sadc_compensation_periodic_callback);

    if (sadc_comp_timer_handle == NULL) {
        /* The timer was not created. */
    } else {
        /* Start the timer.  No block time is specified, and
        even if one was it would be ignored because the RTOS
        scheduler has not yet been started. */
        if (xTimerStart(sadc_comp_timer_handle, 0) != pdPASS) {
            /* The timer could not be set into the Active state. */
        }
    }
}

void sadc_compensation_deinit(void) {
    if (sadc_comp_timer_handle != NULL) {
        if (xTimerDelete(sadc_comp_timer_handle, 0) == pdPASS) {
            /* The delete command was successfully sent to the timer command queue. */
            sadc_comp_timer_handle = NULL;
        }
    }

    sadc_register_adcomp_int_callback(NULL);
}

void sadc_compensation_offset_update(sadc_value_t adc_val) {
    uint32_t read_status;
    mp_cal_vcm_adc_t mp_cal_vcmadc;

    read_status = mpcalvcmadcread(&mp_cal_vcmadc);

    if ((read_status == STATUS_SUCCESS) && (mp_cal_vcmadc.enable)) {
        sadc_compensation_offset = adc_val - mp_cal_vcmadc.adc_1;
    } else {
        sadc_compensation_offset = 0;
    }

#ifdef CONFIG_SADC_COMP_DEBUG
    printf("\nOffset = %d, ADC = %d, VCMADC = %d\n", sadc_compensation_offset,
           adc_val, mp_cal_vcmadc.adc_1);
#endif
}

sadc_value_t sadc_compensation(sadc_value_t adc_val) {
    sadc_value_t comp_vol = 0;

    comp_vol = adc_val - sadc_compensation_offset;

    return comp_vol;
}

/**
 * \brief           SADC Interrupt Handler
 */
void sadc_handler(void) {
    sadc_cb_t cb;
    sadc_int_t reg_sadc_int_status;
    sadc_value_t sadc_value;
    sadc_cal_type_t cal_type;

    reg_sadc_int_status.reg = SADC->int_status.reg;
    SADC->int_clear.reg = reg_sadc_int_status.reg;

    if (reg_sadc_int_status.reg != 0) {
        if (reg_sadc_int_status.bit.done == 1) {}
        if (reg_sadc_int_status.bit.valid == 1) {
            if (sadc_convert_ch <= SADC_CH_VBAT) {
                if (sadc_reg_handler != NULL) {
                    cb.type = SADC_CB_SAMPLE;
                    sadc_value = sadc_get_adc_value();

                    sadc_resolution_compensation(&sadc_value);
                    cb.raw.conversion_value = sadc_value;

                    sadc_value = sadc_compensation(sadc_value);
                    cb.raw.compensation_value = sadc_value;

                    if (sadc_convert_ch <= SADC_CH_AIN7) {
                        cal_type = SADC_CALIBRATION_AIO;
                    } else if (sadc_convert_ch <= SADC_CH_VBAT) {
                        cal_type = SADC_CALIBRATION_VBAT;
                    }
                    sadc_value = sadc_calibration(cal_type, sadc_value);
                    cb.raw.calibration_value = sadc_value;

                    if (sadc_convert_ch == SADC_CH_VBAT) {
                        /* vbat value for Temperature sensor */
                        vbatadc = sadc_value;
                    }

                    cb.data.sample.value = sadc_value;
                    cb.data.sample.channel = sadc_convert_ch;

                    sadc_reg_handler(&cb);
                }
            } else if (sadc_convert_ch <= SADC_COMP_TEMPERATURE) {
                if (sadc_txcomp_handler != NULL) {
                    cb.type = SADC_CB_SAMPLE;
                    sadc_value = sadc_get_adc_value();

                    if (sadc_convert_ch == SADC_COMP_VBAT) {
                        sadc_resolution_compensation(&sadc_value);
                        cb.raw.conversion_value = sadc_value;

                        /* cvbat for temperature sensor */
                        cvbat = sadc_value;

                        sadc_value = sadc_compensation(sadc_value);
                        cb.raw.compensation_value = sadc_value;

                        sadc_value = sadc_calibration(SADC_CALIBRATION_VBAT,
                                                      sadc_value);
                        cb.raw.calibration_value = sadc_value;
                    } else if (sadc_convert_ch == SADC_COMP_TEMPERATURE) {
                        sadc_temperature_disable();

                        sadc_resolution_compensation(&sadc_value);
                        cb.raw.conversion_value = sadc_value;

                        sadc_value = sadc_compensation(sadc_value);

                        cb.raw.compensation_value = sadc_value;

                        sadc_value = sadc_temperature_calibration(sadc_value);

                        cb.raw.calibration_value = sadc_value;
                    }

                    cb.data.sample.value = sadc_value;
                    cb.data.sample.channel = sadc_convert_ch;

                    sadc_txcomp_handler(&cb);
                }
            } else if (sadc_convert_ch <= SADC_COMP_VCM) {
                if (sadc_adcomp_handler != NULL) {
                    cb.type = SADC_CB_SAMPLE;
                    sadc_value = sadc_get_adc_value();

                    if (sadc_convert_ch == SADC_COMP_VCM) {
                        /* fix ADC resolution 12-bit for VCM temperature compensation */
                        sadc_value >>= 2;
                        cb.raw.conversion_value = sadc_value;

                        cb.raw.compensation_value = sadc_value;

                        cb.raw.calibration_value = sadc_value;
                    }

                    cb.data.sample.value = sadc_value;
                    cb.data.sample.channel = sadc_convert_ch;

                    sadc_adcomp_handler(&cb);
                }
            } else if (sadc_convert_ch <= SADC_0VADC) {
                if (sadc_txcomp_handler != NULL) {
                    cb.type = SADC_CB_SAMPLE;
                    sadc_value = sadc_get_adc_value();

                    sadc_resolution_compensation(&sadc_value);
                    cb.raw.conversion_value = sadc_value;

                    sadc_value = sadc_compensation(sadc_value);
                    cb.raw.compensation_value = sadc_value;

                    /* 0V ADC Value for temperature senor calcuation */
                    c0vadc = sadc_value;

                    cb.data.sample.value = sadc_value;
                    cb.data.sample.channel = sadc_convert_ch;
                }
            }

            lpm_low_power_unmask(LOW_POWER_MASK_BIT_TASK_ADC);
            sadc_disable();

            sadc_ch_read_convert_state = SADC_CONVERT_DONE;
        }
        if (reg_sadc_int_status.bit.mode_done == 1) {
            if (sadc_get_sample_mode() == SADC_SAMPLE_START) {
                sadc_convert_state = SADC_CONVERT_DONE;
            }

            /*
            if(sadc_reg_handler != NULL)
            {
                cb.type = SADC_CB_DONE;
                cb.data.done.p_buffer = (sadc_value_t *)(SADC_GET_XDMA_START_ADDRESS());
                cb.data.done.size = SADC_GET_XDMA_RESULT_NUMBER();
                sadc_reg_handler(&cb);
            }
            */
        }
        if (reg_sadc_int_status.bit.wdma == 1) {}
        if (reg_sadc_int_status.bit.wdma_error == 1) {}
    }

    return;
}
