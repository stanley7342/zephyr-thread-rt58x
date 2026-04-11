/*
 * Compatibility shim: Zephyr v4.4 / mbedTLS 4.0 (TF-PSA-Crypto) moved legacy
 * PK symbols (mbedtls_pk_type_t enum including MBEDTLS_PK_ECKEY, mbedtls_pk_get_type,
 * mbedtls_pk_ec, etc.) from the public header mbedtls/pk.h to an internal header
 * mbedtls/private/pk_private.h, guarded by MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS.
 *
 * connectedhomeip (CHIPCryptoPALmbedTLSCert.cpp) still uses these symbols via
 * the public API.  This shim:
 *   1. Pulls in the real public mbedtls/pk.h (which includes private_access.h and
 *      thereby defines MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS when
 *      MBEDTLS_ALLOW_PRIVATE_ACCESS is set — as it is in the Matter GN build).
 *   2. Then includes mbedtls/private/pk_private.h to expose the private symbols.
 *
 * The mbedtls_shim/ directory is first in the GN -I path, so this file shadows
 * the installed pk.h.  #include_next skips mbedtls_shim/ and reaches the real
 * installed header, preventing recursive inclusion.
 */
#ifndef MBEDTLS_PK_SHIM_H
#define MBEDTLS_PK_SHIM_H

/* Real public header (from the tf-psa-crypto system include path). */
#include_next <mbedtls/pk.h>

/* Private legacy symbols (mbedtls_pk_type_t enum, mbedtls_pk_get_type,
 * mbedtls_pk_ec, etc.) — only emitted when MBEDTLS_DECLARE_PRIVATE_IDENTIFIERS
 * is defined, which private_access.h sets when MBEDTLS_ALLOW_PRIVATE_ACCESS is
 * defined.  The real pk.h already included private_access.h, so the macro is
 * live by the time we reach this line. */
#include <mbedtls/private/pk_private.h>

#endif /* MBEDTLS_PK_SHIM_H */
