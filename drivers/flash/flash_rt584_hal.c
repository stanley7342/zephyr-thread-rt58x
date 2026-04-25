/*
 * flash_rt584_hal.c — HAL shim for RT584 flash driver
 *
 * Includes flashctl.h (which defines flash_erase, flash_check_busy, …) and
 * re-exposes them under rt584_hal_* prefixed names. Keeps the symbols out
 * of flash_rt584.c, which #includes <zephyr/drivers/flash.h> and would
 * otherwise collide (Zephyr defines its own flash_erase syscall wrapper).
 *
 * Address convention: Zephyr passes 0-based offsets into the flash device.
 * With CONFIG_FLASHCTRL_SECURE_EN defined (secure-world build), the SDK's
 * flash_check_address validates against FLASH_SECURE_MODE_BASE_ADDR =
 * 0x10000000 — i.e., it expects absolute CPU addresses, not offsets.
 * We add the base here so the rest of Zephyr's flash API stays offset-based.
 */

#include "flashctl.h"

#define RT584_FLASH_BASE  0x10000000u

uint32_t rt584_hal_flash_read(uint32_t off, uint32_t buf, uint32_t len)
{
    return flash_read_n_bytes(RT584_FLASH_BASE + off, buf, len);
}

uint32_t rt584_hal_flash_write(uint32_t off, uint32_t buf, uint32_t len)
{
    return flash_write_n_bytes(RT584_FLASH_BASE + off, buf, len);
}

uint32_t rt584_hal_flash_erase_sector(uint32_t off)
{
    return flash_erase(FLASH_ERASE_SECTOR, RT584_FLASH_BASE + off);
}

uint32_t rt584_hal_flash_erase_32k(uint32_t off)
{
    return flash_erase(FLASH_ERASE_32K, RT584_FLASH_BASE + off);
}

uint32_t rt584_hal_flash_erase_64k(uint32_t off)
{
    return flash_erase(FLASH_ERASE_64K, RT584_FLASH_BASE + off);
}

int rt584_hal_flash_busy(void)
{
    return flash_check_busy() ? 1 : 0;
}

void rt584_hal_flush_cache(void)
{
    flush_cache();
}

void rt584_hal_flash_init(void)
{
    flash_timing_init();
    flash_enable_qe();
    flash_set_read_pagesize();
}
