/*
 * rt583_crypto_mutex.c — Zephyr mutex backend for the RT583 shared crypto accelerator.
 *
 * Both the AES driver (rt_aes.c) and ECC driver (rt_crypto.c/hosal_crypto_aes.c) call
 * crypto_mutex_lock() / crypto_mutex_unlock() when CONFIG_SUPPORT_MULTITASKING is defined.
 * They are declared as `extern void` in those files; we provide the actual Zephyr k_mutex
 * implementation here.
 *
 * The accelerator loads different firmware for AES vs ECC operations:
 *   - AES: aes_fw_init() loads AES microcode; aes_acquire/release lock/unlock via this mutex.
 *   - ECC: gfp_ecc_curve_p256_init() / gfp_ecdsa_p256_verify_init() load ECC microcode and
 *           lock this mutex; gfp_point_p256_mult/gfp_ecdsa_p256_verify unlock it on completion.
 *
 * Because both subsystems share the same mutex, concurrent requests from different threads
 * (e.g., Matter CASE and OpenThread MLE) serialise naturally.
 */

#include <zephyr/kernel.h>

static K_MUTEX_DEFINE(s_crypto_hw_mutex);

void crypto_mutex_lock(void)
{
    k_mutex_lock(&s_crypto_hw_mutex, K_FOREVER);
}

void crypto_mutex_unlock(void)
{
    k_mutex_unlock(&s_crypto_hw_mutex);
}
