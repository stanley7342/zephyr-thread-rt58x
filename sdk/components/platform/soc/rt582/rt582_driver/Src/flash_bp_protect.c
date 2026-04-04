/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


#include "flash_bp_protect.h"
#include "flashctl.h"
#include "status.h"
#include <stdio.h>
#include <string.h>

/* ================= logging ================= */
#if 0

#ifndef FLASH_BP_PROTECT_LOGI
#define FLASH_BP_PROTECT_LOGI(...)  do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
#endif

#ifndef FLASH_BP_PROTECT_LOGE
#define FLASH_BP_PROTECT_LOGE(...)  do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
#endif

#else

#ifndef FLASH_BP_PROTECT_LOGI
#define FLASH_BP_PROTECT_LOGI(...)  do { } while (0)
#endif

#ifndef FLASH_BP_PROTECT_LOGE
#define FLASH_BP_PROTECT_LOGE(...)  do { } while (0)
#endif

#endif

/* ================= sizes ================= */
#define FLASH_SIZE_1MB   (0x00100000u)
#define FLASH_SIZE_2MB   (0x00200000u)
#define FLASH_SIZE_4MB   (0x00400000u)

#define FLASH_END_1MB    (FLASH_SIZE_1MB - 1u)
#define FLASH_END_2MB    (FLASH_SIZE_2MB - 1u)
#define FLASH_END_4MB    (FLASH_SIZE_4MB - 1u)


/* ================= global device ================= */
static flash_bp_device_t g_flash_device = {
    .detected = false
};

/* ================= internal table types ================= */
typedef struct {
    uint8_t  mask;     /* bit=1: care */
    uint8_t  value;    /* must match after mask */
    uint32_t start;    /* inclusive */
    uint32_t end;      /* inclusive */
    const char *desc;
} flash_bp_def_t;

static inline uint32_t _range_size(uint32_t s, uint32_t e)
{
    return (e >= s) ? (e - s + 1u) : 0u;
}

static inline uint32_t flash_addr_base(void)
{
    #if defined(CONFIG_RT581) || defined(CONFIG_RT582) || defined(CONFIG_RT583)
    return 0x00000000u;
    #else
    return 0x10000000u;  // RT584/RF1301
    #endif
}

static bool _bp_lookup(const flash_bp_def_t *tbl, size_t n, uint8_t bp, flash_bp_protect_info_t *info)
{
     FLASH_BP_PROTECT_LOGI("[bp_lookup] bp=0x%02X n=%u", bp, (unsigned)n);

    for (size_t i = 0; i < n; i++) {
        if ( (bp & tbl[i].mask) == (tbl[i].value & tbl[i].mask) ) {
            if (tbl[i].start == 0u && tbl[i].end == 0u && tbl[i].mask != 0u) {
                /* NONE entry encoded as start=end=0, size=0 */

 FLASH_BP_PROTECT_LOGI("[bp_lookup] HIT i=%u mask=0x%02X val=0x%02X desc=%s",
                                   (unsigned)i, tbl[i].mask, tbl[i].value, tbl[i].desc);
                info->start_addr  = 0;
                info->end_addr    = 0;
                info->size        = 0;
                
            } else {

 FLASH_BP_PROTECT_LOGI("[bp_lookup] HIT i=%u mask=0x%02X val=0x%02X desc=%s",
                                   (unsigned)i, m, v, tbl[i].desc);
                                  
                info->start_addr  = tbl[i].start;
                info->end_addr    = tbl[i].end;
                info->size        = _range_size(tbl[i].start, tbl[i].end);
            }
            info->description = tbl[i].desc;
            return true;
        }
    }
 // FLASH_BP_PROTECT_LOGI("[bp_lookup] MISS bp=0x%02X", bp);
    return false;
}

/* CMP=1: invert contiguous protected region (matches datasheet BP behavior) */
static void _invert_region(uint32_t flash_end, flash_bp_protect_info_t *info)
{
    const uint32_t flash_size = flash_end + 1u;

    /* NONE -> ALL */
    if (info->size == 0u) {
        info->start_addr  = 0u;
        info->end_addr    = flash_end;
        info->size        = flash_size;
        info->description = "ALL Protected (CMP=1 inverted)";
        return;
    }

    /* ALL -> NONE */
    if (info->start_addr == 0u && info->end_addr == flash_end && info->size == flash_size) {
        info->start_addr  = 0u;
        info->end_addr    = 0u;
        info->size        = 0u;
        info->description = "No Protection (CMP=1 inverted)";
        return;
    }

    /* Upper [X..end] -> Lower [0..X-1] */
    if (info->end_addr == flash_end && info->start_addr > 0u) {
        uint32_t new_end = info->start_addr - 1u;
        info->start_addr  = 0u;
        info->end_addr    = new_end;
        info->size        = new_end + 1u;
        info->description = "Lower Protected (CMP=1 inverted)";
        return;
    }

    /* Lower [0..Y] -> Upper [Y+1..end] */
    if (info->start_addr == 0u && info->end_addr < flash_end) {
        uint32_t new_start = info->end_addr + 1u;
        info->start_addr  = new_start;
        info->end_addr    = flash_end;
        info->size        = flash_end - new_start + 1u;
        info->description = "Upper Protected (CMP=1 inverted)";
        return;
    }

    info->description = "CMP=1 inverted (unsupported shape)";
}

/* ================= CMP=0 decode tables (per capacity) =================
 * Notes:
 * - bp_value is BP4..BP0 (5-bit)
 * - We intentionally use mask patterns for "XX000"/"XX111" entries.
 * - Addresses are 0-based offsets within flash (as your previous code did).
 */

/* 4MB (0x16):  */
static const flash_bp_def_t g_tbl_4mb[] = {
    /* NONE: XX000 (keep 0,0 as special NONE encoding) */
    { .mask=0x07, .value=0x00, .start=0, .end=0, .desc="No Protection" },

    /* Upper fractions */
    { .mask=0x1F, .value=0x01, .start= 0x003F0000u, .end=FLASH_END_4MB, .desc="Upper 1/64 (64KB)"  },
    { .mask=0x1F, .value=0x02, .start=0x003E0000u, .end=FLASH_END_4MB, .desc="Upper 1/32 (128KB)" },
    { .mask=0x1F, .value=0x03, .start= 0x003C0000u, .end=FLASH_END_4MB, .desc="Upper 1/16 (256KB)" },
    { .mask=0x1F, .value=0x04, .start=0x00380000u, .end=FLASH_END_4MB, .desc="Upper 1/8 (512KB)"  },
    { .mask=0x1F, .value=0x05, .start=0x00300000u, .end=FLASH_END_4MB, .desc="Upper 1/4 (1MB)"    },
    { .mask=0x1F, .value=0x06, .start=0x00200000u, .end=FLASH_END_4MB, .desc="Upper 1/2 (2MB)"    },

    /* Lower fractions */
    { .mask=0x1F, .value=0x09, .start= 0x00000000u, .end= 0x0000FFFFu, .desc="Lower 1/64 (64KB)"  },
    { .mask=0x1F, .value=0x0A, .start= 0x00000000u, .end= 0x0001FFFFu, .desc="Lower 1/32 (128KB)" },
    { .mask=0x1F, .value=0x0B, .start= 0x00000000u, .end= 0x0003FFFFu, .desc="Lower 1/16 (256KB)" },
    { .mask=0x1F, .value=0x0C, .start= 0x00000000u, .end= 0x0007FFFFu, .desc="Lower 1/8 (512KB)"  },
    { .mask=0x1F, .value=0x0D, .start= 0x00000000u, .end= 0x000FFFFFu, .desc="Lower 1/4 (1MB)"    },
    { .mask=0x1F, .value=0x0E, .start= 0x00000000u, .end= 0x001FFFFFu, .desc="Lower 1/2 (2MB)"    },

    /* ALL: XX111 */
    { .mask=0x07, .value=0x07, .start= 0x00000000u, .end=FLASH_END_4MB, .desc="ALL Protected" },

    /* Top fixed-size */
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_4KB,  .start= 0x003FF000u, .end=FLASH_END_4MB, .desc="Top 4KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_8KB,  .start= 0x003FE000u, .end=FLASH_END_4MB, .desc="Top 8KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_16KB, .start= 0x003FC000u, .end=FLASH_END_4MB, .desc="Top 16KB" },
    { .mask=0x1E, .value=FLASH_BP_RAW_TOP_32KB, .start= 0x003F8000u, .end=FLASH_END_4MB, .desc="Top 32KB" }, /* BP0=X */

    /* Bottom fixed-size */
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_4KB,  .start= 0x00000000u, .end= 0x00000FFFu, .desc="Bottom 4KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_8KB,  .start= 0x00000000u, .end= 0x00001FFFu, .desc="Bottom 8KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_16KB, .start= 0x00000000u, .end= 0x00003FFFu, .desc="Bottom 16KB" },
    { .mask=0x1E, .value=FLASH_BP_RAW_BOTTOM_32KB, .start= 0x00000000u, .end= 0x00007FFFu, .desc="Bottom 32KB" }, /* BP0=X */
};

/* 2MB (0x15): */
static const flash_bp_def_t g_tbl_2mb[] = {
    /* NONE */
    { .mask=0x07, .value=0x00, .start=0, .end=0, .desc="No Protection" },

    /* Upper fractions */
    { .mask=0x1F, .value=0x01, .start= 0x001F0000u, .end=FLASH_END_2MB, .desc="Upper 1/32 (64KB)"  },
    { .mask=0x1F, .value=0x02, .start= 0x001E0000u, .end=FLASH_END_2MB, .desc="Upper 1/16 (128KB)" },
    { .mask=0x1F, .value=0x03, .start= 0x001C0000u, .end=FLASH_END_2MB, .desc="Upper 1/8 (256KB)"  },
    { .mask=0x1F, .value=0x04, .start= 0x00180000u, .end=FLASH_END_2MB, .desc="Upper 1/4 (512KB)"  },
    { .mask=0x1F, .value=0x05, .start= 0x00100000u, .end=FLASH_END_2MB, .desc="Upper 1/2 (1MB)"    },

    /* Lower fractions */
    { .mask=0x1F, .value=0x09, .start= 0x00000000u, .end= 0x0000FFFFu, .desc="Lower 1/32 (64KB)"  },
    { .mask=0x1F, .value=0x0A, .start= 0x00000000u, .end= 0x0001FFFFu, .desc="Lower 1/16 (128KB)" },
    { .mask=0x1F, .value=0x0B, .start= 0x00000000u, .end= 0x0003FFFFu, .desc="Lower 1/8 (256KB)"  },
    { .mask=0x1F, .value=0x0C, .start= 0x00000000u, .end= 0x0007FFFFu, .desc="Lower 1/4 (512KB)"  },
    { .mask=0x1F, .value=0x0D, .start= 0x00000000u, .end= 0x000FFFFFu, .desc="Lower 1/2 (1MB)"    },

    /* ALL: XX11X */
    { .mask=0x06, .value=0x06, .start= 0x00000000u, .end=FLASH_END_2MB, .desc="ALL Protected" },

    /* Top fixed-size */
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_4KB,  .start= 0x001FF000u, .end=FLASH_END_2MB, .desc="Top 4KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_8KB,  .start= 0x001FE000u, .end=FLASH_END_2MB, .desc="Top 8KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_16KB, .start= 0x001FC000u, .end=FLASH_END_2MB, .desc="Top 16KB" },
    { .mask=0x1E, .value=FLASH_BP_RAW_TOP_32KB, .start= 0x001F8000u, .end=FLASH_END_2MB, .desc="Top 32KB" },

    /* Bottom fixed-size */
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_4KB,  .start= 0x00000000u, .end= 0x00000FFFu, .desc="Bottom 4KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_8KB,  .start= 0x00000000u, .end= 0x00001FFFu, .desc="Bottom 8KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_16KB, .start= 0x00000000u, .end= 0x00003FFFu, .desc="Bottom 16KB" },
    { .mask=0x1E, .value=FLASH_BP_RAW_BOTTOM_32KB, .start= 0x00000000u, .end= 0x00007FFFu, .desc="Bottom 32KB" },
};
/* 1MB (0x14):*/
static const flash_bp_def_t g_tbl_1mb[] = {
    /* NONE */
    { .mask=0x07, .value=0x00, .start=0, .end=0, .desc="No Protection" },

    /* Upper fractions */
    { .mask=0x1F, .value=0x01, .start= 0x000F0000u, .end=FLASH_END_1MB, .desc="Upper 1/16 (64KB)"  },
    { .mask=0x1F, .value=0x02, .start= 0x000E0000u, .end=FLASH_END_1MB, .desc="Upper 1/8 (128KB)"  },
    { .mask=0x1F, .value=0x03, .start= 0x000C0000u, .end=FLASH_END_1MB, .desc="Upper 1/4 (256KB)"  },
    { .mask=0x1F, .value=0x04, .start= 0x00080000u, .end=FLASH_END_1MB, .desc="Upper 1/2 (512KB)"  },

    /* Lower fractions */
    { .mask=0x1F, .value=0x09, .start= 0x00000000u, .end= 0x0000FFFFu, .desc="Lower 1/16 (64KB)"  },
    { .mask=0x1F, .value=0x0A, .start= 0x00000000u, .end= 0x0001FFFFu, .desc="Lower 1/8 (128KB)"  },
    { .mask=0x1F, .value=0x0B, .start= 0x00000000u, .end= 0x0003FFFFu, .desc="Lower 1/4 (256KB)"  },
    { .mask=0x1F, .value=0x0C, .start= 0x00000000u, .end= 0x0007FFFFu, .desc="Lower 1/2 (512KB)"  },

    /* ALL (accept multiple encodings) */
    { .mask=0x07, .value=0x07, .start= 0x00000000u, .end=FLASH_END_1MB, .desc="ALL Protected" },
    { .mask=0x07, .value=0x06, .start= 0x00000000u, .end=FLASH_END_1MB, .desc="ALL Protected" },
    { .mask=0x07, .value=0x05, .start= 0x00000000u, .end=FLASH_END_1MB, .desc="ALL Protected" },

    /* Top fixed-size */
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_4KB,  .start= 0x000FF000u, .end=FLASH_END_1MB, .desc="Top 4KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_8KB,  .start= 0x000FE000u, .end=FLASH_END_1MB, .desc="Top 8KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_TOP_16KB, .start= 0x000FC000u, .end=FLASH_END_1MB, .desc="Top 16KB" },
    { .mask=0x1E, .value=FLASH_BP_RAW_TOP_32KB, .start= 0x000F8000u, .end=FLASH_END_1MB, .desc="Top 32KB" },

    /* Bottom fixed-size */
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_4KB,  .start= 0x00000000u, .end= 0x00000FFFu, .desc="Bottom 4KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_8KB,  .start= 0x00000000u, .end= 0x00001FFFu, .desc="Bottom 8KB"  },
    { .mask=0x1F, .value=FLASH_BP_RAW_BOTTOM_16KB, .start= 0x00000000u, .end= 0x00003FFFu, .desc="Bottom 16KB" },
    { .mask=0x1E, .value=FLASH_BP_RAW_BOTTOM_32KB, .start= 0x00000000u, .end= 0x00007FFFu, .desc="Bottom 32KB" },
};
/* Pick table by capacity_id */
static uint32_t _get_tbl(uint8_t cap, const flash_bp_def_t **out_tbl, 
                         size_t *out_n, uint32_t *out_flash_end)
{
    if (!out_tbl || !out_n || !out_flash_end) 
        return FLASH_BP_STATUS_INVALID_PARAM;


     uint32_t base = flash_addr_base();  // 
    switch (cap) {
       
    case FLASH_SIZE_ID_1MB:
        *out_tbl = g_tbl_1mb;
        *out_n = sizeof(g_tbl_1mb)/sizeof(g_tbl_1mb[0]);
        *out_flash_end = base + FLASH_END_1MB;  // 
        return FLASH_BP_STATUS_SUCCESS;

    case FLASH_SIZE_ID_2MB:
        *out_tbl = g_tbl_2mb;
        *out_n = sizeof(g_tbl_2mb)/sizeof(g_tbl_2mb[0]);
        *out_flash_end = base + FLASH_END_2MB;  //
        return FLASH_BP_STATUS_SUCCESS;

    case FLASH_SIZE_ID_4MB:
        *out_tbl = g_tbl_4mb;
        *out_n = sizeof(g_tbl_4mb)/sizeof(g_tbl_4mb[0]);
        *out_flash_end = base + FLASH_END_4MB;  // 
        return FLASH_BP_STATUS_SUCCESS;

    default:
        return FLASH_BP_STATUS_UNSUPPORTED;
    }
}

/* ================= public: detect/init ================= */

uint32_t flash_bp_auto_detect(void)
{
    const uint32_t jedec = flash_get_deviceinfo();
    uint8_t manufacturer_id = GET_BYTE0(jedec); /* 0xC8 */
    uint8_t memory_type     = GET_BYTE1(jedec); /* 0x65 */
    uint8_t capacity_id     = GET_BYTE2(jedec); /* 0x14/0x15/0x16 */

    g_flash_device.manufacturer_id = manufacturer_id;
    g_flash_device.memory_type     = memory_type;
    g_flash_device.capacity_id     = capacity_id;


    switch (capacity_id) {
    case FLASH_SIZE_ID_1MB:
        g_flash_device.size_bytes = FLASH_SIZE_1MB;
        g_flash_device.size_kb    = 1024;
        g_flash_device.blocks     = 16;  /* 64KB blocks */
        break;

    case FLASH_SIZE_ID_2MB:
        g_flash_device.size_bytes = FLASH_SIZE_2MB;
        g_flash_device.size_kb    = 2048;
        g_flash_device.blocks     = 32;
        break;

    case FLASH_SIZE_ID_4MB:
        g_flash_device.size_bytes = FLASH_SIZE_4MB;
        g_flash_device.size_kb    = 4096;
        g_flash_device.blocks     = 64;
        break;

    default:
        FLASH_BP_PROTECT_LOGE("[FLASH_BP] Unsupported capacity id: 0x%02X", capacity_id);
        g_flash_device.detected = false;
        return FLASH_BP_STATUS_UNSUPPORTED;
    }

    g_flash_device.detected = true;

    FLASH_BP_PROTECT_LOGI("[FLASH_BP] Detected JEDEC: %02X %02X %02X (%u KB, %u blocks)",
                          manufacturer_id, memory_type, capacity_id,
                          (unsigned)g_flash_device.size_kb,
                          (unsigned)g_flash_device.blocks);

    return FLASH_BP_STATUS_SUCCESS;
}

uint32_t flash_bp_protect_init(void)
{
    return flash_bp_auto_detect();
}

const flash_bp_device_t* flash_bp_get_device_info(void)
{
    return &g_flash_device;
}

/* ================= public: mode -> raw mapping =================
 * "Defined mapping", no dynamic calculation.
 * Return INVALID_PARAM if the mode is not supported on that capacity.
 */
uint32_t flash_bp_mode_to_raw(flash_bp_mode_t mode, uint8_t *out_bp_raw)
{
    if (!out_bp_raw) return FLASH_BP_STATUS_INVALID_PARAM;
    if (!g_flash_device.detected) return FLASH_BP_STATUS_UNSUPPORTED;

    uint8_t cap = g_flash_device.capacity_id;

    /* fixed-size modes share same raw on all */
    switch (mode) {
    case FLASH_BP_MODE_NONE:        *out_bp_raw = FLASH_BP_RAW_NONE; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_TOP_4KB:     *out_bp_raw = FLASH_BP_RAW_TOP_4KB; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_TOP_8KB:     *out_bp_raw = FLASH_BP_RAW_TOP_8KB; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_TOP_16KB:    *out_bp_raw = FLASH_BP_RAW_TOP_16KB; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_TOP_32KB:    *out_bp_raw = FLASH_BP_RAW_TOP_32KB; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_BOTTOM_4KB:  *out_bp_raw = FLASH_BP_RAW_BOTTOM_4KB; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_BOTTOM_8KB:  *out_bp_raw = FLASH_BP_RAW_BOTTOM_8KB; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_BOTTOM_16KB: *out_bp_raw = FLASH_BP_RAW_BOTTOM_16KB; return FLASH_BP_STATUS_SUCCESS;
    case FLASH_BP_MODE_BOTTOM_32KB: *out_bp_raw = FLASH_BP_RAW_BOTTOM_32KB; return FLASH_BP_STATUS_SUCCESS;
    default:
        break;
    }

    /* fraction modes: map per capacity */
    if (cap == FLASH_SIZE_ID_1MB) {
        switch (mode) {
        case FLASH_BP_MODE_UPPER_1_16: *out_bp_raw = 0x01; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_8:  *out_bp_raw = 0x02; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_4:  *out_bp_raw = 0x03; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_2:  *out_bp_raw = 0x04; return FLASH_BP_STATUS_SUCCESS;

        case FLASH_BP_MODE_LOWER_1_16: *out_bp_raw = 0x09; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_8:  *out_bp_raw = 0x0A; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_4:  *out_bp_raw = 0x0B; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_2:  *out_bp_raw = 0x0C; return FLASH_BP_STATUS_SUCCESS;

        case FLASH_BP_MODE_ALL:        *out_bp_raw = 0x07; return FLASH_BP_STATUS_SUCCESS; /* accept 0x07 */
        default:
            return FLASH_BP_STATUS_INVALID_PARAM;
        }
    }

    if (cap == FLASH_SIZE_ID_2MB) {
        switch (mode) {
        case FLASH_BP_MODE_UPPER_1_32: *out_bp_raw = 0x01; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_16: *out_bp_raw = 0x02; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_8:  *out_bp_raw = 0x03; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_4:  *out_bp_raw = 0x04; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_2:  *out_bp_raw = 0x05; return FLASH_BP_STATUS_SUCCESS;

        case FLASH_BP_MODE_LOWER_1_32: *out_bp_raw = 0x09; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_16: *out_bp_raw = 0x0A; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_8:  *out_bp_raw = 0x0B; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_4:  *out_bp_raw = 0x0C; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_2:  *out_bp_raw = 0x0D; return FLASH_BP_STATUS_SUCCESS;

        case FLASH_BP_MODE_ALL:        *out_bp_raw = 0x1F; return FLASH_BP_STATUS_SUCCESS; /* any XX11X, 0x1C is OK */
        default:
            return FLASH_BP_STATUS_INVALID_PARAM;
        }
    }

    if (cap == FLASH_SIZE_ID_4MB) {
        switch (mode) {
        case FLASH_BP_MODE_UPPER_1_64: *out_bp_raw = 0x01; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_32: *out_bp_raw = 0x02; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_16: *out_bp_raw = 0x03; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_8:  *out_bp_raw = 0x04; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_4:  *out_bp_raw = 0x05; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_UPPER_1_2:  *out_bp_raw = 0x06; return FLASH_BP_STATUS_SUCCESS;

        case FLASH_BP_MODE_LOWER_1_64: *out_bp_raw = 0x09; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_32: *out_bp_raw = 0x0A; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_16: *out_bp_raw = 0x0B; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_8:  *out_bp_raw = 0x0C; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_4:  *out_bp_raw = 0x0D; return FLASH_BP_STATUS_SUCCESS;
        case FLASH_BP_MODE_LOWER_1_2:  *out_bp_raw = 0x0E; return FLASH_BP_STATUS_SUCCESS;

        case FLASH_BP_MODE_ALL:        *out_bp_raw = 0x07; return FLASH_BP_STATUS_SUCCESS; /* XX111 */
        default:
            return FLASH_BP_STATUS_INVALID_PARAM;
        }
    }

    return FLASH_BP_STATUS_UNSUPPORTED;
}


/* ================= public: region calculation ================= */
uint32_t flash_bp_calculate_protect_region(uint8_t bp_value,
                                           bool cmp_enabled,
                                           flash_bp_protect_info_t *info)
{
    if (!info) return FLASH_BP_STATUS_INVALID_PARAM;
    if (!g_flash_device.detected) return FLASH_BP_STATUS_UNSUPPORTED;

    info->start_addr  = 0u;
    info->end_addr    = 0u;
    info->size        = 0u;
    info->description = "Unknown";

    const flash_bp_def_t *tbl = NULL;
    size_t n = 0;
    uint32_t flash_end = 0;

    uint32_t st = _get_tbl(g_flash_device.capacity_id, &tbl, &n, &flash_end);
    if (st != FLASH_BP_STATUS_SUCCESS) return st;

    uint8_t bp = (uint8_t)(bp_value & 0x1Fu);

    if (!_bp_lookup(tbl, n, bp, info)) {
        //printf("bp looup return\r\n");
        return FLASH_BP_STATUS_INVALID_PARAM;
    }

    if (cmp_enabled) {
        _invert_region(flash_end, info); /* using offset end，corret */
    }

    /* ===== absolute address ===== */
    const uint32_t base = flash_addr_base();

    if (info->size != 0u) {
        info->start_addr += base;
        info->end_addr   += base;
    } else {
        /* size==0 => No Protection，start/end keep 0 */
    }

    return FLASH_BP_STATUS_SUCCESS;
}

/* ================= public: status read ================= */
uint32_t flash_bp_get_protect_status(uint8_t *bp_value, bool *cmp_enabled)
{
    flash_status_t flash;
    uint8_t status1, status2;  // 

    if (!bp_value || !cmp_enabled) return FLASH_BP_STATUS_INVALID_PARAM;
    if (!g_flash_device.detected) return FLASH_BP_STATUS_UNSUPPORTED;

    flash.status1 = 0;
    flash.status2 = 0;
    flash.status3 = 0;

    flash.require_mode = FLASH_STATUS_RW1;
    flash_get_status_reg(&flash);
    status1 = flash.status1;  // ← 立即保存

    flash.require_mode = FLASH_STATUS_RW2;
    flash_get_status_reg(&flash);
    status2 = flash.status2;  // ← 立即保存

    // FLASH_BP_PROTECT_LOGI("flash_bp_get_protect_status \r\n");
    // FLASH_BP_PROTECT_LOGI("flash.status1 = %.2x\r\n",status1);
    // FLASH_BP_PROTECT_LOGI("flash.status2 = %.2x\r\n",status2);


    *bp_value     = (uint8_t)((status1 & FLASH_BP_BP_MASK) >> 2);  //
    *cmp_enabled  = ((status2 & FLASH_BP_CMP_BIT) != 0u);          // 


    return FLASH_BP_STATUS_SUCCESS;
}

/* ================= internal: set protection ================= */
static uint32_t _set_protection_internal(uint8_t bp_value, bool cmp_enable)
{
    uint8_t status1, status2;
    flash_status_t flash;

    if (!g_flash_device.detected) return FLASH_BP_STATUS_UNSUPPORTED;
    if (flash_check_busy()) return FLASH_BP_STATUS_EBUSY;
    if (bp_value > FLASH_BP_RAW_MAX) return FLASH_BP_STATUS_INVALID_PARAM;

    flash.status1 = 0;
    flash.status2 = 0;
    flash.status3 = 0;

    flash.require_mode = FLASH_STATUS_RW1;
    flash_get_status_reg(&flash);
    flash.require_mode = FLASH_STATUS_RW2;
    flash_get_status_reg(&flash);

    status1 = flash.status1;
    status2 = flash.status2;

    // FLASH_BP_PROTECT_LOGI("_set_protection_internal 1 \r\n");
    // FLASH_BP_PROTECT_LOGI("flash.status1 = %.2x\r\n",flash.status1);
    // FLASH_BP_PROTECT_LOGI("flash.status2 = %.2x\r\n",flash.status2);
    /* BP bits in SR1[6:2] */
    status1 &= (uint8_t)~FLASH_BP_BP_MASK;
    status1 |= (uint8_t)((bp_value & 0x1Fu) << 2);

    if (cmp_enable) status2 |= FLASH_BP_CMP_BIT;
    else            status2 &= (uint8_t)~FLASH_BP_CMP_BIT;

    flash.require_mode = FLASH_STATUS_RW1_2;
    flash.status1 = status1;
    flash.status2 = status2;

    // FLASH_BP_PROTECT_LOGI("_set_protection_internal 2\r\n");
    // FLASH_BP_PROTECT_LOGI("flash.status1 = %.2x\r\n",flash.status1);
    // FLASH_BP_PROTECT_LOGI("flash.status2 = %.2x\r\n",flash.status2);

    flash_set_status_reg(&flash);


    // flash.status1 = 0;
    // flash.status2 = 0;
    // flash.status3 = 0;

    // flash.require_mode = FLASH_STATUS_RW1;
    // flash_get_status_reg(&flash);
    // flash.require_mode = FLASH_STATUS_RW2;
    // flash_get_status_reg(&flash);

    // status1 = flash.status1;
    // status2 = flash.status2;

    // FLASH_BP_PROTECT_LOGI("_set_protection_internal 3\r\n");
    // FLASH_BP_PROTECT_LOGI("flash.status1 = %.2x\r\n",flash.status1);
    // FLASH_BP_PROTECT_LOGI("flash.status2 = %.2x\r\n",flash.status2);


    return FLASH_BP_STATUS_SUCCESS;
}

static uint32_t flash_bp_apply_raw(uint8_t bp_value, bool inverted)
{
    return _set_protection_internal(bp_value & 0x1F, inverted);
}

uint32_t flash_bp_set_block_protect_mode(flash_bp_mode_t mode, bool inverted)
{
    uint8_t bp_raw = 0;
    uint32_t ret = flash_bp_mode_to_raw(mode, &bp_raw);
    if (ret != FLASH_BP_STATUS_SUCCESS) {
        return ret;
    }
    return flash_bp_apply_raw(bp_raw, inverted);
}

/* raw */
uint32_t flash_bp_set_block_protect(uint8_t bp_value)
{
    return flash_bp_apply_raw(bp_value, false);
}

uint32_t flash_bp_set_block_protect_inverted(uint8_t bp_value)
{
    return flash_bp_apply_raw(bp_value, true);
}

uint32_t flash_bp_set_block_protect_raw(uint8_t bp_value)
{
    return flash_bp_apply_raw(bp_value, false);
}

uint32_t flash_bp_set_block_protect_raw_inverted(uint8_t bp_value)
{
    return flash_bp_apply_raw(bp_value, true);
}

uint32_t flash_bp_remove_all_protection(void)
{
    return flash_bp_set_block_protect(FLASH_BP_RAW_NONE);
}

uint32_t flash_bp_set_cmp_mode(bool enable)
{
    flash_status_t flash;
    uint8_t status1, status2;

    if (!g_flash_device.detected) return FLASH_BP_STATUS_UNSUPPORTED;
    if (flash_check_busy()) return FLASH_BP_STATUS_EBUSY;

    flash.status1 = 0;
    flash.status2 = 0;
    flash.status3 = 0;

    flash.require_mode = FLASH_STATUS_RW1;
    flash_get_status_reg(&flash);
    flash.require_mode = FLASH_STATUS_RW2;
    flash_get_status_reg(&flash);

    status1 = flash.status1;
    status2 = flash.status2;

    if (enable) status2 |= FLASH_BP_CMP_BIT;
    else        status2 &= (uint8_t)~FLASH_BP_CMP_BIT;

    flash.require_mode = FLASH_STATUS_RW1_2;
    flash.status1 = status1;
    flash.status2 = status2;

    flash_set_status_reg(&flash);

    return FLASH_BP_STATUS_SUCCESS;
}

/* ================= public: address check ================= */
static inline bool _flash_bp_is_offset_addr(uint32_t address)
{
    #if defined(CONFIG_RT581) || defined(CONFIG_RT582) ||defined(CONFIG_RT583)
        (void)address;
        return true;
    #else
    
        #if defined(CONFIG_FLASHCTRL_SECURE_EN) // rt584 only
        return (address < g_flash_device.size_bytes);
        #else
        (void)address;
        return true;
        #endif 
    #endif
}


uint32_t flash_bp_is_address_protected(uint32_t address, bool *is_protected)
{
    uint8_t bp_value;
    bool cmp_enabled;
    flash_bp_protect_info_t info;
    uint32_t st;

    if (!is_protected) return FLASH_BP_STATUS_INVALID_PARAM;
    if (!g_flash_device.detected) return FLASH_BP_STATUS_UNSUPPORTED;

    /* Normalize input address to absolute (if base is used) */
    const uint32_t base = flash_addr_base();
    uint32_t abs_addr = address;

    if (_flash_bp_is_offset_addr(address)) {
        abs_addr = address + base;
    }

    st = flash_bp_get_protect_status(&bp_value, &cmp_enabled);
    if (st != FLASH_BP_STATUS_SUCCESS) return st;

    /* calculate_protect_region() already returns absolute if base defined */
    st = flash_bp_calculate_protect_region(bp_value, cmp_enabled, &info);
    if (st != FLASH_BP_STATUS_SUCCESS) return st;

    if (info.size == 0u) {
        *is_protected = false;
    } else {
        *is_protected = (abs_addr >= info.start_addr && abs_addr <= info.end_addr);
    }

    return FLASH_BP_STATUS_SUCCESS;
}

/* ================= debug print ================= */
void flash_bp_print_protect_status(void)
{
    uint8_t bp_value;
    bool cmp_enabled;
    flash_bp_protect_info_t info;
    uint32_t st;

    if (!g_flash_device.detected) {
        FLASH_BP_PROTECT_LOGI("Flash not detected! Run flash_bp_protect_init() first.\n");
        return;
    }

    st = flash_bp_get_protect_status(&bp_value, &cmp_enabled);
    if (st != FLASH_BP_STATUS_SUCCESS) {
        FLASH_BP_PROTECT_LOGI("Failed to read protection status\n");
        return;
    }

    st = flash_bp_calculate_protect_region(bp_value, cmp_enabled, &info);
    if (st != FLASH_BP_STATUS_SUCCESS) {
        FLASH_BP_PROTECT_LOGI("Failed to calculate protection region\n");
        return;
    }

    FLASH_BP_PROTECT_LOGI("\n========================================\n");
    FLASH_BP_PROTECT_LOGI("Flash BP Protection Status\n");
    FLASH_BP_PROTECT_LOGI("========================================\n");
    FLASH_BP_PROTECT_LOGI("JEDEC: %02X %02X %02X\n",
           g_flash_device.manufacturer_id,
           g_flash_device.memory_type,
           g_flash_device.capacity_id);
    FLASH_BP_PROTECT_LOGI("Device: (%u KB)\n", (unsigned)g_flash_device.size_kb);
    FLASH_BP_PROTECT_LOGI("BP Value: 0x%02X (BP4-BP0)\n", (unsigned)bp_value);
    FLASH_BP_PROTECT_LOGI("CMP Mode: %s\n", cmp_enabled ? "Inverted (1)" : "Normal (0)");
    FLASH_BP_PROTECT_LOGI("----------------------------------------\n");
    FLASH_BP_PROTECT_LOGI("Protection: %s\n", info.description);

    if (info.size > 0u) {
        FLASH_BP_PROTECT_LOGI("Protected Range: 0x%08X - 0x%08X\n", (unsigned)info.start_addr, (unsigned)info.end_addr);
        FLASH_BP_PROTECT_LOGI("Protected Size: %u bytes", (unsigned)info.size);
        if (info.size >= 1024u * 1024u) {
            FLASH_BP_PROTECT_LOGI(" (%.2f MB)", (double)info.size / (1024.0 * 1024.0));
        } else if (info.size >= 1024u) {
            FLASH_BP_PROTECT_LOGI(" (%.2f KB)", (double)info.size / 1024.0);
        }
        FLASH_BP_PROTECT_LOGI("\n");
    } else {
        FLASH_BP_PROTECT_LOGI("Protected Range: None\n");
        FLASH_BP_PROTECT_LOGI("Writable Size: All (%u KB)\n", (unsigned)g_flash_device.size_kb);
    }

    FLASH_BP_PROTECT_LOGI("========================================\n\n");
}

uint32_t flash_bp_verify_status_registers(uint8_t bp, bool cmp_enabled)
{
    FLASH_BP_PROTECT_LOGI("[VERIFY] Verifying status registers...\n");
    
    // get real register
    uint8_t actual_bp;
    bool actual_cmp;
    
    uint32_t result = flash_bp_get_protect_status(&actual_bp, &actual_cmp);
    if (result != FLASH_BP_STATUS_SUCCESS) {
        FLASH_BP_PROTECT_LOGI("[VERIFY] Failed to read status registers\n");
        return STATUS_ERROR;
    }
    
    // verify block value
    if (actual_bp != bp) {
        FLASH_BP_PROTECT_LOGI("[VERIFY] BP mismatch! Expected: 0x%02X, Actual: 0x%02X\n", 
               bp, actual_bp);
        return STATUS_ERROR;
    }
    
    // verify block cmp value
    if (actual_cmp != cmp_enabled) {
        FLASH_BP_PROTECT_LOGI("[VERIFY] CMP mismatch! Expected: %d, Actual: %d\n", 
               cmp_enabled, actual_cmp);
        return STATUS_ERROR;
    }
    
    FLASH_BP_PROTECT_LOGI("[VERIFY]   Status registers verified successfully\n");
    FLASH_BP_PROTECT_LOGI("[VERIFY]   BP = 0x%02X, CMP = %d\n", actual_bp, actual_cmp);
    
    return STATUS_SUCCESS;
}