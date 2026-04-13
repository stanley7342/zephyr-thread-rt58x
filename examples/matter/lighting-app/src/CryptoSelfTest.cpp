/*
 * RT583 Hardware Crypto Self-Test
 *
 * Verifies AES-128-ECB, AES-128-CBC, and SHA-256 hardware accelerators
 * against NIST Known-Answer Test (KAT) vectors.
 *
 * Call before hosal_rf_init() so the shared crypto accelerator is idle
 * (lmac15p4 has not started and no concurrent ECC/AES operations can occur).
 *
 * AES and ECC share the same hardware accelerator on RT583.  After this
 * test the accelerator has AES firmware loaded; lmac15p4 will reload
 * whichever firmware it needs when it starts.
 *
 * SHA-256 is implemented in RT583 ROM (function pointers at fixed ROM
 * addresses).  sha256_vector_init() wires those pointers; subsequent
 * calls to sha256_init/update/finish invoke the ROM code directly.
 */

#include <string.h>
#include <zephyr/kernel.h>

extern "C" {
#include "rt_aes.h"
#include "rt_sha256.h"
}

/* ── AES-128-ECB — NIST FIPS-197 Appendix B ───────────────────────────────── */
static const uint8_t kAesKey128[16] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c,
};
static const uint8_t kAesPlain[16] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
    0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
};
static const uint8_t kAesEcbCipher[16] = {
    0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
    0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97,
};

/* ── AES-128-CBC — NIST SP 800-38A Example F.2.1 (first block) ────────────── */
static const uint8_t kAesCbcIv[16] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};
static const uint8_t kAesCbcCipher[16] = {
    0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46,
    0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
};

/* ── SHA-256("abc") — FIPS 180-4 ──────────────────────────────────────────── */
static const uint8_t kShaInput[] = { 'a', 'b', 'c' };
static const uint8_t kShaExpected[SHA256_DIGEST_SIZE] = {
    0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
    0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x2e, 0xc7,
    0x3b, 0x00, 0x36, 0x1b, 0xbe, 0xf0, 0x46, 0x9f,
    0x82, 0xcf, 0x68, 0x3a, 0xb9, 0xbc, 0x5b, 0x5b,
};

/* ── HMAC-SHA256 — RFC 4231 Test Case 1 ───────────────────────────────────── */
static const uint8_t kHmacKey[20] = {
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    0x0b, 0x0b, 0x0b, 0x0b,
};
static const uint8_t kHmacData[] = { 'H', 'i', ' ', 'T', 'h', 'e', 'r', 'e' };
static const uint8_t kHmacExpected[SHA256_DIGEST_SIZE] = {
    0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
    0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
    0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
    0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
};

/* ── Helpers ──────────────────────────────────────────────────────────────── */

static void print_hex(const char *label, const uint8_t *buf, size_t len)
{
    printk("  %s: ", label);
    for (size_t i = 0; i < len; i++) {
        printk("%02x", buf[i]);
    }
    printk("\n");
}

/* ── AES-128-ECB test ─────────────────────────────────────────────────────── */

static bool test_aes128_ecb(void)
{
    struct aes_ctx ctx;
    uint8_t cipher[16];
    uint8_t recovered[16];
    bool ok = true;

    /* Load AES firmware to the shared crypto accelerator */
    aes_fw_init();
    aes_key_init(&ctx, kAesKey128, AES_KEY128);
    aes_load_round_key(&ctx);

    uint32_t t0 = k_uptime_get_32();
    aes_ecb_encrypt(&ctx, (uint8_t *)kAesPlain, cipher);
    uint32_t enc_ms = k_uptime_get_32() - t0;

    if (memcmp(cipher, kAesEcbCipher, 16) != 0) {
        printk("[CRYPTO] AES-128-ECB encrypt FAIL\n");
        print_hex("got ", cipher, 16);
        print_hex("want", kAesEcbCipher, 16);
        ok = false;
    }

    aes_ecb_decrypt(&ctx, cipher, recovered);
    if (memcmp(recovered, kAesPlain, 16) != 0) {
        printk("[CRYPTO] AES-128-ECB decrypt FAIL\n");
        ok = false;
    }

    if (ok) {
        printk("[CRYPTO] AES-128-ECB PASS (encrypt %u ms)\n", enc_ms);
    }
    return ok;
}

/* ── AES-128-CBC test ─────────────────────────────────────────────────────── */

static bool test_aes128_cbc(void)
{
    struct aes_ctx ctx;
    uint8_t cipher[16];
    uint8_t recovered[16];
    bool ok = true;

    /* aes_fw_init() already called in ECB test; key still loaded */
    aes_key_init(&ctx, kAesKey128, AES_KEY128);
    aes_load_round_key(&ctx);
    aes_iv_set(&ctx, kAesCbcIv);

    uint32_t t0 = k_uptime_get_32();
    aes_cbc_buffer_encrypt(&ctx, (uint8_t *)kAesPlain, cipher, 16);
    uint32_t enc_ms = k_uptime_get_32() - t0;

    if (memcmp(cipher, kAesCbcCipher, 16) != 0) {
        printk("[CRYPTO] AES-128-CBC encrypt FAIL\n");
        print_hex("got ", cipher, 16);
        print_hex("want", kAesCbcCipher, 16);
        ok = false;
    }

    /* Decrypt: reset key and IV */
    aes_key_init(&ctx, kAesKey128, AES_KEY128);
    aes_load_round_key(&ctx);
    aes_iv_set(&ctx, kAesCbcIv);
    aes_cbc_buffer_decrypt(&ctx, cipher, recovered, 16);

    if (memcmp(recovered, kAesPlain, 16) != 0) {
        printk("[CRYPTO] AES-128-CBC decrypt FAIL\n");
        ok = false;
    }

    if (ok) {
        printk("[CRYPTO] AES-128-CBC PASS (encrypt %u ms)\n", enc_ms);
    }
    return ok;
}

/* ── SHA-256 test ─────────────────────────────────────────────────────────── */

static bool test_sha256(void)
{
    sha256_context ctx;
    uint8_t digest[SHA256_DIGEST_SIZE];

    /* Wire sha256_init/update/finish function pointers to ROM implementation */
    sha256_vector_init();

    uint32_t t0 = k_uptime_get_32();
    sha256_init(&ctx);
    sha256_update(&ctx, (uint8_t *)kShaInput, sizeof(kShaInput));
    sha256_finish(&ctx, digest);
    uint32_t hash_ms = k_uptime_get_32() - t0;

    if (memcmp(digest, kShaExpected, SHA256_DIGEST_SIZE) != 0) {
        printk("[CRYPTO] SHA-256 FAIL\n");
        print_hex("got ", digest, SHA256_DIGEST_SIZE);
        print_hex("want", kShaExpected, SHA256_DIGEST_SIZE);
        return false;
    }

    printk("[CRYPTO] SHA-256(\"abc\") PASS (hash %u ms)\n", hash_ms);
    return true;
}

/* ── HMAC-SHA256 test ─────────────────────────────────────────────────────── */

static bool test_hmac_sha256(void)
{
    uint8_t mac[SHA256_DIGEST_SIZE];

    uint32_t t0 = k_uptime_get_32();
    uint32_t ret = hmac_sha256(kHmacKey, sizeof(kHmacKey),
                               kHmacData, sizeof(kHmacData),
                               mac);
    uint32_t hmac_ms = k_uptime_get_32() - t0;

    if (ret != 0) {
        printk("[CRYPTO] HMAC-SHA256 error: %u\n", (unsigned)ret);
        return false;
    }

    if (memcmp(mac, kHmacExpected, SHA256_DIGEST_SIZE) != 0) {
        printk("[CRYPTO] HMAC-SHA256 FAIL\n");
        print_hex("got ", mac, SHA256_DIGEST_SIZE);
        print_hex("want", kHmacExpected, SHA256_DIGEST_SIZE);
        return false;
    }

    printk("[CRYPTO] HMAC-SHA256 PASS (hmac %u ms)\n", hmac_ms);
    return true;
}

/* ── Public entry point ───────────────────────────────────────────────────── */

void crypto_selftest(void)
{
    printk("[CRYPTO] === RT583 hardware crypto self-test ===\n");

    bool aes_ecb_ok  = test_aes128_ecb();
    bool aes_cbc_ok  = test_aes128_cbc();
    bool sha_ok      = test_sha256();
    bool hmac_ok     = test_hmac_sha256();

    bool all_ok = aes_ecb_ok && aes_cbc_ok && sha_ok && hmac_ok;
    printk("[CRYPTO] Result: AES-ECB=%s AES-CBC=%s SHA256=%s HMAC=%s — %s\n",
           aes_ecb_ok ? "OK" : "FAIL",
           aes_cbc_ok ? "OK" : "FAIL",
           sha_ok     ? "OK" : "FAIL",
           hmac_ok    ? "OK" : "FAIL",
           all_ok     ? "PASS" : "FAIL");
}
