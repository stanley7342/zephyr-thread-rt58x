/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           sysctrl_reg.h
 * \brief          system control interrupt register defineds header file
 */
/*
 * This file is part of library_name.
 * Author:
 */

#ifndef SYSCTRL_REG_H
#define SYSCTRL_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
 * \brief           System control soc chip info register at offet 0x00
 */
typedef union sysctrl_soc_chip_info_s {
    struct sysctrl_soc_chip_info_b {
        uint32_t chip_rev : 8;                  /*!< chip revision */
        uint32_t reserved : 8;                  /*!< reserved bits */
        uint32_t chip_id  : 16;                 /*!< chip id */
    } bit;
    uint32_t reg;
} sysctrl_soc_chip_info_t;

/**
 * \brief           System control clock control 0 register at offet 0x04
 */
typedef union sysctrl_clk_ctl0_s {
    struct sysctrl_clk_ctl0_b {
        uint32_t hclk_sel       : 2;            /*!< hclk selection */
        uint32_t per_clk_sel    : 2;            /*!< peripheral clock selection */
        uint32_t reserved0      : 2;            /*!< reserved bits */
        uint32_t slow_clk_sel   : 2;            /*!< slow clock selection */
        uint32_t cfg_bbpll_freq : 3;            /*!< baseband pll frequency */
        uint32_t reserved1      : 4;            /*!< reserved bits */
        uint32_t cfg_bbpll_en   : 1;            /*!< baseband pll enable */
        uint32_t reserved2      : 16;           /*!< reserved bits */
    } bit;
    uint32_t reg;
} sysctrl_clk_ctl0_t;

/**
 * \brief           System control clock control 1 register at offet 0x08
 */
typedef union sysctrl_clk_ctl1_s {
    struct sysctrl_clk_ctl1_b {
        uint32_t uart0_clk_sel    : 2;          /*!< uart0 clock selection */
        uint32_t uart1_clk_sel    : 2;          /*!< uart1 clock selection */
        uint32_t uart2_clk_sel    : 2;          /*!< uart2 clock selection */
        uint32_t reserved0        : 2;          /*!< reserved bits */
        uint32_t ext_slow_clk_sel : 5;          /*!< select external slow clock source */
        uint32_t ext_slow_clk_en  : 1;          /*!< select external slow clock enable */
        uint32_t reserved1        : 2;          /*!< reserved bits */
        uint32_t pwm0_clk_sel     : 2;          /*!< pwm0 clock selection */
        uint32_t pwm1_clk_sel     : 2;          /*!< pwm1 clock selection */
        uint32_t pwm2_clk_sel     : 2;          /*!< pwm2 clock selection */
        uint32_t pwm3_clk_sel     : 2;          /*!< pwm3 clock selection */
        uint32_t pwm4_clk_sel     : 2;          /*!< pwm4 clock selection */
        uint32_t timer0_clk_sel   : 2;          /*!< timert0 clock selection */
        uint32_t timer1_clk_sel   : 2;          /*!< timert1 clock selection */
        uint32_t timer2_clk_sel   : 2;          /*!< timert2 clock selection */
    } bit;
    uint32_t reg;
} sysctrl_clk_ctl1_t;

/**
 * \brief           System control power state register at offet 0x0C
 */
typedef union sysctrl_power_state_s {
    struct sysctrl_power_state_b {
        uint32_t cfg_set_lowpower : 3;          /*!< set mcu power state */
        uint32_t reserved0        : 29;         /*!< reserved bits */
    } bit;
    uint32_t reg;
} sysctrl_power_state_t;

/**
 * \brief           System control soc gpio enable aio register at offet 0x44
 */
typedef union sysctrl_soc_gpio_en_aio_s {
    struct sysctrl_soc_gpio_en_aio_b {
        uint32_t gpio_en_aio       : 8;         /*!< aiox enable */
        uint32_t flash_sio_pull_en : 4;         /*!< flash siox pull-high enable */
        uint32_t flash_drv_sel     : 2;         /*!< flash pad driving selection */
        uint32_t reserved0         : 2;         /*!< reserved bits */
        uint32_t reserved1         : 16;        /*!< reserved bits */
    } bit;
    uint32_t reg;
} sysctrl_soc_gpio_en_aio_t;

//0x48
/**
 * \brief           System control cache control register at offet 0x48
 */
typedef union sysctrl_cache_ctl_s {
    struct sysctrl_cache_ctl_b {
        uint32_t cache_en        : 1;           /*!< cache enable */
        uint32_t cache_way_1_en  : 1;           /*!< cache way 1 enable */
        uint32_t reserved0       : 6;           /*!< reserved bits */
        uint32_t cache_way_0_clr : 1;           /*!< clear way 0 cache  */
        uint32_t cache_way_1_clr : 1;           /*!< clear way 1 cache  */
        uint32_t reserved1       : 22;          /*!< reserved bits */
    } bit;
    uint32_t reg;
} sysctrl_cache_ctl_t;

/**
 * \brief           System control soc pwm selection register at offet 0x4C
 */
typedef union sysctrl_soc_pwm_sel_s {
    struct sysctrl_soc_pwm_sel_b {
        uint32_t pwm0_src_sel : 2;              /*!< pwm0 output source selection */
        uint32_t pwm1_src_sel : 2;              /*!< pwm0 output source selection */
        uint32_t pwm2_src_sel : 2;              /*!< pwm0 output source selection */
        uint32_t pwm3_src_sel : 2;              /*!< pwm0 output source selection */
        uint32_t pwm4_src_sel : 2;              /*!< pwm0 output source selection */
        uint32_t reserved     : 22;             /*!< pwm0 output source selection */
    } bit;
    uint32_t reg;
} sysctrl_soc_pwm_sel_t;

/**
 * \brief           System control sram lowpower 0 register at offet 0x50
 */
typedef union sysctrl_sram_lowpower_0_s {
    struct sysctrl_sram_lowpower_0_b {
        uint32_t  cfg_sram_ds_sp_ram0       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_ram1       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_ram2       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_ram3       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_ram4       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_ram5       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_ram6       : 1;    /*!<  */
        uint32_t  reserved1                 : 6;    /*!<  */
        uint32_t  cfg_sram_ds_sp_rom        : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_cache_ram  : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sp_crypto_ram : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_ram0       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_ram1       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_ram2       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_ram3       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_ram4       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_ram5       : 1;    /*!<  */
        uint32_t  cfg_sram_ds_sd_ram6       : 1;    /*!<  */
        uint32_t  reserved2                 : 6;    /*!<  */
        uint32_t  cfg_sram_ds_ds_rom        : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_cache_ram  : 1;    /*!<  */
        uint32_t  cfg_sram_ds_ds_crypto_ram : 1;    /*!<  */
    } bit;
    uint32_t reg;
} sysctrl_sram_lowpower_0_t;

/**
 * \brief           System control sram lowpower 1 register at offet 0x54
 */
typedef union sysctrl_sram_lowpower_1_s {
    struct sysctrl_sram_lowpower_1_b {
        uint32_t  cfg_sram_sd_nm_ram0       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_ram1       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_ram2       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_ram3       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_ram4       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_ram5       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_ram6       : 1;    /*!<  */
        uint32_t  reserved1                 : 6;    /*!<  */
        uint32_t  cfg_sram_sd_nm_rom        : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_cache_ram  : 1;    /*!<  */
        uint32_t  cfg_sram_sd_nm_crypto_ram : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_ram0       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_ram1       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_ram2       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_ram3       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_ram4       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_ram5       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_ram6       : 1;    /*!<  */
        uint32_t  reserved2                 : 6;    /*!<  */
        uint32_t  cfg_sram_sd_sp_rom        : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_cache_ram  : 1;    /*!<  */
        uint32_t  cfg_sram_sd_sp_crypto_ram : 1;    /*!<  */
    } bit;
    uint32_t reg;
} sysctrl_sram_lowpower_1_t;

/**
 * \brief           System control sram lowpower 2 register at offet 0x58
 */
typedef union sysctrl_sram_lowpower_2_s {
    struct sysctrl_sram_lowpower_2_b {
        uint32_t  cfg_sram_sd_ds_ram0       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_ram1       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_ram2       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_ram3       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_ram4       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_ram5       : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_ram6       : 1;    /*!<  */
        uint32_t  reserved1                 : 6;    /*!<  */
        uint32_t  cfg_sram_sd_ds_rom        : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_cache_ram  : 1;    /*!<  */
        uint32_t  cfg_sram_sd_ds_crypto_ram : 1;    /*!<  */
        uint32_t  cfg_sram_pd_nm_ram0_ram1  : 1;    /*!<  */
        uint32_t  cfg_sram_pd_nm_ram2_ram3  : 1;    /*!<  */
        uint32_t  cfg_sram_pd_nm_ram4       : 1;    /*!<  */
        uint32_t  cfg_sram_pd_nm_ram5       : 1;    /*!<  */
        uint32_t  cfg_sram_pd_nm_ram6       : 1;    /*!<  */
        uint32_t  reserved2                 : 3;    /*!<  */
        uint32_t  cfg_sram_pd_sp_ram0_ram1  : 1;    /*!<  */
        uint32_t  cfg_sram_pd_sp_ram2_ram3  : 1;    /*!<  */
        uint32_t  cfg_sram_pd_sp_ram4       : 1;    /*!<  */
        uint32_t  cfg_sram_pd_sp_ram5       : 1;    /*!<  */
        uint32_t  cfg_sram_pd_sp_ram6       : 1;    /*!<  */
        uint32_t  reserved3                 : 3;    /*!<  */
    } bit;
    uint32_t reg;
} sysctrl_sram_lowpower_2_t;

/**
 * \brief           System control sram lowpower 3 register at offet 0x5C
 */
typedef union sysctrl_sram_lowpower_3_s {
    struct sysctrl_sram_lowpower_3_b {
        uint32_t  cfg_sram_pd_ds_ram0_ram1 : 1; /*!<  */
        uint32_t  cfg_sram_pd_ds_ram2_ram3 : 1; /*!<  */
        uint32_t  cfg_sram_pd_ds_ram4      : 1; /*!<  */
        uint32_t  cfg_sram_pd_ds_ram5      : 1; /*!<  */
        uint32_t  cfg_sram_pd_ds_ram6      : 1; /*!<  */
        uint32_t  reserved0                : 3; /*!<  */
        uint32_t cfg_sram_rm               : 4; /*!<  */
        uint32_t cfg_sram_test1            : 1; /*!<  */
        uint32_t cfg_sram_tme              : 1; /*!<  */
        uint32_t reserved1                 : 2; /*!<  */
        uint32_t reserved2                 : 3; /*!<  */
        uint32_t cfg_peri1_off_ds          : 1; /*!<  */
        uint32_t cfg_peri2_off_sp          : 1; /*!<  */
        uint32_t cfg_peri2_off_ds          : 1; /*!<  */
        uint32_t cfg_peri3_off_sp          : 1; /*!<  */
        uint32_t cfg_peri3_off_ds          : 1; /*!<  */
        uint32_t cfg_cache_auto_flush_sp   : 1; /*!<  */
        uint32_t cfg_ds_rco32k_off         : 1; /*!<  */
        uint32_t reserved3                 : 6; /*!<  */
    } bit;
    uint32_t reg;
} sysctrl_sram_lowpower_3_t;

/**
 * \brief           System control clock control 2 register at offet 0x60
 */
typedef union sysctrl_clk_ctl2_s {
    struct sysctrl_clk_ctl2_b {
        uint32_t en_ck_uart0        : 1;        /*!<  */
        uint32_t en_ck_uart1        : 1;        /*!<  */
        uint32_t en_ck_uart2        : 1;        /*!<  */
        uint32_t reserved0          : 1;        /*!<  */
        uint32_t en_ck_qspi0        : 1;        /*!<  */
        uint32_t en_ck_qspi1        : 1;        /*!<  */
        uint32_t reserved1          : 2;        /*!<  */
        uint32_t en_ck_i2cm0        : 1;        /*!<  */
        uint32_t en_ck_i2cm1        : 1;        /*!<  */
        uint32_t en_ck_i2s0         : 1;        /*!<  */
        uint32_t reserved2          : 1;        /*!<  */
        uint32_t en_ck_crypto       : 1;        /*!<  */
        uint32_t en_ck_xdma         : 1;        /*!<  */
        uint32_t en_ck_irm          : 1;        /*!<  */
        uint32_t reserved3          : 1;        /*!<  */
        uint32_t en_ck_timer0       : 1;        /*!<  */
        uint32_t en_ck_timer1       : 1;        /*!<  */
        uint32_t en_ck_timer2       : 1;        /*!<  */
        uint32_t reserved4          : 1;        /*!<  */
        uint32_t en_ck32_timer3     : 1;        /*!<  */
        uint32_t en_ck32_timer4     : 1;        /*!<  */
        uint32_t en_ck32_rtc        : 1;        /*!<  */
        uint32_t en_ck32_gpio       : 1;        /*!<  */
        uint32_t en_rtc_pclk_ds     : 1;        /*!<  */
        uint32_t en_gpio_pclk_ds    : 1;        /*!<  */
        uint32_t en_auxcomp_pclk_ds : 1;        /*!<  */
        uint32_t en_bodcomp_pclk_ds : 1;        /*!<  */
        uint32_t en_ck32_auxcomp    : 1;        /*!<  */
        uint32_t en_ck32_bodcomp    : 1;        /*!<  */
        uint32_t en_ck_div_32k      : 1;        /*!<  */
        uint32_t en_rco32k_div2     : 1;        /*!<  */
    } bit;
    uint32_t reg;
} sysctrl_clk_ctl2_t;

/**
 * \brief           System control test register at offet 0x64
 */
typedef union sysctrl_test_s {
    struct sysctrl_test_b {
        uint32_t cfg_wlan_active_sel : 5;       /*!<  */
        uint32_t reserved0           : 2;       /*!<  */
        uint32_t cfg_wlan_active_en  : 1;       /*!<  */
        uint32_t dbg_clk_sel         : 3;       /*!<  */
        uint32_t reserved1           : 1;       /*!<  */
        uint32_t cfg_dpc_spi_div     : 3;       /*!<  */
        uint32_t reserved2           : 1;       /*!<  */
        uint32_t dbg_out_sel         : 5;       /*!<  */
        uint32_t reserved3           : 3;       /*!<  */
        uint32_t dbg_clkout_en       : 1;       /*!<  */
        uint32_t cfg_ice_wakeup_pmu  : 1;       /*!<  */
        uint32_t reserved4           : 2;       /*!<  */
        uint32_t otp_test_en         : 1;       /*!<  */
        uint32_t otp_test_sel        : 3;       /*!<  */
    } bit;
    uint32_t reg;
} sysctrl_test_t;

/**
 * \brief          System control total register 
 */
typedef struct {
    __I  sysctrl_soc_chip_info_t   soc_chip_info;       /*!< 0x00 */
    __IO sysctrl_clk_ctl0_t        sys_clk_ctrl;        /*!< 0x04 */
    __IO sysctrl_clk_ctl1_t        sys_clk_ctrl1;       /*!< 0x08 */
    __IO sysctrl_power_state_t     sys_power_state;     /*!< 0x0C */
    __IO uint32_t                  reserved_1;          /*!< 0x10 */
    __IO uint32_t                  reserved_2;          /*!< 0x14 */
    __IO uint32_t                  reserved_3;          /*!< 0x18 */
    __IO uint32_t                  reserved_4;          /*!< 0x1C */
    __IO uint32_t                  gpio_pull_ctrl[4];   /*!< 0x20 ~ 0x2C */
    __IO uint32_t                  gpio_drv_ctrl[2];    /*!< 0x30 ~ 0x34 */
    __IO uint32_t                  gpio_od_ctrl;        /*!< 0x38 */
    __IO uint32_t                  gpio_en_schmitt;     /*!< 0x3C */
    __IO uint32_t                  gpio_en_filter;      /*!< 0x40 */
    __IO sysctrl_soc_gpio_en_aio_t gpio_aio_ctrl;       /*!< 0x44 */
    __IO sysctrl_cache_ctl_t       cache_ctrl;          /*!< 0x48 */
    __IO sysctrl_soc_pwm_sel_t     soc_pwm_sel;         /*!< 0x4C */
    __IO sysctrl_sram_lowpower_0_t sram_lowpower_0;     /*!< 0x50 */
    __IO sysctrl_sram_lowpower_1_t sram_lowpower_1;     /*!< 0x54 */
    __IO sysctrl_sram_lowpower_2_t sram_lowpower_2;     /*!< 0x58 */
    __IO sysctrl_sram_lowpower_3_t sram_lowpower_3;     /*!< 0x5C */
    __IO sysctrl_clk_ctl2_t        sys_clk_ctrl2;       /*!< 0x60 */
    __IO sysctrl_test_t            sys_test;            /*!< 0x64 */
    __IO uint32_t                  reserved_5;          /*!< 0x68 */
    __IO uint32_t                  reserved_6;          /*!< 0x6C */
    __IO uint32_t                  reserved_7;          /*!< 0x70 */
    __IO uint32_t                  reserved_8;          /*!< 0x74 */
    __IO uint32_t                  reserved_9;          /*!< 0x78 */
    __IO uint32_t                  reserved_10;         /*!< 0x7C */
    __IO uint32_t                  sys_gpio_omux[8];    /*!< 0x80~0x9C */
    //__IO uint32_t                  sys_gpio_omux0;      /*!<0x80 */
    //__IO uint32_t                  sys_gpio_omux1;      /*!<0x84 */
    //__IO uint32_t                  sys_gpio_omux2;      /*!<0x88 */
    //__IO uint32_t                  sys_gpio_omux3;      /*!<0x8C */
    //__IO uint32_t                  sys_gpio_omux4;      /*!<0x90 */
    //__IO uint32_t                  sys_gpio_omux5;      /*!<0x94 */
    //__IO uint32_t                  sys_gpio_omux6;      /*!<0x98 */
    //__IO uint32_t                  sys_gpio_omux7;      /*!<0x9C */
    __IO uint32_t                  sys_gpio_imux[8];    /*!<0xA0 */
    //__IO uint32_t                  sys_gpio_imux0;      /*!<0xA0 */
    //__IO uint32_t                  sys_gpio_imux1;      /*!<0xA4 */
    //__IO uint32_t                  sys_gpio_imux2;      /*!<0xA8 */
    //__IO uint32_t                  sys_gpio_imux3;      /*!<0xAC */
    //__IO uint32_t                  sys_gpio_imux4;      /*!<0xB0 */
    //__IO uint32_t                  sys_gpio_imux5;      /*!<0xB4 */
    //__IO uint32_t                  sys_gpio_imux6;      /*!<0xB8 */
    //__IO uint32_t                  sys_gpio_imux7;      /*!<0xBC */
} sysctrl_t;


#define MCU_HCLK_SEL_SHFT                   0
#define MCU_HCLK_SEL_MASK                   (3<<MCU_HCLK_SEL_SHFT)
#define MCU_HCLK_SEL_32M                    (0<<MCU_HCLK_SEL_SHFT)
#define MCU_HCLK_SEL_PLL                    (1<<MCU_HCLK_SEL_SHFT)
#define MCU_HCLK_SEL_16M                    (2<<MCU_HCLK_SEL_SHFT)
#define MCU_HCLK_SEL_RCO1M                  (3<<MCU_HCLK_SEL_SHFT)

#define MCU_PERCLK_SEL_SHFT                 2
#define MCU_PERCLK_SEL_MASK                 (3<<MCU_PERCLK_SEL_SHFT)
#define MCU_PERCLK_SEL_32M                  (0<<MCU_PERCLK_SEL_SHFT)
#define MCU_PERCLK_SEL_16M                  (2<<MCU_PERCLK_SEL_SHFT)
#define MCU_PERCLK_SEL_RCO1M                (3<<MCU_PERCLK_SEL_SHFT)

#define MCU_BBPLL_ENABLE_SHIFT              15
#define MCU_BBPLL_MASK                      (1<<MCU_BBPLL_ENABLE_SHIFT)
#define MCU_BBPLL_ENABLE                    (1<<MCU_BBPLL_ENABLE_SHIFT)
#define MCU_BBPLL_DISABLE                   (0<<MCU_BBPLL_ENABLE_SHIFT)

#define MCU_BBPLL_CLK_SHIFT                 8
#define MCU_BBPLL_CLK_MASK                  (7<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_48M                       (0<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_64M                       (1<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_RVD1                      (2<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_RVD2                      (3<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_24M                       (4<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_32M                       (5<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_36M                       (6<<MCU_BBPLL_CLK_SHIFT)
#define MCU_BBPLL_40M                       (7<<MCU_BBPLL_CLK_SHIFT)

#define MCU_PERI1_PWR_OFF_DEEP_SLEEP        (1<<19)
#define MCU_PERI2_PWR_OFF_SLEEP             (1<<20)
#define MCU_PERI2_PWR_OFF_DEEP_SLEEP        (1<<21)
#define MCU_PERI3_PWR_OFF_SLEEP             (1<<22)
#define MCU_PERI3_PWR_OFF_DEEP_SLEEP        (1<<23)

#ifdef __cplusplus
}
#endif

#endif /* End of SYSCTRL_REG_H */

