# CLAUDE.md — RT582 Zephyr + OpenThread Project

## Project Overview
Zephyr RTOS port for the Rafael Microelectronics RT582 (Cortex-M3) with OpenThread FTD CLI.
The SDK is vendored into `sdk/` — no external `RAFAEL_SDK_BASE` env var needed.

## Build Command
```sh
export ZEPHYR_BASE=/c/Users/Stanley/zephyrproject/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=/c/Users/Stanley/Rafael-IoT-SDK-Internal/toolchain/arm/Windows
# Run from /c/Users/Stanley/zephyrproject (west workspace root)
west build -p always -b rt582_evb /c/Users/Stanley/zephyr-thread
# Binary output: zephyr-thread/build/zephyr/zephyr.bin
```

## Critical Rules (learned the hard way)

### UART
1. **Never call `printk` from an ISR** — ISR context causes spinlock deadlock (single-core Cortex-M3).
2. **`uart_rt582_init` must return 0**, not the return value of `hosal_uart_init`.
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
   `librt582_system.a` configures BBPLL to 64 MHz (`change_ahb_system_clk(SYS_CLK_64MHZ)`).
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
9. **`mac_frame.cpp`** must use the pre-compiled object from `libopenthread_port.a`
   (extracted via `cmake -E ar -x`). Compiling from source produces a `uint32_t` ABI
   mismatch with `libopenthread-ftd.a`.
10. **`--allow-multiple-definition`** is required because `ot_radio.c` and
    `libopenthread-platform-utils-static.a` both define `soft_source_match_table` symbols.

## Project Structure
```
zephyr-thread/
├── CMakeLists.txt          # Top-level; guards subsys/openthread with CONFIG_OPENTHREAD_RT582
├── prj.conf                # Kconfig; CONFIG_UART_INTERRUPT_DRIVEN=n, CONFIG_SHELL disabled
├── Kconfig                 # Top-level Kconfig
├── boards/arm/rt582_evb/   # Board definition
├── dts/arm/rafael/         # rt582.dtsi: UART0@0xA0000000, TX=GPIO16, RX=GPIO17, 115200 baud
├── soc/arm/rafael_micro/rt582/
│   ├── soc.c               # PRE_KERNEL_1 prio 0: calls SystemInit(), wires COMM_SUBSYSTEM IRQ
│   └── CMakeLists.txt      # Links librt582_system.a + librt582_driver.a (always)
├── drivers/serial/
│   └── uart_rt582.c        # Zephyr UART driver using hosal_uart_*
├── sdk/                    # Vendored Rafael IoT SDK
│   ├── lib/                # Pre-built .a libraries
│   └── components/         # Headers and selected source files
└── subsys/openthread/
    ├── CMakeLists.txt      # OT platform sources, SDK library links, compile flags
    └── platform/           # ot_zephyr.c, ot_radio.c, ot_uart.c, hosal_rf_zephyr.c, …
```

## Hardware: RT582 EVB
- **CPU**: ARM Cortex-M3, 64 MHz (BBPLL)
- **Flash**: 1 MB at 0x00008000 (app region; bootloader occupies 0x00000000–0x00007FFF)
- **RAM**: 144 KB at 0x20000000
- **UART0**: 0xA0000000, IRQ=4, TX=GPIO16, RX=GPIO17, 115200 8N1
- **COMM_SUBSYSTEM** (RF MCU): 0xA0400000, IRQ=20
- **Key clock bits in SYSCTRL->sys_clk_ctrl**: UART0_CLK=bit16, UART1_CLK=bit17

## SDK Notes
- `librt582_system.a` was built with `SET_SYS_CLK=SYS_CLK_64MHZ=2` (confirmed by disassembly:
  `movs r0, #2; bl change_ahb_system_clk` in SystemInit).
- `librt582_driver.a` provides `change_ahb_system_clk`, `enable_perclk`, etc.
  We also compile `sysctrl.c` from source (for `hosal_uart_init` dependencies);
  `--allow-multiple-definition` keeps our version.
- `CHIP_TYPE=2` (RT582) must be passed as a compile definition.
  `CHIP_VERSION` defaults to `RT58X_MPB=2` in `chip_define.h` — correct for EVB boards.

## Diagnostics
- **Bare UART test**: Set `CONFIG_OPENTHREAD_RT582=n`, use minimal `main.c` (just printk loop).
  This isolates UART hardware from OT stack issues.
- **OT radio hang debug**: `ot_radio.c::ot_radioInit()` has `printk("[OT-RADIO] ...")` probes
  at each init step. Re-enable with `CONFIG_OPENTHREAD_RT582=y`.
- **Watchdog register dump**: `main.c::wdog_work_handler` dumps COMM_SUBSYSTEM registers
  and NVIC state every 1 s — useful for diagnosing RF MCU lockups.
