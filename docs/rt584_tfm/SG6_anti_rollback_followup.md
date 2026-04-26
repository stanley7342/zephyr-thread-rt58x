# SG.6 Anti-Rollback — Follow-up: HW counter integration

**Status**: deferred (1.5–2 days of work)
**Prerequisite**: closing this gap requires touching MCUboot's build glue.

## What's already in place (current state)

`CONFIG_BOOT_VERSION_CMP_USE_BUILD_NUMBER=y` enabled in
[examples/thread/sysbuild/mcuboot/prj.conf](../../examples/thread/sysbuild/mcuboot/prj.conf).
Combined with Direct-XIP mode, this gives:

- MCUboot boots whichever of slot0/slot1 has the higher
  `major.minor.revision+build`
- An attacker who lands a downgraded image in the inactive slot
  cannot displace the current image (slot wins by version)
- Sign images with `--version M.m.r+b` via `imgtool` (sysbuild does
  this automatically based on `CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION`)

This protects the **OTA downgrade vector** (T.OTA_REPLAY in
[TMSA](TMSA.md) §5.1).

## What's still missing

The version-comparison protection breaks if an attacker can write
directly to slot0 — e.g., via a debug port that hasn't been locked
in production lifecycle (SG.2). To plug that, MCUboot's
**HW-anchored security counter** must be enabled:

1. `CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION=y` in
   `examples/<app>/sysbuild/mcuboot/prj.conf`
2. Provide platform implementation of:
   - `fih_ret boot_nv_security_counter_get(uint32_t image_id, fih_int *security_cnt);`
   - `int32_t boot_nv_security_counter_update(uint32_t image_id, uint32_t img_security_cnt);`
3. Sign images with `--security-counter N` (imgtool)
4. Production: anchor the counter in OTP (Rafael fuse mapping
   undocumented for rt584; see SG.2 follow-up)

## Implementation skeleton

### Counter storage (option A — flash partition)

Add to `dts/arm/rafael/rt584.dtsi` partitions (carve from existing
`storage_partition` or shrink slot1):

```
boot_counter_partition: partition@1F0000 {
    label = "boot-counter";
    reg = <0x1F0000 DT_SIZE_K(4)>;
};
```

### Source file (write to `boot/security_cnt/security_cnt_rt58x.c`)

```c
#include <bootutil/security_cnt.h>
#include <bootutil/fault_injection_hardening.h>
#include <flash_map_backend/flash_map_backend.h>
#include <bootutil/bootutil_log.h>
#include <stdint.h>

#define COUNTER_PARTITION_ID  FLASH_AREA_ID(boot_counter_partition)

fih_ret boot_nv_security_counter_get(uint32_t image_id,
                                     fih_int *security_cnt)
{
    const struct flash_area *fap;
    uint32_t cnt;
    int rc;

    rc = flash_area_open(COUNTER_PARTITION_ID, &fap);
    if (rc) FIH_RET(FIH_FAILURE);

    rc = flash_area_read(fap, 0, &cnt, sizeof(cnt));
    flash_area_close(fap);
    if (rc) FIH_RET(FIH_FAILURE);

    /* Erased flash reads 0xFFFFFFFF — treat as 0. */
    if (cnt == 0xFFFFFFFF) cnt = 0;

    *security_cnt = fih_int_encode(cnt);
    FIH_RET(FIH_SUCCESS);
}

int32_t boot_nv_security_counter_update(uint32_t image_id,
                                        uint32_t img_security_cnt)
{
    const struct flash_area *fap;
    int rc;

    rc = flash_area_open(COUNTER_PARTITION_ID, &fap);
    if (rc) return rc;

    rc = flash_area_erase(fap, 0, fap->fa_size);
    if (rc) goto out;

    rc = flash_area_write(fap, 0, &img_security_cnt, sizeof(img_security_cnt));
out:
    flash_area_close(fap);
    return rc;
}
```

### Injection into MCUboot build (the hard part)

MCUboot's Zephyr port (`bootloader/mcuboot/boot/zephyr/CMakeLists.txt`)
does **not** auto-include platform-side counter sources. Three options:

1. **Patch MCUboot CMakeLists.txt locally** to source from
   `${ZEPHYR_BASE}/../zephyr-thread-rt58x/boot/security_cnt/*.c` when
   `CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION=y`. Hacky but works.
2. **Author a Zephyr module under `boot/security_cnt/`** with its own
   `zephyr/module.yml` that contributes the source via
   `zephyr_library_sources()` guarded on `BOOT_BUILD`. Cleaner. Needs
   sysbuild to load module both in app and mcuboot subbuilds.
3. **Pre-build the counter as a static lib** in our project's
   top-level CMakeLists.txt and pass library path via mcuboot's
   `EXTRA_LIBS_*` variable. Requires MCUboot CMake to honour that
   variable — verify in upstream first.

Option 2 is the canonical path for upstreaming someday.

### OTP anchoring (option B — production)

Replace flash-partition impl with reads/writes to rt584 OTP region.
Requires Rafael's confirmation on:
- Which OTP word(s) are reserved for monotonic counter
- Whether OTP allows multiple writes (fuse style: only 0→1 transitions)
- OTP read latency (MCUboot must tolerate it during boot)

If OTP allows only 0→1 transitions, the counter implementation flips
to "count of cleared bits" — see `bootloader/mcuboot/boot/cypress/MCUBootApp/cy_security_cnt.c`
for prior art on that pattern.

## Test plan (when implemented)

1. Build app v1.0.0+5 with `--security-counter 5`, flash, boot OK
2. Build v1.0.0+6 with `--security-counter 6`, flash to slot1, boot OK
   (slot1 wins on next boot, counter advances 5→6 on confirmation)
3. Build v1.0.0+5 with `--security-counter 5` again, flash to slot0
   directly via SWD, reboot — MCUboot rejects (image counter < device counter)
4. Verify rejection log: `mcuboot: failed to advance security counter`

## Effort estimate

| Step | Time |
|------|------|
| Add boot_counter_partition to dts | 15 min |
| Write security_cnt_rt58x.c | 30 min |
| Inject into MCUboot build (option 2) | 4–6 hrs |
| Sign + flash + verify v1 boots | 30 min |
| Demonstrate v0 rejection via direct slot0 write | 1 hr |
| OTP anchoring (option B) | TBD by Rafael docs |

Total to ✅ on TMSA SG.6: **1.5–2 days**.
