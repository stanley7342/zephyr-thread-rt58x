/*
 * ot_entropy.c — OpenThread entropy platform for Zephyr on RT583
 *
 * Uses Rafael SDK hosal_trng_get_random_number() which calls
 * get_random_number() directly against the RT583 TRNG hardware.
 * No FreeRTOS dependency in the TRNG path.
 */

#include <zephyr/sys/printk.h>
#include <openthread/platform/entropy.h>
#include "code_utils.h"
#include "mcu.h"
#include "hosal_trng.h"

void ot_entropy_init(void)
{
    uint32_t seed;
    printk("[OT-ENTROPY] init: reading first random word from TRNG...\n");
    hosal_trng_get_random_number(&seed, 1);
    printk("[OT-ENTROPY] init done, seed=0x%08x\n", seed);
    (void)seed;
}

otError otPlatEntropyGet(uint8_t *aOutput, uint16_t aOutputLength)
{
    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(aOutput, error = OT_ERROR_INVALID_ARGS);

    printk("[OT-ENTROPY] otPlatEntropyGet len=%u\n", (unsigned)aOutputLength);
    for (uint16_t i = 0; i < aOutputLength; i++) {
        uint32_t r;
        hosal_trng_get_random_number(&r, 1);
        aOutput[i] = (uint8_t)(r & 0xFF);
    }
    printk("[OT-ENTROPY] otPlatEntropyGet done\n");
exit:
    return error;
}
