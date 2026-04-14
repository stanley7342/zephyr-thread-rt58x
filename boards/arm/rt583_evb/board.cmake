# SPDX-License-Identifier: Apache-2.0
#
# west flash runner for RT583 EVB (CMSIS-DAP + openocd-rt58x)
#
# Uses BIN mode with explicit flash address so openocd does a raw memory
# write (flash bank is at 0x08000000 in rt58x.cfg; addresses at 0x0/0x10000
# fall outside the bank, triggering the raw-write path which works on RT583).
#
# CONFIG_FLASH_LOAD_OFFSET provides the correct address per-image:
#   bootloader (MCUboot itself)  → 0x0
#   app with CONFIG_BOOTLOADER_MCUBOOT=y → 0x10000  (slot0_partition)

get_filename_component(RT583_REPO_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

# Determine flash address: slot0 offset for MCUboot apps, else 0x0.
if(DEFINED CONFIG_FLASH_LOAD_OFFSET AND NOT CONFIG_FLASH_LOAD_OFFSET EQUAL 0)
    set(RT583_FLASH_ADDR "${CONFIG_FLASH_LOAD_OFFSET}")
else()
    set(RT583_FLASH_ADDR "0x0")
endif()

if(CMAKE_HOST_WIN32)
    board_runner_args(openocd
        "--openocd=${RT583_REPO_DIR}/tools/windows/openocd.exe"
        "--openocd-search=${RT583_REPO_DIR}/tools/windows/tcl"
        "--config=interface/cmsis-dap.cfg"
        "--config=target/rt58x.cfg"
        "--target-handle=RT58x.cpu"
        "--file-type=bin"
        "--flash-address=${RT583_FLASH_ADDR}"
        "--cmd-load=flash write_image erase"
        "--cmd-verify=verify_image"
    )
else()
    board_runner_args(openocd
        "--config=interface/cmsis-dap.cfg"
        "--config=target/rt58x.cfg"
        "--target-handle=RT58x.cpu"
        "--file-type=bin"
        "--flash-address=${RT583_FLASH_ADDR}"
        "--cmd-load=flash write_image erase"
        "--cmd-verify=verify_image"
    )
endif()

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
