/*
 * psa-arch-tests runner — Zephyr on rt58x.
 *
 * Calls val_entry() (PSA test framework) which dispatches every test
 * registered via TEST_PUBLISH in the test_entry_list.inc file generated
 * at CMake configure time.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

extern int32_t val_entry(void);

int main(void)
{
    printk("\n=========================================\n");
    printk("  %s  psa-arch-tests runner\n", CONFIG_BOARD);
    printk("=========================================\n");

    int32_t status = val_entry();

    printk("\nval_entry() returned %d\n", (int)status);

    while (1) {
        k_msleep(60000);
    }
    return 0;
}
