/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/****************************************************************************
 * @file     startup_cm33.c
 * @version
 * @brief   startup config
 *
*****************************************************************************/

#include "mcu.h"
#include "cmsis_gcc.h"

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;
#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    extern uint32_t __STACK_SEAL;
#endif

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler  (void);
void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler                     (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler               (void) __attribute__ ((weak));
void MemManage_Handler               (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler                (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler              (void) __attribute__ ((weak, alias("Default_Handler")));
void SecureFault_Handler             (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler                     (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler                (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler                 (void) __attribute__ ((weak, alias("Default_Handler")));


void gpio_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));

void timer0_handler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void timer1_handler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void timer2_handler                  (void) __attribute__ ((weak, alias("Default_Handler")));
void slowtimer0_handler               (void) __attribute__ ((weak, alias("Default_Handler")));
void slowtimer1_handler               (void) __attribute__ ((weak, alias("Default_Handler")));

void wdt_handler                     (void) __attribute__ ((weak, alias("Default_Handler")));
void rtc_handler                     (void) __attribute__ ((weak, alias("Default_Handler")));

void swi0_handler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void swi1_handler                   (void) __attribute__ ((weak, alias("Default_Handler")));

void dma0_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void dma1_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));

void uart0_handler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void uart1_handler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void uart2_handler                   (void) __attribute__ ((weak, alias("Default_Handler")));

void irm_handler                     (void) __attribute__ ((weak, alias("Default_Handler")));

void i2c_master0_handler             (void) __attribute__ ((weak, alias("Default_Handler")));
void i2c_master1_handler             (void) __attribute__ ((weak, alias("Default_Handler")));
void i2c_slave_handler               (void) __attribute__ ((weak, alias("Default_Handler")));

void qspi0_handler                   (void) __attribute__ ((weak, alias("Default_Handler")));
void qspi1_handler                   (void) __attribute__ ((weak, alias("Default_Handler")));

void i2s0_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));

void pwm0_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void pwm1_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void pwm2_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void pwm3_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void pwm4_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));

void flashctl_handler                (void) __attribute__ ((weak, alias("Default_Handler")));

void otp_handler                     (void) __attribute__ ((weak, alias("Default_Handler")));

void crypto_handler                  (void) __attribute__ ((weak, alias("Default_Handler")));

void bod_comp_handler                (void) __attribute__ ((weak, alias("Default_Handler")));

void CCM_AES_Handler                 (void) __attribute__ ((weak, alias("Default_Handler")));
void Sec_Ctrl_Handler                (void) __attribute__ ((weak, alias("Default_Handler")));

void CommSubsys_Handler              (void) __attribute__ ((weak, alias("Default_Handler")));
void sadc_handler                    (void) __attribute__ ((weak, alias("Default_Handler")));
void aux_comp_handler                (void) __attribute__ ((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[64];
const VECTOR_TABLE_Type __VECTOR_TABLE[64] __VECTOR_TABLE_ATTRIBUTE =
{
    (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*     Initial Stack Pointer */
    Reset_Handler,                            /*     Reset Handler */
    NMI_Handler,                              /* -14 NMI Handler */
    HardFault_Handler,                        /* -13 Hard Fault Handler */
    MemManage_Handler,                        /* -12 MPU Fault Handler */
    BusFault_Handler,                         /* -11 Bus Fault Handler */
    UsageFault_Handler,                       /* -10 Usage Fault Handler */
    SecureFault_Handler,                      /*  -9 Secure Fault Handler */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    0,                                        /*     Reserved */
    SVC_Handler,                              /*  -5 SVC Handler */
    DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
    0,                                        /*     Reserved */
    PendSV_Handler,                           /*  -2 PendSV Handler */
    SysTick_Handler,                          /*  -1 SysTick Handler */

    /* Core IoT Interrupts */
    gpio_handler,                             /*   0 GPIO Handler */
    timer0_handler,                           /*   1 Timer0 Handler */
    timer1_handler,                           /*   2 Timer1 Handler */
    timer2_handler,                           /*   3 Timer2 Handler */
    slowtimer0_handler,                        /*   4 Slow timer0 Handler */
    slowtimer1_handler,                        /*   5 Slow timer1 Handler */
    wdt_handler,                              /*   6 WatchDog Timer Handler */
    rtc_handler,                              /*   7 RTC Timer Handler */
    0,                                        /*   8 Reserved */
    swi0_handler,                            /*   9 Software0 Handler */
    swi1_handler,                            /*   10 Software1 Handler */
    0,                                        /*   11 Reserved */
    dma0_handler,                             /*   12 DMA Channel 0 Handler */
    dma1_handler,                             /*   13 DMA Channel 1 Handler */
    0,                                        /*   14 Reserved */
    0,                                        /*   15 Reserved */
    uart0_handler,                            /*   16 UART0 Handler */
    uart1_handler,                            /*   17 UART1 Handler */
    uart2_handler,                            /*   18 UART2 Handler */
    0,                                        /*   19 Reserved */
    irm_handler,                              /*   20 IRM Handler */
    i2c_master0_handler,                      /*   21 I2C Master0 Handler */
    i2c_master1_handler,                      /*   22 I2C Master1 Handler */
    i2c_slave_handler,                        /*   23 I2C Slave Handler*/
    0,                                        /*   24 Reserved */
    qspi0_handler,                            /*   25 QSPI0 Handler */
    qspi1_handler,                            /*   26 QSPI1 Handler */
    0,                                        /*   27 Reserved */
    0,                                        /*   28 Reserved */
    i2s0_handler,                             /*   29 I2S Handler */
    0,                                        /*   30 Reserved */
    0,                                        /*   31 Reserved */
    pwm0_handler,                             /*   32 PWM0 Handler */
    pwm1_handler,                             /*   33 PWM1 Handler */
    pwm2_handler,                             /*   34 PWM2 Handler */
    pwm3_handler,                             /*   35 PWM3 Handler */
    pwm4_handler,                             /*   36 PWM4 Handler */
    0,                                        /*   37 Reserved */
    0,                                        /*   38 Reserved */
    flashctl_handler,                         /*   39 Flash Controler Handler */
    otp_handler,                              /*   40 OTP Handler */
    crypto_handler,                           /*   41 Crypto Handler */
    bod_comp_handler,                         /*   42 Bod Comparator Handler */
    0,                                        /*   43 AES CCM Handler*/
    Sec_Ctrl_Handler,                         /*   44 Secure Control Handler*/
    CommSubsys_Handler,                       /*   45 Comm SubSystem Handler*/
    sadc_handler,                             /*   46 SADC Handler*/
    aux_comp_handler,                         /*   47 Aux Comparator Handler*/
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{

    __set_MSP((uint32_t)(&__INITIAL_SP));
    __set_PSP((uint32_t)(&__INITIAL_SP));

#if 1
    __set_MSPLIM((uint32_t)(&__STACK_LIMIT));
    __set_PSPLIM((uint32_t)(&__STACK_LIMIT));
#endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
    __TZ_set_STACKSEAL_S((uint32_t *)(&__STACK_SEAL));
#endif

    systeminit();                             /* CMSIS System Initialization */
    
    __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}


#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*----------------------------------------------------------------------------
  Hard Fault Handler
 *----------------------------------------------------------------------------*/
void HardFault_Handler(void)
{
    while (1);
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
    while (1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#pragma clang diagnostic pop
#endif

