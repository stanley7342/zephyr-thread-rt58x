/*
 * flash_rt582_hal.c — HAL shim for RT582 flash driver
 *
 * This file includes flashctl.h (which defines flash_erase, flash_check_busy,
 * etc.) and exposes them under rt582_hal_* prefixed names. This avoids the
 * symbol conflict with Zephyr's <zephyr/drivers/flash.h> which defines its
 * own flash_erase() syscall wrapper.
 *
 * flash_rt582.c calls these rt582_hal_* functions instead of the raw
 * flashctl functions directly.
 */

#include "flashctl.h"

uint32_t rt582_hal_flash_read(uint32_t addr, uint32_t buf, uint32_t len)
{
    return flash_read_n_bytes(addr, buf, len);
}

uint32_t rt582_hal_flash_write(uint32_t addr, uint32_t buf, uint32_t len)
{
    return flash_write_n_bytes(addr, buf, len);
}

uint32_t rt582_hal_flash_erase_sector(uint32_t addr)
{
    return flash_erase(FLASH_ERASE_SECTOR, addr);
}

uint32_t rt582_hal_flash_erase_32k(uint32_t addr)
{
    return flash_erase(FLASH_ERASE_32K, addr);
}

uint32_t rt582_hal_flash_erase_64k(uint32_t addr)
{
    return flash_erase(FLASH_ERASE_64K, addr);
}

int rt582_hal_flash_busy(void)
{
    return flash_check_busy() ? 1 : 0;
}

void rt582_hal_flush_cache(void)
{
    flush_cache();
}

void rt582_hal_flash_init(void)
{
    flash_timing_init();
    flash_enable_qe();
    flash_set_read_pagesize();
}
