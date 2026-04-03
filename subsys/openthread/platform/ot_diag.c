/*
 * ot_diag.c — OpenThread diagnostics stubs (no OS dependencies).
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <openthread/config.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/radio.h>

#if OPENTHREAD_CONFIG_DIAG_ENABLE

static bool sDiagMode = false;

void otPlatDiagModeSet(bool aMode)           { sDiagMode = aMode; }
bool otPlatDiagModeGet(void)                 { return sDiagMode; }
void otPlatDiagChannelSet(uint8_t aChannel)  { OT_UNUSED_VARIABLE(aChannel); }
void otPlatDiagTxPowerSet(int8_t aTxPower)  { OT_UNUSED_VARIABLE(aTxPower); }

void otPlatDiagRadioReceived(otInstance *aInstance, otRadioFrame *aFrame,
                              otError aError)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aFrame);
    OT_UNUSED_VARIABLE(aError);
}

void otPlatDiagAlarmCallback(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
}

#endif /* OPENTHREAD_CONFIG_DIAG_ENABLE */
