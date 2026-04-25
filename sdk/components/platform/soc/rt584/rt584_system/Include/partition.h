/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


#ifndef _PARTITION_RT584_CM33_H_
#define _PARTITION_RT584_CM33_H_

#include "mcu.h"

/*
// Flash Size Information Address
//
//
//
*/
#define   FLASH_SIZE_INFO 0x50009038
///*
//// <e>Setup behaviour of Sleep and Exception Handling
//*/
#define SCB_CSR_AIRCR_INIT          1

/*
//   <o> Deep Sleep can be enabled by
//     <0=>Secure and Non-Secure state
//     <1=>Secure state only
//   <i> Value for SCB->CSR register bit DEEPSLEEPS
*/
#define SCB_CSR_DEEPSLEEPS_VAL      0

/*
//   <o>System reset request accessible from
//     <0=> Secure and Non-Secure state
//     <1=> Secure state only
//   <i> Value for SCB->AIRCR register bit SYSRESETREQS
*/
#define SCB_AIRCR_SYSRESETREQS_VAL  0

/*
//   <o>Priority of Non-Secure exceptions is
//     <0=> Not altered
//     <1=> Lowered to 0x80-0xFF
//   <i> Value for SCB->AIRCR register bit PRIS
*/
#define SCB_AIRCR_PRIS_VAL          1

/*
//   <o>BusFault, HardFault, and NMI target
//     <0=> Secure state
//     <1=> Non-Secure state
//   <i> Value for SCB->AIRCR register bit BFHFNMINS
*/
#define SCB_AIRCR_BFHFNMINS_VAL     0

///*
//// </e>
//*/

/*
// <e>Setup behaviour of Floating Point Unit
*/
#define TZ_FPU_NS_USAGE             1

/*
// <o>Floating Point Unit usage
//     <0=> Secure state only
//     <3=> Secure and Non-Secure state
//   <i> Value for SCB->NSACR register bits CP10, CP11
*/
#define SCB_NSACR_CP10_11_VAL       3

/*
// <o>Treat floating-point registers as Secure
//     <0=> Disabled
//     <1=> Enabled
//   <i> Value for FPU->FPCCR register bit TS
*/
#define FPU_FPCCR_TS_VAL            0

/*
// <o>Clear on return (CLRONRET) accessibility
//     <0=> Secure and Non-Secure state
//     <1=> Secure state only
//   <i> Value for FPU->FPCCR register bit CLRONRETS
*/
#define FPU_FPCCR_CLRONRETS_VAL     0

/*
// <o>Clear floating-point caller saved registers on exception return
//     <0=> Disabled
//     <1=> Enabled
//   <i> Value for FPU->FPCCR register bit CLRONRET
*/
#define FPU_FPCCR_CLRONRET_VAL      1

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
/**
  \brief   Get IADU Target State
  \details Read the peripheral attribute field in the IADU and returns the peripheral attribute bit for the device specific attribute.
  \param [in]      IADU_Type  Device specific number.
  \return             0  if secure peripheral attribute is assigned to Secure
                      1  if secure peripheral attribute is assigned to Non Secure
  \note
 */
__STATIC_INLINE uint32_t SEC_GetIADUState(SEC_IADU_Type SecIADUn)
{
    if ((int32_t)(SecIADUn)<=65)
    {
        return ((uint32_t)(((SEC_CTRL->sec_peri_attr[(((uint32_t)SecIADUn) >> 5UL)] & (1UL << (((uint32_t)SecIADUn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));
    }
    else
    {
        return (0U);
    }
}


/**
  \brief   Set IADU Target State
  \details Sets the peripheral attribute field in the IADU and returns the peripheral attribute bit for the device specific attribute.
  \param [in]      IADU_Type  Device specific number.
  \return             0  if secure peripheral attribute is assigned to Secure
                      1  if secure peripheral attribute is assigned to Non Secure
  \note
 */
__STATIC_INLINE uint32_t SEC_SetIADUState(SEC_IADU_Type SecIADUn)
{
    if ((int32_t)(SecIADUn) <=65)
    {
        SEC_CTRL->sec_peri_attr[(((uint32_t)SecIADUn) >> 5UL)] |=  ((uint32_t)(1UL << (((uint32_t)SecIADUn) & 0x1FUL)));
        return ((uint32_t)(((SEC_CTRL->sec_peri_attr[(((uint32_t)SecIADUn) >> 5UL)] & (1UL << (((uint32_t)SecIADUn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));
    }
    else
    {
        return (0U);
    }
}


/**
  \brief   Clear IADU Target State
  \details Clears peripheral attribute field in the IADU and returns the peripheral attribute bit for the device specific attribute.
  \param [in]      IADU_Type  Device specific number.
  \return             0  if secure peripheral attribute is assigned to Secure
                      1  if secure peripheral attribute is assigned to Non Secure
  \note
 */
__STATIC_INLINE uint32_t SEC_ClearIADUState(SEC_IADU_Type SecIADUn)
{

    if ((int32_t)(SecIADUn) <=65)
    {
        SEC_CTRL->sec_peri_attr[(((uint32_t)SecIADUn) >> 5UL)] &= ~((uint32_t)(1UL << (((uint32_t)SecIADUn) & 0x1FUL)));
        return ((uint32_t)(((SEC_CTRL->sec_peri_attr[(((uint32_t)SecIADUn) >> 5UL)] & (1UL << (((uint32_t)SecIADUn) & 0x1FUL))) != 0UL) ? 1UL : 0UL));
    }
    else
    {
        return (0U);
    }
}

#endif /* defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U) */


__STATIC_INLINE void TZ_SAU_Setup (void)
{

    uint32_t  flash_model_info, flash_size,flash_size_id;

#if defined (SCB_CSR_AIRCR_INIT) && (SCB_CSR_AIRCR_INIT == 1U)
    SCB->SCR   = (SCB->SCR   & ~(SCB_SCR_SLEEPDEEPS_Msk    )) |
                 ((SCB_CSR_DEEPSLEEPS_VAL     << SCB_SCR_SLEEPDEEPS_Pos)     & SCB_SCR_SLEEPDEEPS_Msk);

    SCB->AIRCR = (SCB->AIRCR & ~(SCB_AIRCR_VECTKEY_Msk   | SCB_AIRCR_SYSRESETREQS_Msk |
                                 SCB_AIRCR_BFHFNMINS_Msk | SCB_AIRCR_PRIS_Msk          ))                    |
                 ((0x05FAU                    << SCB_AIRCR_VECTKEY_Pos)      & SCB_AIRCR_VECTKEY_Msk)      |
                 ((SCB_AIRCR_SYSRESETREQS_VAL << SCB_AIRCR_SYSRESETREQS_Pos) & SCB_AIRCR_SYSRESETREQS_Msk) |
                 ((SCB_AIRCR_PRIS_VAL         << SCB_AIRCR_PRIS_Pos)         & SCB_AIRCR_PRIS_Msk)         |
                 ((SCB_AIRCR_BFHFNMINS_VAL    << SCB_AIRCR_BFHFNMINS_Pos)    & SCB_AIRCR_BFHFNMINS_Msk);
#endif /* defined (SCB_CSR_AIRCR_INIT) && (SCB_CSR_AIRCR_INIT == 1U) */

#if defined (__FPU_USED) && (__FPU_USED == 1U) && \
      defined (TZ_FPU_NS_USAGE) && (TZ_FPU_NS_USAGE == 1U)

    SCB->NSACR = (SCB->NSACR & ~(SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk)) |
                 ((SCB_NSACR_CP10_11_VAL << SCB_NSACR_CP10_Pos) & (SCB_NSACR_CP10_Msk | SCB_NSACR_CP11_Msk));

    FPU->FPCCR = (FPU->FPCCR & ~(FPU_FPCCR_TS_Msk | FPU_FPCCR_CLRONRETS_Msk | FPU_FPCCR_CLRONRET_Msk)) |
                 ((FPU_FPCCR_TS_VAL        << FPU_FPCCR_TS_Pos       ) & FPU_FPCCR_TS_Msk       ) |
                 ((FPU_FPCCR_CLRONRETS_VAL << FPU_FPCCR_CLRONRETS_Pos) & FPU_FPCCR_CLRONRETS_Msk) |
                 ((FPU_FPCCR_CLRONRET_VAL  << FPU_FPCCR_CLRONRET_Pos ) & FPU_FPCCR_CLRONRET_Msk );
#endif

    TZ_SAU_Disable();

    SEC_CTRL->sec_peri_attr[0] = 0x0;
    SEC_CTRL->sec_peri_attr[1] = 0x0;
    SEC_CTRL->sec_peri_attr[2] = 0x0;

    flash_model_info =  inp32(FLASH_SIZE_INFO);
    flash_size_id = ((flash_model_info >> 16) & 0xFF);
    flash_size = (1 << ((flash_model_info >> 16) & 0xFF));
    flash_size = (flash_size >> 5);     /*32 bytes uints*/
    
    /*set all flash in secure mode*/
    if(flash_size_id==0x14)	//1MB
    {
        SEC_CTRL->sec_flash_sec_size = 0x8000;
        SEC_CTRL->sec_flash_nsc_start = 0x8000;
        SEC_CTRL->sec_flash_nsc_stop = 0x8000;
        SEC_CTRL->sec_flash_ns_stop = flash_size;
    }
    else if(flash_size_id==0x15)//2MB
    {
        SEC_CTRL->sec_flash_sec_size = 0x10000;
        SEC_CTRL->sec_flash_nsc_start = 0x10000;
        SEC_CTRL->sec_flash_nsc_stop = 0x10000;
        SEC_CTRL->sec_flash_ns_stop = flash_size;
    }   
    else //4MB
    {

        SEC_CTRL->sec_flash_sec_size = 0x20000;
        SEC_CTRL->sec_flash_nsc_start = 0x20000;
        SEC_CTRL->sec_flash_nsc_stop = 0x20000;        
        SEC_CTRL->sec_flash_ns_stop = flash_size;
    }

    /*
     *  Here assume 64KB secure... NSC in 62KB. 0x400 bytes.
     *  Sec     address                           Non Sec address
     *  RAM0    0x30000000  0x30007FFF  32KB      RAM0  0x20000000  0x20007FFF  32KB
     *  RAM1    0x30008000  0x3000FFFF  32KB      RAM1  0x20008000  0x2000FFFF  32KB
     *  RAM2    0x30010000  0x30017FFF  32KB      RAM2  0x20010000  0x20017FFF  32KB
     *  RAM3    0x30018000  0x3001FFFF  32KB      RAM3  0x20018000  0x2001FFFF  32KB
     *  RAM4    0x30020000  0x30027FFF  32KB      RAM4  0x20020000  0x20027FFF  32KB
     *  RAM5    0x30028000  0x3002BFFF  16KB      RAM5  0x20028000  0x2002BFFF  16KB
     *  RAM6    0x3002C000  0x3002FFFF  16KB      RAM6  0x2002C000  0x2002FFFF  16KB
     *
     *  set the sram to be secure during bootrom time
     *  For bootrom we set all SRAM (192KB as secure).
     * NO NSC in SRAM.  so SEC_RAM_NSC_START = SEC_RAM_NSC_STOP
     */
    SEC_CTRL->sec_ram_sec_size = 0x1800;
    SEC_CTRL->sec_ram_nsc_start = 0x1800;
    SEC_CTRL->sec_ram_nsc_stop = 0x1800;

    /******  rt584_CM33 Specific Sec attribuite Numbers *************************************************/
    //SEC_ClearIADUState(SEC_IADU_Type);    secure
    //SEC_SetIADUState(SEC_IADU_Type);      non-secure,
    //* 1 is non-secure, 0 is secure.
    //SEC_ClearIADUState function config interrupt to sec world
    //Attribuite 0
#if defined(CONFIG_SYSCTRL_SECURE_EN)
    SEC_ClearIADUState(SYS_CTRL_IADU_Type);
#else
    SEC_SetIADUState(SYS_CTRL_IADU_Type);
#endif

#if defined(CONFIG_GPIO_SECURE_EN)
    SEC_ClearIADUState(GPIO_IADU_Type);
#else
    SEC_SetIADUState(GPIO_IADU_Type);
#endif

#if defined(CONFIG_RTC_SECURE_EN)
    SEC_ClearIADUState(RTC_IADU_Type);
#else
    SEC_SetIADUState(RTC_IADU_Type);
#endif

#if defined(CONFIG_DPD_SECURE_EN)
    SEC_ClearIADUState(DPD_CTRL_IADU_Type);
#else
    SEC_SetIADUState(DPD_CTRL_IADU_Type);
#endif

#if defined(CONFIG_SOC_PMU_SECURE_EN)
    SEC_ClearIADUState(SOC_PMU_IADU_Type);
#else
    SEC_SetIADUState(SOC_PMU_IADU_Type);
#endif

#if defined(CONFIG_FLASHCTRL_SECURE_EN)
    SEC_ClearIADUState(FLASH_CONTROL_IADU_Type);
#else
    SEC_SetIADUState(FLASH_CONTROL_IADU_Type);
#endif

#if defined(CONFIG_TIMER0_SECURE_EN)
    SEC_ClearIADUState(TIMER0_IADU_Type);
#else
    SEC_SetIADUState(TIMER0_IADU_Type);
#endif

#if defined(CONFIG_TIMER1_SECURE_EN)
    SEC_ClearIADUState(TIMER1_IADU_Type);
#else
    SEC_SetIADUState(TIMER1_IADU_Type);
#endif

#if defined(CONFIG_TIMER2_SECURE_EN)
    SEC_ClearIADUState(TIMER2_IADU_Type);
#else
    SEC_SetIADUState(TIMER2_IADU_Type);
#endif

#if defined(CONFIG_SLOWTIMER0_SECURE_EN)
    SEC_ClearIADUState(SLOWETIMER0_IADU_Type);
#else
    SEC_SetIADUState(SLOWETIMER0_IADU_Type);
#endif

#if defined(CONFIG_SLOWTIMER1_SECURE_EN)
    SEC_ClearIADUState(SLOWETIMER1_IADU_Type);
#else
    SEC_SetIADUState(SLOWETIMER1_IADU_Type);
#endif

#if defined(CONFIG_WDT_SECURE_EN)
    SEC_ClearIADUState(WDT_IADU_Type);
#else
    SEC_SetIADUState(WDT_IADU_Type);
#endif

#if defined(CONFIG_UART0_SECURE_EN)
    SEC_ClearIADUState(UART0_IADU_Type);
#else
    SEC_SetIADUState(UART0_IADU_Type);
#endif

#if defined(CONFIG_UART1_SECURE_EN)
    SEC_ClearIADUState(UART1_IADU_Type);
#else
    SEC_SetIADUState(UART1_IADU_Type);
#endif

#if defined(CONFIG_I2C_SLAVE_SECURE_EN)
    SEC_ClearIADUState(I2C_S_IADU_Type);
#else
    SEC_SetIADUState(I2C_S_IADU_Type);
#endif

#if defined(CONFIG_COMM_SUBSYSTEM_AHB_SECURE_EN)
    SEC_ClearIADUState(RT569_AHB_IADU_Type);
#else
    SEC_SetIADUState(RT569_AHB_IADU_Type);
#endif

#if defined(CONFIG_RCO32K_CAL_SECURE_EN)
    SEC_ClearIADUState(RCO32K_CAL_IADU_Type);
#else
    SEC_SetIADUState(RCO32K_CAL_IADU_Type);
#endif

#if defined(CONFIG_BOD_COMP_SECURE_EN)
    SEC_ClearIADUState(BOD_COMP_IADU_Type);
#else
    SEC_SetIADUState(BOD_COMP_IADU_Type);
#endif

#if defined(CONFIG_AUX_COMP_SECURE_EN)
    SEC_ClearIADUState(AUX_COMP_IADU_Type);
#else
    SEC_SetIADUState(AUX_COMP_IADU_Type);
#endif

#if defined(CONFIG_RCO1M_CAL_SECURE_EN)
    SEC_ClearIADUState(RCO1M_CAL_IADU_Type);
#else
    SEC_SetIADUState(RCO1M_CAL_IADU_Type);
#endif


    //Attribuite 1
#if defined(CONFIG_QSPI0_SECURE_EN)
    SEC_ClearIADUState(QSPI0_IADU_Type);
#else
    SEC_SetIADUState(QSPI0_IADU_Type);
#endif

#if defined(CONFIG_QSPI1_SECURE_EN)
    SEC_ClearIADUState(QSPI1_IADU_Type);
#else
    SEC_SetIADUState(QSPI1_IADU_Type);
#endif

#if defined(CONFIG_IRM_SECURE_EN)
    SEC_ClearIADUState(IRM_IADU_Type);
#else
    SEC_SetIADUState(IRM_IADU_Type);
#endif

#if defined(CONFIG_UART2_SECURE_EN)
    SEC_ClearIADUState(UART2_IADU_Type);
#else
    SEC_SetIADUState(UART2_IADU_Type);
#endif

#if defined(CONFIG_PWM_SECURE_EN)
    SEC_ClearIADUState(PWM_IADU_Type);
#else
    SEC_SetIADUState(PWM_IADU_Type);
#endif

#if defined(CONFIG_XDMA_SECURE_EN)
    SEC_ClearIADUState(XDMA_IADU_Type);
#else
    SEC_SetIADUState(XDMA_IADU_Type);
#endif

#if defined(CONFIG_DMA0_SECURE_EN)
    SEC_ClearIADUState(DMA0_IADU_Type);
#else
    SEC_SetIADUState(DMA0_IADU_Type);
#endif

#if defined(CONFIG_DMA1_SECURE_EN)
    SEC_ClearIADUState(DMA1_IADU_Type);
#else
    SEC_SetIADUState(DMA1_IADU_Type);
#endif

#if defined(CONFIG_I2C_MASTER0_SECURE_EN)
    SEC_ClearIADUState(I2C_M0_IADU_Type);
#else
    SEC_SetIADUState(I2C_M0_IADU_Type);
#endif

#if defined(CONFIG_I2C_MASTER1_SECURE_EN)
    SEC_ClearIADUState(I2C_M1_IADU_Type);
#else
    SEC_SetIADUState(I2C_M1_IADU_Type);
#endif

#if defined(CONFIG_I2S0_SECURE_EN)
    SEC_ClearIADUState(I2S0_M_IADU_Type);
#else
    SEC_SetIADUState(I2S0_M_IADU_Type);
#endif

#if defined(CONFIG_SADC_SECURE_EN)
    SEC_ClearIADUState(SADC_IADU_Type);
#else
    SEC_SetIADUState(SADC_IADU_Type);
#endif

#if defined(CONFIG_SWI0_SECURE_EN)
    SEC_ClearIADUState(SWI0_IADU_Type);
#else
    SEC_SetIADUState(SWI0_IADU_Type);
#endif

#if defined(CONFIG_SWI1_SECURE_EN)
    SEC_ClearIADUState(SWI1_IADU_Type);
#else
    SEC_SetIADUState(SWI1_IADU_Type);
#endif


    //Attribuite 2
    SEC_ClearIADUState(CRYPTO_IADU_Type);
    SEC_ClearIADUState(PUF_OTP_IADU_Type);
    //SEC_SetIADUState function config interrupt to non sec world


    /******  rt584 cm33S pecific Interrupt Numbers *************************************************/
    //NVIC_ClearTargetState(IQRn_Type);     secure
    //NVIC_SetTargetState(IQRn_Type);       no secure
    //Nvic ClearTargeStat function config interrupt to sec world
    NVIC_ClearTargetState(Sec_Ctrl_IQRn);
    //Attribuite 0
#if defined(CONFIG_GPIO_SECURE_EN)
    NVIC_ClearTargetState(Gpio_IRQn);
#else
    NVIC_SetTargetState(Gpio_IRQn);
#endif

#if defined(CONFIG_RTC_SECURE_EN)
    NVIC_ClearTargetState(Rtc_IRQn);
#else
    NVIC_SetTargetState(Rtc_IRQn);
#endif

#if defined(CONFIG_FLASHCTRL_SECURE_EN)
    NVIC_ClearTargetState(FlashCtl_IRQn);
#else
    NVIC_SetTargetState(FlashCtl_IRQn);
#endif

#if defined(CONFIG_TIMER0_SECURE_EN)
    NVIC_ClearTargetState(Timer0_IRQn);
#else
    NVIC_SetTargetState(Timer0_IRQn);
#endif

#if defined(CONFIG_TIMER1_SECURE_EN)
    NVIC_ClearTargetState(Timer1_IRQn);
#else
    NVIC_SetTargetState(Timer1_IRQn);
#endif

#if defined(CONFIG_TIMER2_SECURE_EN)
    NVIC_ClearTargetState(Timer2_IRQn);
#else
    NVIC_SetTargetState(Timer2_IRQn);
#endif

#if defined(CONFIG_SLOWTIMER0_SECURE_EN)
    NVIC_ClearTargetState(SlowTimer0_IRQn);
#else
    NVIC_SetTargetState(SlowTimer0_IRQn);
#endif

#if defined(CONFIG_SLOWTIMER1_SECURE_EN)
    NVIC_ClearTargetState(SlowTimer1_IRQn);
#else
    NVIC_SetTargetState(SlowTimer1_IRQn);
#endif

#if defined(CONFIG_WDT_SECURE_EN)
    NVIC_ClearTargetState(Wdt_IRQn);
#else
    NVIC_SetTargetState(Wdt_IRQn);
#endif

#if defined(CONFIG_UART0_SECURE_EN)
    NVIC_ClearTargetState(Uart0_IRQn);
#else
    NVIC_SetTargetState(Uart0_IRQn);
#endif

#if defined(CONFIG_UART1_SECURE_EN)
    NVIC_ClearTargetState(Uart1_IRQn);
#else
    NVIC_SetTargetState(Uart1_IRQn);
#endif

#if defined(CONFIG_I2C_SLAVE_SECURE_EN)
    NVIC_ClearTargetState(I2C_Slave_IRQn);
#else
    NVIC_SetTargetState(I2C_Slave_IRQn);
#endif

#if defined(CONFIG_COMM_SUBSYSTEM_AHB_SECURE_EN)
    NVIC_ClearTargetState(CommSubsystem_IRQn);
#else
    NVIC_SetTargetState(CommSubsystem_IRQn);
#endif

#if defined(CONFIG_BOD_COMP_SECURE_EN)
    NVIC_ClearTargetState(Bod_Comp_IRQn);
#else
    NVIC_SetTargetState(Bod_Comp_IRQn);
#endif

#if defined(CONFIG_AUX_COMP_SECURE_EN)
    NVIC_ClearTargetState(Aux_Comp_IRQn);
#else
    NVIC_SetTargetState(Aux_Comp_IRQn);
#endif

    //Attribuite 1
#if defined(CONFIG_QSPI0_SECURE_EN)
    NVIC_ClearTargetState(Qspi0_IRQn);
#else
    NVIC_SetTargetState(Qspi0_IRQn);
#endif

#if defined(CONFIG_QSPI1_SECURE_EN)
    NVIC_ClearTargetState(Qspi1_IRQn);
#else
    NVIC_SetTargetState(Qspi1_IRQn);
#endif

#if defined(CONFIG_IRM_SECURE_EN)
    NVIC_ClearTargetState(Irm_IRQn);
#else
    NVIC_SetTargetState(Irm_IRQn);
#endif

#if defined(CONFIG_UART2_SECURE_EN)
    NVIC_ClearTargetState(Uart2_IRQn);
#else
    NVIC_SetTargetState(Uart2_IRQn);
#endif

#if defined(CONFIG_PWM_SECURE_EN)
    NVIC_ClearTargetState(Pwm0_IRQn);
    NVIC_ClearTargetState(Pwm1_IRQn);
    NVIC_ClearTargetState(Pwm2_IRQn);
    NVIC_ClearTargetState(Pwm3_IRQn);
    NVIC_ClearTargetState(Pwm4_IRQn);
#else
    NVIC_SetTargetState(Pwm0_IRQn);
    NVIC_SetTargetState(Pwm1_IRQn);
    NVIC_SetTargetState(Pwm2_IRQn);
    NVIC_SetTargetState(Pwm3_IRQn);
    NVIC_SetTargetState(Pwm4_IRQn);
#endif

#if defined(CONFIG_DMA0_SECURE_EN)
    NVIC_ClearTargetState(Dma0_IRQn);
#else
    NVIC_SetTargetState(Dma0_IRQn);
#endif

#if defined(CONFIG_DMA1_SECURE_EN)
    NVIC_ClearTargetState(Dma1_IRQn);
#else
    NVIC_SetTargetState(Dma1_IRQn);
#endif

#if defined(CONFIG_I2C_MASTER0_SECURE_EN)
    NVIC_ClearTargetState(I2C_Master0_IRQn);
#else
    NVIC_SetTargetState(I2C_Master0_IRQn);
#endif

#if defined(CONFIG_I2C_MASTER1_SECURE_EN)
    NVIC_ClearTargetState(I2C_Master1_IRQn);
#else
    NVIC_SetTargetState(I2C_Master1_IRQn);
#endif

#if defined(CONFIG_I2S0_SECURE_EN)
    NVIC_ClearTargetState(I2s0_IRQn);
#else
    NVIC_SetTargetState(I2s0_IRQn);
#endif

#if defined(CONFIG_SADC_SECURE_EN)
    NVIC_ClearTargetState(Sadc_IRQn);
#else
    NVIC_SetTargetState(Sadc_IRQn);
#endif

#if defined(CONFIG_SWI0_SECURE_EN)
    NVIC_ClearTargetState(SWI0_IRQn);
#else
    NVIC_SetTargetState(SWI0_IRQn);
#endif

#if defined(CONFIG_SWI1_SECURE_EN)
    NVIC_ClearTargetState(SWI1_IRQn);
#else
    NVIC_SetTargetState(SWI1_IRQn);
#endif


    //Attribuite 2
    NVIC_ClearTargetState(Crypto_IRQn);
    NVIC_ClearTargetState(OTP_IRQn);

    //Nvic SetTargeStat function config interrupt to non sec wrold


    /*Enable IDAU*/
    SEC_CTRL->sec_idau_ctrl =  1;

    /*
     * 2022/12/21: set SAU to be all non-secure
     *
     */
    SAU->CTRL |= (SAU_CTRL_ALLNS_Msk);
    /*TODO: depends on system setting.*/

}

#endif