/*
 * RT582 Matter Lighting App — Application task.
 *
 * Manages:
 *  - Matter stack initialisation (BLE advertising, OpenThread, Server)
 *  - Lighting state (on/off + level)
 *  - ZCL → hardware callbacks (via UpdateClusterState / LightingAction)
 */

#pragma once

#include "AppEvent.h"

#include <app/clusters/identify-server/identify-server.h>
#include <platform/CHIPDeviceLayer.h>

#include <cstdint>

struct k_msgq;

class AppTask
{
public:
    static AppTask & Instance()
    {
        static AppTask sAppTask;
        return sAppTask;
    }

    CHIP_ERROR StartApp();

    /* Called from ZclCallbacks when ZCL cluster attributes change */
    void SetLightOn(bool on);
    void SetLightLevel(uint8_t level);
    bool IsLightOn() const { return mLightOn; }
    uint8_t GetLightLevel() const { return mLightLevel; }

    /* Update CHIP cluster attributes to match current state */
    void UpdateClusterState();

    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);

private:
    CHIP_ERROR Init();

    static void PostEvent(const AppEvent & event);
    static void DispatchEvent(const AppEvent & event);
    static void LightingActionEventHandler(const AppEvent & event);
    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event,
                                 intptr_t arg);

    /* Simulated LED — replace with real GPIO driver when hardware is wired */
    static void SetLed(bool on, uint8_t level);

    bool    mLightOn    = false;
    uint8_t mLightLevel = 254; /* 0–254 */
};
