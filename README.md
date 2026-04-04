# RT582-EVB Build Guide

Zephyr RTOS + OpenThread FTD for the **RT582-EVB** (Rafael Microelectronics RT582, ARM Cortex-M3 @ 64 MHz).

| Item | Value |
|------|-------|
| SoC | RT582 (ARM Cortex-M3, 64 MHz BBPLL) |
| Zephyr | 4.1.0 |
| OpenThread | FTD (Full Thread Device) |
| Console | UART0 — TX: GPIO16, RX: GPIO17, **115200 8N1** |
| Flash usage | ~373 KB / 1 MB |
| RAM usage | ~90 KB / 144 KB |

---

## Step 1 — Install Git LFS

The prebuilt Rafael SDK libraries in `sdk/lib/*.a` are stored in Git LFS.  
Install Git LFS **before cloning** or run `git lfs pull` after.

```bash
# macOS
brew install git-lfs

# Ubuntu / Debian
sudo apt-get install git-lfs

# Windows — download from https://git-lfs.com
git lfs install
```

> If you already cloned without LFS, run `git lfs pull` inside the repo.

---

## Step 2 — Set Up Zephyr Workspace

```bash
pip install west==1.5.0

# Run from the PARENT directory of this repo
west init -l zephyr-thread          # uses west.yml in this repo
west update --narrow
pip install -r zephyr/scripts/requirements-base.txt
west zephyr-export
```

Workspace layout after init:

```
workspace/
├── zephyr-thread/   ← this repo
├── zephyr/          ← fetched by west
└── .west/
```

---

## Step 3 — Install ARM Toolchain

**Option A — Zephyr SDK (Linux/macOS, minimal ARM only):**

```bash
SDK_VER=0.16.8
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VER}/zephyr-sdk-${SDK_VER}_linux-x86_64_minimal.tar.xz
tar xf zephyr-sdk-${SDK_VER}_linux-x86_64_minimal.tar.xz
zephyr-sdk-${SDK_VER}/setup.sh -t arm-zephyr-eabi -c
```

**Option B — Rafael IoT SDK toolchain (Windows):**

```bash
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=<path-to-Rafael-IoT-SDK>/toolchain/arm/Windows
```

---

## Step 4 — Set Environment Variables

```bash
export ZEPHYR_BASE=<zephyr-workspace>/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=<path-to-Rafael-IoT-SDK>/toolchain/arm/Windows
```

---

## Step 5 — Build

Run from the **workspace root** (parent of `zephyr-thread/`):

```bash
west build -p always -b rt582_evb -d zephyr-thread/build zephyr-thread/
```

Or from inside the repo:

```bash
cd zephyr-thread
west build -p always -b rt582_evb -d build .
```

### Build Output

| File | Description |
|------|-------------|
| `build/zephyr/zephyr.bin` | Raw binary — flash to chip |
| `build/zephyr/zephyr.elf` | ELF with debug info — use with GDB |

---

## Step 6 — Flash

> **Important:** Flash the binary at address **`0x00000000`** (start of flash).  
> The binary contains its own vector table and boots directly — no bootloader required.  
> Flashing at `0x8000` will produce no output unless a compatible bootloader is at `0x0`.

### Hardware Wiring

```
RT582-EVB          USB-UART Adapter
GPIO16 (TX)  ───►  RX
GPIO17 (RX)  ◄───  TX
GND          ───── GND
```

### Using OpenOCD (CMSIS-DAP / DAPLink)

**1. Start OpenOCD server:**

```bash
OPENOCD=<path-to-Rafael-IoT-SDK>/tools/Debugger/OpenOCD

"$OPENOCD/bin/win/openocd.exe" \
  -s "$OPENOCD/script" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg
```

**2. Flash (new terminal):**

```bash
OPENOCD=<path-to-Rafael-IoT-SDK>/tools/Debugger/OpenOCD
BIN=zephyr-thread/build/zephyr/zephyr.bin

"$OPENOCD/bin/win/openocd.exe" \
  -s "$OPENOCD/script" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg \
  -c "init; halt; flash write_image erase $BIN 0x0; reset run; exit"
```

---

## Step 7 — Verify

Open a serial terminal at **115200 baud, 8N1, no flow control**.  
After reset you should see:

```
======================================
  RT582-EVB  Zephyr + OpenThread CLI
======================================
[MAIN] wdog started
[UART] RX interrupt enabled
OpenThread FTD task started.
[UART] otPlatUartEnable: dev=...
[CLI] otAppCliInit called, ...
[CLI] otCliInit done
>
```

Type OpenThread CLI commands with or without the `ot` prefix:

```
> ot state
disabled
> ot dataset init new
Done
> ot dataset commit active
Done
> ot ifconfig up
Done
> ot thread start
Done
> ot state
leader
```

---

## CI

GitHub Actions builds `rt582_evb` on every push and pull request.  
See [`.github/workflows/build.yml`](.github/workflows/build.yml).

Built artifacts (`zephyr.bin`, `zephyr.elf`) are uploaded per run.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| `sdk/lib/*.a` are 134-byte pointer files | Git LFS not installed | Run `git lfs install` then `git lfs pull` |
| No UART output at all | Binary flashed at wrong address | Flash at **0x0**, not `0x8000` |
| No UART output after a few seconds | `printk` called from ISR deadlocks | Never call `printk` from ISR; use `k_work_submit` |
| UART RX interrupt never fires | `hosal_uart_ioctl` called with `&mode` instead of `(void*)(uintptr_t)mode` | Check `uart_rt582_irq_rx_enable` |
| OT task hangs after RF init | RF MCU init blocking | Check `RF TRAP` or `init fw fail` output; verify `CONFIG_RF_FW_INCLUDE_PCI=TRUE` |
| `rf_common_init_by_fw` returns false | Missing RUCI firmware define | Add `-DCONFIG_RF_FW_INCLUDE_PCI=TRUE` to compile definitions |
| CLI `>` prompt never appears | `otCliInit` called with NULL callback | Ensure `cli_output_cb` is passed to `otCliInit` |
| OpenOCD: `LIBUSB_ERROR_NOT_FOUND` | CMSIS-DAP not detected | Install WinUSB driver via Zadig |

---

## Project Structure

```
zephyr-thread/
├── CMakeLists.txt                   # Top-level build (OT platform inlined)
├── Kconfig                          # Root Kconfig (CONFIG_OPENTHREAD_RT582)
├── prj.conf                         # Project Kconfig fragments
├── west.yml                         # West manifest (Zephyr v4.1.0)
│
├── sdk/                             # Vendored Rafael IoT SDK
│   ├── lib/                         # Prebuilt .a libraries (Git LFS)
│   └── components/network/
│       ├── thread/openthread_port/  # OT platform: ot_zephyr.c, ot_uart.c, ot_radio.c, …
│       ├── lmac15p4/Src/            # lmac15p4.c (Zephyr k_sem wrapper)
│       ├── rt569-rf/                # RF MCU headers
│       ├── rt569-fw/                # RF firmware headers
│       └── ruci/                    # RUCI command/event headers
│
├── src/
│   └── main.c                       # RF init, OT task start, watchdog thread
│
├── drivers/
│   ├── serial/
│   │   └── uart_rt582.c             # Zephyr UART driver (HOSAL wrapper)
│   ├── hosal/rt582_hosal/
│   │   ├── Inc/                     # hosal_uart.h, hosal_rf.h, …
│   │   └── Src/
│   │       ├── hosal_uart.c         # HOSAL UART implementation
│   │       └── hosal_rf.c           # RF MCU event thread (Zephyr k_thread)
│   └── soc/rt582/rt582_driver/      # SoC register/clock driver
│       ├── Inc/                     # mcu.h, sysctrl.h, uart_drv.h, …
│       └── Src/                     # sysctrl.c
│
├── boards/arm/rt582_evb/            # Board definition
├── dts/arm/rafael/rt582.dtsi        # SoC DTS (flash@0x0, SRAM, UART0/1)
│
└── soc/arm/rafael_micro/rt582/
    ├── soc.c                        # SystemInit (BBPLL 64 MHz) + CommSubsystem IRQ
    └── soc.h
```
