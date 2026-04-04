# RT582-EVB — Zephyr + OpenThread FTD

Zephyr RTOS port for the **Rafael Microelectronics RT582** (ARM Cortex-M3 @ 64 MHz) with a full OpenThread FTD CLI.

| Item | Value |
|------|-------|
| SoC | RT582 (ARM Cortex-M3, 64 MHz BBPLL) |
| Zephyr | ≥ 4.4 |
| OpenThread | FTD (Full Thread Device) CLI |
| Console | UART0 — TX: GPIO16, RX: GPIO17, **115200 8N1** |
| Flash usage | ~340 KB / 1 MB (35 %) |
| RAM usage | ~88 KB / 144 KB (61 %) |

---

## 目錄

1. [Prerequisites](#1-prerequisites)
2. [Clone 與 West 工作區設定](#2-clone-與-west-工作區設定)
3. [安裝 Zephyr SDK Toolchain](#3-安裝-zephyr-sdk-toolchain)
4. [設定環境變數](#4-設定環境變數)
5. [編譯](#5-編譯)
6. [燒錄](#6-燒錄)
7. [驗證 — 序列終端機](#7-驗證--序列終端機)
8. [OpenThread CLI 快速上手](#8-openthread-cli-快速上手)
9. [疑難排解](#9-疑難排解)
10. [專案結構](#10-專案結構)

---

## 1. Prerequisites

| 工具 | 最低版本 | 安裝 |
|------|---------|------|
| Python | 3.12 | [python.org](https://www.python.org/) |
| west | 0.14 | `pip install west` |
| CMake | 3.20 | [cmake.org](https://cmake.org/) 或 `pip install cmake` |
| Ninja | 1.10 | [ninja-build.org](https://ninja-build.org/) |
| Git | 任意 | [git-scm.com](https://git-scm.com/) |
| DTC | 1.4.6 | Windows: 含在 Zephyr SDK 內；Linux: `apt install device-tree-compiler` |

> **Windows 注意**：建議使用 PowerShell 7 或 Git Bash。MSYS2 亦可，但路徑需調整為 `/c/...` 格式。

---

## 2. Clone 與 West 工作區設定

本專案使用 **west** 管理 Zephyr 依賴。工作區結構如下：

```
<workspace>/          ← west 工作區根目錄（例如 C:\Users\Stanley）
├── zephyr/           ← Zephyr 原始碼（west 自動下載）
└── zephyr-thread/    ← 本專案
```

### 2.1 初始化工作區（首次）

```powershell
# 建立工作區目錄
mkdir C:\zephyr-workspace
cd C:\zephyr-workspace

# Clone 本專案
git clone https://github.com/<your-org>/zephyr-thread-rt58x.git zephyr-thread

# 以本專案為 manifest 初始化 west 工作區
west init -l zephyr-thread

# 下載 Zephyr（約 500 MB）
west update --narrow

# 安裝 Zephyr Python 依賴
pip install -r zephyr/scripts/requirements-base.txt

# 匯出 Zephyr CMake 套件（讓 find_package(Zephyr) 能找到）
west zephyr-export
```

> `--narrow` 只下載 west.yml 列出的專案，不下載全部 Zephyr module（加速）。

### 2.2 日後更新

```powershell
cd zephyr-thread
git pull

cd ..
west update --narrow
```

---

## 3. 安裝 Zephyr SDK Toolchain

Zephyr SDK 提供官方 ARM 工具鏈（`arm-zephyr-eabi`），推薦使用。

### Windows

1. 從 [Zephyr SDK GitHub Releases](https://github.com/zephyrproject-rtos/sdk-ng/releases) 下載 Windows 安裝包：
   - 選擇最新的 `zephyr-sdk-<version>_windows-x86_64.7z`（或 minimal 版本）

2. 解壓縮至固定路徑（路徑不能有空格），例如：
   ```
   C:\zephyr-sdk-1.0.1\
   ```

3. 執行安裝腳本（Windows CMD）：
   ```cmd
   C:\zephyr-sdk-1.0.1\setup.cmd
   ```
   或在 Git Bash：
   ```bash
   /c/zephyr-sdk-1.0.1/setup.sh -c
   ```

4. 確認 `arm-zephyr-eabi` 工具鏈存在：
   ```
   C:\zephyr-sdk-1.0.1\arm-zephyr-eabi\bin\arm-zephyr-eabi-gcc.exe
   ```

### Linux / macOS

```bash
SDK_VER=1.0.1
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VER}/zephyr-sdk-${SDK_VER}_linux-x86_64.tar.xz
tar xf zephyr-sdk-${SDK_VER}_linux-x86_64.tar.xz -C ~/
~/zephyr-sdk-${SDK_VER}/setup.sh -t arm-zephyr-eabi -c
```

---

## 4. 設定環境變數

每次開啟新的終端機會話都需要設定以下變數。

### PowerShell

```powershell
$env:ZEPHYR_BASE         = "C:/zephyr-workspace/zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:/zephyr-sdk-1.0.1"
```

### Bash / Git Bash

```bash
export ZEPHYR_BASE=/c/zephyr-workspace/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=/c/zephyr-sdk-1.0.1
```

> **提示**：將這三行存成 `env.ps1`（PowerShell）或 `env.sh`（Bash）放在工作區根目錄，每次 `. env.ps1` 或 `source env.sh` 即可。

### 備用：Rafael IoT SDK 工具鏈（gnuarmemb）

若已安裝 Rafael IoT SDK 內附的工具鏈，也可使用：

```powershell
$env:ZEPHYR_TOOLCHAIN_VARIANT  = "gnuarmemb"
$env:GNUARMEMB_TOOLCHAIN_PATH  = "C:/Rafael-IoT-SDK-Internal/toolchain/arm/Windows"
```

---

## 5. 編譯

所有指令從 **west 工作區根目錄** 執行（即 `C:\zephyr-workspace`）。

### Clean build（推薦，首次或清除快取）

```powershell
west build -p always -b rt582_evb zephyr-thread --build-dir zephyr-thread/build
```

### 增量 build（修改程式碼後快速重編）

```powershell
west build -b rt582_evb zephyr-thread --build-dir zephyr-thread/build
```

### 輸出檔案

| 檔案 | 說明 |
|------|------|
| `zephyr-thread/build/zephyr/zephyr.bin` | 原始 binary，直接燒錄至 0x00000000 |
| `zephyr-thread/build/zephyr/zephyr.elf` | ELF，含 debug 資訊，用於 GDB |
| `zephyr-thread/build/zephyr/zephyr.hex` | Intel HEX 格式 |

---

## 6. 燒錄

> **重要**：請燒錄至位址 **`0x00000000`**（Flash 起始位址）。
> Binary 含有自己的 vector table，直接開機——不需要 bootloader。
> 燒錄至 `0x8000` 將導致無輸出（除非有相容 bootloader）。

### 硬體接線（UART Console）

```
RT582-EVB          USB-UART 轉接器
GPIO16 (TX)  ───►  RX
GPIO17 (RX)  ◄───  TX
GND          ───── GND
```

### 使用 OpenOCD（CMSIS-DAP / DAPLink，推薦）

將 CMSIS-DAP debug probe 接至 RT582-EVB 的 SWD 排針。

**啟動 OpenOCD server（終端機 1）：**

```bash
OPENOCD=C:/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD

"$OPENOCD/bin/win/openocd.exe" \
  -s "$OPENOCD/script" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg
```

**燒錄並重置（終端機 2）：**

```bash
OPENOCD=C:/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD
BIN=C:/zephyr-workspace/zephyr-thread/build/zephyr/zephyr.bin

"$OPENOCD/bin/win/openocd.exe" \
  -s "$OPENOCD/script" \
  -f interface/cmsis-dap.cfg \
  -f target/rt58x.cfg \
  -c "init; halt; flash write_image erase \"$BIN\" 0x0; reset run; exit"
```

---

## 7. 驗證 — 序列終端機

以 **115200 baud、8N1、無 flow control** 開啟序列終端機（PuTTY、Tera Term、minicom）。

Reset 後應看到：

```
======================================
  RT582-EVB  Zephyr + OpenThread CLI
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

若看到 `>` prompt，表示 OpenThread CLI 已就緒。

---

## 8. OpenThread CLI 快速上手

```
> state
disabled

# 建立新 Thread 網路
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

# 查看 IPv6 位址
> ipaddr
fd11:22:0:0:...
fe80::...
Done

# 查看鄰居
> neighbor table
Done
```

完整指令請參考 [OpenThread CLI Reference](https://openthread.io/reference/cli).

---

## 9. 疑難排解

| 症狀 | 可能原因 | 解法 |
|------|---------|------|
| `find_package(Zephyr)` 失敗 | `ZEPHYR_BASE` 未設定或路徑錯誤 | 確認 `$env:ZEPHYR_BASE` 指向正確的 `zephyr/` 目錄 |
| `No CMAKE_C_COMPILER` | Toolchain 未設定 | 確認 `ZEPHYR_TOOLCHAIN_VARIANT` 與 SDK 路徑 |
| `west update` 超慢 | 下載整個 Zephyr module | 加上 `--narrow` 參數 |
| 完全沒有 UART 輸出 | Binary 燒錄至錯誤位址 | 燒錄至 **0x0**，不是 `0x8000` |
| `printk` 輸出後系統卡死 | 從 ISR 呼叫 `printk`（spinlock 死鎖） | 使用 `k_work_submit` 延後到 thread context |
| UART RX 中斷從不觸發 | `IRQ_CONNECT` 未呼叫 | 確認 `uart_rt582.c` 中有呼叫 `IRQ_CONNECT` |
| RF init 後 OT task 卡住 | RF MCU init 阻塞 | 確認 `CONFIG_RF_FW_INCLUDE_PCI=TRUE` compile definition |
| `>` prompt 不出現 | OT CLI 初始化失敗 | 確認 `otrInitUser` 有呼叫 `otAppCliInit` |
| OpenOCD: `LIBUSB_ERROR_NOT_FOUND` | CMSIS-DAP 未被識別 | 用 [Zadig](https://zadig.akeo.ie/) 安裝 WinUSB 驅動 |
| CMake warning `drivers__serial No SOURCES` | Zephyr 內部空庫檢查（非錯誤） | 忽略，build 不受影響；driver 已在 `app` 中編譯 |

---

## 10. 專案結構

```
zephyr-thread/
├── CMakeLists.txt              # 根 CMake；平台驅動 + OT include 點
├── Kconfig                     # 根 Kconfig；含 PHY/MAC PIB 設定選單
├── prj.conf                    # Kconfig fragments（UART、heap、OT 開關）
├── west.yml                    # West manifest（Zephyr 依賴）
│
├── subsys/openthread/
│   └── CMakeLists.txt          # OT core、mbedTLS、RUCI、RF 等所有 OT 相關 sources
│
├── src/
│   └── main.c                  # RF init、PIB 設定、OT task 啟動、watchdog thread
│
├── drivers/
│   ├── serial/
│   │   ├── uart_rt582.c        # Zephyr UART driver（HOSAL 封裝）
│   │   ├── Kconfig             # CONFIG_UART_RT582
│   │   └── CMakeLists.txt
│   ├── hosal/rt582_hosal/
│   │   ├── Inc/                # hosal_uart.h、hosal_rf.h 等
│   │   └── Src/
│   │       ├── hosal_uart.c    # HOSAL UART 實作
│   │       ├── hosal_rf.c      # RF MCU 事件 thread（Zephyr k_thread）
│   │       └── hosal_trng.c    # TRNG 封裝
│   └── soc/rt582/
│       ├── rt582_driver/       # SoC 暫存器/時鐘驅動（sysctrl、gpio、dma 等）
│       ├── rt582_system/       # SystemInit（BBPLL → 64 MHz）
│       └── rt582_crypto/       # AES、SHA256、ECJPAKE 等加密實作
│
├── sdk/                        # Vendored Rafael IoT SDK（僅原始碼，無預編譯 .a）
│   └── components/
│       ├── network/thread/     # OpenThread port（ot_radio.c、ot_uart.c 等）
│       ├── network/ruci/       # RUCI 指令/事件
│       ├── network/lmac15p4/   # IEEE 802.15.4 MAC
│       ├── network/rt569-rf/   # RT569 RF MCU 驅動
│       ├── network/rt569-fw/   # RT569 RF 韌體 blob
│       └── utility/            # log、fsm、util_queue 等
│
├── boards/arm/rt582_evb/       # Board 定義（DTS、defconfig、Kconfig）
├── dts/arm/rafael/rt582.dtsi   # SoC DTS（Flash 1 MB、RAM 144 KB、UART0/1）
└── soc/arm/rafael_micro/rt582/ # SOC 定義（soc.c：SystemInit + COMM_SUBSYSTEM IRQ）
```

### 關鍵設計決策

| 決策 | 原因 |
|------|------|
| 全部從原始碼編譯，無預編譯 `.a` | 消除工具鏈升級時的 ABI 不相容問題 |
| `uart_rt582.c` 加至 `app` 而非 `drivers__serial` | Zephyr 空庫檢查在 `find_package` 內部執行，app 為唯一可靠的掛載點 |
| 排除 `soft_source_match_table.c` | `ot_radio.c` 已提供 hardware-backed src match via lmac15p4，避免符號衝突 |
| PIB 常數透過 Kconfig 設定 | 調整 RF 參數只需改 `prj.conf`，不需動應用程式碼 |
| Shell 停用（`CONFIG_SHELL=n`） | Shell 的 TX 中斷路徑與 `printk` polling 競爭，會導致輸出亂碼 |
