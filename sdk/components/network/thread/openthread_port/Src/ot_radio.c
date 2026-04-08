/*
 * ot_radio.c — OpenThread radio platform for RT583 on Zephyr
 *
 * Ported from Rafael SDK openthread_port/src/ot_radio.c.
 * Changes from the original:
 *   - Removed #include "common/logging.hpp" (C++ OT internal header)
 *   - Removed Rafael shell CLI command registration (STATIC_CLI_CMD_ATTRIBUTE)
 *   - Replaced vTaskDelay(5) → k_sleep(K_MSEC(5))
 *   - log_info/warn/error → printk
 *   - OT_ENTER_CRITICAL/OT_EXIT_CRITICAL now use irq_lock via openthread_port.h
 *   - All lmac15p4_* calls kept as-is (binary library, no OS deps)
 *   - EUI-64 now derived from flash unique ID (was hardcoded)
 */

#include <string.h>
#include <assert.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <openthread/config.h>
#include <openthread/link.h>
#include <openthread/platform/alarm-micro.h>
#include <openthread/platform/alarm-milli.h>
#include <openthread/platform/diag.h>
#include <openthread/platform/radio.h>
#include <openthread/platform/time.h>

#include "openthread_port.h"
#include "code_utils.h"
#include "mac_frame.h"

#include "lmac15p4.h"
#include "util_list.h"
#include "flashctl.h"
#include "hosal_trng.h"
#include "tprintk.h"

/* ── Constants ───────────────────────────────────────────────────────────── */
#define CCA_THRESHOLD_DEFAULT       85
#define RAFAEL_RECEIVE_SENSITIVITY  101

#define PHY_PIB_TURNAROUND_TIMER    192
#define PHY_PIB_CCA_DETECTED_TIME   128
#define PHY_PIB_CCA_DETECT_MODE     0
#define PHY_PIB_CCA_THRESHOLD       CCA_THRESHOLD_DEFAULT
#define MAC_PIB_UNIT_BACKOFF_PERIOD 320
#define MAC_PIB_MAC_ACK_WAIT_DURATION 544
#define MAC_PIB_MAC_MAX_BE          8
#define MAC_PIB_MAC_MAX_FRAME_TOTAL_WAIT_TIME 16416
#define MAC_PIB_MAC_MAX_FRAME_RETRIES 4
#define MAC_PIB_MAC_MAX_CSMACA_BACKOFFS 10
#define MAC_PIB_MAC_MIN_BE          5

#define OTRADIO_MAC_HEADER_ACK_REQUEST_MASK (1 << 5)
#define OTRADIO_MAX_PSDU                    152
#define OTRADIO_RX_FRAME_BUFFER_NUM         8

/* ── Types ───────────────────────────────────────────────────────────────── */
typedef struct _otRadio_rxFrame_t {
    utils_dlist_t dlist;
    otRadioFrame  frame;
} otRadio_rxFrame_t;

#define ALIGNED_RX_FRAME_SIZE  ((sizeof(otRadio_rxFrame_t) + 3) & 0xfffffffcU)
#define TOTAL_RX_FRAME_SIZE    (ALIGNED_RX_FRAME_SIZE + OTRADIO_MAX_PSDU)

typedef struct _otRadio_t {
    otInstance    *aInstance;
    utils_dlist_t  rxFrameList;
    utils_dlist_t  frameList;
    otRadioFrame  *pTxFrame;
    otRadioFrame  *pAckFrame;

    uint64_t tstx;
    uint64_t tsIsr;

    uint32_t dbgRxFrameNum;
    uint32_t dbgFrameNum;
    uint32_t dbgMaxAckFrameLength;
    uint32_t dbgMaxPendingFrameNum;

    uint8_t buffPool[TOTAL_RX_FRAME_SIZE * (OTRADIO_RX_FRAME_BUFFER_NUM + 2)];
} otRadio_t;

/* ── Static state ────────────────────────────────────────────────────────── */
static otRadio_t otRadio_var;

static bool     sIsSrcMatchEnabled = false;
static int8_t   sCcaThresholdDbm   = CCA_THRESHOLD_DEFAULT;
static otExtAddress sExtAddress;

/* EUI-64: overwritten by ot_radioInit with OTP MAC or flash unique ID.
 * Fallback value used only if both OTP and flash UID are unavailable. */
static uint8_t  sIEEE_EUI64Addr[OT_EXT_ADDRESS_SIZE] = {
    0xAA, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

static uint16_t sTurnaroundTime = PHY_PIB_TURNAROUND_TIMER;
static uint8_t  sCCAMode        = PHY_PIB_CCA_DETECT_MODE;
static uint8_t  sCCAThreshold   = PHY_PIB_CCA_THRESHOLD;
static uint16_t sCCADuration    = PHY_PIB_CCA_DETECTED_TIME;

static bool     sPromiscuous   = false;
static uint16_t sShortAddress  = 0xFFFF;
static uint32_t sExtendAddr_0  = 0x01020304;
static uint32_t sExtendAddr_1  = 0x05060709;
static uint16_t sPANID         = 0xFFFF;
static uint8_t  sCoordinator   = 0;
static uint8_t  sCurrentChannel = 11; /* kMinChannel */
static otRadioState sState     = OT_RADIO_STATE_DISABLED;

static uint32_t sMacFrameCounter;
static uint8_t  sMacAddrReadMode = 1;
static bool     sAuto_State_Set  = false;

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
static uint8_t           sKeyId;
static otMacKeyMaterial  sPrevKey;
static otMacKeyMaterial  sCurrKey;
static otMacKeyMaterial  sNextKey;
static otRadioKeyType    sKeyType;
#endif

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
static uint32_t sCslSampleTime;
static uint32_t sCslPeriod;
#endif

/* ── Helpers ─────────────────────────────────────────────────────────────── */
enum { kMinChannel = 11, kMaxChannel = 26 };

static uint64_t longtime_to_longlong_time(uint32_t *prev32, uint32_t now32,
                                          uint32_t *wraps)
{
    if (now32 < *prev32) {
        (*wraps)++;
    }
    *prev32 = now32;
    return ((uint64_t)(*wraps) << 32) + now32;
}

static void ReverseExtAddress(otExtAddress *dst, const otExtAddress *src)
{
    for (size_t i = 0; i < sizeof(*dst); i++) {
        dst->m8[i] = src->m8[sizeof(*src) - 1 - i];
    }
}

/* ── Radio config getters ────────────────────────────────────────────────── */
uint32_t otPlatRadioGetBusSpeed(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return 2000000;
}

otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    otRadioCaps caps = OT_RADIO_CAPS_ACK_TIMEOUT
                     | OT_RADIO_CAPS_TRANSMIT_RETRIES
                     | OT_RADIO_CAPS_SLEEP_TO_TX
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
                     | OT_RADIO_CAPS_TRANSMIT_SEC
#endif
                     | OT_RADIO_CAPS_CSMA_BACKOFF;
    return caps;
}

otError otPlatRadioGetCcaEnergyDetectThreshold(otInstance *aInstance,
                                               int8_t *aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);
    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(aThreshold, error = OT_ERROR_INVALID_ARGS);
    *aThreshold = -(int8_t)sCcaThresholdDbm;
exit:
    return error;
}

otError otPlatRadioGetFemLnaGain(otInstance *aInstance, int8_t *aGain)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aGain);
    return OT_ERROR_NOT_IMPLEMENTED;
}

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64)
{
    OT_UNUSED_VARIABLE(aInstance);
    memcpy(aIeeeEui64, sIEEE_EUI64Addr, OT_EXT_ADDRESS_SIZE);
}

uint64_t otPlatRadioGetNow(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return otPlatTimeGet();
}

otRadioState otPlatRadioGetState(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return sState;
}

bool otPlatRadioGetPromiscuous(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return sPromiscuous;
}

int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return -(int8_t)RAFAEL_RECEIVE_SENSITIVITY;
}

otError otPlatRadioGetTransmitPower(otInstance *aInstance, int8_t *aPower)
{
    OT_UNUSED_VARIABLE(aInstance);
    otError error = OT_ERROR_NONE;
    otEXPECT_ACTION(aPower, error = OT_ERROR_INVALID_ARGS);
    *aPower = 10;
exit:
    return error;
}

/* ── Radio config setters ────────────────────────────────────────────────── */
otError otPlatRadioSetCcaEnergyDetectThreshold(otInstance *aInstance,
                                               int8_t aThreshold)
{
    OT_UNUSED_VARIABLE(aInstance);
    sCCAThreshold = (uint8_t)aThreshold;
    lmac15p4_phy_pib_set(sTurnaroundTime, sCCAMode, sCCAThreshold, sCCADuration);
    return OT_ERROR_NONE;
}

void otPlatRadioSetExtendedAddress(otInstance *aInstance,
                                   const otExtAddress *aAddress)
{
    OT_UNUSED_VARIABLE(aInstance);
    ReverseExtAddress(&sExtAddress, aAddress);
    sExtendAddr_0 = (uint32_t)aAddress->m8[0]
                  | ((uint32_t)aAddress->m8[1] << 8)
                  | ((uint32_t)aAddress->m8[2] << 16)
                  | ((uint32_t)aAddress->m8[3] << 24);
    sExtendAddr_1 = (uint32_t)aAddress->m8[4]
                  | ((uint32_t)aAddress->m8[5] << 8)
                  | ((uint32_t)aAddress->m8[6] << 16)
                  | ((uint32_t)aAddress->m8[7] << 24);
    lmac15p4_address_filter_set(0, sPromiscuous, sShortAddress,
                                sExtendAddr_0, sExtendAddr_1, sPANID,
                                sCoordinator);
}

otError otPlatRadioSetFemLnaGain(otInstance *aInstance, int8_t aGain)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aGain);
    return OT_ERROR_NOT_IMPLEMENTED;
}

void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanId)
{
    OT_UNUSED_VARIABLE(aInstance);
    sPANID = aPanId;
    lmac15p4_address_filter_set(0, sPromiscuous, sShortAddress,
                                sExtendAddr_0, sExtendAddr_1, sPANID,
                                sCoordinator);
}

void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    sPromiscuous = aEnable;
    lmac15p4_address_filter_set(0, sPromiscuous, sShortAddress,
                                sExtendAddr_0, sExtendAddr_1, sPANID,
                                sCoordinator);
}

void otPlatRadioSetShortAddress(otInstance *aInstance, uint16_t aAddress)
{
    OT_UNUSED_VARIABLE(aInstance);
    sShortAddress = aAddress;
    lmac15p4_address_filter_set(0, sPromiscuous, sShortAddress,
                                sExtendAddr_0, sExtendAddr_1, sPANID,
                                sCoordinator);
}

otError otPlatRadioSetTransmitPower(otInstance *aInstance, int8_t aPower)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aPower);
    return OT_ERROR_NOT_IMPLEMENTED;
}

/* ── Radio state machine ─────────────────────────────────────────────────── */
otError otPlatRadioDisable(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    if (otPlatRadioIsEnabled(aInstance)) {
        sState = OT_RADIO_STATE_DISABLED;
        if (sAuto_State_Set) {
            lmac15p4_auto_state_set(false);
            sAuto_State_Set = false;
        }
    }
    return OT_ERROR_NONE;
}

otError otPlatRadioEnable(otInstance *aInstance)
{
    if (!otPlatRadioIsEnabled(aInstance)) {
        sState = OT_RADIO_STATE_SLEEP;
    }
    return OT_ERROR_NONE;
}

void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
    OT_UNUSED_VARIABLE(aInstance);
    sIsSrcMatchEnabled = aEnable;
    lmac15p4_src_match_ctrl(0, aEnable);
}

otError otPlatRadioEnergyScan(otInstance *aInstance, uint8_t aScanChannel,
                              uint16_t aScanDuration)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aScanDuration);
    sCurrentChannel = aScanChannel;
    lmac15p4_channel_set((lmac154_channel_t)(sCurrentChannel - kMinChannel));
    uint8_t rssi = lmac15p4_read_rssi();
    otPlatRadioEnergyScanDone(aInstance, -(int8_t)rssi);
    return OT_ERROR_NONE;
}

int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return -(int8_t)lmac15p4_read_rssi();
}

otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance)
{
    otRadio_var.aInstance = aInstance;
    otRadioFrame *f = (otRadioFrame *)otRadio_var.buffPool;
    f->mPsdu = otRadio_var.buffPool + ALIGNED_RX_FRAME_SIZE;
    return f;
}

bool otPlatRadioIsEnabled(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return (sState != OT_RADIO_STATE_DISABLED);
}

otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
    OT_UNUSED_VARIABLE(aInstance);
    assert(aInstance != NULL);
    otError error = OT_ERROR_INVALID_STATE;

    if (sState != OT_RADIO_STATE_DISABLED) {
        error = OT_ERROR_NONE;
        sState = OT_RADIO_STATE_RECEIVE;
        if (aChannel != sCurrentChannel) {
            sCurrentChannel = aChannel;
            lmac15p4_channel_set(
                (lmac154_channel_t)(sCurrentChannel - kMinChannel));
        }
        if (!sAuto_State_Set) {
            lmac15p4_auto_state_set(true);
            sAuto_State_Set = true;
        }
    }
    return error;
}

otError otPlatRadioSleep(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    otError error = OT_ERROR_INVALID_STATE;

    if (sState == OT_RADIO_STATE_SLEEP || sState == OT_RADIO_STATE_RECEIVE) {
        error  = OT_ERROR_NONE;
        sState = OT_RADIO_STATE_SLEEP;
        if (sAuto_State_Set) {
            lmac15p4_auto_state_set(false);
            sAuto_State_Set = false;
        }
    }
    return error;
}

/* ── CCA & CSL ───────────────────────────────────────────────────────────── */
otError otPlatRadioGetCca(otInstance *aInstance, int8_t *aThreshold,
                          uint16_t *turnaroundtime, uint16_t *duration)
{
    OT_UNUSED_VARIABLE(aInstance);
    *aThreshold    = -(int8_t)sCCAThreshold;
    *turnaroundtime = sTurnaroundTime;
    *duration       = sCCADuration;
    return OT_ERROR_NONE;
}

otError otPlatRadioSetCca(otInstance *aInstance, int8_t aThreshold,
                          uint16_t turnaroundtime, uint16_t duration)
{
    OT_UNUSED_VARIABLE(aInstance);
    if (aThreshold <= 0 || turnaroundtime <= 0 || duration <= 0) {
        return OT_ERROR_INVALID_ARGS;
    }
    sCCAThreshold   = (uint8_t)aThreshold;
    sTurnaroundTime = turnaroundtime;
    sCCADuration    = duration;
    lmac15p4_phy_pib_set(sTurnaroundTime, sCCAMode, sCCAThreshold, sCCADuration);
    return OT_ERROR_NONE;
}

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
static uint16_t getCslPhase(void)
{
    uint32_t curTime      = otPlatAlarmMicroGetNow();
    uint32_t cslPeriodUs  = sCslPeriod * OT_US_PER_TEN_SYMBOLS;
    uint32_t diff = ((sCslSampleTime % cslPeriodUs)
                     - (curTime % cslPeriodUs)
                     + cslPeriodUs) % cslPeriodUs;
    return (uint16_t)(diff / OT_US_PER_TEN_SYMBOLS);
}

otError otPlatRadioEnableCsl(otInstance *aInstance, uint32_t aCslPeriod,
                             otShortAddress aShortAddr,
                             const otExtAddress *aExtAddr)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aShortAddr);
    OT_UNUSED_VARIABLE(aExtAddr);
    sCslPeriod = aCslPeriod;
    lmac15p4_csl_receiver_ctrl(aCslPeriod > 0 ? 1 : 0, aCslPeriod);
    return OT_ERROR_NONE;
}

void otPlatRadioUpdateCslSampleTime(otInstance *aInstance, uint32_t aCslSampleTime)
{
    OT_UNUSED_VARIABLE(aInstance);
    sCslSampleTime = aCslSampleTime;
    lmac15p4_csl_sample_time_update(sCslSampleTime);
}
#endif

uint8_t otPlatRadioGetCslAccuracy(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return 255;
}

uint8_t otPlatRadioGetCslUncertainty(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    return 175;
}

/* ── Security keys ───────────────────────────────────────────────────────── */
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
void otPlatRadioSetMacKey(otInstance *aInstance, uint8_t aKeyIdMode,
                          uint8_t aKeyId,
                          const otMacKeyMaterial *aPrevKey,
                          const otMacKeyMaterial *aCurrKey,
                          const otMacKeyMaterial *aNextKey,
                          otRadioKeyType aKeyType)
{
    OT_UNUSED_VARIABLE(aInstance);
    OT_UNUSED_VARIABLE(aKeyIdMode);
    otEXPECT(aPrevKey && aCurrKey && aNextKey);

    sKeyId   = aKeyId;
    sKeyType = aKeyType;
    memcpy(&sPrevKey, aPrevKey, sizeof(otMacKeyMaterial));
    memcpy(&sCurrKey, aCurrKey, sizeof(otMacKeyMaterial));
    memcpy(&sNextKey, aNextKey, sizeof(otMacKeyMaterial));
    lmac15p4_key_set(0, sCurrKey.mKeyMaterial.mKey.m8);
exit:
    return;
}

void otPlatRadioSetMacFrameCounter(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    OT_UNUSED_VARIABLE(aInstance);
    sMacFrameCounter = aMacFrameCounter;
}
#endif

#if OPENTHREAD_CONFIG_MLE_LINK_METRICS_SUBJECT_ENABLE
otError otPlatRadioConfigureEnhAckProbing(otInstance *aInstance,
        otLinkMetrics aLinkMetrics,
        const otShortAddress aShortAddress,
        const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);
    uint8_t ext[OT_EXT_ADDRESS_SIZE];
    for (int i = 0; i < OT_EXT_ADDRESS_SIZE; i++) {
        ext[i] = aExtAddress->m8[7 - i];
    }
    int base = (aLinkMetrics.mLqi + aLinkMetrics.mLinkMargin + aLinkMetrics.mRssi < 3)
               ? 3 : 7;

    if (aLinkMetrics.mLqi) {
        lmac15p4_src_match_short_entry(base, (uint8_t *)&aShortAddress);
        lmac15p4_src_match_extended_entry(base, ext);
    } else {
        lmac15p4_src_match_short_entry(base + 1, (uint8_t *)&aShortAddress);
        lmac15p4_src_match_extended_entry(base + 1, ext);
    }
    return OT_ERROR_NONE;
}
#endif

/* ── TX security processing ──────────────────────────────────────────────── */
static otError radioProcessTransmitSecurity(otRadioFrame *aFrame)
{
    otError error = OT_ERROR_NONE;
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    otMacKeyMaterial *key = NULL;
    uint8_t keyId;

    otEXPECT(otMacFrameIsSecurityEnabled(aFrame) &&
              otMacFrameIsKeyIdMode1(aFrame) &&
              !aFrame->mInfo.mTxInfo.mIsSecurityProcessed);

    if (otMacFrameIsAck(aFrame)) {
        keyId = otMacFrameGetKeyId(aFrame);
        otEXPECT_ACTION(keyId != 0, error = OT_ERROR_FAILED);
        if      (keyId == sKeyId)     key = &sCurrKey;
        else if (keyId == sKeyId - 1) key = &sPrevKey;
        else if (keyId == sKeyId + 1) key = &sNextKey;
        else { error = OT_ERROR_SECURITY; otEXPECT(false); }
    } else {
        key   = &sCurrKey;
        keyId = sKeyId;
    }

    aFrame->mInfo.mTxInfo.mAesKey = key;
    if (!aFrame->mInfo.mTxInfo.mIsHeaderUpdated) {
        otMacFrameSetKeyId(aFrame, keyId);
        otMacFrameSetFrameCounter(aFrame, sMacFrameCounter++);
    }
#else
    otEXPECT(!aFrame->mInfo.mTxInfo.mIsSecurityProcessed);
#endif
    otMacFrameProcessTransmitAesCcm(aFrame, &sExtAddress);
exit:
    return error;
}

/* ── Transmit ────────────────────────────────────────────────────────────── */
otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aFrame)
{
    OT_UNUSED_VARIABLE(aInstance);
    assert(aInstance && aFrame);

    uint8_t  tx_control = 0;
    uint8_t  temp[OT_RADIO_FRAME_MAX_SIZE + 4];
    otError  error = OT_ERROR_INVALID_STATE;

    if (!otRadio_var.pTxFrame) {
        otRadio_var.pTxFrame = aFrame;
        sCurrentChannel = aFrame->mChannel;
        lmac15p4_channel_set(
            (lmac154_channel_t)(sCurrentChannel - kMinChannel));
    }

#if OPENTHREAD_CONFIG_MAC_CSL_RECEIVER_ENABLE
    if (sCslPeriod > 0 && !aFrame->mInfo.mTxInfo.mIsHeaderUpdated) {
        otMacFrameSetCslIe(aFrame, (uint16_t)sCslPeriod, getCslPhase());
        aFrame->mInfo.mTxInfo.mCsmaCaEnabled = false;
    }
#endif

    radioProcessTransmitSecurity(aFrame);

    tx_control = (otRadio_var.pTxFrame->mPsdu[0] & (1 << 5)) ? 0x03 : 0x02;
    tx_control |= (1 << 2);

    if (otMacFrameIsSecurityEnabled(aFrame)) {
        uint32_t hw_fc = lmac15p4_frame_counter_get();
        if (sMacFrameCounter < hw_fc) {
            sMacFrameCounter = hw_fc + 1;
        }
    }

    temp[0] = (sMacFrameCounter >>  0) & 0xFF;
    temp[1] = (sMacFrameCounter >>  8) & 0xFF;
    temp[2] = (sMacFrameCounter >> 16) & 0xFF;
    temp[3] = (sMacFrameCounter >> 24) & 0xFF;
    memcpy(temp + 4, otRadio_var.pTxFrame->mPsdu,
           otRadio_var.pTxFrame->mLength);

    if (aFrame->mInfo.mTxInfo.mTxDelay != 0 ||
        !aFrame->mInfo.mTxInfo.mCsmaCaEnabled) {
        tx_control ^= (1 << 1);
    }

    if (lmac15p4_tx_data_send(0, temp,
                              otRadio_var.pTxFrame->mLength + 2,
                              tx_control,
                              otMacFrameGetSequence(aFrame)) != 0) {
        printk("[ot_radio] TX failed!");
        OT_NOTIFY(OT_SYSTEM_EVENT_RADIO_TX_NO_ACK);
    }

    otPlatRadioTxStarted(aInstance, aFrame);
    otRadio_var.tstx = otPlatTimeGet() + 1;
    error = OT_ERROR_NONE;
    return error;
}

/* ── Source address matching ─────────────────────────────────────────────── *
 * We delegate entirely to the lmac15p4 hardware MAC filter.               *
 * The soft-table is only used for FindEntry lookups (frame-pending logic). */
otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance, uint16_t aAddr)
{
    OT_UNUSED_VARIABLE(aInstance);
    lmac15p4_src_match_short_entry(ADD_ADDRESS_PENDING, (uint8_t *)&aAddr);
    return OT_ERROR_NONE;
}

otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance,
                                       const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);
    otExtAddress ext;
    ReverseExtAddress(&ext, aExtAddress);
    lmac15p4_src_match_extended_entry(ADD_ADDRESS_PENDING, ext.m8);
    return OT_ERROR_NONE;
}

otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance, uint16_t aAddr)
{
    OT_UNUSED_VARIABLE(aInstance);
    lmac15p4_src_match_short_entry(REMOVE_ADDRESS_PENDING, (uint8_t *)&aAddr);
    return OT_ERROR_NONE;
}

otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance,
                                         const otExtAddress *aExtAddress)
{
    OT_UNUSED_VARIABLE(aInstance);
    otExtAddress ext;
    ReverseExtAddress(&ext, aExtAddress);
    lmac15p4_src_match_extended_entry(REMOVE_ADDRESS_PENDING, ext.m8);
    return OT_ERROR_NONE;
}

void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    lmac15p4_src_match_short_entry(CLEAR_ALL, NULL);
}

void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);
    lmac15p4_src_match_extended_entry(CLEAR_ALL, NULL);
}

/* ── TX done callback (called from ISR via lmac15p4) ────────────────────── */
static void _TxDoneEvent(uint32_t tx_status)
{
    uint32_t ack_now = 0;
    static uint32_t ack_wraps = 0, ack_prev = 0;
    uint8_t ack_rssi = 127;

    if (!otRadio_var.pTxFrame) {
        return;
    }

    switch (tx_status) {
    case 0x00: /* LMAC154_TX_SUCCESS */
        OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ);
        break;
    case 0x10: /* LMAC154_TX_CSMACA_FAIL */
        OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_TX_CCA_FAIL);
        break;
    case 0x20: /* LMAC154_TX_NO_ACK_FAIL */
        OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_TX_NO_ACK);
        break;
    case 0x40: /* LMAC154_TX_GET_ACK_FP_SUCCESS */
    case 0x80: /* LMAC154_TX_GET_ACK_SUCCESS */
        otRadio_var.pAckFrame->mLength =
            lmac15p4_read_ack((uint8_t *)otRadio_var.pAckFrame->mPsdu,
                              (uint8_t *)&ack_now, 0, &ack_rssi);
        otRadio_var.pAckFrame->mInfo.mRxInfo.mTimestamp =
            longtime_to_longlong_time(&ack_prev, ack_now, &ack_wraps);
        otRadio_var.pAckFrame->mInfo.mRxInfo.mTimestamp -=
            130 + ((otRadio_var.pAckFrame->mLength + 1) * 32);
        OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_TX_ACKED);
        break;
    default:
        OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_TX_ERROR);
        break;
    }
}

/* ── RX done callback (called from ISR via lmac15p4) ────────────────────── */
static void _RxDoneEvent(uint16_t packet_length, uint8_t *rx_data,
                          uint8_t crc_status, uint8_t rssi, uint8_t snr)
{
    static uint32_t rx_wraps = 0, rx_prev = 0;
    static uint8_t  rx_cnt = 0;

    if (crc_status != 0) {
        OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_RX_CRC_FIALED);
        return;
    }

    otRadio_rxFrame_t *p = NULL;
    OT_ENTER_CRITICAL_ISR();
    if (!utils_dlist_empty(&otRadio_var.frameList)) {
        p = (otRadio_rxFrame_t *)otRadio_var.frameList.next;
        otRadio_var.dbgFrameNum--;
        utils_dlist_del(&p->dlist);
    }
    OT_EXIT_CRITICAL_ISR(0);

    if (!p) {
        OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_RX_NO_BUFF);
        return;
    }

    uint32_t rx_now = 0;
    lmac15p4_rx_rtc_time_read(rx_cnt, &rx_now);
    p->frame.mInfo.mRxInfo.mTimestamp =
        longtime_to_longlong_time(&rx_prev, rx_now, &rx_wraps);
    memcpy(p->frame.mPsdu, rx_data + 8, packet_length - 9);
    p->frame.mLength  = packet_length - 9;
    p->frame.mChannel = sCurrentChannel;
    p->frame.mInfo.mRxInfo.mRssi = -(int8_t)rssi;
    p->frame.mInfo.mRxInfo.mLqi  =
        (uint8_t)(((RAFAEL_RECEIVE_SENSITIVITY - rssi) * 0xFF)
                  / RAFAEL_RECEIVE_SENSITIVITY);

#ifdef CONFIG_OT_DEVICE_TYPE_RCP
    p->frame.mInfo.mRxInfo.mTimestamp -= 2500;
#else
    p->frame.mInfo.mRxInfo.mTimestamp -= 50 + (p->frame.mLength + 1) * 32;
#endif

    p->frame.mInfo.mRxInfo.mAckedWithFramePending = false;
    uint8_t pending = (rx_data[8 + p->frame.mLength + 3] & 0x20) >> 5;
    if (pending &&
#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
        ((otMacFrameIsVersion2015(&p->frame) && otMacFrameIsCommand(&p->frame))
         || otMacFrameIsDataRequest(&p->frame))
#else
        otMacFrameIsDataRequest(&p->frame)
#endif
        ) {
        p->frame.mInfo.mRxInfo.mAckedWithFramePending = true;
    }

#if OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2
    if (otMacFrameIsVersion2015(&p->frame) &&
        otMacFrameIsSecurityEnabled(&p->frame) &&
        otMacFrameIsAckRequested(&p->frame)) {
        p->frame.mInfo.mRxInfo.mAckedWithSecEnhAck = true;
        p->frame.mInfo.mRxInfo.mAckFrameCounter    = ++sMacFrameCounter;
    }
#endif

    OT_ENTER_CRITICAL_ISR();
    utils_dlist_add_tail(&p->dlist, &otRadio_var.rxFrameList);
    otRadio_var.dbgRxFrameNum++;
    if (otRadio_var.dbgMaxPendingFrameNum < otRadio_var.dbgRxFrameNum) {
        otRadio_var.dbgMaxPendingFrameNum = otRadio_var.dbgRxFrameNum;
    }
    OT_EXIT_CRITICAL_ISR(0);

    OT_NOTIFY_ISR(OT_SYSTEM_EVENT_RADIO_RX_DONE);
    rx_cnt = (rx_cnt > 4) ? 0 : rx_cnt + 1;
}

/* ── ot_radioTask ────────────────────────────────────────────────────────── */
void ot_radioTask(ot_system_event_t trxEvent)
{
    otRadio_rxFrame_t *pframe;
    otRadioFrame      *txframe;

    if (!(OT_SYSTEM_EVENT_RADIO_ALL_MASK & trxEvent)) {
        return;
    }

    if (otRadio_var.pTxFrame && (OT_SYSTEM_EVENT_RADIO_TX_ALL_MASK & trxEvent)) {
        txframe = otRadio_var.pTxFrame;
        otRadio_var.pTxFrame = NULL;
        otRadio_var.tstx     = 0;

        if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_DONE_NO_ACK_REQ) {
            otPlatRadioTxDone(otRadio_var.aInstance, txframe, NULL,
                              OT_ERROR_NONE);
        }
        if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_ACKED) {
            otPlatRadioTxDone(otRadio_var.aInstance, txframe,
                              otRadio_var.pAckFrame, OT_ERROR_NONE);
        }
        if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_NO_ACK) {
            otPlatRadioTxDone(otRadio_var.aInstance, txframe, NULL,
                              OT_ERROR_NO_ACK);
        }
        if (trxEvent & OT_SYSTEM_EVENT_RADIO_TX_CCA_FAIL) {
            otPlatRadioTxDone(otRadio_var.aInstance, txframe, NULL,
                              OT_ERROR_CHANNEL_ACCESS_FAILURE);
        }
    }

    if (trxEvent & OT_SYSTEM_EVENT_RADIO_RX_DONE) {
        OT_ENTER_CRITICAL();
        pframe = NULL;
        if (!utils_dlist_empty(&otRadio_var.rxFrameList)) {
            pframe = (otRadio_rxFrame_t *)otRadio_var.rxFrameList.next;
            otRadio_var.dbgRxFrameNum--;
            utils_dlist_del(&pframe->dlist);
        }
        OT_EXIT_CRITICAL();

        if (pframe) {
            otPlatRadioReceiveDone(otRadio_var.aInstance, &pframe->frame,
                                   OT_ERROR_NONE);
            OT_ENTER_CRITICAL();
            utils_dlist_add_tail(&pframe->dlist, &otRadio_var.frameList);
            otRadio_var.dbgFrameNum++;
            if (!utils_dlist_empty(&otRadio_var.rxFrameList)) {
                /* More frames pending — yield briefly then re-signal */
                OT_EXIT_CRITICAL();
                k_sleep(K_MSEC(5));
                OT_NOTIFY(OT_SYSTEM_EVENT_RADIO_RX_DONE);
                return;
            }
            OT_EXIT_CRITICAL();
        }
    }
}

/* ── ot_radioInit ────────────────────────────────────────────────────────── */
static void rafael_otp_mac_addr(uint8_t *addr)
{
    uint8_t tmp[256];
    flash_read_sec_register((uint32_t)(uintptr_t)tmp, 0x1100);
    memcpy(addr, tmp + OT_EXT_ADDRESS_SIZE, OT_EXT_ADDRESS_SIZE);
}

void ot_radioInit(void)
{
    lmac15p4_callback_t mac_cb;
    uint8_t ff[OT_EXT_ADDRESS_SIZE];
    memset(ff, 0xFF, sizeof(ff));

    if (sMacAddrReadMode == 1) {
        rafael_otp_mac_addr(sIEEE_EUI64Addr);
        if (!memcmp(ff, sIEEE_EUI64Addr, OT_EXT_ADDRESS_SIZE)) {
            flash_get_unique_id((uint32_t)(uintptr_t)sIEEE_EUI64Addr,
                                OT_EXT_ADDRESS_SIZE);
        }
    } else {
        flash_get_unique_id((uint32_t)(uintptr_t)sIEEE_EUI64Addr,
                            OT_EXT_ADDRESS_SIZE);
    }

    /* Set locally-administered bit (bit 1 of byte 0) if the address came
     * from flash UID rather than a globally assigned OTP MAC address.
     * This prevents collisions with manufacturer-assigned addresses. */
    if (!memcmp(ff, sIEEE_EUI64Addr, OT_EXT_ADDRESS_SIZE)) {
        /* Both OTP and flash UID returned all-0xFF — use TRNG fallback */
        hosal_trng_get_random_number((uint32_t *)&sIEEE_EUI64Addr[0], 2);
        sIEEE_EUI64Addr[0] |= 0x02;  /* locally administered */
        sIEEE_EUI64Addr[0] &= ~0x01; /* unicast */
    }

    printk("[OT-RADIO] EUI-64: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
           sIEEE_EUI64Addr[0], sIEEE_EUI64Addr[1],
           sIEEE_EUI64Addr[2], sIEEE_EUI64Addr[3],
           sIEEE_EUI64Addr[4], sIEEE_EUI64Addr[5],
           sIEEE_EUI64Addr[6], sIEEE_EUI64Addr[7]);

    memset(&otRadio_var, 0, offsetof(otRadio_t, buffPool));

    otRadio_var.pAckFrame =
        (otRadioFrame *)(otRadio_var.buffPool + TOTAL_RX_FRAME_SIZE);
    otRadio_var.pAckFrame->mPsdu =
        ((uint8_t *)otRadio_var.pAckFrame) + ALIGNED_RX_FRAME_SIZE;

    OT_ENTER_CRITICAL();
    utils_dlist_init(&otRadio_var.frameList);
    utils_dlist_init(&otRadio_var.rxFrameList);

    for (int i = 0; i < OTRADIO_RX_FRAME_BUFFER_NUM; i++) {
        otRadio_rxFrame_t *pf = (otRadio_rxFrame_t *)
            (otRadio_var.buffPool + TOTAL_RX_FRAME_SIZE * (i + 2));
        pf->frame.mPsdu = ((uint8_t *)pf) + ALIGNED_RX_FRAME_SIZE;
        utils_dlist_add_tail(&pf->dlist, &otRadio_var.frameList);
    }
    otRadio_var.dbgFrameNum = OTRADIO_RX_FRAME_BUFFER_NUM;
    OT_EXIT_CRITICAL();

    mac_cb.rx_cb = _RxDoneEvent;
    mac_cb.tx_cb = _TxDoneEvent;
    lmac15p4_cb_set(0, &mac_cb);

    lmac15p4_channel_set((lmac154_channel_t)(sCurrentChannel - kMinChannel));
    lmac15p4_auto_ack_set(true);
    lmac15p4_auto_state_set(sAuto_State_Set);
    lmac15p4_src_match_ctrl(0, true);
    lmac15p4_src_match_short_entry(CLEAR_ALL, NULL);
}

/* ── Address control helpers (public) ────────────────────────────────────── */
void ot_radio_short_addr_ctrl(uint8_t ctrl_type, uint8_t *short_addr)
{
    lmac15p4_src_match_short_entry(ctrl_type, short_addr);
}

void ot_radio_extend_addr_ctrl(uint8_t ctrl_type, uint8_t *extend_addr)
{
    lmac15p4_src_match_extended_entry(ctrl_type, extend_addr);
}
