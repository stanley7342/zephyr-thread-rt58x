# RT582-EVB — Zephyr + OpenThread CLI

Zephyr RTOS port of the Rafael IoT SDK OpenThread FTD stack for the **RT582-EVB** development board.  
OT CLI commands are entered directly over UART0 (no shell prefix needed).

| Item | Value |
|------|-------|
| SoC | Rafael Microelectronics RT582 (ARM Cortex-M3, 48 MHz) |
| Zephyr version | 4.1.0 |
| Toolchain | Arm GNU Toolchain 14.2.1 (bundled with Rafael IoT SDK) |
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

### 1. Rafael IoT SDK

Must be present at:
```
C:\Users\Stanley\Rafael-IoT-SDK-Internal\
```
Override with `RAFAEL_SDK_BASE` environment variable.

### 2. Zephyr RTOS 4.1.0

Must be present at `C:\Users\Stanley\zephyrproject\`.

```bash
pip install west
west init C:/Users/Stanley/zephyrproject --mr v4.1.0
cd C:/Users/Stanley/zephyrproject
west update
pip install -r zephyr/scripts/requirements.txt
```

### 3. ARM Toolchain

Uses the toolchain bundled with the Rafael IoT SDK:
```
C:\Users\Stanley\Rafael-IoT-SDK-Internal\toolchain\arm\Windows\
```

---

## Environment Setup

Run once per shell session before building:

```bash
export ZEPHYR_BASE=C:/Users/Stanley/zephyrproject/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
export GNUARMEMB_TOOLCHAIN_PATH=C:/Users/Stanley/Rafael-IoT-SDK-Internal/toolchain/arm/Windows

# Optional — defaults to ../Rafael-IoT-SDK-Internal
export RAFAEL_SDK_BASE=C:/Users/Stanley/Rafael-IoT-SDK-Internal
```

> Save these in `env.sh` at the repo root and `source env.sh` at the start of each session.

---

## Build

### First build

```bash
cd C:/Users/Stanley/zephyr-thread

cmake -B build -DBOARD=rt582_evb -GNinja .
cd build && ninja
```

### Incremental build

```bash
cd C:/Users/Stanley/zephyr-thread/build && ninja
```

### Clean build

```bash
cd C:/Users/Stanley/zephyr-thread
rm -rf build
cmake -B build -DBOARD=rt582_evb -GNinja .
cd build && ninja
```

### Output

| File | Description |
|------|-------------|
| `build/zephyr/zephyr.bin` | Raw binary — flash to chip |
| `build/zephyr/zephyr.elf` | ELF with debug info — use with GDB |

---

## Flashing

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
ELF=C:/Users/Stanley/zephyr-thread/build/zephyr/zephyr.elf

"$OPENOCD/bin/win/openocd.exe" \
  -s "$OPENOCD/script" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg \
  -c "program \"$ELF\" verify reset exit"
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
Type OT commands directly, e.g.: state
```

Then the watchdog prints RF/COMM subsystem diagnostics every second:

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
> ipaddr
fd11:22::/64
Done
```

---

## Project Structure

```
zephyr-thread/
├── CMakeLists.txt                   # Top-level build
├── Kconfig                          # Root Kconfig
├── prj.conf                         # Project Kconfig fragments
│
├── src/
│   └── main.c                       # RF init, OT task start, watchdog
│
├── drivers/serial/
│   ├── uart_rt582.c                 # Zephyr UART driver (HOSAL wrapper)
│   ├── Kconfig                      # CONFIG_UART_RT582
│   └── CMakeLists.txt
│
├── boards/arm/rt582_evb/
│   ├── rt582_evb.dts                # Board DTS (chosen console = uart0)
│   └── rt582_evb_defconfig          # Board default Kconfig
│
├── dts/
│   ├── arm/rafael/rt582.dtsi        # SoC DTS (flash, SRAM, uart0/1)
│   └── bindings/serial/
│       └── rafael,rt582-uart.yaml   # UART driver DTS binding
│
├── soc/arm/rafael_micro/rt582/
│   ├── soc.c                        # SystemInit + CommSubsystem IRQ_CONNECT
│   └── soc.h                        # Includes Rafael mcu.h (CMSIS)
│
└── subsys/openthread/
    ├── CMakeLists.txt               # Links Rafael SDK static libs
    └── platform/
        ├── openthread_port.h        # FreeRTOS → Zephyr primitive mapping
        ├── ot_zephyr.c              # OT thread, semaphore, mutex
        ├── ot_uart.c                # UART RX → otCliInputLine (direct CLI)
        ├── ot_radio.c               # 802.15.4 radio platform (lmac15p4)
        ├── ot_alarm.c               # Millisecond / microsecond alarms
        ├── ot_entropy.c             # RNG entropy source
        ├── hosal_rf_zephyr.c        # RF MCU event thread (FreeRTOS → Zephyr)
        └── freertos_shim.c          # FreeRTOS API stubs (unused SDK paths)
```

---

## Architecture

```
main()
  ├── hosal_rf_init()          RF MCU firmware load + event thread start
  ├── lmac15p4_init()          802.15.4 MAC init
  └── otrStart()               Spawns OT thread (priority 5)

OT thread (otrStackTask)
  ├── ot_entropy_init()
  ├── ot_alarmInit()
  ├── ot_radioInit()           lmac15p4 callbacks, channel, ACK settings
  ├── otInstanceInitSingle()
  └── otrInitUser()
        └── otAppCliInit()
              ├── otPlatUartEnable()   Installs UART RX interrupt callback
              └── otCliInit()          Registers CLI output → uart_poll_out

UART RX ISR
  └── ot_uart_irq_cb()         Accumulates chars → on CR/LF signals OT thread

OT thread event loop
  └── ot_uartTask()
        └── otCliInputLine()   Parses command, produces output via cli_output_cb

RF event thread (hosal-rf, priority 2)
  └── Handles CommSubsystem IRQ callbacks (RX done, TX done, ACK, etc.)
```

---

## Key Design Notes

### UART driver (`uart_rt582.c`)
- Wraps the Rafael HOSAL UART (`hosal_uart.c`) as a Zephyr driver
- `IRQ_CONNECT` registers the UART ISR in Zephyr's dispatch table (required — raw `NVIC_EnableIRQ` alone causes `z_irq_spurious` panic)
- `hosal_uart_init()` returns `uart->RBR & 0xFF` (RX flush), **not** an error code — the init function always returns `0` to prevent Zephyr from marking the device not-ready
- `hosal_uart_ioctl(HOSAL_UART_MODE_SET, ...)` treats `p_arg` as the **value** cast to `void*`, not a pointer — pass `(void*)(uintptr_t)HOSAL_UART_MODE_INT_RX`, not `&mode`
- **No `printk` anywhere in the driver** — calling `printk` from an ISR while the main thread holds the printk spinlock causes a single-core deadlock

### Watchdog timer (`main.c`)
- Timer expiry runs in SysTick ISR context — **never call `printk` directly**
- Uses `k_work_submit()` to defer printing to the system workqueue (thread context)

### OT thread (`ot_zephyr.c`)
- FreeRTOS `ulTaskNotifyTake` → `k_sem_take(&ot_task_sem, K_MSEC(10))`
- FreeRTOS `vTaskNotifyGiveFromISR` → `k_sem_give(&ot_task_sem)` (ISR-safe in Zephyr)
- `OT_THREAD_SAFE(...)` macro wraps calls with `k_mutex_lock(&ot_ext_lock)`

### RF platform (`hosal_rf_zephyr.c`)
- `rf_common_init_by_fw()` requires `CommSubsystem_IRQn` (IRQ 20) to be connected **before** it is called — done in `soc.c` at `PRE_KERNEL_1` priority 0
- `CONFIG_RF_FW_INCLUDE_PCI=TRUE` must be defined to include the RUCI firmware blob

---

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| No UART output at all | `uart_rt582_init` returns non-zero → device not ready → printk hook not installed | Verify init function returns `0`, not `hosal_uart_init()` return value |
| No UART output after a few seconds | `printk` called from ISR (timer callback, UART ISR) deadlocks with main thread | Never call `printk` from ISR context; use `k_work_submit` |
| UART RX interrupt never fires | HOSAL ioctl bug: passing `&mode` instead of `(void*)(uintptr_t)mode` | Check `uart_rt582_irq_rx_enable` — must use cast, not address |
| OT task hangs at `ot_radioInit` | `flash_read_sec_register` or `lmac15p4_*` blocking | See `[OT-RADIO]` debug prints in `ot_radio.c` to identify exact hang point |
| `rf_common_init_by_fw` returns false | Missing RUCI firmware blob | Add `CONFIG_RF_FW_INCLUDE_PCI=TRUE` to compile definitions |
| OpenOCD: `LIBUSB_ERROR_NOT_FOUND` | CMSIS-DAP not detected | Install WinUSB driver via Zadig |
| Linker: `z_irq_spurious` at runtime | UART IRQ enabled but no `IRQ_CONNECT` | Ensure `IRQ_CONNECT` is called in the per-instance init function |
