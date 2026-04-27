# PSA Cert L1 Portfolio — Zephyr on Rafael rt584

A self-contained reference for PSA Certified Level 1 self-assessment
on a non-TF-M Cortex-M33 platform. Built as a learning + portfolio
exercise on the [zephyr-thread-rt58x](../../) project; not submitted
to a PSA Certified evaluator.

## Why this exists

Most PSA-Cert references assume a TF-M-capable platform (Nordic nRF54,
NXP K32W, ST STM32U5). This bundle documents the path on a platform
where TF-M's standard "Zephyr-NS + TF-M-S + NSC veneers" model is
**not viable** because the silicon omits SAU regions and refuses to
route the NS peripheral alias. We pivot to **mbedTLS PSA Crypto without
isolation** and recover as much of the PSA Cert L1 SG checklist as
the silicon allows.

## Status

| Security Goal | Status | Evidence |
|---------------|--------|----------|
| SG.1 Unique Identification | ✅ | [TMSA §7.SG.1](TMSA.md#sg1--unique-identification) → [psa_hello main.c](../../examples/psa_hello/src/main.c) PUF UID + PSA-derived implementation_id |
| SG.2 Lifecycle | ❌ | Blocked: rt584 OTP fuse mapping for debug-lock undocumented |
| SG.3 Attestation | ❌ | Open: 1–2 weeks of work to port Initial Attestation token generator |
| SG.4 Secure Boot | ✅ | MCUboot ED25519 — [examples/thread/sysbuild.conf](../../examples/thread/sysbuild.conf) |
| SG.5 Secure Update | ✅ | MCUboot Direct-XIP — [examples/thread/sysbuild.conf](../../examples/thread/sysbuild.conf) |
| SG.6 Anti-Rollback | ⚠️ | OTA path covered by Direct-XIP version-cmp; HW counter is [follow-up](SG6_anti_rollback_followup.md) |
| SG.7 Isolation | ❌ | **Permanently blocked**: rt584 silicon — [T0 survey](T0_ns_alias_survey.md) |
| SG.8 Interaction | N/A | No S/NS boundary to validate |
| SG.9 Secure Storage | ✅ | NV-backed PSA ITS via Zephyr settings/NVS — [psa_hello prj.conf](../../examples/psa_hello/prj.conf) |
| SG.10 Cryptographic Services | ✅ | 56/64 PASS in ARM psa-arch-tests Crypto — [P2 results](P2_psa_arch_tests_results.md) |

**Scorecard: 5 ✅ / 1 ⚠️ / 3 ❌** (excluding N/A SG.8). One permanent
silicon block. Two open gaps require either Rafael documentation
(SG.2) or focused engineering work (SG.3).

## Documents

| Document | Role | Pages |
|---|---|---|
| [TMSA.md](TMSA.md) | Threat Model + Security Analysis (PSA L1 main deliverable) — TOE description, 8 assets, 13 threats, per-SG implementation map | ~25 |
| [T0_ns_alias_survey.md](T0_ns_alias_survey.md) | Empirical silicon analysis: SAU/IDAU/NS-alias behaviour on rt584 → why TF-M's standard model is unviable | ~3 |
| [P2_psa_arch_tests_results.md](P2_psa_arch_tests_results.md) | ARM PSA Functional API conformance test results: 56/64 PASS on rt584 | ~3 |
| [SG6_anti_rollback_followup.md](SG6_anti_rollback_followup.md) | HW security counter implementation skeleton (deferred from current scope) | ~2 |

## Reproducible artefacts

All examples build and flash from the [project root](../..).

```powershell
cd <repo-root>
. .\env.ps1
```

| Demo | What | Build & flash |
|---|---|---|
| `examples/psa_hello` | SG.1 UID + SG.9 ITS counter + SG.10 SHA-256 hash | `west build --sysbuild -p always -b rt584_evb examples/psa_hello -d build/psa584 && west flash -d build/psa584` |
| `examples/psa_arch_tests` | SG.10 — runs ARM psa-arch-tests v1.8 Crypto suite | `west build --sysbuild -p always -b rt584_evb examples/psa_arch_tests -d build/psa_at584 && west flash -d build/psa_at584` |
| `examples/thread` | SG.4 ED25519 + SG.5 Direct-XIP + SG.6 version-cmp | `west build --sysbuild -p always -b rt584_evb examples/thread -d build/thread584 && west flash -d build/thread584` |

UART monitor at 115200 8N1 on the rt584_evb USB-UART bridge.

## Reproducible diagnostic tooling

| Tool | Purpose |
|---|---|
| [debug/rt584_t0_survey.gdb](../../debug/rt584_t0_survey.gdb) | Reproduces T0 NS-alias / SAU / IDAU experiments |
| [debug/rt584_rf_step.gdb](../../debug/rt584_rf_step.gdb) | Step-through GDB for RF MCU bring-up debug |
| [debug/rt584_diag.gdb](../../debug/rt584_diag.gdb) | Generic boot-fault diagnostic dump |

Run via openocd + arm-zephyr-eabi-gdb:

```powershell
.\tools\windows\openocd.exe -s .\tools\windows\tcl `
    -f interface\cmsis-dap.cfg -f target\rt584.cfg `
    -c "init" -c "reset halt"
arm-zephyr-eabi-gdb -x debug\rt584_t0_survey.gdb build\thread584\thread\zephyr\zephyr.elf
```

## Commit timeline (PSA-cert track only)

| Commit | Goal | What landed |
|--------|------|-------------|
| `b04c9d9` | T0 silicon survey | docs/rt584_tfm/T0_ns_alias_survey.md + GDB script |
| `5d7c5ef` | P1 psa_hello | SHA-256 hello world via mbedTLS PSA |
| `d3a04bf` | P2 psa-arch-tests | Integration of ARM official Functional API conformance suite |
| `7c0f2bf` | P2 results doc | docs/rt584_tfm/P2_psa_arch_tests_results.md (56/64 PASS) |
| `b667b42` | TMSA v0.1 | docs/rt584_tfm/TMSA.md initial draft |
| `4943e8f` | SG.6 partial | Direct-XIP version-cmp + HW counter follow-up doc |
| `42775e1` | SG.4 ✅ | Switched thread example to ED25519 signed images |
| `f767ee0` | SG.1 ✅ | rt584 PUF UID read + PSA implementation_id derive |
| `1cb3ace` | SG.9 ✅ | NV-backed PSA ITS via secure_storage + settings + NVS |
| `dddca3d` | SG.9 fix | Enable PSA AEAD primitives so ITS encrypt path actually runs |

## Honest limitations

1. **No PSA Cert L1 evaluator review has been done** — this is portfolio
   evidence, not certified work.
2. The "WARNING: Using an insecure PSA ITS encryption key provider"
   shown by Zephyr at boot accurately reflects state — the AEAD
   transform uses a placeholder KEK, not a HW-rooted one.
3. **rt584 OTP secure-boot fuse layout is not documented** by Rafael
   in the public SDK; SG.4's claim of ED25519 verification is
   accurate, but production hardening (key-hash in OTP) is blocked
   on vendor docs.
4. **SG.7 Isolation is permanently blocked** by the silicon's missing
   SAU regions ([T0 survey](T0_ns_alias_survey.md)). Any project
   actually requiring PSA L1 isolation should pick a different SoC.

## What this portfolio demonstrates

- ARM PSA Crypto API integration on a non-TF-M Cortex-M33 platform
- 87.5% pass rate against ARM's official Functional API conformance suite
- Empirically grounded silicon analysis showing the limits of the
  chosen platform — and a clear pivot strategy when TF-M is unviable
- PSA Cert L1 paperwork structure: TMSA, threat model, security
  goal mapping, residual risk register
- Honest scoping: features that don't work are documented as such,
  not handwaved
