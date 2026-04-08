/*
 * hosal_rf_zephyr.c — Zephyr RTOS port of hosal_rf.c
 *
 * Original: Rafael-IoT-SDK-Internal/components/platform/hosal/rt583_hosal/Src/hosal_rf.c
 *
 * FreeRTOS → Zephyr conversions:
 *   xSemaphoreCreateBinaryStatic / xSemaphoreTake / xSemaphoreGive
 *                                   → K_SEM_DEFINE / k_sem_take / k_sem_give
 *   xQueueCreate / xQueueSend / xQueueReceive
 *                                   → K_MSGQ_DEFINE / k_msgq_put / k_msgq_get
 *   xTaskCreate                     → k_thread_create
 *   vTaskNotifyGiveFromISR          → k_sem_give  (ISR-safe in Zephyr)
 *   ulTaskNotifyTake                → k_sem_take
 *   taskYIELD                       → k_yield
 *   pvPortMalloc / vPortFree        → malloc / free
 *   configASSERT(0)                 → k_panic()
 *   ASSERT()                        → k_panic()
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "hosal_rf.h"
#include "log.h"
#include "mp_sector.h"
#include "rf_common_init.h"
#include "rf_mcu.h"
#include "ruci.h"

/* ── FreeRTOS critical-section helpers (defined in freertos_shim.c) ─────────
 * lmac15p4 calls vPortEnterCritical() (irq_lock) before invoking hosal_rf
 * APIs.  Any blocking Zephyr call made while irq_lock is held will deadlock
 * on single-core Cortex-M because the COMM_SUBSYSTEM IRQ (which wakes
 * __rf_proc) is masked by BASEPRI.  _crit_unlock / _crit_relock temporarily
 * release and restore the FreeRTOS critical section around those calls.      */
extern uint32_t _crit_unlock(void);
extern void     _crit_relock(uint32_t nest);

/**************************************************************************************************
 *    MACROS
 *************************************************************************************************/
/**************************************************************************************************
 *    CONSTANTS AND DEFINES
 *************************************************************************************************/
#define HOSAL_RF_NO_STS             (0x00)
#define HOSAL_RF_EVENT_STS          (0x01)
#define HOSAL_RF_TX_DONE_STS        (0x02)
#define HOSAL_RF_TRAP_STS           (0x04)
#define HOSAL_RF_RX_DATA_STS        (0x20)
#define HOSAL_RF_RTC_WAKE_STS       (0x80)

#define HOSAL_RF_Q_MAX_SIZE         (16u)

/* Zephyr thread parameters for __rf_proc */
#define RF_PROC_STACK_SIZE          (2048)
/* Priority 2: higher than OT thread (5), lower than ISRs */
#define RF_PROC_PRIORITY            (2)

#define HOSAL_RF_HCI_EVENT          (0x04)
#define HOSAL_RF_RUCI_PCI_EVENT     (0x16)
#define HOSAL_RF_RUCI_SF_HOST_EVENT (0xF1)
#define HOSAL_RF_RUCI_CMN_EVENT     (0x3F)

#define HOSAL_RF_HCI_ACL_DATA       (0x02)

/**************************************************************************************************
 *    TYPEDEFS
 *************************************************************************************************/
typedef struct {
    uint8_t rp;
    uint8_t wp;
    uint8_t cmd_queue[HOSAL_RF_Q_MAX_SIZE];
} hosal_rf_cmd_state_t;

/**************************************************************************************************
 *    GLOBAL VARIABLES
 *************************************************************************************************/
static hosal_rf_cmd_state_t ghosal_rf_cmd_state_q;

/* Binary semaphore protecting command/event access (replaces xSemaphore).
 * Initial count = 1 (given) so the first xSemaphoreTake succeeds. */
static K_SEM_DEFINE(g_rf_cmd_sem, 1, 1);

/* Task-notification semaphore waking __rf_proc (replaces task notification).
 * Initial count = 0 so __rf_proc blocks until the first __rf_signal(). */
static K_SEM_DEFINE(g_rf_notify_sem, 0, 1);

/* Event queue: 4 slots of void* (replaces xQueueCreate(4, sizeof(void*))). */
K_MSGQ_DEFINE(g_rf_evt_msgq, sizeof(void *), 4, sizeof(void *));

/* __rf_proc Zephyr thread */
K_THREAD_STACK_DEFINE(g_rf_stack, RF_PROC_STACK_SIZE);
static struct k_thread g_rf_thread;

static uint8_t g_event_buffer[384], g_rx_data[2064];
static hosal_rf_callback_t g_pci_event_cb   = NULL;  /* RUCI cmd-confirm events (lmac15p4) */
static hosal_rf_callback_t g_pci_rx_done_cb = NULL;
static hosal_rf_callback_t g_pci_tx_done_cb = NULL;
static hosal_rf_callback_t g_hci_evt_cb     = NULL;
static hosal_rf_callback_t g_hci_data_cb    = NULL;

/**************************************************************************************************
 *    LOCAL FUNCTIONS
 *************************************************************************************************/
__STATIC_FORCEINLINE void __rf_push_state(uint8_t intStatus) {
    if (intStatus == 0) {
        return;
    }
    RfMcu_InterruptDisableAll();
    if (ghosal_rf_cmd_state_q.cmd_queue[ghosal_rf_cmd_state_q.wp] == 0) {
        ghosal_rf_cmd_state_q.cmd_queue[ghosal_rf_cmd_state_q.wp] = intStatus;
        if ((++ghosal_rf_cmd_state_q.wp) == HOSAL_RF_Q_MAX_SIZE) {
            ghosal_rf_cmd_state_q.wp = 0;
        }
    }
    RfMcu_InterruptEnableAll();
}

__STATIC_FORCEINLINE uint8_t __rf_pop_state(void) {
    uint8_t ret;

    RfMcu_InterruptDisableAll();
    ret = ghosal_rf_cmd_state_q.cmd_queue[ghosal_rf_cmd_state_q.rp];
    if (ret != 0) {
        ghosal_rf_cmd_state_q.cmd_queue[ghosal_rf_cmd_state_q.rp] = 0;
        if (ghosal_rf_cmd_state_q.rp <= ghosal_rf_cmd_state_q.wp) {
            ghosal_rf_cmd_state_q.rp++;
        } else {
            if (++ghosal_rf_cmd_state_q.rp == HOSAL_RF_Q_MAX_SIZE) {
                ghosal_rf_cmd_state_q.rp = 0;
            }
        }
    }
    RfMcu_InterruptEnableAll();
    return ret;
}

/* Signal __rf_proc from ISR context.
 * k_sem_give() is ISR-safe in Zephyr — no portYIELD_FROM_ISR needed. */
__STATIC_FORCEINLINE void __rf_signal(void) {
    k_sem_give(&g_rf_notify_sem);
}

static void __rf_event_callback(uint8_t intStatus) {
    RfMcu_InterruptClear(intStatus);

    if (intStatus & HOSAL_RF_EVENT_STS) {
        __rf_push_state(HOSAL_RF_EVENT_STS);
    }

    if (intStatus & HOSAL_RF_TRAP_STS) {
        uint32_t debug_value;
        RfMcu_MemoryGet(0x4008, (uint8_t *)&debug_value, sizeof(debug_value));
        printk("RF TRAP 0x%08x\n", debug_value);
        k_panic();
    }

    if (intStatus & HOSAL_RF_TX_DONE_STS) {
        __rf_push_state(HOSAL_RF_TX_DONE_STS);
    }

    if (intStatus & HOSAL_RF_RX_DATA_STS) {
        __rf_push_state(HOSAL_RF_RX_DATA_STS);
    }

    if (intStatus & HOSAL_RF_RTC_WAKE_STS) {
        __rf_push_state(HOSAL_RF_RTC_WAKE_STS);
    }

    __rf_signal();
}

static void __rf_tx_data_queue_init(void) {
    uint8_t i;

    ghosal_rf_cmd_state_q.rp = 0;
    ghosal_rf_cmd_state_q.wp = 0;
    for (i = 0; i < HOSAL_RF_Q_MAX_SIZE; i++) {
        ghosal_rf_cmd_state_q.cmd_queue[i] = 0;
    }
}

__STATIC_FORCEINLINE void handle_event_status(void) {
    RF_MCU_RX_CMDQ_ERROR rxCmdError = RF_MCU_RX_CMDQ_ERR_INIT;
    uint32_t event_len = 0;
    uint8_t *evt_ptr   = NULL;

    event_len = RfMcu_EvtQueueRead(g_event_buffer, &rxCmdError);
    if (rxCmdError == RF_MCU_RX_CMDQ_GET_SUCCESS) {
        switch (g_event_buffer[0]) {
        case HOSAL_RF_HCI_EVENT:
            if (g_hci_evt_cb) {
                if (g_hci_evt_cb(g_event_buffer) == HOSAL_RF_CB_CMD_COMPLETE) {
                    k_sem_give(&g_rf_cmd_sem);
                }
            }
            break;

        case HOSAL_RF_RUCI_PCI_EVENT:
        case HOSAL_RF_RUCI_SF_HOST_EVENT:
        case HOSAL_RF_RUCI_CMN_EVENT:
            if (g_pci_event_cb) {
                /* lmac15p4 registered this to receive RUCI command
                 * confirmation events.  Call directly with the stack
                 * buffer — no malloc/queue needed.  lmac15p4 will call
                 * xQueueGenericSend(done_sem) inside the callback.      */
                g_pci_event_cb(g_event_buffer);
                k_sem_give(&g_rf_cmd_sem);
            } else {
                /* No RUCI-event callback registered — queue for
                 * hosal_rf_read_event() consumers.                      */
                do {
                    evt_ptr = malloc(event_len);
                    if (evt_ptr == NULL) {
                        k_yield();
                    }
                } while (evt_ptr == NULL);
                memcpy(evt_ptr, g_event_buffer, event_len);
                if (k_msgq_put(&g_rf_evt_msgq, (void *)&evt_ptr, K_NO_WAIT) != 0) {
                    log_error("RF evt queue full, discarding\n");
                    free(evt_ptr);
                }
                k_sem_give(&g_rf_cmd_sem);
            }
            break;

        default:
            log_error("unknown event 0x%02x\n", g_event_buffer[0]);
            break;
        }
    }
}

__STATIC_FORCEINLINE void handle_rx_data_status(void) {
    RF_MCU_RXQ_ERROR rx_queue_error = RF_MCU_RXQ_ERR_INIT;

    while (RfMcu_RxQueueRead(g_rx_data, &rx_queue_error) != 0) {
        if (rx_queue_error != RF_MCU_RXQ_GET_SUCCESS) {
            break;
        }
        if (g_rx_data[0] == RUCI_PCI_DATA_HEADER) {
            if (g_pci_rx_done_cb) {
                g_pci_rx_done_cb(g_rx_data);
            }
        } else if (g_rx_data[0] == HOSAL_RF_HCI_ACL_DATA) {
            if (g_hci_data_cb) {
                g_hci_data_cb(g_rx_data);
            }
        } else {
            log_error("invalid data header 0x%02x\n", g_rx_data[0]);
        }
    }
}

__STATIC_FORCEINLINE void handle_tx_done_status(void) {
    uint32_t tx_status = RfMcu_McuStateRead();
    tx_status = (tx_status & ~RF_MCU_STATE_EVENT_DONE);
    if (g_pci_tx_done_cb) {
        g_pci_tx_done_cb((void *)tx_status);
    }
    RfMcu_HostCmdSet((tx_status & 0xF4));
}

static void __rf_check_state(void) {
    uint8_t rf_state = 0;

    do {
        rf_state = __rf_pop_state();

        switch (rf_state) {
        case HOSAL_RF_NO_STS:
            break;
        case HOSAL_RF_EVENT_STS:
            handle_event_status();
            break;
        case HOSAL_RF_RX_DATA_STS:
            handle_rx_data_status();
            break;
        case HOSAL_RF_TX_DONE_STS:
            handle_tx_done_status();
            break;
        case HOSAL_RF_RTC_WAKE_STS:
            break;
        default:
            break;
        }
    } while (rf_state != 0);
}

/* RF event processing thread (replaces FreeRTOS __rf_proc task). */
static void __rf_proc(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (1) {
        k_sem_take(&g_rf_notify_sem, K_FOREVER);
        __rf_check_state();
    }
}

static hosal_rf_status_t __rf_sub_command_set(uint8_t *cmd_ptr,
        uint32_t cmd_len,
        uint8_t expected_subheader) {
    ruci_para_cnf_event_t sCnfEvent = {0};

    while (hosal_rf_write_command(cmd_ptr, cmd_len) != HOSAL_RF_STATUS_SUCCESS);

    hosal_rf_read_event((uint8_t *)&sCnfEvent);
    RUCI_ENDIAN_CONVERT((uint8_t *)&sCnfEvent, RUCI_CNF_EVENT);
    if (sCnfEvent.pci_cmd_subheader != expected_subheader) {
        return HOSAL_RF_STATUS_CONTENT_ERROR;
    }
    if (sCnfEvent.status != HOSAL_RF_STATUS_SUCCESS) {
        return sCnfEvent.status;
    }
    return HOSAL_RF_STATUS_SUCCESS;
}

static hosal_rf_status_t __rf_modem_set(hosal_rf_15p4_modem_cnf_t *modem_cnf) {
    uint8_t *cmd_ptr = NULL;
    uint16_t cmd_len = 0;
    uint8_t cfn_sub_hdr = 0;
    hosal_rf_status_t rval = HOSAL_RF_STATUS_SUCCESS;

    switch (modem_cnf->modem) {
    case HOSAL_RF_MODEM_FSK:
        cmd_ptr = malloc(RUCI_LEN_INITIATE_FSK);
        if (cmd_ptr == NULL) { rval = HOSAL_RF_STATUS_NO_MEMORY; break; }
        SET_RUCI_PARA_INITIATE_FSK(cmd_ptr, modem_cnf->band_type);
        RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_INITIATE_FSK);
        cmd_len = RUCI_LEN_INITIATE_FSK;
        cfn_sub_hdr = RUCI_CODE_INITIATE_FSK;
        break;

    case HOSAL_RF_MODEM_2P4G_OQPSK:
        cmd_ptr = malloc(sizeof(ruci_para_initiate_zigbee_t));
        if (cmd_ptr == NULL) { rval = HOSAL_RF_STATUS_NO_MEMORY; break; }
        SET_RUCI_PARA_INITIATE_ZIGBEE(cmd_ptr);
        RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_INITIATE_ZIGBEE);
        cmd_len = RUCI_LEN_INITIATE_ZIGBEE;
        cfn_sub_hdr = RUCI_CODE_INITIATE_ZIGBEE;
        break;

    case HOSAL_RF_MODEM_BLE:
        break;

    case HOSAL_RF_MODEM_SUBG_OQPSK:
        cmd_ptr = malloc(RUCI_LEN_INITIATE_OQPSK);
        if (cmd_ptr == NULL) { rval = HOSAL_RF_STATUS_NO_MEMORY; break; }
        SET_RUCI_PARA_INITIATE_OQPSK(cmd_ptr, modem_cnf->band_type);
        RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_INITIATE_OQPSK);
        cmd_len = RUCI_LEN_INITIATE_OQPSK;
        cfn_sub_hdr = RUCI_CODE_INITIATE_OQPSK;
        break;

    default:
        rval = HOSAL_RF_STATUS_INVALID_PARAMETER;
        break;
    }

    if (rval == HOSAL_RF_STATUS_SUCCESS) {
        rval = __rf_sub_command_set(cmd_ptr, cmd_len, cfn_sub_hdr);
        if (cmd_ptr) {
            free(cmd_ptr);
        }
    }
    return rval;
}

static hosal_rf_status_t __rf_frequency_set(uint32_t frequency) {
    uint8_t cmd_ptr[RUCI_LEN_SET_RF_FREQUENCY];
    SET_RUCI_PARA_SET_RF_FREQUENCY(cmd_ptr, frequency);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_RF_FREQUENCY);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_RF_FREQUENCY,
                                RUCI_CODE_SET_RF_FREQUENCY);
}

static hosal_rf_status_t __rf_rssi_get(uint8_t *rssi) {
    ruci_para_get_rssi_event_t sGetRssiEvent = {0};
    ruci_para_cnf_event_t sCnfEvent = {0};
    uint8_t cmd_ptr[RUCI_LEN_GET_RSSI];

    SET_RUCI_PARA_GET_RSSI(cmd_ptr);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_GET_RSSI);
    while (hosal_rf_write_command(cmd_ptr, RUCI_LEN_GET_RSSI) != HOSAL_RF_STATUS_SUCCESS);

    hosal_rf_read_event((uint8_t *)&sCnfEvent);
    hosal_rf_read_event((uint8_t *)&sGetRssiEvent);
    RUCI_ENDIAN_CONVERT((uint8_t *)&sCnfEvent, RUCI_CNF_EVENT);
    if (sCnfEvent.pci_cmd_subheader != RUCI_CODE_GET_RSSI) {
        return HOSAL_RF_STATUS_CONTENT_ERROR;
    }
    if (sCnfEvent.status != HOSAL_RF_STATUS_SUCCESS) {
        return sCnfEvent.status;
    }
    RUCI_ENDIAN_CONVERT((uint8_t *)&sGetRssiEvent, RUCI_GET_RSSI_EVENT);
    if (sGetRssiEvent.sub_header != RUCI_CODE_GET_RSSI_EVENT) {
        return HOSAL_RF_STATUS_CONTENT_ERROR;
    }
    *rssi = sGetRssiEvent.rssi;
    return HOSAL_RF_STATUS_SUCCESS;
}

static hosal_rf_status_t __rf_rx_enable_set(uint32_t rx_timeout) {
    uint8_t cmd_ptr[RUCI_LEN_SET_RX_ENABLE];
    SET_RUCI_PARA_SET_RX_ENABLE(cmd_ptr, rx_timeout);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_RX_ENABLE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_RX_ENABLE,
                                RUCI_CODE_SET_RX_ENABLE);
}

static hosal_rf_status_t __rf_sleep_set(uint32_t enable) {
    uint8_t cmd_ptr[RUCI_LEN_SET_RF_SLEEP];
    SET_RUCI_PARA_SET_RF_SLEEP(cmd_ptr, enable);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_RF_SLEEP);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_RF_SLEEP,
                                RUCI_CODE_SET_RF_SLEEP);
}

static hosal_rf_status_t __rf_idle_set(void) {
    uint8_t cmd_ptr[RUCI_LEN_SET_RF_IDLE];
    SET_RUCI_PARA_SET_RF_IDLE(cmd_ptr);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_RF_IDLE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_RF_IDLE,
                                RUCI_CODE_SET_RF_IDLE);
}

static hosal_rf_status_t __rf_key_set(uint8_t *pKey) {
    uint8_t cmd_ptr[RUCI_LEN_SET_RFE_SECURITY];
    SET_RUCI_PARA_SET_RFE_SECURITY(
        cmd_ptr, *pKey, *(pKey+1), *(pKey+2), *(pKey+3), *(pKey+4),
        *(pKey+5), *(pKey+6), *(pKey+7), *(pKey+8), *(pKey+9),
        *(pKey+10), *(pKey+11), *(pKey+12), *(pKey+13), *(pKey+14),
        *(pKey+15), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_RFE_SECURITY);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_RFE_SECURITY,
                                RUCI_CODE_SET_RFE_SECURITY);
}

static hosal_rf_status_t __rf_pta_ctrl_set(hosal_rf_pta_ctrl_t *pta_ctrl) {
    uint8_t cmd_ptr[RUCI_LEN_SET_PTA_DEFAULT];
    SET_RUCI_PARA_SET_PTA_DEFAULT(cmd_ptr, pta_ctrl->enable, pta_ctrl->inverse);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_PTA_DEFAULT);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_PTA_DEFAULT,
                                RUCI_CODE_SET_PTA_DEFAULT);
}

static hosal_rf_status_t __rf_tx_data_start_set(hosal_rf_tx_data_t *tx_data) {
    hosal_rf_status_t rval = HOSAL_RF_STATUS_SUCCESS;
    uint8_t *cmd_ptr = NULL;
    uint16_t tmp_len = 2;

    tmp_len += tx_data->data_len;
    cmd_ptr = malloc(RUCI_LEN_SET_TX_CONTROL_FIELD + tx_data->data_len);
    if (cmd_ptr == NULL) {
        return HOSAL_RF_STATUS_NO_MEMORY;
    }
    SET_RUCI_PARA_SET_TX_CONTROL_FIELD(cmd_ptr, tx_data->control, tx_data->dsn);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_TX_CONTROL_FIELD);
    memcpy(&cmd_ptr[2], (uint8_t *)&tmp_len, 2);
    memcpy(&cmd_ptr[RUCI_LEN_SET_TX_CONTROL_FIELD], tx_data->pData,
           tx_data->data_len);

    do {
        rval = hosal_rf_write_tx_data(
                   cmd_ptr, RUCI_LEN_SET_TX_CONTROL_FIELD + tx_data->data_len);
    } while (rval != HOSAL_RF_STATUS_SUCCESS);

    if (cmd_ptr) {
        free(cmd_ptr);
    }
    return rval;
}

static hosal_rf_status_t __rf_single_tone_mode_set(uint8_t st_mode) {
    uint8_t cmd_ptr[RUCI_LEN_SET_SINGLE_TONE_MODE];
    SET_RUCI_PARA_SET_SINGLE_TONE_MODE(cmd_ptr, st_mode);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_SINGLE_TONE_MODE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_SINGLE_TONE_MODE,
                                RUCI_CODE_SET_SINGLE_TONE_MODE);
}

static hosal_rf_status_t __rf_tx_continuous_wave_set(uint32_t tx_enable) {
    hosal_rf_status_t status;
    hosal_rf_tx_data_t tx_data = {0};
    static uint8_t dummy_tx_data[10];

    tx_data.data_len = 10;
    tx_data.control  = 0;
    tx_data.dsn      = 0;
    tx_data.pData    = dummy_tx_data;

    if (tx_enable) {
        status = __rf_single_tone_mode_set(2);
        if (status != HOSAL_RF_STATUS_SUCCESS) {
            return status;
        }
        status = __rf_tx_data_start_set(&tx_data);
    } else {
        status = __rf_single_tone_mode_set(0);
    }
    return status;
}

static hosal_rf_status_t __rf_tx_pwr_set(hosal_rf_tx_power_t *hosal_rf_tx_power) {
    uint8_t cmd_ptr[RUCI_LEN_SET_TX_POWER];
    uint8_t band_type_ruci;
    band_type_ruci = (hosal_rf_tx_power->band_type < HOSAL_RF_BAND_SUBG_868M)
                     ? (hosal_rf_tx_power->band_type ^ 1)
                     : hosal_rf_tx_power->band_type;

    if (hosal_rf_tx_power->modem == HOSAL_RF_MODEM_FSK) {
        SET_RUCI_PARA_SET_TX_POWER(cmd_ptr, band_type_ruci,
                                   hosal_rf_tx_power->power_index);
        RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_TX_POWER);
        return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_TX_POWER,
                                    RUCI_CODE_SET_TX_POWER);
    } else {
        SET_RUCI_PARA_SET_TX_POWER_OQPSK(cmd_ptr, band_type_ruci,
                                         hosal_rf_tx_power->power_index);
        RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_TX_POWER_OQPSK);
        return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_TX_POWER_OQPSK,
                                    RUCI_CODE_SET_TX_POWER_OQPSK);
    }
}

static hosal_rf_status_t __rf_tx_pwr_comp_set(hosal_rf_modem_t *modem) {
    uint8_t cmd_ptr[RUCI_LEN_SET_TX_POWER_COMPENSATION];

#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    uint32_t r_txpolyb_gain = 0;
    uint32_t r_pab_pw_pre   = 0;
    uint8_t  r_modem_type   = 0;

    RfMcu_MemoryGet(0x034C, (uint8_t *)&r_txpolyb_gain, 4);
    RfMcu_MemoryGet(0x0348, (uint8_t *)&r_pab_pw_pre, 4);
    r_txpolyb_gain = (r_txpolyb_gain >> 16) & 0x3F;
    r_pab_pw_pre   = (r_pab_pw_pre >> 12) & 0x3;

    switch (*modem) {
    case HOSAL_RF_MODEM_BLE:         r_modem_type = 0; break;
    case HOSAL_RF_MODEM_2P4G_OQPSK: r_modem_type = 1; break;
    default: break;
    }
    log_info("modem type: %d", r_modem_type);
    SET_RUCI_PARA_SET_TX_POWER_COMPENSATION(cmd_ptr, 0, r_txpolyb_gain,
                                            r_pab_pw_pre, r_modem_type);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_TX_POWER_COMPENSATION);
#endif
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_TX_POWER_COMPENSATION,
                                RUCI_CODE_SET_TX_POWER_COMPENSATION);
}

static hosal_rf_status_t
__rf_tx_pwr_ch_comp_set(hosal_rf_tx_power_ch_comp_t *tx_pwr_ch_comp_ctrl) {
    uint8_t cmd_ptr[RUCI_LEN_SET_TX_POWER_CHANNEL_COMPENSATION];

#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    int8_t  r_tx_pwr_offset0 = 0;
    int8_t  r_tx_pwr_offset1 = 0;
    int8_t  r_tx_pwr_offset2 = 0;
    int8_t  r_tx_pwr_offset3 = 0;
    uint8_t r_modem_type     = 0;
    mp_tx_power_trim_t sTx_power_trim;

    mpcalrftrimread(MP_ID_TX_POWER_TRIM, MP_CNT_TX_POWER_TRIM,
                    (uint8_t *)(&sTx_power_trim));
    log_info("default stage: %d", sTx_power_trim.tx_gain_idx_2g_fsk);

    r_tx_pwr_offset0 = tx_pwr_ch_comp_ctrl->tx_power_stage0 - sTx_power_trim.tx_gain_idx_2g_fsk;
    r_tx_pwr_offset1 = tx_pwr_ch_comp_ctrl->tx_power_stage1 - sTx_power_trim.tx_gain_idx_2g_fsk;
    r_tx_pwr_offset2 = tx_pwr_ch_comp_ctrl->tx_power_stage2 - sTx_power_trim.tx_gain_idx_2g_fsk;
    r_tx_pwr_offset3 = tx_pwr_ch_comp_ctrl->tx_power_stage3 - sTx_power_trim.tx_gain_idx_2g_fsk;
    log_info("offset: %d, %d, %d, %d",
             r_tx_pwr_offset0, r_tx_pwr_offset1, r_tx_pwr_offset2, r_tx_pwr_offset3);

    switch (tx_pwr_ch_comp_ctrl->modem_type) {
    case HOSAL_RF_MODEM_BLE:         r_modem_type = 0; break;
    case HOSAL_RF_MODEM_2P4G_OQPSK: r_modem_type = 1; break;
    default: break;
    }
    log_info("modem type: %d", r_modem_type);
    SET_RUCI_PARA_SET_TX_POWER_CHANNEL_COMPENSATION(
        cmd_ptr, r_tx_pwr_offset0, r_tx_pwr_offset1,
        r_tx_pwr_offset2, r_tx_pwr_offset3, r_modem_type);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_TX_POWER_CHANNEL_COMPENSATION);
#endif
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_TX_POWER_CHANNEL_COMPENSATION,
                                RUCI_CODE_SET_TX_POWER_CHANNEL_COMPENSATION);
}

static hosal_rf_status_t
__rf_tx_pwr_comp_seg_set(hosal_rf_tx_power_comp_seg_t *tx_pwr_comp_seg_ctrl) {
    uint8_t cmd_ptr[RUCI_LEN_SET_TX_POWER_CHANNEL_SEGMENT];

#if (RF_MCU_CHIP_MODEL == RF_MCU_CHIP_569MP)
    uint8_t r_modem_type = 0;
    log_info("segment: %d, %d, %d", tx_pwr_comp_seg_ctrl->segmentA,
             tx_pwr_comp_seg_ctrl->segmentB, tx_pwr_comp_seg_ctrl->segmentC);

    switch (tx_pwr_comp_seg_ctrl->modem_type) {
    case HOSAL_RF_MODEM_BLE:         r_modem_type = 0; break;
    case HOSAL_RF_MODEM_2P4G_OQPSK: r_modem_type = 1; break;
    default: break;
    }
    log_info("modem type: %d", r_modem_type);
    SET_RUCI_PARA_SET_TX_POWER_CHANNEL_SEGMENT(
        cmd_ptr, tx_pwr_comp_seg_ctrl->segmentA, tx_pwr_comp_seg_ctrl->segmentB,
        tx_pwr_comp_seg_ctrl->segmentC, r_modem_type);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_TX_POWER_CHANNEL_SEGMENT);
#endif
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_TX_POWER_CHANNEL_SEGMENT,
                                RUCI_CODE_SET_TX_POWER_CHANNEL_SEGMENT);
}

static hosal_rf_status_t __rf_wake_on_radio_set(hosal_rf_wake_on_radio_t *wake_on_radio) {
    uint8_t cmd_ptr[RUCI_LEN_SET_WAKE_ON_RADIO];
    if (wake_on_radio->sleep_time > 0x400000) {
        return HOSAL_RF_STATUS_INVALID_CMD;
    }
    SET_RUCI_PARA_SET_WAKE_ON_RADIO(cmd_ptr, wake_on_radio->frequency,
                                    wake_on_radio->rx_on_time, wake_on_radio->sleep_time);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_WAKE_ON_RADIO);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_WAKE_ON_RADIO,
                                RUCI_CODE_SET_WAKE_ON_RADIO);
}

static hosal_rf_status_t __rf_15p4_mac_pib_set(hosal_rf_15p4_mac_pib_t *pib) {
    uint8_t cmd_ptr[RUCI_LEN_SET15P4_MAC_PIB];
    SET_RUCI_PARA_SET15P4_MAC_PIB(
        cmd_ptr, pib->backoff_period, pib->ack_wait_duration, pib->max_BE,
        pib->max_CSMA_backoffs, pib->max_frame_total_wait_time,
        pib->max_frame_retries, pib->min_BE);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET15P4_SW_MAC);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET15P4_MAC_PIB,
                                RUCI_CODE_SET15P4_MAC_PIB);
}

static hosal_rf_status_t __rf_15p4_phy_pib_set(hosal_rf_15p4_phy_pib_t *pib) {
    uint8_t cmd_ptr[RUCI_LEN_SET15P4_PHY_PIB];
    SET_RUCI_PARA_SET15P4_PHY_PIB(cmd_ptr, pib->turnaround_time, pib->cca_mode,
                                  pib->cca_threshold, pib->cca_duration);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET15P4_PHY_PIB);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET15P4_PHY_PIB,
                                RUCI_CODE_SET15P4_PHY_PIB);
}

static hosal_rf_status_t __rf_15p4_ack_pending_set(uint32_t state) {
    uint8_t cmd_ptr[RUCI_LEN_SET15P4_PENDING_BIT];
    SET_RUCI_PARA_SET15P4_PENDING_BIT(cmd_ptr, state);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET15P4_PENDING_BIT);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET15P4_PENDING_BIT,
                                RUCI_CODE_SET15P4_PENDING_BIT);
}

static hosal_rf_status_t __rf_15p4_auto_ack_set(uint32_t state) {
    uint8_t cmd_ptr[RUCI_LEN_SET15P4_AUTO_ACK];
    SET_RUCI_PARA_SET15P4_AUTO_ACK(cmd_ptr, state);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET15P4_AUTO_ACK);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET15P4_AUTO_ACK,
                                RUCI_CODE_SET15P4_AUTO_ACK);
}

static hosal_rf_status_t __rf_15p4_auto_state_set(uint32_t state) {
    uint8_t cmd_ptr[RUCI_LEN_SET_RFB_AUTO_STATE];
    SET_RUCI_PARA_SET_RFB_AUTO_STATE(cmd_ptr, state);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_RFB_AUTO_STATE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_RFB_AUTO_STATE,
                                RUCI_CODE_SET_RFB_AUTO_STATE);
}

static hosal_rf_status_t
__rf_15p4_address_filter_set(hosal_rf_15p4_address_filter_t *filter) {
    uint8_t cmd_ptr[RUCI_LEN_SET15P4_ADDRESS_FILTER];
    SET_RUCI_PARA_SET15P4_ADDRESS_FILTER(
        cmd_ptr, filter->promiscuous, filter->short_address,
        filter->long_address_0, filter->long_address_1, filter->panid,
        filter->is_coordinator);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET15P4_ADDRESS_FILTER);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET15P4_ADDRESS_FILTER,
                                RUCI_CODE_SET15P4_ADDRESS_FILTER);
}

static hosal_rf_status_t __rf_15p4_ack_packet_get(hosal_rf_15p4_ack_packet_t *ack) {
    uint32_t temp_data;
    uint16_t temp_addr;
    uint8_t  packet_length;
    uint8_t  page;
    uint8_t  i;
    uint8_t  tmp_rtc_time;
    bool is2bytephr = ack->is2bytephr;
    bool skip = is2bytephr;

    RfMcu_MemoryGet(0x4038, (uint8_t *)&temp_data, 4);
    page = (uint8_t)((temp_data >> 8) & 0xff);
    temp_addr = 0x4000 + (page * 64) + 4;
    RfMcu_MemoryGet(temp_addr, (uint8_t *)&temp_data, 4);
    temp_addr += is2bytephr ? 4 : 0;
    RfMcu_MemoryGet(temp_addr, (uint8_t *)&temp_data, 4);
    packet_length = is2bytephr ? (uint8_t)(temp_data & 0xff)
                               : (uint8_t)((temp_data >> 24) & 0xff);
    temp_addr += is2bytephr ? 0 : 4;
    ack->ack_len = packet_length;

    while (packet_length > 0) {
        RfMcu_MemoryGet(temp_addr, (uint8_t *)&temp_data, 4);
        for (i = 0; i < 4; i++) {
            if (packet_length <= 0) continue;
            if (skip) { skip = 0; continue; }
            *ack->pdata = (uint8_t)((temp_data >> (8 * i)) & 0xff);
            ack->pdata += sizeof(uint8_t);
            packet_length--;
        }
        temp_addr += 4;
    }

    page += 1;
    temp_addr = 0x4000 + (page * 64) + (5 * 4);
    RfMcu_MemoryGet(temp_addr, (uint8_t *)ack->ptime, 4);
    tmp_rtc_time   = ack->ptime[3]; ack->ptime[3] = ack->ptime[0]; ack->ptime[0] = tmp_rtc_time;
    tmp_rtc_time   = ack->ptime[1]; ack->ptime[1] = ack->ptime[2]; ack->ptime[2] = tmp_rtc_time;
    temp_addr = 0x4000 + (page * 64) + (12 * 4);
    RfMcu_MemoryGet(temp_addr, (uint8_t *)&temp_data, 4);
    *ack->prssi = (temp_data & 0xff);
    return HOSAL_RF_STATUS_SUCCESS;
}

static hosal_rf_status_t __rf_15p4_src_match_set(uint32_t enable) {
    uint8_t cmd_ptr[RUCI_LEN_SET_SRC_MATCH_ENABLE];
    SET_RUCI_PARA_SET_SRC_MATCH_ENABLE(cmd_ptr, enable);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_SRC_MATCH_ENABLE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_SRC_MATCH_ENABLE,
                                RUCI_CODE_SET_SRC_MATCH_ENABLE);
}

static hosal_rf_status_t
__rf_15p4_src_match_short_entry(hosal_rf_15p4_src_match_t *entry) {
    uint8_t cmd_ptr[RUCI_LEN_SET_SRC_MATCH_SHORT_ENTRY_CTRL];
    SET_RUCI_PARA_SET_SRC_MATCH_SHORT_ENTRY_CTRL(
        cmd_ptr, entry->control_type, *entry->addr, *(entry->addr + 1));
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_SRC_MATCH_SHORT_ENTRY_CTRL);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_SRC_MATCH_SHORT_ENTRY_CTRL,
                                RUCI_CODE_SET_SRC_MATCH_SHORT_ENTRY_CTRL);
}

static hosal_rf_status_t
__rf_15p4_src_match_extend_entry(hosal_rf_15p4_src_match_t *entry) {
    uint8_t cmd_ptr[RUCI_LEN_SET_SRC_MATCH_EXTENDED_ENTRY_CTRL];
    SET_RUCI_PARA_SET_SRC_MATCH_EXTENDED_ENTRY_CTRL(
        cmd_ptr, entry->control_type, *entry->addr, *(entry->addr+1),
        *(entry->addr+2), *(entry->addr+3), *(entry->addr+4),
        *(entry->addr+5), *(entry->addr+6), *(entry->addr+7));
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_SRC_MATCH_EXTENDED_ENTRY_CTRL);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_SRC_MATCH_EXTENDED_ENTRY_CTRL,
                                RUCI_CODE_SET_SRC_MATCH_EXTENDED_ENTRY_CTRL);
}

static hosal_rf_status_t __rf_15p4_frame_counter_get(uint32_t *frame_counter) {
    uint32_t temp_data;
    uint8_t  page;
    uint16_t q_addr;

    RfMcu_MemoryGet(0x04038, (uint8_t *)&temp_data, 4);
    page   = (uint8_t)((temp_data >> 8) & 0xff) + 1;
    q_addr = 0x4000 + (page * 64) + (8 * 4);
    RfMcu_MemoryGet(q_addr, (uint8_t *)frame_counter, 4);
    *frame_counter = (((*frame_counter & 0xFF000000) >> 24)
                    | ((*frame_counter & 0x00FF0000) >> 8)
                    | ((*frame_counter & 0x0000FF00) << 8)
                    | ((*frame_counter & 0x000000FF) << 24));
    return HOSAL_RF_STATUS_SUCCESS;
}

hosal_rf_status_t __rf_15p4_rtc_time_read(uint32_t *rtc_time) {
    uint32_t rtc_time_tmp;
    uint32_t rtc_trigger = 0x200;
    RfMcu_MemorySet(0x400, (uint8_t *)&rtc_trigger, 4);
    do {
        RfMcu_MemoryGet(0x414, (uint8_t *)&rtc_time_tmp, 4);
        RfMcu_MemoryGet(0x414, (uint8_t *)rtc_time, 4);
    } while (rtc_time_tmp != *rtc_time);
    return HOSAL_RF_STATUS_SUCCESS;
}

hosal_rf_status_t __rf_15p4_rx_rtc_time_read(hosal_rf_15p4_rx_rtc_t *rx_rtc) {
    uint32_t temp_data;
    uint8_t  page;
    uint16_t q_addr;
    uint8_t  tmp_rtc_time;

    RfMcu_MemoryGet(0x04038, (uint8_t *)&temp_data, 4);
    page   = (uint8_t)((temp_data >> 8) & 0xff) + 1;
    q_addr = 0x4000 + (page * 64) + (rx_rtc->cnt * 4);
    RfMcu_MemoryGet(q_addr, (uint8_t *)rx_rtc->time, 4);
    tmp_rtc_time      = rx_rtc->time[3]; rx_rtc->time[3] = rx_rtc->time[0]; rx_rtc->time[0] = tmp_rtc_time;
    tmp_rtc_time      = rx_rtc->time[1]; rx_rtc->time[1] = rx_rtc->time[2]; rx_rtc->time[2] = tmp_rtc_time;
    return HOSAL_RF_STATUS_SUCCESS;
}

static hosal_rf_status_t
__rf_15p4_2ch_scan_frequency_set(hosal_rf_15p4_2ch_scan_frequency_t *p_freq) {
    uint8_t cmd_ptr[RUCI_LEN_SET_2CH_SCAN_FREQUENCY];
    SET_RUCI_PARA_SET_2CH_SCAN_FREQUENCY(cmd_ptr, p_freq->scan_enable,
                                         p_freq->rf_freq1, p_freq->rf_freq2);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_2CH_SCAN_FREQUENCY);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_2CH_SCAN_FREQUENCY,
                                RUCI_CODE_SET_2CH_SCAN_FREQUENCY);
}

static hosal_rf_status_t
__rf_15p4_rx_pkt_channel_get(hosal_rf_15p4_rx_pkt_channel_t *rx) {
    uint8_t  channel_tmp;
    uint32_t temp_data;
    uint8_t  page;
    uint16_t q_addr;

    RfMcu_MemoryGet(0x04038, (uint8_t *)&temp_data, 4);
    page = (uint8_t)((temp_data >> 8) & 0xff) + 1;
    if (rx->rx_cnt < 4) {
        q_addr = 0x4000 + (page * 64) + (9 * 4);
        RfMcu_MemoryGet(q_addr, (uint8_t *)&temp_data, 4);
        channel_tmp = (temp_data >> (8 * rx->rx_cnt)) & 0xff;
    } else {
        q_addr = 0x4000 + (page * 64) + (10 * 4);
        RfMcu_MemoryGet(q_addr, (uint8_t *)&temp_data, 4);
        channel_tmp = temp_data & 0xff;
    }
    rx->channel = channel_tmp;
    return HOSAL_RF_STATUS_SUCCESS;
}

static hosal_rf_status_t __rf_15p4_csl_sample_time_update_set(uint32_t time) {
    uint8_t cmd_ptr[RUCI_LEN_UPDATE_CSL_SAMPLE_TIME];
    SET_RUCI_PARA_UPDATE_CSL_SAMPLE_TIME(cmd_ptr, time);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_CSL_SAMPLE_TIME_UPDATE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_UPDATE_CSL_SAMPLE_TIME,
                                RUCI_CODE_UPDATE_CSL_SAMPLE_TIME);
}

static hosal_rf_status_t
__rf_15p4_csl_receiver_ctrl_set(hosal_rf_15p4_csl_receiver_ctrl_t *ctrl) {
    uint8_t cmd_ptr[RUCI_LEN_SET_CSL_RECEIVER_CTRL];
    SET_RUCI_PARA_SET_CSL_RECEIVER_CTRL(cmd_ptr, ctrl->csl_ctrl, ctrl->csl_period);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_CSL_RECEIVER_CTRL);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_CSL_RECEIVER_CTRL,
                                RUCI_CODE_SET_CSL_RECEIVER_CTRL);
}

static hosal_rf_status_t
__rf_sub_qpsk_data_rate_set(hosal_rf_phy_oqpsk_data_rate_t *data_rate) {
    uint8_t cmd_ptr[RUCI_LEN_SET_OQPSK_MODEM];
    SET_RUCI_PARA_SET_OQPSK_MODEM(cmd_ptr, *data_rate);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_OQPSK_MODEM);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_OQPSK_MODEM,
                                RUCI_CODE_SET_OQPSK_MODEM);
}

static hosal_rf_status_t __rf_sub_qpsk_mac_set(hosal_rf_mac_set_t *mac_set) {
    uint8_t cmd_ptr[RUCI_LEN_SET_OQPSK_MAC];
    SET_RUCI_PARA_SET_OQPSK_MAC(cmd_ptr, mac_set->crc_type, mac_set->whiten_enable);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_OQPSK_MAC);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_OQPSK_MAC, RUCI_CODE_SET_OQPSK_MAC);
}

static hosal_rf_status_t __rf_sub_qpsk_preamble_set(uint32_t preamble_len) {
    uint8_t cmd_ptr[RUCI_LEN_SET_OQPSK_EXTRA_PREAMBLE];
    SET_RUCI_PARA_SET_OQPSK_EXTRA_PREAMBLE(cmd_ptr, preamble_len);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_OQPSK_PREAMBLE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_OQPSK_EXTRA_PREAMBLE,
                                RUCI_CODE_SET_OQPSK_EXTRA_PREAMBLE);
}

static hosal_rf_status_t
__rf_sub_fsk_modem_config_set(hosal_rf_modem_config_set_t *config) {
    uint8_t cmd_ptr[RUCI_LEN_SET_FSK_MODEM];
    SET_RUCI_PARA_SET_FSK_MODEM(cmd_ptr, config->data_rate, config->modulation_index);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_FSK_MODEM);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_FSK_MODEM, RUCI_CODE_SET_FSK_MODEM);
}

static hosal_rf_status_t __rf_sub_fsk_mac_set(hosal_rf_mac_set_t *mac_set) {
    uint8_t cmd_ptr[RUCI_LEN_SET_FSK_MAC];
    SET_RUCI_PARA_SET_FSK_MAC(cmd_ptr, mac_set->crc_type, mac_set->whiten_enable);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_FSK_MAC);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_FSK_MAC, RUCI_CODE_SET_FSK_MAC);
}

static hosal_rf_status_t __rf_sub_fsk_preamble_set(uint32_t preamble_len) {
    uint8_t cmd_ptr[RUCI_LEN_SET_FSK_PREAMBLE];
    SET_RUCI_PARA_SET_FSK_PREAMBLE(cmd_ptr, preamble_len);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_FSK_PREAMBLE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_FSK_PREAMBLE, RUCI_CODE_SET_FSK_PREAMBLE);
}

static hosal_rf_status_t __rf_sub_fsk_sfd_set(uint32_t sfd) {
    uint8_t cmd_ptr[RUCI_LEN_SET_FSK_SFD];
    SET_RUCI_PARA_SET_FSK_SFD(cmd_ptr, sfd);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_FSK_SFD);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_FSK_SFD, RUCI_CODE_SET_FSK_SFD);
}

static hosal_rf_status_t __rf_sub_fsk_filter_set(uint32_t filter) {
    uint8_t cmd_ptr[RUCI_LEN_SET_FSK_TYPE];
    SET_RUCI_PARA_SET_FSK_TYPE(cmd_ptr, filter);
    RUCI_ENDIAN_CONVERT(cmd_ptr, RUCI_SET_FSK_TYPE);
    return __rf_sub_command_set(cmd_ptr, RUCI_LEN_SET_FSK_TYPE, RUCI_CODE_SET_FSK_TYPE);
}

static hosal_rf_status_t __rf_15p4_op_pan_idx_set(uint32_t pan_idx) {
    uint32_t temp_data;
    uint8_t  page;
    uint16_t q_addr;

    RfMcu_MemoryGet(0x04038, (uint8_t *)&temp_data, 4);
    page   = (uint8_t)((temp_data >> 8) & 0xFF) + 1;
    q_addr = 0x4000 + (page * 64) + (7 * 4);
    RfMcu_MemorySet(q_addr, (uint8_t *)&pan_idx, 1);
    return HOSAL_RF_STATUS_SUCCESS;
}

/**************************************************************************************************
 *    PUBLIC FUNCTIONS
 *************************************************************************************************/

int hosal_rf_write_command(uint8_t *command_ptr, uint32_t command_len) {
    /* Release FreeRTOS irq_lock before blocking so __rf_proc can run. */
    uint32_t nest = _crit_unlock();
    k_sem_take(&g_rf_cmd_sem, K_FOREVER);
    _crit_relock(nest);
    RfMcu_HostWakeUpMcu();
    if (RfMcu_PowerStateCheck() != 0x03) {
        k_sem_give(&g_rf_cmd_sem);
        k_yield();
        return HOSAL_RF_STATUS_INVALID_STATE;
    }
    if (RF_MCU_TX_CMDQ_SET_SUCCESS
            != RfMcu_CmdQueueSend(command_ptr, (uint32_t)command_len)) {
        k_sem_give(&g_rf_cmd_sem);
        log_warn("Command queue is full");
        k_yield();
        return HOSAL_RF_STATUS_NO_MEMORY;
    }
    return HOSAL_RF_STATUS_SUCCESS;
}

int hosal_rf_write_tx_data(uint8_t *tx_data_ptr, uint32_t tx_data_len) {
    uint32_t reg_val;
    uint8_t  txq_id;
    RF_MCU_TXQ_ERROR tx_q_error;

    RfMcu_HostWakeUpMcu();
    if (RfMcu_PowerStateCheck() != 0x03) {
        k_yield();
        return HOSAL_RF_STATUS_INVALID_STATE;
    }

    RfMcu_MemoryGet(0x0048, (uint8_t *)&reg_val, 4);
    if (!reg_val) {
        /* TX queue register is zero — unexpected hardware state */
        return HOSAL_RF_STATUS_NO_MEMORY;
    }

    if ((reg_val & 0x7F) == 0x00) {
        log_warn("Data queue full");
        k_yield();
        return HOSAL_RF_STATUS_NO_MEMORY;
    }
    for (txq_id = 0; txq_id < 7; txq_id++) {
        if (reg_val & (1 << txq_id)) {
            break;
        }
    }
    tx_q_error = RF_MCU_TXQ_ERR_INIT;
    tx_q_error = RfMcu_TxQueueSendById(txq_id, tx_data_ptr, tx_data_len);
    if (tx_q_error != RF_MCU_TXQ_SET_SUCCESS) {
        log_warn("Tx Q send fail %d", tx_q_error);
        k_yield();
        return HOSAL_RF_STATUS_NO_MEMORY;
    }
    return HOSAL_RF_STATUS_SUCCESS;
}

uint32_t hosal_rf_read_event(uint8_t *event_data_ptr) {
    uint8_t *pdata     = NULL;
    uint32_t event_len = 0;

    /* Release FreeRTOS irq_lock before blocking so COMM_SUBSYSTEM IRQ can
     * fire → __rf_proc runs → puts event in g_rf_evt_msgq → we unblock.   */
    uint32_t nest = _crit_unlock();
    int rc = k_msgq_get(&g_rf_evt_msgq, (void *)&pdata, K_FOREVER);
    _crit_relock(nest);

    if (rc != 0) {
        log_error("RF evt queue get failed\n");
        k_sem_give(&g_rf_cmd_sem);
    } else {
        event_len = pdata[2] + 3;
        memcpy(event_data_ptr, pdata, event_len);
        free(pdata);
    }
    return event_len;
}

uint32_t hosal_rf_read_rx_data(uint8_t *rx_data_ptr) {
    RF_MCU_RXQ_ERROR rx_queue_error = RF_MCU_RXQ_ERR_INIT;
    return (uint32_t)RfMcu_RxQueueRead(rx_data_ptr, &rx_queue_error);
}

hosal_rf_status_t hosal_rf_ioctl(hosal_rf_ioctl_t ctl, void *p_arg) {
    hosal_rf_status_t rval = HOSAL_RF_STATUS_SUCCESS;

    switch (ctl) {
    case HOSAL_RF_IOCTL_MODEM_SET:
        rval = __rf_modem_set((hosal_rf_15p4_modem_cnf_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_FREQUENCY_SET:
        /* lmac15p4 passes frequency as (void*)value, not a pointer */
        rval = __rf_frequency_set((uint32_t)(uintptr_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_RSSI_GET:
        rval = __rf_rssi_get((uint8_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_RX_ENABLE_SET:
        /* subg_ctrl passes value as (void*)value, not a pointer */
        rval = __rf_rx_enable_set((uint32_t)(uintptr_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_SLEEP_SET:
        /* subg_ctrl passes value as (void*)value, not a pointer */
        rval = __rf_sleep_set((uint32_t)(uintptr_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_IDLE_SET:
        rval = __rf_idle_set();
        break;
    case HOSAL_RF_IOCTL_KEY_SET:
        rval = __rf_key_set((uint8_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_PTA_CTRL_SET:
        rval = __rf_pta_ctrl_set((hosal_rf_pta_ctrl_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_TX_START_SET:
        rval = __rf_tx_data_start_set((hosal_rf_tx_data_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_TX_CONTINOUS_WAVE_SET:
        /* lmac15p4 passes value as (void*)value, not a pointer */
        rval = __rf_tx_continuous_wave_set((uint32_t)(uintptr_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_TX_PWR_SET:
        rval = __rf_tx_pwr_set((hosal_rf_tx_power_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_TX_PWR_COMP_SET:
        rval = __rf_tx_pwr_comp_set((hosal_rf_modem_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_TX_PWR_CH_COMP_SET:
        rval = __rf_tx_pwr_ch_comp_set((hosal_rf_tx_power_ch_comp_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_COMP_SEG_SET:
        rval = __rf_tx_pwr_comp_seg_set((hosal_rf_tx_power_comp_seg_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_WAKE_ON_RADIO_SET:
        rval = __rf_wake_on_radio_set((hosal_rf_wake_on_radio_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_MAC_PIB_SET:
        rval = __rf_15p4_mac_pib_set((hosal_rf_15p4_mac_pib_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_PHY_PIB_SET:
        rval = __rf_15p4_phy_pib_set((hosal_rf_15p4_phy_pib_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_ACK_PENDING_BIT_SET:
        rval = __rf_15p4_ack_pending_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_AUTO_ACK_SET:
        rval = __rf_15p4_auto_ack_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_AUTO_STATE_SET:
        rval = __rf_15p4_auto_state_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_ADDRESS_FILTER_SET:
        rval = __rf_15p4_address_filter_set((hosal_rf_15p4_address_filter_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_ACK_PACKET_GET:
        rval = __rf_15p4_ack_packet_get((hosal_rf_15p4_ack_packet_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_SOURCE_ADDRESS_MATCH_SET:
        rval = __rf_15p4_src_match_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_SOURCE_ADDRESS_SHORT_CONTROL_SET:
        rval = __rf_15p4_src_match_short_entry((hosal_rf_15p4_src_match_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_SOURCE_ADDRESS_EXTEND_CONTROL_SET:
        rval = __rf_15p4_src_match_extend_entry((hosal_rf_15p4_src_match_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_FRAME_COUNTER_GET:
        rval = __rf_15p4_frame_counter_get((uint32_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_RTC_TIME_GET:
        rval = __rf_15p4_rtc_time_read((uint32_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_RX_RTC_TIME_GET:
        rval = __rf_15p4_rx_rtc_time_read((hosal_rf_15p4_rx_rtc_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_2CH_SCAN_FREQUENCY_SET:
        rval = __rf_15p4_2ch_scan_frequency_set((hosal_rf_15p4_2ch_scan_frequency_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_RX_DATA_CHANNEL_GET:
        rval = __rf_15p4_rx_pkt_channel_get((hosal_rf_15p4_rx_pkt_channel_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_CSL_SAMPLE_TIME_UPDATE_SET:
        rval = __rf_15p4_csl_sample_time_update_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_CSL_RECEIVER_CTRL_SET:
        rval = __rf_15p4_csl_receiver_ctrl_set((hosal_rf_15p4_csl_receiver_ctrl_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_OQPSK_DATA_RATE_SET:
        rval = __rf_sub_qpsk_data_rate_set((hosal_rf_phy_oqpsk_data_rate_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_OQPSK_MAC_SET:
        rval = __rf_sub_qpsk_mac_set((hosal_rf_mac_set_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_OQPSK_PREAMBLE_SET:
        rval = __rf_sub_qpsk_preamble_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_FSK_MODEM_CONFIG_SET:
        rval = __rf_sub_fsk_modem_config_set((hosal_rf_modem_config_set_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_FSK_MAC_SET:
        rval = __rf_sub_fsk_mac_set((hosal_rf_mac_set_t *)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_FSK_PREAMBLE_SET:
        rval = __rf_sub_fsk_preamble_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_FSK_SFD_SET:
        rval = __rf_sub_fsk_sfd_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_SUBG_FSK_FILTER_SET:
        rval = __rf_sub_fsk_filter_set((uint32_t)p_arg);
        break;
    case HOSAL_RF_IOCTL_15P4_OPERATION_PAN_IDX_SET:
        rval = __rf_15p4_op_pan_idx_set((uint32_t)p_arg);
        break;
    default:
        rval = HOSAL_RF_STATUS_INVALID_PARAMETER;
        break;
    }
    if (rval != HOSAL_RF_STATUS_SUCCESS) {
        log_error("RF IOCTL(%d) status %d", ctl, rval);
    }
    return rval;
}

int hosal_rf_callback_set(int callback_type, hosal_rf_callback_t pfn_callback,
                          void *arg) {
    switch (callback_type) {
    case HOSAL_RF_PCI_EVENT_CALLBACK: g_pci_event_cb  = pfn_callback; break;
    case HOSAL_RF_PCI_RX_CALLBACK:  g_pci_rx_done_cb  = pfn_callback; break;
    case HOSAL_RF_PCI_TX_CALLBACK:  g_pci_tx_done_cb  = pfn_callback; break;
    case HOSAL_RF_BLE_EVENT_CALLBACK: g_hci_evt_cb    = pfn_callback; break;
    case HOSAL_RF_BLE_RX_CALLBACK:  g_hci_data_cb     = pfn_callback; break;
    default: return HOSAL_RF_STATUS_INVALID_PARAMETER;
    }
    return HOSAL_RF_STATUS_SUCCESS;
}

void hosal_rf_init(hosal_rf_mode_t mode)
{
    printk("[RF] hosal_rf_init entry mode=0x%x\n", (unsigned)mode);
    NVIC_SetPriority(CommSubsystem_IRQn, 0x4);
    /* DmaInit only enables NVIC + DMA interrupt (no RESET) — matches
     * reference rt584 and is safe to call before rf_common_init_by_fw. */
    RfMcu_DmaInit();
    if (rf_common_init_by_fw(mode, __rf_event_callback) != true) {
        printk("[RF] init fw fail — halting\n");
        k_panic();
    }

    /* g_rf_cmd_sem is statically initialised to 1 (given) — no runtime init. */
    /* g_rf_notify_sem is statically initialised to 0 (taken) — no runtime init. */

    /* Launch the RF event processing thread. */
    k_tid_t tid = k_thread_create(&g_rf_thread, g_rf_stack,
                                  K_THREAD_STACK_SIZEOF(g_rf_stack),
                                  __rf_proc,
                                  NULL, NULL, NULL,
                                  RF_PROC_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(tid, "hosal-rf");

    /* g_rf_evt_msgq is statically initialised — no xQueueCreate needed. */

    __rf_tx_data_queue_init();
}

/* Force-reset the command semaphore to "available" state.
 * Used by test harnesses after a command timeout to unblock
 * subsequent hosal_rf_write_command() calls. */
void hosal_rf_cmd_sem_reset(void)
{
    k_sem_reset(&g_rf_cmd_sem);
    k_sem_give(&g_rf_cmd_sem);
}
