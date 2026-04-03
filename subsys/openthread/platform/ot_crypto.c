/*
 * ot_crypto.c — OpenThread crypto platform for Zephyr on RT582
 * Uses Rafael SDK TRNG for random; hardware AES/SHA disabled for simplicity.
 * OT_ENTER/EXIT_CRITICAL now use Zephyr irq_lock via openthread_port.h.
 */

#include <assert.h>
#include <string.h>

#include <openthread/config.h>
#include <openthread/platform/crypto.h>

#include "openthread_port.h"
#include "hosal_trng.h"

otError otPlatCryptoRandomGet(uint8_t *aBuffer, uint16_t aSize)
{
    for (uint16_t i = 0; i < aSize; i++) {
        uint32_t r;
        hosal_trng_get_random_number(&r, 1);
        aBuffer[i] = (uint8_t)(r & 0xFF);
    }
    return OT_ERROR_NONE;
}
