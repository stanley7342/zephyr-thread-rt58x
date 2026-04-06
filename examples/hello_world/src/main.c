#include <zephyr/kernel.h>

int main(void)
{
	printk("Hello World! RT582-EVB\n");

	while (1) {
		k_msleep(1000);
		printk("Hello World!\n");
	}

	return 0;
}
