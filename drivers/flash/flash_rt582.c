/*
 * Zephyr flash driver for Rafael RT582
 *
 * Wraps the RT582 flashctl API (flash_read_n_bytes, flash_write_n_bytes,
 * flash_erase, flash_check_busy, flush_cache) into Zephyr's flash_driver_api.
 *
 * Key hardware constraints:
 *   - Write granularity: 256-byte pages (sub-page handled by flash_write_n_bytes)
 *   - Erase granularity: 4 KB sector (smallest erase unit for Zephyr API)
 *   - flash_erase() is non-blocking — must busy-wait until complete
 *   - flush_cache() required after every write/erase
 *   - Addresses < BOOT_LOADER_END_PROTECT_ADDR (0x7000) are rejected by HW
 */

#define DT_DRV_COMPAT rafael_rt582_flash

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "flashctl.h"

LOG_MODULE_REGISTER(flash_rt582, CONFIG_FLASH_LOG_LEVEL);

/* ── Flash parameters ───────────────────────────────────────────────────── */
#define RT582_FLASH_BASE    0x00000000
#define RT582_FLASH_SIZE    DT_INST_REG_SIZE(0)
#define RT582_PAGE_SIZE     256         /* write page size */
#define RT582_SECTOR_SIZE   4096        /* smallest erase unit */
#define RT582_ERASE_VALUE   0xFF

/* ── Driver data ────────────────────────────────────────────────────────── */
struct flash_rt582_data {
    struct k_mutex lock;
};

static struct flash_rt582_data flash_data;

/* ── Helpers ────────────────────────────────────────────────────────────── */

/* Busy-wait for flash to become idle. Returns 0 on success, -ETIMEDOUT. */
static int flash_wait_idle(void)
{
    uint32_t retries = 0;
    while (flash_check_busy()) {
        if (++retries > 1000000) {
            LOG_ERR("flash busy timeout");
            return -ETIMEDOUT;
        }
        k_busy_wait(1);
    }
    return 0;
}

/* ── flash_driver_api: read ─────────────────────────────────────────────── */
static int rt582_flash_read(const struct device *dev, off_t offset,
                            void *data, size_t len)
{
    struct flash_rt582_data *ctx = dev->data;

    if (!len) {
        return 0;
    }
    if (offset < 0 || (offset + len) > RT582_FLASH_SIZE) {
        return -EINVAL;
    }

    k_mutex_lock(&ctx->lock, K_FOREVER);

    uint32_t rc = flash_read_n_bytes((uint32_t)offset,
                                     (uint32_t)(uintptr_t)data,
                                     (uint32_t)len);
    k_mutex_unlock(&ctx->lock);

    return (rc == STATUS_SUCCESS) ? 0 : -EIO;
}

/* ── flash_driver_api: write ────────────────────────────────────────────── */
static int rt582_flash_write(const struct device *dev, off_t offset,
                             const void *data, size_t len)
{
    struct flash_rt582_data *ctx = dev->data;

    if (!len) {
        return 0;
    }
    if (offset < 0 || (offset + len) > RT582_FLASH_SIZE) {
        return -EINVAL;
    }

    k_mutex_lock(&ctx->lock, K_FOREVER);

    int ret = flash_wait_idle();
    if (ret) {
        goto out;
    }

    uint32_t rc = flash_write_n_bytes((uint32_t)offset,
                                      (uint32_t)(uintptr_t)data,
                                      (uint32_t)len);
    if (rc != STATUS_SUCCESS) {
        ret = -EIO;
        goto out;
    }

    ret = flash_wait_idle();
    if (ret == 0) {
        flush_cache();
    }

out:
    k_mutex_unlock(&ctx->lock);
    return ret;
}

/* ── flash_driver_api: erase ────────────────────────────────────────────── */
static int rt582_flash_erase(const struct device *dev, off_t offset,
                             size_t size)
{
    struct flash_rt582_data *ctx = dev->data;

    if (!size) {
        return 0;
    }
    if (offset < 0 || (offset + size) > RT582_FLASH_SIZE) {
        return -EINVAL;
    }
    /* Must be sector-aligned */
    if ((offset % RT582_SECTOR_SIZE) != 0 ||
        (size % RT582_SECTOR_SIZE) != 0) {
        return -EINVAL;
    }

    k_mutex_lock(&ctx->lock, K_FOREVER);

    int ret = 0;
    size_t remaining = size;
    uint32_t addr = (uint32_t)offset;

    while (remaining > 0) {
        flash_erase_mode_t mode;
        size_t erase_len;

        /* Pick largest aligned erase unit to minimize erase count */
        if (remaining >= LENGTH_64KB && (addr % LENGTH_64KB) == 0) {
            mode = FLASH_ERASE_64K;
            erase_len = LENGTH_64KB;
        } else if (remaining >= LENGTH_32KB && (addr % LENGTH_32KB) == 0) {
            mode = FLASH_ERASE_32K;
            erase_len = LENGTH_32KB;
        } else {
            mode = FLASH_ERASE_SECTOR;
            erase_len = LENGTH_4KB;
        }

        ret = flash_wait_idle();
        if (ret) {
            break;
        }

        uint32_t rc = flash_erase(mode, addr);
        if (rc != STATUS_SUCCESS) {
            LOG_ERR("flash_erase(0x%x, mode=%d) failed: %u", addr, mode, rc);
            ret = -EIO;
            break;
        }

        ret = flash_wait_idle();
        if (ret) {
            break;
        }

        addr += erase_len;
        remaining -= erase_len;
    }

    if (ret == 0) {
        flush_cache();
    }

    k_mutex_unlock(&ctx->lock);
    return ret;
}

/* ── flash_driver_api: get_parameters ───────────────────────────────────── */
static const struct flash_parameters rt582_flash_params = {
    .write_block_size = 1,      /* flash_write_n_bytes handles any alignment */
    .erase_value = RT582_ERASE_VALUE,
};

static const struct flash_parameters *
rt582_flash_get_parameters(const struct device *dev)
{
    ARG_UNUSED(dev);
    return &rt582_flash_params;
}

/* ── flash_driver_api: page_layout ──────────────────────────────────────── */
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
static const struct flash_pages_layout rt582_flash_layout = {
    .pages_count = RT582_FLASH_SIZE / RT582_SECTOR_SIZE,
    .pages_size  = RT582_SECTOR_SIZE,
};

static void rt582_flash_page_layout(const struct device *dev,
                                    const struct flash_pages_layout **layout,
                                    size_t *layout_size)
{
    ARG_UNUSED(dev);
    *layout = &rt582_flash_layout;
    *layout_size = 1;
}
#endif /* CONFIG_FLASH_PAGE_LAYOUT */

/* ── Driver API struct ──────────────────────────────────────────────────── */
static const struct flash_driver_api rt582_flash_api = {
    .read = rt582_flash_read,
    .write = rt582_flash_write,
    .erase = rt582_flash_erase,
    .get_parameters = rt582_flash_get_parameters,
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
    .page_layout = rt582_flash_page_layout,
#endif
};

/* ── Init ───────────────────────────────────────────────────────────────── */
static int rt582_flash_init(const struct device *dev)
{
    struct flash_rt582_data *ctx = dev->data;

    k_mutex_init(&ctx->lock);
    flash_timing_init();
    flash_enable_qe();
    flash_set_read_pagesize();

    LOG_INF("RT582 flash driver ready (size=%u KB)", RT582_FLASH_SIZE / 1024);
    return 0;
}

/* ── Device instantiation ───────────────────────────────────────────────── */
DEVICE_DT_INST_DEFINE(0,
                      rt582_flash_init,
                      NULL,                     /* pm */
                      &flash_data,              /* data */
                      NULL,                     /* config */
                      POST_KERNEL,
                      CONFIG_FLASH_INIT_PRIORITY,
                      &rt582_flash_api);
