# cmake/zephyr_module/CMakeLists.txt
#
# Zephyr module entry point — included via ZEPHYR_EXTRA_MODULES during
# Zephyr's module loop. Contributes out-of-tree drivers and Rafael platform
# sources to Zephyr's library targets.
#
# NOTE: CMAKE_SOURCE_DIR points to the app source dir (e.g. examples/thread/),
# not the module root. Use CMAKE_CURRENT_LIST_DIR to derive the module root.
get_filename_component(_RT583_MODULE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

set(_RT583_PLATFORM ${_RT583_MODULE_ROOT}/sdk/components/platform)

# ══════════════════════════════════════════════════════════════════════════
# RT583 (Cortex-M3)
# ══════════════════════════════════════════════════════════════════════════
if(CONFIG_SOC_SERIES_RT583)
    set(_RT583_DRV_SRC  ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Src)
    set(_RT583_SYS_SRC  ${_RT583_PLATFORM}/soc/rt583/rt583_system/Src)

    # ── RT583 platform sources (always required) ──────────────────────────
    # Provide SystemInit, enable_perclk, delay_us, pin_set_mode, etc.
    # needed by soc.c, the UART driver, and the flash driver regardless of
    # which application (thread FTD CLI or MCUboot) is being built.
    zephyr_compile_definitions(
        SET_SYS_CLK=2
        CHIP_TYPE=3
    )

    zephyr_sources(
        ${_RT583_SYS_SRC}/system_mcu.c
        ${_RT583_SYS_SRC}/mp_sector.c
        ${_RT583_DRV_SRC}/sysctrl.c
        ${_RT583_DRV_SRC}/sysfun.c
        ${_RT583_DRV_SRC}/flashctl.c
        ${_RT583_DRV_SRC}/flash_bp_protect.c
        ${_RT583_DRV_SRC}/gpio.c
        ${_RT583_DRV_SRC}/dma.c
        ${_RT583_DRV_SRC}/rtc.c
        ${_RT583_DRV_SRC}/swi.c
        ${_RT583_DRV_SRC}/timer.c
        ${_RT583_DRV_SRC}/trng.c
        ${_RT583_DRV_SRC}/wdt.c
        ${_RT583_DRV_SRC}/pwm.c
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Src/hosal_pwm.c
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Src/hosal_sysctrl.c
    )

    zephyr_include_directories(
        ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
        ${_RT583_PLATFORM}/soc/rt583/rt583_system/Include
        ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
    )

    # ── UART driver (drivers__serial) ─────────────────────────────────────
    if(CONFIG_UART_RT583)
        target_sources(drivers__serial PRIVATE
            ${_RT583_MODULE_ROOT}/drivers/serial/uart_rt583.c
            ${_RT583_PLATFORM}/hosal/rt583_hosal/Src/hosal_uart.c
        )

        target_include_directories(drivers__serial PRIVATE
            ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
            ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
        )
    endif()

    # ── Entropy driver (drivers__entropy) ─────────────────────────────────
    # Provides CONFIG_ENTROPY_HAS_DRIVER=y so that CONFIG_CSPRNG_ENABLED is set
    # and Zephyr's OT platform entropy.c compiles without error.
    if(CONFIG_ENTROPY_RT583 AND TARGET drivers__entropy)
        target_sources(drivers__entropy PRIVATE
            ${_RT583_MODULE_ROOT}/drivers/entropy/entropy_rt583.c
            ${_RT583_PLATFORM}/hosal/rt583_hosal/Src/hosal_trng.c
        )
        target_include_directories(drivers__entropy PRIVATE
            ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
            ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
        )
    endif()

    # ── IEEE 802.15.4 stub driver (drivers__ieee802154) ───────────────────
    # Provides DEVICE_DT_DEFINE for the rt583 ieee802154 DTS node so that
    # Zephyr's openthread/platform/radio.c can resolve DT_CHOSEN(zephyr_ieee802154).
    if(CONFIG_IEEE802154_RT583 AND TARGET drivers__ieee802154)
        target_sources(drivers__ieee802154 PRIVATE
            ${_RT583_MODULE_ROOT}/drivers/ieee802154/ieee802154_rt583.c
        )
        target_include_directories(drivers__ieee802154 PRIVATE
            ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
            # Real driver (CONFIG_IEEE802154_RT583_FULL) needs lmac15p4 + hosal_rf + hosal_trng
            ${_RT583_MODULE_ROOT}/sdk/components/network/lmac15p4/Inc
            ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
            # Also needs rf_mcu_types (included transitively by hosal_rf.h)
            ${_RT583_MODULE_ROOT}/sdk/components/network/rt569-rf/rt583/Inc
        )
    endif()

    # ── Flash driver (drivers__flash) ─────────────────────────────────────
    # flash_rt583.c includes <zephyr/drivers/flash.h> (Zephyr flash API).
    # flash_rt583_hal.c includes "flashctl.h" (RT583 HW API).
    # They MUST be separate TUs because both headers define flash_erase().
    if(CONFIG_FLASH_RT583 AND TARGET drivers__flash)
        target_sources(drivers__flash PRIVATE
            ${_RT583_MODULE_ROOT}/drivers/flash/flash_rt583.c
            ${_RT583_MODULE_ROOT}/drivers/flash/flash_rt583_hal.c
        )

        target_include_directories(drivers__flash PRIVATE
            ${_RT583_PLATFORM}/soc/rt583/rt583_driver/Inc
            ${_RT583_PLATFORM}/soc/rt583/rt583_system/Include
            ${_RT583_PLATFORM}/hosal/rt583_hosal/Inc
        )
    endif()
endif() # CONFIG_SOC_SERIES_RT583

# ══════════════════════════════════════════════════════════════════════════
# RT584 (Cortex-M33) — minimal bring-up: system clock + UART only.
# ══════════════════════════════════════════════════════════════════════════
if(CONFIG_SOC_SERIES_RT584)
    set(_RT584_DRV_SRC ${_RT583_PLATFORM}/soc/rt584/rt584_driver/Src)
    set(_RT584_SYS_SRC ${_RT583_PLATFORM}/soc/rt584/rt584_system/Src)
    set(_RT584_HOSAL_SRC ${_RT583_PLATFORM}/hosal/rt584_hosal/Src)

    # rt584 peripheral aliasing (verified empirically with GDB on hardware):
    #   - 0x4xxxxxxx (NS alias) reads always return 0; writes are dropped.
    #     This is what happens even from secure state — the IDAU on rt584
    #     does NOT make the NS alias accessible the way the ARMv8-M
    #     architecture spec implies. UART0 LSR at 0x40012014 always reads
    #     0, so hosal_uart_send spins on THRE forever and no UART output
    #     ever appears.
    #   - 0x5xxxxxxx (secure alias) is the real peripheral. UART0 LSR at
    #     0x50012014 actually reflects hardware (e.g. 0x60 = THRE|TEMT).
    # The vendor's `CONFIG_*_SECURE_EN` defines OR PERIPH_SECURE_OFFSET
    # (0x10000000) into peripheral pointer macros, routing every access
    # to the secure alias. Required, not optional — without these flags
    # the chip silently no-ops every peripheral write.
    # rt584 peripheral aliasing: the entire 0x4xxxxxxx NS bank is
    # non-functional from secure state. Every peripheral access must go
    # through the 0x5xxxxxxx secure alias, which the vendor macros
    # produce only when the corresponding CONFIG_*_SECURE_EN is defined.
    # Mirror the full set from upstream default-rt584h-evb.config — anything
    # we leave NS-aliased silently no-ops at runtime (hosal_rf_init hung
    # because COMM_SUBSYSTEM_AHB was at NS, never asserted IRQ 45).
    zephyr_compile_definitions(
        SET_SYS_CLK=2
        CONFIG_HELLOWORLD=1
        CONFIG_HOSAL_SOC_DISABLE_MP_SECTOR_INIT=1
        CONFIG_AUX_COMP_SECURE_EN=1
        CONFIG_BOD_COMP_SECURE_EN=1
        CONFIG_COMM_SUBSYSTEM_AHB_SECURE_EN=1
        CONFIG_DMA0_SECURE_EN=1
        CONFIG_DMA1_SECURE_EN=1
        CONFIG_DPD_SECURE_EN=1
        CONFIG_FLASHCTRL_SECURE_EN=1
        CONFIG_GPIO_SECURE_EN=1
        CONFIG_I2C_MASTER0_SECURE_EN=1
        CONFIG_I2C_MASTER1_SECURE_EN=1
        CONFIG_I2C_SLAVE_SECURE_EN=1
        CONFIG_I2S0_SECURE_EN=1
        CONFIG_IRM_SECURE_EN=1
        CONFIG_PWM_SECURE_EN=1
        CONFIG_QSPI0_SECURE_EN=1
        CONFIG_QSPI1_SECURE_EN=1
        CONFIG_RCO1M_CAL_SECURE_EN=1
        CONFIG_RCO32K_CAL_SECURE_EN=1
        CONFIG_RTC_SECURE_EN=1
        CONFIG_SADC_SECURE_EN=1
        CONFIG_SLOWTIMER0_SECURE_EN=1
        CONFIG_SLOWTIMER1_SECURE_EN=1
        CONFIG_SOC_PMU_SECURE_EN=1
        CONFIG_SWI0_SECURE_EN=1
        CONFIG_SWI1_SECURE_EN=1
        CONFIG_SYSCTRL_SECURE_EN=1
        CONFIG_TIMER0_SECURE_EN=1
        CONFIG_TIMER1_SECURE_EN=1
        CONFIG_TIMER2_SECURE_EN=1
        CONFIG_UART0_SECURE_EN=1
        CONFIG_UART1_SECURE_EN=1
        CONFIG_UART2_SECURE_EN=1
        CONFIG_WDT_SECURE_EN=1
        CONFIG_XDMA_SECURE_EN=1
    )

    # Platform sources: sysctrl (clock + pinmux), sysfun (delay),
    # gpio, flashctl (for flash driver + MCUboot), hosal_sysctrl.
    # Warning-suppression is done in-source (vendor files patched directly)
    # because set_source_files_properties is scope-local and doesn't reach
    # Zephyr's library target directory where the build rules live.
    zephyr_sources(
        ${_RT584_SYS_SRC}/system_mcu.c
        ${_RT584_DRV_SRC}/sysctrl.c
        ${_RT584_DRV_SRC}/sysfun.c
        ${_RT584_DRV_SRC}/gpio.c
        ${_RT584_DRV_SRC}/flashctl.c
        ${_RT584_HOSAL_SRC}/hosal_sysctrl.c
    )

    zephyr_include_directories(
        ${_RT583_PLATFORM}/soc/rt584/rt584_driver/Inc
        ${_RT583_PLATFORM}/soc/rt584/rt584_system/Include
        ${_RT583_PLATFORM}/hosal/rt584_hosal/Inc
    )

    # UART driver — uart_rt584.c + hosal_uart.c wired into drivers__serial
    # so they pick up the right include paths.
    if(CONFIG_UART_RT584)
        target_sources(drivers__serial PRIVATE
            ${_RT583_MODULE_ROOT}/drivers/serial/uart_rt584.c
            ${_RT584_HOSAL_SRC}/hosal_uart.c
        )

        target_include_directories(drivers__serial PRIVATE
            ${_RT583_PLATFORM}/soc/rt584/rt584_driver/Inc
            ${_RT583_PLATFORM}/soc/rt584/rt584_system/Include
            ${_RT583_PLATFORM}/hosal/rt584_hosal/Inc
        )
    endif()

    # Flash driver (drivers__flash) — flash_rt584.c includes
    # <zephyr/drivers/flash.h>; flash_rt584_hal.c includes "flashctl.h".
    # Must be separate TUs because both headers define flash_erase().
    if(CONFIG_FLASH_RT584 AND TARGET drivers__flash)
        target_sources(drivers__flash PRIVATE
            ${_RT583_MODULE_ROOT}/drivers/flash/flash_rt584.c
            ${_RT583_MODULE_ROOT}/drivers/flash/flash_rt584_hal.c
        )

        target_include_directories(drivers__flash PRIVATE
            ${_RT583_PLATFORM}/soc/rt584/rt584_driver/Inc
            ${_RT583_PLATFORM}/soc/rt584/rt584_system/Include
            ${_RT583_PLATFORM}/hosal/rt584_hosal/Inc
        )
    endif()
endif() # CONFIG_SOC_SERIES_RT584


# ══════════════════════════════════════════════════════════════════════════
# OpenThread platform integration — applies to BOTH rt583 and rt584 builds.
# When CONFIG_OPENTHREAD_RT58{3,4}=y, our subsys/openthread/CMakeLists.txt
# pulls the SDK OT sources into libapp.a. Zephyr's modules/lib/openthread
# is still loaded but its CONFIG_OPENTHREAD_SOURCES=n suppresses most
# duplicate compilation; the leftovers (entropy/alarm/platform stubs in
# openthread_platform target, openthread.c in openthread_utils) need to
# be stripped because their otPlat*/otSys* symbols collide with our SDK
# port's. We strip them here (deferred to after the OT module finishes
# creating its targets).
# ══════════════════════════════════════════════════════════════════════════
if(CONFIG_OPENTHREAD_RT583 OR CONFIG_OPENTHREAD_RT584)
    # OpenThread public headers (for Matter GN build):
    # CONFIG_OPENTHREAD_SOURCES=n means Zephyr's OT module does NOT add
    # modules/lib/openthread/include to the global system include path.
    # Add it here so:
    #  1. Zephyr's OT L2 layer (openthread.c) can find <openthread/instance.h>
    #  2. Matter's chip-module picks it up via zephyr_get_compile_flags() and
    #     writes it into the GN build's args.gn target_cflags — must happen
    #     during find_package(Zephyr) module processing (before chip-module
    #     runs).
    zephyr_system_include_directories(
        ${ZEPHYR_BASE}/../modules/lib/openthread/include
        ${ZEPHYR_BASE}/../modules/lib/openthread/examples/platforms
    )

    # Strip Zephyr OT platform stubs that we override in sdk/.
    # Zephyr's modules/openthread unconditionally builds openthread_platform
    # with default alarm / entropy / misc / platform / crypto stubs. Our
    # SDK provides hardware-backed replacements in subsys/openthread/platform
    # and sdk/.../openthread_port/Src/ that also export the same otPlat*
    # symbols — collisions previously were masked by
    # -Wl,--allow-multiple-definition.
    #
    # We defer a cleanup that edits the openthread_platform target AFTER
    # Zephyr's module processing has created it, removing files whose
    # symbols our sources re-define. memory.c / settings.c / logging.c /
    # radio.c shim are left intact (we don't override those).
    #
    # openthread.c in openthread_utils also defines
    # otTaskletsSignalPending / otSysEventSignalPending which our ot_zephyr.c
    # (Thread CLI build) defines for the ot_task_sem wake path. Strip it
    # only when Zephyr's L2 OT is NOT enabled (Matter build keeps L2 →
    # keeps openthread.c).
    function(_rafael_strip_ot_platform_sources)
        if(CONFIG_IEEE802154_RT583_FULL)
            set(_strip_regex
                "(alarm_milli|alarm_micro|alarm_counter|entropy|misc|platform|settings|logging|crypto_psa|memory)\\.c$"
            )
        else()
            set(_strip_regex
                "(alarm_milli|alarm_micro|alarm_counter|entropy|misc|platform|settings|logging|crypto_psa|radio|memory)\\.c$"
            )
        endif()
        set(_did_platform 0)
        set(_did_utils 0)
        if(TARGET openthread_platform)
            get_target_property(_srcs openthread_platform SOURCES)
            if(_srcs)
                set(_before ${_srcs})
                list(FILTER _srcs EXCLUDE REGEX "${_strip_regex}")
                set_target_properties(openthread_platform PROPERTIES SOURCES "${_srcs}")
                list(LENGTH _before _b)
                list(LENGTH _srcs _a)
                set(_did_platform 1)
                message(STATUS "[ot-strip] openthread_platform: ${_b} -> ${_a} sources")
            endif()
        endif()
        if(NOT CONFIG_NET_L2_OPENTHREAD AND TARGET openthread_utils)
            get_target_property(_srcs openthread_utils SOURCES)
            if(_srcs)
                list(LENGTH _srcs _b)
                list(FILTER _srcs EXCLUDE REGEX "(^|/)openthread\\.c$")
                set_target_properties(openthread_utils PROPERTIES SOURCES "${_srcs}")
                list(LENGTH _srcs _a)
                set(_did_utils 1)
                message(STATUS "[ot-strip] openthread_utils: ${_b} -> ${_a} sources")
            endif()
        endif()
        if(NOT _did_platform)
            message(STATUS "[ot-strip] openthread_platform target not found (skipped)")
        endif()
    endfunction()
    cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL _rafael_strip_ot_platform_sources)
endif() # CONFIG_OPENTHREAD_RT583 OR CONFIG_OPENTHREAD_RT584
