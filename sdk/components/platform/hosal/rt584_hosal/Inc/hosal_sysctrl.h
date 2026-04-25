/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_sysctrl.h
 * \brief           Hosal system control driver header file
 */
/*
 * Author:          
 */

#ifndef HOSAL_SYSCTRL_H
#define HOSAL_SYSCTRL_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "sysctrl.h"
#include "dwt.h"



/**
 * \defgroup        HOSAL_SYSCTRL Hosal sysctrl
 * \ingroup         RT584_HOSAL
 * \brief           Define Hosal sysctrl definitions, structures, and functions
 * @{
 */

/**
 * \brief           Output pin mode 
 */
#define HOSAL_MODE_GPIO                 MODE_GPIO
#define HOSAL_MODE_UART0_TX             MODE_UART0_TX
#define HOSAL_MODE_UART0_RX             MODE_UART0_RX
#define HOSAL_MODE_UART1_TX             MODE_UART1_TX
#define HOSAL_MODE_UART1_RX             MODE_UART1_RX
#define HOSAL_MODE_UART1_RTSN           MODE_UART1_RTSN
#define HOSAL_MODE_UART1_CTSN           MODE_UART1_CTSN
#define HOSAL_MODE_UART2_TX             MODE_UART2_TX
#define HOSAL_MODE_UART2_RX             MODE_UART2_RX
#define HOSAL_MODE_UART2_RTSN           MODE_UART2_RTSN
#define HOSAL_MODE_UART2_CTSN           MODE_UART2_CTSN
#define HOSAL_MODE_PWM0                 MODE_PWM0
#define HOSAL_MODE_PWM1                 MODE_PWM1
#define HOSAL_MODE_PWM2                 MODE_PWM2
#define HOSAL_MODE_PWM3                 MODE_PWM3
#define HOSAL_MODE_PWM4                 MODE_PWM4
#define HOSAL_MODE_IRM                  MODE_IRM
#define HOSAL_MODE_I2CM0_SCL            MODE_I2CM0_SCL
#define HOSAL_MODE_I2CM0_SDA            MODE_I2CM0_SDA
#define HOSAL_MODE_I2CM1_SCL            MODE_I2CM1_SCL
#define HOSAL_MODE_I2CM1_SDA            MODE_I2CM1_SDA
#define HOSAL_MODE_I2CS_SCL             MODE_I2CS_SCL
#define HOSAL_MODE_I2CS_SDA             MODE_I2CS_SDA


#define HOSAL_MODE_SPI0_MASTER_SCLK    MODE_SPI0_MASTER_SCLK 
#define HOSAL_MODE_SPI0_MASTER_CSN0    MODE_SPI0_MASTER_CSN0   
#define HOSAL_MODE_SPI0_MASTER_MOSI    MODE_SPI0_MASTER_MOSI   
#define HOSAL_MODE_SPI0_MASTER_MISO    MODE_SPI0_MASTER_MISO  
#define HOSAL_MODE_SPI0_SLAVE_SCLK      MODE_SPI0_SLAVE_SCLK
#define HOSAL_MODE_SPI0_SLAVE_CSN0      MODE_SPI0_SLAVE_CSN0
#define HOSAL_MODE_SPI0_SLAVE_MOSI      MODE_SPI0_SLAVE_MOSI
#define HOSAL_MODE_SPI0_SLAVE_MISO      MODE_SPI0_SLAVE_MISO


#define HOSAL_MODE_SPI1_MASTER_SCLK     MODE_SPI1_MASTER_SCLK
#define HOSAL_MODE_SPI1_MASTER_CSN0     MODE_SPI1_MASTER_CSN0
#define HOSAL_MODE_SPI1_MASTER_MOSI     MODE_SPI1_MASTER_MOSI
#define HOSAL_MODE_SPI1_MASTER_MISO     MODE_SPI1_MASTER_MISO
#define HOSAL_MODE_SPI1_SLAVE_SCLK      MODE_SPI1_SLAVE_SCLK
#define HOSAL_MODE_SPI1_SLAVE_CSN0      MODE_SPI1_SLAVE_CSN0
#define HOSAL_MODE_SPI1_SLAVE_MOSI      MODE_SPI1_SLAVE_MOSI
#define HOSAL_MODE_SPI1_SLAVE_MISO      MODE_SPI1_SLAVE_MISO
#define HOSAL_MODE_SPI0_MASTER_SDATA0   MODE_SPI0_MASTER_SDATA0
#define HOSAL_MODE_SPI0_MASTER_SDATA1   MODE_SPI0_MASTER_SDATA1
#define HOSAL_MODE_SPI0_MASTER_SDATA2   MODE_SPI0_MASTER_SDATA2
#define HOSAL_MODE_SPI0_MASTER_SDATA3   MODE_SPI0_MASTER_SDATA3
#define HOSAL_MODE_SPI1_MASTER_SDATA0   MODE_SPI1_MASTER_SDATA0
#define HOSAL_MODE_SPI1_MASTER_SDATA1   MODE_SPI1_MASTER_SDATA1
#define HOSAL_MODE_SPI1_MASTER_SDATA2   MODE_SPI1_MASTER_SDATA2
#define HOSAL_MODE_SPI1_MASTER_SDATA3   MODE_SPI1_MASTER_SDATA3
#define HOSAL_MODE_SPI0_MASTER_CSN1     MODE_SPI0_MASTER_CSN1
#define HOSAL_MODE_SPI0_MASTER_CSN2     MODE_SPI0_MASTER_CSN2
#define HOSAL_MODE_SPI0_MASTER_CSN3     MODE_SPI0_MASTER_CSN3
#define HOSAL_MODE_SPI0_SLAVE_CSN1      MODE_SPI0_SLAVE_CSN1
#define HOSAL_MODE_SPI0_SLAVE_CSN2      MODE_SPI0_SLAVE_CSN2
#define HOSAL_MODE_SPI0_SLAVE_CSN3      MODE_SPI0_SLAVE_CSN3
#define HOSAL_MODE_SPI1_MASTER_CSN1     MODE_SPI1_MASTER_CSN1
#define HOSAL_MODE_SPI1_MASTER_CSN2     MODE_SPI1_MASTER_CSN2
#define HOSAL_MODE_SPI1_MASTER_CSN3     MODE_SPI1_MASTER_CSN3
#define HOSAL_MODE_SPI1_SLAVE_CSN1      MODE_SPI1_SLAVE_CSN1
#define HOSAL_MODE_SPI1_SLAVE_CSN2      MODE_SPI1_SLAVE_CSN2
#define HOSAL_MODE_SPI1_SLAVE_CSN3      MODE_SPI1_SLAVE_CSN3
#define HOSAL_MODE_I2S_BCK              MODE_I2S_BCK
#define HOSAL_MODE_I2S_WCK              MODE_I2S_WCK
#define HOSAL_MODE_I2S_SDO              MODE_I2S_SDO
#define HOSAL_MODE_I2S_SDI              MODE_I2S_SDI
#define HOSAL_MODE_I2S_MCLK             MODE_I2S_MCLK

/**
 * \brief           Gpio pull up define
 */
#define HOSAL_PULL_NONE         PULL_NONE
#define HOSAL_PULL_DOWN_10K     PULLDOWN_10K
#define HOSAL_PULL_DOWN_100K    PULLDOWN_100K
#define HOSAL_PULL_DOWN_1M      PULLDOWN_1M
#define HOSAL_PULL_UP_10K       PULLUP_10K
#define HOSAL_PULL_UP_100K      PULLUP_100K
#define HOSAL_PULL_UP_1M        PULLUP_1M

/**
 * \brief           system clock define
 */
#define HOSAL_SYSTEM_DEF_CLK_32M    SYS_32MHZ_CLK 
#define HOSAL_SYSTEM_DEF_CLK_16M    SYS_16MHZ_CLK 
#define HOSAL_SYSTEM_DEF_CLK_1M     SYS_RCO1MHZ_CLK 
#define HOSAL_SYSTEM_PLLCLK_48M     SYS_48MHZ_PLLCLK
#define HOSAL_SYSTEM_PLLCLK_64M     SYS_64MHZ_PLLCLK 
#define HOSAL_SYSTEM_PLLCLK_RVD1    SYS_RVD1_PLLCLK  
#define HOSAL_SYSTEM_PLLCLK_RVD2    SYS_RVD2_PLLCLK  
#define HOSAL_SYSTEM_PLLCLK_24M     SYS_24MHZ_PLLCLK 
#define HOSAL_SYSTEM_PLLCLK_32M     SYS_32MHZ_PLLCLK 
#define HOSAL_SYSTEM_PLLCLK_36M     SYS_36MHZ_PLLCLK
#define HOSAL_SYSTEM_PLLCLK_40M     SYS_40MHZ_PLLCLK 

/**
 * \brief           system clock define
 */
#define HOSAL_SYSTEM_CLK_32M         HCLK_SEL_32M
#define HOSAL_SYSTEM_CLK_PLL         HCLK_SEL_PLL
#define HOSAL_SYSTEM_CLK_16M         HCLK_SEL_16M
#define HOSAL_SYSTEM_CLK_1M          HCLK_SEL_RCO1M

/**
 * \brief           peripheral clock define
 */
#define HOSAL_PERIPHERAL_CLK_32M         PERCLK_SEL_32M
#define HOSAL_PERIPHERAL_CLK_16M         PERCLK_SEL_16M
#define HOSAL_PERIPHERAL_CLK_1M          PERCLK_SEL_RCO1M


/**
 * \brief           system control const define
 */
#define HOSAL_SET_TX_POWER_LEVEL         1
#define HOSAL_GET_TX_POWER_LEVEL         2
#define HOSAL_SLOW_CLOCK_CALIBRATION     3

/**
 * \brief           system control const define
 */
typedef enum {
    HOSAL_SYSCTRL_TIMEOUT_INIT = 1,
    HOSAL_SYSCTRL_TIMEOUT_UNINIT,
    HOSAL_SYSCTRL_TIMEOUT_START,
    HOSAL_SYSCTRL_TIMEOUT_CHECK,
    HOSAL_SYSCTRL_TIMEOUT_GET_REMAINING,
} hosal_sysctrl_cmd_t;

typedef struct{
    TimeoutTimer tmo;
    TimeoutCallback_t cb;  /* callback function */
    void *arg;             /* callback paramater */
    bool period;
}hosal_sys_tmo_t;

/**
 * \brief           hosal_delay_us
 * \param[in]       us: delay times    
 */
void hosal_delay_us(volatile uint32_t us);

/**
 * \brief           hosal_delay_ms
 * \param[in]       ms: delay times      
 */
void hosal_delay_ms(volatile uint32_t ms);

/**
 * \brief           Get pin function mode
 * \param[in]       pin_number: Specifies the pin number
 * \return          The pin function mode
 */
uint32_t hosal_pin_get_mode(uint32_t pin_number);

/**
 * \brief           Set pin function mode
 * \param[in]       pin_number: Specifies the pin number
 * \param[in]       mode: The specail function mode for the pin_number
 *                        Config GPIO To --> UART/I2S/PWM/SADC/I2C/SPI
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM
 */
uint32_t hosal_pin_set_mode(uint32_t pin_number, uint32_t mode);

/**
 * \brief           Set pin pull option
 * \param[in]       pin_number: Specifies the pin number
 * \param[in]       mode: The specail pull option for the pin_number
 *                        HOSAL_PULL_NONE        0
 *                        HOSAL_PULL_DOWN_10K    1
 *                        HOSAL_PULL_DOWN_100K   2
 *                        HOSAL_PULL_DOWN_1M     3
 *                        HOSAL_PULL_UP_10K      5
 *                        HOSAL_PULL_UP_100K     6
 *                        HOSAL_PULL_UP_1M       7
 */
void hosal_pin_set_pullopt(uint32_t pin_number, uint32_t mode);

/**
 * \brief           Set pin to opendrain option
 * \param[in]       pin_number: Specifies the pin number
 */
void hosal_enable_pin_opendrain(uint32_t pin_number);

/**
 * \brief           Disable pin to opendrain option
 * \param[in]       pin_number: Specifies the pin number
 */
void hosal_disable_pin_opendrain(uint32_t pin_number);

/**
 * \brief           Enable pin schmitt
 */
void hosal_pin_enable_schmitt(uint32_t pin_number);

/**
 * \brief disable pin schmitt
 */
void hosal_pin_disable_schmitt(uint32_t pin_number);

/**
 * \brief           Enable pin filter
 */
void hosal_pin_enable_filter(uint32_t pin_number);

/**
 * \brief           Disable pin filter
 */
void hosal_pin_disable_filter(uint32_t pin_number);

/**
 * \brief           Get periperhal clock
 */
uint32_t hosal_get_periperhal_clock(void);

/**
 * \brief           tx power level config
 */
void hosal_tx_power_level_ctrl(int ctl, void* cfg_para);

/**
 * \brief           hosal_get_rco_clock_tick
 */
int hosal_get_rco_clock_tick(uint32_t* rco_tick);

/**
 * \brief           hosal_syctrl_ioctl
 * \param[in]       uart_dev: uart device struct
 * \param[in]       ctl: control command
 * \param[in]       para: paramater
 * \return          Function status, STATUS_SUCCESS, STATUS_INVALID_PARAM, 
 *                  STATUS_INVALID_REQUEST
 */
int hosal_sysctrl_ioctrl(hosal_sys_tmo_t *sys_tmo,int ctl, void* para);

/*@}*/ /* end of RT584_HOSAL HOSAL_SYSCTRL */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_SYSCTRL_H */
