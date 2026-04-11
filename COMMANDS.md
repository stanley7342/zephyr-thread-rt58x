# RT583-EVB — 指令速查

## 一鍵安裝（不需先 clone）

```powershell
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex
```

> 以 **PowerShell 7（系統管理員）** 執行。Clone 至當下目錄的 `zephyr-rt58x/` 並完成環境建置。

---

## 環境載入（每次開新 shell）

```powershell
. $env:USERPROFILE\..\env.ps1
```

或完整路徑：

```powershell
. C:\Users\Stanley\env.ps1
```

---

## 安裝（第一次）

```powershell
.\scripts\windows\install.ps1
```

---

## 編譯

### lighting-app（Matter）

```sh
west build -p always -b rt583_evb examples/matter/lighting-app
```

```sh
west build -b rt583_evb examples/matter/lighting-app
```

### OpenThread FTD CLI

```sh
west build -p always -b rt583_evb .
```

```sh
west build -b rt583_evb .
```

### Bootloader（MCUboot）

```sh
west build -p always -b rt583_evb bootloader/mcuboot/boot/zephyr -- -DBOARD_ROOT=. -DOVERLAY_CONFIG=examples/matter/lighting-app/bootloader.conf -DDTC_OVERLAY_FILE=examples/matter/lighting-app/boards/rt583_evb.overlay
```

---

## 燒錄

```sh
west flash
```

指定 binary：

```sh
west flash --bin-file build/lighting-app/zephyr/zephyr.bin
```

---

## 序列監視器

```powershell
.\scripts\windows\attach-serial.ps1
```

或用 PuTTY / Tera Term：`115200 8N1`，無 flow control。

---

## Binary 位置

| 目標 | 路徑 |
|------|------|
| lighting-app | `build/lighting-app/zephyr/zephyr.bin` |
| OpenThread | `build/zephyr/zephyr.bin` |
| Bootloader | `build/bootloader/zephyr/zephyr.bin` |
