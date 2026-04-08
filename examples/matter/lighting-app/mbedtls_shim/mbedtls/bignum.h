/*
 * Compatibility shim: Zephyr v4.4 / mbedTLS 4.0 (TF-PSA-Crypto) moved
 * mbedtls/bignum.h to an internal-use header mbedtls/private/bignum.h.
 * connectedhomeip still uses the old public API.
 * See ecp.h for full explanation.
 */
#pragma once
#include <mbedtls/private/bignum.h>
