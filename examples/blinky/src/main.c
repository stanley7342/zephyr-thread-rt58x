/*
 * RT583-EVB LED Blinky
 *
 * LED pin: GPIO22 (default — adjust to match your EVB schematic)
 */

#include <zephyr/kernel.h>
#include "gpio.h"

#define LED_PIN  22
#define BLINK_MS 500

int main(void)
{
	printk("RT583 Blinky — LED on GPIO%d\n", LED_PIN);

	gpio_cfg_output(LED_PIN);
	gpio_pin_write(LED_PIN, 0);

	while (1) {
		gpio_pin_toggle(LED_PIN);
		k_msleep(BLINK_MS);
	}

	return 0;
}
