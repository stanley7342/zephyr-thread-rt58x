/*
 * RT583 Matter Lighting App — Application task.
 *
 * Initialises the CHIP/Matter stack and drives the lighting state machine.
 * Hardware LED is currently a stub — wire to a real GPIO
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
#include <platform/Zephyr/DeviceInstanceInfoProviderImpl.h>
#include <DeviceInfoProviderImpl.h>
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

extern "C" {
#include "hosal_rf.h"
#include "hosal_lpm.h"
extern bool hci_rt58x_rf_already_init;
void ot_radioInit(void);
void ot_alarmInit(void);
void ot_entropy_init(void);
void ot_set_instance(struct otInstance *inst);
}

#ifdef CONFIG_NET_L2_OPENTHREAD
#include <zephyr/net/openthread.h>  /* openthread_init(), openthread_get_default_instance() */
#include <openthread/ip6.h>          /* otIp6SetEnabled */
#include <openthread/thread.h>       /* OT_DEVICE_ROLE_LEADER */
#include <zephyr/net/net_if.h>       /* net_if_up, net_if_get_default */
#endif
#ifdef CONFIG_OPENTHREAD_SRP_SERVER
#include <openthread/srp_server.h>   /* otSrpServerSetEnabled */
#endif

#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
#include <inet/EndPointStateOpenThread.h>
#include <platform/Zephyr/ThreadStackManagerImpl.h>  /* ThreadStackMgrImpl().OTInstance() */
#endif

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

/* File-scope pointer to the serverParams so ServerInitWork can access it. */
chip::CommonCaseDeviceServerInitParams * gServerInitParams = nullptr;

/* ── LED simulation (replace with real GPIO when EVB LED pin is known) ─────── */

void AppTask::SetLed(bool on, uint8_t level)
{
    /* TODO: wire to GPIO
     * const struct device *led = DEVICE_DT_GET(DT_ALIAS(led0));
     * gpio_pin_set(led, PIN, on ? 1 : 0);
     */
    ARG_UNUSED(on);
    ARG_UNUSED(level);
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
}

void AppTask::IdentifyStopHandler(Identify *)
{
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
    case DeviceEventType::kThreadStateChange:
        if (event->ThreadStateChange.RoleChanged) {
#ifdef CONFIG_NET_L2_OPENTHREAD
            otInstance * inst = openthread_get_default_instance();
            otDeviceRole role = inst ? otThreadGetDeviceRole(inst) : OT_DEVICE_ROLE_DISABLED;
            static const char * const kRoleStr[] = {
                "Disabled", "Detached", "Child", "Router", "Leader"
            };
            const char *roleStr = (role < (int)ARRAY_SIZE(kRoleStr)) ? kRoleStr[role] : "Unknown";
            printk("[OT] Role changed → %s\n", roleStr);
#ifdef CONFIG_OPENTHREAD_SRP_SERVER
            /* Enable the built-in SRP server when this device is the Thread
             * Leader (standalone — no Border Router).  The SRP server
             * publishes its anycast address via NETDATA_PUBLISHER, the SRP
             * client auto-starts and registers services, and the Matter DNS-SD
             * init callback fires → mState=kInitialized → advertising works.
             * When an OTBR is present the device is not Leader; the SRP server
             * stays off and the OTBR provides the SRP server instead. */
            if (inst) {
                openthread_mutex_lock();
                otSrpServerSetEnabled(inst, role == OT_DEVICE_ROLE_LEADER);
                openthread_mutex_unlock();
            }
#endif
#endif
        }
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

    /* ── Firmware version marker ────────────────────────────────────────────
     * Confirms new firmware is running and AppTask::Init() is entered.
     * If this line is absent from the log the object file was not rebuilt. */
    printk("[DIAG] AppTask::Init entered (build " __DATE__ " " __TIME__ ")\n");

    /* Initialize RF MCU in multi-protocol mode (BLE + 802.15.4/Thread).
     * Must be called once before InitChipStack() triggers either BLE or OT
     * radio init.  The BLE HCI driver (hci_rt58x_open) and OT radio layer
     * both require an initialized RF MCU; MULTI_PROTOCOL loads firmware that
     * supports both simultaneously. */
    hosal_lpm_init();
    hosal_lpm_ioctrl(HOSAL_LPM_SET_POWER_LEVEL, HOSAL_LOW_POWER_LEVEL_SLEEP0);
    hosal_rf_init(HOSAL_RF_MODE_MULTI_PROTOCOL);
    /* IRQ 20 (COMM_SUBSYSTEM) is enabled inside hosal_rf_init → RfMcu_DmaInit
     * → NVIC_EnableIRQ AFTER SysRdySignalWait.  Do NOT enable it before
     * hosal_rf_init: gRfMcuIsrCfg.commsubsystem_isr is NULL until
     * rf_common_init_by_fw sets it, and a spurious IRQ with null isr_cb
     * corrupts RF MCU register state (RfMcu_SysRdySignalWait never exits). */
    k_sleep(K_MSEC(50));
    hci_rt58x_rf_already_init = true;  /* tell BLE HCI driver to skip re-init */

    /* Initialise lmac15p4 radio layer for OpenThread (sets up RUCI callbacks,
     * channel, and MAC address from OTP/flash).  Must be called after
     * hosal_rf_init() so the RF MCU RUCI interface is ready. */
    ot_radioInit();

    /* Initialise the hardware alarm timer (TIMER3) used for OT microsecond
     * alarms.  Called here so TIMER3 is configured before openthread_init()
     * starts the OT work queue and may schedule the first alarm. */
    ot_alarmInit();

    /* Warm up the TRNG before otInstanceInitSingle() calls otPlatEntropyGet().
     * hosal_trng_get_random_number() requires the TRNG clock to be running;
     * this call exercises it once so any first-access latency is absorbed here
     * rather than deep inside otInstanceInitSingle(). */
    ot_entropy_init();

#ifdef CONFIG_NET_L2_OPENTHREAD
    /* Explicitly initialise the OT instance on this thread (8 KB stack).
     * openthread_init() calls otInstanceInitSingle() which needs ~4 KB of
     * stack — far more than the 2 KB main thread used during POST_KERNEL
     * device init.  After this call openthread_get_default_instance() returns
     * a valid pointer and InitThreadStack() no longer asserts. */
    {
        int ot_err = openthread_init();
        if (ot_err != 0) {
            LOG_ERR("openthread_init failed: %d", ot_err);
        }
    }
    /* Sync our ot_instance accessor (used by ot_alarmTask / ot_radioTask)
     * with Zephyr's openthread_instance set by openthread_init(). */
    ot_set_instance(openthread_get_default_instance());
#endif

    /* Initialise CHIP memory allocator */
    err = chip::Platform::MemoryInit();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("MemoryInit failed: %" CHIP_ERROR_FORMAT, err.Format()));

    /* Initialise CHIP device layer (BLE + OpenThread) */
    err = PlatformMgr().InitChipStack();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("InitChipStack failed: %" CHIP_ERROR_FORMAT, err.Format()));

    /* Register device event handler */
    err = PlatformMgr().AddEventHandler(ChipEventHandler, 0);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("AddEventHandler failed: %" CHIP_ERROR_FORMAT, err.Format()));

#ifdef CONFIG_NET_L2_OPENTHREAD
    err = ThreadStackMgr().InitThreadStack();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("InitThreadStack failed: %" CHIP_ERROR_FORMAT, err.Format()));

    err = ConnectivityMgr().SetThreadDeviceType(
        ConnectivityManager::kThreadDeviceType_Router);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("SetThreadDeviceType failed: %" CHIP_ERROR_FORMAT, err.Format()));

    err = sThreadNetworkDriver.Init();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("ThreadNetworkDriver Init failed: %" CHIP_ERROR_FORMAT, err.Format()));
#endif

#ifdef CONFIG_NET_L2_OPENTHREAD
    /* Enable OT IPv6 so that OT UDP endpoints (mDNS, CASE) work.
     * With CONFIG_CHIP_USE_OT_ENDPOINT=y, Matter uses otUdp* directly —
     * no POSIX socket bind() needed, so the net_if_up() workaround is gone.
     * otIp6SetEnabled must still be called here because InitThreadStack()
     * only enables IPv6 under THREAD_AUTOSTART when the dataset is already
     * commissioned, which is not the case on first boot. */
    openthread_mutex_lock();
    otIp6SetEnabled(openthread_get_default_instance(), true);
    openthread_mutex_unlock();
#endif

    /* Initialise Matter server (GATT, mDNS, session management) */
    static chip::CommonCaseDeviceServerInitParams sServerInitParams;
    gServerInitParams = &sServerInitParams;
    err = sServerInitParams.InitializeStaticResourcesBeforeServerInit();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("ServerInitParams failed: %" CHIP_ERROR_FORMAT, err.Format()));

    /* Use example (test) DAC provider — replace with factory data for production */
    SetDeviceAttestationCredentialsProvider(
        Examples::GetExampleDACProvider());

    /* DeviceInstanceInfoProvider — required by BasicInformation cluster init.
     * Must be set before Server::Init which triggers SetDataModelProvider. */
    static DeviceLayer::DeviceInstanceInfoProviderImpl sDeviceInstanceInfoProvider(
        DeviceLayer::ConfigurationManagerImpl::GetDefaultInstance());
    DeviceLayer::SetDeviceInstanceInfoProvider(&sDeviceInstanceInfoProvider);

    /* DeviceInfoProvider — required by UserLabel cluster init. */
    static DeviceLayer::DeviceInfoProviderImpl sDeviceInfoProvider;
    sDeviceInfoProvider.SetStorageDelegate(sServerInitParams.persistentStorageDelegate);
    DeviceLayer::SetDeviceInfoProvider(&sDeviceInfoProvider);

    /* Data model provider — required since connectedhomeip removed the default. */
    sServerInitParams.dataModelProvider =
        chip::app::CodegenDataModelProviderInstance(sServerInitParams.persistentStorageDelegate);

    return CHIP_NO_ERROR;
}

/* Called from the CHIP event loop thread via ScheduleWork — holds the
 * stack lock and can safely post platform events. */
static void ServerInitWork(intptr_t arg)
{
    auto * serverParams = reinterpret_cast<chip::CommonCaseDeviceServerInitParams *>(arg);

#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
    /* Wire OpenThread's native UDP stack into Matter's inet layer.
     * With CONFIG_CHIP_USE_OT_ENDPOINT=y, all UDP endpoints (CASE, mDNS)
     * use otUdp* directly — no POSIX socket bind() required.  This fixes
     * the "Failed to advertise commissionable node: 3" (CHIP_ERROR_INCORRECT_STATE)
     * that occurred because Advertiser_ImplMinimalMdns::Init() couldn't bind
     * a POSIX socket (EADDRNOTAVAIL: no Zephyr net_if for the vendored lmac15p4
     * Thread radio).  With a valid OT instance here, Init() uses otUdpBind()
     * on the Thread interface and succeeds. */
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb   = []() { chip::DeviceLayer::ThreadStackMgr().LockThreadStack(); };
    nativeParams.unlockCb = []() { chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack(); };
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    serverParams->endpointNativeParams = static_cast<void *>(&nativeParams);
#endif

    uint32_t t0 = k_uptime_get_32();
    CHIP_ERROR err = chip::Server::GetInstance().Init(*serverParams);
    LOG_INF("[APP] Server::Init took %u ms", k_uptime_get_32() - t0);
    if (err != CHIP_NO_ERROR) {
        LOG_ERR("Server::Init failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

#if defined(MATTER_FACTORY_RESET_ON_BOOT)
    /* One-shot factory reset: clears all Matter KVS data + OT NVM, then reboots.
     * Build with -DMATTER_FACTORY_RESET_ON_BOOT=1 (CMake, not Kconfig) to wipe
     * the device once; rebuild without it for normal operation. */
    printk("[DIAG] MATTER_FACTORY_RESET_ON_BOOT set — initiating factory reset\n");
    chip::Server::GetInstance().ScheduleFactoryReset();
    return;
#endif

    /* Set initial light state */
    AppTask & task = AppTask::Instance();
    task.SetLightOn(task.IsLightOn());
    task.UpdateClusterState();

    /* Print commissioning QR code URL and manual pairing code */
    PrintOnboardingCodes(chip::RendezvousInformationFlags(
        chip::RendezvousInformationFlag::kBLE));
}

/* ── Main event loop ────────────────────────────────────────────────────────── */

CHIP_ERROR AppTask::StartApp()
{
    CHIP_ERROR err = Init();
    if (err != CHIP_NO_ERROR) {
        LOG_ERR("AppTask::Init() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    /* Start CHIP device layer event loop BEFORE Server::Init.
     * Server::Init → emberAfInit → cluster callbacks may call PostEventOrDie
     * which requires the event loop to be running. */
    err = PlatformMgr().StartEventLoopTask();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("StartEventLoopTask failed: %" CHIP_ERROR_FORMAT, err.Format()));

    /* Schedule Server::Init on the CHIP event loop thread so it holds the
     * stack lock and can dispatch platform events.  The serverParams static
     * in Init() outlives this call. */
    {
        /* Get pointer to the static serverParams in Init() — it's the only
         * CommonCaseDeviceServerInitParams we created. */
        extern chip::CommonCaseDeviceServerInitParams * gServerInitParams;
        err = PlatformMgr().ScheduleWork(ServerInitWork,
                                         reinterpret_cast<intptr_t>(gServerInitParams));
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            LOG_ERR("ScheduleWork failed: %" CHIP_ERROR_FORMAT, err.Format()));
    }

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
