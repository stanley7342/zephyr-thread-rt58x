/*
 * Copyright (c) 2025 Rafael Microelectronics
 * SPDX-License-Identifier: Apache-2.0
 *
 * Zephyr entropy driver for the RT583 hardware TRNG.
 * Wraps hosal_trng_get_random_number() to provide the Zephyr entropy API.
 * This sets CONFIG_ENTROPY_HAS_DRIVER=y (via Kconfig 'select') so that
 * CONFIG_CSPRNG_ENABLED becomes true and Zephyr's OT entropy.c compiles.
 */

#include <zephyr/drivers/entropy.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include "hosal_trng.h"

#define DT_DRV_COMPAT rafael_rt583_trng

static int entropy_rt583_get_entropy(const struct device *dev,
				     uint8_t *buf, uint16_t len)
{
	ARG_UNUSED(dev);

	uint32_t tmp;
	uint16_t remaining = len;
	uint8_t *p = buf;

	while (remaining > 0) {
		if (hosal_trng_get_random_number(&tmp, 1) != 0) {
			return -EIO;
		}
		uint16_t chunk = MIN(remaining, sizeof(tmp));

		memcpy(p, &tmp, chunk);
		p += chunk;
		remaining -= chunk;
	}
	return 0;
}

static const struct entropy_driver_api entropy_rt583_api = {
	.get_entropy = entropy_rt583_get_entropy,
};

static int entropy_rt583_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

#define ENTROPY_RT583_INIT(n)						\
	DEVICE_DT_INST_DEFINE(n,					\
			      entropy_rt583_init,			\
			      NULL,					\
			      NULL, NULL,				\
			      PRE_KERNEL_1,				\
			      CONFIG_ENTROPY_INIT_PRIORITY,		\
			      &entropy_rt583_api);

DT_INST_FOREACH_STATUS_OKAY(ENTROPY_RT583_INIT)
