/*
 * Zephyr flash driver for Rafael RT584
 *
 * Thin wrapper over the RT584 flashctl API. The API surface and semantics
 * match RT583 (same vendor lineage) so the structure mirrors flash_rt583.c;
 * the only differences are the DT_DRV_COMPAT, HAL symbol prefix, and the
 * base address used by the DTS (flash lives at 0x10000000 on RT584 but the
 * non-TrustZone flashctl still treats the controller's own address argument
 * as a 0-based offset — see FLASH_SECURE_MODE_BASE_ADDR in flashctl.h).
 *
 * Hardware constraints (same as RT583):
 *   - Write granularity: 256-byte page (sub-page handled by flash_write_n_bytes)
 *   - Erase granularity: 4 KB sector (smallest Zephyr erase unit)
 *   - flash_erase() is non-blocking — must busy-wait until complete
 *   - flush_cache() required after every write/erase
 *   - Offsets < BOOT_LOADER_END_PROTECT_ADDR (0xF000 on rt584) are rejected
 */

#define DT_DRV_COMPAT rafael_rt584_flash

#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(flash_rt584, CONFIG_FLASH_LOG_LEVEL);

extern uint32_t rt584_hal_flash_read(uint32_t addr, uint32_t buf, uint32_t len);
extern uint32_t rt584_hal_flash_write(uint32_t addr, uint32_t buf, uint32_t len);
extern uint32_t rt584_hal_flash_erase_sector(uint32_t addr);
extern uint32_t rt584_hal_flash_erase_32k(uint32_t addr);
extern uint32_t rt584_hal_flash_erase_64k(uint32_t addr);
extern int      rt584_hal_flash_busy(void);
extern void     rt584_hal_flush_cache(void);
extern void     rt584_hal_flash_init(void);

#define RT584_FLASH_SIZE    DT_REG_SIZE(DT_NODELABEL(flash0))
#define RT584_SECTOR_SIZE   4096
#define RT584_ERASE_VALUE   0xFF
#define LENGTH_32KB         (32 * 1024)
#define LENGTH_64KB         (64 * 1024)

struct flash_rt584_data {
    struct k_mutex lock;
};

static struct flash_rt584_data flash_data;

static int flash_wait_idle(void)
{
    uint32_t retries = 0;
    while (rt584_hal_flash_busy()) {
        if (++retries > 1000000) {
            LOG_ERR("flash busy timeout");
            return -ETIMEDOUT;
        }
        k_busy_wait(1);
    }
    return 0;
}

static int rt584_flash_read(const struct device *dev, off_t offset,
                            void *data, size_t len)
{
    struct flash_rt584_data *ctx = dev->data;

    if (!len) return 0;
    if (offset < 0 || (offset + len) > RT584_FLASH_SIZE) return -EINVAL;

    k_mutex_lock(&ctx->lock, K_FOREVER);
    uint32_t rc = rt584_hal_flash_read((uint32_t)offset,
                                       (uint32_t)(uintptr_t)data,
                                       (uint32_t)len);
    k_mutex_unlock(&ctx->lock);
    return (rc == STATUS_SUCCESS) ? 0 : -EIO;
}

static int rt584_flash_write(const struct device *dev, off_t offset,
                             const void *data, size_t len)
{
    struct flash_rt584_data *ctx = dev->data;

    if (!len) return 0;
    if (offset < 0 || (offset + len) > RT584_FLASH_SIZE) return -EINVAL;

    k_mutex_lock(&ctx->lock, K_FOREVER);
    int ret = flash_wait_idle();
    if (ret) goto out;

    uint32_t rc = rt584_hal_flash_write((uint32_t)offset,
                                        (uint32_t)(uintptr_t)data,
                                        (uint32_t)len);
    if (rc != STATUS_SUCCESS) { ret = -EIO; goto out; }

    ret = flash_wait_idle();
    if (ret == 0) rt584_hal_flush_cache();

out:
    k_mutex_unlock(&ctx->lock);
    return ret;
}

static int rt584_flash_erase_op(const struct device *dev, off_t offset,
                                size_t size)
{
    struct flash_rt584_data *ctx = dev->data;

    if (!size) return 0;
    if (offset < 0 || (offset + size) > RT584_FLASH_SIZE) return -EINVAL;
    if ((offset % RT584_SECTOR_SIZE) != 0 || (size % RT584_SECTOR_SIZE) != 0)
        return -EINVAL;

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
            rc = rt584_hal_flash_erase_64k(addr);
        } else if (remaining >= LENGTH_32KB && (addr % LENGTH_32KB) == 0) {
            erase_len = LENGTH_32KB;
            ret = flash_wait_idle();
            if (ret) break;
            rc = rt584_hal_flash_erase_32k(addr);
        } else {
            erase_len = RT584_SECTOR_SIZE;
            ret = flash_wait_idle();
            if (ret) break;
            rc = rt584_hal_flash_erase_sector(addr);
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

    if (ret == 0) rt584_hal_flush_cache();

    k_mutex_unlock(&ctx->lock);
    return ret;
}

static const struct flash_parameters rt584_flash_params = {
    .write_block_size = 1,
    .erase_value = RT584_ERASE_VALUE,
};

static const struct flash_parameters *
rt584_flash_get_parameters(const struct device *dev)
{
    ARG_UNUSED(dev);
    return &rt584_flash_params;
}

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
static const struct flash_pages_layout rt584_flash_layout = {
    .pages_count = RT584_FLASH_SIZE / RT584_SECTOR_SIZE,
    .pages_size  = RT584_SECTOR_SIZE,
};

static void rt584_flash_page_layout(const struct device *dev,
                                    const struct flash_pages_layout **layout,
                                    size_t *layout_size)
{
    ARG_UNUSED(dev);
    *layout = &rt584_flash_layout;
    *layout_size = 1;
}
#endif

static const struct flash_driver_api rt584_flash_api = {
    .read = rt584_flash_read,
    .write = rt584_flash_write,
    .erase = rt584_flash_erase_op,
    .get_parameters = rt584_flash_get_parameters,
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
    .page_layout = rt584_flash_page_layout,
#endif
};

static int rt584_flash_init_fn(const struct device *dev)
{
    struct flash_rt584_data *ctx = dev->data;

    k_mutex_init(&ctx->lock);
    rt584_hal_flash_init();

    LOG_INF("RT584 flash driver ready (size=%u KB)", RT584_FLASH_SIZE / 1024);
    return 0;
}

DEVICE_DT_INST_DEFINE(0,
                      rt584_flash_init_fn,
                      NULL,
                      &flash_data,
                      NULL,
                      POST_KERNEL,
                      CONFIG_FLASH_INIT_PRIORITY,
                      &rt584_flash_api);
