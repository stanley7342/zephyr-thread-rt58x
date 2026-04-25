/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            mcu.h
 * \brief           mcu header file (cm33)
 */
/*
 * This file is part of library_name.
 * Author: ives.lee
 */




#ifndef CM33_H
#define CM33_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <arm_cmse.h>     // CMSE definitions


/**
 * \defgroup        CM33 Cm33
 * \ingroup         RT584_DRIVER
 * \brief           Define Cm33 definitions, structures, and functions
 * @{
 */

/**
 * \brief           Interrupt Number Definition
 */
typedef enum IRQn_Type
{
    /******  Cortex-CM33 Processor Exceptions Numbers *************************************************/
    NonMaskableInt_IRQn           = -14,    /*!<  2 Cortex-M33 Non Maskable Interrupt                */
    HardFault_IRQn                = -13,    /*!<  3 Cortex-M33 Hard Fault Interrupt                  */
    MemoryManagement_IRQn         = -12,    /*!<  4 Cortex-M33 Memory Management Interrupt           */
    BusFault_IRQn                 = -11,    /*!<  5 Cortex-M33 Bus Fault Interrupt                   */
    UsageFault_IRQn               = -10,    /*!<  6 Cortex-M33 Usage Fault Interrupt                 */
    SecureFault_IRQn              = -9,     /*!<  7 Cortex-M33 Secure Fault Interrupt                */
    SVCall_IRQn                   = -5,     /*!< 11 Cortex-M33 SV Call Interrupt                     */
    DebugMonitor_IRQn             = -4,     /*!< 12 Cortex-M33 Debug Monitor Interrupt               */
    PendSV_IRQn                   = -2,     /*!< 14 Cortex-M33 Pend SV Interrupt                     */
    SysTick_IRQn                  = -1,     /*!< 15 Cortex-M33 System Tick Interrupt                 */

    /******  RT584_CM33 Specific Interrupt Numbers *************************************************/
    Gpio_IRQn                     = 0,       /*!< GPIO combined Interrupt                          */

    Timer0_IRQn                   = 1,       /*!< TIMER 0 Interrupt                                */
    Timer1_IRQn                   = 2,       /*!< TIMER 1 Interrupt                                */
    Timer2_IRQn                   = 3,       /*!< TIMER 2 Interrupt                                */
    SlowTimer0_IRQn               = 4,       /*!< SLOW TIMER 0 Interrupt                           */
    SlowTimer1_IRQn               = 5,       /*!< SLOW TIMER 1 Interrupt                           */
    Wdt_IRQn                      = 6,       /*!< WatchDog Interrupt                               */
    Rtc_IRQn                      = 7,       /*!< RTC Interrupt                                    */

    SWI0_IRQn                    = 9,       /*!< SOFTWARE0  Interrupt                             */
    SWI1_IRQn                    = 10,      /*!< SOFTWARE1  Interrupt                             */

    Dma0_IRQn                  = 12,      /*!< DMA Channel 0 Interrupt                          */
    Dma1_IRQn                  = 13,      /*!< DMA Channel 1 Interrupt                          */

    Uart0_IRQn                    = 16,      /*!< UART 0 Interrupt                                 */
    Uart1_IRQn                    = 17,      /*!< UART 1 Interrupt                                 */
    Uart2_IRQn                    = 18,      /*!< UART 2 Interrupt                                 */

    Irm_IRQn                      = 20,      /*!< IRM Interrupt                                    */

    I2C_Master0_IRQn              = 21,      /*!< I2C Master0 Interrupt                            */
    I2C_Master1_IRQn              = 22,      /*!< I2C Master1 Interrupt                            */
    I2C_Slave_IRQn                = 23,      /*!< I2C Slave Interrupt                              */

    Qspi0_IRQn                    = 25,      /*!< QSPI0 Interrupt                                  */
    Qspi1_IRQn                    = 26,      /*!< QSPI1 Interrupt                                  */

    I2s0_IRQn                     = 29,      /*!< I2S0  Interrupt                                  */
    Soft_IRQn                     = 30,      /*!< SOFTWARE Interrupt                               */
    Pwm0_IRQn                     = 32,      /*!< PWM0 Interrupt                                   */
    Pwm1_IRQn                     = 33,      /*!< PWM1 Interrupt                                   */
    Pwm2_IRQn                     = 34,      /*!< PWM2 Interrupt                                   */
    Pwm3_IRQn                     = 35,      /*!< PWM3 Interrupt                                   */
    Pwm4_IRQn                     = 36,      /*!< PWM4 Interrupt                                   */

    FlashCtl_IRQn                 = 39,      /*!< FlashCtl Interrupt                               */
    OTP_IRQn                      = 40,      /*!< OTP Interrupt                                    */
    Crypto_IRQn                   = 41,      /*!< Crypto  Interrupt                                */
    Bod_Comp_IRQn                 = 42,      /*!< BOD COMPARATOR  Interrupt                        */

    CCM_AES_IQRn                  = 43,      /*!< AES CCM Interrupt                                */
    Sec_Ctrl_IQRn                 = 44,      /*!< SECURE CTRL Interrupt                            */

    CommSubsystem_IRQn            = 45,      /*!< COMM_SUBSYSTEM_COMM Interrupt                    */

    Sadc_IRQn                     = 46,      /*!< SADC Interrupt                                   */
    Aux_Comp_IRQn                 = 47,      /*!< AUX COMPARATOR Interrupt                         */

} IRQn_Type;

/**
 * \brief           Processor and Core Peripheral Section
 */
/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined (__ICCARM__)
#pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc11-extensions"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
/* anonymous unions are enabled by default */
#elif defined (__TMS470__)
/* anonymous unions are enabled by default */
#elif defined (__TASKING__)
#pragma warning 586
#elif defined (__CSMC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif


#define __CM33_REV                0x0001U       /*!< Core revision r0p1 */
#define __MPU_PRESENT                  1U       /*!< Defines if an MPU is present or not */
#define __NVIC_PRIO_BITS               3U       /*!< Number of priority bits implemented in the NVIC */
#define __Vendor_SysTickConfig         0U       /*!< Vendor specific implementation of SysTickConfig is defined */
#define __FPU_PRESENT                  1U       /*!< Defines if an FPU is present or not */
#define __DSP_PRESENT                  1U       /*!< Defines if Armv8-M Mainline core supports DSP instructions */
#define __SAUREGION_PRESENT            1U       /*!< Defines if an SAU is present or not */
#define __VTOR_PRESENT                 1U       /*!< VTOR present */

#include "core_cm33.h"                          /*!< Core Peripheral Access Layer */
#include "system_mcu.h"



#include "status.h"

#include "sysfun.h"
#include "assert_help.h"

/**
 * \brief           Peripheral Register Header
 */
#if !defined(CONFIG_BOOTLOADER)
#include "comm_subsystem_ahb_reg.h"
#endif

#include "reg_base.h"
#include "rco1m_reg.h"
#include "rco32k_reg.h"
#include "pufs_rt_regs.h"
#include "aux_comp_reg.h"
#include "bod_comp_reg.h"
#include "crypto_reg.h"
#include "dma_reg.h"
#include "dpd_reg.h"
#include "flashctl_reg.h"
#include "gpio_reg.h"
#include "i2c_master_reg.h"
#include "i2c_slave_reg.h"
#include "i2s_reg.h"
#include "irm_reg.h"
#include "pmu_reg.h"
#include "pwm_reg.h"
#include "qspi_reg.h"
#include "rtc_reg.h"
#include "sadc_reg.h"
#include "sec_ctrl_reg.h"
#include "swi_reg.h"
#include "sysctrl_reg.h"
#include "timer_reg.h"
#include "trng_reg.h"
#include "uart_reg.h"
#include "wdt_reg.h"
#include "xdma_reg.h"

/* --------------------  End of section using anonymous unions  ------------------- */
#if   defined (__CC_ARM)
#pragma pop
#elif defined (__ICCARM__)
/* leave anonymous unions enabled */
#elif (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic pop
#elif defined (__GNUC__)
/* anonymous unions are enabled by default */
#elif defined (__TMS470__)
/* anonymous unions are enabled by default */
#elif defined (__TASKING__)
#pragma warning restore
#elif defined (__CSMC__)
/* anonymous unions are enabled by default */
#else
#warning Not supported compiler type
#endif


/**
 * \brief           Peripheral declaration
 */
#if defined(CONFIG_SYSCTRL_SECURE_EN)
#define SYSCTRL             ((sysctrl_t *) (SYSCTRL_BASE | PERIPH_SECURE_OFFSET))
#else
#define SYSCTRL             ((sysctrl_t *) SYSCTRL_BASE)
#endif

#if defined(CONFIG_GPIO_SECURE_EN)
#define GPIO                  ((gpio_t *) (GPIO_BASE | PERIPH_SECURE_OFFSET))
#else
#define GPIO                  ((gpio_t *) GPIO_BASE)
#endif

#if defined(CONFIG_RTC_SECURE_EN)
#define RTC                   ((rtc_t *) (RTC_BASE | PERIPH_SECURE_OFFSET))
#else
#define RTC                   ((rtc_t *) RTC_BASE)
#endif

#if defined(CONFIG_DPD_SECURE_EN)
#define DPD_CTRL            ((dpd_t *) (DPD_BASE | PERIPH_SECURE_OFFSET))
#else
#define DPD_CTRL            ((dpd_t *) DPD_BASE)
#endif

#if defined(CONFIG_SOC_PMU_SECURE_EN)
#define PMU_CTRL            ((pmu_t *) (SOC_PMU_BASE | PERIPH_SECURE_OFFSET))
#else
#define PMU_CTRL            ((pmu_t *) SOC_PMU_BASE)
#endif

#if defined(CONFIG_FLASHCTRL_SECURE_EN)
#define FLASH            ((flashctl_t *) (FLASHCTRL_BASE | PERIPH_SECURE_OFFSET))
#else
#define FLASH            ((flashctl_t *) (FLASHCTRL_BASE))
#endif

#if defined(CONFIG_TIMER0_SECURE_EN)
#define TIMER0            ((timern_t *) (TIMER0_BASE | PERIPH_SECURE_OFFSET))
#else
#define TIMER0            ((timern_t *) TIMER0_BASE)
#endif

#if defined(CONFIG_TIMER1_SECURE_EN)
#define TIMER1            ((timern_t *) (TIMER1_BASE | PERIPH_SECURE_OFFSET))
#else
#define TIMER1            ((timern_t *) TIMER1_BASE)
#endif

#if defined(CONFIG_TIMER2_SECURE_EN)
#define TIMER2            ((timern_t *) (TIMER2_BASE | PERIPH_SECURE_OFFSET))
#else
#define TIMER2            ((timern_t *) TIMER2_BASE)
#endif

#if defined(CONFIG_SLOWTIMER0_SECURE_EN)
#define SLOWTIMER0            ((slowtimern_t *) (SLOWTIMER0_BASE | PERIPH_SECURE_OFFSET))
#else
#define SLOWTIMER0            ((slowtimern_t *) SLOWTIMER0_BASE)
#endif

#if defined(CONFIG_SLOWTIMER1_SECURE_EN)
#define SLOWTIMER1            ((slowtimern_t *) (SLOWTIMER1_BASE | PERIPH_SECURE_OFFSET))
#else
#define SLOWTIMER1            ((slowtimern_t *) SLOWTIMER1_BASE)
#endif

#if defined(CONFIG_WDT_SECURE_EN)
#define WDT                   ((wdt_t *) (WDT_BASE | PERIPH_SECURE_OFFSET))
#else
#define WDT ((wdt_t*)WDT_BASE)
#endif

#if defined(CONFIG_UART0_SECURE_EN)
#define UART0                 ((uart_t *) (UART0_BASE | PERIPH_SECURE_OFFSET))
#else
#define UART0                 ((uart_t *) UART0_BASE)
#endif

#if defined(CONFIG_UART1_SECURE_EN)
#define UART1                 ((uart_t *) (UART1_BASE | PERIPH_SECURE_OFFSET))
#else
#define UART1                 ((uart_t *) UART1_BASE)
#endif

#if defined(CONFIG_UART2_SECURE_EN)
#define UART2                 ((uart_t *) (UART2_BASE | PERIPH_SECURE_OFFSET))
#else
#define UART2                 ((uart_t *) UART2_BASE)
#endif

#if defined(CONFIG_PWM_SECURE_EN)
#define PWM0                ((pwm_t *) (PWM0_BASE | PERIPH_SECURE_OFFSET))
#define PWM1                ((pwm_t *) (PWM1_BASE | PERIPH_SECURE_OFFSET))
#define PWM2                ((pwm_t *) (PWM2_BASE | PERIPH_SECURE_OFFSET))
#define PWM3                ((pwm_t *) (PWM3_BASE | PERIPH_SECURE_OFFSET))
#define PWM4                ((pwm_t *) (PWM4_BASE | PERIPH_SECURE_OFFSET))
#else
#define PWM0                ((pwm_t *) PWM0_BASE)
#define PWM1                ((pwm_t *) PWM1_BASE)
#define PWM2                ((pwm_t *) PWM2_BASE)
#define PWM3                ((pwm_t *) PWM3_BASE)
#define PWM4                ((pwm_t *) PWM4_BASE)
#endif

#if defined(CONFIG_I2C_MASTER0_SECURE_EN)
#define I2C_MASTER0           ((i2c_master_t *) (I2C_MASTER0_BASE | PERIPH_SECURE_OFFSET))
#else
#define I2C_MASTER0           ((i2c_master_t *) I2C_MASTER0_BASE)
#endif

#if defined(CONFIG_I2C_MASTER1_SECURE_EN)
#define I2C_MASTER1           ((i2c_master_t *) (I2C_MASTER1_BASE | PERIPH_SECURE_OFFSET))
#else
#define I2C_MASTER1           ((i2c_master_t *) I2C_MASTER1_BASE)
#endif

#if defined(CONFIG_I2C_SLAVE_SECURE_EN)
#define I2C_SLAVE             ((i2c_slave_t *) (I2C_SLAVE_BASE | PERIPH_SECURE_OFFSET))
#else
#define I2C_SLAVE             ((i2c_slave_t *) I2C_SLAVE_BASE)
#endif

#if defined(CONFIG_COMM_SUBSYSTEM_AHB_SECURE_EN)
#define COMM_SUBSYSTEM_AHB             ((comm_subsystem_ahb_t *) (COMM_SUBSYSTEM_AHB_BASE | PERIPH_SECURE_OFFSET))
#else
#define COMM_SUBSYSTEM_AHB             ((comm_subsystem_ahb_t *) COMM_SUBSYSTEM_AHB_BASE)
#endif

#if defined(CONFIG_RCO32K_CAL_SECURE_EN)
#define RCO32K_CAL            ((rco32k_cal_t *) (RCO32K_BASE | PERIPH_SECURE_OFFSET))
#else
#define RCO32K_CAL            ((rco32k_cal_t *) RCO32K_BASE)
#endif


#if defined(CONFIG_AUX_COMP_SECURE_EN)
#define AUX_COMP            ((aux_comp_ctl_t *) (AUX_COMP_BASE | PERIPH_SECURE_OFFSET))
#else
#define AUX_COMP            ((aux_comp_ctl_t *) AUX_COMP_BASE)
#endif

#if defined(CONFIG_BOD_COMP_SECURE_EN)
#define BOD_COMP            ((bod_comp_ctl_t *) (BOD_COMP_BASE | PERIPH_SECURE_OFFSET))
#else
#define BOD_COMP            ((bod_comp_ctl_t *) BOD_COMP_BASE)
#endif

#if defined(CONFIG_RCO1M_CAL_SECURE_EN)
#define RCO1M_CAL            ((rco1m_cal_t *) (RCO1M_BASE | PERIPH_SECURE_OFFSET))
#else
#define RCO1M_CAL            ((rco1m_cal_t *) RCO1M_BASE)
#endif

#if defined(CONFIG_QSPI0_SECURE_EN)
#define QSPI0                 ((qspi_t *) (QSPI0_BASE | PERIPH_SECURE_OFFSET))
#else
#define QSPI0                 ((qspi_t *) QSPI0_BASE)
#endif

#if defined(CONFIG_QSPI1_SECURE_EN)
#define QSPI1                 ((qspi_t *) (QSPI1_BASE | PERIPH_SECURE_OFFSET))
#else
#define QSPI1                 ((qspi_t *) QSPI1_BASE)
#endif

#if defined(CONFIG_IRM_SECURE_EN)
#define IRM                   ((irm_t *) (IRM_BASE | PERIPH_SECURE_OFFSET))
#else
#define IRM                   ((irm_t *) IRM_BASE)
#endif

#if defined(CONFIG_XDMA_SECURE_EN)
#define XDMA                  ((xdma_t *) (XDMA_BASE | PERIPH_SECURE_OFFSET))
#else
#define XDMA                  ((xdma_t *) XDMA_BASE)
#endif

#if defined(CONFIG_DMA0_SECURE_EN)
#define DMA0                  ((dma_t *) (DMA0_BASE | PERIPH_SECURE_OFFSET))
#else
#define DMA0                  ((dma_t *) DMA0_BASE)
#endif

#if defined(CONFIG_DMA1_SECURE_EN)
#define DMA1                  ((dma_t *) (DMA1_BASE | PERIPH_SECURE_OFFSET))
#else
#define DMA1                  ((dma_t *) DMA1_BASE)
#endif

#if defined(CONFIG_I2S0_SECURE_EN)
#define I2S_MASTER            ((i2s_t *) (I2S0_BASE | PERIPH_SECURE_OFFSET))
#else
#define I2S_MASTER            ((i2s_t *) I2S0_BASE)
#endif

#if defined(CONFIG_SADC_SECURE_EN)
#define SADC                  ((sadc_t *) (SADC_BASE | PERIPH_SECURE_OFFSET))
#else
#define SADC                  ((sadc_t *) SADC_BASE)
#endif

#if defined(CONFIG_SWI0_SECURE_EN)
#define SWI0               ((swi_t *) (SWI0_BASE | PERIPH_SECURE_OFFSET))
#else
#define SWI0               ((swi_t *) SWI0_BASE)
#endif

#if defined(CONFIG_SWI1_SECURE_EN)
#define SWI1               ((swi_t *) (SWI1_BASE | PERIPH_SECURE_OFFSET))
#else
#define SWI1               ((swi_t *) SWI1_BASE)
#endif


#define CRYPTO               ((crypto_t *)(CRYPTO_BASE))

#define TRNG_CTRL           ((trng_t *) (TRNG_BASE | PERIPH_SECURE_OFFSET))

/*IDAU Secure controller is only used in secure mode only*/
#define SEC_CTRL                ((sec_ctrl_t *)(SEC_CTRL_BASE))


/**
 * \brief           Cortex-M33 INTS Setting Define 
 */
#define  GPIO_INT_NONSEC              BIT0      /*!< (1UL << 0)  */
#define  TIMER0_INT_NONSEC            BIT1      /*!< (1UL << 1)  */
#define  TIMER1_INT_NONSEC            BIT2      /*!< (1UL << 2)  */
#define  TIMER2_INT_NONSEC            BIT3      /*!< (1UL << 3)  */
#define  TIMER32K0_INT_NONSEC         BIT4      /*!< (1UL << 4)  */
#define  TIMER32K1_INT_NONSEC         BIT5      /*!< (1UL << 5)  */
#define  WATCHDOG_INT_NONSEC          BIT6      /*!< (1UL << 6)  */
#define  RTC_INT_NONSEC               BIT7      /*!< (1UL << 7)  */
#define  SOFT0_INT_NONSEC             BIT9      /*!< (1UL << 9)  */
#define  SOFT1_INT_NONSEC             BIT10     /*!< (1UL << 10) */
#define  DMA_CH0_INT_NONSEC           BIT12     /*!< (1UL << 12) */
#define  DMA_CH1_INT_NONSEC           BIT13     /*!< (1UL << 13) */
#define  UART0_INT_NONSEC             BIT16     /*!< (1UL << 16) */
#define  UART1_INT_NONSEC             BIT17     /*!< (1UL << 17) */
#define  UART2_INT_NONSEC             BIT18     /*!< (1UL << 18) */
#define  IRM_INT_NONSEC               BIT20     /*!< (1UL << 20) */
#define  I2C_MASTER0_INT_NONSEC       BIT21     /*!< (1UL << 21) */
#define  I2C_MASTER1_INT_NONSEC       BIT22     /*!< (1UL << 22) */
#define  I2C_SLAVE_INT_NONSEC         BIT23     /*!< (1UL << 23) */
#define  QSPI0_INT_NONSEC             BIT25     /*!< (1UL << 25) */
#define  QSPI1_INT_NONSEC             BIT26     /*!< (1UL << 26) */
#define  I2S0_INT_NONSEC              BIT29     /*!< (1UL << 29) */

/*ITNS[1]*/
#define  PWM0_INT_NONSEC              BIT0      /*!< (1UL << 0)  */
#define  PWM1_INT_NONSEC              BIT1      /*!< (1UL << 1)  */
#define  PWM2_INT_NONSEC              BIT2      /*!< (1UL << 2)  */
#define  PWM3_INT_NONSEC              BIT3      /*!< (1UL << 3)  */
#define  PWM4_INT_NONSEC              BIT4      /*!< (1UL << 4)  */
#define  FLASH_INT_NONSEC             BIT7      /*!< (1UL << 7)  */
#define  PUF_TRNG_OTP_INT_NONSEC      BIT8      /*!< (1UL << 8)  */
#define  CRYPTO_INT_NONSEC            BIT9      /*!< (1UL << 9)  */
#define  BOD_COMP_INT_NONSEC          BIT10     /*!< (1UL << 10) */
#define  SEC_INT_NONSEC               BIT12     /*!< (1UL << 12) */
#define  CommSubsys_INT_NONSEC        BIT13     /*!< (1UL << 13) */
#define  SADC_INT_NONSEC              BIT14     /*!< (1UL << 14) */
#define  AUX_COMP_INT_NONSEC          BIT16     /*!< (1UL << 16) */



/**
  * \brief          Set a 32-bit unsigned value to specified I/O port
  * \param[in]      port: Port address to set 32-bit data
  * \param[in]      value: Value to write to I/O port
  * \note           The output port must be 32-bit aligned
  */
#define outp32(port,value)    *((volatile uint32_t *)(port)) = (value)

/**
  * \brief          Get a 32-bit unsigned value from specified I/O port
  * \param[in]      port: Port address to get 32-bit data from
  * \return         32-bit unsigned value stored in specified I/O port
  * \note T         he input port must be 32-bit aligned
  */
#define inp32(port)           (*((volatile uint32_t *)(port)))

/**
  * \brief          Set a 16-bit unsigned value to specified I/O port
  * \param[in]      port: Port address to set 16-bit data
  * \param[in]      value: Value to write to I/O port
  * \note           The output port must be 16-bit aligned
  */
#define outp16(port,value)    *((volatile uint16_t *)(port)) = (value)

/**
  * \brief          Get a 16-bit unsigned value from specified I/O port
  * \param[in]      port: Port address to get 16-bit data from
  * \return         16-bit unsigned value stored in specified I/O port
  * \note           The input port must be 16-bit aligned
  */
#define inp16(port)           (*((volatile uint16_t *)(port)))

/**
  * \brief          Set a 8-bit unsigned value to specified I/O port
  * \param[in]      port: Port address to set 8-bit data
  * \param[in]      value: Value to write to I/O port
  */
#define outp8(port,value)     *((volatile uint8_t *)(port)) = (value)

/**
  * \brief          Get a 8-bit unsigned value from specified I/O port
  * \param[in]      port: Port address to get 8-bit data from
  * \return         8-bit unsigned value stored in specified I/O port
  */
#define inp8(port)            (*((volatile uint8_t *)(port)))


/**
 * \brief           Legacy Constants
 */
#ifndef NULL
#define NULL (0)                                /*!< NULL pointer */
#endif

#define TRUE           (1UL)                    /*!< Boolean true, define to use in API parameters or return value */
#define FALSE          (0UL)                    /*!< Boolean false, define to use in API parameters or return value */

#define ENABLE         (1UL)                    /*!< Enable, define to use in API parameters */
#define DISABLE        (0UL)                    /*!< Disable, define to use in API parameters */

/* Define one bit mask */
#define BIT0     (0x00000001UL)                 /*!< Bit 0 mask of an 32 bit integer */
#define BIT1     (0x00000002UL)                 /*!< Bit 1 mask of an 32 bit integer */
#define BIT2     (0x00000004UL)                 /*!< Bit 2 mask of an 32 bit integer */
#define BIT3     (0x00000008UL)                 /*!< Bit 3 mask of an 32 bit integer */
#define BIT4     (0x00000010UL)                 /*!< Bit 4 mask of an 32 bit integer */
#define BIT5     (0x00000020UL)                 /*!< Bit 5 mask of an 32 bit integer */
#define BIT6     (0x00000040UL)                 /*!< Bit 6 mask of an 32 bit integer */
#define BIT7     (0x00000080UL)                 /*!< Bit 7 mask of an 32 bit integer */
#define BIT8     (0x00000100UL)                 /*!< Bit 8 mask of an 32 bit integer */
#define BIT9     (0x00000200UL)                 /*!< Bit 9 mask of an 32 bit integer */
#define BIT10    (0x00000400UL)                 /*!< Bit 10 mask of an 32 bit integer */
#define BIT11    (0x00000800UL)                 /*!< Bit 11 mask of an 32 bit integer */
#define BIT12    (0x00001000UL)                 /*!< Bit 12 mask of an 32 bit integer */
#define BIT13    (0x00002000UL)                 /*!< Bit 13 mask of an 32 bit integer */
#define BIT14    (0x00004000UL)                 /*!< Bit 14 mask of an 32 bit integer */
#define BIT15    (0x00008000UL)                 /*!< Bit 15 mask of an 32 bit integer */
#define BIT16    (0x00010000UL)                 /*!< Bit 16 mask of an 32 bit integer */
#define BIT17    (0x00020000UL)                 /*!< Bit 17 mask of an 32 bit integer */
#define BIT18    (0x00040000UL)                 /*!< Bit 18 mask of an 32 bit integer */
#define BIT19    (0x00080000UL)                 /*!< Bit 19 mask of an 32 bit integer */
#define BIT20    (0x00100000UL)                 /*!< Bit 20 mask of an 32 bit integer */
#define BIT21    (0x00200000UL)                 /*!< Bit 21 mask of an 32 bit integer */
#define BIT22    (0x00400000UL)                 /*!< Bit 22 mask of an 32 bit integer */
#define BIT23    (0x00800000UL)                 /*!< Bit 23 mask of an 32 bit integer */
#define BIT24    (0x01000000UL)                 /*!< Bit 24 mask of an 32 bit integer */
#define BIT25    (0x02000000UL)                 /*!< Bit 25 mask of an 32 bit integer */
#define BIT26    (0x04000000UL)                 /*!< Bit 26 mask of an 32 bit integer */
#define BIT27    (0x08000000UL)                 /*!< Bit 27 mask of an 32 bit integer */
#define BIT28    (0x10000000UL)                 /*!< Bit 28 mask of an 32 bit integer */
#define BIT29    (0x20000000UL)                 /*!< Bit 29 mask of an 32 bit integer */
#define BIT30    (0x40000000UL)                 /*!< Bit 30 mask of an 32 bit integer */
#define BIT31    (0x80000000UL)                 /*!< Bit 31 mask of an 32 bit integer */

/* Byte Mask Definitions */
#define BYTE0_Msk   (0x000000FFUL)              /*!< Mask to get bit0~bit7 from a 32 bit integer */
#define BYTE1_Msk   (0x0000FF00UL)              /*!< Mask to get bit8~bit15 from a 32 bit integer */
#define BYTE2_Msk   (0x00FF0000UL)              /*!< Mask to get bit16~bit23 from a 32 bit integer */
#define BYTE3_Msk   (0xFF000000UL)              /*!< Mask to get bit24~bit31 from a 32 bit integer */

#define GET_BYTE0(u32Param) (((u32Param) & BYTE0_Msk)      )    /*!< Extract Byte 0 (Bit  0~ 7) from parameter u32Param */
#define GET_BYTE1(u32Param) (((u32Param) & BYTE1_Msk) >>  8)    /*!< Extract Byte 1 (Bit  8~15) from parameter u32Param */
#define GET_BYTE2(u32Param) (((u32Param) & BYTE2_Msk) >> 16)    /*!< Extract Byte 2 (Bit 16~23) from parameter u32Param */
#define GET_BYTE3(u32Param) (((u32Param) & BYTE3_Msk) >> 24)    /*!< Extract Byte 3 (Bit 24~31) from parameter u32Param */



/*This is a software breakpoint, which will stop the firmware when using ICE debugging*/
#define BREAK()     do{ __asm volatile ("BKPT    #0"); while(1);} while(0)

#define ASSERT()    do{ while(1);} while(0)

/*@}*/ /* end of RT584_DRIVER CM33 */

#ifdef __cplusplus
}
#endif

#endif /* End of CM33_H */
