/*
 * SoC configuration for Rafael RT582 (ARM Cortex-M3)
 *
 * This file is required by Zephyr's CMSIS integration (cmsis_core_m.h
 * does #include <soc.h> to pick up SoC-specific definitions).
 */

#ifndef _RAFAEL_RT582_SOC_H_
#define _RAFAEL_RT582_SOC_H_

#include <zephyr/sys/util.h>

#ifndef _ASMLANGUAGE
/* Pull in the Rafael SDK MCU header which defines the peripheral map,
 * IRQ table, CMSIS device-specific settings (__NVIC_PRIO_BITS, etc.)
 *
 * mcu.h pulls in every peripheral API header (flashctl.h, gpio.h, timer.h,
 * etc.).  Many of these declare functions that clash with Zephyr's driver API
 * (flash_erase, gpio_pin_get, …).  Block ALL peripheral API headers here by
 * pre-defining their include guards; only the _reg.h register-map headers
 * (struct/macro definitions only) are allowed through.
 * Each driver that legitimately needs a peripheral API includes it directly
 * in its own .c file, where Zephyr's conflicting headers are not present. */
#define FLASHCTL_H
#define GPIO_H
#define I2C_MASTER_H
#define SYSCTRL_H
#define TIMER_H
#define WDT_H
#define DMA_H
#define RTC_H
#define COMPARATOR_H
#define I2S_H
#define LPM_H
#define PWM_H
#define SADC_H
#define SWI_H
#define DPD_H
#define QSPI_MASTER_H
/* mcu.h unconditionally redefines ASSERT (function-like macro) even when a
 * caller (e.g. MCUboot's bootutil_public.h) already defined it as an
 * object-like macro.  Undefine any prior definition so the compiler does not
 * emit a -Wmacro-redefined warning. */
#ifdef ASSERT
#undef ASSERT
#endif
#include "mcu.h"
#undef FLASHCTL_H
#undef GPIO_H
#undef I2C_MASTER_H
#undef SYSCTRL_H
#undef TIMER_H
#undef WDT_H
#undef DMA_H
#undef RTC_H
#undef COMPARATOR_H
#undef I2S_H
#undef LPM_H
#undef PWM_H
#undef SADC_H
#undef SWI_H
#undef DPD_H
#undef QSPI_MASTER_H
/* Prevent mcu.h's ASSERT() function-like macro from leaking into callers that
 * have their own definition (e.g. MCUboot defines ASSERT as assert). */
#undef ASSERT
#endif /* !_ASMLANGUAGE */

#endif /* _RAFAEL_RT582_SOC_H_ */
