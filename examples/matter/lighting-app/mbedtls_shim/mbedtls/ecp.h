/*
 * Compatibility shim: Zephyr v4.4 / mbedTLS 4.0 (TF-PSA-Crypto) moved the
 * ECC/ECP API from the public header mbedtls/ecp.h to an internal-use header
 * mbedtls/private/ecp.h.  connectedhomeip still uses the old public API.
 * This shim redirects #include <mbedtls/ecp.h> to the private header so that
 * the Matter GN build compiles against Zephyr's system mbedTLS without source
 * patches.
 *
 * The mbedtls/private/ directory is provided by the tf-psa-crypto builtin
 * driver include path, which is already in the GN build's -isystem paths.
 */
#pragma once
#include <mbedtls/private/ecp.h>
