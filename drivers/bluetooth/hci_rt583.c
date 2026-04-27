/*
 * HCI driver for Rafael RT58x integrated BLE controller.
 *
 * The RT58x COMM_SUBSYSTEM co-processor runs a BLE controller firmware
 * that exposes a standard HCI interface over shared-memory queues.
 * This driver bridges HOSAL RF callbacks to Zephyr's BLE host stack.
 *
 * The controller requires a vendor-specific "Set Controller Info" command
 * (OGF=0x3F, OCF=0x01) before it will respond to standard HCI commands.
 * This is sent from the setup() callback before the host starts its
 * normal init sequence.
 *
 * Data flow:
 *   TX: bt_send() → hosal_rf_write_command / hosal_rf_write_tx_data
 *   RX: hosal_rf callbacks → k_fifo → rx_thread → bt_recv()
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/drivers/bluetooth.h>
#include <zephyr/net_buf.h>

#include "hosal_rf.h"
#include "hosal_lpm.h"
#include "rf_mcu.h"
#include "ruci.h"
#include "flashctl.h"



/* BLE HCI H:4 packet type indicators */
#define H4_CMD  0x01
#define H4_ACL  0x02
#define H4_EVT  0x04

#define RT583_COMM_SUBSYSTEM_IRQN 20

/* Rafael vendor HCI command: Set Controller Info (OGF=0x3F, OCF=0x01) */
#define RT58X_VS_SET_CTRL_INFO_PLEN    9
#define RT58X_BLE_VERSION              0x0C    /* Bluetooth 5.3 */

/* RX thread */
#define RX_THREAD_STACK_SIZE 1536
#define RX_THREAD_PRIORITY   8

static K_THREAD_STACK_DEFINE(rx_thread_stack, RX_THREAD_STACK_SIZE);
static struct k_thread rx_thread_data;
static K_FIFO_DEFINE(rx_fifo);

struct hci_rt58x_data {
	bt_hci_recv_t recv;
	bool setup_in_progress;
};

static struct hci_rt58x_data hci_data;

/* Set to true by external code (e.g. AppTask) when hosal_rf_init() has
 * already been called with HOSAL_RF_MODE_MULTI_PROTOCOL before bt_enable().
 * In that case hci_rt58x_open() must skip its own hosal_rf_init() call to
 * avoid re-loading the RF MCU firmware and losing OT state. */
bool hci_rt58x_rf_already_init = false;

/* FreeRTOS critical-section shim */
__weak uint32_t _crit_unlock(void) { return 0; }
__weak void     _crit_relock(uint32_t nest) { ARG_UNUSED(nest); }

/* ── Debug helpers (set HCI_RT58X_DEBUG=1 to enable HCI hex dumps) ── */

#define HCI_RT58X_DEBUG 0

#if HCI_RT58X_DEBUG
static void dump_hex(const char *dir, const uint8_t *data, uint16_t len)
{
	for (uint16_t i = 0; i < len && i < 40; i++) {
		printk("%02x ", data[i]);
	}
	if (len > 40) {
		printk("...");
	}
	printk("\n");
}

static void decode_tx(const uint8_t *data, uint16_t len)
{
	if (len < 4 || data[0] != H4_CMD) {
		return;
	}
	uint16_t opcode = (uint16_t)data[2] << 8 | data[1];
	uint8_t ogf = opcode >> 10;
	uint16_t ocf = opcode & 0x3FF;
}

static void decode_rx(const uint8_t *data, uint16_t len)
{
	if (len < 3 || data[0] != H4_EVT) {
		return;
	}
	uint8_t evt = data[1];
	uint8_t plen = data[2];

	if (evt == 0x0E && plen >= 4) {
		uint16_t opcode = (uint16_t)data[5] << 8 | data[4];
	} else if (evt == 0x0F && plen >= 4) {
		uint16_t opcode = (uint16_t)data[6] << 8 | data[5];
	}
}
#else
#define dump_hex(dir, data, len)
#define decode_tx(data, len)
#define decode_rx(data, len)
#endif

/* ── RX path (HOSAL callbacks → fifo → rx_thread → bt_recv) ────────────── */

static int hci_evt_cb(void *p_arg)
{
	uint8_t *buf_raw = (uint8_t *)p_arg;
	uint8_t evt_code = buf_raw[1];
	uint8_t param_len = buf_raw[2];
	uint16_t total = 3 + param_len;

	dump_hex("RX", buf_raw, total);
	decode_rx(buf_raw, total);

	/* During setup, discard events (don't forward to host) */
	if (hci_data.setup_in_progress) {
		if (evt_code == BT_HCI_EVT_CMD_COMPLETE ||
		    evt_code == BT_HCI_EVT_CMD_STATUS) {
			return 1;
		}
		return 0;
	}

	struct net_buf *buf = bt_buf_get_evt(evt_code, false, K_NO_WAIT);
	if (!buf) {
		/* CRITICAL: even if we can't forward the event to the host,
		 * we must still return 1 for CMD_COMPLETE/CMD_STATUS so that
		 * handle_event_status() releases g_rf_cmd_sem.  Without this,
		 * a buffer-pool exhaustion permanently deadlocks the HCI path. */
		if (evt_code == BT_HCI_EVT_CMD_COMPLETE ||
		    evt_code == BT_HCI_EVT_CMD_STATUS) {
			return 1;
		}
		return 0;
	}

	/* bt_buf_get_evt() already prepends the H:4 type byte (0x04),
	 * so skip buf_raw[0] to avoid duplication. */
	net_buf_add_mem(buf, &buf_raw[1], total - 1);
	k_fifo_put(&rx_fifo, buf);

	if (evt_code == BT_HCI_EVT_CMD_COMPLETE ||
	    evt_code == BT_HCI_EVT_CMD_STATUS) {
		return 1;
	}
	return 0;
}

static int hci_acl_cb(void *p_arg)
{
	/*
	 * Rafael BLE controller RX ACL wire format (NO sequence field):
	 *   [0]   transport_id  (0x02)
	 *   [1-2] handle:12 + pb_flag:2 + bc_flag:2  (uint16 LE)
	 *   [3-4] data_len      (uint16 LE, payload byte count)
	 *   [5+]  payload
	 *
	 * NOTE: Unlike TX (which host adds a sequence field at [1-2]),
	 * the controller does NOT include sequence in received ACL data.
	 * See Rafael SDK: ble_hci_acl_data_struct (RX) vs
	 *                 ble_hci_acl_data_sn_struct (TX).
	 *
	 * bt_buf_get_rx(BT_BUF_ACL_IN) records the H:4 type in net_buf
	 * metadata, so we must NOT copy buf_raw[0].
	 * The HCI ACL content passed to the host starts at buf_raw[1].
	 */
	uint8_t *buf_raw = (uint8_t *)p_arg;
	uint16_t data_len = sys_get_le16(&buf_raw[3]);

	dump_hex("RX-ACL", buf_raw, 5u + data_len);

	struct net_buf *buf = bt_buf_get_rx(BT_BUF_ACL_IN, K_NO_WAIT);
	if (!buf) {
		return 0;
	}

	/* Copy handle+flags(2) + data_len(2) + payload, skipping type byte. */
	net_buf_add_mem(buf, &buf_raw[1], 4u + data_len);
	k_fifo_put(&rx_fifo, buf);
	return 0;
}

static void rx_thread(void *p1, void *p2, void *p3)
{
	const struct device *dev = p1;
	struct hci_rt58x_data *data = dev->data;

	while (1) {
		struct net_buf *buf = k_fifo_get(&rx_fifo, K_FOREVER);
		if (data->recv) {
			data->recv(dev, buf);
		} else {
			net_buf_unref(buf);
		}
	}
}

/* ── TX path (bt_send → HOSAL) ─────────────────────────────────────────── */

#define SEND_RETRY_MAX   20
#define SEND_RETRY_DELAY K_MSEC(10)

static int hci_rt58x_send(const struct device *dev, struct net_buf *buf)
{
	uint8_t type = buf->data[0];
	int rc;

	dump_hex("TX", buf->data, buf->len);
	decode_tx(buf->data, buf->len);

	switch (type) {
	case H4_CMD: {
		for (int i = 0; i < SEND_RETRY_MAX; i++) {
			rc = hosal_rf_write_command(buf->data, buf->len);
			if (rc == 0) {
				break;
			}
			k_sleep(SEND_RETRY_DELAY);
		}
		if (rc != 0) {
			net_buf_unref(buf);
			return -EIO;
		}
		break;
	}
	case H4_ACL: {
		/*
		 * Rafael BLE controller wire format (hosal_rf_write_tx_data):
		 *   [0]   transport_id  (0x02)
		 *   [1-2] sequence      (uint16 LE, per-connection TX counter)
		 *   [3-4] handle:12 + pb_flag:2 + bc_flag:2  (uint16 LE)
		 *   [5-6] length        (uint16 LE, payload byte count)
		 *   [7+]  payload
		 *
		 * Standard HCI ACL (buf->data):
		 *   [0]   H4 type (0x02)
		 *   [1-2] handle+flags (uint16 LE)
		 *   [3-4] length       (uint16 LE)
		 *   [5+]  payload
		 *
		 * We must repack to insert the sequence field.
		 */
		static uint16_t tx_sn;

		uint16_t hci_handle_flags = sys_get_le16(&buf->data[1]);
		uint16_t hci_len          = sys_get_le16(&buf->data[3]);
		uint8_t *payload          = &buf->data[5];
		uint16_t total            = 7u + hci_len;

		uint8_t *raf_buf = k_malloc(total);
		if (!raf_buf) {
			net_buf_unref(buf);
			return -ENOMEM;
		}

		raf_buf[0] = H4_ACL;
		sys_put_le16(tx_sn++,           &raf_buf[1]);
		sys_put_le16(hci_handle_flags,  &raf_buf[3]);
		sys_put_le16(hci_len,           &raf_buf[5]);
		memcpy(&raf_buf[7], payload, hci_len);


		for (int i = 0; i < SEND_RETRY_MAX; i++) {
			rc = hosal_rf_write_tx_data(raf_buf, total);
			if (rc == 0) {
				break;
			}
			k_sleep(SEND_RETRY_DELAY);
		}

		k_free(raf_buf);

		if (rc != 0) {
			net_buf_unref(buf);
			return -EIO;
		}
		break;
	}
	default:
		net_buf_unref(buf);
		return -EINVAL;
	}

	net_buf_unref(buf);
	return 0;
}

/* ── Vendor-specific setup ─────────────────────────────────────────────── */

static int hci_rt58x_setup(const struct device *dev,
			    const struct bt_hci_setup_params *params)
{
	/*
	 * Read factory MAC from flash Security Register 0x1100.
	 * Layout matches ot_radio.c::rafael_otp_mac_addr(): the 8-byte
	 * EUI-64 sits at offset 8 in the 256-byte page.  BLE uses a 6-byte
	 * address, so take the last 6 bytes of the EUI-64.  If the OTP is
	 * blank (all 0xFF), fall back to flash unique ID.
	 */
	uint8_t addr[6];
	{
		uint8_t otp[256];
		uint8_t eui64[8];
		uint8_t ff[8];

		memset(ff, 0xFF, sizeof(ff));
		flash_read_sec_register((uint32_t)(uintptr_t)otp, 0x1100);
		memcpy(eui64, otp + 8, 8);

		if (!memcmp(eui64, ff, 8)) {
			/* OTP blank — derive from flash unique ID */
			flash_get_unique_id((uint32_t)(uintptr_t)eui64, 8);
		}

		/* BLE 6-byte address from EUI-64: take bytes [2..7] */
		memcpy(addr, eui64 + 2, 6);
	}

	int rc;

	/* ── Step 0: RUCI INITIATE_BLE ──────────────────────────────────────
	 * In multi-protocol mode the RF MCU scheduler must be told that BLE
	 * is active (analogous to RUCI_INITIATE_ZIGBEE on the OT side).
	 * Without this, the BLE HCI command queue is not polled while the RF
	 * MCU is in NORMAL state doing 802.15.4 — causing BLE commands like
	 * LE Set Random Address (0x2005) to time out.
	 *
	 * Packet: [0x12=PCI_BLE_CMD_HEADER, 0x01=INITIATE_BLE code, 0x00=len]
	 * Response: RUCI_PCI_EVENT confirmation → handle_event_status releases
	 * g_rf_cmd_sem automatically even if g_pci_event_cb is NULL.          */
	/* RUCI_INITIATE_BLE is now sent early in AppTask::Init() (right after
	 * lmac15p4_init), matching Rafael official SDK's ble_init() timing.
	 * Sending it here (much later, during bt_enable) causes the RF MCU
	 * scheduler to de-prioritize the 15.4 TX queue → 15.4 frames never
	 * appear OTA.  Skip it here; the early init already activated BLE. */

	uint8_t cmd[] = {
		H4_CMD, 0x01, 0xFC, RT58X_VS_SET_CTRL_INFO_PLEN,
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
		RT58X_BLE_VERSION,                     /* 0x0C = BT 5.3 */
		0x64, 0x08,                            /* Company ID 0x0864 LE */
	};

	hci_data.setup_in_progress = true;

	for (int i = 0; i < SEND_RETRY_MAX; i++) {
		rc = hosal_rf_write_command(cmd, sizeof(cmd));
		if (rc == 0) {
			break;
		}
		k_sleep(SEND_RETRY_DELAY);
	}
	if (rc != 0) {
		hci_data.setup_in_progress = false;
		return -EIO;
	}

	/* Wait for vendor command response (discarded by setup_in_progress gate) */
	k_sleep(K_MSEC(100));
	hci_data.setup_in_progress = false;
	return 0;
}

/* ── Open / close ──────────────────────────────────────────────────────── */

static int hci_rt58x_open(const struct device *dev, bt_hci_recv_t recv)
{
	struct hci_rt58x_data *data = dev->data;

	data->recv = recv;

	hosal_rf_callback_set(HOSAL_RF_BLE_EVENT_CALLBACK,
			      (hosal_rf_callback_t)hci_evt_cb, NULL);
	hosal_rf_callback_set(HOSAL_RF_BLE_RX_CALLBACK,
			      (hosal_rf_callback_t)hci_acl_cb, NULL);

	if (!hci_rt58x_rf_already_init) {
		/* Standalone BLE (no OT): initialize RF MCU in BLE-only mode. */
		hosal_lpm_init();
		hosal_lpm_ioctrl(HOSAL_LPM_SET_POWER_LEVEL,
				 HOSAL_LOW_POWER_LEVEL_SLEEP0);
		hosal_rf_init(HOSAL_RF_MODE_BLE_CONTROLLER);
		k_sleep(K_MSEC(50));
	}

	k_thread_create(&rx_thread_data, rx_thread_stack,
			K_THREAD_STACK_SIZEOF(rx_thread_stack),
			rx_thread, (void *)dev, NULL, NULL,
			RX_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&rx_thread_data, "bt_rx");

	return 0;
}

static int hci_rt58x_close(const struct device *dev)
{
	struct hci_rt58x_data *data = dev->data;
	data->recv = NULL;
	return 0;
}

/* ── Device instantiation ──────────────────────────────────────────────── */

static int hci_rt58x_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static const struct bt_hci_driver_api hci_rt58x_api = {
	.open  = hci_rt58x_open,
	.close = hci_rt58x_close,
	.send  = hci_rt58x_send,
	.setup = hci_rt58x_setup,
};

/* DT compat is SoC-specific because Zephyr's DEVICE_DT_INST_DEFINE keys off
 * a single DT_DRV_COMPAT. The driver body is shared; only the compat string
 * differs between rt583 (rafael,rt583-bt-hci) and rt584 (rafael,rt584-bt-hci). */
#if defined(CONFIG_SOC_SERIES_RT584)
#define DT_DRV_COMPAT rafael_rt584_bt_hci
#else
#define DT_DRV_COMPAT rafael_rt583_bt_hci
#endif

DEVICE_DT_INST_DEFINE(0, hci_rt58x_init, NULL,
		      &hci_data, NULL,
		      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		      &hci_rt58x_api);
