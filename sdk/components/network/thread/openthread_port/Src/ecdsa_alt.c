/*
 * ecdsa_alt.c — RT583 hardware ECDSA backend for mbedTLS.
 *
 * Implements mbedtls_ecdsa_sign() and mbedtls_ecdsa_verify() using the RT583
 * ECC hardware accelerator (rt_ecc.h / rt_crypto.c), enabled by:
 *   MBEDTLS_ECDSA_SIGN_ALT
 *   MBEDTLS_ECDSA_VERIFY_ALT
 *
 * Only SECP256R1 (P-256) is supported — matches Matter and Thread usage.
 *
 * ── Endianness ──────────────────────────────────────────────────────────────
 * mbedTLS MPIs are big-endian (network byte order).
 * RT583 ECC hardware is little-endian.
 * Every 32-byte coordinate buffer is reversed before passing to hardware and
 * after reading back from hardware.
 *
 * ── ECP point access ────────────────────────────────────────────────────────
 * mbedTLS 3.x makes mbedtls_ecp_point members MBEDTLS_PRIVATE.  We access
 * them only through the public mbedtls_ecp_point_write_binary() /
 * mbedtls_mpi_write_binary() APIs to stay ABI-safe.
 *
 * ── Mutex ───────────────────────────────────────────────────────────────────
 * gfp_ecc_curve_p256_init() / gfp_ecdsa_p256_verify_init() lock the shared
 * crypto accelerator (via crypto_mutex_lock); the corresponding operation
 * functions unlock it on completion.
 */

#include <string.h>
#include "mbedtls/ecdsa.h"
#include "mbedtls/ecp.h"
#include "mbedtls/bignum.h"
#include "rt_ecc.h"

#define P256_BYTES      32
#define P256_UNCOMPRESSED_LEN  (1 + 2 * P256_BYTES)   /* 04 || X || Y = 65 bytes */

static void buf_reverse(uint8_t *dst, const uint8_t *src, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        dst[i] = src[len - 1 - i];
    }
}

/* ── mbedtls_ecdsa_sign ───────────────────────────────────────────────────── */

int mbedtls_ecdsa_sign(mbedtls_ecp_group *grp,
                       mbedtls_mpi *r, mbedtls_mpi *s,
                       const mbedtls_mpi *d,
                       const unsigned char *buf, size_t blen,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng)
{
    int ret;

    if (!grp || !r || !s || !d || !buf || !f_rng) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) {
        return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }
    if (blen > P256_BYTES) {
        blen = P256_BYTES;  /* SEC1 §4.1.3 step 5: truncate to curve order bits */
    }

    /* ── Private key d: big-endian → little-endian ── */
    uint8_t d_be[P256_BYTES] = {0};
    uint8_t d_le[P256_BYTES];
    ret = mbedtls_mpi_write_binary(d, d_be, P256_BYTES);
    if (ret != 0) return ret;
    buf_reverse(d_le, d_be, P256_BYTES);

    /* ── Hash: right-align into 32 bytes, big-endian → little-endian ── */
    uint8_t hash_be[P256_BYTES] = {0};
    uint8_t hash_le[P256_BYTES];
    memcpy(hash_be + (P256_BYTES - blen), buf, blen);
    buf_reverse(hash_le, hash_be, P256_BYTES);

    /* ── Ephemeral key k via caller's RNG, big-endian → little-endian ── */
    uint8_t k_be[P256_BYTES];
    uint8_t k_le[P256_BYTES];
    ret = f_rng(p_rng, k_be, P256_BYTES);
    if (ret != 0) return ret;
    buf_reverse(k_le, k_be, P256_BYTES);

    /* ── Hardware ECDSA sign ── */
    Signature_P256 sig;
    memset(&sig, 0, sizeof(sig));

    gfp_ecc_curve_p256_init();           /* load signing firmware, lock mutex */
    uint32_t hw_ret = gfp_ecdsa_p256_signature(
        &sig,
        (uint32_t *)hash_le,
        (uint32_t *)d_le,
        (uint32_t *)k_le);               /* releases mutex on return */

    if (hw_ret != 0) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    /* ── sig.r / sig.s: little-endian → big-endian → mbedTLS MPI ── */
    uint8_t r_be[P256_BYTES], s_be[P256_BYTES];
    buf_reverse(r_be, sig.r, P256_BYTES);
    buf_reverse(s_be, sig.s, P256_BYTES);

    ret = mbedtls_mpi_read_binary(r, r_be, P256_BYTES);
    if (ret != 0) return ret;
    ret = mbedtls_mpi_read_binary(s, s_be, P256_BYTES);
    return ret;
}

/*
 * mbedtls_ecdsa_sign_restartable — hardware has no restart concept;
 * delegate to mbedtls_ecdsa_sign() and ignore the restart context.
 */
int mbedtls_ecdsa_sign_restartable(
    mbedtls_ecp_group *grp,
    mbedtls_mpi *r, mbedtls_mpi *s,
    const mbedtls_mpi *d,
    const unsigned char *buf, size_t blen,
    int (*f_rng)(void *, unsigned char *, size_t),
    void *p_rng,
    int (*f_rng_blind)(void *, unsigned char *, size_t),
    void *p_rng_blind,
    mbedtls_ecdsa_restart_ctx *rs_ctx)
{
    (void)f_rng_blind;
    (void)p_rng_blind;
    (void)rs_ctx;
    return mbedtls_ecdsa_sign(grp, r, s, d, buf, blen, f_rng, p_rng);
}

/* ── mbedtls_ecdsa_verify ─────────────────────────────────────────────────── */

int mbedtls_ecdsa_verify(mbedtls_ecp_group *grp,
                         const unsigned char *buf, size_t blen,
                         const mbedtls_ecp_point *Q,
                         const mbedtls_mpi *r,
                         const mbedtls_mpi *s)
{
    int ret;

    if (!grp || !buf || !Q || !r || !s) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) {
        return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }
    if (blen > P256_BYTES) {
        blen = P256_BYTES;
    }

    /*
     * Export Q as uncompressed: 04 || X(32) || Y(32) = 65 bytes.
     * mbedtls_ecp_point members are MBEDTLS_PRIVATE in 3.x — use the public API.
     */
    uint8_t pub_buf[P256_UNCOMPRESSED_LEN];
    size_t pub_len = 0;
    ret = mbedtls_ecp_point_write_binary(grp, Q,
                                         MBEDTLS_ECP_PF_UNCOMPRESSED,
                                         &pub_len, pub_buf, sizeof(pub_buf));
    if (ret != 0 || pub_len != P256_UNCOMPRESSED_LEN) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }
    /* pub_buf[0] = 0x04 (uncompressed), pub_buf[1..32] = X, pub_buf[33..64] = Y */

    ECPoint_P256 pub_hw;
    buf_reverse(pub_hw.x, pub_buf + 1,              P256_BYTES);   /* X: BE→LE */
    buf_reverse(pub_hw.y, pub_buf + 1 + P256_BYTES, P256_BYTES);   /* Y: BE→LE */

    /* ── Signature (r, s): big-endian → little-endian ── */
    uint8_t r_be[P256_BYTES], s_be[P256_BYTES];
    ret = mbedtls_mpi_write_binary(r, r_be, P256_BYTES);
    if (ret != 0) return ret;
    ret = mbedtls_mpi_write_binary(s, s_be, P256_BYTES);
    if (ret != 0) return ret;

    Signature_P256 sig;
    buf_reverse(sig.r, r_be, P256_BYTES);
    buf_reverse(sig.s, s_be, P256_BYTES);

    /* ── Hash: right-align, big-endian → little-endian ── */
    uint8_t hash_be[P256_BYTES] = {0};
    uint8_t hash_le[P256_BYTES];
    memcpy(hash_be + (P256_BYTES - blen), buf, blen);
    buf_reverse(hash_le, hash_be, P256_BYTES);

    /* ── Hardware ECDSA verify ── */
    gfp_ecdsa_p256_verify_init();        /* load verify firmware, lock mutex */
    uint32_t hw_ret = gfp_ecdsa_p256_verify(&sig, (uint32_t *)hash_le, &pub_hw);
                                         /* releases mutex on return */

    return (hw_ret == 0) ? 0 : MBEDTLS_ERR_ECP_VERIFY_FAILED;
}

/*
 * mbedtls_ecdsa_verify_restartable — delegate to mbedtls_ecdsa_verify().
 */
int mbedtls_ecdsa_verify_restartable(mbedtls_ecp_group *grp,
                                     const unsigned char *buf, size_t blen,
                                     const mbedtls_ecp_point *Q,
                                     const mbedtls_mpi *r,
                                     const mbedtls_mpi *s,
                                     mbedtls_ecdsa_restart_ctx *rs_ctx)
{
    (void)rs_ctx;
    return mbedtls_ecdsa_verify(grp, buf, blen, Q, r, s);
}
