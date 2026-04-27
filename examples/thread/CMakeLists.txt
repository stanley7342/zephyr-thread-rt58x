cmake_minimum_required(VERSION 3.20.0)

# Project root lives two levels above this example (examples/thread/ → repo root).
get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOARD_ROOT   ${RT583_PROJECT_DIR})
set(SOC_ROOT     ${RT583_PROJECT_DIR})
set(DTS_ROOT     ${RT583_PROJECT_DIR})
set(KCONFIG_ROOT ${RT583_PROJECT_DIR}/Kconfig)

# Register the project root as a Zephyr module so that cmake/zephyr_module/
# CMakeLists.txt is included at the correct point in Zephyr's build graph.
# That module contributes all SoC-aware platform sources, include dirs,
# and driver wiring — example stays SoC-agnostic.
list(APPEND ZEPHYR_EXTRA_MODULES ${RT583_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rafael_ot_ftd_cli VERSION 1.0.0)

target_sources(app PRIVATE src/main.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

# Increase OT task stack from default 4 KB to 8 KB — CLI command processing
# (otCliInputLine → version/state/dataset handlers) can consume 3-4 KB of stack
# on top of the base OT FTD loop frame, overflowing the 4 KB default.
target_compile_definitions(app PRIVATE CONFIG_OPENTHREAD_TASK_SIZE=8192)

if(CONFIG_OTA)
    target_sources(app PRIVATE src/ota_app.c)
    include(${RT583_PROJECT_DIR}/subsys/ota/CMakeLists.txt)
endif()

# OpenThread subsystem (SDK port) — pulls in the SDK OT core + Rafael
# port glue. Sets up app's OT/LMAC/HOSAL include paths internally.
if(CONFIG_OPENTHREAD_RT583 OR CONFIG_OPENTHREAD_RT584)
    set(RAFAEL_SDK ${RT583_PROJECT_DIR}/sdk)
    include(${RT583_PROJECT_DIR}/subsys/openthread/CMakeLists.txt)
endif()

# OT's bundled mbedTLS 3.6 overlaps with Zephyr's TF-PSA-Crypto 4.0 for
# platform.c / cipher.c / constant_time.c / psa_util.c — these MUST use OT's
# 3.6 copies because OT code is compiled against 3.6 headers (struct layouts
# differ between 3.6 and 4.0; mixing causes mbedtls_cipher_free to deref
# ctx_free_func at the wrong offset).  libapp.a precedes libtfpsacrypto.a in
# link order, so OT's 3.6 .o files win when this flag is set.
zephyr_ld_options(-Wl,--allow-multiple-definition)
