# T0: rt584 IDAU / SAU / NS-alias behavior survey.
#
# Run via openocd + GDB --batch:
#   ./tools/windows/openocd.exe -s ./tools/windows/tcl \
#       -f interface/cmsis-dap.cfg -f target/rt584.cfg \
#       -c "init" -c "reset halt"
#   arm-zephyr-eabi-gdb --batch -x debug/rt584_t0_survey.gdb \
#       build/thread584/thread/zephyr/zephyr.elf
#
# Goal: characterize IDAU + SAU + sec_peri_attr behavior so we can plan
# whether NS peripheral alias (0x4xxxxxxx) becomes routable when IDAU
# is on and sec_peri_attr clears the corresponding bit.
#
# Memory map for the survey:
#   SEC_CTRL @ 0x50013000:
#     0x00 sec_flash_sec_size       0x10 sec_ram_sec_size
#     0x04 sec_flash_nsc_start      0x14 sec_ram_nsc_start
#     0x08 sec_flash_nsc_stop       0x18 sec_ram_nsc_stop
#     0x0C sec_flash_ns_stop        0x1C sec_ram_ns_stop
#     0x20 sec_peri_attr[0]  bit18=UART0, bit26=COMM_AHB
#     0x24 sec_peri_attr[1]
#     0x28 sec_peri_attr[2]  bit0=CRYPTO, bit1=OTP
#     0x2C sec_idau_ctrl     1=enable IDAU
#     0x40 sec_mcu_debug
#     0x44 sec_lock_mcu_ctrl bit4=lock SAU
#
#   SAU (Cortex-M33) @ 0xE000EDD0:
#     0xE000EDD0 SAU_CTRL   bit0=ENABLE bit1=ALLNS
#     0xE000EDD4 SAU_TYPE   region count
#     0xE000EDD8 SAU_RNR
#     0xE000EDDC SAU_RBAR
#     0xE000EDE0 SAU_RLAR

set pagination off
set print pretty on
set confirm off

target remote localhost:3333
monitor reset halt
load
monitor reset halt
monitor resume
shell sleep 5
monitor halt

printf "\n========================================================\n"
printf " T0 SURVEY — rt584 IDAU/SAU/NS-alias characterisation\n"
printf "========================================================\n"

printf "\n=== CPU state at halt ===\n"
monitor reg pc
monitor reg xpsr
monitor reg msp
monitor reg psp

printf "\n=== SAU registers (Cortex-M33 stock) ===\n"
printf "SAU_CTRL  (0xE000EDD0): "
monitor mdw 0xE000EDD0
printf "SAU_TYPE  (0xE000EDD4): "
monitor mdw 0xE000EDD4
printf "SAU_RNR   (0xE000EDD8): "
monitor mdw 0xE000EDD8

printf "\n--- All 8 SAU regions (write RNR, read RBAR/RLAR) ---\n"
monitor mww 0xE000EDD8 0
printf "Region 0 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
monitor mww 0xE000EDD8 1
printf "Region 1 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
monitor mww 0xE000EDD8 2
printf "Region 2 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
monitor mww 0xE000EDD8 3
printf "Region 3 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
monitor mww 0xE000EDD8 4
printf "Region 4 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
monitor mww 0xE000EDD8 5
printf "Region 5 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
monitor mww 0xE000EDD8 6
printf "Region 6 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
monitor mww 0xE000EDD8 7
printf "Region 7 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2

printf "\n=== rt584 SEC_CTRL (vendor IDAU) — full dump ===\n"
printf "0x50013000–0x5001304C (20 words):\n"
monitor mdw 0x50013000 20

printf "\n=== rt584 SEC_CTRL named fields ===\n"
printf "sec_flash_sec_size   (0x00): "
monitor mdw 0x50013000
printf "sec_flash_nsc_start  (0x04): "
monitor mdw 0x50013004
printf "sec_flash_nsc_stop   (0x08): "
monitor mdw 0x50013008
printf "sec_flash_ns_stop    (0x0C): "
monitor mdw 0x5001300C
printf "sec_ram_sec_size     (0x10): "
monitor mdw 0x50013010
printf "sec_ram_nsc_start    (0x14): "
monitor mdw 0x50013014
printf "sec_ram_nsc_stop     (0x18): "
monitor mdw 0x50013018
printf "sec_ram_ns_stop      (0x1C): "
monitor mdw 0x5001301C
printf "sec_peri_attr[0]     (0x20): "
monitor mdw 0x50013020
printf "sec_peri_attr[1]     (0x24): "
monitor mdw 0x50013024
printf "sec_peri_attr[2]     (0x28): "
monitor mdw 0x50013028
printf "sec_idau_ctrl        (0x2C): "
monitor mdw 0x5001302C
printf "sec_mcu_debug        (0x40): "
monitor mdw 0x50013040
printf "sec_lock_mcu_ctrl    (0x44): "
monitor mdw 0x50013044

printf "\n=== Baseline NS vs S alias readings (UART0) ===\n"
printf "NS UART0 LSR (0x40012014): "
monitor mdw 0x40012014
printf "S  UART0 LSR (0x50012014): "
monitor mdw 0x50012014
printf "NS UART0 BASE+0 (0x40012000): "
monitor mdw 0x40012000
printf "S  UART0 BASE+0 (0x50012000): "
monitor mdw 0x50012000

printf "\n=== EXPERIMENT 1: Disable IDAU (write sec_idau_ctrl=0) ===\n"
monitor mww 0x5001302C 0
printf "After write, sec_idau_ctrl: "
monitor mdw 0x5001302C
printf "NS UART0 LSR (0x40012014): "
monitor mdw 0x40012014
printf "S  UART0 LSR (0x50012014): "
monitor mdw 0x50012014

printf "\n=== EXPERIMENT 2: Re-enable IDAU (sec_idau_ctrl=1) ===\n"
monitor mww 0x5001302C 1
printf "After write, sec_idau_ctrl: "
monitor mdw 0x5001302C
printf "NS UART0 LSR (0x40012014): "
monitor mdw 0x40012014
printf "S  UART0 LSR (0x50012014): "
monitor mdw 0x50012014

printf "\n=== EXPERIMENT 3: Read current sec_peri_attr[0] then clear UART0 bit (bit18) ===\n"
printf "Before sec_peri_attr[0]: "
monitor mdw 0x50013020
printf "(UART0 bit18 = 0x40000)\n"
printf "Reading existing value, computing new...\n"
# We'll compute by hand: clear bit 18. We don't know current value via gdb expr.
# Easier: just set sec_peri_attr[0] = 0 (all NS), then read UART0.
monitor mww 0x50013020 0
printf "After mww 0x50013020 0, sec_peri_attr[0]: "
monitor mdw 0x50013020
printf "NS UART0 LSR (0x40012014): "
monitor mdw 0x40012014
printf "S  UART0 LSR (0x50012014): "
monitor mdw 0x50012014

printf "\n=== EXPERIMENT 4: Restore sec_peri_attr[0]=0xFFFFFFFF (all S) ===\n"
monitor mww 0x50013020 0xFFFFFFFF
printf "After write, sec_peri_attr[0]: "
monitor mdw 0x50013020
printf "NS UART0 LSR (0x40012014): "
monitor mdw 0x40012014
printf "S  UART0 LSR (0x50012014): "
monitor mdw 0x50012014

printf "\n=== EXPERIMENT 5: SAU enable + region 0 = whole 0x40000000-0x4FFFFFFF NS ===\n"
printf "Configuring SAU region 0...\n"
# RNR = 0
monitor mww 0xE000EDD8 0
# RBAR = 0x40000000 (32-byte aligned)
monitor mww 0xE000EDDC 0x40000000
# RLAR = 0x4FFFFFE0 | NSC=0(1) | ENABLE=1(0)  → 0x4FFFFFE1
monitor mww 0xE000EDE0 0x4FFFFFE1
# SAU_CTRL = ENABLE=1, ALLNS=0
monitor mww 0xE000EDD0 0x00000001
printf "Verify SAU_CTRL: "
monitor mdw 0xE000EDD0
printf "Verify region 0 RBAR/RLAR: "
monitor mdw 0xE000EDDC 2
printf "NS UART0 LSR (0x40012014): "
monitor mdw 0x40012014

printf "\n=== EXPERIMENT 6: SAU region 0 NS + sec_peri_attr[0] UART0 cleared ===\n"
monitor mww 0x50013020 0
printf "sec_peri_attr[0]=0 + SAU region 0 NS active.\n"
printf "NS UART0 LSR (0x40012014): "
monitor mdw 0x40012014
printf "S  UART0 LSR (0x50012014): "
monitor mdw 0x50012014

printf "\n=== Restore safe state (all S, IDAU on, SAU off) ===\n"
monitor mww 0x50013020 0xFFFFFFFF
monitor mww 0xE000EDD0 0x00000000
monitor mww 0x5001302C 1

printf "\n=== Done. Reset chip back to fresh state. ===\n"
monitor reset halt
