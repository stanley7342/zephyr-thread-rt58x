/*
 * RT583 Matter Lighting App — CHIP project configuration overrides.
 *
 * Values here override defaults in src/include/CHIPProjectConfig.h.
 * Only add entries that differ from the defaults.
 */

#pragma once

/* Suppress verbose progress logs to keep UART output readable */
#define CHIP_CONFIG_LOG_MODULE_Zcl_PROGRESS                  0
#define CHIP_CONFIG_LOG_MODULE_InteractionModel_PROGRESS      0
#define CHIP_CONFIG_LOG_MODULE_InteractionModel_DETAIL        0
#define CHIP_CONFIG_LOG_MODULE_DataManagement_PROGRESS        0
#define CHIP_CONFIG_LOG_MODULE_FabricProvisioning_PROGRESS    0
#define CHIP_CONFIG_LOG_MODULE_SecureChannel_PROGRESS         0

/* Reduce memory footprint for Cortex-M3 (144 KB RAM) */
#define CHIP_CONFIG_MAX_FABRICS                     2
#define CHIP_CONFIG_MAX_ACTIVE_CASE_CLIENTS         1
#define CHIP_CONFIG_MAX_ACTIVE_DEVICES              2

/* Dynamic packet buffer allocation (saves 24 KB BSS at the cost of heap).
 * With POOL_SIZE=0, buffers are malloc'd on demand instead of being
 * statically allocated. The 20 KB heap in prj.conf covers peak usage
 * during BLE commissioning (~5 concurrent buffers × 1.6 KB). */
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE   0
