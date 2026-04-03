# RT582 Zephyr Hello World — Build Guide

## Overview

This project ports the Rafael IoT SDK `helloworld` example to Zephyr RTOS v4.1.0.
It demonstrates two concurrent threads printing to the UART console using Zephyr kernel
primitives instead of FreeRTOS.

| Item | Value |
|------|-------|
| Target SoC | Rafael Microelectronics RT582 (ARM Cortex-M3, 48 MHz) |
| Zephyr version | 4.1.0 |
| Toolchain | Arm GNU Toolchain 14.2.1 (bundled with Rafael IoT SDK) |
| Console | UART0 — TX: GPIO16, RX: GPIO17, 115200 8N1 |
| Flash usage | ~10 KB / 1 MB |
| RAM usage | ~6.5 KB / 144 KB |

---

## Prerequisites

### 1. Rafael IoT SDK

The SDK must be present at the path below (or set `RAFAEL_SDK_BASE` to override):

```
C:\Users\Stanley\Rafael-IoT-SDK-Internal\
```

The build only requires these SDK components — no full SDK build is needed:

| Component | Path under SDK |
|-----------|----------------|
| MCU headers (mcu.h, etc.) | `components/platform/soc/rt582/rt582_driver/Inc/` |
| System headers | `components/platform/soc/rt582/rt582_system/Include/` |
| HOSAL UART header & source | `components/platform/hosal/rt582_hosal/` |
| sysctrl.c (pin/clock helpers) | `components/platform/soc/rt582/rt582_driver/Src/sysctrl.c` |

### 2. Zephyr RTOS

Zephyr 4.1.0 workspace must be present at:

```
C:\Users\Stanley\zephyrproject\
```

If you have not installed it yet, run:

```bash
pip install west
west init C:/Users/Stanley/zephyrproject --mr v4.1.0
cd C:/Users/Stanley/zephyrproject
west update
pip install -r zephyr/scripts/requirements.txt
```

### 3. ARM Toolchain

The ARM GNU toolchain shipped with the Rafael IoT SDK is used:

```
C:\Users\Stanley\Rafael-IoT-SDK-Internal\toolchain\arm\Windows\
```

### 4. CMake & Ninja

CMake ≥ 3.20 and Ninja are required. Verify:

```bash
cmake --version
ninja --version
```

---

## Environment Setup

Open a shell (Git Bash, MSYS2, or PowerShell) and set these variables **before every
build session**:

```bash
# Zephyr base
export ZEPHYR_BASE=C:/Users/Stanley/zephyrproject/zephyr

# Zephyr Python venv (if using the west venv)
# source C:/Users/Stanley/zephyrproject/.venv/Scripts/activate

# Rafael IoT SDK (optional — defaults to ../Rafael-IoT-SDK-Internal if unset)
export RAFAEL_SDK_BASE=C:/Users/Stanley/Rafael-IoT-SDK-Internal

# ARM toolchain
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=C:/Users/Stanley/Rafael-IoT-SDK-Internal/toolchain/arm/Windows
```

> **Tip:** Save these lines in a `env.sh` file at the repo root and `source env.sh`
> at the start of each session.

---

## Build

### First build (configure + compile)

```bash
cd C:/Users/Stanley/rt582-zephyr-helloworld

west build -b rt582_evb .
```

CMake configures automatically on the first run. The build directory is `build/`.

### Subsequent builds (incremental)

```bash
west build
```

### Clean build

```bash
west build -p always -b rt582_evb .
```

`-p always` (pristine) deletes the existing `build/` directory before configuring.

### Build outputs

| File | Description |
|------|-------------|
| `build/zephyr/zephyr.elf` | ELF with full debug info — use for GDB |
| `build/zephyr/zephyr.bin` | Raw binary — flash directly to 0x00000000 |
| `build/zephyr/zephyr.hex` | Intel HEX — alternative flash format |

---

## Flashing

### Method 1 — OpenOCD (CMSIS-DAP, recommended)

Connect a CMSIS-DAP debug probe (e.g. DAPLink) to the RT582-EVB SWD header.

**Start OpenOCD server:**

```bash
OPENOCD=C:/Users/Stanley/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD
OPENOCD_BIN=$OPENOCD/bin/win/openocd.exe
SCRIPT_DIR=$OPENOCD/script

"$OPENOCD_BIN" \
  -s "$SCRIPT_DIR" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg
```

**Flash and reset (new terminal):**

```bash
OPENOCD=C:/Users/Stanley/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD
OPENOCD_BIN=$OPENOCD/bin/win/openocd.exe
SCRIPT_DIR=$OPENOCD/script
ELF=C:/Users/Stanley/rt582-zephyr-helloworld/build/zephyr/zephyr.elf

"$OPENOCD_BIN" \
  -s "$SCRIPT_DIR" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg \
  -c "program \"$ELF\" verify reset exit"
```

### Method 2 — GDB over OpenOCD

With OpenOCD running (Method 1 server), attach GDB from the toolchain:

```bash
GDB=C:/Users/Stanley/Rafael-IoT-SDK-Internal/toolchain/arm/Windows/bin/arm-none-eabi-gdb.exe
ELF=C:/Users/Stanley/rt582-zephyr-helloworld/build/zephyr/zephyr.elf

"$GDB" "$ELF" \
  -ex "target remote :50000" \
  -ex "monitor reset halt" \
  -ex "load" \
  -ex "monitor reset run" \
  -ex "disconnect" \
  -ex "quit"
```

---

## Serial Console

Connect a USB-UART adapter to the RT582-EVB:

| Signal | RT582 GPIO | Adapter pin |
|--------|-----------|-------------|
| TX     | GPIO16    | RX          |
| RX     | GPIO17    | TX          |
| GND    | GND       | GND         |

Settings: **115200 baud, 8N1, no flow control**

Open the port with any terminal (PuTTY, Tera Term, screen, minicom).
After reset, expected output:

```
*** Booting Zephyr OS build v4.1.0 ***
Starting RT582-EVB Zephyr Hello World
[OS] Zephyr kernel scheduler running
task 1 running
task 2 running
task 1 running
task 2 running
...
```

---

## Project Structure

```
rt582-zephyr-helloworld/
├── CMakeLists.txt              # Top-level build; adds UART driver sources to app target
├── Kconfig                     # Root Kconfig (KCONFIG_ROOT); sources driver Kconfig
├── prj.conf                    # Project config (SERIAL, CONSOLE, UART_RT582, PRINTK)
│
├── src/
│   └── main.c                  # Application: two K_THREAD_DEFINE threads + main()
│
├── drivers/
│   └── serial/
│       ├── uart_rt582.c        # Zephyr UART driver wrapping Rafael HOSAL
│       ├── Kconfig             # CONFIG_UART_RT582 symbol
│       └── CMakeLists.txt
│
├── boards/arm/rt582_evb/
│   ├── board.yml               # Board descriptor (Zephyr HWMv2)
│   ├── rt582_evb.dts           # Board DTS; sets chosen console = uart0
│   ├── rt582_evb.yaml          # Board metadata (toolchain, arch)
│   ├── rt582_evb_defconfig     # Board default Kconfig fragments
│   ├── Kconfig.board           # Declares BOARD_RT582_EVB symbol
│   ├── Kconfig.defconfig       # Board-level Kconfig defaults
│   └── Kconfig.rt582_evb       # Selects SOC_RT582 (required by Zephyr 4.x)
│
├── soc/arm/rafael_micro/
│   ├── CMakeLists.txt
│   ├── Kconfig.series
│   └── rt582/
│       ├── soc.yml             # SoC descriptor (Zephyr HWMv2)
│       ├── Kconfig             # SOC_SERIES_RT582 / SOC_RT582 (selects ARM + CPU_CORTEX_M3)
│       ├── Kconfig.defconfig   # NUM_IRQS=33, SYS_CLOCK_HW_CYCLES_PER_SEC=48000000
│       ├── Kconfig.soc
│       ├── CMakeLists.txt      # Sets SOC_LINKER_SCRIPT; exposes Rafael SDK includes
│       ├── soc.c               # Minimal SoC init stub (SYS_INIT PRE_KERNEL_1)
│       └── soc.h               # Includes Rafael mcu.h for CMSIS integration
│
└── dts/
    ├── arm/rafael/rt582.dtsi   # SoC DTS: flash@0x0/1MB, sram@0x20000000/144KB, uart0/uart1
    └── bindings/
        ├── serial/
        │   └── rafael,rt582-uart.yaml   # DTS binding for UART driver
        └── vendor-prefixes.txt          # Adds "rafael" vendor prefix
```

### Key design decisions

**Driver in `app` target, not a separate `zephyr_library`**
Zephyr device registration macros (`DEVICE_DT_INST_DEFINE`) rely on linker magic that
only works inside `libapp.a`, which is linked with `--whole-archive`. A standalone
`zephyr_library()` would be optimised away. Sources are therefore added via
`target_sources(app PRIVATE ...)` in the top-level `CMakeLists.txt`.

**`soc.c` is a stub**
`system_mcu.c` from the Rafael SDK pulls in `flash_get_deviceinfo`, `mpsectorinit`, and
`change_ahb_system_clk` — functions in other, not-yet-ported SDK modules. The RT582 runs
fine at its power-on default (48 MHz) without calling `SystemInit()`, so the stub is
intentional. Replace it once the flash/clock drivers are ported.

**UART driver — polling only**
`uart_rt582.c` implements `poll_in` / `poll_out` via `hosal_uart_receive` /
`hosal_uart_send`. This is sufficient for `printk` and the hello-world demo.
IRQ-driven RX can be added by wiring the HOSAL `rx_cb` into Zephyr's UART ISR path.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| `west build` fails: `ZEPHYR_BASE not set` | Environment not sourced | Run `export ZEPHYR_BASE=C:/Users/Stanley/zephyrproject/zephyr` |
| `west build` fails: `SOC_RT582 not found` | Wrong BOARD name | Use `-b rt582_evb` (lowercase, underscores) |
| Linker error: `mcu.h not found` | `RAFAEL_SDK_BASE` points to wrong path | Verify the SDK path and that `components/platform/soc/rt582/rt582_driver/Inc/mcu.h` exists |
| Linker error: `hosal_uart.h not found` | Same SDK path issue | Check `RAFAEL_SDK_BASE` |
| No serial output after flashing | TX/RX swapped, or wrong COM port | Swap GPIO16 ↔ GPIO17 wires; verify baud=115200 |
| OpenOCD: `LIBUSB_ERROR_NOT_FOUND` | CMSIS-DAP probe not detected | Install WinUSB driver via Zadig |
| `west flash` not supported | No `board.cmake` flash runner configured | Use the OpenOCD commands in the Flashing section directly |
