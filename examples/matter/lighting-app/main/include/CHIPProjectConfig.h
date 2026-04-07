/*
 * RT582 Matter Lighting App — CHIP project configuration overrides.
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
#define CHIP_CONFIG_MAX_FABRICS                     4
#define CHIP_CONFIG_MAX_ACTIVE_CASE_CLIENTS         2
#define CHIP_CONFIG_MAX_ACTIVE_DEVICES              4
