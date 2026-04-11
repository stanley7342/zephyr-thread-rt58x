# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview
Zephyr RTOS port for the Rafael Microelectronics RT583 (Cortex-M3) with OpenThread FTD CLI.
The SDK is vendored into `sdk/` — no external `RAFAEL_SDK_BASE` env var needed.

## Setup and Build

> See [`docs/BUILD_GUIDE.md`](docs/BUILD_GUIDE.md) for the full step-by-step guide.

**First-time setup (Windows):**
```powershell
# Bootstrap from the web (no prior clone needed):
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex

# Or, if already cloned:
.\scripts\windows\install.ps1
```

**First-time setup (Linux/WSL):**
```sh
bash scripts/linux/install.sh
```

**First-time setup (macOS):**
```sh
bash scripts/macos/install.sh
```

**Load environment** (once per terminal, Windows):
```powershell
cd zephyr-thread-rt58x
. .\env.ps1
```

**Load environment** (once per terminal, Linux/macOS):
```sh
source ../env.sh
```

**Build (Windows — bootloader first, then app):**
```powershell
west build -p always -b rt583_evb ../bootloader/mcuboot/boot/zephyr `
    -d build/bootloader `
    -- -DOVERLAY_CONFIG="examples/bootloader/mcuboot.conf" `
       -DDTC_OVERLAY_FILE="examples/bootloader/mcuboot.overlay"

west build -p always -b rt583_evb examples/matter/lighting-app -d build/lighting-app
```

**Build (Linux/WSL):**
```sh
west build -p always -b rt583_evb examples/thread
```

**Flash (Windows via OpenOCD + CMSIS-DAP):**
```powershell
west flash -d build/bootloader
west flash -d build/lighting-app
```

**Flash (WSL/Linux via OpenOCD + CMSIS-DAP):**
```sh
bash scripts/linux/flash.sh -p thread        # slot0 (0x10000)
bash scripts/linux/flash.sh --setup-udev     # first-time udev setup
```

**CI:** `.github/workflows/build.yml` — caches `zephyr/` and `modules/` keyed on `west.yml`.

## Critical Rules (learned the hard way)

### UART
1. **Never call `printk` from an ISR** — ISR context causes spinlock deadlock (single-core Cortex-M3).
2. **`uart_rt583_init` must return 0**, not the return value of `hosal_uart_init`.
   `hosal_uart_init` returns `uart->RBR & 0xFF` (RX FIFO flush) — not an error code.
   A non-zero return makes Zephyr mark the device not-ready and `uart_console_init` skips
   the printk hook silently.
3. **Never call `hosal_uart_send_complete`** from `poll_out` or `fifo_fill`.
   It polls `LSR.TEMT` (TX shift register empty), which hangs forever if the UART
   peripheral clock is not stable. `hosal_uart_send` already polls `LSR.THRE` (safer).
4. **Shell must stay disabled** (`CONFIG_SHELL=n`). Shell sends TX via interrupt path
   concurrently with `printk` polling, corrupting output.

### System Clock
5. **`SystemInit()` in `soc.c` is mandatory** (PRE_KERNEL_1, priority 0).
   `librt583_system.a` configures BBPLL to 64 MHz (`change_ahb_system_clk(SYS_CLK_64MHZ)`).
   Without it, the UART peripheral clock is absent and `hosal_uart_send` spins on `LSR.THRE`.
   All hosal baud-rate divisors are calibrated for 64 MHz (e.g., 115200 → DLL=35).

### IRQ Setup
6. **`IRQ_CONNECT` must be called** (not just `NVIC_EnableIRQ`) so Zephyr's ISR dispatch
   table is populated. Without it, `z_irq_spurious` fires on UART RX.
7. **`hosal_uart_ioctl(MODE_SET)`** takes the mode VALUE cast to `void*`, not a pointer.
   Pass `(void*)(uintptr_t)HOSAL_UART_MODE_INT_RX`, not `&mode`.

### OpenThread
8. **`CONFIG_RF_FW_INCLUDE_PCI=TRUE`** must be defined when compiling `rf_common_init.c`
   from source (enables the RUCI/MAC firmware load path; guarded by `#if` in the source).
9. **`mac_frame.cpp`** is compiled from source (`openthread_port/utils/mac_frame.cpp`).
   All TUs share the same toolchain so `uint32_t` ABI is consistent; no pre-compiled
   object needed.
10. **`soft_source_match_table.c` is excluded from the build.** `ot_radio.c` provides
    hardware-backed `otPlatRadio*SrcMatch*` via `lmac15p4` directly and no longer calls
    `utilsSoftSrcMatchSetPanId`; the duplicate symbols and `--allow-multiple-definition`
    are gone.

## Project Structure
```
zephyr-thread-rt58x/
├── CMakeLists.txt          # Top-level; guards subsys/openthread with CONFIG_OPENTHREAD_RT583
├── prj.conf                # Kconfig; CONFIG_UART_INTERRUPT_DRIVEN=y, CONFIG_SHELL disabled
├── Kconfig                 # Top-level Kconfig
├── boards/arm/rt583_evb/   # Board definition
├── dts/arm/rafael/         # rt583.dtsi: UART0@0xA0000000, TX=GPIO16, RX=GPIO17, 115200 baud
├── soc/arm/rafael_micro/rt583/
│   ├── soc.c               # PRE_KERNEL_1 prio 0: calls SystemInit(), wires COMM_SUBSYSTEM IRQ
│   └── CMakeLists.txt      # Links librt583_system.a + librt583_driver.a (always)
├── drivers/
│   ├── serial/
│   │   └── uart_rt583.c        # Zephyr UART driver using hosal_uart_*
│   ├── hosal/rt583_hosal/      # HOSAL platform layer (moved from sdk/components/platform/hosal/)
│   │   ├── Inc/                # hosal_uart.h, hosal_trng.h, etc.
│   │   └── Src/                # hosal_uart.c, hosal_trng.c
│   └── soc/rt583/rt583_driver/ # SOC register/clock driver (moved from sdk/components/platform/soc/)
│       ├── Inc/                # mcu.h, sysctrl.h, uart_drv.h, etc.
│       └── Src/                # sysctrl.c
├── sdk/                    # Vendored Rafael IoT SDK (source only — no pre-built libs)
│   └── components/         # Network stack sources (OT, RF, RUCI, utility, log)
└── subsys/openthread/
    ├── CMakeLists.txt      # OT platform sources, SDK library links, compile flags
    └── platform/           # ot_zephyr.c, ot_radio.c, ot_uart.c, hosal_rf_zephyr.c, …
```

## Hardware: RT583 EVB
- **CPU**: ARM Cortex-M3, 64 MHz (BBPLL)
- **Flash**: 1 MB at 0x00008000 (app region; bootloader occupies 0x00000000–0x00007FFF)
- **RAM**: 144 KB at 0x20000000
- **UART0**: 0xA0000000, IRQ=4, TX=GPIO16, RX=GPIO17, 115200 8N1
- **COMM_SUBSYSTEM** (RF MCU): 0xA0400000, IRQ=20
- **Key clock bits in SYSCTRL->sys_clk_ctrl**: UART0_CLK=bit16, UART1_CLK=bit17

## SDK Notes
- `librt583_system.a` was built with `SET_SYS_CLK=SYS_CLK_64MHZ=2` (confirmed by disassembly:
  `movs r0, #2; bl change_ahb_system_clk` in SystemInit).
- `librt583_driver.a` provides `change_ahb_system_clk`, `enable_perclk`, etc.
  We also compile `sysctrl.c` from source (for `hosal_uart_init` dependencies);
  `--allow-multiple-definition` keeps our version.
- `CHIP_TYPE=2` (RT583) must be passed as a compile definition.
  `CHIP_VERSION` defaults to `RT58X_MPB=2` in `chip_define.h` — correct for EVB boards.

## RF Parameter Tuning
PHY/MAC PIB constants (CCA threshold, backoff, retries, etc.) are configurable via Kconfig
in `prj.conf` — prefixed `CONFIG_RT583_PHY_PIB_*` and `CONFIG_RT583_MAC_PIB_*`.
See `Kconfig` for ranges and defaults. No application code changes needed.

## Diagnostics
- **Bare UART test**: Set `CONFIG_OPENTHREAD_RT583=n`, use minimal `main.c` (just printk loop).
  This isolates UART hardware from OT stack issues.
- **OT radio hang debug**: `ot_radio.c::ot_radioInit()` has `printk("[OT-RADIO] ...")` probes
  at each init step. Re-enable with `CONFIG_OPENTHREAD_RT583=y`.
- **Watchdog register dump**: `main.c::wdog_work_handler` dumps COMM_SUBSYSTEM registers
  and NVIC state every 1 s — useful for diagnosing RF MCU lockups.
