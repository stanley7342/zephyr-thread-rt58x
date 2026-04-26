/*
 * Zephyr-on-rt58x PAL implementation for psa-arch-tests.
 *
 * Adapted from psa-arch-tests/api-tests/platform/targets/tgt_dev_apis_stdc/
 * nspe/pal_driver_intf.c. Replaces stdio/printf with Zephyr printk and
 * stubs out watchdog/system reset (this implementation doesn't run tests
 * that require process or system restart).
 */

#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include <zephyr/sys/printk.h>
#include "pal_common.h"

/* Regression test status reporting buffer (vendor convention). */
uint8_t test_status_buffer[256] = {0};

/* In-RAM "NVMEM" — we don't run tests that need real persistence. */
#define NVMEM_BASE 0
#define NVMEM_SIZE (1024)
static uint8_t g_nvmem[NVMEM_SIZE];

static int nvmem_check_bounds(addr_t base, uint32_t offset, int size)
{
    if (base != NVMEM_BASE) return PAL_STATUS_ERROR;
    if (offset > NVMEM_SIZE) return PAL_STATUS_ERROR;
    if (size < 0) return PAL_STATUS_ERROR;
    if (offset > (uint32_t)(INT_MAX - size)) return PAL_STATUS_ERROR;
    if (offset + size > NVMEM_SIZE) return PAL_STATUS_ERROR;
    return PAL_STATUS_SUCCESS;
}

int pal_nvm_read(uint32_t offset, void *buffer, size_t size)
{
    if (nvmem_check_bounds((addr_t)PLATFORM_NVM_BASE, offset, size) != PAL_STATUS_SUCCESS) {
        return PAL_STATUS_ERROR;
    }
    memcpy(buffer, g_nvmem + offset, size);
    return PAL_STATUS_SUCCESS;
}

int pal_nvm_write(uint32_t offset, void *buffer, size_t size)
{
    if (nvmem_check_bounds((addr_t)PLATFORM_NVM_BASE, offset, size) != PAL_STATUS_SUCCESS) {
        return PAL_STATUS_ERROR;
    }
    memcpy(g_nvmem + offset, buffer, size);
    return PAL_STATUS_SUCCESS;
}

int pal_uart_init_ns(void)
{
    /* Zephyr console driver is already up before main() — no-op. */
    return PAL_STATUS_SUCCESS;
}

/* val_print() ultimately calls pal_print_ns(fmt, data). The vendor signature
 * passes one int32_t for the single %-substitution. printk handles formats
 * directly; we route through it. */
int pal_print_ns(const char *str, int32_t data)
{
    printk(str, data);
    return PAL_STATUS_SUCCESS;
}

int pal_wd_timer_init_ns(uint32_t time_us, uint32_t timer_tick_us)
{
    (void)time_us;
    (void)timer_tick_us;
    return PAL_STATUS_SUCCESS;
}

int pal_watchdog_enable(void)  { return PAL_STATUS_SUCCESS; }
int pal_watchdog_disable(void) { return PAL_STATUS_SUCCESS; }

void pal_terminate_simulation(void) { /* idle loop in main keeps board alive */ }

int pal_system_reset(void) { return PAL_STATUS_UNSUPPORTED_FUNC; }

/* val_interfaces.c builds a function-pointer table that always references
 * val_attestation_function and val_storage_function. The bodies of those
 * source files are guarded by #ifdef INITIAL_ATTESTATION / #ifdef STORAGE
 * — if we only compile the CRYPTO suite, the symbols are unresolved.
 * Provide stubs that return UNSUPPORTED so the link succeeds; tests that
 * actually call them aren't selected when only -DCRYPTO is set. */
int32_t val_attestation_function(int type, ...)
{
    (void)type;
    return PAL_STATUS_UNSUPPORTED_FUNC;
}
