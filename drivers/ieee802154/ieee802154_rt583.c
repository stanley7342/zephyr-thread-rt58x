/*
 * Copyright (c) 2025 Rafael Microelectronics
 * SPDX-License-Identifier: Apache-2.0
 *
 * Stub IEEE 802.15.4 device driver for the RT583.
 * This driver exists solely to provide a valid DEVICE_DT_DEFINE so that
 * Zephyr's openthread/platform/radio.c can compile with:
 *   static const struct device *const radio_dev =
 *       DEVICE_DT_GET(DT_CHOSEN(zephyr_ieee802154));
 *
 * The actual 802.15.4 radio access is handled by subsys/openthread/platform/
 * ot_radio.c via the lmac15p4 library — those functions override radio.c's
 * weak implementations at link time via --allow-multiple-definition.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/net/ieee802154_radio.h>
#include <zephyr/init.h>

#define DT_DRV_COMPAT rafael_rt583_ieee802154

static int ieee802154_rt583_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	/* Actual radio init is done by ot_radio.c / lmac15p4. */
	return 0;
}

static enum ieee802154_hw_caps ieee802154_rt583_get_capabilities(
	const struct device *dev)
{
	return 0;
}

static int ieee802154_rt583_cca(const struct device *dev)
{
	return -ENOTSUP;
}

static int ieee802154_rt583_set_channel(const struct device *dev,
					uint16_t channel)
{
	return -ENOTSUP;
}

static int ieee802154_rt583_filter(const struct device *dev, bool set,
				   enum ieee802154_filter_type type,
				   const struct ieee802154_filter *filter)
{
	return -ENOTSUP;
}

static int ieee802154_rt583_set_txpower(const struct device *dev, int16_t dbm)
{
	return -ENOTSUP;
}

static int ieee802154_rt583_tx(const struct device *dev,
			       enum ieee802154_tx_mode mode,
			       struct net_pkt *pkt,
			       struct net_buf *frag)
{
	return -ENOTSUP;
}

static int ieee802154_rt583_start(const struct device *dev)
{
	return -ENOTSUP;
}

static int ieee802154_rt583_stop(const struct device *dev)
{
	return -ENOTSUP;
}

static struct ieee802154_radio_api ieee802154_rt583_api = {
	.iface_api.init = NULL,
	.get_capabilities = ieee802154_rt583_get_capabilities,
	.cca              = ieee802154_rt583_cca,
	.set_channel      = ieee802154_rt583_set_channel,
	.filter           = ieee802154_rt583_filter,
	.set_txpower      = ieee802154_rt583_set_txpower,
	.tx               = ieee802154_rt583_tx,
	.start            = ieee802154_rt583_start,
	.stop             = ieee802154_rt583_stop,
};

#define IEEE802154_RT583_INIT(n)					\
	DEVICE_DT_INST_DEFINE(n,					\
			      ieee802154_rt583_init,			\
			      NULL,					\
			      NULL, NULL,				\
			      POST_KERNEL,				\
			      CONFIG_IEEE802154_RT583_INIT_PRIORITY,	\
			      &ieee802154_rt583_api);

DT_INST_FOREACH_STATUS_OKAY(IEEE802154_RT583_INIT)
