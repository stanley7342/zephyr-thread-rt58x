# Automated rt584 boot diagnostic script.
#
# Usage (Terminal 1 — start openocd first):
#   tools\windows\openocd.exe -s tools\windows\tcl \
#       -f interface\cmsis-dap.cfg -f target\rt584.cfg \
#       -c "init" -c "reset halt"
#
# Usage (Terminal 2 — run GDB with this script):
#   arm-zephyr-eabi-gdb -x debug\rt584_diag.gdb build\hw584_bare\zephyr\zephyr.elf

set pagination off
set print pretty on
set confirm off

target remote localhost:3333
monitor reset halt

# Re-flash via openocd so we know the binary on chip matches the .elf we
# loaded into GDB. (Skip if you already flashed via isp_cli.)
load

# ---------------------------------------------------------------------------
# Pass 1 — let CPU run from reset, catch the first breakpoint or fault.
# ---------------------------------------------------------------------------
break z_arm_reset
break z_prep_c
break soc_prep_hook
break Reset_Handler
break HardFault_Handler
break z_arm_fault_dump
break z_arm_fault

monitor reset halt

printf "\n========== PASS 1: free-running boot ==========\n"
continue

# ---------------------------------------------------------------------------
# Where did we land?
# ---------------------------------------------------------------------------
printf "\n========== CPU state at first stop ==========\n"
info registers
printf "\n--- Disassembly around PC ---\n"
x/8i $pc
printf "\n--- Disassembly at LR (caller) ---\n"
x/4i $lr & ~1

printf "\n========== Cortex-M33 Fault Status Registers ==========\n"
printf "CFSR (0xE000ED28):  "
x/x 0xE000ED28
printf "HFSR (0xE000ED2C):  "
x/x 0xE000ED2C
printf "MMFAR(0xE000ED34):  "
x/x 0xE000ED34
printf "BFAR (0xE000ED38):  "
x/x 0xE000ED38
printf "AFSR (0xE000ED3C):  "
x/x 0xE000ED3C
printf "SFSR (0xE000EDE4):  "
x/x 0xE000EDE4
printf "SFAR (0xE000EDE8):  "
x/x 0xE000EDE8

printf "\n========== Stack-Limit Registers ==========\n"
printf "MSP:    "
info registers msp
printf "MSPLIM: "
info registers msplim
printf "PSP:    "
info registers psp
printf "PSPLIM: "
info registers psplim

printf "\n========== Vector Table @ 0x10000000 ==========\n"
x/8wx 0x10000000

printf "\n========== SCB->VTOR ==========\n"
x/wx 0xE000ED08

printf "\n========== AIRCR (CM33 reset/priority) ==========\n"
x/wx 0xE000ED0C

printf "\n========== SAU CTRL ==========\n"
x/wx 0xE000EDD0

printf "\n========== Done. Use 'si' to single-step, 'continue' to resume. ==========\n"
