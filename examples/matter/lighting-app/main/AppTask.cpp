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
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>
#include <psa/crypto.h>

extern "C" {
#include "hosal_rf.h"
#include "hosal_lpm.h"
extern bool hci_rt58x_rf_already_init;
void ot_radioInit(void);
void ot_alarmInit(void);
void ot_entropy_init(void);
void ot_set_instance(struct otInstance *inst);
void chip_heap_print_stats(void);
}

#ifdef CONFIG_NET_L2_OPENTHREAD
#include <zephyr/net/openthread.h>  /* openthread_init(), openthread_get_default_instance() */
#include <openthread/ip6.h>          /* otIp6SetEnabled */
#include <zephyr/net/net_if.h>       /* net_if_up, net_if_get_default */
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

    /* Initialize RF MCU in multi-protocol mode (BLE + 802.15.4/Thread).
     * Must be called once before InitChipStack() triggers either BLE or OT
     * radio init.  The BLE HCI driver (hci_rt58x_open) and OT radio layer
     * both require an initialized RF MCU; MULTI_PROTOCOL loads firmware that
     * supports both simultaneously. */
    {
        uintptr_t sp_before;
        __asm__ volatile("mov %0, sp" : "=r"(sp_before));
        printk("[STACK] before hosal_rf_init SP=0x%08x\n", (unsigned)sp_before);
    }
    printk("[APP] hosal_rf_init MULTI_PROTOCOL...\n");
    hosal_lpm_init();
    hosal_lpm_ioctrl(HOSAL_LPM_SET_POWER_LEVEL, HOSAL_LOW_POWER_LEVEL_SLEEP0);
    hosal_rf_init(HOSAL_RF_MODE_MULTI_PROTOCOL);
    {
        uintptr_t sp_after;
        __asm__ volatile("mov %0, sp" : "=r"(sp_after));
        printk("[STACK] after hosal_rf_init SP=0x%08x\n", (unsigned)sp_after);
    }
    /* IRQ 20 (COMM_SUBSYSTEM) is enabled inside hosal_rf_init → RfMcu_DmaInit
     * → NVIC_EnableIRQ AFTER SysRdySignalWait.  Do NOT enable it before
     * hosal_rf_init: gRfMcuIsrCfg.commsubsystem_isr is NULL until
     * rf_common_init_by_fw sets it, and a spurious IRQ with null isr_cb
     * corrupts RF MCU register state (RfMcu_SysRdySignalWait never exits). */
    k_sleep(K_MSEC(50));
    hci_rt58x_rf_already_init = true;  /* tell BLE HCI driver to skip re-init */
    printk("[APP] hosal_rf_init done\n");

    /* Initialise lmac15p4 radio layer for OpenThread (sets up RUCI callbacks,
     * channel, and MAC address from OTP/flash).  Must be called after
     * hosal_rf_init() so the RF MCU RUCI interface is ready. */
    printk("[APP] ot_radioInit...\n");
    ot_radioInit();
    printk("[APP] ot_radioInit done\n");

    /* Initialise the hardware alarm timer (TIMER3) used for OT microsecond
     * alarms.  Called here so TIMER3 is configured before openthread_init()
     * starts the OT work queue and may schedule the first alarm. */
    printk("[APP] ot_alarmInit...\n");
    ot_alarmInit();
    printk("[APP] ot_alarmInit done\n");

    /* Warm up the TRNG before otInstanceInitSingle() calls otPlatEntropyGet().
     * hosal_trng_get_random_number() requires the TRNG clock to be running;
     * this call exercises it once so any first-access latency is absorbed here
     * rather than deep inside otInstanceInitSingle(). */
    printk("[APP] ot_entropy_init...\n");
    ot_entropy_init();
    printk("[APP] ot_entropy_init done\n");

#ifdef CONFIG_NET_L2_OPENTHREAD
    /* Explicitly initialise the OT instance on this thread (8 KB stack).
     * openthread_init() calls otInstanceInitSingle() which needs ~4 KB of
     * stack — far more than the 2 KB main thread used during POST_KERNEL
     * device init.  After this call openthread_get_default_instance() returns
     * a valid pointer and InitThreadStack() no longer asserts. */
    printk("[APP] openthread_init...\n");
    {
        int ot_err = openthread_init();
        printk("[APP] openthread_init done, err=%d\n", ot_err);
    }
    /* Sync our ot_instance accessor (used by ot_alarmTask / ot_radioTask)
     * with Zephyr's openthread_instance set by openthread_init(). */
    ot_set_instance(openthread_get_default_instance());
    printk("[APP] ot_set_instance done\n");
#endif

    /* PSA entropy diagnostic — test before InitChipStack */
    {
        psa_status_t s1 = psa_crypto_init();
        printk("[PSA] psa_crypto_init status=%d\n", (int)s1);
        uint8_t rbuf[4] = {0};
        psa_status_t s2 = psa_generate_random(rbuf, sizeof(rbuf));
        printk("[PSA] psa_generate_random status=%d bytes=%02x%02x%02x%02x\n",
               (int)s2, rbuf[0], rbuf[1], rbuf[2], rbuf[3]);
        int cs = sys_csrand_get(rbuf, sizeof(rbuf));
        printk("[CSRAND] sys_csrand_get ret=%d\n", cs);
    }

    printk("[APP] MemoryInit...\n");
    /* Initialise CHIP memory allocator */
    err = chip::Platform::MemoryInit();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                        LOG_ERR("MemoryInit failed: %" CHIP_ERROR_FORMAT, err.Format()));

    printk("[APP] InitChipStack — enter...\n");
    /* Initialise CHIP device layer (BLE + OpenThread) */
    err = PlatformMgr().InitChipStack();
    printk("[APP] InitChipStack — returned err=%d\n", err.AsInteger());
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
    printk("[APP] InitThreadStack returned err=%d\n", err.AsInteger());
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

#ifdef CONFIG_NET_L2_OPENTHREAD
    /* Bring up the OT IPv6 interface so that Server::Init can bind UDP
     * sockets.  Thread itself stays idle (MANUAL_START) — commissioning
     * via BLE will provision and start Thread later.  Without this,
     * bind(::, 5540) returns EADDRNOTAVAIL because no interface is up. */
    {
        openthread_mutex_lock();
        otIp6SetEnabled(openthread_get_default_instance(), true);
        openthread_mutex_unlock();

        /* Bring Zephyr net interfaces up so bind() succeeds. */
        for (int idx = 1; idx <= 4; idx++) {
            struct net_if *iface = net_if_get_by_index(idx);
            if (iface) {
                int r = net_if_up(iface);
                printk("[APP] net_if_up(idx=%d %p) = %d\n", idx, iface, r);
            }
        }
        printk("[APP] net_if default=%p\n", net_if_get_default());
        /* Give the stack a moment to assign the link-local address. */
        k_sleep(K_MSEC(200));
    }
#endif

    printk("[APP] ServerInit...\n");
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

    /* Stack watermark after full Init() — measures actual peak stack depth. */
    {
        uintptr_t sp;
        __asm__ volatile("mov %0, sp" : "=r"(sp));
        extern char app_stack[];
        uintptr_t base = (uintptr_t)app_stack;
        uintptr_t top  = base + 8192; /* matches APP_STACK_SIZE in main.cpp */
        printk("[STACK] after Init SP=0x%08x used=%u free=%u\n",
               (unsigned)sp, (unsigned)(top - sp), (unsigned)(sp - base));
    }

    return CHIP_NO_ERROR;
}

/* Called from the CHIP event loop thread via ScheduleWork — holds the
 * stack lock and can safely post platform events. */
static void ServerInitWork(intptr_t arg)
{
    auto * serverParams = reinterpret_cast<chip::CommonCaseDeviceServerInitParams *>(arg);

    {
        uintptr_t sp;
        __asm__ volatile("mov %0, sp" : "=r"(sp));
        printk("[CHIP-TASK] ServerInitWork SP=0x%08x\n", (unsigned)sp);
    }
    chip_heap_print_stats();
    printk("[APP] Server::Init (from event loop)...\n");
    CHIP_ERROR err = chip::Server::GetInstance().Init(*serverParams);
    chip_heap_print_stats();
    printk("[APP] Server::Init returned err=%d\n", err.AsInteger());
    if (err != CHIP_NO_ERROR) {
        printk("[APP] Server::Init FAILED\n");
        return;
    }

    /* Set initial light state */
    AppTask & task = AppTask::Instance();
    printk("[APP] SetLightOn...\n");
    task.SetLightOn(task.IsLightOn());
    printk("[APP] UpdateClusterState...\n");
    task.UpdateClusterState();
    printk("[APP] UpdateClusterState done\n");

    /* Print commissioning QR code and manual pairing code */
    printk("[APP] PrintOnboardingCodes...\n");
    PrintOnboardingCodes(chip::RendezvousInformationFlags(
        chip::RendezvousInformationFlag::kBLE));
    printk("[APP] PrintOnboardingCodes done\n");
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

    /* Start CHIP device layer event loop BEFORE Server::Init.
     * Server::Init → emberAfInit → cluster callbacks may call PostEventOrDie
     * which requires the event loop to be running. */
    printk("[APP] StartEventLoopTask...\n");
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
