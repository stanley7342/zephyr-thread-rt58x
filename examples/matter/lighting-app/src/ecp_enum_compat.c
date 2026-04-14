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
 * Fix 1 — mbedtls_ecp_group_load:
 *   Wrap via GNU ld --wrap.  Translate value 1 (TF-PSA-Crypto SECP256R1)
 *   to value 3 (mbedTLS 3.6.0 SECP256R1) before forwarding to OT's real
 *   implementation.  Calls from OT's own code use value 3 → unchanged.
 *
 * Fix 2 — psa_key_derivation_setup:
 *   A linker resolution bug causes psa_key_derivation_setup to call the
 *   wrong function, zeroing operation->alg and operation->capacity.
 *   The wrap restores both fields when the bug is detected.
 *   (Root cause was also fixed in tf-psa-crypto/core/psa_crypto.c with
 *   memset; this wrap remains as a safety net.)
 *
 * Enabled by:
 *   zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_group_load)
 *   zephyr_ld_options(-Wl,--wrap=psa_key_derivation_setup)
 *   target_sources(app PRIVATE src/ecp_enum_compat.c)
 */

#include <stddef.h>

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
    int translated = (id == TF_PSA_CRYPTO_SECP256R1) ? OT_MBEDTLS_SECP256R1 : id;
    return __real_mbedtls_ecp_group_load(grp, translated);
}

/* ── psa_key_derivation_setup ────────────────────────────────────────────── */
/* psa_algorithm_t is uint32_t in PSA spec; use unsigned to avoid header pull. */
typedef int psa_status_t_compat;
typedef unsigned int psa_algorithm_t_compat;

psa_status_t_compat __real_psa_key_derivation_setup(void *operation,
                                                     psa_algorithm_t_compat alg);

psa_status_t_compat __wrap_psa_key_derivation_setup(void *operation,
                                                     psa_algorithm_t_compat alg)
{
    volatile unsigned int *op = (volatile unsigned int *)operation;
    psa_status_t_compat ret = __real_psa_key_derivation_setup(operation, alg);

    /* Safety-net fixup: if the linker resolution bug zeroed op->alg, restore it.
     * psa_key_derivation_operation_t layout (Cortex-M3, 32-bit):
     *   op[0] = alg  (offset 0)
     *   op[2] = capacity (offset 8, size_t) */
    if (ret == 0 && op[0] == 0) {
        op[0] = (unsigned int)alg;
        op[2] = 255u * 64u;  /* HKDF max capacity (SHA-512 max) */
    }
    return ret;
}
