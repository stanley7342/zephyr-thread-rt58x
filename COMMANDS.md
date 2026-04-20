# RT583-EVB — 指令速查

## 一鍵安裝（不需先 clone）

```powershell
irm https://raw.githubusercontent.com/stanley7342/zephyr-thread-rt58x/master/scripts/windows/bootstrap.ps1 | iex
```

> 以 **PowerShell 7（系統管理員）** 執行。Clone 至當下目錄的 `zephyr-thread-rt58x/` 並完成環境建置。

---

## 環境載入（每次開新 shell）

從 `zephyr-thread-rt58x/` 目錄執行：

```powershell
. .\env.ps1
```

`env.ps1` 由 `install.ps1` 自動產生於**專案目錄內**（`zephyr-thread-rt58x/env.ps1`）。

---

## 安裝（第一次）

```powershell
.\scripts\windows\install.ps1
```

---

## 編譯

### lighting-app（Matter）

```sh
cd zephyr-thread-rt58x
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

PuTTY / Tera Term 或 pyserial miniterm：`115200 8N1`，無 flow control。

```powershell
python -m serial.tools.miniterm <COM_PORT> 115200
```

---

## Binary 位置

| 目標 | 路徑 |
|------|------|
| lighting-app | `build/lighting-app/zephyr/zephyr.bin` |
| OpenThread | `build/zephyr/zephyr.bin` |
| Bootloader | `build/bootloader/zephyr/zephyr.bin` |
