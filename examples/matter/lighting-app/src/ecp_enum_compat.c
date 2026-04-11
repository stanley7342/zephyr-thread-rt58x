/*
 * ECP enum compatibility wrapper for RT583 Matter + OpenThread coexistence.
 *
 * Root cause:
 *   OT bundles its own mbedTLS 3.6.0 (compiled from source into libapp.a).
 *   With --allow-multiple-definition OT's mbedtls_ecp_* functions win.
 *   TF-PSA-Crypto (mbedTLS 4.0) reordered the mbedtls_ecp_group_id enum:
 *     TF-PSA-Crypto:  NONE=0, SECP256R1=1, SECP384R1=2, SECP521R1=3, ...
 *     mbedTLS 3.6.0:  NONE=0, SECP192R1=1, SECP224R1=2, SECP256R1=3, ...
 *   CHIP's Spake2p is compiled against TF-PSA-Crypto headers and calls
 *   mbedtls_ecp_group_load(1) meaning SECP256R1, but OT's linked function
 *   interprets value 1 as SECP192R1 (not enabled) → FEATURE_UNAVAILABLE.
 *
 * Fix:
 *   Wrap mbedtls_ecp_group_load via GNU ld --wrap. Translate value 1
 *   (TF-PSA-Crypto SECP256R1) to value 3 (mbedTLS 3.6.0 SECP256R1) before
 *   forwarding to OT's implementation (__real_mbedtls_ecp_group_load).
 *   Calls from OT's own code use value 3 → pass through unchanged.
 *
 * Diagnostics:
 *   mbedtls_ecp_muladd and mbedtls_ecp_mul are also wrapped to log any
 *   non-zero return value so the exact PASE failure point can be identified.
 *   Remove the diagnostic wrappers once the root cause is found.
 *
 * Enabled by:  zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_group_load)
 *              zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_muladd)
 *              zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_mul)
 *              target_sources(app PRIVATE main/ecp_enum_compat.c)
 */

#include <zephyr/sys/printk.h>

/* Avoid including any mbedTLS headers — use opaque types to prevent
 * header-version conflicts in this translation unit. */

/* mbedtls_ecp_group_id enum values we care about:
 *   TF-PSA-Crypto SECP256R1 = 1
 *   mbedTLS 3.6.0 SECP256R1 = 3  (NONE=0, SECP192R1=1, SECP224R1=2, SECP256R1=3)
 */
#define TF_PSA_CRYPTO_SECP256R1  1
#define OT_MBEDTLS_SECP256R1     3

/* ── mbedtls_ecp_group_load ─────────────────────────────────────────────────*/
int __real_mbedtls_ecp_group_load(void *grp, int id);

int __wrap_mbedtls_ecp_group_load(void *grp, int id)
{
    int translated = id;
    if (id == TF_PSA_CRYPTO_SECP256R1) {
        translated = OT_MBEDTLS_SECP256R1;
    }
    int ret = __real_mbedtls_ecp_group_load(grp, translated);
    if (ret != 0) {
        printk("[ECP] group_load(id=%d→%d) failed: -0x%04X\n",
               id, translated, (unsigned)(-ret));
    }
    return ret;
}

/* ── mbedtls_ecp_muladd (SPAKE2+ PointAddMul) ───────────────────────────── */
int __real_mbedtls_ecp_muladd(void *grp, void *R,
                               const void *m, const void *P,
                               const void *n, const void *Q);

int __wrap_mbedtls_ecp_muladd(void *grp, void *R,
                               const void *m, const void *P,
                               const void *n, const void *Q)
{
    int ret = __real_mbedtls_ecp_muladd(grp, R, m, P, n, Q);
    if (ret != 0) {
        printk("[ECP] mbedtls_ecp_muladd failed: -0x%04X\n", (unsigned)(-ret));
    }
    return ret;
}

/* ── mbedtls_ecp_mul (SPAKE2+ PointMul with blinding) ───────────────────── */
typedef int (*ecp_rng_fn)(void *, unsigned char *, unsigned int);

int __real_mbedtls_ecp_mul(void *grp, void *R,
                            const void *m, const void *P,
                            ecp_rng_fn f_rng, void *p_rng);

int __wrap_mbedtls_ecp_mul(void *grp, void *R,
                            const void *m, const void *P,
                            ecp_rng_fn f_rng, void *p_rng)
{
    int ret = __real_mbedtls_ecp_mul(grp, R, m, P, f_rng, p_rng);
    if (ret != 0) {
        printk("[ECP] mbedtls_ecp_mul failed: -0x%04X\n", (unsigned)(-ret));
    }
    return ret;
}
