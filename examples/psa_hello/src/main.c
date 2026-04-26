/*
 * PSA Crypto hello-world — rt583_evb / rt584_evb.
 *
 * Calls psa_hash_compute() with SHA-256 on the literal string "hello"
 * and verifies the result against the known good digest. This is the
 * minimum viable proof that mbedTLS PSA Crypto is wired up correctly
 * on rt58x — pre-requisite for running ARM's psa-arch-tests suite.
 */

#include <psa/crypto.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/* SHA-256("hello") — well-known test vector. */
static const uint8_t expected_sha256[32] = {
    0x2c, 0xf2, 0x4d, 0xba, 0x5f, 0xb0, 0xa3, 0x0e,
    0x26, 0xe8, 0x3b, 0x2a, 0xc5, 0xb9, 0xe2, 0x9e,
    0x1b, 0x16, 0x1e, 0x5c, 0x1f, 0xa7, 0x42, 0x5e,
    0x73, 0x04, 0x33, 0x62, 0x93, 0x8b, 0x98, 0x24,
};

static void hexdump(const char *label, const uint8_t *buf, size_t len)
{
    printk("%s ", label);
    for (size_t i = 0; i < len; i++) {
        printk("%02x", buf[i]);
    }
    printk("\n");
}

int main(void)
{
    printk("=========================================\n");
    printk("  %s  PSA Crypto hello\n", CONFIG_BOARD);
    printk("=========================================\n");

    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        printk("FAIL: psa_crypto_init -> %d\n", status);
        return 0;
    }
    printk("[PSA] init ok\n");

    const char *msg = "hello";
    uint8_t hash[PSA_HASH_LENGTH(PSA_ALG_SHA_256)];
    size_t hash_len = 0;

    status = psa_hash_compute(PSA_ALG_SHA_256,
                              (const uint8_t *)msg, strlen(msg),
                              hash, sizeof(hash), &hash_len);
    if (status != PSA_SUCCESS) {
        printk("FAIL: psa_hash_compute -> %d\n", status);
        return 0;
    }
    printk("[PSA] sha256(\"hello\") computed, len=%u\n", (unsigned)hash_len);

    hexdump("  got     :", hash, hash_len);
    hexdump("  expected:", expected_sha256, sizeof(expected_sha256));

    if (hash_len == sizeof(expected_sha256) &&
        memcmp(hash, expected_sha256, hash_len) == 0) {
        printk("[PSA] PASS\n");
    } else {
        printk("[PSA] FAIL — digest mismatch\n");
    }

    while (1) {
        k_msleep(1000);
    }
    return 0;
}
