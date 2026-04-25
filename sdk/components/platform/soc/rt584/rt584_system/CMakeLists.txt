sdk_generate_library(rt584_system)

if(CONFIG_RF1301)
sdk_set_linker_script(${CMAKE_CURRENT_LIST_DIR}/ldscripts/rf1301.ld)
endif()

if(CONFIG_RT584H)
sdk_set_linker_script(${CMAKE_CURRENT_LIST_DIR}/ldscripts/rt584H.ld)
endif()

if(CONFIG_RT584HA4)
sdk_set_linker_script(${CMAKE_CURRENT_LIST_DIR}/ldscripts/rt584HA4.ld)
endif()

if(CONFIG_RT584L)
sdk_set_linker_script(${CMAKE_CURRENT_LIST_DIR}/ldscripts/rt584L.ld)
endif()

sdk_add_include_directories(Include)

if(CONFIG_BOOTLOADER)
sdk_library_add_sources(
    Src/system_mcu.c
    Src/mp_sector.c
)
else()
sdk_library_add_sources(
    Startup/startup.c
    Src/system_mcu.c
    Src/mp_sector.c
)
endif()


