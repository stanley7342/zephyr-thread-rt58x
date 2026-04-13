/*
 * ecdh_alt.c — RT583 hardware ECDH backend for mbedTLS.
 *
 * Implements mbedtls_ecdh_gen_public() and mbedtls_ecdh_compute_shared()
 * using the RT583 ECC hardware accelerator (rt_ecc.h), enabled by:
 *   MBEDTLS_ECDH_GEN_PUBLIC_ALT
 *   MBEDTLS_ECDH_COMPUTE_SHARED_ALT
 *
 * Only SECP256R1 (P-256) is supported.
 *
 * ── Endianness ──────────────────────────────────────────────────────────────
 * mbedTLS MPIs/points are big-endian; RT583 hardware is little-endian.
 * All 32-byte buffers are reversed before passing to hardware and after
 * reading back from hardware.
 *
 * ── Generator point ─────────────────────────────────────────────────────────
 * RT583 SDK exports `Curve_Gx_p256` as a little-endian ECPoint_P256 containing
 * the standard P-256 base point G.  We use it directly for key generation.
 *
 * ── ECP point access ────────────────────────────────────────────────────────
 * mbedTLS 3.x makes mbedtls_ecp_point members MBEDTLS_PRIVATE.  We access
 * them only through the public mbedtls_ecp_point_write_binary() /
 * mbedtls_ecp_point_read_binary() APIs.
 *
 * ── Mutex ───────────────────────────────────────────────────────────────────
 * gfp_ecc_curve_p256_init() locks the shared accelerator (crypto_mutex_lock);
 * gfp_point_p256_mult() unlocks it on completion.
 */

#include <string.h>
#include "mbedtls/ecdh.h"
#include "mbedtls/ecp.h"
#include "mbedtls/bignum.h"
#include "rt_ecc.h"

#define P256_BYTES             32
#define P256_UNCOMPRESSED_LEN  (1 + 2 * P256_BYTES)   /* 04 || X || Y = 65 bytes */

static void buf_reverse(uint8_t *dst, const uint8_t *src, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        dst[i] = src[len - 1 - i];
    }
}

/* ── mbedtls_ecdh_gen_public ─────────────────────────────────────────────── */
/*
 * Generate an ephemeral P-256 key pair:
 *   d  = random 256-bit private scalar (via f_rng)
 *   Q  = d * G  (public key, stored as mbedtls_ecp_point)
 */
int mbedtls_ecdh_gen_public(mbedtls_ecp_group *grp,
                             mbedtls_mpi *d,
                             mbedtls_ecp_point *Q,
                             int (*f_rng)(void *, unsigned char *, size_t),
                             void *p_rng)
{
    int ret;

    if (!grp || !d || !Q || !f_rng) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) {
        return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    /* ── Generate random private scalar d ── */
    uint8_t d_be[P256_BYTES];
    ret = f_rng(p_rng, d_be, P256_BYTES);
    if (ret != 0) return ret;

    /* Store d as a big-endian MPI */
    ret = mbedtls_mpi_read_binary(d, d_be, P256_BYTES);
    if (ret != 0) return ret;

    /* Convert d to little-endian for RT583 */
    uint8_t d_le[P256_BYTES];
    buf_reverse(d_le, d_be, P256_BYTES);

    /*
     * Compute Q = d * G using RT583 hardware.
     * Curve_Gx_p256 is the P-256 base point in little-endian ECPoint_P256 format
     * as provided by the RT583 SDK.
     *
     * gfp_ecc_curve_p256_init() — loads firmware, locks crypto mutex
     * gfp_point_p256_mult()     — performs d*G, releases mutex on return
     */
    ECPoint_P256 Q_le;
    gfp_ecc_curve_p256_init();
    uint32_t hw_ret = gfp_point_p256_mult(
        &Q_le,
        (ECPoint_P256 *)&Curve_Gx_p256,
        (uint32_t *)d_le);

    if (hw_ret != 0) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    /*
     * Q_le.x / Q_le.y are little-endian 32-byte coordinates.
     * Assemble the uncompressed point representation (04 || X_be || Y_be)
     * and import it into the mbedTLS ECP point using the public API.
     */
    uint8_t pub[P256_UNCOMPRESSED_LEN];
    pub[0] = 0x04;                                          /* uncompressed prefix */
    buf_reverse(pub + 1,              Q_le.x, P256_BYTES);  /* X: LE→BE */
    buf_reverse(pub + 1 + P256_BYTES, Q_le.y, P256_BYTES);  /* Y: LE→BE */

    ret = mbedtls_ecp_point_read_binary(grp, Q, pub, P256_UNCOMPRESSED_LEN);
    return ret;
}

/* ── mbedtls_ecdh_compute_shared ─────────────────────────────────────────── */
/*
 * Compute the ECDH shared secret:
 *   z  = (d * Q_peer).x   (x-coordinate of the scalar multiply result)
 *
 * d       = our private scalar (mbedTLS MPI, big-endian)
 * Q_peer  = peer's public key (mbedTLS ECP point)
 * z       = shared secret output (mbedTLS MPI, big-endian)
 */
int mbedtls_ecdh_compute_shared(mbedtls_ecp_group *grp,
                                 mbedtls_mpi *z,
                                 const mbedtls_ecp_point *Q,
                                 const mbedtls_mpi *d,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng)
{
    int ret;
    (void)f_rng;
    (void)p_rng;

    if (!grp || !z || !Q || !d) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1) {
        return MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    /*
     * Export peer's public key as uncompressed: 04 || X(32) || Y(32).
     * mbedtls_ecp_point_write_binary() avoids touching MBEDTLS_PRIVATE members.
     */
    uint8_t pub[P256_UNCOMPRESSED_LEN];
    size_t pub_len = 0;
    ret = mbedtls_ecp_point_write_binary(grp, Q,
                                         MBEDTLS_ECP_PF_UNCOMPRESSED,
                                         &pub_len, pub, sizeof(pub));
    if (ret != 0 || pub_len != P256_UNCOMPRESSED_LEN) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    /* pub[1..32] = X (BE), pub[33..64] = Y (BE) → convert to LE for RT583 */
    ECPoint_P256 peer_le;
    buf_reverse(peer_le.x, pub + 1,              P256_BYTES);
    buf_reverse(peer_le.y, pub + 1 + P256_BYTES, P256_BYTES);

    /* ── Private key d: big-endian → little-endian ── */
    uint8_t d_be[P256_BYTES];
    ret = mbedtls_mpi_write_binary(d, d_be, P256_BYTES);
    if (ret != 0) return ret;

    uint8_t d_le[P256_BYTES];
    buf_reverse(d_le, d_be, P256_BYTES);

    /* ── Compute shared = d * Q_peer using hardware ── */
    ECPoint_P256 shared_le;
    gfp_ecc_curve_p256_init();
    uint32_t hw_ret = gfp_point_p256_mult(
        &shared_le,
        &peer_le,
        (uint32_t *)d_le);

    if (hw_ret != 0) {
        return MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    /* ── z = x-coordinate of result (shared secret) — LE→BE→MPI ── */
    uint8_t z_be[P256_BYTES];
    buf_reverse(z_be, shared_le.x, P256_BYTES);

    ret = mbedtls_mpi_read_binary(z, z_be, P256_BYTES);
    return ret;
}
