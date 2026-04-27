/*
 * Copyright (c) 2025 Rafael Microelectronics
 * SPDX-License-Identifier: Apache-2.0
 *
 * IEEE 802.15.4 radio driver for Rafael RT58x SoCs (rt583 / rt584).
 *
 * The driver body is SoC-agnostic — it bridges Zephyr's
 * ieee802154_radio_api to the vendor lmac15p4 / hosal_rf APIs which are
 * the same on both SoCs. Only the DT_DRV_COMPAT string switches per SoC
 * because Zephyr's DEVICE_DT_INST_DEFINE keys off a single compat.
 *
 * Two modes selectable via Kconfig:
 *
 *   CONFIG_IEEE802154_RAFAEL_FULL=n (default):
 *     Stub driver — every API returns -ENOTSUP.  Exists only so that
 *     Zephyr's OpenThread platform radio.c can resolve
 *     DT_CHOSEN(zephyr_ieee802154).  Actual radio traffic is handled by
 *     sdk/.../openthread_port/Src/ot_radio.c which calls lmac15p4_* and
 *     overrides the otPlatRadio* family directly.
 *
 *   CONFIG_IEEE802154_RAFAEL_FULL=y:
 *     Real driver — implements ieee802154_radio_api over lmac15p4, so that
 *     Zephyr's radio.c drives the radio via the standard driver API.
 *     Registers a net_if (NET_DEVICE_DT_INST_DEFINE) so the L2
 *     openthread stack and socket path work.  Must be enabled together
 *     with removing ot_radio.c's otPlatRadio* overrides — both paths
 *     cannot coexist.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/net/ieee802154_radio.h>
#include <zephyr/init.h>

#if defined(CONFIG_SOC_SERIES_RT584)
#define DT_DRV_COMPAT rafael_rt584_ieee802154
#else
#define DT_DRV_COMPAT rafael_rt583_ieee802154
#endif

#ifdef CONFIG_IEEE802154_RAFAEL_FULL

#include <zephyr/logging/log.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/net_if.h>
#if defined(CONFIG_NET_L2_OPENTHREAD)
#include <zephyr/net/openthread.h>
#endif
#include <zephyr/random/random.h>
#include <string.h>

/* lmac15p4 and hosal from the vendor SDK */
#include "lmac15p4.h"
#include "hosal_rf.h"
#include "hosal_trng.h"
#include "flashctl.h"

LOG_MODULE_REGISTER(ieee802154_rafael, CONFIG_IEEE802154_DRIVER_LOG_LEVEL);

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

	/* Last values pushed to HW via lmac15p4_address_filter_set().  Cached
	 * so we can skip redundant pushes — the lmac call resets the RX
	 * pipeline and drops in-flight frames. */
	uint8_t  hw_mac_addr[8];
	uint16_t hw_short_addr;
	uint16_t hw_pan_id;
	bool     hw_promiscuous;
	bool     hw_coordinator;

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
	if ((uint8_t)channel == data->channel && data->lmac_ready) {
		return 0;  /* already on this channel — don't disturb RX */
	}
	data->channel = (uint8_t)channel;
	/* Defer real HW set to start() when lmac/RF MCU is ready */
	if (data->lmac_ready) {
		lmac15p4_channel_set((lmac154_channel_t)(channel - RT583_MIN_CHANNEL));
		/* lmac15p4_channel_set clears auto_state internally.  If RX was
		 * running (started=true), re-enable it so auto-ACK keeps working.
		 * Without this, unicast Parent Response from OTBR gets received
		 * but the device never sends the 15.4 ACK back, so OTBR retries
		 * then drops us. */
		if (data->started) {
			lmac15p4_auto_state_set(true);
		}
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

		/* Skip redundant push: lmac15p4_address_filter_set() resets the
		 * RX pipeline internally, which drops in-flight frames.  OT
		 * refreshes the short addr on every Parent Request cycle
		 * (~2.5s) while DETACHED, which would otherwise blackhole the
		 * MLE Parent Response that arrives right after. */
		if (data->hw_pan_id == data->pan_id &&
		    data->hw_short_addr == data->short_addr &&
		    data->hw_promiscuous == data->promiscuous &&
		    data->hw_coordinator == data->coordinator &&
		    memcmp(data->hw_mac_addr, data->mac_addr,
			   sizeof(data->mac_addr)) == 0) {
			return 0;
		}

		lmac15p4_address_filter_set(/* pan_idx */ 0,
					    data->promiscuous ? 1 : 0,
					    data->short_addr,
					    ext_lo,
					    ext_hi,
					    data->pan_id,
					    data->coordinator);
		data->hw_pan_id       = data->pan_id;
		data->hw_short_addr   = data->short_addr;
		data->hw_promiscuous  = data->promiscuous;
		data->hw_coordinator  = data->coordinator;
		memcpy(data->hw_mac_addr, data->mac_addr, sizeof(data->mac_addr));
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

		/* Push any address-filter values that Zephyr L2 cached before
		 * lmac was ready.  Without this the MAC stays at its default
		 * (pan_id=0xFFFF, short=0xFFFF) and drops every unicast frame
		 * addressed to us — e.g. MLE Parent Response from the leader. */
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
	/* Skip redundant auto_state toggles.  OT may call start() several
	 * times in quick succession (e.g. AttachToThreadNetwork), and each
	 * auto_state_set(true) resets the lmac RX pipeline, dropping any
	 * in-flight MLE Parent Response. */
	if (!data->started) {
		lmac15p4_auto_state_set(true);
		data->started = true;
	}
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
	int rc;

	ARG_UNUSED(pkt);

	/* Parse IEEE 802.15.4 Frame Control bits used by lmac15p4:
	 *   FC byte 0: bit 3 = security enabled, bit 5 = ACK request */
	uint8_t fc0 = (frag->len >= 1) ? frag->data[0] : 0;
	uint8_t ack_req = (fc0 & 0x20) ? 1 : 0;
	uint8_t sec_enabled = (fc0 & 0x08) ? 1 : 0;
	/* MAC sequence number is byte 2 of the PSDU (after 2-byte FC). */
	uint8_t mac_dsn = (frag->len >= 3) ? frag->data[2] : 0;

	/* lmac15p4 mac_control bitfield (matches legacy ot_radio.c TX path):
	 *   bit 0: ACK request (1 = wait for ACK)
	 *   bit 1: always 1 (encoded data-frame type to lmac)
	 *   bit 2: CSMA-CA (1 = enable random backoff)
	 * CSMA is always on unless caller explicitly asks direct TX. */
	uint32_t csma = (mode == IEEE802154_TX_MODE_DIRECT) ? 0 : 1;
	uint8_t mac_control = (ack_req ? 0x01u : 0x00u) | 0x02u
			      | (csma ? 0x04u : 0x00u);

	/* For MAC-secured frames lmac15p4 expects a 4-byte MAC frame counter
	 * prepended BEFORE the PSDU, and the length argument to count from
	 * those 4 bytes.  Without this, lmac cannot compute the correct CCM*
	 * nonce, OTBR's MAC security check fails, and the frame is silently
	 * dropped at OTBR (no MAC ACK, no upper-layer logs).
	 *
	 * We allocate a scratch buffer big enough for the whole PSDU + 4-byte
	 * counter prefix.  For unsecured frames (most MLE), lmac15p4 still
	 * accepts the prefix but simply doesn't use the counter value. */
	static uint32_t s_mac_frame_counter;
	uint32_t hw_fc = lmac15p4_frame_counter_get();
	if (s_mac_frame_counter < hw_fc) {
		s_mac_frame_counter = hw_fc + 1;
	}

	/* 127 = aMaxPHYPacketSize per IEEE 802.15.4-2006; +4 for the frame
	 * counter prefix lmac15p4 requires in front of the PSDU. */
	uint8_t tx_buf[127 + 4];
	if (frag->len > sizeof(tx_buf) - 4) {
		return -EMSGSIZE;
	}
	tx_buf[0] = (s_mac_frame_counter >>  0) & 0xFF;
	tx_buf[1] = (s_mac_frame_counter >>  8) & 0xFF;
	tx_buf[2] = (s_mac_frame_counter >> 16) & 0xFF;
	tx_buf[3] = (s_mac_frame_counter >> 24) & 0xFF;
	memcpy(tx_buf + 4, frag->data, frag->len);

	k_sem_reset(&data->tx_done_sem);
	data->tx_done_status = LMAC154_TX_FAIL;

	static uint32_t tx_count = 0;
	tx_count++;

	/* Length passed to lmac15p4: legacy ot_radio.c used `mLength + 2`
	 * where OT's mLength INCLUDES the 2-byte FCS placeholder.  Zephyr's
	 * OT glue strips FCS on TX (`tx_payload->len = mLength - FCS_SIZE`),
	 * so frag->len is 2 bytes shorter than OT's mLength.  Therefore the
	 * equivalent here is `frag->len + 4` (which equals `mLength + 2`). */
	rc = lmac15p4_tx_data_send(/* pan_idx */ 0,
				   tx_buf,
				   /* packet_length */ frag->len + 4,
				   mac_control,
				   mac_dsn);
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
	static uint32_t rx_count = 0;
	static uint32_t rx_bad_crc = 0;
	if (crc_status != 0) {
		rx_bad_crc++;
	} else {
		rx_count++;
	}

	if (crc_status != 0 || packet_length < 10) {
		return;
	}
	/* lmac15p4 layout (mirroring ot_radio.c _RxDoneEvent):
	 *   [0..7]    = 8-byte lmac meta header (timestamp + control)
	 *   [8..N-3]  = IEEE 802.15.4 MAC header + payload
	 *   [N-2..N-1]= 2-byte FCS (verified by hw; keep — OT glue at
	 *               modules/openthread/platform/radio.c:511 reads
	 *               net_buf_frags_len() as mLength and explicitly
	 *               comments "Length inc. CRC", and CONFIG_IEEE802154_
	 *               L2_PKT_INCL_FCS defaults to y under NET_L2_OPENTHREAD)
	 *   [N]       = 1-byte trailer
	 * Strip 9 bytes total: 8 meta header + 1 trailer.  FCS stays. */
	if (packet_length < 10) {
		return;
	}
	uint16_t psdu_len   = packet_length - 9;
	uint8_t * psdu_data = pdata + 8;

	pkt = net_pkt_rx_alloc_with_buffer(data->iface, psdu_len,
					   AF_UNSPEC, 0, K_NO_WAIT);
	if (!pkt) {
		return;
	}
	if (net_pkt_write(pkt, psdu_data, psdu_len) < 0) {
		net_pkt_unref(pkt);
		return;
	}
	net_pkt_set_ieee802154_rssi_dbm(pkt, -(int8_t)rssi);
	/* Diagnostic: lmac `snr` is not directly a Thread LQI; force max (255)
	 * to see if MLE was rejecting the parent on link-quality grounds. */
	net_pkt_set_ieee802154_lqi(pkt, 255);
	/* Timestamp in nanoseconds; required by OT's MLE security layer for
	 * Thread 1.2+ frames (Enhanced ACK / CSL).  Without a non-zero
	 * timestamp, some MLE validations silently reject the frame. */
	net_pkt_set_timestamp_ns(pkt, k_uptime_ticks() *
				 (1000000000ULL / CONFIG_SYS_CLOCK_TICKS_PER_SEC));
	net_pkt_set_ieee802154_ack_fpb(pkt, false);

	int rc = net_recv_data(data->iface, pkt);
	if (rc < 0) {
		static uint32_t nrd_fail = 0;
		nrd_fail++;
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

	/* HW-cache starts inverted so first push always goes through. */
	data->hw_short_addr = 0;
	data->hw_pan_id     = 0;
	memset(data->hw_mac_addr, 0xFF, sizeof(data->hw_mac_addr));
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

/* Register as a real network device.  L2 selection mirrors upstream nrf5:
 * with CONFIG_NET_L2_OPENTHREAD=y (Matter/Thread) we MUST bind to
 * OPENTHREAD_L2 so the OT L2 handler marks the iface NET_IF_UP.  Binding
 * to IEEE802154_L2 instead leaves the iface DOWN and every RX gets
 * silently dropped by net_recv_data() with -ENETDOWN (-115). */
#if defined(CONFIG_NET_L2_OPENTHREAD)
#define RT583_L2        OPENTHREAD_L2
#define RT583_L2_CTX    NET_L2_GET_CTX_TYPE(OPENTHREAD_L2)
#define RT583_MTU       1280
#elif defined(CONFIG_NET_L2_IEEE802154)
#define RT583_L2        IEEE802154_L2
#define RT583_L2_CTX    NET_L2_GET_CTX_TYPE(IEEE802154_L2)
#define RT583_MTU       125
#elif defined(CONFIG_NET_L2_CUSTOM_IEEE802154)
#define RT583_L2        CUSTOM_IEEE802154_L2
#define RT583_L2_CTX    NET_L2_GET_CTX_TYPE(CUSTOM_IEEE802154_L2)
#define RT583_MTU       CONFIG_NET_L2_CUSTOM_IEEE802154_MTU
#else
#error "No Zephyr L2 selected for the RT583 ieee802154 driver"
#endif

#define IEEE802154_RAFAEL_INIT(n) \
	NET_DEVICE_DT_INST_DEFINE(n, \
				  rt583_radio_init, NULL, \
				  &rt583_radio_data_0, NULL, \
				  CONFIG_IEEE802154_RAFAEL_INIT_PRIORITY, \
				  &rt583_radio_api, \
				  RT583_L2, \
				  RT583_L2_CTX, \
				  RT583_MTU);

#else  /* !CONFIG_IEEE802154_RAFAEL_FULL — stub mode */

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

#define IEEE802154_RAFAEL_INIT(n) \
	DEVICE_DT_INST_DEFINE(n, \
			      ieee802154_rt583_init, \
			      NULL, \
			      NULL, NULL, \
			      POST_KERNEL, \
			      CONFIG_IEEE802154_RAFAEL_INIT_PRIORITY, \
			      &ieee802154_rt583_api);

#endif  /* CONFIG_IEEE802154_RAFAEL_FULL */

DT_INST_FOREACH_STATUS_OKAY(IEEE802154_RAFAEL_INIT)
