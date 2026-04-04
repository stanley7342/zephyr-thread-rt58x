---
name: zephyr-thread 專案 session 備份
description: RT582 Zephyr+OpenThread 重構進度、已完成工作與當前狀態
type: project
---

## 當前狀態（2026-04-04）

**最新 commit**：`fc2b4db`（master，ahead of remote by 3）
**build 狀態**：通過，FLASH 374 KB / 1 MB，RAM 93 KB / 144 KB
**硬體測試**：EVB 可正常開機，channel_set 不再卡住，OT CLI 可用

---

## 已完成的重構工作

### 1. 移除所有預編譯 .a 函式庫
- 刪除 `sdk/lib/` 下所有 `.a`：libopenthread-ftd.a、liblmac15p4.a、libruci.a 等
- OT core、tcplp、mbedTLS、RUCI、utility、log、rt582_crypto 全改從原始碼編譯

### 2. FreeRTOS → Zephyr 移植
- `lmac15p4`：以 `k_sem` 取代 FreeRTOS semaphore，移至 `sdk/components/network/lmac15p4/Src/lmac15p4.c`
- `hosal_rf`：以 `k_sem/k_msgq/k_thread` 取代 FreeRTOS，移至 `drivers/hosal/rt582_hosal/Src/hosal_rf.c`
- `freertos_shim.c` → `rt582_compat.c`，移至 `openthread_port/Src/`
- 移除 FreeRTOS stub headers，`log.h` 改用 `k_uptime_ticks()`
- `rt_crypto.c`：`#if !defined(CONFIG_BOOTLOADER)` → `#if defined(CONFIG_CRYPTO_INT_ENABLE)`

### 3. 目錄整理
- `subsys/openthread/` 刪除，內容併入頂層 `CMakeLists.txt` 與 `Kconfig`
- `platform/` 所有檔案移至 `sdk/components/network/thread/openthread_port/Src/` 與 `Inc/`
- `include/` → `Inc/`（ruci、rt569-rf、rt569-fw、openthread_port）
- `openthread_port/utils/utils/`（重複目錄）刪除

### 4. 關鍵 bug 修復
- **channel_set deadlock**：`hosal_rf_zephyr.c` ioctl 使用 `(uint32_t)(uintptr_t)p_arg` 取代 `*(uint32_t*)p_arg`
- **mac_frame.cpp**：OT core 版（C++ class methods）與 Rafael 版（C API wrappers）兩者都需要
- **_fini**：加 `zephyr_ld_options(-Wl,--defsym=_fini=0)` 解決 newlib 連結錯誤
- **mbedTLS**：排除 `net_sockets.c`、`entropy_poll.c`（POSIX-only）
- **extension_example.cpp**：排除（依賴未啟用的 OT Extension API）

### 5. OT 編譯設定（CMakeLists.txt）
```cmake
OPENTHREAD_CONFIG_FILE="openthread-core-rt582-zephyr-config.h"
CONFIG_RT582=1 / OPENTHREAD_FTD=1
CHIP_TYPE=2 / CONFIG_RF_FW_INCLUDE_PCI=TRUE
CONFIG_CRYPTO_SECP256R1_ENABLE=1
MBEDTLS_KEY_EXCHANGE_ECJPAKE_ENABLED
MBEDTLS_PLATFORM_MEMORY
PACKAGE_NAME / PACKAGE_VERSION
```

---

## 目前專案結構（關鍵路徑）

```
zephyr-thread/
├── CMakeLists.txt          ← 所有 build 邏輯在此（已無 subsys/）
├── Kconfig                 ← CONFIG_OPENTHREAD_RT582 定義在此
├── drivers/
│   ├── hosal/rt582_hosal/  ← hosal_rf.c（Zephyr 版）、hosal_trng.c 等
│   └── soc/rt582/          ← rt582_driver、rt582_system、rt582_crypto
└── sdk/components/
    ├── network/
    │   ├── lmac15p4/Src/lmac15p4.c          ← Zephyr k_sem 版
    │   ├── ruci/Src/ + Inc/
    │   ├── rt569-rf/rt582/Src/ + Inc/
    │   ├── rt569-fw/rt582/Src/ + Inc/
    │   └── thread/
    │       ├── openthread/                   ← OT core 原始碼
    │       └── openthread_port/
    │           ├── Inc/                      ← openthread_port.h（Zephyr 版）等
    │           ├── Src/                      ← ot_radio.c、ot_zephyr.c 等
    │           └── utils/                    ← mac_frame.cpp、mac_frame_gen.c 等
    └── utility/
        ├── log/log.c + log.h
        └── utility/Src/
```

---

## 待處理 / 觀察中

- WDG 後 CLI prompt (`>`) 是否出現需觀察
- 開機 log 仍有部分交錯（直接 printk，無 queue，屬預期行為）
- `ot_logging.c` 已還原為直接 `printk`（deferred queue 方案已撤銷）
