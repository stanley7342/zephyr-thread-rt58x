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
/* #include "flashctl.h" — not safe during early BLE setup */

#define LOG_PREFIX "[HCI] "

/* ISR count from soc.c */
extern volatile uint32_t rt582_comm_irq_count;

/* BLE HCI H:4 packet type indicators */
#define H4_CMD  0x01
#define H4_ACL  0x02
#define H4_EVT  0x04

#define RT582_COMM_SUBSYSTEM_IRQN 20

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

/* FreeRTOS critical-section shim */
__weak uint32_t _crit_unlock(void) { return 0; }
__weak void     _crit_relock(uint32_t nest) { ARG_UNUSED(nest); }

/* ── Debug helpers (disabled — enable by setting HCI_RT58X_DEBUG to 1) ── */

#define HCI_RT58X_DEBUG 0

#if HCI_RT58X_DEBUG
static void dump_hex(const char *dir, const uint8_t *data, uint16_t len)
{
	printk(LOG_PREFIX "%s[%u]: ", dir, len);
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
	printk(LOG_PREFIX "  -> CMD opcode=0x%04x (OGF=0x%02x OCF=0x%04x) plen=%u\n",
	       opcode, ogf, ocf, data[3]);
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
		printk(LOG_PREFIX "  <- CMD_COMPLETE opcode=0x%04x status=0x%02x ncmd=%u\n",
		       opcode, data[6], data[3]);
	} else if (evt == 0x0F && plen >= 4) {
		uint16_t opcode = (uint16_t)data[6] << 8 | data[5];
		printk(LOG_PREFIX "  <- CMD_STATUS opcode=0x%04x status=0x%02x ncmd=%u\n",
		       opcode, data[3], data[4]);
	} else {
		printk(LOG_PREFIX "  <- EVT code=0x%02x plen=%u\n", evt, plen);
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
		printk(LOG_PREFIX "drop evt 0x%02x\n", evt_code);
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
	uint8_t *buf_raw = (uint8_t *)p_arg;
	uint16_t data_len = sys_get_le16(&buf_raw[3]);
	uint16_t total = 5 + data_len;

	dump_hex("RX-ACL", buf_raw, total);

	struct net_buf *buf = bt_buf_get_rx(BT_BUF_ACL_IN, K_NO_WAIT);
	if (!buf) {
		printk(LOG_PREFIX "drop ACL\n");
		return 0;
	}

	/* bt_buf_get_rx() prepends the H:4 type byte (0x02),
	 * so skip buf_raw[0] to avoid duplication. */
	net_buf_add_mem(buf, &buf_raw[1], total - 1);
	/* printk(LOG_PREFIX "ACL-IN len=%u\n", buf->len); */
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
	case H4_CMD:
		for (int i = 0; i < SEND_RETRY_MAX; i++) {
			rc = hosal_rf_write_command(buf->data, buf->len);
			if (rc == 0) {
				break;
			}
			printk(LOG_PREFIX "cmd retry %d\n", i);
			k_sleep(SEND_RETRY_DELAY);
		}
		if (rc != 0) {
			printk(LOG_PREFIX "cmd send failed\n");
			net_buf_unref(buf);
			return -EIO;
		}
		break;
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
			printk(LOG_PREFIX "ACL alloc failed\n");
			net_buf_unref(buf);
			return -ENOMEM;
		}

		raf_buf[0] = H4_ACL;
		sys_put_le16(tx_sn++,           &raf_buf[1]);
		sys_put_le16(hci_handle_flags,  &raf_buf[3]);
		sys_put_le16(hci_len,           &raf_buf[5]);
		memcpy(&raf_buf[7], payload, hci_len);

		/* printk(LOG_PREFIX "ACL-OUT len=%u seq=%u\n", total, tx_sn - 1); */
		dump_hex("TX-ACL", raf_buf, total);

		for (int i = 0; i < SEND_RETRY_MAX; i++) {
			rc = hosal_rf_write_tx_data(raf_buf, total);
			if (rc == 0) {
				break;
			}
			printk(LOG_PREFIX "ACL retry %d rc=%d pwr=0x%02x\n",
			       i, rc, RfMcu_PowerStateCheck());
			k_sleep(SEND_RETRY_DELAY);
		}

		k_free(raf_buf);

		if (rc != 0) {
			printk(LOG_PREFIX "ACL send failed rc=%d\n", rc);
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
	 * TODO: read factory MAC from flash Security Register 0x1100
	 * after flash controller init is confirmed safe at this stage.
	 * For now use a fixed unique address.
	 */
	uint8_t addr[6] = {0x58, 0x82, 0xBE, 0xEF, 0xCA, 0xFE};

	printk(LOG_PREFIX "setup: BD ADDR=%02X:%02X:%02X:%02X:%02X:%02X\n",
	       addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

	uint8_t cmd[] = {
		H4_CMD, 0x01, 0xFC, RT58X_VS_SET_CTRL_INFO_PLEN,
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5],
		RT58X_BLE_VERSION,                     /* 0x0C = BT 5.3 */
		0x64, 0x08,                            /* Company ID 0x0864 LE */
	};
	int rc;

	printk(LOG_PREFIX "setup: vendor Set Controller Info\n");
	dump_hex("TX-SETUP", cmd, sizeof(cmd));

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
		printk(LOG_PREFIX "setup: FAILED rc=%d\n", rc);
		return -EIO;
	}

	k_sleep(K_MSEC(100));
	hci_data.setup_in_progress = false;

	printk(LOG_PREFIX "setup: done irq=%u\n", rt582_comm_irq_count);
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

	hosal_lpm_init();
	hosal_lpm_ioctrl(HOSAL_LPM_SET_POWER_LEVEL,
			 HOSAL_LOW_POWER_LEVEL_SLEEP0);

	/* Disable IRQ while hosal_rf_init() sets gRfMcuIsrCfg.commsubsystem_isr.
	 * Without this, a spurious COMM_SUBSYSTEM interrupt (e.g. from RF MCU
	 * waking up during MCUboot's longer boot window) fires before the ISR
	 * callback pointer is set, calling isr_cb(NULL) → PC=0x0 crash. */
	irq_disable(RT582_COMM_SUBSYSTEM_IRQN);
	hosal_rf_init(HOSAL_RF_MODE_BLE_CONTROLLER);
	irq_enable(RT582_COMM_SUBSYSTEM_IRQN);
	k_sleep(K_MSEC(50));

	k_thread_create(&rx_thread_data, rx_thread_stack,
			K_THREAD_STACK_SIZEOF(rx_thread_stack),
			rx_thread, (void *)dev, NULL, NULL,
			RX_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&rx_thread_data, "bt_rx");

	printk(LOG_PREFIX "opened pwr=0x%02x irq=%u\n",
	       RfMcu_PowerStateCheck(), rt582_comm_irq_count);
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

#define DT_DRV_COMPAT rafael_rt582_bt_hci

DEVICE_DT_INST_DEFINE(0, hci_rt58x_init, NULL,
		      &hci_data, NULL,
		      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		      &hci_rt58x_api);
