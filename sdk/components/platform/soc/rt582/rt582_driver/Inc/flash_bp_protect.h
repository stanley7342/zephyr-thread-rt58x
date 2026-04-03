/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

#ifndef FLASH_BP_PROTECT_H
#define FLASH_BP_PROTECT_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Status / Error codes
 * ==========================================================================*/
#define FLASH_BP_STATUS_SUCCESS          0u
#define FLASH_BP_STATUS_EBUSY            1u
#define FLASH_BP_STATUS_INVALID_PARAM    2u
#define FLASH_BP_STATUS_TIMEOUT          3u
#define FLASH_BP_STATUS_UNSUPPORTED      4u

/* ============================================================================
 * Status Register bit definitions
 * ==========================================================================*/
/* Status Register 1 */
#define FLASH_BP_WIP_BIT         (1u << 0)   /* Write In Progress */
#define FLASH_BP_WEL_BIT         (1u << 1)   /* Write Enable Latch */
#define FLASH_BP_BP_MASK         (0x7Cu)     /* BP4..BP0 (bits 2..6) */

/* Status Register 2 */
#define FLASH_BP_CMP_BIT         (1u << 6)   /* Complement Protect bit */

/* ============================================================================
 * Raw BP values (BP4..BP0)
 * ==========================================================================*/
#define FLASH_BP_RAW_NONE        0x00u
#define FLASH_BP_RAW_MAX         0x1Fu

/*
 * Fixed-size protection modes
 * - Raw BP values defined by flash datasheet
 * - Applicable to all flash capacities
 */
#define FLASH_BP_RAW_TOP_4KB     0x11u
#define FLASH_BP_RAW_TOP_8KB     0x12u
#define FLASH_BP_RAW_TOP_16KB    0x13u
#define FLASH_BP_RAW_TOP_32KB    0x14u

#define FLASH_BP_RAW_BOTTOM_4KB  0x19u
#define FLASH_BP_RAW_BOTTOM_8KB  0x1Au
#define FLASH_BP_RAW_BOTTOM_16KB 0x1Bu
#define FLASH_BP_RAW_BOTTOM_32KB 0x1Cu

/* ============================================================================
 * Semantic protection modes (platform independent)
 *
 * Notes:
 * - These are NOT raw BP values
 * - They represent logical protection intents
 * - Actual raw BP values are resolved at runtime based on capacity_id
 *   using lookup tables in the .c file
 * ==========================================================================*/
typedef enum {
    FLASH_BP_MODE_NONE = 0,

    /* Upper / Lower fractional protection */
    FLASH_BP_MODE_UPPER_1_64,
    FLASH_BP_MODE_UPPER_1_32,
    FLASH_BP_MODE_UPPER_1_16,
    FLASH_BP_MODE_UPPER_1_8,
    FLASH_BP_MODE_UPPER_1_4,
    FLASH_BP_MODE_UPPER_1_2,

    FLASH_BP_MODE_LOWER_1_64,
    FLASH_BP_MODE_LOWER_1_32,
    FLASH_BP_MODE_LOWER_1_16,
    FLASH_BP_MODE_LOWER_1_8,
    FLASH_BP_MODE_LOWER_1_4,
    FLASH_BP_MODE_LOWER_1_2,

    /* Fixed-size protection */
    FLASH_BP_MODE_TOP_4KB,
    FLASH_BP_MODE_TOP_8KB,
    FLASH_BP_MODE_TOP_16KB,
    FLASH_BP_MODE_TOP_32KB,

    FLASH_BP_MODE_BOTTOM_4KB,
    FLASH_BP_MODE_BOTTOM_8KB,
    FLASH_BP_MODE_BOTTOM_16KB,
    FLASH_BP_MODE_BOTTOM_32KB,

    /* Full-chip protection (resolved by capacity table) */
    FLASH_BP_MODE_ALL,

} flash_bp_mode_t;

/* ============================================================================
 * Device information / protection region information
 * ==========================================================================*/
typedef struct {
    uint8_t  manufacturer_id;
    uint8_t  memory_type;
    uint8_t  capacity_id;
    uint8_t  blocks;          /* Number of 64KB blocks */
    uint32_t size_bytes;
    uint32_t size_kb;
    bool     detected;
} flash_bp_device_t;

typedef struct {
    uint32_t start_addr;      /* Protected region start address (inclusive) */
    uint32_t end_addr;        /* Protected region end address (inclusive) */
    uint32_t size;            /* Size in bytes */
    const char *description; /* Human-readable description */
} flash_bp_protect_info_t;

/* ============================================================================
 * Public API
 * ==========================================================================*/
uint32_t flash_bp_protect_init(void);
uint32_t flash_bp_auto_detect(void);
const flash_bp_device_t* flash_bp_get_device_info(void);

/* --------------------------------------------------------------------------
 * Raw BP APIs (kept for backward compatibility)
 * --------------------------------------------------------------------------*/
uint32_t flash_bp_set_block_protect(uint8_t bp_value);               /* CMP = 0 */
uint32_t flash_bp_set_block_protect_inverted(uint8_t bp_value);      /* CMP = 1 */
uint32_t flash_bp_set_block_protect_raw(uint8_t bp_value);           /* alias */
uint32_t flash_bp_set_block_protect_raw_inverted(uint8_t bp_value);
uint32_t flash_bp_remove_all_protection(void);
uint32_t flash_bp_set_cmp_mode(bool enable);

uint32_t flash_bp_get_protect_status(uint8_t *bp_value, bool *cmp_enabled);
uint32_t flash_bp_calculate_protect_region(uint8_t bp_value,
                                           bool cmp_enabled,
                                           flash_bp_protect_info_t *info);
uint32_t flash_bp_is_address_protected(uint32_t address, bool *is_protected);
void     flash_bp_print_protect_status(void);

/* ============================================================================
 * New helper APIs (recommended for SDK users)
 *
 * These APIs prevent misuse of raw BP values by providing
 * semantic, capacity-aware protection configuration.
 * ==========================================================================*/

/**
 * \brief Convert semantic protection mode to raw BP value.
 *
 * The conversion is performed using runtime flash capacity_id
 * and internal lookup tables.
 *
 * \param mode        Semantic protection mode
 * \param out_bp_raw  Output raw BP value (0x00 ~ 0x1F)
 *
 * \return FLASH_BP_STATUS_SUCCESS or error code
 */
uint32_t flash_bp_mode_to_raw(flash_bp_mode_t mode, uint8_t *out_bp_raw);

/**
 * \brief Configure block protection using semantic mode.
 *
 * \param mode        Semantic protection mode
 * \param cmp_enable  true = CMP inverted mode, false = normal mode
 *
 * \return FLASH_BP_STATUS_SUCCESS or error code
 */
uint32_t flash_bp_set_block_protect_mode(flash_bp_mode_t mode, bool cmp_enable);
/**
 * \brief verify block protection register.
 *
 * \param bp           block protection mode
 * \param cmp_enabled  true = CMP inverted mode, false = normal mode
 *
 * \return STATUS_SUCCESS or error code
 */
uint32_t flash_bp_verify_status_registers(uint8_t bp, bool cmp_enabled);
#endif /* FLASH_BP_PROTECT_H */
