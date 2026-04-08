/*
 * RT583 Matter Lighting App — Application task.
 *
 * Initialises the CHIP/Matter stack and drives the lighting state machine.
 * Hardware LED is currently simulated via printk — wire to a real GPIO
 * using Zephyr's gpio_pin_set() when the EVB LED pin is known.
 */

#include "AppTask.h"
#include "AppEvent.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

#ifdef CONFIG_NET_L2_OPENTHREAD
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/network-commissioning/CodegenInstance.h>
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/printk.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace {

constexpr EndpointId kLightEndpointId = 1;
constexpr int        kAppEventQueueSize = 10;

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));

Identify sIdentify = {
    kLightEndpointId,
    AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler,
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
};

#ifdef CONFIG_NET_L2_OPENTHREAD
app::Clusters::NetworkCommissioning::InstanceAndDriver<NetworkCommissioning::GenericThreadDriver>
    sThreadNetworkDriver(0 /* endpoint */);
#endif

} /* namespace */

/* ── LED simulation (replace with real GPIO when EVB LED pin is known) ─────── */

void AppTask::SetLed(bool on, uint8_t level)
{
    printk("[LED] %s  level=%u/254\n", on ? "ON " : "OFF", level);
    /* TODO: wire to GPIO
     * const struct device *led = DEVICE_DT_GET(DT_ALIAS(led0));
     * gpio_pin_set(led, PIN, on ? 1 : 0);
     */
}

/* ── Public API ─────────────────────────────────────────────────────────────── */

void AppTask::SetLightOn(bool on)
{
    mLightOn = on;
    SetLed(mLightOn, mLightLevel);
}

void AppTask::SetLightLevel(uint8_t level)
{
    mLightLevel = level;
    if (mLightOn) {
        SetLed(true, mLightLevel);
    }
}

void AppTask::UpdateClusterState()
{
    using namespace chip::app::Clusters;

    Protocols::InteractionModel::Status status;

    status = OnOff::Attributes::OnOff::Set(kLightEndpointId, mLightOn);
    if (status != Protocols::InteractionModel::Status::Success) {
        LOG_ERR("UpdateClusterState: OnOff write failed (%u)",
                chip::to_underlying(status));
    }

    status = LevelControl::Attributes::CurrentLevel::Set(kLightEndpointId, mLightLevel);
    if (status != Protocols::InteractionModel::Status::Success) {
        LOG_ERR("UpdateClusterState: Level write failed (%u)",
                chip::to_underlying(status));
    }
}

/* ── Identify cluster callbacks ─────────────────────────────────────────────── */

void AppTask::IdentifyStartHandler(Identify *)
{
    printk("[LED] Identify: blinking\n");
}

void AppTask::IdentifyStopHandler(Identify *)
{
    printk("[LED] Identify: stop\n");
    AppTask::Instance().SetLed(AppTask::Instance().IsLightOn(),
                               AppTask::Instance().GetLightLevel());
}

/* ── CHIP device event handler ──────────────────────────────────────────────── */

void AppTask::ChipEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    switch (event->Type) {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        LOG_INF("BLE advertising: %s",
                event->CHIPoBLEAdvertisingChange.Result == kActivity_Started
                    ? "started" : "stopped");
        break;
    case DeviceEventType::kThreadConnectivityChange:
        LOG_INF("Thread connectivity: %s",
                event->ThreadConnectivityChange.Result == kConnectivity_Established
                    ? "established" : "lost");
        break;
    case DeviceEventType::kInternetConnectivityChange:
        LOG_INF("Internet connectivity: %s",
                event->InternetConnectivityChange.IPv6 == kConnectivity_Established
                    ? "established" : "lost");
        break;
    case DeviceEventType::kCommissioningComplete:
        LOG_INF("Commissioning complete");
        break;
    default:
        break;
    }
}

/* ── Event dispatch ─────────────────────────────────────────────────────────── */

void AppTask::PostEvent(const AppEvent & event)
{
    if (k_msgq_put(&sAppEventQueue, &event, K_NO_WAIT) != 0) {
        LOG_ERR("PostEvent: queue full");
    }
}

void AppTask::DispatchEvent(const AppEvent & event)
{
    if (event.Handler) {
        event.Handler(event);
    }
}

void AppTask::LightingActionEventHandler(const AppEvent & event)
{
    AppTask & task = AppTask::Instance();

    switch (event.LightingEvent.Action) {
    case LightingAction::On:
        task.SetLightOn(true);
        break;
    case LightingAction::Off:
        task.SetLightOn(false);
        break;
    case LightingAction::Toggle:
        task.SetLightOn(!task.IsLightOn());
        break;
    case LightingAction::Level:
        task.SetLightLevel(event.LightingEvent.Level);
        break;
    }
}

/* ── Initialisation ─────────────────────────────────────────────────────────── */

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err;

    printk("[APP] MemoryInit...\n");
    /* Initialise CHIP memory allocator */
    err = chip::Platform::MemoryInit();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("MemoryInit failed: %" CHIP_ERROR_FORMAT, err.Format()));

    printk("[APP] InitChipStack...\n");
    /* Initialise CHIP device layer (BLE + OpenThread) */
    err = PlatformMgr().InitChipStack();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("InitChipStack failed: %" CHIP_ERROR_FORMAT, err.Format()));

    printk("[APP] AddEventHandler...\n");
    /* Register device event handler */
    err = PlatformMgr().AddEventHandler(ChipEventHandler, 0);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("AddEventHandler failed: %" CHIP_ERROR_FORMAT, err.Format()));

#ifdef CONFIG_NET_L2_OPENTHREAD
    printk("[APP] InitThreadStack...\n");
    err = ThreadStackMgr().InitThreadStack();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("InitThreadStack failed: %" CHIP_ERROR_FORMAT, err.Format()));

    printk("[APP] SetThreadDeviceType...\n");
    err = ConnectivityMgr().SetThreadDeviceType(
        ConnectivityManager::kThreadDeviceType_Router);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("SetThreadDeviceType failed: %" CHIP_ERROR_FORMAT, err.Format()));

    printk("[APP] ThreadNetworkDriver Init...\n");
    err = sThreadNetworkDriver.Init();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("ThreadNetworkDriver Init failed: %" CHIP_ERROR_FORMAT, err.Format()));
#endif

    printk("[APP] ServerInit...\n");
    /* Initialise Matter server (GATT, mDNS, session management) */
    static chip::CommonCaseDeviceServerInitParams serverParams;
    err = serverParams.InitializeStaticResourcesBeforeServerInit();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("ServerInitParams failed: %" CHIP_ERROR_FORMAT, err.Format()));

    /* Use example (test) DAC provider — replace with factory data for production */
    SetDeviceAttestationCredentialsProvider(
        Examples::GetExampleDACProvider());

    printk("[APP] Server::Init...\n");
    err = chip::Server::GetInstance().Init(serverParams);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("Server::Init failed: %" CHIP_ERROR_FORMAT, err.Format()));

    /* Set initial light state */
    SetLed(mLightOn, mLightLevel);
    UpdateClusterState();

    /* Print commissioning QR code and manual pairing code */
    PrintOnboardingCodes(chip::RendezvousInformationFlags(
        chip::RendezvousInformationFlag::kBLE));

    return CHIP_NO_ERROR;
}

/* ── Main event loop ────────────────────────────────────────────────────────── */

CHIP_ERROR AppTask::StartApp()
{
    printk("[APP] StartApp\n");
    CHIP_ERROR err = Init();
    if (err != CHIP_NO_ERROR) {
        LOG_ERR("AppTask::Init() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    /* Start CHIP device layer event loop (BLE + Thread tasks) */
    err = PlatformMgr().StartEventLoopTask();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("StartEventLoopTask failed: %" CHIP_ERROR_FORMAT, err.Format()));

#ifdef CONFIG_NET_L2_OPENTHREAD
    err = ThreadStackMgr().StartThreadTask();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("StartThreadTask failed: %" CHIP_ERROR_FORMAT, err.Format()));
#endif

    /* Main app event loop */
    AppEvent event;
    while (true) {
        k_msgq_get(&sAppEventQueue, &event, K_FOREVER);
        DispatchEvent(event);
    }

    return CHIP_NO_ERROR;
}
