# P2: psa-arch-tests Crypto Suite — rt584 Results

**Date**: 2026-04-26
**Build**: `examples/psa_arch_tests` on `rt584_evb`
**Target**: ARM PSA Architecture Test Suite v1.8 — Crypto suite (1.1.0_testsuite.db, 64 tests)
**Backend**: mbedTLS PSA Crypto without TF-M (in-process, no isolation — rt584 silicon doesn't support SAU per [T0_ns_alias_survey.md](T0_ns_alias_survey.md))

## Summary

```
TOTAL TESTS  : 64
TOTAL PASSED : 56  (87.5%)
TOTAL FAILED : 8
TOTAL SKIPPED: 0
TOTAL SIM ERROR: 0
```

Image size: FLASH 300 KB / RAM 92 KB (rt584 has 2 MB flash + 192 KB RAM, plenty of headroom).

## Pass / Fail breakdown

### Passed 56 (samples)
- Hash APIs全套 (compute, compare, setup, update, verify, finish, abort, clone) — SHA-1/224/256/384/512
- Key derivation 全套 (setup, input_bytes, input_key, output_bytes, output_key, key_agreement, capacity, abort, input_integer)
- AEAD 全套 (encrypt, decrypt, encrypt_setup, decrypt_setup, generate_nonce, set_nonce, set_lengths, update_ad, update, finish, abort, verify) — CCM, GCM, ChaCha20-Poly1305
- MAC 全套 (sign_setup, verify_setup, update, sign_finish, verify_finish, compute, verify, sign_multipart, verify_multipart, abort) — HMAC-SHA224/256/512, CMAC-AES128
- Cipher 全套 (generate_iv, set_iv, update, finish, abort, encrypt, decrypt) — AES-CBC/CTR, ChaCha20
- Asymmetric (sign_hash, verify_hash, sign_message, verify_message, asymmetric_encrypt, asymmetric_decrypt) — RSA-2048 PKCS1V15/OAEP, ECDSA P-256 SHA-256
- Random (psa_generate_random)
- Key attributes set/get
- Raw key agreement (ECDH P-256, P-384)

### Failed 8 (root cause identical)

| Test # | Name | Failed at | Root cause |
|--------|------|-----------|------------|
| 202 | psa_import_key | check 3 (-134) | 32-byte AES (AES-256) |
| 203 | psa_export_key | check 3 (-134) | 32-byte AES |
| 204 | psa_export_public_key | check 3 (-134) | 32-byte AES |
| 205 | psa_destroy_key | check 3 (-134) | 32-byte AES |
| 216 | psa_generate_key | check 3 (-134) | 32-byte AES |
| 232 | psa_cipher_encrypt_setup | check 3 (-134) | 32-byte AES |
| 233 | psa_cipher_decrypt_setup | check 3 (-134) | 32-byte AES |
| 244 | psa_copy_key | check 4 (got -134, expected -135) | mbedTLS PSA returns NOT_SUPPORTED for invalid lifetime, spec says INVALID_ARGUMENT |

7 of 8 fail at the same place: importing / using a 32-byte AES key. AES-128 (16 byte) and AES-192 (24 byte) both pass.

#### Why AES-256 fails

`-134` = `PSA_ERROR_NOT_SUPPORTED`. mbedTLS PSA in Zephyr 4.4 + tf-psa-crypto rejects AES-256 key import with NOT_SUPPORTED. Not controllable via the standard `CONFIG_MBEDTLS_AES_*` or `CONFIG_PSA_WANT_*` Kconfig options as-shipped (no `MBEDTLS_AES_ONLY_128_BIT_KEY_LENGTH` exposed; no per-key-size selector). Likely root in the auto-generated `tf_psa_crypto_config.c` or the static-key-slot buffer sizing of the platform-built mbedTLS — needs deeper investigation if we ever want 100% pass.

#### Test 244 nuance

Test expects `PSA_ERROR_INVALID_ARGUMENT` (-135) when a key is copied with an invalid lifetime; mbedTLS returns `PSA_ERROR_NOT_SUPPORTED` (-134). This is an **implementation-vs-spec disagreement**, not a configuration issue — likely fixed by mbedTLS upstream after the spec version we built against. Could be addressed by setting `-DSPEC_VERSION=1.0.1` instead of latest.

## What this delivers (PSA Cert L1 portfolio)

The 56 / 64 pass record on a non-TZ Cortex-M33 target, with full ARM-official test framework integrated, is a credible "Functional API" demonstration. PSA Certified Level 1 self-assessment lets you cite this as evidence for the **Cryptographic Services** security goal (item 10 of 10 in the SG list).

Specifically:
- **Goal 10 (Cryptographic Services)**: ✅ Demonstrated via 56 PASS in independent compliance tests
- **Goal 1 (Unique Identification)**: rt584 OTP UID readable, but no Zephyr API yet
- **Goal 2 (Lifecycle)**: ❌ Not addressed (no fuse-locked production state)
- **Goal 3 (Attestation)**: ❌ Not yet — needs PSA Initial Attestation + token impl
- **Goal 4 (Secure Boot)**: ✅ MCUboot signature verify
- **Goal 5 (Secure Update)**: ✅ MCUboot Direct-XIP
- **Goal 6 (Anti-rollback)**: ❌ MCUboot security counter not configured
- **Goal 7 (Isolation)**: ❌ Impossible on rt584 silicon (no SAU)
- **Goal 8 (Interaction)**: N/A (no S/NS boundary)
- **Goal 9 (Secure Storage)**: ⚠️ PSA ITS available via mbedTLS but no NV backing

## Reproduce

```powershell
cd D:\matter\zephyr-thread-rt58x
. .\env.ps1
west build --sysbuild -p always -b rt584_evb examples/psa_arch_tests -d build/psa_at584
west flash -d build/psa_at584
# UART monitor at 115200 8N1 — full run takes ~90 seconds
```

## Follow-ups (low priority)

- [ ] Investigate AES-256 NOT_SUPPORTED root in tf-psa-crypto module — try `-DPSA_WANT_AES_KEY_BITS_256=1` if such a switch exists, or expose mbedtls_psa key-size config
- [ ] Try `SPEC_VERSION=1.0.1` in CMakeLists.txt, see if test 244 passes
- [ ] Add ITS suite (separate `prj.conf` with `STORAGE` define; needs flash-backed key store impl)
- [ ] Add Initial Attestation suite (needs attestation key + EAT token generator)
