/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           sysctrl.h
 * \brief          system control defineds header file
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#ifndef SYSCTRL_H
#define SYSCTRL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "mcu.h"


/**
 * \defgroup        SYSCTRL Sysctrl
 * \ingroup         RT584_DRIVER
 * \brief           Define Sysctrl definitions, structures, and functions
 * @{
 */

#define MAP_BASE         ((uint32_t)SYSCTRL + 0x80)

#define PULLOPT_BASE     ((uint32_t)SYSCTRL + 0x20)
#define DRV_BASE         ((uint32_t)SYSCTRL + 0x30)
#define OD_BASE          ((uint32_t)SYSCTRL + 0x38)


#define SYS_CLK_OFFSET              1
#define SYS_CLK_32MHZ               0
#define SYS_CLK_48MHZ               1
#define SYS_CLK_64MHZ               2
#define SYS_CLK_16MHZ               3
#define SYS_CLK_RCO1MHZ             4
#define SYS_CLK_24MHZ               5
#define SYS_CLK_36MHZ               6
#define SYS_CLK_40MHZ               7



#define PLL_LOCK_DOWN                   1
#define PLL_LOCK_VIBIT_1                1
#define PLL_LOCK_VIBIT_0                0
#define PLL_LOCK_VIBIT_3                3
#define PLL_LOCK_BANK_VCO_4             4
#define PLL_LOCK_BANK_VCO_7             7
#define PLL_CHECK_COUNT                 5


/**
 * \brief           Because compiler code optimize, 
 *                  we should set PLL_WAIT_PERIOD as 4N
 */
#define PLL_WAIT_PERIOD              1600
#define PLL_DELAY_PERIOD             400
/**
 * \brief           Slow clock source select
 *                  
 */
#define EXT_GPIO_RCO32K              3
/**
 * \brief           RT584 output pin mux register value definitions
 */
#define  MODE_GPIO_REG_VALUE              0x00
#define  MODE_UART0_TX_REG_VALUE          0x01
#define  MODE_UART1_TX_REG_VALUE          0x02
#define  MODE_UART1_RTSN_REG_VALUE        0x03
#define  MODE_UART2_TX_REG_VALUE          0x04
#define  MODE_UART2_RTSN_REG_VALUE        0x05
#define  MODE_PWM0_REG_VALUE              0x06
#define  MODE_PWM1_REG_VALUE              0x07
#define  MODE_PWM2_REG_VALUE              0x08
#define  MODE_PWM3_REG_VALUE              0x09
#define  MODE_PWM4_REG_VALUE              0x0A
#define  MODE_IRM_REG_VALUE               0x0B
#define  MODE_I2CM0_SCL_OUT_REG_VALUE     0x0C
#define  MODE_I2CM0_SDA_OUT_REG_VALUE     0x0D
#define  MODE_I2CM1_SCL_OUT_REG_VALUE     0x0E
#define  MODE_I2CM1_SDA_OUT_REG_VALUE     0x0F
#define  MODE_I2CS_SCL_OUT_REG_VALUE      0x10
#define  MODE_I2CS_SDA_OUT_REG_VALUE      0x11
#define  MODE_SPI0_SCLK_OUT_REG_VALUE     0x12
#define  MODE_SPI0_SDATA_OUT0_REG_VALUE   0x13
#define  MODE_SPI0_SDATA_OUT1_REG_VALUE   0x14
#define  MODE_SPI0_SDATA_OUT2_REG_VALUE   0x15
#define  MODE_SPI0_SDATA_OUT3_REG_VALUE   0x16
#define  MODE_SPI0_CSN0_REG_VALUE         0x17
#define  MODE_SPI0_CSN1_REG_VALUE         0x18
#define  MODE_SPI0_CSN2_REG_VALUE         0x19
#define  MODE_SPI0_CSN3_REG_VALUE         0x1A
#define  MODE_SPI1_SCLK_OUT_REG_VALUE     0x1B
#define  MODE_SPI1_SDATA_OUT0_REG_VALUE   0x1C
#define  MODE_SPI1_SDATA_OUT1_REG_VALUE   0x1D
#define  MODE_SPI1_SDATA_OUT2_REG_VALUE   0x1E
#define  MODE_SPI1_SDATA_OUT3_REG_VALUE   0x1F
#define  MODE_SPI1_CSN0_REG_VALUE         0x20
#define  MODE_SPI1_CSN1_REG_VALUE         0x21
#define  MODE_SPI1_CSN2_REG_VALUE         0x22
#define  MODE_SPI1_CSN3_REG_VALUE         0x23
#define  MODE_I2S_BCK_REG_VALUE           0x24
#define  MODE_I2S_WCK_REG_VALUE           0x25
#define  MODE_I2S_SDO_REG_VALUE           0x26
#define  MODE_I2S_MCLK_REG_VALUE          0x27
#define  MODE_SWD_REG_VALUE               0x2F
#define  MODE_DBG0_REG_VALUE              0x30
#define  MODE_DBG1_REG_VALUE              0x31
#define  MODE_DBG2_REG_VALUE              0x32
#define  MODE_DBG3_REG_VALUE              0x33
#define  MODE_DBG4_REG_VALUE              0x34
#define  MODE_DBG5_REG_VALUE              0x35
#define  MODE_DBG6_REG_VALUE              0x36
#define  MODE_DBG7_REG_VALUE              0x37
#define  MODE_DBG8_REG_VALUE              0x38
#define  MODE_DBG9_REG_VALUE              0x39
#define  MODE_DBGA_REG_VALUE              0x3A
#define  MODE_DBGB_REG_VALUE              0x3B
#define  MODE_DBGC_REG_VALUE              0x3C
#define  MODE_DBGD_REG_VALUE              0x3D
#define  MODE_DBGE_REG_VALUE              0x3E
#define  MODE_DBGF_REG_VALUE              0x3F


/**
 * \brief           RT584 input pin mux register value definitions
 */
//A0
#define  MODE_UART1_RX_REG_VALUE           0x00000000
#define  MODE_UART1_CTSN_REG_VALUE         0x00000008
#define  MODE_UART2_RX_REG_VALUE           0x00000010
#define  MODE_UART2_CTSN_REG_VALUE         0x00000018
//A4
#define  MODE_UART0_RX_REG_VALUE           0x10000000
#define  MODE_I2S_SDI_REG_VALUE            0x10000008
#define  MODE_I2CS_SCL_REG_VALUE           0x10000010
#define  MODE_I2CS_SDA_REG_VALUE           0x10000018
//A8
#define  MODE_I2CM0_SCL_REG_VALUE          0x20000000
#define  MODE_I2CM0_SDA_REG_VALUE          0x20000008
#define  MODE_I2CM1_SCL_REG_VALUE          0x20000010
#define  MODE_I2CM1_SDA_REG_VALUE          0x20000018
//AC
#define  MODE_QSPI0_CSN_REG_VALUE          0x40000000
#define  MODE_QSPI0_SCLK_REG_VALUE         0x40000008
#define  MODE_QSPI0_SDATA0_REG_VALUE       0x40000010
#define  MODE_QSPI0_SDATA1_REG_VALUE       0x40000018
//B0
#define  MODE_QSPI0_SDATA2_REG_VALUE       0x50000000
#define  MODE_QSPI0_SDATA3_REG_VALUE       0x50000008
//B4
#define  MODE_QSPI1_CSN_REG_VALUE          0x60000000
#define  MODE_QSPI1_SCLK_REG_VALUE         0x60000008
#define  MODE_QSPI1_SDATA0_REG_VALUE       0x60000010
#define  MODE_QSPI1_SDATA1_REG_VALUE       0x60000018
//B8
#define  MODE_QSPI1_SDATA2_REG_VALUE       0x70000000
#define  MODE_QSPI1_SDATA3_REG_VALUE       0x70000008

#define  MODE_PIN_MUX_NULL                 0xFFFFFFFF


/**
 * \brief           RT584 pin mux definitions
 */
#define  MODE_GPIO              0x00
#define  MODE_UART0_TX          0x01
#define  MODE_UART0_RX          0x02
#define  MODE_UART1_TX          0x03
#define  MODE_UART1_RX          0x04
#define  MODE_UART1_RTSN        0x05
#define  MODE_UART1_CTSN        0x06
#define  MODE_UART2_TX          0x07
#define  MODE_UART2_RX          0x08
#define  MODE_UART2_RTSN        0x09
#define  MODE_UART2_CTSN        0x0A
#define  MODE_PWM0              0x0B
#define  MODE_PWM1              0x0C
#define  MODE_PWM2              0x0D
#define  MODE_PWM3              0x0E
#define  MODE_PWM4              0x0F
#define  MODE_IRM               0x10
#define  MODE_I2CM0_SCL         0x11
#define  MODE_I2CM0_SDA         0x12
#define  MODE_I2CM1_SCL         0x13
#define  MODE_I2CM1_SDA         0x14
#define  MODE_I2CS_SCL          0x15
#define  MODE_I2CS_SDA          0x16

#define  MODE_SPI0_MASTER_SCLK   0x17
#define  MODE_SPI0_MASTER_CSN0   0x18
#define  MODE_SPI0_MASTER_MOSI   0x19
#define  MODE_SPI0_MASTER_MISO   0x1A

#define  MODE_SPI0_SLAVE_SCLK    0x1B
#define  MODE_SPI0_SLAVE_CSN0    0x1C
#define  MODE_SPI0_SLAVE_MOSI    0x1D
#define  MODE_SPI0_SLAVE_MISO    0x1E

#define  MODE_SPI1_MASTER_SCLK   0x1F
#define  MODE_SPI1_MASTER_CSN0   0x20
#define  MODE_SPI1_MASTER_MOSI   0x21
#define  MODE_SPI1_MASTER_MISO   0x22
#define  MODE_SPI1_SLAVE_SCLK    0x23
#define  MODE_SPI1_SLAVE_CSN0    0x24
#define  MODE_SPI1_SLAVE_MOSI    0x25
#define  MODE_SPI1_SLAVE_MISO    0x26
#define  MODE_SPI0_MASTER_SDATA0 0x27
#define  MODE_SPI0_MASTER_SDATA1 0x28
#define  MODE_SPI0_MASTER_SDATA2 0x29
#define  MODE_SPI0_MASTER_SDATA3 0x2A
#define  MODE_SPI1_MASTER_SDATA0 0x2B
#define  MODE_SPI1_MASTER_SDATA1 0x2C
#define  MODE_SPI1_MASTER_SDATA2 0x2D
#define  MODE_SPI1_MASTER_SDATA3 0x2E
#define  MODE_SPI0_MASTER_CSN1   0x2F
#define  MODE_SPI0_MASTER_CSN2   0x30
#define  MODE_SPI0_MASTER_CSN3   0x31
#define  MODE_SPI0_SLAVE_CSN1    0x32
#define  MODE_SPI0_SLAVE_CSN2    0x33
#define  MODE_SPI0_SLAVE_CSN3    0x34
#define  MODE_SPI1_MASTER_CSN1   0x35
#define  MODE_SPI1_MASTER_CSN2   0x36
#define  MODE_SPI1_MASTER_CSN3   0x37
#define  MODE_SPI1_SLAVE_CSN1    0x38
#define  MODE_SPI1_SLAVE_CSN2    0x39
#define  MODE_SPI1_SLAVE_CSN3    0x3A
#define  MODE_I2S_BCK            0x3B
#define  MODE_I2S_WCK            0x3C
#define  MODE_I2S_SDO            0x3D
#define  MODE_I2S_SDI            0x3E
#define  MODE_I2S_MCLK           0x3F
#define  MODE_SWD                0x40
#define  MODE_DBG0               0x41
#define  MODE_DBG1               0x42
#define  MODE_DBG2               0x43
#define  MODE_DBG3               0x44
#define  MODE_DBG4               0x45
#define  MODE_DBG5               0x46
#define  MODE_DBG6               0x47
#define  MODE_DBG7               0x48
#define  MODE_DBG8               0x49
#define  MODE_DBG9               0x4A
#define  MODE_DBGA               0x4B
#define  MODE_DBGB               0x4C
#define  MODE_DBGC               0x4D
#define  MODE_DBGD               0x4E
#define  MODE_DBGE               0x4F
#define  MODE_DBGF               0x50
#define  MODE_MAX                0x51


/**
 * \brief  Driving through setting mode
 */
#define PULL_NONE         0                     /*!< set pin for no pull */
#define PULLDOWN_10K      1                     /*!< set pin for 10K pull down */
#define PULLDOWN_100K     2                     /*!< set pin for 100K pull down */
#define PULLDOWN_1M       3                     /*!< set pin for 1M pull down */
#define PULLUP_10K        5                     /*!< set pin for 10K pull up */
#define PULLUP_100K       6                     /*!< set pin for 100K pull up, this is default pin mode */
#define PULLUP_1M         7                     /*!< set pin for 1M pull up */

/**
 * \brief           Define pin driver option
 */
#define DRV_6MA             0                   /*!< set pin for 6mA driver   */
#define DRV_15MA            1                   /*!< set pin for 15mA driver  */
#define DRV_20MA            2                   /*!< set pin for 20mA driver  */
#define DRV_30MA            3                   /*!< set pin for 30mA driver  */

/**
 * \brief           Define IC chip id  and chip revision information
 */
#define IC_CHIP_ID_MASK_SHIFT               16
#define IC_CHIP_ID_MASK                     (0xFFFF<<IC_CHIP_ID_MASK_SHIFT)
#define IC_RT58X                            (0x0584<<IC_CHIP_ID_MASK_SHIFT)        /*!< RT584 IC Chip ID  */
#define IC_CHIP_REVISION_MASK_SHIFT         0
#define IC_CHIP_REVISION_MASK               (0xF<<IC_CHIP_REVISION_MASK_SHIFT)
#define IC_CHIP_REVISION_MPA                (1<<IC_CHIP_REVISION_MASK_SHIFT)       /*!< RT584 IC Chip Revision ID For MPA  */
#define IC_CHIP_REVISION_MPB                (2<<IC_CHIP_REVISION_MASK_SHIFT)       /*!< RT584 IC Chip Revision ID For MPB  */


#define DPD_RETENTION_BASE          0x10

/**
 * \brief           Selecting the system clock source.
 */
typedef enum
{
    SYS_32MHZ_CLK = SYS_CLK_32MHZ,
    SYS_48MHZ_CLK = SYS_CLK_48MHZ,
    SYS_64MHZ_CLK = SYS_CLK_64MHZ,
    SYS_16MHZ_CLK  = SYS_CLK_16MHZ,
    SYS_RCO1MHZ_CLK  = SYS_CLK_RCO1MHZ,
    SYS_24MHZ_CLK = SYS_CLK_24MHZ,
    SYS_36MHZ_CLK = SYS_CLK_36MHZ,
    SYS_40MHZ_CLK = SYS_CLK_40MHZ,
    SYS_CLK_MAX,
} sys_clk_sel_t;


/**
 * \brief           Selecting the hclk source.
 */
typedef enum {
    HCLK_SEL_32M,                               /*!<  */
    HCLK_SEL_PLL,                               /*!<  */
    HCLK_SEL_16M,                               /*!<  */
    HCLK_SEL_RCO1M,                             /*!<  */
} hclk_clk_sel_t;

/**
 * \brief           Selecting the peripheral source.
 */
typedef enum {
    PERCLK_SEL_32M,                             /*!<  */
    PERCLK_SEL_16M = 2,                         /*!<  */
    PERCLK_SEL_RCO1M,                           /*!<  */
} perclk_clk_sel_t;

/**
 * \brief           Enable peripheral interface clock.
 */
typedef enum {
    UART0_CLK,                                  /*!<  */
    UART1_CLK,                                  /*!<  */
    UART2_CLK,                                  /*!<  */
    QSPI0_CLK = 4,                              /*!<  */
    QSPI1_CLK,                                  /*!<  */
    I2CM0_CLK = 8,                              /*!<  */
    I2CM1_CLK,                                  /*!<  */
    I2CS0_CLK,                                  /*!<  */
    CRYPTO_CLK = 12,                            /*!<  */
    XDMA_CLK,                                   /*!<  */
    IRM_CLK,                                    /*!<  */
    TIMER0_CLK = 16,                            /*!<  */
    TIMER1_CLK,                                 /*!<  */
    TIMER2_CLK,                                 /*!<  */
    TIMER3_32K_CLK = 20,                        /*!<  */
    TIMER4_32K_CLK,                             /*!<  */
    RTC_32K_CLK,                                /*!<  */
    GPIO_32K_CLK,                               /*!<  */
    RTC_PCLK,                                   /*!<  */
    GPIO_PCLK,                                  /*!<  */
    AUX_PCLK,                                   /*!<  */
    BOD_PCLK,                                   /*!<  */
    AUX_32K_CLK,                                /*!<  */
    BOD_32K_CLK,                                /*!<  */
    CLK_32K_DIV,                                /*!<  */
    RCO32K_DIV2                                 /*!<  */
} per_interface_clk_en_t;
/**
 * \brief           slow clock enable enum typedef
 * \param[in]       us: 
 */
typedef enum
{
    RCO32K_SELECT = 1,               /*!< System slow clock 32k Mode */
    RCO16K_SELECT,                   /*!< System slow clock 16k Mode */
    RCO1M_SELECT,                    /*!< System slow clock 1M Mode */
    RCO_NULL,
} slow_clock_select_t;
/**
 * \brief           Delay us
 * \param[in]       us: 
 */
void delay_us(volatile uint32_t us);

/**
 * \brief           Delay ms
 * \param[in]       ms ; 
 */
void delay_ms(volatile uint32_t ms);


/**
 * \brief           Get pin function mode
 * \param[in]       pin_number: Specifies the pin number
 * \return          get the pin function mode
 */
uint32_t pin_get_mode(uint32_t pin_number);

/**
 * \brief           Set pin function mode
 * \param[in]       pin_number: Specifies the pin number
 * \param[in]       mode: The specail function mode for the pin_number
 */
void pin_set_mode(uint32_t pin_number, uint32_t mode);

/**
 * \brief           set pin function mode extend for RT584
 * \param[in]       pin_number: Specifies the pin number.
 *                  GPIO0~GPIO31
 * \param[in]       mode: The specail function mode for the pin_number
 *                  Config GPIO To --> UART/I2S/PWM/SADC/I2C/SPI...
 * \details
 *                  each pin has different function pin setting, please read RT584 datasheet to know each pin
 *                  function usage setting.
 */
void pin_set_out_mode_ex(uint32_t pin_number, uint32_t mode);

/**
 * \brief           Set pin function mode extend for RT584
 * \param[in]       pin_number: Specifies the pin number.
 *                  GPIO0~GPIO31
 * \param[in]       mode: The specail function mode for the pin_number
 *                  Config GPIO To --> UART/I2S/PWM/SADC/I2C/SPI...
 * \details
 *                  each pin has different function pin setting, please read RT584 datasheet to know each pin
 *                  function usage setting.
 */
void pin_set_in_mode_ex(uint32_t pin_number, uint32_t mode);

/**
 * \brief           Enable peripherial interface clock
 * \param[in]       clock_id: enable the specifies peripheral "clock_id" interface clock.
 *                  UART0_CLK
 *                  UART1_CLK
 *                  UART2_CLK
 *                  I2CM_CLK
 *                  QSPI0_CLK
 *                  QSPI1_CLK
 *                  TIMER1_CLK
 *                  TIMER2_CLK
 *                  I2S_CLK
 */
void enable_perclk(uint32_t clock);

/**
 * \brief           Disable peripherial interface clock
 * \param[in]       clock_id: disable the specifies peripheral "clock_id" interface clock.
 *                  UART0_CLK
 *                  UART1_CLK
 *                  UART2_CLK
 *                  I2CM_CLK
 *                  QSPI0_CLK
 *                  QSPI1_CLK
 *                  TIMER1_CLK
 *                  TIMER2_CLK
 *                  I2S_CLK
 */
void disable_perclk(uint32_t clock);

/**
 * \brief           Set pin pull option.
 * \param[in]       clock: Specifies the pin number.
 *                  PULL_NONE        0
 *                  PULL_DOWN_10K    1
 *                  PULL_DOWN_100K   2
 *                  PULL_DOWN_1M     3
 *                  PULL_UP_10K      5
 *                  PULL_UP_100K     6
 *                  PULL_UP_1M       7
 * \details
 *                  Pin default pull option is 100K pull up, User can use this function to change the pull up/down setting.
 *                  If user set the pin  set to gpio output mode, then the pin will auto to be set as no pull option.
 */
void pin_set_pullopt(uint32_t pin_number, uint32_t mode);

/**
 * \brief           Set pin driving option
 * \param[in]       pin_number: Specifies the pin number.
 * \param[in]       mode: pin driving option
 *                  DRV_4MA      0
 *                  DRV_10MA     1
 *                  DRV_14MA     2
 *                  DRV_20MA     3
 * \details
 *                  Pin default driving option is 20mA, User can use this function to change the pin driving setting.
 */
void pin_set_drvopt(uint32_t pin_number, uint32_t mode);

/**
 * \brief           Set pin to opendrain option
 * \param[in]       pin_number: Specifies the pin number
 */
void enable_pin_opendrain(uint32_t pin_number);

/**
 * \brief           Disable pin to opendrain option
 * \param[in]       pin_number: Specifies the pin number
 */
void disable_pin_opendrain(uint32_t pin_number);

/**
 * \brief check pll unlock
 * \param[in] NONE
 * \return    system clock mode 32MHZ/48MHz/64MHz
 * \retval    SYS_32MHZ_CLK   for CPU AHB 32MHz clock.
 * \retval    SYS_48MHZ_CLK   for CPU AHB 48MHz clock.
 * \retval    SYS_64MHZ_CLK   for CPU AHB 64MHz clock.
 * \details
 */
sys_clk_sel_t pll_unlock_check(void);

/**
 * \brief           Change CPU AHB CLOCK,
 * \return          STATUS_SUCCESS: for change success, 
 *                  STATUS_INVALID_PARAM: for change fail
 */
uint32_t change_ahb_system_clk(sys_clk_sel_t sys_clk_mode);

/**
 * \brief           Get CPU AHB CLOCK,
 * \return          SYS_CLK_32MHZ: for CPU AHB 32MHz clock.
 *                  SYS_CLK_48MHZ: for CPU AHB 48MHz clock.
 *                  SYS_CLK_64MHZ: for CPU AHB 64MHz clock.
 */
uint32_t get_ahb_system_clk(void);

/**
 * \brief           Change peripherial interface clock
 * \param[in]       perclk_clk_sel_t: enable the specifies peripheral "clock_id" interface clock.
 *                  PERCLK_SEL_32M
 *                  PERCLK_SEL_16M
 *                  PERCLK_SEL_RCO1M
 */
uint32_t change_peri_clk(perclk_clk_sel_t sys_clk_mode);

/**
 * \brief           Get peripherial interface clock
 * \return          perclk_clk_sel_t    enable the specifies peripheral "clock_id" interface clock.
 *                  PERCLK_SEL_32M
 *                  PERCLK_SEL_16M
 *                  PERCLK_SEL_RCO1M
 */
uint32_t get_peri_clk(void);



/**
 * \brief           Check IC version
 * \retval          IC version
 * \details         Return IC version information
 *                      Bit7 ~ Bit4 is chip_revision
 *                      Bit15 ~ Bit8 is chip_id
 */
__STATIC_INLINE uint32_t get_chip_version(void) {
    return ((uint32_t)(SYSCTRL->soc_chip_info.reg));
}

/** 
 * \brief           Use to save some retention value.
 * \param           index: The index for which scratchpad register to save
 *                  It should be 0~3.
 * \param           value: register value
 * \details         Please notice when system power-reset (cold boot), all system retention scratchpad register (0~3)
 */
__STATIC_INLINE void sys_set_retention_reg(uint32_t index, uint32_t value) {
    if (index < 3) {
        outp32((((uint8_t *)DPD_CTRL) + DPD_RETENTION_BASE + (index << 2)), value);
    }
}

/** 
 * \brief           Use to get retention value.
 * \param[in]       index: The index for which scratchpad register to get
 *                         It should be 0~3.
 * \param[out]      *value: the address for return value.
 */
__STATIC_INLINE void sys_get_retention_reg(uint32_t index, uint32_t *value) {
    if (index < 3) {
        *value =  inp32((((uint8_t *)DPD_CTRL) + DPD_RETENTION_BASE + (index << 2)));
    } else {
        *value = 0;
    }
}

/** for security world using
 * \brief sys_get_retention_reg. Use to get retention value.
 *
 * \param[in]   index:     The index for which scratchpad register to get
 *                         It should be 0~3.
 * \param[out]  *value     the address for return value.
 *
 *
 */
__STATIC_INLINE void set_deepsleep_wakeup_pin(uint32_t value) {
    GPIO->set_ds_en |= value;
}

/** for security world using
 * \brief           sys_get_retention_reg. Use to get retention value.
 *
 * \param[in]       index:     The index for which scratchpad register to get
 *                              It should be 0~3.
 * \param[out]      *value     the address for return value.
 *
 *
 */
__STATIC_INLINE void set_deepsleep_wakeup_invert(uint32_t value) {
    GPIO->set_ds_inv |= value;
}

/**
 * \brief           Set sram shutdown in normal
 * \param[in]       value: 
 */
__STATIC_INLINE void set_sram_shutdown_normal(uint32_t value) {
    SYSCTRL->sram_lowpower_1.reg |= value;
}

/**
 * \brief           Set sram shutdown in sleep
 * \param[in]       value: 
 */
__STATIC_INLINE void set_sram_shutdown_sleep(uint32_t value) {
    SYSCTRL->sram_lowpower_1.reg |= (value << 16) ;
}

/**
 * \brief           Set sram shutdown in deepsleep
 * \param[in]       value: 
 */
__STATIC_INLINE void set_sram_shutdown_deepsleep(uint32_t value) {
    SYSCTRL->sram_lowpower_2.reg |= value;
}

__STATIC_INLINE void set_peripheral_power_off (uint32_t peripwroffctlbit, uint32_t isenabled) {
    if (isenabled == 1)
    {
        SYSCTRL->sram_lowpower_3.reg |= peripwroffctlbit;
    }
    else
    {
        SYSCTRL->sram_lowpower_3.reg &= ~peripwroffctlbit;
    }

}

/**
 * \brief           Enable secure write protect
 * \details         Notice: if postmasking flag set... write_otp_key
 *                  is no use. write_otp_key is only use when postmasking is
 *                  not setting.
 */
__STATIC_INLINE void enable_secure_write_protect(void) {
    SEC_CTRL->sec_otp_write_key = SEC_WRITE_OTP_MAGIC_KEY;
}

/**
 * \brief           Disable secure write protect
 */
__STATIC_INLINE void disable_secure_write_protect(void) {
    SEC_CTRL->sec_otp_write_key = 0;
}

/**
 * \brief           Enable pin schmitt
 */
void pin_enable_schmitt(uint32_t pin_number);

/**
 * \brief           Disable pin schmitt
 */
void pin_disable_schmitt(uint32_t pin_number);

/**
 * \brief           Enable pin filter
 */
void pin_enable_filter(uint32_t pin_number);

/**
 * \brief           Disable pin filter
 */
void pin_disable_filter(uint32_t pin_number);

/**
 * \brief           Disable pin filter
 */
void slow_clock_calibration(slow_clock_select_t rco_select);
/**
 * \brief           Select Slow clock source.
 * \param[in]       mode:
 *                  SLOW_CLOCK_INTERNAL   --- default value.
 *                      If system don't call this function, then slow clock source is from internal RCO by default.
 *                  SLOW_CLOCK_FROM_GPIO ---
 *                      If system set this mode, system should use an external 32K source from GPIO.
 */
void set_slow_clock_source(uint32_t mode);
/**
 * \brief           Set Slow clock source gpio.
 * \param[in]       pin_number: gpio pin number
 *   
 */
void set_ext32k_pin(uint32_t pin_number);

/*@}*/ /* end of RT584_DRIVER SYSCTRL */

#ifdef __cplusplus
}
#endif

#endif /* End of SYSCTRL_H */
