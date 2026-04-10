cmake_minimum_required(VERSION 3.20.0)

get_filename_component(RT583_PROJECT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

set(BOARD_ROOT   ${RT583_PROJECT_DIR})
set(SOC_ROOT     ${RT583_PROJECT_DIR})
set(DTS_ROOT     ${RT583_PROJECT_DIR})
set(KCONFIG_ROOT ${RT583_PROJECT_DIR}/Kconfig)

list(APPEND ZEPHYR_EXTRA_MODULES ${RT583_PROJECT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(rt583_test_hci VERSION 1.0.0)

target_sources(app PRIVATE src/main.c)

set(RT583_PLATFORM ${RT583_PROJECT_DIR}/sdk/components/platform)
target_include_directories(app PRIVATE
    ${RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
    ${RT583_PLATFORM}/soc/rt583/rt583_system/Include
    ${RT583_PLATFORM}/hosal/rt583_hosal/Inc
)

target_sources(app PRIVATE
    ${RT583_PLATFORM}/soc/rt583/rt583_driver/Src/comm_subsystem_drv.c
)

if(CONFIG_BLE_RT583)
    set(RAFAEL_SDK ${RT583_PROJECT_DIR}/sdk)
    include(${RT583_PROJECT_DIR}/subsys/ble/CMakeLists.txt)

    # Override: enable multi-protocol (BLE + 802.15.4) firmware blob so that
    # hosal_rf_init(HOSAL_RF_MODE_MULTI_PROTOCOL) succeeds in this test.
    # subsys/ble sets CONFIG_RF_FW_INCLUDE_BLE=TRUE / PCI=FALSE (BLE-only).
    # We need MULTI_2P4G=TRUE and PCI=TRUE to load the multi-protocol binary.
    target_compile_options(app PRIVATE
        -UCONFIG_RF_FW_INCLUDE_BLE
        -DCONFIG_RF_FW_INCLUDE_BLE=FALSE
        -UCONFIG_RF_FW_INCLUDE_PCI
        -DCONFIG_RF_FW_INCLUDE_PCI=TRUE
        -UCONFIG_RF_FW_INCLUDE_MULTI_2P4G
        -DCONFIG_RF_FW_INCLUDE_MULTI_2P4G=TRUE
    )
endif()
