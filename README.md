# RT583-EVB — Zephyr + OpenThread FTD

Zephyr RTOS port for the **Rafael Microelectronics RT583** (ARM Cortex-M3 @ 64 MHz) with a full OpenThread FTD CLI.

| Item | Value |
|------|-------|
| SoC | RT583 (ARM Cortex-M3, 64 MHz BBPLL) |
| Zephyr | 4.4.0-rc1 (`v4.4.0-rc1-211-gcf4d0f72478e`) |
| OpenThread | FTD (Full Thread Device) CLI |
| Console | UART0 — TX: GPIO16, RX: GPIO17, **115200 8N1** |
| Flash usage | ~340 KB / 1 MB (35 %) |
| RAM usage | ~88 KB / 144 KB (61 %) |

---

## Table of Contents

1. [Quick Start — Windows](#1-quick-start--windows)
2. [Quick Start — Linux / WSL](#2-quick-start--linux--wsl)
3. [Quick Start — macOS](#3-quick-start--macos)
4. [Flashing](#4-flashing)
5. [Verification — Serial Terminal](#5-verification--serial-terminal)
6. [OpenThread CLI Quick Start](#6-openthread-cli-quick-start)
7. [Troubleshooting](#7-troubleshooting)
8. [Project Structure](#8-project-structure)

---

## 1. Quick Start — Windows

> **Requirement**: PowerShell 7 (run as Administrator).
> If not installed: `winget install Microsoft.PowerShell`

### Step 1 — One-click download and environment setup

Open PowerShell 7 (as Administrator) in any directory and run:

```powershell
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex
```

The script automatically:
- Clones the project to `<current directory>\zephyr-thread-rt58x`
- Installs Python, CMake, Ninja, Git, 7-Zip (via winget)
- Creates a Python venv and installs west, GN, jsonschema
- Downloads and installs Zephyr SDK 1.0.1
- Initializes the west workspace and downloads Zephyr
- Generates the `env.ps1` environment script

When complete, you will see:
```
West workspace : <current directory>
Project dir    : <current directory>\zephyr-thread-rt58x

Open a new PowerShell and run:
  . <current directory>\env.ps1

Build:
  cd <current directory>\zephyr-thread-rt58x
  west build -p always -b rt583_evb examples/matter/lighting-app
```

### Step 2 — Load environment (every new shell)

```powershell
. <workspace>\env.ps1
```

### Step 3 — Build

```powershell
cd zephyr-thread-rt58x
```

**Clean build (first time or after Kconfig changes):**

```sh
west build -p always -b rt583_evb examples/matter/lighting-app
```

**Incremental build (C/C++ code changes only):**

```sh
west build -b rt583_evb examples/matter/lighting-app
```

Build outputs:

```
build/lighting-app/zephyr/zephyr.bin
build/lighting-app/zephyr/zephyr.hex
```

---

## 2. Quick Start — Linux / WSL

```bash
git clone https://github.com/stanley7342/zephyr-thread-rt58x.git
cd zephyr-thread-rt58x

bash scripts/linux/install.sh
source ../env.sh

west build -p always -b rt583_evb examples/matter/lighting-app
# Incremental:
west build -b rt583_evb examples/matter/lighting-app
```

---

## 3. Quick Start — macOS

```bash
git clone https://github.com/stanley7342/zephyr-thread-rt58x.git
cd zephyr-thread-rt58x

bash scripts/macos/install.sh
source ../env.sh

west build -p always -b rt583_evb examples/matter/lighting-app
# Incremental:
west build -b rt583_evb examples/matter/lighting-app
```

### Install script options

| Platform | Option | Description |
|----------|--------|-------------|
| Windows | `-SdkDir <path>` | SDK installation directory (default: `C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1`) |
| Windows | `-Bg` | Run in background; log written to `<workspace>\install.log` |
| Linux/macOS | `--sdk-dir <path>` | SDK installation directory (default: `~/zephyr-sdk-1.0.1`) |

### Uninstall

```powershell
# Windows
.\scripts\windows\uninstall.ps1
```

```bash
# Linux / WSL
bash scripts/linux/uninstall.sh

# macOS
bash scripts/macos/uninstall.sh
```

The script lists what will be removed and asks for confirmation (`y`):

| Item | Action |
|------|--------|
| `.west` directory | Deleted entirely |
| `zephyr` (Zephyr source) | Deleted entirely |
| `env.ps1` / `env.sh` | Deleted |
| Zephyr SDK directory | Deleted entirely |
| `west` (pip package) | `pip uninstall west` |
| Python / CMake / Ninja and other system tools | Windows: `winget uninstall` (with Registry fallback); Linux/macOS: listed only, must be removed manually |

> To reinstall, simply re-run the `install` script.

---

## 4. Flashing

> Wiring: RT583-EVB GPIO16(TX) → USB-UART RX, GPIO17(RX) → TX, GND → GND

```sh
west flash
```

---

## 5. Verification — Serial Terminal

Open a terminal (PuTTY / Tera Term) at **115200 baud, 8N1, no flow control**.

After reset you should see:

```
[MAIN] start
...
> 
```

---

## 6. OpenThread CLI Quick Start

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
```

---

## 7. Troubleshooting

| Symptom | Fix |
|---------|-----|
| `find_package(Zephyr)` fails | Confirm `. env.ps1` has been sourced |
| `Could not find GN_EXECUTABLE` | `pip install gn` |
| `Missing jsonschema` | `pip install jsonschema` |
| `west init already initialized` | bootstrap skips this automatically — no action needed |
| No output after flashing | Confirm flash address is **0x0** (not 0x8000) |
| UART RX interrupt never fires | Confirm `IRQ_CONNECT` is called |

---

## 8. Project Structure

```
zephyr-thread-rt58x/
├── CMakeLists.txt              # Root CMake; ZEPHYR_EXTRA_MODULES registers modules + platform drivers
├── Kconfig                     # Root Kconfig; includes PHY/MAC PIB configuration menu
├── prj.conf                    # Kconfig fragments (UART, heap, OT enable)
├── west.yml                    # West manifest (Zephyr dependency versions)
│
├── scripts/
│   ├── windows/                # Windows scripts (PowerShell 7+)
│   │   ├── install.ps1         # Installs tools, SDK, west workspace, generates env.ps1
│   │   ├── build.ps1           # Build helper
│   │   └── uninstall.ps1       # Removes environment (with Registry fallback)
│   ├── linux/                  # Linux / WSL scripts (Bash)
│   │   ├── install.sh
│   │   ├── build.sh
│   │   └── uninstall.sh
│   ├── macos/                  # macOS scripts (Bash, Apple Silicon supported)
│   │   ├── install.sh
│   │   ├── build.sh
│   │   └── uninstall.sh
│   └── strip_printk_newlines.py
│
├── zephyr/
│   └── module.yml              # Zephyr module descriptor; points to cmake/zephyr_serial/
│
├── cmake/
│   └── zephyr_serial/
│       └── CMakeLists.txt      # Adds uart_rt583.c + hosal_uart.c to drivers__serial
│
├── subsys/openthread/
│   └── CMakeLists.txt          # All OT-related sources: OT core, mbedTLS, RUCI, RF, etc.
│
├── src/
│   └── main.c                  # RF init, PIB setup, OT task start, watchdog thread
│
├── drivers/
│   └── serial/
│       ├── uart_rt583.c        # Zephyr UART driver (HOSAL wrapper)
│       ├── Kconfig
│       └── CMakeLists.txt
│
├── sdk/                        # Vendored Rafael IoT SDK (source only — no prebuilt .a)
│   └── components/
│       ├── network/thread/     # OpenThread port (ot_radio.c, ot_uart.c, etc.)
│       ├── network/ruci/       # RUCI commands/events
│       ├── network/lmac15p4/   # IEEE 802.15.4 MAC
│       ├── network/rt569-rf/   # RT569 RF MCU driver
│       ├── network/rt569-fw/   # RT569 RF firmware blob
│       ├── network/thread/openthread/  # OpenThread source
│       ├── utility/            # log, fsm, util_queue, etc.
│       └── platform/           # SoC / HOSAL platform drivers
│           ├── hosal/rt583_hosal/
│           └── soc/rt583/
│
├── boards/arm/rt583_evb/       # Board definition (DTS, defconfig, Kconfig)
├── dts/arm/rafael/rt583.dtsi   # SoC DTS (Flash 1 MB, RAM 144 KB, UART0/1)
└── soc/arm/rafael_micro/rt583/ # SoC definition (soc.c: SystemInit + COMM_SUBSYSTEM IRQ)
```

### Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| Build everything from source; no prebuilt `.a` | Eliminates ABI incompatibilities when the toolchain is upgraded |
| `uart_rt583.c` added to `drivers__serial` via a Zephyr module | `ZEPHYR_EXTRA_MODULES` ensures the module CMake runs at the correct time |
| `soft_source_match_table.c` excluded | `ot_radio.c` already provides hardware-backed src match via lmac15p4; avoids symbol conflicts |
| PIB constants configured through Kconfig | RF parameter tuning requires only `prj.conf` changes — no application code edits |
| Shell disabled (`CONFIG_SHELL=n`) | Shell's interrupt-driven TX path races with `printk` polling, corrupting output |

---

## Reference — Clone and West Workspace Setup

This project uses **west** to manage Zephyr dependencies. Workspace layout:

```
<workspace>/              ← west workspace root (e.g. ~/)
├── zephyr/               ← Zephyr source (downloaded by west, ~500 MB)
└── zephyr-thread-rt58x/  ← this project (west manifest)
```

### 3.1 Initialize workspace (manual, first time)

```bash
# Initialize the west workspace with this project as the manifest
cd <workspace>
west init -l zephyr-thread-rt58x

# Download Zephyr
west update

# Install Zephyr Python dependencies
pip install -r zephyr/scripts/requirements-base.txt
```

### 3.2 Updating later

```bash
cd zephyr-thread-rt58x
git pull

cd ..
west update
```

---

## 4. Install Zephyr SDK Toolchain

The Zephyr SDK provides the official ARM toolchain (`arm-zephyr-eabi`).

### Windows

Download from [Zephyr SDK GitHub Releases](https://github.com/zephyrproject-rtos/sdk-ng/releases):
- `zephyr-sdk-<version>_windows-x86_64_gnu.7z`

Extract with 7-Zip to a path without spaces (e.g. `C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1\`), then run:

```powershell
$env:PATH += ";C:\Program Files\7-Zip"
& "C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1\setup.cmd"
```

### Linux / WSL

```bash
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v1.0.1/zephyr-sdk-1.0.1_linux-x86_64.tar.xz
tar -xf zephyr-sdk-1.0.1_linux-x86_64.tar.xz -C ~/
~/zephyr-sdk-1.0.1/setup.sh -c arm-zephyr-eabi
```

### macOS

```bash
# Apple Silicon
curl -LO https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v1.0.1/zephyr-sdk-1.0.1_macos-aarch64.tar.xz
tar -xf zephyr-sdk-1.0.1_macos-aarch64.tar.xz -C ~/
~/zephyr-sdk-1.0.1/setup.sh -c arm-zephyr-eabi
```

---

## 5. Set Environment Variables

These variables must be set every time you open a new shell (the `install` script generates an env file automatically).

### Windows

```powershell
. <workspace>\env.ps1
```

Or manually:

```powershell
$env:ZEPHYR_BASE              = "C:/Users/Stanley/zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:/zephyr-sdk-1.0.1/zephyr-sdk-1.0.1"
```

### Linux / macOS

```bash
source <workspace>/env.sh
```

Or manually:

```bash
export ZEPHYR_BASE=~/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-1.0.1
```

---

## 6. Build

Run from the `zephyr-thread-rt58x/` directory:

**Clean build (first time or after Kconfig changes):**

```sh
west build -p always -b rt583_evb examples/matter/lighting-app
```

**Incremental build:**

```sh
west build -b rt583_evb examples/matter/lighting-app
```

**OpenThread FTD CLI (non-Matter):**

```sh
west build -p always -b rt583_evb .
```

Output binary: `build/lighting-app/zephyr/zephyr.bin`

---

## 7. Flash

Connect a CMSIS-DAP debugger to the hardware, then run from `zephyr-thread-rt58x/`:

```sh
west flash
```

Specify a binary explicitly (optional):

```sh
west flash --bin-file build/lighting-app/zephyr/zephyr.bin
```

### Hardware wiring (UART console)

```
RT583-EVB          USB-UART adapter
GPIO16 (TX)  ───►  RX
GPIO17 (RX)  ◄───  TX
GND          ───── GND
```

---

## 8. Verification — Serial Terminal

Open a serial terminal (PuTTY, Tera Term) at **115200 baud, 8N1, no flow control**.

After reset you should see:

```
======================================
  RT583-EVB  Zephyr + OpenThread CLI
======================================
[MAIN] wdog started
[RF] hosal_rf_init...
[RF] hosal_rf_init done
[RF] lmac15p4_init...
[RF] lmac15p4_init done
[RF] PIB set done
OpenThread FTD task started.
>
```

When the `>` prompt appears, the OpenThread CLI is ready.

---

## 9. OpenThread CLI Quick Start

```
> state
disabled

# Create a new Thread network
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

# Show IPv6 addresses
> ipaddr
fd11:22:0:0:...
fe80::...
Done

# Show neighbors
> neighbor table
Done
```

For the full command reference see [OpenThread CLI Reference](https://openthread.io/reference/cli).

---

## 10. Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| `find_package(Zephyr)` fails | `ZEPHYR_BASE` not set or wrong path | Confirm `ZEPHYR_BASE` points to the correct `zephyr/` directory |
| `Could NOT find Python3: Found unsuitable version` | Python version too old | Confirm Python ≥ 3.12 |
| `No CMAKE_C_COMPILER` | Toolchain not configured | Confirm `ZEPHYR_TOOLCHAIN_VARIANT` and `ZEPHYR_SDK_INSTALL_DIR` are set |
| `west update` fails | revision not found on remote | Confirm the `revision` in `west.yml` is a valid tag or commit |
| No UART output at all | Binary flashed to wrong address | Flash to **0x0**, not `0x8000` |
| System hangs after `printk` | `printk` called from ISR (spinlock deadlock) | Use `k_work_submit` to defer to thread context |
| UART RX interrupt never fires | `IRQ_CONNECT` not called | Confirm `IRQ_CONNECT` is called in `uart_rt583.c` |
| OT task hangs after RF init | RF MCU init blocking | Confirm `CONFIG_RF_FW_INCLUDE_PCI=TRUE` compile definition |
| `>` prompt never appears | OT CLI init failed | Confirm `main.c` calls `otAppCliInit` |
| `west flash` can't find device | CMSIS-DAP driver not installed | Use [Zadig](https://zadig.akeo.ie/) to install the WinUSB driver |
| winget removal fails (exit 1603) | MSI database corrupted | `uninstall.ps1` automatically attempts Registry fallback (msiexec + forced cleanup) |

---

## 11. Project Structure

```
zephyr-thread-rt58x/
├── CMakeLists.txt              # Root CMake; ZEPHYR_EXTRA_MODULES registers modules + platform drivers
├── Kconfig                     # Root Kconfig; includes PHY/MAC PIB configuration menu
├── prj.conf                    # Kconfig fragments (UART, heap, OT enable)
├── west.yml                    # West manifest (Zephyr dependency versions)
│
├── scripts/
│   ├── windows/                # Windows scripts (PowerShell 7+)
│   │   ├── install.ps1         # Installs tools, SDK, west workspace, generates env.ps1
│   │   ├── build.ps1           # Build helper
│   │   └── uninstall.ps1       # Removes environment (with Registry fallback)
│   ├── linux/                  # Linux / WSL scripts (Bash)
│   │   ├── install.sh
│   │   ├── build.sh
│   │   └── uninstall.sh
│   ├── macos/                  # macOS scripts (Bash, Apple Silicon supported)
│   │   ├── install.sh
│   │   ├── build.sh
│   │   └── uninstall.sh
│   └── strip_printk_newlines.py
│
├── zephyr/
│   └── module.yml              # Zephyr module descriptor; points to cmake/zephyr_serial/
│
├── cmake/
│   └── zephyr_serial/
│       └── CMakeLists.txt      # Adds uart_rt583.c + hosal_uart.c to drivers__serial
│
├── subsys/openthread/
│   └── CMakeLists.txt          # All OT-related sources: OT core, mbedTLS, RUCI, RF, etc.
│
├── src/
│   └── main.c                  # RF init, PIB setup, OT task start, watchdog thread
│
├── drivers/
│   └── serial/
│       ├── uart_rt583.c        # Zephyr UART driver (HOSAL wrapper)
│       ├── Kconfig
│       └── CMakeLists.txt
│
├── sdk/                        # Vendored Rafael IoT SDK (source only — no prebuilt .a)
│   └── components/
│       ├── network/thread/     # OpenThread port (ot_radio.c, ot_uart.c, etc.)
│       ├── network/ruci/       # RUCI commands/events
│       ├── network/lmac15p4/   # IEEE 802.15.4 MAC
│       ├── network/rt569-rf/   # RT569 RF MCU driver
│       ├── network/rt569-fw/   # RT569 RF firmware blob
│       ├── network/thread/openthread/  # OpenThread source
│       ├── utility/            # log, fsm, util_queue, etc.
│       └── platform/           # SoC / HOSAL platform drivers
│           ├── hosal/rt583_hosal/
│           └── soc/rt583/
│
├── boards/arm/rt583_evb/       # Board definition (DTS, defconfig, Kconfig)
├── dts/arm/rafael/rt583.dtsi   # SoC DTS (Flash 1 MB, RAM 144 KB, UART0/1)
└── soc/arm/rafael_micro/rt583/ # SoC definition (soc.c: SystemInit + COMM_SUBSYSTEM IRQ)
```

### Key Design Decisions

| Decision | Rationale |
|----------|-----------|
| Build everything from source; no prebuilt `.a` | Eliminates ABI incompatibilities when the toolchain is upgraded |
| `uart_rt583.c` added to `drivers__serial` via a Zephyr module | `ZEPHYR_EXTRA_MODULES` ensures the module CMake runs at the correct time |
| `soft_source_match_table.c` excluded | `ot_radio.c` already provides hardware-backed src match via lmac15p4; avoids symbol conflicts |
| PIB constants configured through Kconfig | RF parameter tuning requires only `prj.conf` changes — no application code edits |
| Shell disabled (`CONFIG_SHELL=n`) | Shell's interrupt-driven TX path races with `printk` polling, corrupting output |
