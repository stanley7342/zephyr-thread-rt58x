/*
 * ota_app.c — OTA integration for RT583 Thread example
 *
 * Usage (OpenThread CLI):
 *   ota check  fd00::1              ← check + download from server fd00::1
 *   ota apply                       ← decompress staging → slot1 + reboot
 *   ota status                      ← show current OTA state
 *   ota confirm                     ← mark running image as healthy
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/reboot.h>
#include <string.h>

#include <openthread/cli.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>

#include "ota.h"

/* ── Apply thread (decompression is CPU-heavy, run outside OT task) ───── */
#define APPLY_STACK_SIZE   CONFIG_OTA_APPLY_THREAD_STACK_SIZE
K_THREAD_STACK_DEFINE(ota_apply_stack, APPLY_STACK_SIZE);
static struct k_thread ota_apply_thread;
static ota_meta_t      pending_meta;
static bool            apply_pending;

static void apply_thread_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

    while (1) {
        k_sleep(K_SECONDS(1));
        if (!apply_pending) continue;

        apply_pending = false;
        printk("[OTA-App] starting decompress + apply\n");

        int rc = ota_flash_apply(&pending_meta);
        if (rc == OTA_OK) {
            printk("[OTA-App] apply OK — rebooting in 3 s\n");
            k_sleep(K_SECONDS(3));
            sys_reboot(SYS_REBOOT_COLD);
        } else {
            printk("[OTA-App] apply failed: %d\n", rc);
        }
    }
}

/* ── Download callback (runs in OT task context) ─────────────────────── */
static void on_download_done(int result, const ota_meta_t *meta)
{
    if (result != OTA_OK) {
        printk("[OTA-App] download failed: %d\n", result);
        return;
    }

    printk("[OTA-App] download OK — queuing apply\n");
    pending_meta  = *meta;
    apply_pending = true;
}

/* ── CLI command: ota <check|apply|status|confirm> [addr] ────────────── */
static otError cmd_ota(void *ctx, uint8_t argc, char *argv[])
{
    ARG_UNUSED(ctx);

    if (argc < 2) {
        otCliOutputFormat("usage: ota <check|apply|status|confirm> [server_addr]\r\n");
        return OT_ERROR_NONE;
    }

    if (strcmp(argv[1], "check") == 0) {
        if (argc < 3) {
            otCliOutputFormat("usage: ota check <server_ipv6_addr>\r\n");
            return OT_ERROR_NONE;
        }

        otIp6Address addr;
        if (otIp6AddressFromString(argv[2], &addr) != OT_ERROR_NONE) {
            otCliOutputFormat("invalid address: %s\r\n", argv[2]);
            return OT_ERROR_NONE;
        }

        int rc = ota_check_and_download(&addr, CONFIG_OTA_COAP_PORT,
                                        on_download_done);
        if (rc == OTA_ERR_BUSY)
            otCliOutputFormat("OTA already in progress\r\n");
        else
            otCliOutputFormat("OTA check started → %s\r\n", argv[2]);

    } else if (strcmp(argv[1], "apply") == 0) {
        if (ota_state_get() != OTA_STATE_DOWNLOADING) {
            otCliOutputFormat("no staged image (state=%d)\r\n",
                              ota_state_get());
            return OT_ERROR_NONE;
        }
        apply_pending = true;
        otCliOutputFormat("apply queued\r\n");

    } else if (strcmp(argv[1], "status") == 0) {
        static const char *state_str[] = {
            "IDLE", "DOWNLOADING", "DECOMPRESSING", "READY", "CONFIRMED"
        };
        ota_state_t st = ota_state_get();
        otCliOutputFormat("state: %s (%d)\r\n",
                          st < 5 ? state_str[st] : "?", st);
        otCliOutputFormat("staging: %u bytes written\r\n",
                          ota_staging_written());

    } else if (strcmp(argv[1], "confirm") == 0) {
        ota_confirm();
        otCliOutputFormat("confirmed\r\n");

    } else {
        otCliOutputFormat("unknown subcommand: %s\r\n", argv[1]);
    }

    return OT_ERROR_NONE;
}

static const otCliCommand kOtaCommand = { "ota", cmd_ota };

/* ── Init: call from otrInitUser() ──────────────────────────────────── */
void ota_app_init(otInstance *instance)
{
    ota_init(instance);

    k_thread_create(&ota_apply_thread, ota_apply_stack,
                    K_THREAD_STACK_SIZEOF(ota_apply_stack),
                    apply_thread_fn, NULL, NULL, NULL,
                    7, 0, K_NO_WAIT);
    k_thread_name_set(&ota_apply_thread, "ota_apply");

    otCliSetUserCommands(&kOtaCommand, 1, instance);
    printk("[OTA-App] ready\n");
}
