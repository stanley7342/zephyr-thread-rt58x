zephyr_library()
zephyr_library_sources_ifdef(CONFIG_FLASH_RT583
    flash_rt583.c
    flash_rt583_hal.c
)
