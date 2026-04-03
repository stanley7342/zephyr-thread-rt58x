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
 * IRQ table, CMSIS device-specific settings (__NVIC_PRIO_BITS, etc.)  */
#include "mcu.h"
#endif /* !_ASMLANGUAGE */

#endif /* _RAFAEL_RT582_SOC_H_ */
