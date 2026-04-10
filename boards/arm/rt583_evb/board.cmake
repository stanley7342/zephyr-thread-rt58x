# SPDX-License-Identifier: Apache-2.0
#
# west flash runner for RT583 EVB (CMSIS-DAP + openocd-rt58x)
#
# Windows: uses the bundled tools/windows/openocd.exe automatically.
# Linux/macOS: install openocd-rt58x and set OPENOCD env var if not in PATH.
#              Set OPENOCD_DEFAULT_PATH to the tcl/ directory of your install.

get_filename_component(RT583_REPO_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

if(CMAKE_HOST_WIN32)
    # Bundled openocd in repo — no external install needed on Windows.
    # HEX mode: load address is embedded in the Intel HEX file, so this works
    # for both standalone (0x0) and MCUboot (slot0 = 0x10000) images without
    # changing board.cmake per app.
    board_runner_args(openocd
        "--openocd=${RT583_REPO_DIR}/tools/windows/openocd.exe"
        "--openocd-search=${RT583_REPO_DIR}/tools/windows/tcl"
        "--config=interface/cmsis-dap.cfg"
        "--config=target/rt58x.cfg"
        "--target-handle=RT58x.cpu"
        "--file-type=hex"
        "--cmd-load=flash write_image erase"
        "--cmd-verify=verify_image"
    )
else()
    # Linux/macOS: openocd-rt58x must be in PATH (or set OPENOCD env var).
    # Point OPENOCD_DEFAULT_PATH to the tcl/ directory, e.g.:
    #   export OPENOCD_DEFAULT_PATH=~/openocd-rt58x/tcl
    board_runner_args(openocd
        "--config=interface/cmsis-dap.cfg"
        "--config=target/rt58x.cfg"
        "--target-handle=RT58x.cpu"
        "--file-type=hex"
        "--cmd-load=flash write_image erase"
        "--cmd-verify=verify_image"
    )
endif()

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
