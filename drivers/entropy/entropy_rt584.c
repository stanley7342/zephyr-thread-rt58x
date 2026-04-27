/*
 * Copyright (c) 2025 Rafael Microelectronics
 * SPDX-License-Identifier: Apache-2.0
 *
 * Zephyr entropy driver for the RT584 hardware TRNG.
 * Wraps hosal_trng_get_random_number() to provide the Zephyr entropy API.
 * Selects ENTROPY_HAS_DRIVER (via Kconfig) so that mbedTLS PSA can seed its
 * DRBG and Matter's psa_crypto_init() succeeds.
 */

#include <zephyr/drivers/entropy.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <string.h>
#include "hosal_trng.h"

#define DT_DRV_COMPAT rafael_rt584_trng

static int entropy_rt584_get_entropy(const struct device *dev,
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

static const struct entropy_driver_api entropy_rt584_api = {
	.get_entropy = entropy_rt584_get_entropy,
};

static int entropy_rt584_init(const struct device *dev)
{
	ARG_UNUSED(dev);
	return 0;
}

#define ENTROPY_RT584_INIT(n)						\
	DEVICE_DT_INST_DEFINE(n,					\
			      entropy_rt584_init,			\
			      NULL,					\
			      NULL, NULL,				\
			      PRE_KERNEL_1,				\
			      CONFIG_ENTROPY_INIT_PRIORITY,		\
			      &entropy_rt584_api);

DT_INST_FOREACH_STATUS_OKAY(ENTROPY_RT584_INIT)
