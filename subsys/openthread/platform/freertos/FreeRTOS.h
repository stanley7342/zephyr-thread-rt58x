/*
 * FreeRTOS.h — Minimal stub for Zephyr builds
 *
 * The Rafael SDK's log.h includes <FreeRTOS.h> and <task.h> when
 * CONFIG_FREERTOS is defined, then uses xPortIsInsideInterrupt(),
 * xTaskGetTickCount(), and xTaskGetTickCountFromISR() in logging macros.
 *
 * This stub provides the type definitions and forward declarations so that
 * SDK source files (rf_mcu.c, rf_mcu_ahb.c, etc.) compile cleanly under
 * Zephyr without the real FreeRTOS headers.
 *
 * The actual implementations are in platform/freertos_shim.c.
 */
#ifndef FREERTOS_STUB_H
#define FREERTOS_STUB_H

#include <stdint.h>

typedef uint32_t TickType_t;
typedef int32_t  BaseType_t;
typedef uint32_t UBaseType_t;

/* Declared in freertos_shim.c */
BaseType_t xPortIsInsideInterrupt(void);

#endif /* FREERTOS_STUB_H */
