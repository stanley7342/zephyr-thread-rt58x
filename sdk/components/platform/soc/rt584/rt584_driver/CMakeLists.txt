sdk_generate_library(rt584_driver)
sdk_add_include_directories(
    CMSIS/Core/Include
    Inc
)
if(CONFIG_BOOTLOADER)
sdk_library_add_sources(
    Src/bod_comp.c
    Src/dma.c
    Src/gpio.c
    Src/flashctl.c
    Src/sysctrl.c
    Src/dwt.c
    Src/sadc.c
    Src/timer.c
    Src/wdt.c
    Src/sysfun.c 
)
else()
sdk_library_add_sources(
    Src/bod_comp.c
    Src/comm_subsystem_drv.c
    Src/flashctl.c
    Src/gpio.c
    Src/sysctrl.c
    Src/dwt.c
    Src/sysfun.c
    Src/wdt.c
    Src/sadc.c
    Src/otp.c
    Src/lpm.c
    Src/trng.c
)

    if(CONFIG_PERIPHERAL_AUX_COMP)
        message("--       + Peipheral aux_comp ON")
        sdk_library_add_sources(Src/aux_comp.c)
    else()
        message("--       + Peipheral aux_comp OFF")
    endif()

    if(CONFIG_PERIPHERAL_BOD_COMP)
        message("--       + Peipheral bod_comp ON")
        sdk_library_add_sources(Src/bod_comp.c)
    else()
        message("--       + Peipheral bod_comp OFF")
    endif()

    if(CONFIG_PERIPHERAL_DMA)
        message("--       + Peipheral dma ON")
        sdk_library_add_sources(Src/dma.c)
    else()
        message("--       + Peipheral dma OFF")
    endif()

    if(CONFIG_PERIPHERAL_I2C)
        message("--       + Peipheral i2c ON")
        sdk_library_add_sources(
            Src/i2c_master.c
            Src/i2c_slave.c
            )
    else()
        message("--       + Peipheral i2c OFF")
    endif()

    if(CONFIG_PERIPHERAL_I2S)
        message("--       + Peipheral i2s ON")
        sdk_library_add_sources(Src/i2s.c)
    else()
        message("--       + Peipheral i2s OFF")
    endif()

    if(CONFIG_PERIPHERAL_IRM)
        message("--       + Peipheral irm ON")
        sdk_library_add_sources(Src/irm.c)
    else()
        message("--       + Peipheral irm OFF")
    endif()

    if(CONFIG_PERIPHERAL_TIMER)
        message("--       + Peipheral timer ON")
        sdk_library_add_sources(Src/timer.c)
    else()
        message("--       + Peipheral timer OFF")
    endif()

    if(CONFIG_PERIPHERAL_PWM)
        message("--       + Peipheral pwm ON")  
        sdk_library_add_sources(Src/pwm.c)
    else()
        message("--       + Peipheral pwm OFF")
    endif()

    if(CONFIG_PERIPHERAL_RTC)
        message("--       + Peipheral rtc ON")
        sdk_library_add_sources(Src/rtc.c)
    else()
        message("--       + Peipheral rtc OFF")
    endif()

    if(CONFIG_PERIPHERAL_SADC)
        message("--       + Peipheral sadc ON")
        sdk_library_add_sources(Src/sadc.c)
    else()
        message("--       + Peipheral sadc OFF")
    endif()

    if(CONFIG_PERIPHERAL_SWI)
        message("--       + Peipheral swi ON")
        sdk_library_add_sources( Src/swi.c)
    else()
        message("--       + Peipheral swi OFF")
    endif()

endif()
