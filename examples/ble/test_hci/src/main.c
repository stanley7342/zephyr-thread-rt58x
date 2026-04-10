/*
 * RT583 HCI 0x2005 Isolation Test
 *
 * Replicates the exact Matter app failure scenario:
 *   1. hosal_rf_init(MULTI_PROTOCOL)
 *   2. Send OT RUCI commands (simulate ot_radioInit)
 *   3. Send BLE vendor setup (0xFC01)
 *   4. Send BLE init sequence (0x1003, 0x1001, 0x1002, ...)
 *   5. Send LE Set Random Address (0x2005) — this was timing out
 *
 * Verifies the wake-up doorbell fix in hosal_rf_write_command.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#include "hosal_rf.h"
#include "hosal_lpm.h"
#include "rf_mcu.h"
#include "ruci.h"

/* hosal_rf.c — reset g_rf_cmd_sem after timeout */
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

/* ── HCI event callback ────────────────────────────────────────────────── */

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

	uint8_t total = raw[2] + 3;
	if (total > 32) { total = 32; }
	printk("  RX[%u]:", total);
	for (int i = 0; i < total; i++) { printk(" %02x", raw[i]); }
	printk("\n");

	k_sem_give(&evt_sem);
	return 1; /* release g_rf_cmd_sem */
}

/* RUCI event callback (OT side) */
static int ruci_evt_cb(void *p_arg)
{
	uint8_t *raw = (uint8_t *)p_arg;
	printk("  [RUCI-EVT] type=0x%02x\n", raw[0]);
	k_sem_give(&evt_sem);
	return 1;
}

static int hci_acl_cb(void *p_arg) { return 0; }

/* ── Worker thread ─────────────────────────────────────────────────────── */

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

/* ── Generic send with 3-second event timeout ──────────────────────────── */

static bool send_cmd(const uint8_t *pkt, uint8_t len,
		     uint16_t opcode_display, const char *name)
{
	int rc;

	k_sem_reset(&evt_sem);
	last_evt_code = 0;
	last_evt_opcode = 0;
	last_evt_status = 0xFF;

	printk("  TX[%u]:", len);
	for (int i = 0; i < len; i++) { printk(" %02x", pkt[i]); }
	printk("\n");

	worker_pkt = pkt;
	worker_pkt_len = len;
	k_sem_give(&worker_start);

	rc = k_sem_take(&worker_done, K_MSEC(2000));
	if (rc == -EAGAIN) {
		printk("  %-38s 0x%04x => SEND HANG\n", name, opcode_display);
		hosal_rf_cmd_sem_reset();
		k_sem_take(&worker_done, K_MSEC(500));
		return false;
	}
	if (worker_rc != 0) {
		printk("  %-38s 0x%04x => SEND FAIL rc=%d\n",
		       name, opcode_display, worker_rc);
		return false;
	}

	rc = k_sem_take(&evt_sem, K_MSEC(3000));
	if (rc == -EAGAIN) {
		printk("  %-38s 0x%04x => *** TIMEOUT (no response) ***\n",
		       name, opcode_display);
		hosal_rf_cmd_sem_reset();
		return false;
	}

	bool ok = (last_evt_status == 0x00);
	printk("  %-38s 0x%04x => %s (st=0x%02x)\n",
	       name, opcode_display, ok ? "OK " : "ERR", last_evt_status);
	return ok;
}

static bool test_cmd0(uint8_t ogf, uint16_t ocf, const char *name)
{
	uint16_t op = (ocf & 0x3FF) | ((uint16_t)(ogf & 0x3F) << 10);
	uint8_t pkt[4] = { 0x01, op & 0xFF, op >> 8, 0x00 };
	return send_cmd(pkt, 4, op, name);
}

/* ── Main ──────────────────────────────────────────────────────────────── */

int main(void)
{
	printk("\n");
	printk("============================================\n");
	printk("  RT583 HCI 0x2005 Isolation Test\n");
	printk("============================================\n\n");

	k_thread_create(&worker_thread, worker_stack,
			K_THREAD_STACK_SIZEOF(worker_stack),
			worker_entry, NULL, NULL, NULL,
			5, 0, K_NO_WAIT);

	/* ── Step 1: Init in MULTI_PROTOCOL mode (same as Matter app) ── */
	printk("[Step 1] hosal_rf_init(MULTI_PROTOCOL)\n");
	hosal_lpm_init();
	hosal_lpm_ioctrl(HOSAL_LPM_SET_POWER_LEVEL, HOSAL_LOW_POWER_LEVEL_SLEEP0);
	hosal_rf_init(HOSAL_RF_MODE_MULTI_PROTOCOL);
	k_sleep(K_MSEC(50));
	printk("  RF init done: pwr=0x%02x\n\n", RfMcu_PowerStateCheck());

	/* ── Step 2: Simulate ot_radioInit RUCI commands ─────────────── */
	printk("[Step 2] Simulate OT RUCI commands (ot_radioInit)\n");
	/* Register RUCI event callback like lmac15p4 does */
	hosal_rf_callback_set(HOSAL_RF_PCI_EVENT_CALLBACK,
			      (hosal_rf_callback_t)ruci_evt_cb, NULL);

	/* Initiate 15.4 (matches ot_radioInit RUCI sequence) */
	{
		uint8_t cmd[RUCI_LEN_INITIATE_ZIGBEE];
		SET_RUCI_PARA_INITIATE_ZIGBEE(cmd);
		RUCI_ENDIAN_CONVERT(cmd, RUCI_INITIATE_ZIGBEE);
		send_cmd(cmd, RUCI_LEN_INITIATE_ZIGBEE, 0x1300, "RUCI Initiate Zigbee");
	}
	{
		uint8_t cmd[RUCI_LEN_SET_RF_FREQUENCY];
		SET_RUCI_PARA_SET_RF_FREQUENCY(cmd, 2405000000UL); /* ch11 */
		RUCI_ENDIAN_CONVERT(cmd, RUCI_SET_RF_FREQUENCY);
		send_cmd(cmd, RUCI_LEN_SET_RF_FREQUENCY, 0x1302, "RUCI Set RF Frequency");
	}
	/* RUCI_INITIATE_BLE: tells multi-protocol scheduler BLE is active.
	 * Without this, BLE HCI queue is only polled on sleep→normal transitions,
	 * causing commands like 0x2005 to fail when RF MCU is active on 802.15.4. */
	{
		uint8_t cmd[RUCI_LEN_INITIATE_BLE];
		SET_RUCI_PARA_INITIATE_BLE(cmd);
		RUCI_ENDIAN_CONVERT(cmd, RUCI_INITIATE_BLE);
		send_cmd(cmd, RUCI_LEN_INITIATE_BLE, 0x1201, "RUCI Initiate BLE");
	}
	printk("\n");

	/* ── Step 3: Register BLE HCI callbacks ───────────────────────── */
	hosal_rf_callback_set(HOSAL_RF_BLE_EVENT_CALLBACK,
			      (hosal_rf_callback_t)hci_evt_cb, NULL);
	hosal_rf_callback_set(HOSAL_RF_BLE_RX_CALLBACK,
			      (hosal_rf_callback_t)hci_acl_cb, NULL);

	/* ── Step 4: Vendor BLE setup (0xFC01) ────────────────────────── */
	printk("[Step 3] BLE Vendor Init\n");
	{
		uint8_t cmd[] = {
			0x01, 0x01, 0xFC, 0x09,
			0x58, 0x82, 0xBE, 0xEF, 0xCA, 0xFE,
			0x0C, 0x64, 0x08,
		};
		send_cmd(cmd, sizeof(cmd), 0xFC01, "Vendor: Set Controller Info");
	}
	printk("\n");

	/* ── Step 5: Full Zephyr BLE host init sequence ───────────────── */
	printk("[Step 4] BLE Init Sequence (matches Zephyr bt_enable order)\n");
	test_cmd0(0x04, 0x0003, "Read Local Supported Features");
	test_cmd0(0x04, 0x0001, "Read Local Version Info");
	test_cmd0(0x04, 0x0002, "Read Local Supported Commands");
	test_cmd0(0x08, 0x0003, "LE Read Local Supported Features");
	test_cmd0(0x08, 0x0002, "LE Read Buffer Size");
	test_cmd0(0x08, 0x001C, "LE Read Supported States");
	test_cmd0(0x08, 0x002F, "LE Read Transmit Power");
	{
		uint8_t cmd[] = { 0x01, 0x24, 0x20, 0x04, 0xFB, 0x00, 0x48, 0x08 };
		send_cmd(cmd, sizeof(cmd), 0x2024, "LE Write Suggested Default Data Len");
	}
	{
		uint8_t cmd[] = {
			0x01, 0x01, 0x20, 0x08,
			0x4F, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		};
		send_cmd(cmd, sizeof(cmd), 0x2001, "LE Set Event Mask");
	}
	{
		uint8_t cmd[] = {
			0x01, 0x01, 0x0C, 0x08,
			0x10, 0x88, 0x00, 0x02, 0x00, 0x00, 0x00, 0x20
		};
		send_cmd(cmd, sizeof(cmd), 0x0C01, "Set Event Mask");
	}
	printk("\n");

	/* ── Step 6: Send LE Set Random Address (the failing command) ──── */
	printk("[Step 5] LE Set Random Address (0x2005) — THE TEST\n");
	printk("  pwr=0x%02x before send\n", RfMcu_PowerStateCheck());
	{
		/* Random static address: MSB must have bits[7:6]=11 */
		uint8_t cmd[] = {
			0x01, 0x05, 0x20, 0x06,
			0x9D, 0x6C, 0xF2, 0x12, 0x8D, 0xF1
		};
		bool ok = send_cmd(cmd, sizeof(cmd), 0x2005, "LE Set Random Address");
		printk("\n");
		if (ok) {
			printk("  *** 0x2005 PASSED — wake-up fix works! ***\n");
		} else {
			printk("  *** 0x2005 FAILED — fix did not help ***\n");
		}
	}

	/* ── Step 7: Retry after explicit wake (sanity check) ─────────── */
	printk("\n[Step 6] Retry 0x2005 after explicit HostWakeUpMcu\n");
	{
		RfMcu_HostWakeUpMcu();
		k_sleep(K_MSEC(10));
		uint8_t cmd[] = {
			0x01, 0x05, 0x20, 0x06,
			0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
		};
		send_cmd(cmd, sizeof(cmd), 0x2005, "LE Set Random Address (retry)");
	}

	printk("\n");
	printk("============================================\n");
	printk("  Test Complete\n");
	printk("============================================\n");

	while (1) { k_sleep(K_FOREVER); }
	return 0;
}
