/*
 * Dummy net interface for Matter UDP socket bind.
 *
 * RT583's OpenThread radio uses vendored lmac15p4 (not a Zephyr ieee802154
 * driver), so Zephyr's NET_L2_OPENTHREAD layer has no net_if registered.
 * Without a net_if, socket bind(::, port) returns EADDRNOTAVAIL.
 *
 * This driver creates a minimal dummy Ethernet-type interface with an
 * auto-configured IPv6 link-local address so that Matter's Server::Init
 * can bind its UDP listening socket.  Actual IPv6 traffic goes through
 * OpenThread's native UDP layer once Thread is commissioned.
 */

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_l2.h>
#include <zephyr/net/dummy.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/sys/printk.h>

static int dummy_send(const struct device *dev, struct net_pkt *pkt)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(pkt);
    return -ENOTSUP;
}

static void dummy_iface_init(struct net_if *iface)
{
    /* Generate a stable link-local address from a fixed MAC.
     * The actual Thread IPv6 addresses are managed by OpenThread. */
    static uint8_t mac[6] = { 0x02, 0x00, 0x00, 0xFF, 0xFE, 0x01 };

    net_if_set_link_addr(iface, mac, sizeof(mac), NET_LINK_DUMMY);

    /* Flag the interface as up so socket bind() works. */
    net_if_flag_set(iface, NET_IF_UP);

    printk("[DUMMY-NET] iface %p initialised\n", iface);
}

static struct dummy_api dummy_api_funcs = {
    .iface_api.init = dummy_iface_init,
    .send = dummy_send,
};

/* Use DEVICE_DEFINE + NET_DEVICE_INIT pattern */
NET_DEVICE_INIT(dummy_net, "DUMMY_NET",
                NULL,       /* init_fn — no HW to init */
                NULL,       /* pm */
                NULL,       /* data */
                NULL,       /* config */
                CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                &dummy_api_funcs,
                DUMMY_L2,   /* L2 */
                NET_L2_GET_CTX_TYPE(DUMMY_L2),
                128);       /* MTU */
