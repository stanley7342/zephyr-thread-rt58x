/*
 * ot_entropy.c — OpenThread entropy platform for Zephyr on RT582
 *
 * Uses Rafael SDK hosal_trng_get_random_number() which calls
 * get_random_number() directly against the RT582 TRNG hardware.
 * No FreeRTOS dependency in the TRNG path.
 */

#include <openthread/platform/entropy.h>
#include "code_utils.h"
#include "mcu.h"
#include "hosal_trng.h"

void ot_entropy_init(void)
{
    uint32_t seed;
    hosal_trng_get_random_number(&seed, 1);
    (void)seed;
}

otError otPlatEntropyGet(uint8_t *aOutput, uint16_t aOutputLength)
{
    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(aOutput, error = OT_ERROR_INVALID_ARGS);

    for (uint16_t i = 0; i < aOutputLength; i++) {
        uint32_t r;
        hosal_trng_get_random_number(&r, 1);
        aOutput[i] = (uint8_t)(r & 0xFF);
    }
exit:
    return error;
}
