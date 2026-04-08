/*
 * RT583 HCI Command Probe
 *
 * Uses hosal_rf_write_command() (proper HOSAL flow control) but runs
 * the send in a worker thread so that the main thread can time out
 * and recover the HOSAL semaphore if the controller doesn't respond.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#include "hosal_rf.h"
#include "hosal_lpm.h"
#include "rf_mcu.h"

/* Provided by hosal_rf.c — force-resets g_rf_cmd_sem after timeout */
extern void hosal_rf_cmd_sem_reset(void);

/* ── Shared state ──────────────────────────────────────────────────────── */

static K_SEM_DEFINE(evt_sem, 0, 1);
static volatile uint8_t  last_evt_code;
static volatile uint8_t  last_evt_len;
static volatile uint16_t last_evt_opcode;
static volatile uint8_t  last_evt_status;

/* FreeRTOS shims */
__weak uint32_t _crit_unlock(void) { return 0; }
__weak void     _crit_relock(uint32_t nest) { ARG_UNUSED(nest); }

/* ── HCI event callback (runs in hosal-rf thread) ─────────────────────── */

static int hci_evt_cb(void *p_arg)
{
	uint8_t *raw = (uint8_t *)p_arg;
	last_evt_code = raw[1];
	last_evt_len  = raw[2];

	if (last_evt_code == 0x0E && last_evt_len >= 4) {
		last_evt_opcode = (uint16_t)raw[5] << 8 | raw[4];
		last_evt_status = raw[6];
	} else if (last_evt_code == 0x0F && last_evt_len >= 4) {
		last_evt_status = raw[3];
		last_evt_opcode = (uint16_t)raw[6] << 8 | raw[5];
	} else {
		last_evt_opcode = 0;
		last_evt_status = 0xFF;
	}

	/* Print raw RX bytes */
	uint8_t total = (raw[2] + 3 > 32) ? 32 : raw[2] + 3;
	printk("  RX[%u]: ", total);
	for (int i = 0; i < total; i++) {
		printk("%02x ", raw[i]);
	}
	printk("\n");

	k_sem_give(&evt_sem);
	return 1; /* release g_rf_cmd_sem */
}

static int hci_acl_cb(void *p_arg) { return 0; }

/* ── Worker thread for hosal_rf_write_command (blocks on sem) ──────────── */

#define WORKER_STACK 1024
static K_THREAD_STACK_DEFINE(worker_stack, WORKER_STACK);
static struct k_thread worker_thread;
static K_SEM_DEFINE(worker_start, 0, 1);
static K_SEM_DEFINE(worker_done, 0, 1);
static const uint8_t *worker_pkt;
static uint8_t worker_pkt_len;
static volatile int worker_rc;

static void worker_entry(void *p1, void *p2, void *p3)
{
	while (1) {
		k_sem_take(&worker_start, K_FOREVER);
		worker_rc = hosal_rf_write_command((uint8_t *)worker_pkt,
						   worker_pkt_len);
		k_sem_give(&worker_done);
	}
}

/* ── Send one HCI command with timeout recovery ────────────────────────── */

static void send_cmd(const uint8_t *pkt, uint8_t len,
		     uint16_t opcode_display, const char *name)
{
	int rc;

	k_sem_reset(&evt_sem);
	last_evt_code = 0;
	last_evt_opcode = 0;
	last_evt_status = 0xFF;

	/* Print raw TX bytes */
	printk("  TX[%u]: ", len);
	for (int i = 0; i < len; i++) {
		printk("%02x ", pkt[i]);
	}
	printk("\n");

	/* Kick worker to call hosal_rf_write_command */
	worker_pkt = pkt;
	worker_pkt_len = len;
	k_sem_give(&worker_start);

	/* Wait for worker to finish sending (max 2s — covers retry loop) */
	rc = k_sem_take(&worker_done, K_MSEC(2000));
	if (rc == -EAGAIN) {
		printk("  %-40s 0x%04x  => SEND HANG (sem stuck)\n",
		       name, opcode_display);
		hosal_rf_cmd_sem_reset();
		/* worker_done will be given when worker eventually unblocks */
		k_sem_take(&worker_done, K_MSEC(500));
		return;
	}

	if (worker_rc != 0) {
		printk("  %-40s 0x%04x  => SEND FAIL (rc=%d) pwr=0x%02x\n",
		       name, opcode_display, worker_rc,
		       RfMcu_PowerStateCheck());
		return;
	}

	/* Command sent — wait for event response */
	rc = k_sem_take(&evt_sem, K_MSEC(3000));
	if (rc == -EAGAIN) {
		printk("  %-40s 0x%04x  => TIMEOUT (no response)\n",
		       name, opcode_display);
		/* The sem is stuck inside HOSAL — reset it */
		hosal_rf_cmd_sem_reset();
		return;
	}

	printk("  %-40s 0x%04x  => evt=0x%02x st=0x%02x op=0x%04x len=%u\n",
	       name, opcode_display,
	       last_evt_code, last_evt_status, last_evt_opcode, last_evt_len);
}

/* ── Convenience: zero-parameter HCI command ───────────────────────────── */

static void test_cmd(uint8_t ogf, uint16_t ocf, const char *name)
{
	uint16_t raw_opcode = (ocf & 0x3FF) | ((uint16_t)(ogf & 0x3F) << 10);
	uint8_t pkt[4] = {
		0x01,
		raw_opcode & 0xFF,
		(raw_opcode >> 8) & 0xFF,
		0x00
	};
	send_cmd(pkt, 4, raw_opcode, name);
}

/* ── Main ──────────────────────────────────────────────────────────────── */

int main(void)
{
	printk("\n");
	printk("============================================\n");
	printk("  RT583 HCI Command Probe\n");
	printk("============================================\n\n");

	/* Start worker thread (lower priority than main) */
	k_thread_create(&worker_thread, worker_stack,
			K_THREAD_STACK_SIZEOF(worker_stack),
			worker_entry, NULL, NULL, NULL,
			5, 0, K_NO_WAIT);

	/* Register callbacks */
	hosal_rf_callback_set(HOSAL_RF_BLE_EVENT_CALLBACK,
			      (hosal_rf_callback_t)hci_evt_cb, NULL);
	hosal_rf_callback_set(HOSAL_RF_BLE_RX_CALLBACK,
			      (hosal_rf_callback_t)hci_acl_cb, NULL);

	/* LPM + RF init */
	hosal_lpm_init();
	hosal_lpm_ioctrl(HOSAL_LPM_SET_POWER_LEVEL, HOSAL_LOW_POWER_LEVEL_SLEEP0);
	hosal_rf_init(HOSAL_RF_MODE_BLE_CONTROLLER);
	k_sleep(K_MSEC(200));

	printk("RF init done: pwr=0x%02x\n\n", RfMcu_PowerStateCheck());

	/* ── Phase 1: Vendor init (MUST be first) ──────────────────────────── */
	printk("[Phase 1] Vendor Commands\n");
	{
		uint8_t cmd[] = {
			0x01, 0x01, 0xFC, 0x09,
			0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
			0x0C, 0x64, 0x08,
		};
		send_cmd(cmd, sizeof(cmd), 0xFC01, "Vendor: Set Controller Info");
	}
	printk("\n");

	/* ── Phase 2: Rafael SDK init sequence ─────────────────────────────── */
	printk("[Phase 2] Rafael SDK Init Sequence\n");
	{
		uint8_t cmd[] = {
			0x01, 0x01, 0x20, 0x08,
			0x7F, 0x0C, 0x60, 0x00, 0x00, 0x01, 0x00, 0x00
		};
		send_cmd(cmd, sizeof(cmd), 0x2001, "LE Set Event Mask");
	}
	test_cmd(0x04, 0x0001, "Read Local Version Info");
	test_cmd(0x08, 0x0002, "LE Read Buffer Size");
	printk("\n");

	/* ── Phase 3: Standard HCI Info commands ───────────────────────────── */
	printk("[Phase 3] HCI Informational Parameters\n");
	test_cmd(0x04, 0x0002, "Read Local Supported Commands");
	test_cmd(0x04, 0x0003, "Read Local Supported Features");
	test_cmd(0x04, 0x0009, "Read BD ADDR");
	printk("\n");

	/* ── Phase 4: LE Controller commands ───────────────────────────────── */
	printk("[Phase 4] LE Controller Commands\n");
	test_cmd(0x08, 0x0003, "LE Read Local Supported Features");
	test_cmd(0x08, 0x000F, "LE Read Filter Accept List Size");
	test_cmd(0x08, 0x001C, "LE Read Supported States");
	test_cmd(0x08, 0x0018, "LE Rand");
	printk("\n");

	/* ── Phase 5: Controller & Baseband ────────────────────────────────── */
	printk("[Phase 5] Controller & Baseband\n");
	test_cmd(0x03, 0x0001, "Set Event Mask");
	test_cmd(0x03, 0x0003, "HCI Reset");
	printk("\n");

	printk("============================================\n");
	printk("  Test Complete\n");
	printk("============================================\n");

	while (1) {
		k_sleep(K_FOREVER);
	}
	return 0;
}
