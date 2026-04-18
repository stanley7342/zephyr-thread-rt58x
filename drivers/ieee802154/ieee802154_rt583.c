/*
 * Copyright (c) 2025 Rafael Microelectronics
 * SPDX-License-Identifier: Apache-2.0
 *
 * IEEE 802.15.4 radio driver for the RT583.
 *
 * Two modes selectable via Kconfig:
 *
 *   CONFIG_IEEE802154_RT583_FULL=n (default):
 *     Stub driver — every API returns -ENOTSUP.  Exists only so that
 *     Zephyr's OpenThread platform radio.c can resolve
 *     DT_CHOSEN(zephyr_ieee802154).  Actual radio traffic is handled by
 *     sdk/.../openthread_port/Src/ot_radio.c which calls lmac15p4_* and
 *     overrides the otPlatRadio* family directly.
 *
 *   CONFIG_IEEE802154_RT583_FULL=y:
 *     Real driver — implements ieee802154_radio_api over lmac15p4, so that
 *     Zephyr's radio.c drives the RT583 via the standard driver API.
 *     Registers a net_if (NET_DEVICE_DT_INST_DEFINE) so the L2
 *     openthread stack and socket path work.  Must be enabled together
 *     with removing ot_radio.c's otPlatRadio* overrides — both paths
 *     cannot coexist.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/net/ieee802154_radio.h>
#include <zephyr/init.h>

#define DT_DRV_COMPAT rafael_rt583_ieee802154

#ifdef CONFIG_IEEE802154_RT583_FULL

#include <zephyr/logging/log.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/random/random.h>
#include <string.h>

/* lmac15p4 and hosal from the vendor SDK */
#include "lmac15p4.h"
#include "hosal_rf.h"
#include "hosal_trng.h"
#include "flashctl.h"

LOG_MODULE_REGISTER(ieee802154_rt583, CONFIG_IEEE802154_DRIVER_LOG_LEVEL);

/* ── Constants ────────────────────────────────────────────────────────────── */
#define RT583_RX_SENSITIVITY_DBM    (-101)
#define RT583_DEFAULT_TX_POWER_DBM  10
#define RT583_MIN_CHANNEL           11
#define RT583_MAX_CHANNEL           26

/* ── Per-instance state ───────────────────────────────────────────────────── */
struct rt583_radio_data {
	const struct device *dev;
	struct net_if       *iface;

	uint8_t  eui64[8];          /* big-endian, as exposed to the stack      */
	uint8_t  mac_addr[8];       /* little-endian, as stored by lmac         */
	uint16_t short_addr;
	uint16_t pan_id;

	bool     lmac_ready;        /* set on first start() after hosal_rf_init */
	bool     started;
	bool     promiscuous;
	bool     coordinator;
	uint8_t  channel;
	int8_t   tx_power_dbm;

	/* TX wait: lmac TX-done fires in ISR, signal a sem */
	struct k_sem tx_done_sem;
	uint32_t     tx_done_status;  /* LMAC154_TX_SUCCESS etc.                */
};

static struct rt583_radio_data rt583_radio_data_0;

/* ── Forward decls ────────────────────────────────────────────────────────── */
static void rt583_rx_done_cb(uint16_t packet_length, uint8_t *pdata,
			     uint8_t crc_status, uint8_t rssi, uint8_t snr);
static void rt583_tx_done_cb(uint32_t tx_status);

/* ── ieee802154_radio_api implementations ─────────────────────────────────── */

static enum ieee802154_hw_caps rt583_get_capabilities(const struct device *dev)
{
	ARG_UNUSED(dev);
	/* lmac15p4 handles auto-ACK and CSMA-CA in hardware; filter is HW too.
	 * Energy scan is done via read_rssi.  TXTIME is not supported by the
	 * lmac API we use here. */
	return IEEE802154_HW_FCS
	     | IEEE802154_HW_FILTER
	     | IEEE802154_HW_PROMISC
	     | IEEE802154_HW_CSMA
	     | IEEE802154_HW_TX_RX_ACK
	     | IEEE802154_HW_RX_TX_ACK
	     | IEEE802154_HW_ENERGY_SCAN;
}

static int rt583_cca(const struct device *dev)
{
	ARG_UNUSED(dev);
	/* lmac15p4 performs CCA as part of CSMA-CA in tx().  A standalone
	 * CCA with the hardware is not exposed by the vendor API; Zephyr's
	 * radio.c only calls this when the transmit mode requires explicit
	 * CCA without CSMA — for Thread that path is unused. */
	return -ENOTSUP;
}

static int rt583_set_channel(const struct device *dev, uint16_t channel)
{
	struct rt583_radio_data *data = dev->data;

	if (channel < RT583_MIN_CHANNEL || channel > RT583_MAX_CHANNEL) {
		return -EINVAL;
	}
	data->channel = (uint8_t)channel;
	/* Defer real HW set to start() when lmac/RF MCU is ready */
	if (data->lmac_ready) {
		lmac15p4_channel_set((lmac154_channel_t)(channel - RT583_MIN_CHANNEL));
	}
	return 0;
}

static int rt583_filter(const struct device *dev, bool set,
			enum ieee802154_filter_type type,
			const struct ieee802154_filter *filter)
{
	struct rt583_radio_data *data = dev->data;

	/* lmac15p4_address_filter_set takes all four pieces at once; we cache
	 * them here and push the full set on every filter() call.  Zephyr's
	 * L2 calls this for short_addr, ieee_addr and pan_id in sequence on
	 * network start, so the cached values become coherent before RX. */
	if (!set) {
		/* L2 never requests unset for the types we handle */
		return -ENOTSUP;
	}

	switch (type) {
	case IEEE802154_FILTER_TYPE_IEEE_ADDR:
		memcpy(data->mac_addr, filter->ieee_addr, sizeof(data->mac_addr));
		break;
	case IEEE802154_FILTER_TYPE_SHORT_ADDR:
		data->short_addr = filter->short_addr;
		break;
	case IEEE802154_FILTER_TYPE_PAN_ID:
		data->pan_id = filter->pan_id;
		break;
	default:
		return -ENOTSUP;
	}

	if (data->lmac_ready) {
		uint32_t ext_lo, ext_hi;
		memcpy(&ext_lo, &data->mac_addr[0], 4);
		memcpy(&ext_hi, &data->mac_addr[4], 4);
		lmac15p4_address_filter_set(/* pan_idx */ 0,
					    data->promiscuous ? 1 : 0,
					    data->short_addr,
					    ext_lo,
					    ext_hi,
					    data->pan_id,
					    data->coordinator);
	}
	return 0;
}

static int rt583_set_txpower(const struct device *dev, int16_t dbm)
{
	struct rt583_radio_data *data = dev->data;
	/* RT583 TX power is controlled via hosal_rf; clamp to reasonable range.
	 * The vendor API does not expose dBm directly here — the channel_set
	 * path applies a calibration table.  Cache the requested value; a
	 * future refinement can map dBm → register values via hosal_rf. */
	data->tx_power_dbm = (int8_t)CLAMP(dbm, -20, 20);
	return 0;
}

static int rt583_start(const struct device *dev)
{
	struct rt583_radio_data *data = dev->data;

	/* First-call lmac setup: deferred from POST_KERNEL device init so
	 * that hosal_rf_init() (called from AppTask → BLEManagerImpl) has
	 * had a chance to load the RF MCU firmware.  Calling lmac15p4_*
	 * before that would dispatch to an uninitialised RF MCU and hang. */
	if (!data->lmac_ready) {
		lmac15p4_callback_t cb = {
			.rx_cb = rt583_rx_done_cb,
			.tx_cb = rt583_tx_done_cb,
		};
		lmac15p4_cb_set(/* pan_idx */ 0, &cb);
		lmac15p4_auto_ack_set(true);
		lmac15p4_channel_set((lmac154_channel_t)(data->channel - RT583_MIN_CHANNEL));
		lmac15p4_src_match_ctrl(/* pan_idx */ 0, true);
		lmac15p4_src_match_short_entry(/* CLEAR_ALL */ 2, NULL);
		data->lmac_ready = true;
	}
	lmac15p4_auto_state_set(true);
	data->started = true;
	return 0;
}

static int rt583_stop(const struct device *dev)
{
	struct rt583_radio_data *data = dev->data;
	lmac15p4_auto_state_set(false);
	data->started = false;
	return 0;
}

static int rt583_tx(const struct device *dev,
		    enum ieee802154_tx_mode mode,
		    struct net_pkt *pkt,
		    struct net_buf *frag)
{
	struct rt583_radio_data *data = dev->data;
	uint32_t csma_ca_flag;
	int rc;

	ARG_UNUSED(pkt);

	switch (mode) {
	case IEEE802154_TX_MODE_DIRECT:
		csma_ca_flag = 0;
		break;
	case IEEE802154_TX_MODE_CSMA_CA:
		csma_ca_flag = 1;
		break;
	default:
		LOG_WRN("TX mode %d not supported, falling back to CSMA-CA", mode);
		csma_ca_flag = 1;
		break;
	}

	/* lmac15p4 expects: pan_idx, address, length, csma_enable.  "length"
	 * includes the 2-byte FCS which the hardware appends; frag->len is the
	 * full PSDU (payload + FCS space). */
	k_sem_reset(&data->tx_done_sem);
	data->tx_done_status = LMAC154_TX_FAIL;

	rc = lmac15p4_tx_data_send(/* pan_idx */ 0,
				   frag->data,
				   frag->len,
				   /* ack_required */ 0,
				   csma_ca_flag);
	if (rc != 0) {
		return -EIO;
	}

	if (k_sem_take(&data->tx_done_sem, K_MSEC(200)) != 0) {
		LOG_WRN("TX done timeout");
		return -EIO;
	}

	switch (data->tx_done_status) {
	case LMAC154_TX_SUCCESS:
	case LMAC154_TX_GET_ACK_SUCCESS:
	case LMAC154_TX_GET_ACK_FP_SUCCESS:
		return 0;
	case LMAC154_TX_CSMACA_FAIL:
		return -EBUSY;
	case LMAC154_TX_NO_ACK_FAIL:
		return -ENOMSG;
	default:
		return -EIO;
	}
}

static int rt583_configure(const struct device *dev,
			   enum ieee802154_config_type type,
			   const struct ieee802154_config *config)
{
	struct rt583_radio_data *data = dev->data;

	switch (type) {
	case IEEE802154_CONFIG_PROMISCUOUS:
		data->promiscuous = config->promiscuous;
		if (data->lmac_ready) {
			uint32_t ext_lo, ext_hi;
			memcpy(&ext_lo, &data->mac_addr[0], 4);
			memcpy(&ext_hi, &data->mac_addr[4], 4);
			lmac15p4_address_filter_set(/* pan_idx */ 0,
						    data->promiscuous ? 1 : 0,
						    data->short_addr,
						    ext_lo, ext_hi,
						    data->pan_id,
						    data->coordinator);
		}
		return 0;
	case IEEE802154_CONFIG_PAN_COORDINATOR:
		data->coordinator = config->pan_coordinator;
		return 0;
	case IEEE802154_CONFIG_AUTO_ACK_FPB:
		/* lmac15p4 has a source-address match table used for data-req
		 * frame-pending decisions; toggling the whole feature is what
		 * otPlatRadioEnableSrcMatch does — mirror that here. */
		if (data->lmac_ready) {
			lmac15p4_src_match_ctrl(/* pan_idx */ 0,
						config->auto_ack_fpb.enabled ? 1 : 0);
		}
		return 0;
	case IEEE802154_CONFIG_ACK_FPB:
		if (!data->lmac_ready) {
			return 0;
		}
		if (config->ack_fpb.extended) {
			lmac15p4_src_match_extended_entry(
				config->ack_fpb.enabled ? 0 /* ADD */ : 1 /* REMOVE */,
				config->ack_fpb.addr);
		} else {
			lmac15p4_src_match_short_entry(
				config->ack_fpb.enabled ? 0 /* ADD */ : 1 /* REMOVE */,
				config->ack_fpb.addr);
		}
		return 0;
	case IEEE802154_CONFIG_EVENT_HANDLER:
		/* Radio event handler not used on RT583 (no SCAN complete etc.) */
		return 0;
	default:
		return -ENOTSUP;
	}
}

/* ── RX path: lmac15p4 ISR → net_pkt → net_recv_data ──────────────────────── */

static void rt583_rx_done_cb(uint16_t packet_length, uint8_t *pdata,
			     uint8_t crc_status, uint8_t rssi, uint8_t snr)
{
	struct rt583_radio_data *data = &rt583_radio_data_0;
	struct net_pkt *pkt;

	if (crc_status != 0 || packet_length < 5) {
		return;
	}
	/* packet_length includes the 2-byte FCS which we include in the pkt */
	pkt = net_pkt_rx_alloc_with_buffer(data->iface, packet_length,
					   AF_UNSPEC, 0, K_NO_WAIT);
	if (!pkt) {
		return;
	}
	if (net_pkt_write(pkt, pdata, packet_length) < 0) {
		net_pkt_unref(pkt);
		return;
	}
	net_pkt_set_ieee802154_rssi_dbm(pkt, -(int8_t)rssi);
	net_pkt_set_ieee802154_lqi(pkt, snr);

	if (net_recv_data(data->iface, pkt) < 0) {
		net_pkt_unref(pkt);
	}
}

static void rt583_tx_done_cb(uint32_t tx_status)
{
	struct rt583_radio_data *data = &rt583_radio_data_0;
	data->tx_done_status = tx_status;
	k_sem_give(&data->tx_done_sem);
}

/* ── EUI-64 derivation (same logic as ot_radio.c) ─────────────────────────── */
static void rt583_derive_eui64(uint8_t *out)
{
	static const uint8_t ff[8] = {0xFF, 0xFF, 0xFF, 0xFF,
				      0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t buf[256];

	flash_read_sec_register((uint32_t)(uintptr_t)buf, 0x1100);
	memcpy(out, buf + 8, 8);
	if (!memcmp(out, ff, 8)) {
		flash_get_unique_id((uint32_t)(uintptr_t)out, 8);
	}
	if (!memcmp(out, ff, 8)) {
		hosal_trng_get_random_number((uint32_t *)out, 2);
		out[0] |= 0x02;
		out[0] &= ~0x01;
	}
}

/* ── Init ─────────────────────────────────────────────────────────────────── */

static void rt583_iface_init(struct net_if *iface)
{
	const struct device *dev = net_if_get_device(iface);
	struct rt583_radio_data *data = dev->data;

	rt583_derive_eui64(data->eui64);
	net_if_set_link_addr(iface, data->eui64, sizeof(data->eui64),
			     NET_LINK_IEEE802154);
	data->iface = iface;

	ieee802154_init(iface);
}

static int rt583_radio_init(const struct device *dev)
{
	struct rt583_radio_data *data = dev->data;

	/* POST_KERNEL runs before AppTask → hosal_rf_init; must NOT touch
	 * lmac15p4 here or the uninitialised RF MCU hangs the boot.  Only
	 * populate software state; real radio setup happens on first
	 * rt583_start() call. */
	data->dev         = dev;
	data->channel     = RT583_MIN_CHANNEL;
	data->tx_power_dbm = RT583_DEFAULT_TX_POWER_DBM;
	data->short_addr  = 0xFFFF;
	data->pan_id      = 0xFFFF;
	data->lmac_ready  = false;
	memset(data->mac_addr, 0, sizeof(data->mac_addr));
	k_sem_init(&data->tx_done_sem, 0, 1);

	return 0;
}

static struct ieee802154_radio_api rt583_radio_api = {
	.iface_api.init   = rt583_iface_init,
	.get_capabilities = rt583_get_capabilities,
	.cca              = rt583_cca,
	.set_channel      = rt583_set_channel,
	.filter           = rt583_filter,
	.set_txpower      = rt583_set_txpower,
	.tx               = rt583_tx,
	.start            = rt583_start,
	.stop             = rt583_stop,
	.configure        = rt583_configure,
};

/* Register as a real network device so L2 openthread gets a net_if. */
#define IEEE802154_RT583_MTU 125
#define IEEE802154_RT583_INIT(n) \
	NET_DEVICE_DT_INST_DEFINE(n, \
				  rt583_radio_init, NULL, \
				  &rt583_radio_data_0, NULL, \
				  CONFIG_IEEE802154_RT583_INIT_PRIORITY, \
				  &rt583_radio_api, \
				  IEEE802154_L2, \
				  NET_L2_GET_CTX_TYPE(IEEE802154_L2), \
				  IEEE802154_RT583_MTU);

#else  /* !CONFIG_IEEE802154_RT583_FULL — stub mode */

/*
 * Stub driver retained for backwards compatibility.  Exists only to
 * satisfy modules/openthread/platform/radio.c's DT_CHOSEN resolution.
 * The real radio operation is handled by ot_radio.c / lmac15p4 via the
 * otPlatRadio* overrides with --allow-multiple-definition link order.
 */

static int ieee802154_rt583_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

static enum ieee802154_hw_caps ieee802154_rt583_get_capabilities(
	const struct device *dev)
{
	ARG_UNUSED(dev);
	/* HW_TX_RX_ACK prevents Zephyr's platformRadioInit() from k_panic-ing */
	return IEEE802154_HW_TX_RX_ACK;
}

#define STUB(fn, ...) \
	static int ieee802154_rt583_##fn(__VA_ARGS__) { return -ENOTSUP; }
STUB(configure,   const struct device *dev, enum ieee802154_config_type type, \
		   const struct ieee802154_config *config)
STUB(cca,         const struct device *dev)
STUB(set_channel, const struct device *dev, uint16_t channel)
STUB(filter,      const struct device *dev, bool set, \
		   enum ieee802154_filter_type type, \
		   const struct ieee802154_filter *filter)
STUB(set_txpower, const struct device *dev, int16_t dbm)
STUB(tx,          const struct device *dev, enum ieee802154_tx_mode mode, \
		   struct net_pkt *pkt, struct net_buf *frag)
STUB(start,       const struct device *dev)
STUB(stop,        const struct device *dev)
#undef STUB

static struct ieee802154_radio_api ieee802154_rt583_api = {
	.iface_api.init   = NULL,
	.get_capabilities = ieee802154_rt583_get_capabilities,
	.configure        = ieee802154_rt583_configure,
	.cca              = ieee802154_rt583_cca,
	.set_channel      = ieee802154_rt583_set_channel,
	.filter           = ieee802154_rt583_filter,
	.set_txpower      = ieee802154_rt583_set_txpower,
	.tx               = ieee802154_rt583_tx,
	.start            = ieee802154_rt583_start,
	.stop             = ieee802154_rt583_stop,
};

#define IEEE802154_RT583_INIT(n) \
	DEVICE_DT_INST_DEFINE(n, \
			      ieee802154_rt583_init, \
			      NULL, \
			      NULL, NULL, \
			      POST_KERNEL, \
			      CONFIG_IEEE802154_RT583_INIT_PRIORITY, \
			      &ieee802154_rt583_api);

#endif  /* CONFIG_IEEE802154_RT583_FULL */

DT_INST_FOREACH_STATUS_OKAY(IEEE802154_RT583_INIT)
