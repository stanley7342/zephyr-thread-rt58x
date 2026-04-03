# RT582-EVB — Zephyr + OpenThread CLI

Zephyr RTOS port of the Rafael IoT SDK OpenThread FTD stack for the **RT582-EVB** development board.  
OT CLI commands are entered directly over UART0 (no shell prefix needed).

| Item | Value |
|------|-------|
| SoC | Rafael Microelectronics RT582 (ARM Cortex-M3, **64 MHz** BBPLL) |
| Zephyr version | 4.1.0 |
| OpenThread | FTD (Full Thread Device) |
| Console | UART0 — TX: GPIO16, RX: GPIO17, **115200 8N1** |
| Flash usage | ~576 KB / 1 MB |
| RAM usage | ~127 KB / 144 KB |

---

## Hardware Setup

```
RT582-EVB          USB-UART Adapter
GPIO16 (TX)  ───►  RX
GPIO17 (RX)  ◄───  TX
GND          ───── GND
```

Terminal settings: **115200 baud, 8N1, no flow control**.

---

## Prerequisites

### 1. Git LFS

The prebuilt Rafael SDK libraries in `sdk/lib/*.a` are stored in Git LFS.  
Install Git LFS before cloning:

```bash
# macOS
brew install git-lfs

# Ubuntu / Debian
sudo apt-get install git-lfs

# Windows — download from https://git-lfs.com
git lfs install
```

Then clone normally — LFS files are downloaded automatically.

### 2. Zephyr RTOS 4.1.0

```bash
pip install west==1.5.0
west init -l zephyr-thread          # uses west.yml in this repo
west update --narrow
pip install -r zephyr/scripts/requirements-base.txt
west zephyr-export
```

> `west init -l zephyr-thread` should be run from the **parent** of this repo,
> so the workspace layout becomes:
> ```
> workspace/
> ├── zephyr-thread/   ← this repo
> ├── zephyr/          ← fetched by west
> └── .west/
> ```

### 3. ARM Toolchain

Install the **Zephyr SDK** (recommended) or any `arm-none-eabi-gcc` ≥ 12.

**Zephyr SDK (minimal ARM only):**

```bash
SDK_VER=0.16.8
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VER}/zephyr-sdk-${SDK_VER}_linux-x86_64_minimal.tar.xz
tar xf zephyr-sdk-${SDK_VER}_linux-x86_64_minimal.tar.xz
zephyr-sdk-${SDK_VER}/setup.sh -t arm-zephyr-eabi -c
```

**Rafael IoT SDK toolchain (Windows):**

```bash
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=C:/Users/Stanley/Rafael-IoT-SDK-Internal/toolchain/arm/Windows
```

---

## Build

```bash
# From the workspace root (parent of zephyr-thread/)
west build -p always -b rt582_evb -d zephyr-thread/build zephyr-thread/
```

Or from inside the repo:

```bash
cd zephyr-thread
west build -p always -b rt582_evb -d build .
```

### Output

| File | Description |
|------|-------------|
| `build/zephyr/zephyr.bin` | Raw binary — flash to chip |
| `build/zephyr/zephyr.elf` | ELF with debug info — use with GDB |

---

## Flashing

> **Important:** Flash the binary at address **`0x00000000`** (the start of flash).
> The binary contains its own vector table at 0x0 and boots directly — no external
> bootloader is required.  Flashing at 0x8000 will produce no output unless a
> compatible bootloader is present at 0x0.

### OpenOCD (CMSIS-DAP / DAPLink)

**Start OpenOCD server:**

```bash
OPENOCD=C:/Users/Stanley/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD

"$OPENOCD/bin/win/openocd.exe" \
  -s "$OPENOCD/script" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg
```

**Flash (new terminal):**

```bash
OPENOCD=C:/Users/Stanley/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD
BIN=zephyr-thread/build/zephyr/zephyr.bin

"$OPENOCD/bin/win/openocd.exe" \
  -s "$OPENOCD/script" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg \
  -c "init; halt; flash write_image erase $BIN 0x0; reset run; exit"
```

---

## Usage

After reset, the boot log appears:

```
======================================
  RT582-EVB  Zephyr + OpenThread CLI
======================================
[RF] hosal_rf_init...
[RF] hosal_rf_init done
[RF] lmac15p4_init done
[RF] PIB set done
OpenThread FTD task started.
```

Watchdog prints RF/COMM subsystem diagnostics every second:

```
[WDG] t=1s IRQ=3 TX=0x00000100 ... MCU=0x01
```

Type OpenThread CLI commands **directly** (no `ot ` prefix):

```
> state
disabled
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

## Project Structure

```
zephyr-thread/
├── CMakeLists.txt                   # Top-level build
├── Kconfig                          # Root Kconfig
├── prj.conf                         # Project Kconfig fragments
├── west.yml                         # West manifest (Zephyr v4.1.0)
│
├── sdk/                             # Vendored Rafael IoT SDK
│   ├── lib/                         # Prebuilt .a libraries (Git LFS)
│   └── components/                  # Headers + selected source files
│
├── src/
│   └── main.c                       # RF init, OT task start, watchdog
│
├── drivers/serial/
│   └── uart_rt582.c                 # Zephyr UART driver (HOSAL wrapper)
│
├── boards/arm/rt582_evb/            # Board definition
├── dts/arm/rafael/rt582.dtsi        # SoC DTS (flash@0x0, SRAM, UART0/1)
│
├── soc/arm/rafael_micro/rt582/
│   ├── soc.c                        # SystemInit (BBPLL 64 MHz) + CommSubsystem IRQ
│   └── soc.h
│
└── subsys/openthread/
    ├── CMakeLists.txt               # Links Rafael SDK static libs
    └── platform/
        ├── ot_zephyr.c              # OT thread, semaphore, mutex
        ├── ot_uart.c                # UART RX → otCliInputLine
        ├── ot_radio.c               # 802.15.4 radio platform (lmac15p4)
        ├── ot_alarm.c               # ms / µs alarms
        ├── ot_entropy.c             # RNG entropy
        ├── hosal_rf_zephyr.c        # RF MCU event thread
        └── freertos_shim.c          # FreeRTOS API stubs
```

---

## CI

GitHub Actions builds `rt582_evb` on every push and pull request.  
See [`.github/workflows/build.yml`](.github/workflows/build.yml).

The built `zephyr.bin` and `zephyr.elf` are uploaded as workflow artifacts.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|-------------|-----|
| No UART output at all | Binary flashed at wrong address | Flash at **0x0**, not 0x8000 |
| No UART output after a few seconds | `printk` called from ISR deadlocks | Never call `printk` from ISR; use `k_work_submit` |
| UART RX interrupt never fires | `hosal_uart_ioctl` called with `&mode` instead of `(void*)(uintptr_t)mode` | Check `uart_rt582_irq_rx_enable` — must cast value, not pass address |
| OT task hangs at `ot_radioInit` | RF MCU init blocking | Check `[OT-RADIO]` debug prints in `ot_radio.c` for exact hang point |
| `rf_common_init_by_fw` returns false | Missing RUCI firmware define | Add `-DCONFIG_RF_FW_INCLUDE_PCI=TRUE` to compile definitions |
| OpenOCD: `LIBUSB_ERROR_NOT_FOUND` | CMSIS-DAP not detected | Install WinUSB driver via Zadig |
| `sdk/lib/*.a` are 134-byte pointer files | Git LFS not installed | Run `git lfs install` then `git lfs pull` |
