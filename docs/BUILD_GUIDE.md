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
| Required tools | Python 3.14, CMake, Ninja, Git, 7-Zip via winget |
| ZAP CLI | Matter code-generation tool (~60 MB) |
| Zephyr SDK 1.0.1 | ARM toolchain (~2 GB) |
| West workspace | `west init` + `west update` — downloads Zephyr (~500 MB) |
| Python deps | `pip install` for Zephyr and connectedhomeip |
| GN | Generate Ninja binary for the Matter GN build |
| connectedhomeip submodules | pigweed, jsoncpp, nlassert, nlio, nanopb, abseil-cpp, openthread, uriparser |
| OpenOCD DLLs | Runtime DLLs for `tools/windows/openocd.exe` — pulled from `%USERPROFILE%\openocd-rt58x\dist-win\bin\` if present, otherwise downloaded (xPack / MSYS2) |
| env.ps1 | Generated inside the project directory |

> `openocd.exe` and `tools/windows/tcl/` are tracked in the repo and are not touched by `install.ps1` anymore — only the runtime DLLs are handled.

The install script validates the SDK archive (size ≥ 500 MB + 7z magic bytes) and aborts loudly if the download was truncated, so a failed download won't silently leave a broken SDK behind.

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

Verify:
```powershell
west --version       # should print west version
python -c "import jsonschema"   # no output = OK
```

If `jsonschema` is missing (CMake later complains `Missing jsonschema dependency`):
```powershell
pip install jsonschema
# or the full set:
pip install -r C:\Users\Stanley\zephyr\scripts\requirements.txt
```

---

## 3. Build Targets

| Target | Source | Output |
|--------|--------|--------|
| MCUboot bootloader | `../bootloader/mcuboot/boot/zephyr` | `build/bootloader/zephyr/zephyr.bin` |
| **Matter lighting** | `examples/matter/lighting-app` | `build/lighting-app/zephyr/zephyr.signed.bin` |
| Thread FTD CLI | `examples/thread` | `build/thread/zephyr/zephyr.signed.bin` |
| BLE HRS | `examples/ble/peripheral/hrs` | `build/ble_hrs/zephyr/zephyr.signed.bin` |
| Blinky | `examples/blinky` | `build/blinky/zephyr/zephyr.signed.bin` |
| Hello world | `examples/hello_world` | `build/hello_world/zephyr/zephyr.signed.bin` |

Build them all at once:
```powershell
.\scripts\windows\build_all.ps1
```

---

## 4. Build

> **Must** be run from the project root (`zephyr-thread-rt58x/`) — relative paths below depend on it.

### 4.1 MCUboot bootloader

> **Important:** If the app uses a DTS overlay that changes flash partitions
> (e.g. Matter lighting-app enlarges slot0/slot1 to 928 KB), the bootloader
> **must** be built with the same overlay. Otherwise MCUboot's slot layout
> won't match the app image header and it will report *image not found*.

**The bootloader source lives in a different repo** (`../bootloader/mcuboot/boot/zephyr`), so the overlay paths must be **absolute with forward slashes**.  CMake on Windows interprets `\U` (from `C:\Users\...`) as a Unicode escape and errors out. We convert `$PWD` to forward slashes:

```powershell
$root = $PWD.Path -replace '\\','/'

# Matter lighting-app (928 KB slots):
west build -p always -b rt583_evb ../bootloader/mcuboot/boot/zephyr `
    -d build/bootloader `
    -- -DOVERLAY_CONFIG="$root/examples/bootloader/mcuboot.conf" `
       -DDTC_OVERLAY_FILE="$root/examples/bootloader/mcuboot.overlay"
```

Why not a relative path to `DTC_OVERLAY_FILE`? Zephyr resolves relative overlay paths against `APPLICATION_SOURCE_DIR` (the bootloader's source dir in a *different* repo), not against the cwd. A relative path can never reach our project tree, so the path must be absolute.

### 4.2 Thread CLI

Source is in this repo → relative paths work:

```powershell
west build -p always -b rt583_evb examples/thread -d build/thread
```

### 4.3 Matter Lighting App

```powershell
west build -p always -b rt583_evb examples/matter/lighting-app -d build/lighting-app
```

### 4.4 Incremental build (code changes only)

```powershell
west build -d build/lighting-app
```

Use `-p always` only when `prj.conf`, Kconfig, or DTS files changed.

---

## 5. Flash

Hardware: connect **CMSIS-DAP** debugger to RT583-EVB.

```powershell
west flash -d build/bootloader
west flash -d build/lighting-app
west flash -d build/thread
```

### First-time full flash (bootloader + app)

```powershell
west flash -d build/bootloader
west flash -d build/lighting-app
```

The bootloader only needs flashing once — unless you change `mcuboot.conf` / `mcuboot.overlay` / slot sizes.

---

## 6. Full Workflow — Matter from scratch

```powershell
# Once per terminal
cd C:\Users\Stanley\zephyr-thread-rt58x
. .\env.ps1

# Build
$root = $PWD.Path -replace '\\','/'
west build -p always -b rt583_evb ../bootloader/mcuboot/boot/zephyr `
    -d build/bootloader `
    -- -DOVERLAY_CONFIG="$root/examples/bootloader/mcuboot.conf" `
       -DDTC_OVERLAY_FILE="$root/examples/bootloader/mcuboot.overlay"

west build -p always -b rt583_evb examples/matter/lighting-app -d build/lighting-app

# Flash
west flash -d build/bootloader
west flash -d build/lighting-app
```

---

## 7. Matter Commissioning

Once flashed, UART prints the onboarding code:

```
I: [SVR]SetupQRCode: [MT:6FCJ142C00KA0648G00]
I: [SVR]Manual pairing code: [34970112332]
```

Commission with chip-tool (runs on the **commissioner host** — typically Linux / macOS / WSL, **not** on the Windows build machine). Requires an OTBR on the LAN + the OT Active Dataset hex.

On WSL (from Windows) or Linux shell:
```bash
chip-tool pairing ble-thread 0x9 hex:<dataset-tlv-hex> 20202021 3840
```

> If you paste chip-tool commands into PowerShell, PS will interpret `<` as
> input redirection and fail.  Keep `<placeholder>` syntax as documentation
> only — replace the placeholder with the actual value before running, and
> run from a POSIX shell.

- `0x9` — node ID
- `20202021` — SPAKE2+ passcode (default)
- `3840` — discriminator (default)

Get the dataset from OTBR:
```bash
ot-ctl dataset active -x
```

---

## 8. Factory Reset

Three ways — all three wipe the `storage_partition` (`0x1E0000 + 64 KB`) and reboot:

| Method | How |
|--------|-----|
| Hold **GPIO0** for 6 seconds | UART shows `[BTN] Factory reset triggered (6s hold reached)` on release |
| `chip-tool operationalcredentials remove-fabric <idx> <node>` | When the last fabric is removed, the device auto-resets via a `FabricTable::Delegate` hook |
| Manual OpenOCD (PowerShell) | see block below |

PowerShell (backtick line continuation):
```powershell
openocd -f interface/cmsis-dap.cfg -f target/rt583.cfg `
    -c "init" -c "halt" `
    -c "flash erase_address 0x001E0000 0x10000" `
    -c "reset run" -c "shutdown"
```

---

## 9. Flash Layout (MCUboot)

**Default (Thread CLI / Blinky / BLE HRS):**
```
Address    Size    Partition
0x000000   64 KB   boot_partition    (MCUboot)
0x010000   800 KB  slot0_partition   (primary app)
0x0D8000   800 KB  slot1_partition   (OTA / Direct XIP)
0x1A0000   256 KB  staging_partition (OTA compressed download)
0x1E0000   64 KB   storage_partition (NVS / settings / PSA ITS)
0x1F0000   64 KB   factory_partition (reserved)
```

**Matter lighting-app (DTS overlay — larger slots):**
```
Address    Size    Partition
0x000000   64 KB   boot_partition    (MCUboot)
0x010000   928 KB  slot0_partition   (primary app)
0x0F8000   928 KB  slot1_partition   (OTA / Direct XIP)
0x1E0000   64 KB   storage_partition (NVS / settings / PSA ITS)
0x1F0000   64 KB   factory_partition (reserved)
```

> Bootloader and app **must** use the same partition layout.
> Mixing default bootloader with Matter app overlay will cause
> *Unable to find bootable image*.

---

## 10. Serial Console

After flashing, open a serial terminal:

- **Port**: check Device Manager for USB Serial Port (COM?)
- **Baud**: 115200
- **Settings**: 8N1, no flow control

Expected boot output for Matter:
```
*** Booting MCUboot ***
*** Booting Zephyr OS ***
WARNING: Using an insecure PSA ITS encryption key provider.
I: [DL]BLE address: ...
I: [DL]OpenThread started: OK
I: [SVR]Server Listening...
I: [SVR]SetupQRCode: [MT:...]
I: [DL]CHIPoBLE advertising started
```

---

## 11. Quick Reference

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

## 12. Troubleshooting

| Error | Cause | Fix |
|-------|-------|-----|
| `source directory ... does not exist` | Run from wrong cwd | `cd zephyr-thread-rt58x` then use `../bootloader/...` |
| `env.ps1 not found` | Not installed yet | Run `.\scripts\windows\install.ps1` first |
| `west: command not found` | venv not active | `. .\env.ps1` |
| `Missing jsonschema dependency` | Python dep missing | `pip install jsonschema` |
| `Could NOT find Python3 (missing: Interpreter)` | venv not active in this shell | re-source `env.ps1` |
| `Invalid character escape '\U'` in CMake | Windows backslash in path | convert to forward slashes: `$PWD.Path -replace '\\','/'` |
| `failed to preprocess devicetree files` | Overlay path relative while source is in another repo | use absolute path (see §4.1) |
| `openocd.exe` missing DLLs (libusb-1.0, libhidapi-0, libftdi1) | install.ps1 didn't run or DLL fetch failed | re-run `install.ps1`; DLLs land in `tools/windows/` |
| `recursive 'source' of 'Kconfig.zephyr' detected` (Windows only) | Zephyr v4.4.0-rc1 `modules/hal_espressif/Kconfig:23` does `osource "$(ZEPHYR_HAL_ESPRESSIF_MODULE_DIR)/zephyr/Kconfig"`; with the var unset, `/zephyr/Kconfig` resolves to the current drive root and re-sources the main Kconfig | re-source `env.ps1` — it sets `$env:ZEPHYR_HAL_ESPRESSIF_MODULE_DIR` to a stub path so `osource` skips silently |
| `Invalid BOARD` after a rebuild | `zephyr/module.yml` missing — it declares `board_root: .` | `git checkout zephyr/module.yml` then rebuild with `-p always` |
| `connectedhomeip not found` | Not cloned | See §1 |
| MCUboot `image not found` | Bootloader/app slot layouts mismatch | rebuild bootloader with the matching overlay |
| RAM overflow during link | `CONFIG_MBEDTLS_HEAP_SIZE` too high | drop to 10240 |
| CASE fails with `PSA error -141 at DeriveKey` | PSA key slots / heap exhausted | ensure `CONFIG_MBEDTLS_HEAP_SIZE ≥ 10240` + `CONFIG_MBEDTLS_PSA_KEY_SLOT_COUNT = 32` |
| `Cannot delete ... file in use` during install | 7-Zip GUI / AV scanning the download | close 7zFM/Explorer preview; `install.ps1` retries 5× then lists candidates |
| chip-tool `Invalid CASE parameter` | Stale fabric on device | factory-reset and retry |
| `west flash` can't find device (Windows) | CMSIS-DAP driver | install WinUSB via [Zadig](https://zadig.akeo.ie/) |

### PowerShell quirks cheat-sheet

| Pitfall | Example | Workaround |
|---------|---------|-----------|
| Backslashes in paths → CMake `\U` escape error | `C:\Users\...` in `-DDTC_OVERLAY_FILE=...` | `$root = $PWD.Path -replace '\\','/'` then use `$root/...` |
| Line continuation | long `west build` command | end line with **backtick** `` ` ``, not `\` |
| `<` / `>` in command args | `chip-tool ... hex:<tlv>` | PS parses as redirection — replace `<placeholder>` with actual value before running, or switch to WSL/bash |
| Dot-sourcing `env.ps1` | forgot the leading dot | `. .\env.ps1` (the dot + space is required) |
| venv disappears in new shell | fresh PowerShell window | `. .\env.ps1` every new session |
| `Remove-Item` fails on locked file | SDK install / stale 7z | `install.ps1` retries 5× and lists holders; otherwise reboot |
