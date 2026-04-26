# T0: rt584 IDAU / SAU / NS-alias 行為實測

**Date**: 2026-04-26
**Method**: openocd + GDB batch 驅動，halt 一個跑到 idle 的 Zephyr image，dump SEC_CTRL / SAU / NS+S UART alias，並做 6 個寫入實驗。
**Script**: [`debug/rt584_t0_survey.gdb`](../../debug/rt584_t0_survey.gdb)

## TL;DR

**rt584 silicon 不支援 TF-M 標準 port**。三條獨立證據：

1. **SAU_TYPE = 0** — Cortex-M33 在 silicon 合成時 omit 了 SAU regions。沒有 SAU = 沒辦法軟體標 NS region。
2. **sec_idau_ctrl 寫 0 或 1 對 NS alias 無影響** — IDAU 主開關不路由 NS alias。
3. **sec_peri_attr[] 寫進去 bit 16+ 不留** — peripheral S/NS bitmap 高位 RO，且寫了也沒影響 NS alias 行為。

→ Zephyr 跑進 NS state 後**無法 access 任何 peripheral**（沒有可用的 NS alias），TF-M 標準路線（Zephyr-NS + TF-M-S + PSA service via NSC）在 rt584 不可行。

## 量測 raw 資料

### CPU 與 SAU 預設

```
xpsr        = 0x41000000   (Thread mode, 在 secure state — bit 0-8 IPSR=0; T bit set)
SAU_CTRL    = 0x00000002   (bit1 ALLNS=1 — 無 SAU 設定下視為全 NS；bit0 ENABLE=0)
SAU_TYPE    = 0x00000000   ← 0 個 SAU regions（硬體沒實作）
SAU regions = 8 個全部 0x00000000 (RBAR/RLAR)
```

`SAU_TYPE=0` 是致命發現。Cortex-M33 SAU 是合成時可選 0/4/8 regions。rt584 選了 0，等於沒有 SAU。沒有 SAU 就只能靠 IDAU；rt584 的 IDAU 是 vendor 自製 (`SEC_CTRL`)，且——

### SEC_CTRL @ 0x50013000 預設值

```
0x00 sec_flash_sec_size    = 0x00000000   (未配置)
0x04 sec_flash_nsc_start   = 0x00000000
0x08 sec_flash_nsc_stop    = 0x00000380   (可疑值)
0x0C sec_flash_ns_stop     = 0x00000000
0x10 sec_ram_sec_size      = 0x00000000
0x14 sec_ram_nsc_start     = 0x00000060   ← 比 stop 大，明顯亂值
0x18 sec_ram_nsc_stop      = 0x00000010
0x1C sec_ram_ns_stop       = 0x00000000
0x20 sec_peri_attr[0]      = 0x00000000   (預期 1=NSC, 0=S 或反向，行為不明)
0x24 sec_peri_attr[1]      = 0x00000000
0x28 sec_peri_attr[2]      = 0x00000000
0x2C sec_idau_ctrl         = 0x00000000   ← IDAU 主開關關著
0x40 sec_mcu_debug         = 0x00000000
0x44 sec_lock_mcu_ctrl     = 0x00000000
```

**MCUboot + Zephyr soc.c 都跑完後，IDAU 仍是停用狀態**。先前 memory 認為 soc.c 有寫 `sec_idau_ctrl=1` ——驗證後是 **soc.c 寫到了錯誤位址 `0x50003020`**（正確是 `0x50013020`），差了 `0x10000`。`0x40003000` 區域是空的 peripheral 槽（GPIO@0x1000 與 RTC@0x4000 之間），寫進去 silently dropped。**rt584 上 COMM_SUBSYSTEM bring-up 能成功不是這段 code 的功勞**。

### Baseline NS / S alias

```
NS UART0 LSR (0x40012014) = 0x00000000
S  UART0 LSR (0x50012014) = 0x00000060   (THRE+TEMT)
NS UART0 BASE (0x40012000) = 0x00000000
S  UART0 BASE (0x50012000) = 0x0000006a
```

NS alias 死、S alias 活。預期。

## 六個實驗結果

| # | 操作 | 結果 |
|---|---|---|
| 1 | `sec_idau_ctrl = 0` (停用 IDAU) | NS alias 仍 0 |
| 2 | `sec_idau_ctrl = 1` (啟用 IDAU) | NS alias 仍 0 |
| 3 | `sec_peri_attr[0] = 0` (UART0 標 NS) | NS alias 仍 0 |
| 4 | `sec_peri_attr[0] = 0xFFFFFFFF` (全 S) | 只有 bit 0-15 寫得進去（讀回 `0x0000FFFF`），NS alias 仍 0 |
| 5 | SAU enable + region 0 = 0x40000000-0x4FFFFFFF NS | SAU_CTRL 寫不進去（讀回 0），region 0 RBAR/RLAR 也保持 0 |
| 6 | 5+3 同時 | NS alias 仍 0 |

**Experiment 5 的失敗**進一步確認 SAU 在 silicon 完全沒實作（不是只有 0 region，是整個 SAU 控制路徑都 dead）。寫 SAU_CTRL 不留值符合 ARMv8-M 在 SAU_TYPE=0 時的行為——SAU regs 全部視為 RAZ/WI。

**Experiment 4 的高 16 bit RO 行為**：可能是 vendor 把 sec_peri_attr 的 width 縮在 16 bit 連 RTL，或是高 bit 對應的 IDAU_Type 槽位沒接，但 Inc/sec_ctrl_reg.h 列了 bit 16-31 的 macro（QSPI、DMA、I2C-master、SADC 等）。**Header 與 RTL 不一致是 Rafael 的 bug**。

## 對 TF-M 路線的影響

### 不可行：標準 TF-M 模型
- Zephyr 跑在 NS state、TF-M 跑在 S、NS 透過 SG/NSC 進 S 拿 PSA service
- **死在 NS state 完全不能 access peripheral**
- 即使 RAM/Flash 切割能用 IDAU 做，peripheral 沒辦法切
- vendor 等於沒做完 TZ 的 silicon 整合

### 可行的替代路線

**(a) 不切 TZ，全 secure，跑 mbedTLS PSA — 練 PSA API**
- 沒有 isolation，但 PSA Crypto / ITS / Attestation API 都可用
- 跑得起來 ARM `psa-arch-tests` 全套
- 等於 PSA Certified L1 paperwork 路線
- 學到：PSA API 怎麼用、key lifecycle、attestation token 結構
- **不會學到**：SAU/NSC veneer、TZ partitioning

**(b) 等 Rafael 出 silicon errata 或下一代版本** — 卡在外部依賴

**(c) 改用 Nordic nRF54L15 / NXP K32W148 / ST STM32U585 等 silicon 已過 PSA L2 RoT 的 dev board 練 TF-M** — 不在 rt584 上練

## 建議

**Phase TFM 改寫**：

| 階段 | 原計畫 | 修正後計畫 |
|---|---|---|
| T0 | NS-alias 偵察 | ✅ 已完成（這份文件） |
| T1 | 自寫 TZ shim | ❌ 跳過（silicon 不支援） |
| T2 | TF-M Profile Small | ⚠️ 走「mbedTLS PSA without isolation」變體 |
| (a) | TMSA + Architecture | ✅ 仍有意義，加上「rt584 platform analysis」一節 |
| (b) | PSA test suite | ✅ 仍有意義，target 是 mbedTLS PSA 而非 TF-M |

**省下的時程**：跳掉 T1 自寫 shim 約 5 天 + T2 TF-M port 從 1-2 週縮成 3-5 天（mbedTLS 整合）。整體 4-6 週可縮到 **2-3 週**。

## 後續 cleanup

- [ ] soc.c 拿掉 `RT584_SEC_PERI_ATTR0/1/2/IDAU_CTRL` 那段（寫到空地址、無作用、誤導）
- [ ] memory 更新 `project_rt584_bringup.md`，把「sec_idau_ctrl=1 re-arm」歸因刪掉
- [ ] 找出 COMM_SUBSYSTEM bring-up 真正的 load-bearing fix 是哪行（最可能 `rco1m_and_rco32k_calibration`）
