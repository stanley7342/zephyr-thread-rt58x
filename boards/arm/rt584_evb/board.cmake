# SPDX-License-Identifier: Apache-2.0
#
# west flash runner for RT584 EVB (CMSIS-DAP + openocd, target/rt584.cfg)
#
# rt584.cfg declares `flash bank rt584 0x10000000 0x100000 ...` and a
# work-area at 0x30000000, size 192 KB — both RT584-specific, distinct
# from the rt58x target used for RT583.
#
# Flash absolute address per-image = 0x10000000 + CONFIG_FLASH_LOAD_OFFSET:
#   MCUboot itself (offset 0)             → 0x10000000
#   App with CONFIG_BOOTLOADER_MCUBOOT=y  → 0x10010000 (slot0_partition)

get_filename_component(RT584_REPO_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(DEFINED CONFIG_FLASH_LOAD_OFFSET AND NOT CONFIG_FLASH_LOAD_OFFSET EQUAL 0)
    math(EXPR RT584_FLASH_ADDR "0x10000000 + ${CONFIG_FLASH_LOAD_OFFSET}" OUTPUT_FORMAT HEXADECIMAL)
else()
    set(RT584_FLASH_ADDR "0x10000000")
endif()

if(CMAKE_HOST_WIN32)
    board_runner_args(openocd
        "--openocd=${RT584_REPO_DIR}/tools/windows/openocd.exe"
        "--openocd-search=${RT584_REPO_DIR}/tools/windows/tcl"
        "--config=interface/cmsis-dap.cfg"
        "--config=target/rt584.cfg"
        "--target-handle=RT584.cpu"
        "--file-type=bin"
        "--flash-address=${RT584_FLASH_ADDR}"
        "--cmd-load=flash write_image erase"
        "--cmd-verify=verify_image"
    )
else()
    board_runner_args(openocd
        "--config=interface/cmsis-dap.cfg"
        "--config=target/rt584.cfg"
        "--target-handle=RT584.cpu"
        "--file-type=bin"
        "--flash-address=${RT584_FLASH_ADDR}"
        "--cmd-load=flash write_image erase"
        "--cmd-verify=verify_image"
    )
endif()

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
