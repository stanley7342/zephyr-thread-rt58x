/*
 * rt584_compat.c — RT584 OpenThread compatibility stubs.
 *
 * Vendor sources for rt584 either reference a different SDK API than
 * rt583 or pull in FreeRTOS-only deps that don't compile under Zephyr.
 * Until Phase B (RF MCU bring-up + crypto verification on real
 * hardware), stub the missing symbols so the link succeeds.
 *
 * 1. aes_fw_init — RT583's rt_aes.c exposes this to load AES microcode.
 *    RT584's rt_aes.c does not have it; the IP block boots differently.
 *    aes_alt.c (port) calls it unconditionally → stub as no-op.
 *
 * 2. Tx_Power_Compensation_* — implemented in rt569-rf/rt584/Src/rf_tx_comp.c
 *    which #includes FreeRTOS.h + timers.h (soft timers). Porting
 *    rf_tx_comp.c to Zephyr is Phase B work; stub the entry points so
 *    rf_common_init.c links. Without TX-power compensation the radio
 *    runs at default Tx power across temperature — acceptable for
 *    bring-up, not for production.
 */

void aes_fw_init(void) { }

void Tx_Power_Compensation_Init(void) { }
void Tx_Power_Compensation_Sadc_Int_Handler(void) { }
