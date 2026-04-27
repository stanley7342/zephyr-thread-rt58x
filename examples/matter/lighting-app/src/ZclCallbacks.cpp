/*
 * RT583 Matter Lighting App — ZCL cluster attribute change callbacks.
 *
 * Called by the Matter stack when a controller writes to the OnOff or
 * LevelControl cluster attributes.  Routes changes to AppTask which
 * applies them to the hardware LED.
 */

#include "AppTask.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>
#include <zephyr/sys/printk.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;

/* ── Attribute change callback ──────────────────────────────────────────────── */

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path,
                                       uint8_t type, uint16_t size, uint8_t * value)
{
    /* Use printk directly — ChipLogProgress(Zcl, ...) is compiled out when
     * Zcl module logging is disabled in this build, leaving the callback
     * effectively empty and silent. printk always prints regardless of
     * Matter log filter config. */
    printk("[ZCL] AttrChange ep=%u cluster=0x%04x attr=0x%04x type=%u size=%u val=%u\n",
           path.mEndpointId, (unsigned) path.mClusterId,
           (unsigned) path.mAttributeId, type, size,
           (size > 0 && value) ? *value : 0);

    if (path.mClusterId == OnOff::Id &&
        path.mAttributeId == OnOff::Attributes::OnOff::Id)
    {
        printk("[ZCL] OnOff -> %u\n", *value);
        AppTask::Instance().SetLightOn(*value != 0);
    }
    else if (path.mClusterId == LevelControl::Id &&
             path.mAttributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        printk("[ZCL] Level -> %u\n", *value);
        if (AppTask::Instance().IsLightOn()) {
            AppTask::Instance().SetLightLevel(*value);
        }
    }
}

/* ── OnOff cluster init callback ────────────────────────────────────────────── */

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    /* Restore persisted on/off state from settings */
    Protocols::InteractionModel::Status status;
    bool storedValue = false;

    status = Attributes::OnOff::Get(endpoint, &storedValue);
    if (status == Protocols::InteractionModel::Status::Success) {
        AppTask::Instance().SetLightOn(storedValue);
    }
}
