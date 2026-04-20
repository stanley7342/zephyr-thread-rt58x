# RT583-EVB — Zephyr + Matter + OpenThread

Zephyr RTOS port for the **Rafael Microelectronics RT583** (ARM Cortex-M3 @ 64 MHz) with a full **Matter-over-Thread** stack (commissionable lighting-app) and a standalone **OpenThread FTD CLI**.

| Item | Value |
|------|-------|
| SoC | RT583 (ARM Cortex-M3, 64 MHz BBPLL) |
| RAM / Flash | 144 KB / 2 MB |
| Zephyr | 4.4.0-rc1 |
| Matter | v1.4.2 (connectedhomeip `72f238add7`) |
| OpenThread | FTD (Full Thread Device) |
| Crypto | TF-PSA-Crypto 4.0 / mbedTLS 4.0 |
| BLE | Zephyr host on RT583 RF MCU HCI |
| Console | UART0 — TX: GPIO16, RX: GPIO17, **115200 8N1** |

---

## Highlights — what works today

- ✅ **Matter commissioning over BLE-Thread** — end-to-end with chip-tool or Apple/Google ecosystems: BLE PASE → Thread attach → CASE Sigma1/2/3 → SRP register → operational.
- ✅ **`_matter._tcp` mDNS advertise** via SRP client to the Thread Border Router.
- ✅ **Factory reset** — 6-sec GPIO0 hold **or** `RemoveFabric` command physically erases the NVS storage partition.
- ✅ **MCUboot** — built and flashed together with the app via one `west build --sysbuild` + `west flash` pair. Per-app modes: **Direct-XIP** (Thread CLI, BLE HRS, Blinky, Hello world) or **SINGLE_APP** (Matter lighting, because connectedhomeip's deeply-nested object paths push `ar`'s archive-path buffer past Windows' 260-char MAX_PATH on the Direct-XIP slot1 rebuild).
- ✅ **OpenThread FTD CLI** — standalone build on the same board.

---

## Table of Contents

1. [Quick Start](#1-quick-start)
2. [Build Targets](#2-build-targets)
3. [Flashing](#3-flashing)
4. [Matter Commissioning with chip-tool](#4-matter-commissioning-with-chip-tool)
5. [Factory Reset](#5-factory-reset)
6. [OpenThread CLI Quick Start](#6-openthread-cli-quick-start)
7. [Flash Layout](#7-flash-layout)
8. [Troubleshooting](#8-troubleshooting)
9. [Project Structure](#9-project-structure)

This repo targets **Windows + PowerShell 7 only**. See
[`docs/BUILD_GUIDE.md`](docs/BUILD_GUIDE.md) for the full step-by-step
reference.

---

## 1. Quick Start

> **Requirement**: PowerShell 7 (run as Administrator) — `winget install Microsoft.PowerShell`

### Step 1 — One-click bootstrap

```powershell
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex
```

Installs Python 3.12 + CMake + Ninja + Git + 7-Zip (via winget), Zephyr SDK 1.0.1, ZAP CLI (Matter codegen), west + GN + jsonschema (pip), clones `connectedhomeip` at the pinned commit, initialises the west workspace, and generates `env.ps1`.

Total time: ~15–40 min first run.

### Step 2 — Load environment (once per shell)

```powershell
cd zephyr-thread-rt58x
. .\env.ps1
```

### Step 3 — Build + Flash

```powershell
west build --sysbuild -p always -b rt583_evb examples/matter/lighting-app -d build/lighting-app
west flash -d build/lighting-app
```

`--sysbuild` builds MCUboot + app in one invocation (driven by each app's
`sysbuild.conf`). `west flash` then writes both images. Do not run `west build`
without `--sysbuild` — the legacy two-step flow has been removed.

---

## 2. Build Targets

| Target | Source | MCUboot mode | App image |
|--------|--------|--------------|-----------|
| **Matter lighting** | `examples/matter/lighting-app` | `SINGLE_APP` | `build/lighting-app/lighting-app/zephyr/zephyr.signed.bin` |
| Thread FTD CLI | `examples/thread` | `DIRECT_XIP` | `build/thread/thread/zephyr/zephyr.signed.bin` |
| BLE HRS (peripheral) | `examples/ble/peripheral/hrs` | `DIRECT_XIP` | `build/ble_hrs/hrs/zephyr/zephyr.signed.bin` |
| Blinky | `examples/blinky` | `DIRECT_XIP` | `build/blinky/blinky/zephyr/zephyr.signed.bin` |
| Hello world | `examples/hello_world` | `DIRECT_XIP` | `build/hello_world/hello_world/zephyr/zephyr.signed.bin` |
| Flash unit test | `tests/flash` | `DIRECT_XIP` | `build/test_flash/test_flash/zephyr/zephyr.signed.bin` |

Every app carries its own `sysbuild.conf` (selects the MCUboot mode) and
`sysbuild/mcuboot.{conf,overlay}` (driver bits + flash partition layout).
One `west build --sysbuild` call configures and builds both MCUboot and the
app into the same output tree, and one `west flash` programs both images:
```powershell
west build --sysbuild -p always -b rt583_evb examples/thread               -d build/thread
west build --sysbuild -p always -b rt583_evb examples/matter/lighting-app  -d build/lighting-app
west build --sysbuild -p always -b rt583_evb examples/ble/peripheral/hrs   -d build/ble_hrs
west build --sysbuild -p always -b rt583_evb examples/blinky               -d build/blinky
west build --sysbuild -p always -b rt583_evb examples/hello_world          -d build/hello_world
west build --sysbuild -p always -b rt583_evb tests/flash                   -d build/test_flash
```

---

## 3. Flashing

Requires a CMSIS-DAP debugger. `tools/windows/openocd.exe` and the `tcl/`
scripts are tracked in the repo, so no separate OpenOCD install is needed on
Windows.

Sysbuild registers MCUboot and the app as two flashable domains inside the
same build directory, so one command writes both:

```powershell
west flash -d build/lighting-app
```

Re-running `west flash` after an app-only rebuild re-programs both domains;
the MCUboot hex is unchanged so it's effectively a no-op for the bootloader
on most OpenOCD runs.

---

## 4. Matter Commissioning with chip-tool

Prerequisites:
- **OpenThread Border Router** (OTBR) running somewhere on your LAN
- `chip-tool` from connectedhomeip repo (build from source or grab from CI artifacts)
- The Thread Active Operational Dataset from OTBR (`ot-ctl dataset active -x`)

### Boot + get the setup code from serial

The device prints its onboarding info on boot:

```
I: [SVR]SetupQRCode: [MT:6FCJ142C00KA0648G00]
I: [SVR]Manual pairing code: [34970112332]
```

### Pair over BLE + Thread

```bash
chip-tool pairing ble-thread 0x9 hex:<dataset-tlv-hex> 20202021 3840
```

- `0x9` — arbitrary node ID to assign
- `hex:<dataset-tlv-hex>` — from OTBR
- `20202021` — SPAKE2+ passcode (dev commissioner default)
- `3840` — discriminator (default in `lighting-app/prj.conf`)

### Verify on-off and level control

```bash
chip-tool onoff toggle 0x9 1
chip-tool levelcontrol move-to-level 128 0 0 0 0x9 1
chip-tool descriptor read parts-list 0x9 1
```

### Remove a fabric (triggers factory reset)

```bash
chip-tool operationalcredentials remove-fabric 1 0x9 0
```

When the last fabric is removed, the device automatically factory-resets (NVS wipe + reboot) via a `FabricTable::Delegate` hook.

---

## 5. Factory Reset

### GPIO0 hold (6 seconds)

Press and hold **GPIO0** for 6 seconds. On release you see:
```
[BTN] Factory reset triggered (6s hold reached)
```

The device calls `flash_erase()` on the storage partition (0x001E0000 + 64 KB) and reboots cold.

### Via `RemoveFabric`

See Section 4 — removing the last fabric triggers the same flash-erase path automatically.

### Manual (OpenOCD)

```powershell
openocd -f interface/cmsis-dap.cfg -f target/rt583.cfg `
    -c "init" -c "halt" `
    -c "flash erase_address 0x001E0000 0x10000" `
    -c "reset run" -c "shutdown"
```

---

## 6. OpenThread CLI Quick Start

The `examples/thread` target builds a **standalone** OT FTD CLI (no Matter).

```
> dataset init new
Done
> dataset commit active
Done
> ifconfig up
Done
> thread start
Done
> state
leader
> ipaddr
fd11:22:0:0:...
fe80::...
Done
```

Full reference: [OpenThread CLI Reference](https://openthread.io/reference/cli).

---

## 7. Flash Layout

**Default (Thread CLI / Blinky / BLE HRS — 800 KB slots):**
```
0x000000   64 KB   boot_partition
0x010000  800 KB   slot0_partition
0x0D8000  800 KB   slot1_partition
0x1A0000  256 KB   staging_partition  (OTA)
0x1E0000   64 KB   storage_partition  (NVS / PSA ITS)
0x1F0000   64 KB   factory_partition
```

**Matter lighting-app (928 KB slots via DTS overlay):**
```
0x000000   64 KB   boot_partition
0x010000  928 KB   slot0_partition
0x0F8000  928 KB   slot1_partition
0x1E0000   64 KB   storage_partition  (NVS / PSA ITS)
0x1F0000   64 KB   factory_partition
```

> Bootloader and app **must** use the same partition layout, or MCUboot reports "image not found".

---

## 8. Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| `find_package(Zephyr)` fails | env not loaded | `. .\env.ps1` |
| `recursive 'source' of 'Kconfig.zephyr' detected` (Windows only) | `ZEPHYR_HAL_ESPRESSIF_MODULE_DIR` unset, resolves `/zephyr/Kconfig` to drive root | re-source `env.ps1` — it sets the var to a stub path |
| `boards/arm/rt583_evb/mcuboot.overlay: No such file` | `$root` wasn't set in the shell that ran `west build` | paste the `$root = $PWD.Path -replace '\\','/'` line in the **same** session, right before `west build` |
| `Could NOT find Python3: missing Interpreter` | venv not activated in this shell | re-source env; `-p always` rebuild |
| MCUboot `image not found` | Partial flash (only app or only MCUboot made it) | `west flash -d build/<app>` programs both domains; if it persists, `rm -rf build/<app>` then rebuild with `-p always` |
| RAM overflow during link | `CONFIG_MBEDTLS_HEAP_SIZE` too high for RAM | lower heap to 10240, keep `CONFIG_MBEDTLS_PSA_KEY_SLOT_COUNT=32` |
| CASE fails with `PSA error -141 at DeriveKey` | mbedTLS heap or PSA key slots exhausted | ensure `CONFIG_MBEDTLS_HEAP_SIZE≥10240` + `CONFIG_MBEDTLS_PSA_KEY_SLOT_COUNT≥32` |
| `Failed to advertise commissionable node: 3` then recovery | DNS-SD publishing races with SRP client register | benign — ignored after SRP client registers with OTBR |
| chip-tool "Invalid CASE parameter" | fabric mismatch or stale commissioning state on device | factory-reset (GPIO0 or OpenOCD erase) and retry |
| UART RX interrupt never fires | `IRQ_CONNECT` missing | see `drivers/serial/uart_rt583.c` |
| `west flash` can't find device (Windows) | CMSIS-DAP driver missing | use [Zadig](https://zadig.akeo.ie/) to install WinUSB |

---

## 9. Project Structure

```
zephyr-thread-rt58x/
├── CMakeLists.txt              # Root; registers ZEPHYR_EXTRA_MODULES
├── Kconfig                     # Root; PHY/MAC PIB tuning menu
├── west.yml                    # West manifest (Zephyr pin)
│
├── examples/
│   ├── matter/lighting-app/    # Matter Light end-device (commissionable)
│   ├── thread/                 # Standalone OpenThread FTD CLI
│   ├── ble/peripheral/hrs/     # BLE HRS sample
│   ├── blinky/ hello_world/
│   └── bootloader/             # MCUboot overlay configs (legacy; sysbuild handles MCUboot per-app now)
│
├── scripts/
│   ├── windows/                # install / build / flash / uninstall (PowerShell 7 — the only supported host)
│   └── strip_printk_newlines.py
│
├── subsys/openthread/
│   ├── CMakeLists.txt          # OT core + RUCI + RF sources
│   └── platform/               # ot_zephyr / ot_radio / ot_uart / hosal_rf glue
│
├── drivers/
│   ├── serial/uart_rt583.c     # Zephyr UART (HOSAL wrapper)
│   ├── ieee802154/             # rt583 802.15.4 driver over lmac15p4
│   ├── bluetooth/hci_rt583.c   # Zephyr HCI driver for the RF MCU
│   ├── hosal/rt583_hosal/      # HOSAL platform layer
│   └── soc/rt583/              # SoC register/clock driver
│
├── sdk/components/
│   ├── network/thread/         # OpenThread source + RT583 port
│   ├── network/ruci/           # RUCI commands/events
│   ├── network/lmac15p4/       # IEEE 802.15.4 MAC
│   ├── network/rt569-rf|-fw/   # RF MCU driver + firmware blob
│   └── utility/                # log, fsm, queues
│
├── boards/arm/rt583_evb/       # Board DTS + defconfig
├── dts/arm/rafael/rt583.dtsi   # SoC DT (flash/RAM/UART)
└── soc/arm/rafael_micro/rt583/ # SoC init (SystemInit, COMM_SUBSYSTEM IRQ)
```

### Key design decisions

| Decision | Rationale |
|----------|-----------|
| Build everything from source (no prebuilt `.a`) | Toolchain upgrades don't break ABI |
| Matter + OT + BLE share one RF MCU | `HOSAL_RF_MODE_MULTI_PROTOCOL` — BLE commissioning and 802.15.4 coexist |
| ECDH bypass via `mbedtls_ecp_mul` (connectedhomeip fork patch) | TF-PSA-Crypto 4.0's `psa_raw_key_agreement` rejects valid keypairs on this build |
| `FabricTable::Delegate` triggers NVS erase on last-fabric removal | Aligns behaviour with the GPIO0 6-sec hold |
| Shell disabled (`CONFIG_SHELL=n`) on Matter | shell TX ISR races with `printk` polling on UART0 |
| OT log level = `NONE` on Matter | Matter's `ChipLogProgress/Error` provides all needed visibility; OT strings waste ~27 KB flash |

---

## License

Apache-2.0 (same as Zephyr, OpenThread, and connectedhomeip).
