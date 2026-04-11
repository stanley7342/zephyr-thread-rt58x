/*
 * Compatibility shim: Zephyr v4.4 / mbedTLS 4.0 (TF-PSA-Crypto) breaking changes.
 *
 * 1. Legacy PK symbols (mbedtls_pk_type_t enum including MBEDTLS_PK_ECKEY,
 *    mbedtls_pk_get_type, etc.) moved from the public mbedtls/pk.h to the
 *    private header mbedtls/private/pk_private.h, guarded by
 *    MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS.  Pull that header in after the real pk.h.
 *
 * 2. mbedtls_pk_ec() was removed entirely.  In 3.x it returned an
 *    mbedtls_ecp_keypair* directly from the pk_context's pk_ctx pointer.
 *    In 4.x the pk_context is PSA-backed (no pk_ctx field), so the function
 *    cannot exist as a one-liner.  Provide a static inline replacement that
 *    reconstructs the keypair from the PSA fields (ec_family, bits, pub_raw).
 *
 * The mbedtls_shim/ directory is first in the GN -I path so this file shadows
 * the installed pk.h.  #include_next skips mbedtls_shim/ and reaches the real
 * system header, preventing recursive inclusion.
 *
 * MBEDTLS_ALLOW_PRIVATE_ACCESS is defined via -D in the Matter GN build, which
 * causes private_access.h (included by the real pk.h) to define
 * MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS and expand MBEDTLS_PRIVATE(x) to just x.
 */
#ifndef MBEDTLS_PK_SHIM_H
#define MBEDTLS_PK_SHIM_H

/* 1. Real public header from the system include path. */
#include_next <mbedtls/pk.h>

/* 2. Private legacy symbols guarded by MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS. */
#include <mbedtls/private/pk_private.h>

/* 3. ECP private header — needed for mbedtls_ecp_keypair and related functions
 *    used by the mbedtls_pk_ec() replacement below. */
#include <mbedtls/private/ecp.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * mbedtls_pk_ec() replacement for TF-PSA-Crypto 4.x.
 *
 * The 3.x implementation was:
 *   return (mbedtls_ecp_keypair *)(pk.pk_ctx);
 * In 4.x there is no pk_ctx.  Instead, the context stores:
 *   - pub_raw[]:    raw public key in PSA format (uncompressed point for EC)
 *   - pub_raw_len:  length in bytes
 *   - ec_family:    PSA EC family (e.g. PSA_ECC_FAMILY_SECP_R1)
 *   - bits:         key size in bits
 *
 * We rebuild an mbedtls_ecp_keypair from those fields on each call.
 * A static local is safe here: connectedhomeip only reads the result
 * (group id + point coordinates) within the same call stack.
 *
 * Returns NULL if the curve family/bits are unrecognised or if loading fails.
 */
static inline mbedtls_ecp_group_id mbedtls_pk_ec_psa_to_grp(
        psa_ecc_family_t family, size_t bits)
{
    /* Each case is guarded by the corresponding MBEDTLS_ECP_DP_xxx_ENABLED
     * macro so that curves disabled in the build config are silently skipped
     * rather than causing undeclared-identifier errors. */
    switch (family) {
        case PSA_ECC_FAMILY_SECP_R1:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_SECP192R1_ENABLED)
                case 192: return MBEDTLS_ECP_DP_SECP192R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP224R1_ENABLED)
                case 224: return MBEDTLS_ECP_DP_SECP224R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP256R1_ENABLED)
                case 256: return MBEDTLS_ECP_DP_SECP256R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP384R1_ENABLED)
                case 384: return MBEDTLS_ECP_DP_SECP384R1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP521R1_ENABLED)
                case 521: return MBEDTLS_ECP_DP_SECP521R1;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        case PSA_ECC_FAMILY_SECP_K1:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_SECP192K1_ENABLED)
                case 192: return MBEDTLS_ECP_DP_SECP192K1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP224K1_ENABLED)
                case 224: return MBEDTLS_ECP_DP_SECP224K1;
#endif
#if defined(MBEDTLS_ECP_DP_SECP256K1_ENABLED)
                case 256: return MBEDTLS_ECP_DP_SECP256K1;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        case PSA_ECC_FAMILY_BRAINPOOL_P_R1:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_BP256R1_ENABLED)
                case 256: return MBEDTLS_ECP_DP_BP256R1;
#endif
#if defined(MBEDTLS_ECP_DP_BP384R1_ENABLED)
                case 384: return MBEDTLS_ECP_DP_BP384R1;
#endif
#if defined(MBEDTLS_ECP_DP_BP512R1_ENABLED)
                case 512: return MBEDTLS_ECP_DP_BP512R1;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        case PSA_ECC_FAMILY_MONTGOMERY:
            switch (bits) {
#if defined(MBEDTLS_ECP_DP_CURVE25519_ENABLED)
                case 255: return MBEDTLS_ECP_DP_CURVE25519;
#endif
#if defined(MBEDTLS_ECP_DP_CURVE448_ENABLED)
                case 448: return MBEDTLS_ECP_DP_CURVE448;
#endif
                default:  return MBEDTLS_ECP_DP_NONE;
            }
        default:
            return MBEDTLS_ECP_DP_NONE;
    }
}

static inline mbedtls_ecp_keypair *mbedtls_pk_ec(mbedtls_pk_context ctx)
{
    static mbedtls_ecp_keypair kp;

    /* Free any state from a previous call before reloading. */
    mbedtls_ecp_keypair_free(&kp);
    mbedtls_ecp_keypair_init(&kp);

    mbedtls_ecp_group_id grp_id = mbedtls_pk_ec_psa_to_grp(
            ctx.MBEDTLS_PRIVATE(ec_family),
            ctx.MBEDTLS_PRIVATE(bits));
    if (grp_id == MBEDTLS_ECP_DP_NONE)
        return NULL;

    if (mbedtls_ecp_group_load(&kp.MBEDTLS_PRIVATE(grp), grp_id) != 0)
        return NULL;

    /* pub_raw holds the PSA public key export: uncompressed point (04||X||Y)
     * for short-Weierstrass curves — identical to MBEDTLS_ECP_PF_UNCOMPRESSED. */
    if (mbedtls_ecp_point_read_binary(
                &kp.MBEDTLS_PRIVATE(grp),
                &kp.MBEDTLS_PRIVATE(Q),
                ctx.MBEDTLS_PRIVATE(pub_raw),
                ctx.MBEDTLS_PRIVATE(pub_raw_len)) != 0)
        return NULL;

    return &kp;
}

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_PK_SHIM_H */
