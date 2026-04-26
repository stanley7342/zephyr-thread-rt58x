# Threat Model and Security Analysis (TMSA)

**Target of Evaluation (TOE)**: Zephyr-on-rt584 IoT device firmware
**Document Version**: 0.1 (draft)
**Date**: 2026-04-26
**Author**: Stanley
**Intended use**: PSA Certified Level 1 self-assessment portfolio reference

---

## Disclaimer

This TMSA is written as a **learning / portfolio exercise** for the
[zephyr-thread-rt58x](https://github.com/stanley7342/zephyr-thread-rt58x)
project. It uses real PSA Certified L1 paperwork structure but is not
submitted to a PSA Certified evaluator. Several Security Goals are
**explicitly not met** (Isolation, Lifecycle, Attestation) due to
Rafael rt584 silicon limitations documented in
[T0_ns_alias_survey.md](T0_ns_alias_survey.md); these are listed as
residual risks rather than papered over.

---

## 1. Document Information

### 1.1 Scope

This document describes the threat model and security analysis for an
IoT device built on:

- **SoC**: Rafael Microelectronics RT584 (Cortex-M33, ARMv8-M Mainline,
  64 MHz, 2 MB internal flash, 192 KB SRAM)
- **RTOS**: Zephyr 4.4-rc1
- **Bootloader**: MCUboot v2.3.0 (Direct-XIP / Single-app modes)
- **Crypto**: mbedTLS PSA Crypto via Zephyr `CONFIG_MBEDTLS_PSA_CRYPTO_C=y`
  (no TF-M; in-process, no hardware isolation)
- **Network stacks**: OpenThread FTD CLI, optional BLE peripheral
- **Application**: vendor-agnostic IoT firmware (Matter target tracked
  separately)

### 1.2 Target Audience

- Developers integrating PSA-compliant security on rt58x family
- PSA Certified L1 self-assessment evidence reviewers
- Security architects evaluating portability of PSA APIs to non-TZ
  Cortex-M33 SoCs

### 1.3 Related Documents

- [T0_ns_alias_survey.md](T0_ns_alias_survey.md) — silicon analysis explaining
  why TF-M / TrustZone isolation is not achievable on rt584
- [P2_psa_arch_tests_results.md](P2_psa_arch_tests_results.md) — PSA
  Functional API compliance test results
- Project [CLAUDE.md](../../CLAUDE.md) — build & flash workflow
- [Memory: project_rt584_bringup.md](memory pointer) — bring-up gotchas
  and root-cause notes

---

## 2. Target of Evaluation (TOE) Description

### 2.1 Product type

A small-resource IoT device firmware. Nominal use cases:

- Thread / Matter end-device (light, sensor, switch, lock)
- BLE peripheral (heart-rate sensor reference)
- Field-upgradable via OTA + MCUboot

### 2.2 TOE physical boundary

- The packaged firmware image (.signed.bin) flashed to slot0 of the
  rt584's internal flash
- The MCUboot bootloader image at flash 0x10000000–0x1000FFFF (64 KB)
- Vendor RT569 RF coprocessor firmware blob loaded by `hosal_rf_init()`
  into the COMM_SUBSYSTEM block at runtime

### 2.3 TOE logical boundary

```
┌─────────────────────────────────────────────────┐
│  Zephyr application (main, threads, OT, BLE,    │
│  Matter clusters, ...)                          │
├─────────────────────────────────────────────────┤
│  Zephyr kernel + drivers (UART, flash, GPIO)    │
├─────────────────────────────────────────────────┤
│  PSA Crypto API ─── mbedTLS PSA Crypto (no TZ)  │
│  PSA ITS API   ─── (not yet wired to NV)        │
├─────────────────────────────────────────────────┤
│  Vendor SDK (rt584_hosal, rt569 RF, OpenThread  │
│  port, BLE host, lmac15p4)                      │
├─────────────────────────────────────────────────┤
│  rt584 silicon (Cortex-M33 secure-only mode)    │
└─────────────────────────────────────────────────┘
```

There is **no Secure / Non-Secure split**. All code runs in Cortex-M33
secure state. The CPU's TrustZone hardware is present but unusable
(see [T0_ns_alias_survey.md](T0_ns_alias_survey.md)).

### 2.4 External interfaces

| Interface | Direction | Use |
|-----------|-----------|-----|
| UART0 (115200 8N1) | bidir | Console / OT CLI / debug log |
| SWD (CMSIS-DAP) | in | Programming / debug (lifecycle: open) |
| 802.15.4 radio | bidir | OpenThread / Matter |
| BLE radio | bidir | BLE peripheral |
| OTA (over Thread/BLE) | in | Firmware update |

### 2.5 Lifecycle states (planned, not enforced)

| State | Debug | Crypto | Notes |
|-------|-------|--------|-------|
| Manufacturer | open | dev keys | Factory provisioning |
| Development | open | dev keys | Engineering use |
| Production | should-lock | prod keys | **rt584 has no OTP debug-lock fuse identified** |
| Decommission | irrelevant | wipe | Out of scope |

---

## 3. Assumptions

| ID | Assumption | Justification |
|---|---|---|
| A.PHYS | Attacker has casual physical access — can power-cycle, press buttons, plug in USB-UART. No semi-pro lab tools (no decap, no FIB) | Consumer-grade product threat model |
| A.NETWORK | Attacker controls the network on which the device communicates — can MITM, replay, jam | Standard IoT threat assumption |
| A.NO_SCA | No side-channel analysis (power / EM) — ruled out by L1 attacker capability | PSA Certified L1 boundary |
| A.NO_FI | No fault injection (voltage / clock / laser glitch) | PSA Certified L1 boundary |
| A.OOB_PROVISIONING | First-time secrets (device cert, root key) provisioned over a trusted OOB channel (factory tooling, JTAG-protected flash write) | Production process, not firmware |
| A.SECURE_DEV | Build infrastructure is not compromised — signing key not exposed | Assumed |

---

## 4. Assets

Items the firmware is responsible for protecting.

| ID | Asset | Confidentiality | Integrity | Availability |
|---|---|---|---|---|
| AS.FW | Application firmware image (slot0 / slot1) | Low (anyone with debugger or external flash reader can dump) | **High** (signature-verified by MCUboot) | High |
| AS.BOOT | MCUboot image | Low | **High** (must be HW-locked in production) | High |
| AS.NETWORK_CRED | Thread network key, BLE LTK | **High** | High | Medium |
| AS.MATTER_DAC | Matter Device Attestation Certificate + private key | **Critical** | High | Medium |
| AS.OTA_KEY | MCUboot signing public key | Low | **Critical** (must not be replaceable) | N/A |
| AS.UID | Per-device unique ID (from rt584 OTP) | Low (public identifier) | High (hardware-immutable) | High |
| AS.ENT | Entropy / randomness source | High | High | High |
| AS.LOG | Diagnostic log on UART | Low | Low | Low |

---

## 5. Threat Model

Threat enumeration follows the STRIDE pattern, restricted to L1 attacker
capability (per A.PHYS, A.NO_SCA, A.NO_FI).

### 5.1 Threat catalog

| ID | Threat | STRIDE | Affected asset | Severity (L1) |
|---|---|---|---|---|
| T.FW_MOD | Attacker modifies firmware in flash and reboots | Tampering | AS.FW | High |
| T.UNSIGNED_FW | Attacker installs unsigned firmware via OTA or ISP | Tampering | AS.FW | High |
| T.ROLLBACK | Attacker installs older signed firmware to exploit known CVE | Tampering | AS.FW | Medium |
| T.DEBUG_DUMP | Attacker connects SWD probe in production and dumps secrets | Information disclosure | AS.NETWORK_CRED, AS.MATTER_DAC | High |
| T.FLASH_DUMP | Attacker desolders flash chip / probes flash bus and reads contents | Information disclosure | AS.NETWORK_CRED, AS.MATTER_DAC | Medium (rt584 has internal flash, no external chip — but on-chip JTAG read still possible) |
| T.UART_LEAK | Diagnostic logs print secret / partial secret on UART | Information disclosure | AS.NETWORK_CRED | Medium |
| T.OTA_REPLAY | Attacker replays old OTA bundle | Tampering | AS.FW | Medium |
| T.MITM_PROVISION | Attacker intercepts factory provisioning UART traffic | Information disclosure | AS.MATTER_DAC | Out of scope (covered by A.OOB_PROVISIONING) |
| T.WEAK_RNG | Entropy source is weak → predictable session keys | Spoofing / Info disclosure | AS.ENT, AS.NETWORK_CRED | **High (current state — see §7)** |
| T.NETWORK_REPLAY | Attacker replays Thread MLE / Matter session on the network | Spoofing | AS.NETWORK_CRED | Mitigated by Thread/Matter protocol counters |
| T.UNAUTH_ACCESS | Attacker uses Matter / OT CLI shell to issue commands | Elevation of privilege | AS.FW | Low (CLI behind protocol auth) |
| T.RUNTIME_EXPLOIT | Buffer overflow / use-after-free in firmware → arbitrary code execution | Tampering | AS.FW | Medium (mitigated by Zephyr stack canaries; not by W^X — rt584 has no MPU isolation per §6) |
| T.SUPPLY_CHAIN | Compromised vendor SDK / mbedTLS / Zephyr drops a backdoor | Tampering | AS.FW | Low (mitigated by checksumming RF firmware blob — see crc32 in main.c) |

### 5.2 Threats explicitly out of scope (require L2+ attacker)

| Threat | Reason |
|---|---|
| Side-channel power analysis | A.NO_SCA — beyond L1 |
| Fault injection (clock / voltage / laser) | A.NO_FI — beyond L1 |
| Decapsulation / FIB | Beyond L1 |
| Compromised JTAG dongle on production line | A.OOB_PROVISIONING |

---

## 6. Security Objectives

### 6.1 Per-threat mitigation table

| Threat | Objective | Implementation evidence |
|---|---|---|
| T.FW_MOD, T.UNSIGNED_FW | O.SECURE_BOOT — only signature-verified firmware runs | MCUboot `BOOT_SIGNATURE_TYPE_NONE=y` (no signing in current dev tree); production must switch to ED25519 / RSA / ECDSA |
| T.ROLLBACK | O.ANTI_ROLLBACK — accept only firmware with security counter ≥ on-device counter | MCUboot supports `MCUBOOT_HW_DOWNGRADE_PREVENTION` — **not yet enabled** |
| T.DEBUG_DUMP | O.DEBUG_LOCK — JTAG/SWD disabled in production lifecycle | **rt584 OTP debug-lock fuse not identified** — see residual risks §9 |
| T.FLASH_DUMP | O.FLASH_PROT — flash readout protection in production | rt584 internal flash readable via SWD when debug enabled — depends on O.DEBUG_LOCK |
| T.UART_LEAK | O.NO_SECRET_PRINT — no asset-tier value printed on UART | Manual code review; no automated check yet |
| T.OTA_REPLAY | O.ANTI_ROLLBACK | Same as above |
| T.WEAK_RNG | O.ENTROPY — entropy backed by hardware TRNG | rt584 has TRNG; **current build uses `CONFIG_TEST_RANDOM_GENERATOR=y` (timer-based) — not production-grade** |
| T.NETWORK_REPLAY | O.PROTO_FRESHNESS — protocol layers enforce nonce / counter | OpenThread MLE, Matter sessions handle this in stack |
| T.UNAUTH_ACCESS | O.AUTH — every CLI / API requires protocol-level auth | Inherited from Thread / Matter |
| T.RUNTIME_EXPLOIT | O.STACK_PROT, O.ASLR_LITE | Zephyr `CONFIG_STACK_CANARIES=y` (TBD), no W^X possible |
| T.SUPPLY_CHAIN | O.BLOB_INTEGRITY | RF firmware CRC32 verified at boot ([main.c](../../examples/thread/src/main.c#L83)) — vendor SDK and mbedTLS pinned to specific commits in `west.yml` |

### 6.2 Cryptographic services objective

| Objective | Implementation |
|---|---|
| O.CRYPTO_API — standardised crypto API for app | PSA Crypto via mbedTLS (`CONFIG_MBEDTLS_PSA_CRYPTO_C=y`) |
| O.CRYPTO_VALIDATED — primitives match spec | 56/64 PASS in psa-arch-tests v1.8 (see [P2 results](P2_psa_arch_tests_results.md)) |
| O.KEY_LIFECYCLE — keys obey usage flags, can be destroyed | PSA `psa_set_key_usage_flags` / `psa_destroy_key` validated by tests c202–c205 |

---

## 7. PSA Security Goals — Implementation Map

The PSA Certified L1 self-assessment scores the device against
[10 Security Goals (SG)](https://www.psacertified.org/development-resources/certification-resources/level-1/).
Below is the per-SG status for this TOE, with evidence and gaps.

### SG.1 — Unique Identification

| | |
|---|---|
| Status | ⚠️ Partial |
| Implementation | rt584 OTP holds unique die ID (visible in UART log: `otp: 0x52 0x54 0x35 0x38 0x34 ... = "RT584LD"` plus per-die fields) |
| Evidence | OT CLI output, hosal SDK `otp.c` |
| Gap | No PSA `psa_initial_attest_get_token` API exposing it; no UID export to upper layers |
| Effort to close | 2 days — wrap OTP read in a PSA driver / PSA platform interface |

### SG.2 — Security Lifecycle

| | |
|---|---|
| Status | ❌ Not implemented |
| Implementation | None |
| Evidence | n/a |
| Gap | rt584 has `sec_mcu_debug` and `sec_lock_mcu_ctrl` registers but their semantics + OTP fuse mapping is undocumented; no production lifecycle transition path |
| Effort to close | Unknown — depends on Rafael's documentation |

### SG.3 — Attestation

| | |
|---|---|
| Status | ❌ Not implemented |
| Implementation | None |
| Evidence | n/a |
| Gap | No attestation key, no EAT token generator |
| Effort to close | 1–2 weeks — port `psa_initial_attest_get_token` from TF-M's standalone implementation, generate provisioning attestation key |

### SG.4 — Secure Boot

| | |
|---|---|
| Status | ⚠️ Code present, signing disabled in dev |
| Implementation | MCUboot with image signature verification |
| Evidence | [examples/thread/sysbuild/mcuboot/prj.conf](../../examples/thread/sysbuild/mcuboot/prj.conf) — `BOOT_SIGNATURE_TYPE_NONE=y` for current build; flips to `BOOT_SIGNATURE_TYPE_ED25519` for production |
| Gap | Public key currently in flash; production must put key hash in OTP and have MCUboot verify against OTP-anchored hash (`MCUBOOT_HW_KEY=y`) |
| Effort to close | 2–3 days for OTP-anchored boot — needs Rafael OTP region docs |

### SG.5 — Secure Update

| | |
|---|---|
| Status | ✅ Code present (Direct-XIP) |
| Implementation | MCUboot Direct-XIP mode (sysbuild + slot1 variant), or Single-app + swap |
| Evidence | [examples/thread/sysbuild.conf](../../examples/thread/sysbuild.conf) — `SB_CONFIG_MCUBOOT_MODE_DIRECT_XIP=y` |
| Gap | OTA transport over Thread / BLE not yet integrated — `examples/thread/src/ota_app.c` is a stub |
| Effort to close | 1 week per transport (Matter OTA Provider, BLE SMP) |

### SG.6 — Anti-Rollback

| | |
|---|---|
| Status | ⚠️ Partial (OTA path covered) |
| Implementation | Direct-XIP version comparison with build number tie-breaker |
| Evidence | [examples/thread/sysbuild/mcuboot/prj.conf](../../examples/thread/sysbuild/mcuboot/prj.conf) — `CONFIG_BOOT_VERSION_CMP_USE_BUILD_NUMBER=y` (commit on `master` after `b667b42`); MCUboot at every boot evaluates both slots and runs the higher `major.minor.revision+build`. An attacker who lands a downgraded image in the inactive slot cannot displace the current image. |
| Gap | An attacker with debug-port access can write a downgraded image directly to slot0 — version comparison alone doesn't help there. Closing this requires an HW-anchored security counter (`MCUBOOT_HW_DOWNGRADE_PREVENTION` + platform `boot_nv_security_counter_*` impl) — see [SG6_anti_rollback_followup.md](SG6_anti_rollback_followup.md) for the implementation skeleton. |
| Effort to close fully | 1.5–2 days for flash-backed counter; OTP anchoring depends on SG.2 / Rafael fuse docs |
| Coverage today | Mitigates T.OTA_REPLAY (the primary Internet-facing downgrade vector). T.DEBUG_DUMP path remains open and is shifted onto SG.2 Lifecycle (which is also ❌ — chained risk). |

### SG.7 — Isolation

| | |
|---|---|
| Status | ❌ Hardware-blocked |
| Implementation | Impossible on this silicon |
| Evidence | [T0_ns_alias_survey.md](T0_ns_alias_survey.md) — SAU_TYPE=0, NS peripheral alias unrouteable, IDAU `sec_peri_attr` high bits RO |
| Gap | Would require Rafael silicon respin |
| Effort to close | ∞ (out of project scope) |

### SG.8 — Interaction

| | |
|---|---|
| Status | N/A |
| Implementation | No S / NS boundary to validate (see SG.7) |
| Evidence | n/a |

### SG.9 — Secure Storage

| | |
|---|---|
| Status | ⚠️ API available, NV backing not wired |
| Implementation | mbedTLS PSA ITS API present in build |
| Evidence | `psa_its_set/get/remove` in mbedTLS PSA Crypto |
| Gap | Backing store is RAM (volatile) — needs internal flash partition + wear-levelling |
| Effort to close | 3–5 days — implement PSA ITS backend over Zephyr `flash_map` + `nvs` |

### SG.10 — Cryptographic Services

| | |
|---|---|
| Status | ✅ Validated |
| Implementation | mbedTLS PSA Crypto |
| Evidence | **56 / 64 PASS** in ARM psa-arch-tests v1.8 Crypto suite ([P2 results](P2_psa_arch_tests_results.md)) |
| Gap | 8 fails on AES-256 NOT_SUPPORTED — Zephyr mbedTLS module config quirk; tracked but not blocking L1 |
| Effort to close | 2 hours for AES-256 investigation; 4 hours for ITS suite addition; 1–2 days for Initial Attestation suite |

### Summary

| SG | Status |
|---|---|
| 1. Unique Identification | ⚠️ Partial |
| 2. Lifecycle | ❌ |
| 3. Attestation | ❌ |
| 4. Secure Boot | ⚠️ |
| 5. Secure Update | ✅ |
| 6. Anti-Rollback | ⚠️ Partial (OTA path) |
| 7. Isolation | ❌ (silicon) |
| 8. Interaction | N/A |
| 9. Secure Storage | ⚠️ |
| 10. Cryptographic Services | ✅ |

**Overall PSA L1 readiness: 2 ✅ / 5 ⚠️ / 3 ❌ — short of self-assessment threshold.**
**Realistic closure effort (excluding SG.7): 3–5 weeks of focused work.**

---

## 8. Test & Compliance Evidence

### 8.1 PSA Functional API

- 56 / 64 ARM-official PSA Crypto compliance tests pass
- See [P2_psa_arch_tests_results.md](P2_psa_arch_tests_results.md)
- Reproducible: `west build --sysbuild -p always -b rt584_evb examples/psa_arch_tests -d build/psa_at584 && west flash -d build/psa_at584`

### 8.2 Bring-up validation

- OpenThread CLI prompt reaches `>` on rt584 with full RF MCU init
- See commit `25d2343` (RF MCU warm-reset bring-up — WAKE_UP fix)
- Reproducible: `west build --sysbuild -p always -b rt584_evb examples/thread -d build/thread584`

### 8.3 Silicon analysis

- IDAU / SAU / NS-alias behaviour empirically characterised
- See [T0_ns_alias_survey.md](T0_ns_alias_survey.md) and reproducible
  GDB script [debug/rt584_t0_survey.gdb](../../debug/rt584_t0_survey.gdb)

### 8.4 Recommended follow-up tests (not yet executed)

- PSA ITS test suite — needs NV-backed PSA ITS impl
- PSA Initial Attestation test suite — needs attestation key + EAT token generator
- MCUboot signed-image rejection test — load image with bad signature, verify boot fails

---

## 9. Residual Risks

### 9.1 Hardware-blocked

| Risk | Why we accept it | Mitigation in product context |
|---|---|---|
| No TZ isolation (SG.7) | rt584 silicon limitation | Document in product spec; warn customers that compromised firmware = compromised crypto keys; offer rt584 only for low-asset use cases |
| No identified debug-lock fuse | rt584 documentation gap | Risk-assess by application; physical security measures (epoxy, tamper switch) compensate |

### 9.2 Configuration-blocked (closable)

| Risk | Effort | Owner |
|---|---|---|
| Production firmware unsigned | 1 day | Eng |
| Anti-rollback not enabled | 1 day | Eng |
| Attestation not implemented | 1–2 weeks | Eng |
| ITS not NV-backed | 3–5 days | Eng |
| Entropy is timer-based, not TRNG | 2–3 days | Eng |

### 9.3 Process-blocked

| Risk | Owner |
|---|---|
| Provisioning SOP not documented | PM |
| Vulnerability disclosure policy not published | PM |
| CVE response SLA not defined | PM |

---

## 10. Conclusions

This TOE in its current state achieves **2 of 10 PSA Security Goals
fully** (SG.5 Secure Update, SG.10 Cryptographic Services), with 4
partial and 4 blocked. The biggest single block (SG.7 Isolation) is
unrecoverable without silicon respin; the remaining blocks are 4–6
weeks of focused engineering work.

The TOE is suitable as a **portfolio reference** demonstrating:

- ARM PSA Crypto API integration on a non-TF-M Cortex-M33 platform
- 87.5% pass rate against ARM's official Functional API conformance suite
- Empirically grounded silicon analysis (T0 survey) showing the limits
  of the chosen platform

The TOE is **not currently** suitable for PSA Certified L1 submission;
it would require closing SG.4 (key in OTP), SG.6 (anti-rollback), and
SG.9 (NV-backed ITS) at minimum.

---

## 11. Revision History

| Version | Date | Author | Notes |
|---------|------|--------|-------|
| 0.1 | 2026-04-26 | Stanley | Initial draft |
