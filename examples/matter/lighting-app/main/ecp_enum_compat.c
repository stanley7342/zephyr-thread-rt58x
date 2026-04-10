/*
 * ECP enum compatibility wrapper for RT583 Matter + OpenThread coexistence.
 *
 * Root cause:
 *   OT bundles its own mbedTLS 2.x (compiled into libapp.a).
 *   With --allow-multiple-definition OT's mbedtls_ecp_group_load wins.
 *   TF-PSA-Crypto (mbedTLS 4.0) reordered the mbedtls_ecp_group_id enum:
 *     TF-PSA-Crypto:  NONE=0, SECP256R1=1, SECP384R1=2, SECP521R1=3, ...
 *     mbedTLS 2.x:    NONE=0, SECP192R1=1, SECP224R1=2, SECP256R1=3, ...
 *   CHIP's Spake2p is compiled against TF-PSA-Crypto headers and calls
 *   mbedtls_ecp_group_load(1) meaning SECP256R1, but OT's linked function
 *   interprets value 1 as SECP192R1 (not enabled) → FEATURE_UNAVAILABLE.
 *
 * Fix:
 *   Wrap mbedtls_ecp_group_load via GNU ld --wrap. Translate value 1
 *   (TF-PSA-Crypto SECP256R1) to value 3 (mbedTLS 2.x SECP256R1) before
 *   forwarding to OT's implementation (__real_mbedtls_ecp_group_load).
 *   Calls from OT's own code use value 3 → pass through unchanged.
 *   No other ECP functions need wrapping: they take a pre-loaded
 *   mbedtls_ecp_group whose struct layout is identical in both versions.
 *
 * Enabled by:  zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_group_load)
 *              target_sources(app PRIVATE main/ecp_enum_compat.c)
 */

/* Avoid including any mbedTLS headers — use opaque types to prevent
 * header-version conflicts in this translation unit. */

/* mbedtls_ecp_group_id enum values we care about:
 *   TF-PSA-Crypto SECP256R1 = 1
 *   mbedTLS 2.x   SECP256R1 = 3
 */
#define TF_PSA_CRYPTO_SECP256R1  1
#define OT_MBEDTLS_SECP256R1     3

/* The real (OT's) mbedtls_ecp_group_load — declared opaquely. */
int __real_mbedtls_ecp_group_load(void *grp, int id);

int __wrap_mbedtls_ecp_group_load(void *grp, int id)
{
    if (id == TF_PSA_CRYPTO_SECP256R1) {
        id = OT_MBEDTLS_SECP256R1;
    }
    return __real_mbedtls_ecp_group_load(grp, id);
}
