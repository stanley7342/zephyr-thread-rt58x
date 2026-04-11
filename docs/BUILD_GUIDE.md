# RT583-EVB Build & Flash Guide

## 1. First-time Setup

### Option A — Bootstrap (no prior clone needed)

Open PowerShell 7 and run:

```powershell
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex
```

This clones the repo and runs `install.ps1` automatically.

### Option B — Manual install (repo already cloned)

```powershell
cd zephyr-thread-rt58x
.\scripts\windows\install.ps1
```

`install.ps1` downloads and installs (first run takes 15–40 min):

| Step | What it does |
|------|-------------|
| Required tools | Python 3.12, CMake, Ninja, Git, 7-Zip via winget |
| ZAP CLI | Matter code-generation tool (~60 MB) |
| Zephyr SDK 1.0.1 | ARM toolchain (~2 GB) |
| West workspace | `west init` + `west update` — downloads Zephyr (~500 MB) |
| Python deps | `pip install` for Zephyr and connectedhomeip |
| GN | Generate Ninja binary for the Matter GN build |
| connectedhomeip submodules | pigweed, jsoncpp, nlassert, nlio, nanopb, abseil-cpp, openthread, uriparser |
| env.ps1 | Generated inside the project directory |

### connectedhomeip (Matter only)

If not already cloned, clone it beside the workspace:

```powershell
git clone --depth=1 https://github.com/project-chip/connectedhomeip ..\connectedhomeip
cd ..\connectedhomeip
git fetch --depth=1 origin fdb31dbcb3ebe000ad3e41b386e90849937948f9
git checkout fdb31dbcb3ebe000ad3e41b386e90849937948f9
```

Then re-run `install.ps1` to initialise its submodules.

---

## 2. Load Environment

Run this **once per terminal session** from inside the project directory:

```powershell
cd zephyr-thread-rt58x
. .\env.ps1
```

---

## 3. Build Targets

| Target | Source | Output |
|--------|--------|--------|
| Thread CLI | `examples/thread` | `build/thread/zephyr/zephyr.signed.bin` |
| MCUboot bootloader | `../bootloader/mcuboot/boot/zephyr` | `build/bootloader/zephyr/zephyr.bin` |
| Matter lighting | `examples/matter/lighting-app` | `build/lighting-app/zephyr/zephyr.signed.bin` |
| BLE HRS | `examples/ble/peripheral/hrs` | `build/ble_hrs/zephyr/zephyr.signed.bin` |
| Blinky | `examples/blinky` | `build/blinky/zephyr/zephyr.signed.bin` |

---

## 4. Build

### MCUboot bootloader

> **Important:** If the app uses a DTS overlay that changes flash partitions
> (e.g. Matter lighting-app enlarges slot0/slot1 to 928 KB), the bootloader
> **must** be built with the same overlay. Otherwise MCUboot's slot layout
> won't match the app image header and it will report "image not found".

```powershell
# For Thread CLI / Blinky / BLE HRS (default 800 KB slots):
west build -p always -b rt583_evb ../bootloader/mcuboot/boot/zephyr `
    -d build/bootloader `
    -- -DOVERLAY_CONFIG="examples/bootloader/mcuboot.conf" `
       -DDTC_OVERLAY_FILE="examples/bootloader/mcuboot.overlay"

# For Matter lighting-app (928 KB slots):
west build -p always -b rt583_evb ../bootloader/mcuboot/boot/zephyr `
    -d build/bootloader-matter `
    -- -DOVERLAY_CONFIG="examples/bootloader/mcuboot.conf" `
       -DDTC_OVERLAY_FILE="examples/bootloader/mcuboot.overlay;examples/matter/lighting-app/boards/rt583_evb.overlay"
```

### Thread CLI

```powershell
west build -p always -b rt583_evb examples/thread -d build/thread
```

### Matter Lighting App

```powershell
west build -p always -b rt583_evb examples/matter/lighting-app -d build/lighting-app
```

### Incremental build (after code changes only)

```powershell
west build -d build/lighting-app
```

Use `-p always` only when `prj.conf`, Kconfig, or DTS files changed.

---

## 5. Flash

Hardware: connect **CMSIS-DAP** debugger to RT583-EVB.

```powershell
# Flash a specific build directory
west flash -d build/bootloader
west flash -d build/lighting-app
west flash -d build/thread
```

### First-time full flash (bootloader + app)

```powershell
# Thread CLI / Blinky / BLE HRS:
west flash -d build/bootloader
west flash -d build/thread

# Matter lighting-app (928 KB slots — use matching bootloader):
west flash -d build/bootloader-matter
west flash -d build/lighting-app
```

---

## 6. Full Workflow

```powershell
# Once per terminal
cd zephyr-thread-rt58x
. .\env.ps1

# Build
west build -p always -b rt583_evb ../bootloader/mcuboot/boot/zephyr `
    -d build/bootloader `
    -- -DOVERLAY_CONFIG="examples/bootloader/mcuboot.conf" `
       -DDTC_OVERLAY_FILE="examples/bootloader/mcuboot.overlay"

west build -p always -b rt583_evb examples/matter/lighting-app -d build/lighting-app

# Flash
west flash -d build/bootloader
west flash -d build/lighting-app
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

---

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

---

## Quick Reference

| Task | Command |
|------|---------|
| Clean build | `west build -p always -b rt583_evb <source> -d <dir>` |
| Incremental build | `west build -d <dir>` |
| Flash | `west flash -d <dir>` |
| Verbose build | `west build -v -d <dir>` |
| Update all modules | `west update` |
| Update one module | `west update tf-psa-crypto` |
| List modules | `west list` |

---

## Troubleshooting

| Error | Fix |
|-------|-----|
| `source directory ... does not exist` | Run from inside `zephyr-thread-rt58x` and use `../bootloader/...` |
| `env.ps1 not found` | Run `.\scripts\windows\install.ps1` first |
| `west: command not found` | Run `. .\env.ps1` to activate the venv |
| `openocd.exe not found` | Run `.\scripts\windows\install.ps1` to install it |
| `connectedhomeip not found` | Clone it: see Section 1 above |
| `image not found` (MCUboot) | Bootloader and app slot layouts do not match — rebuild bootloader with the correct overlay |
