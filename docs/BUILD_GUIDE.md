# RT583-EVB Build & Flash Guide

## Prerequisites

1. **Zephyr SDK + west workspace** — run the install script first:
   ```powershell
   .\scripts\windows\install.ps1
   ```

2. **connectedhomeip** (Matter only):
   ```powershell
   cd C:\Users\Stanley
   git clone --recurse-submodules https://github.com/project-chip/connectedhomeip
   cd connectedhomeip
   git checkout fdb31dbcb3ebe000ad3e41b386e90849937948f9
   ```

3. **GN build tool** (Matter only):
   ```powershell
   pip install gn
   ```

4. **Load environment** — every new terminal session:
   ```powershell
   cd C:\Users\Stanley\zephyr-thread-rt58x
   . ..\env.ps1
   ```

---

## Build Targets

| Target | Source | Output |
|--------|--------|--------|
| Thread CLI | `examples/thread` | `build/thread/zephyr/zephyr.signed.bin` |
| MCUboot bootloader | `../bootloader/mcuboot/boot/zephyr` | `build/bootloader/zephyr/zephyr.bin` |
| Matter lighting | `examples/matter/lighting-app` | `build/lighting-app/zephyr/zephyr.signed.bin` |
| BLE HRS | `examples/ble/peripheral/hrs` | `build/ble_hrs/zephyr/zephyr.signed.bin` |
| Blinky | `examples/blinky` | `build/blinky/zephyr/zephyr.signed.bin` |

---

## Build with west

### PowerShell + CMake arguments

PowerShell does not pass `--` correctly to external programs. Two workarounds:

**Option A — use `cmd /c`:**
```powershell
cmd /c "west build -p always -b rt583_evb <source> -d <build_dir> -- -DFOO=BAR"
```

**Option B — use environment variables instead of `-D`:**
```powershell
$env:OVERLAY_CONFIG = "C:/Users/Stanley/zephyr-thread-rt58x/examples/bootloader/mcuboot.conf"
west build -p always -b rt583_evb ..\bootloader\mcuboot\boot\zephyr -d build/bootloader
Remove-Item env:OVERLAY_CONFIG
```

If you don't need extra CMake args, `west build` works directly without workarounds.

### MCUboot bootloader (first time)

> **Important:** If the app uses a DTS overlay that changes flash partitions
> (e.g. Matter lighting-app enlarges slot0/slot1 to 928 KB), the bootloader
> **must** be built with the same overlay. Otherwise MCUboot's slot layout
> won't match the app image header and it will report "image not found".

```powershell
# For Thread CLI / Blinky / BLE HRS (default 800 KB slots):
$env:OVERLAY_CONFIG = "C:/Users/Stanley/zephyr-thread-rt58x/examples/bootloader/mcuboot.conf"
$env:DTC_OVERLAY_FILE = "C:/Users/Stanley/zephyr-thread-rt58x/examples/bootloader/mcuboot.overlay"
west build -p always -b rt583_evb ..\bootloader\mcuboot\boot\zephyr -d build/bootloader
Remove-Item env:OVERLAY_CONFIG
Remove-Item env:DTC_OVERLAY_FILE

# For Matter lighting-app (928 KB slots):
$env:OVERLAY_CONFIG = "C:/Users/Stanley/zephyr-thread-rt58x/examples/bootloader/mcuboot.conf"
$env:DTC_OVERLAY_FILE = "C:/Users/Stanley/zephyr-thread-rt58x/examples/bootloader/mcuboot.overlay;C:/Users/Stanley/zephyr-thread-rt58x/examples/matter/lighting-app/boards/rt583_evb.overlay"
west build -p always -b rt583_evb ..\bootloader\mcuboot\boot\zephyr -d build/bootloader-matter
Remove-Item env:OVERLAY_CONFIG
Remove-Item env:DTC_OVERLAY_FILE
```

### Thread CLI

```powershell
west build -p always -b rt583_evb examples/thread -d build/thread
```

### Matter Lighting App

```powershell
west build -p always -b rt583_evb examples/matter/lighting-app -d build/lighting-app
```

### Incremental build (after code changes)

```powershell
west build -d build/lighting-app
```

Use `-p always` only when `prj.conf`, Kconfig, or DTS files changed.

---

## Flash with west

Hardware: connect **CMSIS-DAP** debugger to RT583-EVB.

### Flash the most recent build

```powershell
west flash
```

### Flash a specific build directory

```powershell
west flash -d build/bootloader
west flash -d build/lighting-app
west flash -d build/thread
```

### First-time full flash (bootloader + app)

```powershell
# Thread CLI / Blinky / BLE HRS (default slots):
west flash -d build/bootloader
west flash -d build/thread

# Matter lighting-app (928 KB slots — use matching bootloader):
west flash -d build/bootloader-matter
west flash -d build/lighting-app
```

---

## Build script alternative

The PowerShell build/flash scripts handle all the `--` quoting issues automatically:

```powershell
# Build
.\scripts\windows\build.ps1 -p bootloader
.\scripts\windows\build.ps1 -p thread
.\scripts\windows\build.ps1 -p lighting-app
.\scripts\windows\build.ps1 -p lighting-app -NoPristine    # incremental

# Flash
.\scripts\windows\flash.ps1 -p bootloader        # default slots
.\scripts\windows\flash.ps1 -p lighting-app       # auto-uses matching bootloader
.\scripts\windows\flash.ps1 -p thread
```

---

## Flash Layout (MCUboot)

**Default (Thread CLI / Blinky / BLE HRS):**
```
Address    Size    Partition
0x000000   64 KB   boot_partition    (MCUboot)
0x010000   800 KB  slot0_partition   (primary app)
0x0D8000   800 KB  slot1_partition   (OTA / Direct XIP)
0x1A0000   256 KB  staging_partition (OTA compressed download)
0x1E0000   64 KB   storage_partition (NVS / settings)
0x1F0000   64 KB   factory_partition (reserved)
```

**Matter lighting-app (DTS overlay — larger slots):**
```
Address    Size    Partition
0x000000   64 KB   boot_partition    (MCUboot)
0x010000   928 KB  slot0_partition   (primary app)
0x0F8000   928 KB  slot1_partition   (OTA / Direct XIP)
0x1E0000   64 KB   storage_partition (NVS / settings)
0x1F0000   64 KB   factory_partition (reserved)
```

> Bootloader and app **must** use the same partition layout.
> Mixing default bootloader with Matter app overlay will cause
> "Unable to find bootable image".

## Serial Console

After flashing, open a serial terminal:
- **Port**: check Device Manager for USB Serial Port (COM?)
- **Baud**: 115200
- **Settings**: 8N1, no flow control

Expected boot output:
```
*** Booting MCUboot ***
*** Booting Zephyr OS ***
```

## Quick Reference

| Task | Command |
|------|---------|
| Clean build | `west build -p always -b rt583_evb <source> -d <dir>` |
| Incremental build | `west build -d <dir>` |
| Flash | `west flash -d <dir>` |
| Verbose build | `west build -v -d <dir>` |
| Update modules | `west update` |
| Update one module | `west update tf-psa-crypto` |
| List modules | `west list` |
