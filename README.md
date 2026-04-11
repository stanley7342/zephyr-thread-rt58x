# RT583-EVB — Zephyr + OpenThread FTD

Zephyr RTOS port for the **Rafael Microelectronics RT583** (ARM Cortex-M3 @ 64 MHz) with a full OpenThread FTD CLI.

| Item | Value |
|------|-------|
| SoC | RT583 (ARM Cortex-M3, 64 MHz BBPLL) |
| Zephyr | 4.4.0-rc1（`v4.4.0-rc1-211-gcf4d0f72478e`） |
| OpenThread | FTD (Full Thread Device) CLI |
| Console | UART0 — TX: GPIO16, RX: GPIO17, **115200 8N1** |
| Flash usage | ~340 KB / 1 MB (35 %) |
| RAM usage | ~88 KB / 144 KB (61 %) |

---

## 目錄

1. [Prerequisites](#1-prerequisites)
2. [快速開始（自動化腳本）](#2-快速開始自動化腳本)
3. [Clone 與 West 工作區設定](#3-clone-與-west-工作區設定)
4. [安裝 Zephyr SDK Toolchain](#4-安裝-zephyr-sdk-toolchain)
5. [設定環境變數](#5-設定環境變數)
6. [編譯](#6-編譯)
7. [燒錄](#7-燒錄)
8. [驗證 — 序列終端機](#8-驗證--序列終端機)
9. [OpenThread CLI 快速上手](#9-openthread-cli-快速上手)
10. [疑難排解](#10-疑難排解)
11. [專案結構](#11-專案結構)

---

## 1. Prerequisites

### Windows

| 工具 | 最低版本 | 說明 |
|------|---------|------|
| PowerShell | 7.0 | `winget install Microsoft.PowerShell` |
| Python | 3.12 | `winget install Python.Python.3.12` |
| west | 0.14 | `pip install west` |
| CMake | 3.20 | `winget install Kitware.CMake` |
| Ninja | 1.10 | `winget install Ninja-build.Ninja` |
| Git | 任意 | `winget install Git.Git` |
| 7-Zip | 任意 | `winget install 7zip.7zip`（解壓縮 SDK `.7z` 用） |
| DTC | — | Zephyr SDK 1.0.1 Windows 版已內建，無需另外安裝 |

### Linux / WSL

```bash
sudo apt-get install git cmake ninja-build python3 python3-pip wget xz-utils
pip install west
```

### macOS

```bash
brew install git cmake ninja python@3.12 wget xz
pip3 install west
```

> **注意**：macOS 需先安裝 [Homebrew](https://brew.sh)。  
> **注意**：腳本會自動偵測 Apple Silicon（`aarch64`）或 Intel（`x86_64`）並下載對應 SDK。

---

## 2. 快速開始（自動化腳本）

三支腳本分別對應安裝、編譯、移除，依平台放在對應子目錄：

```
scripts/
├── windows/   install.ps1  build.ps1  uninstall.ps1
├── linux/     install.sh   build.sh   uninstall.sh
└── macos/     install.sh   build.sh   uninstall.sh
```

### Windows（PowerShell 7+，以系統管理員執行）

**一鍵安裝（不需先 clone）：**

```powershell
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex
```

或手動 clone 後安裝：

```powershell
git clone https://github.com/stanley7342/zephyr-thread-rt58x.git
Set-Location zephyr-thread-rt58x
.\scripts\windows\install.ps1
```

安裝完成後編譯：

```sh
west build -p always -b rt583_evb examples/matter/lighting-app
```

### Linux / WSL（Bash）

```bash
git clone https://github.com/stanley7342/zephyr-thread-rt58x.git
cd zephyr-thread-rt58x

bash scripts/linux/install.sh
bash scripts/linux/build.sh

# 增量編譯
bash scripts/linux/build.sh --incremental
```

### macOS（Bash）

```bash
git clone https://github.com/stanley7342/zephyr-thread-rt58x.git
cd zephyr-thread-rt58x

bash scripts/macos/install.sh
bash scripts/macos/build.sh

# 增量編譯
bash scripts/macos/build.sh --incremental
```

安裝完成後，**專案父目錄**會產生環境檔，之後每次開啟新 shell 執行一次即可載入環境：

| 平台 | 指令 |
|------|------|
| Windows | `. <workspace>\env.ps1` |
| Linux / macOS | `source <workspace>/env.sh` |

### install 參數

| 平台 | 參數 | 說明 |
|------|------|------|
| Windows | `-SdkDir <路徑>` | SDK 安裝目錄（預設：`C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1`） |
| Windows | `-Bg` | 背景執行，log 輸出至 `<workspace>\install.log` |
| Linux/macOS | `--sdk-dir <路徑>` | SDK 安裝目錄（預設：`~/zephyr-sdk-1.0.1`） |

### 移除環境

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

腳本會顯示即將刪除的項目並要求確認（`y`）：

| 項目 | 動作 |
|------|------|
| `.west` 目錄 | 整個目錄刪除 |
| `zephyr`（Zephyr 原始碼） | 整個目錄刪除 |
| `env.ps1` / `env.sh` | 刪除 |
| Zephyr SDK 目錄 | 整個目錄刪除 |
| `west`（pip 套件） | `pip uninstall west` |
| Python / CMake / Ninja 等系統工具 | Windows：`winget uninstall`（含 Registry fallback）；Linux/macOS：僅顯示，需手動移除 |

> 若想重新安裝，重新執行 `install` 腳本即可。

---

## 3. Clone 與 West 工作區設定

本專案使用 **west** 管理 Zephyr 依賴。工作區結構如下：

```
<workspace>/              ← west 工作區根目錄（例如 ~/）
├── zephyr/               ← Zephyr 原始碼（west 自動下載，~500 MB）
└── zephyr-thread-rt58x/  ← 本專案（west manifest）
```

### 3.1 初始化工作區（手動，首次）

```bash
# 以本專案為 manifest 初始化 west 工作區
cd <workspace>
west init -l zephyr-thread-rt58x

# 下載 Zephyr
west update

# 安裝 Zephyr Python 依賴
pip install -r zephyr/scripts/requirements-base.txt
```

### 3.2 日後更新

```bash
cd zephyr-thread-rt58x
git pull

cd ..
west update
```

---

## 4. 安裝 Zephyr SDK Toolchain

Zephyr SDK 提供官方 ARM 工具鏈（`arm-zephyr-eabi`）。

### Windows

從 [Zephyr SDK GitHub Releases](https://github.com/zephyrproject-rtos/sdk-ng/releases) 下載：
- `zephyr-sdk-<version>_windows-x86_64_gnu.7z`

用 7-Zip 解壓縮至無空格路徑（例如 `C:\zephyr-sdk-1.0.1\zephyr-sdk-1.0.1\`），再執行：

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

## 5. 設定環境變數

每次開啟新的 shell 都需要設定以下變數（`install` 腳本會自動產生環境檔）。

### Windows

```powershell
. <workspace>\env.ps1
```

或手動：

```powershell
$env:ZEPHYR_BASE              = "C:/Users/Stanley/zephyr"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:/zephyr-sdk-1.0.1/zephyr-sdk-1.0.1"
```

### Linux / macOS

```bash
source <workspace>/env.sh
```

或手動：

```bash
export ZEPHYR_BASE=~/zephyr
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-1.0.1
```

---

## 6. 編譯

所有指令從 **west 工作區根目錄**執行。

### Clean build（推薦）

```bash
west build -p always -b rt583_evb zephyr-thread-rt58x --build-dir zephyr-thread-rt58x/build
```

### 增量 build

```bash
west build -b rt583_evb zephyr-thread-rt58x --build-dir zephyr-thread-rt58x/build
```

### 輸出檔案

| 檔案 | 說明 |
|------|------|
| `build/zephyr/zephyr.bin` | 原始 binary，直接燒錄至 `0x00000000` |
| `build/zephyr/zephyr.elf` | ELF，含 debug 資訊，用於 GDB |
| `build/zephyr/zephyr.hex` | Intel HEX 格式 |

---

## 7. 燒錄

> **重要**：請燒錄至位址 **`0x00000000`**（Flash 起始位址）。
> Binary 含有自己的 vector table，直接開機——不需要 bootloader。
> 燒錄至 `0x8000` 將導致無輸出。

### 硬體接線（UART Console）

```
RT583-EVB          USB-UART 轉接器
GPIO16 (TX)  ───►  RX
GPIO17 (RX)  ◄───  TX
GND          ───── GND
```

---

### 7-A. 燒錄（Windows）

```powershell
$OPENOCD = "C:\Rafael-IoT-SDK-Internal\tools\Debugger\OpenOCD"
$BIN     = "C:\Users\Stanley\zephyr-thread-rt58x\build\zephyr\zephyr.bin"

& "$OPENOCD\bin\win\openocd.exe" `
  -s "$OPENOCD\script" `
  -f interface/cmsis-dap.cfg `
  -f target/rt58x.cfg `
  -c "init; halt; flash write_image erase `"$BIN`" 0x0; reset run; exit"
```

---

### 7-B. 燒錄（WSL / Linux）

WSL2 無法直接存取 USB——需先用 **usbipd-win** 將 CMSIS-DAP 橋接至 WSL，再由 WSL 中的 OpenOCD 燒錄。

#### 步驟 1：安裝 usbipd-win（Windows，只需一次）

```powershell
winget install usbipd
```

安裝後**重新開啟 PowerShell**。

#### 步驟 2：設定 udev 規則（WSL，只需一次）

```bash
bash tools/linux/flash.sh --setup-udev
```

設定完後，若提示需重新登入 WSL：

```powershell
# PowerShell
wsl --terminate Ubuntu
# 再重新開啟 WSL
```

#### 步驟 3：每次燒錄流程

```powershell
# ── Windows PowerShell（以系統管理員執行）────
# 橋接 CMSIS-DAP（自動偵測；或用 -BusId 2-3 指定）
.\tools\windows\attach-usb.ps1
```

```bash
# ── WSL（Bash）────────────────────────────────
bash tools/linux/flash.sh
```

```powershell
# ── Windows PowerShell（燒錄完成後）──────────
# 中斷橋接（讓其他 Windows 工具可使用該裝置）
.\tools\windows\attach-usb.ps1 -Detach
```

#### flash.sh 參數

| 參數 | 說明 |
|------|------|
| `--bin <path>` | 指定 binary 路徑（預設：`build/zephyr/zephyr.bin`） |
| `--setup-udev` | 安裝 CMSIS-DAP udev 規則（首次必須執行） |

#### rt58x.cfg 搜尋順序

`flash.sh` 會依下列順序尋找 OpenOCD scripts：

1. `/mnt/c/Rafael-IoT-SDK-Internal/tools/Debugger/OpenOCD/script`（掛載自 Windows SDK）
2. `boards/arm/rt583_evb/support/rt58x.cfg`（本專案內）

若兩處都找不到，會提示錯誤並說明如何手動指定路徑。

#### 疑難排解（WSL 燒錄）

| 症狀 | 解法 |
|------|------|
| `找不到 CMSIS-DAP 裝置` | 確認已在 PowerShell 執行 `attach-usb.ps1`；確認 WSL 已啟動 |
| `usbipd: command not found` | `winget install usbipd`，重開 PowerShell |
| `attach 失敗` | 以**系統管理員**執行 `attach-usb.ps1` |
| OpenOCD `libusb` 錯誤 | 執行 `--setup-udev` 並重新登入 WSL |
| `找不到 rt58x.cfg` | 確認 `C:\Rafael-IoT-SDK-Internal` 可從 `/mnt/c/` 存取 |

---

## 8. 驗證 — 序列終端機

以 **115200 baud、8N1、無 flow control** 開啟序列終端機（PuTTY、Tera Term）。

Reset 後應看到：

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

若看到 `>` prompt，表示 OpenThread CLI 已就緒。

---

## 9. OpenThread CLI 快速上手

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

## 10. 疑難排解

| 症狀 | 可能原因 | 解法 |
|------|---------|------|
| `find_package(Zephyr)` 失敗 | `ZEPHYR_BASE` 未設定或路徑錯誤 | 確認 `ZEPHYR_BASE` 指向正確的 `zephyr/` 目錄 |
| `Could NOT find Python3: Found unsuitable version` | Python 版本不足 | 確認 Python ≥ 3.12 |
| `No CMAKE_C_COMPILER` | Toolchain 未設定 | 確認 `ZEPHYR_TOOLCHAIN_VARIANT` 與 `ZEPHYR_SDK_INSTALL_DIR` 已設定 |
| `west update` 失敗 | revision 在遠端不存在 | 確認 `west.yml` 中的 `revision` 是有效的 tag 或 commit |
| 完全沒有 UART 輸出 | Binary 燒錄至錯誤位址 | 燒錄至 **0x0**，不是 `0x8000` |
| `printk` 輸出後系統卡死 | 從 ISR 呼叫 `printk`（spinlock 死鎖） | 使用 `k_work_submit` 延後到 thread context |
| UART RX 中斷從不觸發 | `IRQ_CONNECT` 未呼叫 | 確認 `uart_rt583.c` 中有呼叫 `IRQ_CONNECT` |
| RF init 後 OT task 卡住 | RF MCU init 阻塞 | 確認 `CONFIG_RF_FW_INCLUDE_PCI=TRUE` compile definition |
| `>` prompt 不出現 | OT CLI 初始化失敗 | 確認 `main.c` 有呼叫 `otAppCliInit` |
| OpenOCD: `LIBUSB_ERROR_NOT_FOUND` | CMSIS-DAP 未被識別 | 用 [Zadig](https://zadig.akeo.ie/) 安裝 WinUSB 驅動 |
| winget 移除失敗（exit 1603） | MSI 資料庫損毀 | `uninstall.ps1` 會自動嘗試 Registry fallback（msiexec + 強制清除） |

---

## 11. 專案結構

```
zephyr-thread-rt58x/
├── CMakeLists.txt              # 根 CMake；ZEPHYR_EXTRA_MODULES 註冊模組 + 平台驅動
├── Kconfig                     # 根 Kconfig；含 PHY/MAC PIB 設定選單
├── prj.conf                    # Kconfig fragments（UART、heap、OT 開關）
├── west.yml                    # West manifest（Zephyr 依賴版本）
│
├── scripts/
│   ├── windows/                # Windows 腳本（PowerShell 7+）
│   │   ├── install.ps1         # 安裝工具、SDK、west 工作區、env.ps1
│   │   ├── build.ps1           # 編譯
│   │   └── uninstall.ps1       # 移除環境（含 Registry fallback）
│   ├── linux/                  # Linux / WSL 腳本（Bash）
│   │   ├── install.sh
│   │   ├── build.sh
│   │   └── uninstall.sh
│   ├── macos/                  # macOS 腳本（Bash，支援 Apple Silicon）
│   │   ├── install.sh
│   │   ├── build.sh
│   │   └── uninstall.sh
│   └── strip_printk_newlines.py
│
├── zephyr/
│   └── module.yml              # Zephyr 模組描述；指向 cmake/zephyr_serial/
│
├── cmake/
│   └── zephyr_serial/
│       └── CMakeLists.txt      # 將 uart_rt583.c + hosal_uart.c 加入 drivers__serial
│
├── subsys/openthread/
│   └── CMakeLists.txt          # OT core、mbedTLS、RUCI、RF 等所有 OT 相關 sources
│
├── src/
│   └── main.c                  # RF init、PIB 設定、OT task 啟動、watchdog thread
│
├── drivers/
│   └── serial/
│       ├── uart_rt583.c        # Zephyr UART driver（HOSAL 封裝）
│       ├── Kconfig
│       └── CMakeLists.txt
│
├── sdk/                        # Vendored Rafael IoT SDK（僅原始碼，無預編譯 .a）
│   └── components/
│       ├── network/thread/     # OpenThread port（ot_radio.c、ot_uart.c 等）
│       ├── network/ruci/       # RUCI 指令/事件
│       ├── network/lmac15p4/   # IEEE 802.15.4 MAC
│       ├── network/rt569-rf/   # RT569 RF MCU 驅動
│       ├── network/rt569-fw/   # RT569 RF 韌體 blob
│       ├── network/thread/openthread/  # OpenThread 原始碼
│       ├── utility/            # log、fsm、util_queue 等
│       └── platform/           # SoC / HOSAL 平台驅動
│           ├── hosal/rt583_hosal/
│           └── soc/rt583/
│
├── boards/arm/rt583_evb/       # Board 定義（DTS、defconfig、Kconfig）
├── dts/arm/rafael/rt583.dtsi   # SoC DTS（Flash 1 MB、RAM 144 KB、UART0/1）
└── soc/arm/rafael_micro/rt583/ # SOC 定義（soc.c：SystemInit + COMM_SUBSYSTEM IRQ）
```

### 關鍵設計決策

| 決策 | 原因 |
|------|------|
| 全部從原始碼編譯，無預編譯 `.a` | 消除工具鏈升級時的 ABI 不相容問題 |
| `uart_rt583.c` 透過 Zephyr 模組加入 `drivers__serial` | `ZEPHYR_EXTRA_MODULES` 使模組 cmake 在正確時間點執行 |
| 排除 `soft_source_match_table.c` | `ot_radio.c` 已提供 hardware-backed src match via lmac15p4，避免符號衝突 |
| PIB 常數透過 Kconfig 設定 | 調整 RF 參數只需改 `prj.conf`，不需動應用程式碼 |
| Shell 停用（`CONFIG_SHELL=n`） | Shell 的 TX 中斷路徑與 `printk` polling 競爭，會導致輸出亂碼 |
