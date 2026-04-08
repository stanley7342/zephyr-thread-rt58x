cmake_minimum_required(VERSION 3.20.0)

# Project root lives two levels above this example (examples/thread/ → repo root).
get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(BOARD_ROOT   ${RT583_PROJECT_DIR})
set(SOC_ROOT     ${RT583_PROJECT_DIR})
set(DTS_ROOT     ${RT583_PROJECT_DIR})
set(KCONFIG_ROOT ${RT583_PROJECT_DIR}/Kconfig)

# Register the project root as a Zephyr module so that cmake/zephyr_module/
# CMakeLists.txt is included at the correct point in Zephyr's build graph.
# This contributes RT583 platform sources, UART, and flash drivers.
list(APPEND ZEPHYR_EXTRA_MODULES ${RT583_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt583_ot_ftd_cli VERSION 1.0.0)

target_sources(app PRIVATE src/main.c)
if(CONFIG_OTA)
    target_sources(app PRIVATE src/ota_app.c)
endif()
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

# RT583 platform include dirs for app-level sources (OT port, main.c, etc.)
# Platform sources themselves are provided by cmake/zephyr_module/CMakeLists.txt.
set(RT583_PLATFORM ${RT583_PROJECT_DIR}/sdk/components/platform)
set(RT569_RF_DIR   ${RT583_PROJECT_DIR}/sdk/components/network/rt569-rf/rt583)
set(OT_ROOT        ${RT583_PROJECT_DIR}/sdk/components/network/thread/openthread)

target_include_directories(app PRIVATE
    ${RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
    ${RT583_PLATFORM}/soc/rt583/rt583_system/Include
    ${RT583_PLATFORM}/hosal/rt583_hosal/Inc
    # RF MCU types — needed by comm_subsystem_drv.c
    ${RT569_RF_DIR}/Inc
    # OpenThread public API — needed by main.c (<openthread/cli.h> etc.)
    ${OT_ROOT}/include
)

# comm_subsystem_drv.c is only needed by the Thread example (not MCUboot).
target_sources(app PRIVATE
    ${RT583_PLATFORM}/soc/rt583/rt583_driver/Src/comm_subsystem_drv.c
)

# ── OpenThread subsystem ──────────────────────────────────────────────────────
# RAFAEL_SDK must be set before including subsys/openthread/CMakeLists.txt
# because it uses the variable to locate SDK sources.
if(CONFIG_OPENTHREAD_RT583)
    set(RAFAEL_SDK ${RT583_PROJECT_DIR}/sdk)
    include(${RT583_PROJECT_DIR}/subsys/openthread/CMakeLists.txt)
endif()

# ── OTA subsystem ─────────────────────────────────────────────────────────────
if(CONFIG_OTA)
    include(${RT583_PROJECT_DIR}/subsys/ota/CMakeLists.txt)
endif()

# ── Linker: suppress multiple-definition conflicts ────────────────────────────
# CONFIG_OPENTHREAD=y activates Zephyr's OT module glue (libopenthread_utils,
# libopenthread_platform) even when OPENTHREAD_RT583 is the choice selection and
# OPENTHREAD_SOURCES=n.  Our SDK OT port (ot_alarm.c, ot_zephyr.c, ot_entropy.c,
# ot_misc.c, ot_settings.c, ot_logging.c, ot_crypto.c, crypto_platform.cpp) and
# OT-bundled mbedTLS overlap with those stubs and with Zephyr's TF-PSA-Crypto.
# hosal_trng_get_random_number appears in both drivers__entropy and the OT subsystem.
zephyr_ld_options(-Wl,--allow-multiple-definition)
