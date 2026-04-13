/*
 * RT583 Matter Lighting App — Application task.
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
#include <system/SystemLayer.h>

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

    /* Button factory reset (GPIO0 = factory reset hold 6 s, GPIO1 = light toggle) */
    static void ButtonEventHandler(uint32_t pin, void * isr_param);
    static void FunctionHandler(const AppEvent & event);
    static void FunctionTimerEventHandler(const AppEvent & event);
    static void TimerEventHandler(chip::System::Layer * layer, void * appState);
    void StartTimer(uint32_t timeoutMs);
    void CancelTimer();

    /* Simulated LED — replace with real GPIO driver when hardware is wired */
    static void SetLed(bool on, uint8_t level);

    bool          mLightOn             = false;
    uint8_t       mLightLevel          = 254; /* 0–254 */
    FunctionEvent mFunction            = FunctionEvent::NoneSelected;
    bool          mFunctionTimerActive = false;
};
