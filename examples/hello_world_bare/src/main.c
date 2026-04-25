#include <zephyr/kernel.h>

int main(void)
{
    printk("Hello World! RT584-EVB BARE\n");
    while (1) {
        k_msleep(1000);
        printk("tick\n");
    }
    return 0;
}
