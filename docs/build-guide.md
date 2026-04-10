# RT583-EVB Build Guide

所有專案的編譯與燒錄說明，涵蓋自動化腳本與直接 `west` 工具兩種方式。

---

## 工作區結構

```
C:\Users\Stanley\              ← west workspace root
├── env.ps1                    ← 環境設定（install.ps1 自動產生）
├── zephyr\                    ← Zephyr 原始碼
├── bootloader\mcuboot\        ← MCUboot（west 管理）
├── mcuboot\                   ← MCUboot 本體
├── modules\                   ← Zephyr modules（mbedTLS 等）
├── connectedhomeip\           ← Matter SDK（需手動 clone）
└── zephyr-thread-rt58x\       ← 本專案（所有指令從此目錄執行）
```

---

## 環境設定

每次開啟新的 PowerShell session，執行一次：

```powershell
# 從 zephyr-thread-rt58x\ 執行
. ..\env.ps1
$env:ZEPHYR_NO_ENV = "1"
```

lighting-app 額外需要：

```powershell
$env:CHIP_ROOT = "C:\Users\Stanley\connectedhomeip"
$env:PATH      = "C:\Users\Stanley\bin;$env:PATH"
```

---

## 專案一覽

| 專案 | 說明 | MCUboot | 燒錄位址 |
|------|------|---------|---------|
| `bootloader` | MCUboot bootloader | — | `0x0` |
| `thread` | OpenThread FTD CLI | ✓ | `0x10000` |
| `blinky` | LED blink 範例 | ✓ | `0x10000` |
| `hello_world` | Hello World 範例 | ✓ | `0x10000` |
| `test_flash` | Flash 單元測試 | ✓ | `0x10000` |
| `ble_hrs` | BLE Heart Rate Sensor | ✓ | `0x10000` |
| `test_hci` | BLE HCI 測試 | ✓ | `0x10000` |
| `lighting-app` | Matter over Thread 燈控 | ✓ | `0x10000` |

> **首次使用 MCUboot 的裝置**：先燒錄 `bootloader`，之後再燒各應用程式。

---

## 方式一：自動化腳本（推薦）

### 編譯

```powershell
.\scripts\windows\build.ps1 -p <專案名>            # Clean build
.\scripts\windows\build.ps1 -p <專案名> -NoPristine # 增量編譯
```

### 燒錄

```powershell
.\scripts\windows\flash.ps1 -p <專案名>
```

### 範例

```powershell
# bootloader（首次燒入，之後不需重複）
.\scripts\windows\build.ps1 -p bootloader
.\scripts\windows\flash.ps1 -p bootloader

# lighting-app
.\scripts\windows\build.ps1 -p lighting-app
.\scripts\windows\flash.ps1 -p lighting-app

# thread
.\scripts\windows\build.ps1 -p thread
.\scripts\windows\flash.ps1 -p thread
```

---

## 方式二：west 工具

> **注意**：第一次 `west build` 為 pristine build（`-p always`），之後可省略 `-p always` 做增量編譯。  
> `west flash` 需要先完成過一次 `west build`（產生 `runners.yaml`）。

---

### bootloader

```powershell
west build -p always -b rt583_evb `
    ../bootloader/mcuboot/boot/zephyr `
    --build-dir build/bootloader `
    -- "-DOVERLAY_CONFIG=$PWD/examples/bootloader/mcuboot.conf"

west flash --build-dir build/bootloader
```

---

### thread

```powershell
west build -p always -b rt583_evb examples/thread --build-dir build/thread
west flash --build-dir build/thread
```

---

### blinky

```powershell
west build -p always -b rt583_evb examples/blinky --build-dir build/blinky
west flash --build-dir build/blinky
```

---

### hello_world

```powershell
west build -p always -b rt583_evb examples/hello_world --build-dir build/hello_world
west flash --build-dir build/hello_world
```

---

### test_flash

```powershell
west build -p always -b rt583_evb tests/flash --build-dir build/test_flash
west flash --build-dir build/test_flash
```

---

### ble_hrs

```powershell
west build -p always -b rt583_evb examples/ble/peripheral/hrs --build-dir build/ble_hrs
west flash --build-dir build/ble_hrs
```

---

### test_hci

```powershell
west build -p always -b rt583_evb examples/ble/test_hci --build-dir build/test_hci
west flash --build-dir build/test_hci
```

---

### lighting-app

```powershell
# 確認環境變數已設定（見上方「環境設定」）
west build -p always -b rt583_evb `
    examples/matter/lighting-app `
    --build-dir build/lighting-app

west flash --build-dir build/lighting-app
```

---

## Flash 分區配置

```
RT583 Flash（2 MB）
┌─────────────────────────────┬──────────┬───────────┐
│ 區域                         │ 位址      │ 大小      │
├─────────────────────────────┼──────────┼───────────┤
│ boot_partition (MCUboot)    │ 0x000000 │  64 KB    │
│ slot0_partition (image-0)   │ 0x010000 │ 800 KB    │  ← thread/ble/blinky/hello_world
│ slot1_partition (image-1)   │ 0x0D8000 │ 800 KB    │  ← OTA 備援槽
│ staging_partition           │ 0x1A0000 │ 256 KB    │  ← OTA 暫存
│ storage_partition           │ 0x1E0000 │  64 KB    │  ← Settings / NVS
│ factory_partition           │ 0x1F0000 │  64 KB    │  ← 出廠資料
└─────────────────────────────┴──────────┴───────────┘

lighting-app（獨立分區配置，透過 boards/rt583_evb.overlay 覆蓋）
┌─────────────────────────────┬──────────┬───────────┐
│ boot_partition (MCUboot)    │ 0x000000 │  64 KB    │
│ slot0_partition (image-0)   │ 0x010000 │ 1024 KB   │  ← lighting-app（~866 KB）
│ storage_partition           │ 0x1E0000 │  64 KB    │
│ factory_partition           │ 0x1F0000 │  64 KB    │
└─────────────────────────────┴──────────┴───────────┘
```

---

## 序列終端機

```powershell
# 自動偵測 COM port 並開啟 TeraTerm（115200 8N1）
$port = (Get-WmiObject Win32_PnPEntity |
    Where-Object Name -match "USB Serial" |
    Select-Object -First 1 -ExpandProperty Name) -replace '.*\((COM\d+)\).*','$1'
ttermpro /C=$($port -replace 'COM','') /BAUD=115200

# 或直接指定
ttermpro /C=3 /BAUD=115200   # COM3
```

---

## 疑難排解

| 症狀 | 解法 |
|------|------|
| `runners.yaml not found` | 執行一次 `west build -p always`（pristine build）重新 configure |
| `source directory does not exist` | 確認從 `zephyr-thread-rt58x\` 執行，bootloader 路徑用 `../bootloader/mcuboot/boot/zephyr` |
| `hex file location is unknown` | `CONFIG_BUILD_OUTPUT_HEX=y` 已在 defconfig 設定，需 pristine build 一次使其生效 |
| lighting-app: GN build 失敗 | 確認 `CHIP_ROOT` 與 `PATH`（clang-format）已設定 |
| 燒錄後無輸出 | MCUboot 應用程式確認 bootloader 已先燒入 `0x0` |
