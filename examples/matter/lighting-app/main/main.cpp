/*
 * RT583 Matter Lighting App — Entry point.
 *
 * AppTask::StartApp() drives psa_crypto_init() and the full CHIP stack init,
 * which needs 10-12 KB of stack (PSA mbedTLS frames alone are 4-8 KB).
 * Running it on main (CONFIG_MAIN_STACK_SIZE) causes a stack overflow.
 * We launch it on a dedicated app_thread with APP_STACK_SIZE instead, and
 * keep main() lightweight so its stack can stay small.
 */

#include "AppTask.h"

#include <system/SystemError.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_REGISTER(app, CONFIG_CHIP_APP_LOG_LEVEL);

#define APP_STACK_SIZE  4096

K_THREAD_STACK_DEFINE(app_stack, APP_STACK_SIZE);  /* non-static for extern in AppTask.cpp */
static struct k_thread app_thread;

static void app_thread_fn(void *, void *, void *)
{
    /* Stack depth diagnostic: print SP offset from stack base at thread entry.
     * With a healthy 16 KB stack this should show < 200 bytes used here. */
    {
        uintptr_t sp;
        __asm__ volatile("mov %0, sp" : "=r"(sp));
        uintptr_t base = (uintptr_t)app_stack;
        uintptr_t top  = base + APP_STACK_SIZE;
        printk("[STACK] entry SP=0x%08x base=0x%08x used=%u free=%u\n",
               (unsigned)sp, (unsigned)base,
               (unsigned)(top - sp), (unsigned)(sp - base));
    }
    CHIP_ERROR err = AppTask::Instance().StartApp();
    if (err != CHIP_NO_ERROR) {
        LOG_ERR("Exited with code %" CHIP_ERROR_FORMAT, err.Format());
    }
}

int main()
{
    printk("[MAIN] start\n");

    k_thread_create(&app_thread, app_stack, APP_STACK_SIZE,
                    app_thread_fn, NULL, NULL, NULL,
                    K_PRIO_PREEMPT(1), 0, K_NO_WAIT);
    k_thread_name_set(&app_thread, "app");
    return 0;
}
