/*
 * ot_settings.c — OpenThread platform settings for Zephyr on RT583
 *
 * Implements otPlatSettings* API.  The SDK's libopenthread-ftd.a calls these
 * directly (it was NOT built with OPENTHREAD_CONFIG_PLATFORM_FLASH_API_ENABLE).
 *
 * This implementation stores all settings in a RAM array.  Data is lost on
 * reset, which is acceptable for initial bring-up.  Flash-backed persistence
 * can be layered on top later.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <openthread/error.h>
#include <openthread/platform/settings.h>
#include <zephyr/sys/printk.h>

/* ── Store limits ────────────────────────────────────────────────────────── *
 * RT583 has 144 KB of SRAM.  Keep the settings store small.  128 bytes     *
 * covers the longest typical OT setting (child/neighbor table entries).    */
#define OT_SETTINGS_MAX_ENTRIES   32
#define OT_SETTINGS_MAX_VALUE_LEN 128

/* ── Entry record ────────────────────────────────────────────────────────── */
typedef struct {
    uint16_t key;
    uint16_t len;
    uint8_t  data[OT_SETTINGS_MAX_VALUE_LEN];
    bool     valid;
} OtSettingsEntry;

static OtSettingsEntry sStore[OT_SETTINGS_MAX_ENTRIES];
static int             sTop;   /* highest used index + 1 */

/* ── otPlatSettingsInit / Deinit ─────────────────────────────────────────── */
void otPlatSettingsInit(otInstance *aInstance,
                        const uint16_t *aSensitiveKeys,
                        uint16_t        aSensitiveKeysLength)
{
    printk("[OT-SETTINGS] otPlatSettingsInit\n");
    (void)aInstance;
    (void)aSensitiveKeys;
    (void)aSensitiveKeysLength;
    memset(sStore, 0, sizeof(sStore));
    sTop = 0;
}

void otPlatSettingsDeinit(otInstance *aInstance)
{
    (void)aInstance;
}

/* ── otPlatSettingsGet ───────────────────────────────────────────────────── */
otError otPlatSettingsGet(otInstance *aInstance, uint16_t aKey, int aIndex,
                          uint8_t *aValue, uint16_t *aValueLength)
{
    (void)aInstance;
    int match = 0;

    for (int i = 0; i < sTop; i++) {
        if (!sStore[i].valid || sStore[i].key != aKey) {
            continue;
        }
        if (match == aIndex) {
            if (aValueLength != NULL) {
                uint16_t copy = (aValue != NULL && *aValueLength < sStore[i].len)
                                ? *aValueLength : sStore[i].len;
                if (aValue != NULL) {
                    memcpy(aValue, sStore[i].data, copy);
                }
                *aValueLength = sStore[i].len;
            }
            return OT_ERROR_NONE;
        }
        match++;
    }
    return OT_ERROR_NOT_FOUND;
}

/* ── otPlatSettingsSet ───────────────────────────────────────────────────── */
otError otPlatSettingsSet(otInstance *aInstance, uint16_t aKey,
                          const uint8_t *aValue, uint16_t aValueLength)
{
    (void)aInstance;

    /* Invalidate all existing entries for this key */
    for (int i = 0; i < sTop; i++) {
        if (sStore[i].valid && sStore[i].key == aKey) {
            sStore[i].valid = false;
        }
    }
    /* Write the new single value */
    return otPlatSettingsAdd(aInstance, aKey, aValue, aValueLength);
}

/* ── otPlatSettingsAdd ───────────────────────────────────────────────────── */
otError otPlatSettingsAdd(otInstance *aInstance, uint16_t aKey,
                          const uint8_t *aValue, uint16_t aValueLength)
{
    (void)aInstance;

    if (aValueLength > OT_SETTINGS_MAX_VALUE_LEN) {
        return OT_ERROR_NO_BUFS;
    }

    /* Find a free slot (prefer reusing invalidated entries) */
    int slot = -1;
    for (int i = 0; i < OT_SETTINGS_MAX_ENTRIES; i++) {
        if (!sStore[i].valid) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        return OT_ERROR_NO_BUFS;
    }

    sStore[slot].key   = aKey;
    sStore[slot].len   = aValueLength;
    memcpy(sStore[slot].data, aValue, aValueLength);
    sStore[slot].valid = true;

    if (slot >= sTop) {
        sTop = slot + 1;
    }
    return OT_ERROR_NONE;
}

/* ── otPlatSettingsDelete ────────────────────────────────────────────────── */
otError otPlatSettingsDelete(otInstance *aInstance, uint16_t aKey, int aIndex)
{
    (void)aInstance;

    if (aIndex < 0) {
        /* Delete all entries with this key */
        bool found = false;
        for (int i = 0; i < sTop; i++) {
            if (sStore[i].valid && sStore[i].key == aKey) {
                sStore[i].valid = false;
                found = true;
            }
        }
        return found ? OT_ERROR_NONE : OT_ERROR_NOT_FOUND;
    }

    int match = 0;
    for (int i = 0; i < sTop; i++) {
        if (!sStore[i].valid || sStore[i].key != aKey) {
            continue;
        }
        if (match == aIndex) {
            sStore[i].valid = false;
            return OT_ERROR_NONE;
        }
        match++;
    }
    return OT_ERROR_NOT_FOUND;
}

/* ── otPlatSettingsWipe ──────────────────────────────────────────────────── */
void otPlatSettingsWipe(otInstance *aInstance)
{
    (void)aInstance;
    memset(sStore, 0, sizeof(sStore));
    sTop = 0;
}
