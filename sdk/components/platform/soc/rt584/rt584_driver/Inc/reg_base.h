/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           reg_base.h
 * \brief          register base address file
 */
/*
 * This file is part of library_name.
 * Author: 
 */

#ifndef REG_BASE_H
#define REG_BASE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * \brief          Peripheral memory map
 */
#define SRAM_BASE                       (0x20000000UL)  /*!< (SRAM      ) Base Address */
#define PERIPH_BASE                     (0x40000000UL)  /*!< (Peripheral) Base Address */
#define PERIPH_SECURE_OFFSET            (0x10000000UL)  /*!< (SECURE Peripheral) Offset */

#define SYSCTRL_BASE                    (PERIPH_BASE)
#define GPIO_BASE                       (PERIPH_BASE+0x1000UL)
#define SEC_CTRL_BASE                   (PERIPH_BASE+PERIPH_SECURE_OFFSET+0x3000UL)
#define RTC_BASE                        (PERIPH_BASE+0x4000UL)
#define DPD_BASE                        (PERIPH_BASE+0x5000UL)
#define SOC_PMU_BASE                    (PERIPH_BASE+0x6000UL)
#define FLASHCTRL_BASE                  (PERIPH_BASE+0x9000UL)
#define TIMER0_BASE                     (PERIPH_BASE+0xA000UL)
#define TIMER1_BASE                     (PERIPH_BASE+0xB000UL)
#define TIMER2_BASE                     (PERIPH_BASE+0xC000UL)
#define SLOWTIMER0_BASE                 (PERIPH_BASE+0xD000UL)
#define SLOWTIMER1_BASE                 (PERIPH_BASE+0xE000UL)
#define WDT_BASE                        (PERIPH_BASE+0x10000UL)
#define UART0_BASE                      (PERIPH_BASE+0x12000UL)
#define UART1_BASE                      (PERIPH_BASE+0x13000UL)
#define PWM0_BASE                       (PERIPH_BASE+0x26000UL)
#define PWM1_BASE                       (PERIPH_BASE+0x26100UL)
#define PWM2_BASE                       (PERIPH_BASE+0x26200UL)
#define PWM3_BASE                       (PERIPH_BASE+0x26300UL)
#define PWM4_BASE                       (PERIPH_BASE+0x26400UL)
#define I2C_SLAVE_BASE                  (PERIPH_BASE+0x18000UL)
#define COMM_SUBSYSTEM_AHB_BASE         (PERIPH_BASE+0x1A000UL)
#define RCO32K_BASE                     (PERIPH_BASE+0x1C000UL)
#define BOD_COMP_BASE                   (PERIPH_BASE+0x1D000UL)
#define AUX_COMP_BASE                   (PERIPH_BASE+0x1E000UL)
#define RCO1M_BASE                      (PERIPH_BASE+0x1F000UL)
#define QSPI0_BASE                      (PERIPH_BASE+0x20000UL)
#define QSPI1_BASE                      (PERIPH_BASE+0x21000UL)
#define TRNG_BASE                       (PERIPH_BASE+0x23000UL)
#define IRM_BASE                        (PERIPH_BASE+0x24000UL)
#define UART2_BASE                      (PERIPH_BASE+0x25000UL)
#define XDMA_BASE                       (PERIPH_BASE+0x28000UL)
#define DMA0_BASE                       (PERIPH_BASE+0x29000UL)
#define DMA1_BASE                       (PERIPH_BASE+0x2A000UL)
#define I2C_MASTER0_BASE                (PERIPH_BASE+0x2B000UL)
#define I2C_MASTER1_BASE                (PERIPH_BASE+0x2C000UL)
#define I2S0_BASE                       (PERIPH_BASE+0x2D000UL)
#define SADC_BASE                       (PERIPH_BASE+0x2F000UL)

#define SWI0_BASE                    (PERIPH_BASE+0x30000UL)
#define SWI1_BASE                    (PERIPH_BASE+0x31000UL)

#define CRYPTO_BASE                     (0x70000000UL)


#ifdef __cplusplus
}
#endif

#endif /* End of REG_BASE_H */
