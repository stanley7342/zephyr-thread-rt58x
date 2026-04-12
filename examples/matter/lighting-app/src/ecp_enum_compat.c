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
 *   muladd, mul, point_write_binary, and check_pubkey are wrapped to log any
 *   non-zero return value so the exact PASE failure point can be identified.
 *   Remove the diagnostic wrappers once the root cause is found.
 *
 * Enabled by:  zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_group_load)
 *              zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_muladd)
 *              zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_mul)
 *              zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_point_write_binary)
 *              zephyr_ld_options(-Wl,--wrap=mbedtls_ecp_check_pubkey)
 *              target_sources(app PRIVATE src/ecp_enum_compat.c)
 */

#include <zephyr/sys/printk.h>
#include <stddef.h>  /* size_t */

/* Avoid including any mbedTLS headers — use opaque types to prevent
 * header-version conflicts in this translation unit. */

/* Globals used by psa_hash_update / psa_key_derivation_setup / output_bytes
 * to gate per-update logging during HKDF only. */
static int s_hkdf_active = 0;
static int s_hkdf_update_count = 0;

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
    /* Always print — confirms the --wrap mechanism is active */
    printk("[ECP] group_load(id=%d->%d) ret=%d\n", id, translated, ret);
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

/* ── mbedtls_ecp_point_write_binary (SPAKE2+ PointWrite) ────────────────── */
int __real_mbedtls_ecp_point_write_binary(const void *grp, const void *P,
                                          int format, size_t *olen,
                                          unsigned char *buf, size_t buflen);

int __wrap_mbedtls_ecp_point_write_binary(const void *grp, const void *P,
                                          int format, size_t *olen,
                                          unsigned char *buf, size_t buflen)
{
    int ret = __real_mbedtls_ecp_point_write_binary(grp, P, format, olen, buf, buflen);
    if (ret != 0) {
        printk("[ECP] ecp_point_write_binary failed: -0x%04X (fmt=%d buflen=%u)\n",
               (unsigned)(-ret), format, (unsigned)buflen);
    }
    return ret;
}

/* ── mbedtls_ecp_point_read_binary (SPAKE2+ PointLoad) ──────────────────── */
int __real_mbedtls_ecp_point_read_binary(const void *grp, void *P,
                                         const unsigned char *buf, size_t ilen);

int __wrap_mbedtls_ecp_point_read_binary(const void *grp, void *P,
                                         const unsigned char *buf, size_t ilen)
{
    int ret = __real_mbedtls_ecp_point_read_binary(grp, P, buf, ilen);
    if (ret != 0) {
        printk("[ECP] ecp_point_read_binary failed: -0x%04X (ilen=%u)\n",
               (unsigned)(-ret), (unsigned)ilen);
    }
    return ret;
}

/* ── mbedtls_ecp_check_pubkey (SPAKE2+ PointIsValid) ────────────────────── */
int __real_mbedtls_ecp_check_pubkey(const void *grp, const void *pt);

int __wrap_mbedtls_ecp_check_pubkey(const void *grp, const void *pt)
{
    int ret = __real_mbedtls_ecp_check_pubkey(grp, pt);
    if (ret != 0) {
        printk("[ECP] ecp_check_pubkey failed: -0x%04X\n", (unsigned)(-ret));
    }
    return ret;
}

/* ── psa_hash_finish (SPAKE2+ transcript HashFinalize) ──────────────────── */
/* psa_status_t == int32_t; avoid psa/crypto.h to prevent header conflicts. */
typedef int psa_status_t_compat;
typedef void * psa_hash_operation_t_ptr;

psa_status_t_compat __real_psa_hash_finish(void *operation,
                                            unsigned char *hash,
                                            unsigned int hash_size,
                                            unsigned int *hash_length);

psa_status_t_compat __wrap_psa_hash_finish(void *operation,
                                            unsigned char *hash,
                                            unsigned int hash_size,
                                            unsigned int *hash_length)
{
    psa_status_t_compat ret = __real_psa_hash_finish(operation, hash, hash_size, hash_length);
    if (ret != 0) {
        printk("[PSA] psa_hash_finish failed: %d\n", ret);
    } else {
        unsigned int hlen = hash_length ? (unsigned)*hash_length : 0u;
        /* Dump first 8 bytes — for the transcript hash (Kae), bytes [0:16] = Ka.
         * For HKDF internal calls the values here are HMAC blocks, not Ka. */
        printk("[PSA] psa_hash_finish OK len=%u [0:8]:", hlen);
        unsigned int dump = hlen < 8u ? hlen : 8u;
        for (unsigned int i = 0; i < dump; i++) {
            printk(" %02x", hash[i]);
        }
        printk("\n");
    }
    return ret;
}

/* ── psa_key_derivation_output_bytes (SPAKE2+ KDF output) ───────────────── */
psa_status_t_compat __real_psa_key_derivation_output_bytes(void *operation,
                                                            unsigned char *output,
                                                            unsigned int output_length);

psa_status_t_compat __wrap_psa_key_derivation_output_bytes(void *operation,
                                                            unsigned char *output,
                                                            unsigned int output_length)
{
    psa_status_t_compat ret = __real_psa_key_derivation_output_bytes(operation, output, output_length);
    if (ret != 0) {
        printk("[PSA] psa_key_derivation_output_bytes failed: %d (len=%u)\n",
               ret, (unsigned)output_length);
    } else {
        /* Dump up to 32 bytes of HKDF output so we can compare Kcab with
         * what the commissioner computes from the same transcript hash. */
        unsigned int dump = output_length < 32u ? output_length : 32u;
        printk("[KDF] output_bytes OK len=%u bytes:", (unsigned)output_length);
        for (unsigned int i = 0; i < dump; i++) {
            printk(" %02x", output[i]);
        }
        printk("\n");
    }
    s_hkdf_active = 0;  /* done — stop per-update logging */
    return ret;
}

/* ── psa_hash_update (transcript InternalHash accumulation) ─────────────── */
/* During HKDF (s_hkdf_active=1) log first 16 bytes of each update to verify
 * that ipad/opad/IKM/inner_hash bytes are exactly what Python expects. */

psa_status_t_compat __real_psa_hash_update(void *operation,
                                            const unsigned char *input,
                                            unsigned int input_length);

psa_status_t_compat __wrap_psa_hash_update(void *operation,
                                            const unsigned char *input,
                                            unsigned int input_length)
{
    psa_status_t_compat ret = __real_psa_hash_update(operation, input, input_length);
    if (ret != 0) {
        printk("[PSA] psa_hash_update failed: %d (len=%u)\n", ret, (unsigned)input_length);
    } else if (s_hkdf_active) {
        /* Log first 16 bytes of each update so we can verify ipad/opad/data. */
        unsigned int dump = input_length < 16u ? input_length : 16u;
        s_hkdf_update_count++;
        printk("[KDF] hash_update#%d len=%u [0:%u]:", s_hkdf_update_count,
               (unsigned)input_length, dump);
        for (unsigned int i = 0; i < dump; i++) {
            printk(" %02x", input[i]);
        }
        printk("\n");
    }
    return ret;
}

/* ── mbedtls_mpi_write_binary (SPAKE2+ FEWrite — w0 scalar write) ────────── */
/* This is the only unmonitored call between the last successful PointWrite(V)
 * and GenerateKeys()/psa_hash_finish.  Logs on both success and failure to
 * confirm whether FEWrite(w0) executes at all. */
int __real_mbedtls_mpi_write_binary(const void *X, unsigned char *buf,
                                    unsigned int buflen);

int __wrap_mbedtls_mpi_write_binary(const void *X, unsigned char *buf,
                                    unsigned int buflen)
{
    int ret = __real_mbedtls_mpi_write_binary(X, buf, buflen);
    if (ret != 0) {
        printk("[MPI] mpi_write_binary FAILED: -0x%04X (buflen=%u)\n",
               (unsigned)(-ret), (unsigned)buflen);
    } else {
        printk("[MPI] mpi_write_binary OK buflen=%u\n", (unsigned)buflen);
    }
    return ret;
}

/* ── psa_key_derivation_setup (GenerateKeys HKDF setup) ─────────────────── */
/* psa_algorithm_t is uint32_t in PSA spec; use unsigned to avoid header pull. */
typedef unsigned int psa_algorithm_t_compat;

psa_status_t_compat __real_psa_key_derivation_setup(void *operation,
                                                     psa_algorithm_t_compat alg);

psa_status_t_compat __wrap_psa_key_derivation_setup(void *operation,
                                                     psa_algorithm_t_compat alg)
{
    /* Use volatile to guarantee fresh memory reads — no caching by compiler. */
    volatile unsigned int *op = (volatile unsigned int *)operation;

    psa_status_t_compat ret = __real_psa_key_derivation_setup(operation, alg);

    /* psa_key_derivation_operation_t layout (Cortex-M3, 32-bit):
     *   op[0] = alg        (offset  0, uint32_t)
     *   op[1] = can_output_key bitfield (offset 4)
     *   op[2] = capacity   (offset  8, size_t)
     *   op[3..] = ctx union (offset 12, 268 bytes)
     *
     * Root cause: the linker resolves the bl from psa_key_derivation_setup to
     * psa_key_derivation_setup_kdf at the WRONG address (20 bytes off, into
     * psa_mac_finalize_alg_and_key_validation.isra.0+0xb0).  That function
     * corrupts r4 (which held alg) so the subsequent
     *   str r4, [r5, #0]   ; operation->alg = alg
     * writes 0 instead of the real alg.  psa_key_derivation_set_maximum_capacity
     * also never runs, leaving capacity=0.  Both fields are fixed up here.
     *
     * ctx (hkdf->state) is already 0 — the memset() added to
     * psa_key_derivation_setup() in psa_crypto.c runs before the bad bl and
     * the wrong callee does not touch hkdf->state (confirmed: op[7]=0 in log). */
    if (ret == 0 && op[0] == 0) {
        op[0] = (unsigned int)alg;   /* restore operation->alg */
        /* HKDF max capacity = 255 × hash_len.  SHA-256: 255×32=8160.
         * Use 255×64=16320 (SHA-512 max) so this covers any HKDF variant. */
        op[2] = 255u * 64u;          /* restore operation->capacity */
        printk("[KDF] setup fixup: alg=0x%X cap=%u\n",
               (unsigned)op[0], (unsigned)op[2]);
    } else {
        printk("[KDF] setup op=%p alg=0x%X ret=%d op[0]=%X op[2]=%X\n",
               operation, (unsigned)alg, ret, (unsigned)op[0], (unsigned)op[2]);
    }
    /* Activate per-update logging for subsequent hash_update calls. */
    s_hkdf_active = 1;
    s_hkdf_update_count = 0;
    return ret;
}

/* ── psa_key_derivation_input_bytes (GenerateKeys HKDF info/salt) ─────────── */
psa_status_t_compat __real_psa_key_derivation_input_bytes(void *operation,
                                                           unsigned int step,
                                                           const unsigned char *data,
                                                           unsigned int data_length);

psa_status_t_compat __wrap_psa_key_derivation_input_bytes(void *operation,
                                                           unsigned int step,
                                                           const unsigned char *data,
                                                           unsigned int data_length)
{
    psa_status_t_compat ret = __real_psa_key_derivation_input_bytes(
        operation, step, data, data_length);
    if (ret != 0) {
        printk("[KDF] psa_key_derivation_input_bytes(step=0x%X len=%u) FAILED: %d\n",
               step, (unsigned)data_length, ret);
    }
    return ret;
}

/* ── psa_key_derivation_input_key (GenerateKeys HKDF IKM) ───────────────── */
/*
 * Root cause (resolved): PSA_KEY_DERIVATION_OPERATION_INIT only zeroes the first
 * member of the ctx union.  Stack-allocated PsaKdf::mOperation retains
 * hkdf->state = HKDF_STATE_KEYED (2) from a prior stack frame, causing
 * psa_hkdf_input(SECRET) to skip the INIT→STARTED transition and return
 * PSA_ERROR_BAD_STATE at the "state != HKDF_STATE_STARTED" check.
 *
 * Fix: memset(&operation->ctx, 0, sizeof(operation->ctx)) added to
 * psa_key_derivation_setup() in tf-psa-crypto/core/psa_crypto.c so that every
 * setup call guarantees HKDF_STATE_INIT regardless of prior stack contents.
 *
 * This wrap is kept for diagnostics only.
 */
psa_status_t_compat __real_psa_key_derivation_input_key(void *operation,
                                                         unsigned int step,
                                                         unsigned int key);

psa_status_t_compat __wrap_psa_key_derivation_input_key(void *operation,
                                                         unsigned int step,
                                                         unsigned int key)
{
    /* Print operation pointer so we can confirm it matches the setup call pointer.
     * Layout (Cortex-M3, 32-bit pointers):
     *   op[0]=alg op[1]=can_output_key(bitfield) op[2..3]=capacity(u64)
     *   op[4]=ctx.hkdf.info op[5]=ctx.hkdf.info_length
     *   op[6]=(offset_in_block|block_number|padding) op[7]=state bitfield word */
    const unsigned int *op = (const unsigned int *)operation;
    printk("[KDF] input_key pre: op=%p op[0](alg)=0x%X op[7](state)=0x%X\n",
           operation, op[0], op[7]);
    psa_status_t_compat ret = __real_psa_key_derivation_input_key(operation, step, key);
    if (ret != 0) {
        printk("[KDF] psa_key_derivation_input_key(step=0x%X key=%u) FAILED: %d\n",
               step, (unsigned)key, ret);
    }
    return ret;
}

/* ── psa_crypto_init (PSA subsystem initialization) ─────────────────────────
 * If this never logs, PSA was not initialized before input_key() is called.
 * If it returns non-zero, that explains BAD_STATE in psa_get_key_slots_initialized. */
psa_status_t_compat __real_psa_crypto_init(void);

psa_status_t_compat __wrap_psa_crypto_init(void)
{
    psa_status_t_compat ret = __real_psa_crypto_init();
    printk("[PSA] psa_crypto_init() ret=%d\n", ret);
    return ret;
}

/* ── psa_import_key (SPAKE2+ w0 key import before HKDF) ─────────────────────
 * If this returns non-zero or is never called, input_key(SECRET) would fail.
 * Logs the assigned key ID so we can cross-reference with input_key(key=...). */
psa_status_t_compat __real_psa_import_key(const void *attributes,
                                           const unsigned char *data,
                                           unsigned int data_length,
                                           unsigned int *key);

psa_status_t_compat __wrap_psa_import_key(const void *attributes,
                                           const unsigned char *data,
                                           unsigned int data_length,
                                           unsigned int *key)
{
    psa_status_t_compat ret = __real_psa_import_key(attributes, data, data_length, key);
    /* For 16-byte keys (Ka or Kcb), dump the raw bytes so we can verify the
     * HKDF output slice that was imported matches what the commissioner has. */
    if (data_length <= 32u) {
        printk("[PSA] psa_import_key(len=%u) ret=%d key=0x%08X bytes:",
               (unsigned)data_length, ret, (key && ret == 0) ? *key : 0u);
        for (unsigned int i = 0; i < data_length; i++) {
            printk(" %02x", data[i]);
        }
        printk("\n");
    } else {
        printk("[PSA] psa_import_key(len=%u) ret=%d key=0x%08X\n",
               (unsigned)data_length, ret, (key && ret == 0) ? *key : 0u);
    }
    return ret;
}

/* ── psa_generate_key (operational keypair generation for CSR) ───────────────
 * P256Keypair::Initialize (CHIPCryptoPALPSA) calls psa_generate_key for the
 * pending operational key during CSRRequest.  Log return value + key ID so we
 * can see whether generation succeeds or fails, and with what PSA status. */
psa_status_t_compat __real_psa_generate_key(const void *attributes,
                                             unsigned int *key);

psa_status_t_compat __wrap_psa_generate_key(const void *attributes,
                                             unsigned int *key)
{
    psa_status_t_compat ret = __real_psa_generate_key(attributes, key);
    printk("[PSA] psa_generate_key ret=%d key=0x%08X\n",
           ret, (key && ret == 0) ? *key : 0u);
    return ret;
}

/* ── psa_hash_setup (trace BAD_STATE source in HMAC init) ───────────────────
 * psa_key_derivation_start_hmac calls psa_driver_wrapper_mac_sign_setup which
 * calls psa_hash_setup.  If hash_op->id != 0, psa_hash_setup returns BAD_STATE.
 * Log every call that returns non-zero; log id=0 confirmation on success. */
psa_status_t_compat __real_psa_hash_setup(void *operation, unsigned int alg);

psa_status_t_compat __wrap_psa_hash_setup(void *operation, unsigned int alg)
{
    /* op->id is the first field (unsigned int) of psa_hash_operation_t */
    unsigned int id_before = ((unsigned int *)operation)[0];
    psa_status_t_compat ret = __real_psa_hash_setup(operation, alg);
    if (ret != 0) {
        printk("[HASH] psa_hash_setup(alg=0x%X) FAILED: %d (id_before=%u)\n",
               alg, ret, id_before);
    }
    return ret;
}
