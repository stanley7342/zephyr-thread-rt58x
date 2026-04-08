/*
 * BLE Peripheral — Heart Rate Service example for RT583 EVB.
 *
 * Advertises as "RT583 HRS" and exposes:
 *   - Heart Rate Service (HRS) — simulated heart rate
 *   - Battery Service (BAS)    — simulated battery level
 *   - Device Information Service (DIS)
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/services/hrs.h>
#include <zephyr/bluetooth/services/bas.h>

/* ── Advertising data ──────────────────────────────────────────────────── */

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_HRS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_DIS_VAL)),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};

/* ── Connection state ──────────────────────────────────────────────────── */

static bool is_connected;

/* ── Advertising helpers ───────────────────────────────────────────────── */

static void start_advertising(struct k_work *work)
{
	int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad),
				  sd, ARRAY_SIZE(sd));
	if (err) {
		printk("[BLE] Re-advertise failed (err %d)\n", err);
	} else {
		printk("[BLE] Re-advertising as \"%s\"\n", CONFIG_BT_DEVICE_NAME);
	}
}

static K_WORK_DELAYABLE_DEFINE(adv_work, start_advertising);

/* ── Connection callbacks ──────────────────────────────────────────────── */

static void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	printk("[BLE] MTU updated: TX=%u RX=%u\n", tx, rx);
}

static struct bt_gatt_cb gatt_cb = {
	.att_mtu_updated = mtu_updated,
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("[BLE] Connection failed (err 0x%02x)\n", err);
		return;
	}
	is_connected = true;
	printk("[BLE] Connected\n");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	is_connected = false;
	printk("[BLE] Disconnected (reason 0x%02x)\n", reason);

	/* Delay re-advertising by 100 ms: the BLE stack releases the
	 * advertising set asynchronously after disconnect, so calling
	 * bt_le_adv_start() immediately returns -ENOMEM. */
	k_work_schedule(&adv_work, K_MSEC(100));
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected    = connected,
	.disconnected = disconnected,
};

/* ── Simulated sensor data ─────────────────────────────────────────────── */

static uint8_t sim_heart_rate = 72;
static uint8_t sim_battery    = 100;

static void simulate_heart_rate(void)
{
	/* Oscillate between 60 and 120 bpm */
	static int8_t direction = 1;

	sim_heart_rate += direction;
	if (sim_heart_rate >= 120) {
		direction = -1;
	} else if (sim_heart_rate <= 60) {
		direction = 1;
	}
}

static void simulate_battery(void)
{
	if (sim_battery > 0) {
		sim_battery--;
	} else {
		sim_battery = 100;
	}
}

/* ── BLE ready callback ───────────────────────────────────────────────── */

static void bt_ready(int err)
{
	if (err) {
		printk("[BLE] Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("[BLE] Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("[BLE] Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("[BLE] Advertising started as \"%s\"\n", CONFIG_BT_DEVICE_NAME);
}

/* ── Main ──────────────────────────────────────────────────────────────── */

int main(void)
{
	int err;

	printk("========================================\n");
	printk("  RT58x BLE Peripheral — Heart Rate\n");
	printk("  Build: " __DATE__ " " __TIME__ "\n");
	printk("========================================\n");

	bt_gatt_cb_register(&gatt_cb);

	err = bt_enable(bt_ready);
	if (err) {
		printk("[BLE] bt_enable() failed (err %d)\n", err);
		return 0;
	}

	/* Main loop: update simulated sensor values every second.
	 * Only notify/print when connected AND client has subscribed.
	 * bt_hrs_notify / bt_bas_set_battery_level return 0 only when
	 * the notification was actually sent to a subscriber. */
	while (1) {
		k_sleep(K_SECONDS(1));

		if (!is_connected) {
			continue;
		}

		simulate_heart_rate();
		if (bt_hrs_notify(sim_heart_rate) == 0) {
			printk("[HRS] HR=%u bpm\n", sim_heart_rate);
		}

		simulate_battery();
		if (bt_bas_set_battery_level(sim_battery) == 0) {
			printk("[BAS] BAT=%u%%\n", sim_battery);
		}
	}

	return 0;
}
