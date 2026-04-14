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

1. [快速開始 — Windows](#1-快速開始--windows)
2. [快速開始 — Linux / WSL](#2-快速開始--linux--wsl)
3. [快速開始 — macOS](#3-快速開始--macos)
4. [燒錄](#4-燒錄)
5. [驗證 — 序列終端機](#5-驗證--序列終端機)
6. [OpenThread CLI 快速上手](#6-openthread-cli-快速上手)
7. [疑難排解](#7-疑難排解)
8. [專案結構](#8-專案結構)

---

## 1. 快速開始 — Windows

> **需求**：PowerShell 7（以系統管理員身份執行）。
> 尚未安裝可執行：`winget install Microsoft.PowerShell`

### 步驟 1 — 一鍵下載並安裝環境

在任意目錄開啟 PowerShell 7（系統管理員），執行：

```powershell
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex
```

腳本會自動完成：
- Clone 專案至 `<當下目錄>\zephyr-thread-rt58x`
- 安裝 Python、CMake、Ninja、Git、7-Zip（via winget）
- 建立 Python venv、安裝 west、GN、jsonschema
- 下載並安裝 Zephyr SDK 1.0.1
- 初始化 west workspace、下載 Zephyr
- 產生 `env.ps1` 環境載入腳本

安裝完成後畫面會顯示：
```
West workspace : <當下目錄>
專案目錄       : <當下目錄>\zephyr-thread-rt58x

每次開啟新 PowerShell 請先執行：
  . <當下目錄>\env.ps1

編譯：
  cd <當下目錄>\zephyr-thread-rt58x
  west build -p always -b rt583_evb examples/matter/lighting-app
```

### 步驟 2 — 載入環境（每次開新 shell）

```powershell
. <workspace>\env.ps1
```

### 步驟 3 — 編譯

```powershell
cd zephyr-thread-rt58x
```

**Clean build（第一次或變更 Kconfig）：**

```sh
west build -p always -b rt583_evb examples/matter/lighting-app
```

**增量編譯（只改了 C/C++ 程式碼）：**

```sh
west build -b rt583_evb examples/matter/lighting-app
```

編譯完成後 binary 位於：

```
build/lighting-app/zephyr/zephyr.bin
build/lighting-app/zephyr/zephyr.hex
```

---

## 2. 快速開始 — Linux / WSL

```bash
git clone https://github.com/stanley7342/zephyr-thread-rt58x.git
cd zephyr-thread-rt58x

bash scripts/linux/install.sh
source ../env.sh

west build -p always -b rt583_evb examples/matter/lighting-app
# 增量：
west build -b rt583_evb examples/matter/lighting-app
```

---

## 3. 快速開始 — macOS

```bash
git clone https://github.com/stanley7342/zephyr-thread-rt58x.git
cd zephyr-thread-rt58x

bash scripts/macos/install.sh
source ../env.sh

west build -p always -b rt583_evb examples/matter/lighting-app
# 增量：
west build -b rt583_evb examples/matter/lighting-app
```

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

---

## 4. 燒錄

> 接線：RT583-EVB GPIO16(TX)→USB-UART RX，GPIO17(RX)→TX，GND→GND

```sh
west flash
```

---

## 5. 驗證 — 序列終端機

以 **115200 baud、8N1、無 flow control** 開啟終端機（PuTTY / Tera Term）。

Reset 後應看到：

```
[MAIN] start
...
> 
```

---

## 6. OpenThread CLI 快速上手

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

## 7. 疑難排解

| 症狀 | 解法 |
|------|------|
| `find_package(Zephyr)` 失敗 | 確認已執行 `. env.ps1` |
| `Could not find GN_EXECUTABLE` | `pip install gn` |
| `Missing jsonschema` | `pip install jsonschema` |
| `west init already initialized` | bootstrap 自動繞過，不需處理 |
| binary 燒錄後無輸出 | 確認燒錄位址為 **0x0**（非 0x8000） |
| UART RX 中斷不觸發 | 確認 `IRQ_CONNECT` 有呼叫 |

---

## 8. 專案結構

（原 §11）

---

## 參考 — Clone 與 West 工作區設定

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

在 `zephyr-thread-rt58x/` 目錄下執行：

**Clean build（第一次或變更 Kconfig）：**

```sh
west build -p always -b rt583_evb examples/matter/lighting-app
```

**增量編譯：**

```sh
west build -b rt583_evb examples/matter/lighting-app
```

**OpenThread FTD CLI（非 Matter）：**

```sh
west build -p always -b rt583_evb .
```

輸出 binary：`build/lighting-app/zephyr/zephyr.bin`

---

## 7. 燒錄

硬體接 CMSIS-DAP（Debugger），在 `zephyr-thread-rt58x/` 下執行：

```sh
west flash
```

指定 binary（可選）：

```sh
west flash --bin-file build/lighting-app/zephyr/zephyr.bin
```

### 硬體接線（UART Console）

```
RT583-EVB          USB-UART 轉接器
GPIO16 (TX)  ───►  RX
GPIO17 (RX)  ◄───  TX
GND          ───── GND
```

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
| `west flash` 找不到裝置 | CMSIS-DAP 驅動未安裝 | 用 [Zadig](https://zadig.akeo.ie/) 安裝 WinUSB 驅動 |
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
