#include <zephyr/kernel.h>

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	while (1) {
		k_msleep(1000);
		printk("Hello World!\n");
	}

	return 0;
}
