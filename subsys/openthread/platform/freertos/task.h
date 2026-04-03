/*
 * task.h — Minimal stub for Zephyr builds
 *
 * Provides declarations for the FreeRTOS task functions referenced by
 * the Rafael SDK's log.h logging macros.
 *
 * The actual implementations are in platform/freertos_shim.c.
 */
#ifndef FREERTOS_TASK_STUB_H
#define FREERTOS_TASK_STUB_H

#include "FreeRTOS.h"

/* Declared in freertos_shim.c */
TickType_t xTaskGetTickCount(void);
TickType_t xTaskGetTickCountFromISR(void);

#define taskSCHEDULER_RUNNING  2

#endif /* FREERTOS_TASK_STUB_H */
