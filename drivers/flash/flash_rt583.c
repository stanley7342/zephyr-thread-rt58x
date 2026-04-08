/*
 * Zephyr flash driver for Rafael RT583
 *
 * Wraps the RT583 flashctl API into Zephyr's flash_driver_api.
 *
 * IMPORTANT: We cannot #include "flashctl.h" here because it defines
 * flash_erase(), flash_read(), etc. which conflict with Zephyr's
 * flash.h syscall wrappers of the same names. Instead, we declare
 * the raw hardware functions we need via extern with rt583_ prefixed
 * wrapper names, and call the originals from a separate .c file
 * (flash_rt583_hal.c) that does NOT include <zephyr/drivers/flash.h>.
 *
 * Key hardware constraints:
 *   - Write granularity: 256-byte pages (sub-page handled by flash_write_n_bytes)
 *   - Erase granularity: 4 KB sector (smallest erase unit for Zephyr API)
 *   - flash_erase() is non-blocking — must busy-wait until complete
 *   - flush_cache() required after every write/erase
 *   - Addresses < BOOT_LOADER_END_PROTECT_ADDR (0x7000) are rejected by HW
 */

#define DT_DRV_COMPAT rafael_rt583_flash

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(flash_rt583, CONFIG_FLASH_LOG_LEVEL);

/* ── HAL wrappers (defined in flash_rt583_hal.c to avoid symbol clash) ── */
extern uint32_t rt583_hal_flash_read(uint32_t addr, uint32_t buf, uint32_t len);
extern uint32_t rt583_hal_flash_write(uint32_t addr, uint32_t buf, uint32_t len);
extern uint32_t rt583_hal_flash_erase_sector(uint32_t addr);
extern uint32_t rt583_hal_flash_erase_32k(uint32_t addr);
extern uint32_t rt583_hal_flash_erase_64k(uint32_t addr);
extern int      rt583_hal_flash_busy(void);
extern void     rt583_hal_flush_cache(void);
extern void     rt583_hal_flash_init(void);

/* ── Flash parameters ───────────────────────────────────────────────────── */
/*
 * The flash controller node's reg (0x40000000, 0x1000) is the MMIO register
 * space, NOT the flash memory size. Get the actual flash size from the
 * soc-nv-flash child node (flash0: flash@0 { reg = <0 0x200000>; }).
 */
#define RT583_FLASH_SIZE    DT_REG_SIZE(DT_NODELABEL(flash0))
#define RT583_SECTOR_SIZE   4096        /* smallest erase unit */
#define RT583_ERASE_VALUE   0xFF
#define LENGTH_32KB         (32 * 1024)
#define LENGTH_64KB         (64 * 1024)

/* STATUS_SUCCESS already defined in status.h (pulled in via mcu.h → soc.h) */

/* ── Driver data ────────────────────────────────────────────────────────── */
struct flash_rt583_data {
    struct k_mutex lock;
};

static struct flash_rt583_data flash_data;

/* ── Helpers ────────────────────────────────────────────────────────────── */

static int flash_wait_idle(void)
{
    uint32_t retries = 0;
    while (rt583_hal_flash_busy()) {
        if (++retries > 1000000) {
            LOG_ERR("flash busy timeout");
            return -ETIMEDOUT;
        }
        k_busy_wait(1);
    }
    return 0;
}

/* ── flash_driver_api: read ─────────────────────────────────────────────── */
static int rt583_flash_read(const struct device *dev, off_t offset,
                            void *data, size_t len)
{
    struct flash_rt583_data *ctx = dev->data;

    if (!len) {
        return 0;
    }
    if (offset < 0 || (offset + len) > RT583_FLASH_SIZE) {
        return -EINVAL;
    }

    k_mutex_lock(&ctx->lock, K_FOREVER);

    uint32_t rc = rt583_hal_flash_read((uint32_t)offset,
                                       (uint32_t)(uintptr_t)data,
                                       (uint32_t)len);
    k_mutex_unlock(&ctx->lock);

    return (rc == STATUS_SUCCESS) ? 0 : -EIO;
}

/* ── flash_driver_api: write ────────────────────────────────────────────── */
static int rt583_flash_write(const struct device *dev, off_t offset,
                             const void *data, size_t len)
{
    struct flash_rt583_data *ctx = dev->data;

    if (!len) {
        return 0;
    }
    if (offset < 0 || (offset + len) > RT583_FLASH_SIZE) {
        return -EINVAL;
    }

    k_mutex_lock(&ctx->lock, K_FOREVER);

    int ret = flash_wait_idle();
    if (ret) {
        goto out;
    }

    uint32_t rc = rt583_hal_flash_write((uint32_t)offset,
                                        (uint32_t)(uintptr_t)data,
                                        (uint32_t)len);
    if (rc != STATUS_SUCCESS) {
        ret = -EIO;
        goto out;
    }

    ret = flash_wait_idle();
    if (ret == 0) {
        rt583_hal_flush_cache();
    }

out:
    k_mutex_unlock(&ctx->lock);
    return ret;
}

/* ── flash_driver_api: erase ────────────────────────────────────────────── */
static int rt583_flash_erase_op(const struct device *dev, off_t offset,
                                size_t size)
{
    struct flash_rt583_data *ctx = dev->data;

    if (!size) {
        return 0;
    }
    if (offset < 0 || (offset + size) > RT583_FLASH_SIZE) {
        return -EINVAL;
    }
    if ((offset % RT583_SECTOR_SIZE) != 0 ||
        (size % RT583_SECTOR_SIZE) != 0) {
        return -EINVAL;
    }

    k_mutex_lock(&ctx->lock, K_FOREVER);

    int ret = 0;
    size_t remaining = size;
    uint32_t addr = (uint32_t)offset;

    while (remaining > 0) {
        uint32_t rc;
        size_t erase_len;

        if (remaining >= LENGTH_64KB && (addr % LENGTH_64KB) == 0) {
            erase_len = LENGTH_64KB;
            ret = flash_wait_idle();
            if (ret) break;
            rc = rt583_hal_flash_erase_64k(addr);
        } else if (remaining >= LENGTH_32KB && (addr % LENGTH_32KB) == 0) {
            erase_len = LENGTH_32KB;
            ret = flash_wait_idle();
            if (ret) break;
            rc = rt583_hal_flash_erase_32k(addr);
        } else {
            erase_len = RT583_SECTOR_SIZE;
            ret = flash_wait_idle();
            if (ret) break;
            rc = rt583_hal_flash_erase_sector(addr);
        }

        if (rc != STATUS_SUCCESS) {
            LOG_ERR("erase failed at 0x%x: %u", addr, rc);
            ret = -EIO;
            break;
        }

        ret = flash_wait_idle();
        if (ret) break;

        addr += erase_len;
        remaining -= erase_len;
    }

    if (ret == 0) {
        rt583_hal_flush_cache();
    }

    k_mutex_unlock(&ctx->lock);
    return ret;
}

/* ── flash_driver_api: get_parameters ───────────────────────────────────── */
static const struct flash_parameters rt583_flash_params = {
    .write_block_size = 1,
    .erase_value = RT583_ERASE_VALUE,
};

static const struct flash_parameters *
rt583_flash_get_parameters(const struct device *dev)
{
    ARG_UNUSED(dev);
    return &rt583_flash_params;
}

/* ── flash_driver_api: page_layout ──────────────────────────────────────── */
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
static const struct flash_pages_layout rt583_flash_layout = {
    .pages_count = RT583_FLASH_SIZE / RT583_SECTOR_SIZE,
    .pages_size  = RT583_SECTOR_SIZE,
};

static void rt583_flash_page_layout(const struct device *dev,
                                    const struct flash_pages_layout **layout,
                                    size_t *layout_size)
{
    ARG_UNUSED(dev);
    *layout = &rt583_flash_layout;
    *layout_size = 1;
}
#endif

/* ── Driver API struct ──────────────────────────────────────────────────── */
static const struct flash_driver_api rt583_flash_api = {
    .read = rt583_flash_read,
    .write = rt583_flash_write,
    .erase = rt583_flash_erase_op,
    .get_parameters = rt583_flash_get_parameters,
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
    .page_layout = rt583_flash_page_layout,
#endif
};

/* ── Init ───────────────────────────────────────────────────────────────── */
static int rt583_flash_init_fn(const struct device *dev)
{
    struct flash_rt583_data *ctx = dev->data;

    k_mutex_init(&ctx->lock);
    rt583_hal_flash_init();

    LOG_INF("RT583 flash driver ready (size=%u KB)", RT583_FLASH_SIZE / 1024);
    return 0;
}

/* ── Device instantiation ───────────────────────────────────────────────── */
DEVICE_DT_INST_DEFINE(0,
                      rt583_flash_init_fn,
                      NULL,
                      &flash_data,
                      NULL,
                      POST_KERNEL,
                      CONFIG_FLASH_INIT_PRIORITY,
                      &rt583_flash_api);
