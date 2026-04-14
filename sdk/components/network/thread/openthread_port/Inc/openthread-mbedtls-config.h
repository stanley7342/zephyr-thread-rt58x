/*
 *  Copyright (c) 2018, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

// Spans multiple lines to avoid being processed by unifdef
#ifndef \
    MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

#include "openthread-core-config.h"

#include <stdio.h>
#include <stdlib.h>

#include <openthread/config.h>
#include <openthread/platform/logging.h>
#include <openthread/platform/memory.h>

#define MBEDTLS_PLATFORM_SNPRINTF_MACRO snprintf

/*
 * RT583 hardware accelerator ALT hooks.
 *
 * MBEDTLS_AES_ALT         — AES ECB/CBC/CCM via RT583 AES engine (aes_alt.c)
 * MBEDTLS_ECDSA_SIGN_ALT  — ECDSA signing via RT583 ECC engine (ecdsa_alt.c)
 * MBEDTLS_ECDSA_VERIFY_ALT — ECDSA verify via RT583 ECC engine (ecdsa_alt.c)
 * MBEDTLS_ECDH_GEN_PUBLIC_ALT   — ECDH key gen via RT583 ECC (ecdh_alt.c)
 * MBEDTLS_ECDH_COMPUTE_SHARED_ALT — ECDH shared secret via RT583 ECC (ecdh_alt.c)
 *
 * AES and ECC share the same hardware accelerator.  Thread safety is provided
 * by crypto_mutex_lock/unlock (rt583_crypto_mutex.c) which serialize all
 * accesses.  CONFIG_SUPPORT_MULTITASKING enables the mutex calls in rt_aes.c
 * and rt_crypto.c; it is set in subsys/openthread/CMakeLists.txt.
 *
 * MBEDTLS_ECP_RESTARTABLE must NOT be defined — it is incompatible with ALT
 * hooks (enforced by mbedtls/check_config.h).
 */
#define MBEDTLS_AES_ALT
#define MBEDTLS_ECDSA_SIGN_ALT
#define MBEDTLS_ECDSA_VERIFY_ALT
#define MBEDTLS_ECDH_GEN_PUBLIC_ALT
#define MBEDTLS_ECDH_COMPUTE_SHARED_ALT

#define MBEDTLS_AES_C
#if (MBEDTLS_VERSION_NUMBER >= 0x03050000)
#define MBEDTLS_AES_ONLY_128_BIT_KEY_LENGTH
#endif
/* MBEDTLS_AES_ROM_TABLES not needed — we use hardware, not software S-boxes */
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C
#if (MBEDTLS_VERSION_NUMBER >= 0x03050000)
#define MBEDTLS_BLOCK_CIPHER_NO_DECRYPT
#endif
#define MBEDTLS_CCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CMAC_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_DEPRECATED_REMOVED
#define MBEDTLS_DEPRECATED_WARNING
#define MBEDTLS_ECJPAKE_C
#define MBEDTLS_ECP_C
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_NIST_OPTIM
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_HAVE_ASM
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
#define MBEDTLS_MD_C
#define MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
#define MBEDTLS_NO_PLATFORM_ENTROPY
#define MBEDTLS_OID_C
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA256_SMALLER
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_DTLS_ANTI_REPLAY
#define MBEDTLS_SSL_DTLS_HELLO_VERIFY
#define MBEDTLS_SSL_EXPORT_KEYS
#define MBEDTLS_SSL_MAX_FRAGMENT_LENGTH
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_DTLS
#define MBEDTLS_SSL_TLS_C

#if OPENTHREAD_CONFIG_BORDER_AGENT_ENABLE || OPENTHREAD_CONFIG_COMMISSIONER_ENABLE || OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
#define MBEDTLS_SSL_COOKIE_C
#define MBEDTLS_SSL_SRV_C
#endif

#if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#endif

#if OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE || OPENTHREAD_CONFIG_TLS_ENABLE
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#endif

#if OPENTHREAD_CONFIG_BLE_TCAT_ENABLE
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_GCM_C
#endif

#ifdef MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_BASE64_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#endif

#if OPENTHREAD_CONFIG_ECDSA_ENABLE
#define MBEDTLS_BASE64_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#if OPENTHREAD_CONFIG_DETERMINISTIC_ECDSA_ENABLE
#define MBEDTLS_ECDSA_DETERMINISTIC
#endif
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_PK_WRITE_C
#endif

#define MBEDTLS_MPI_WINDOW_SIZE            1 /**< Maximum windows size used. */
#define MBEDTLS_MPI_MAX_SIZE              32 /**< Maximum number of bytes for usable MPIs. */
#define MBEDTLS_ECP_MAX_BITS             256 /**< Maximum bit size of groups */
#define MBEDTLS_ECP_WINDOW_SIZE            2 /**< Maximum window size used */
#define MBEDTLS_ECP_FIXED_POINT_OPTIM      0 /**< Enable fixed-point speed-up */
#define MBEDTLS_ENTROPY_MAX_SOURCES        1 /**< Maximum number of sources supported */

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#define MBEDTLS_PLATFORM_STD_CALLOC      otPlatCAlloc /**< Default allocator to use, can be undefined */
#define MBEDTLS_PLATFORM_STD_FREE        otPlatFree /**< Default free to use, can be undefined */
#else
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#endif

#if OPENTHREAD_CONFIG_BLE_TCAT_ENABLE
#define MBEDTLS_SSL_MAX_CONTENT_LEN      2000 /**< Maxium fragment length in bytes */
#elif OPENTHREAD_CONFIG_COAP_SECURE_API_ENABLE
#define MBEDTLS_SSL_MAX_CONTENT_LEN      900 /**< Maxium fragment length in bytes */
#else
#define MBEDTLS_SSL_MAX_CONTENT_LEN      768 /**< Maxium fragment length in bytes */
#endif

#define MBEDTLS_SSL_IN_CONTENT_LEN       MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_OUT_CONTENT_LEN      MBEDTLS_SSL_MAX_CONTENT_LEN
#define MBEDTLS_SSL_CIPHERSUITES         MBEDTLS_TLS_ECJPAKE_WITH_AES_128_CCM_8

// Spans multiple lines to avoid being processed by unifdef
#if defined(\
    MBEDTLS_USER_CONFIG_FILE)
#include MBEDTLS_USER_CONFIG_FILE
#endif

#include "mbedtls/version.h"
#if (MBEDTLS_VERSION_NUMBER < 0x03000000)
    // Configuration sanity check. Done automatically in Mbed TLS >= 3.0.
    #include "mbedtls/check_config.h"
#endif

#endif /* MBEDTLS_CONFIG_H */
