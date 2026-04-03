/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * @file lmac15p4.h
 * @author Rex Huang (rex.huang@rafaelmicro.com)
 * @brief 802.15.4 MAC defination & API header file.
 * @version 0.1
 * @date 2023-07-26
 */

#ifndef __LMAC154_H__
#define __LMAC154_H__

/**************************************************************************************************
 *    INCLUDES
 *************************************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup lmac15p4 MAC API Definition
 * \ingroup 15P4_group
 * \brief Define 802.15.4 MAC API definitions, structures, and functions.
 * @{
 */
/**************************************************************************************************
 *    TYPEDEFS
 *************************************************************************************************/
/**
 * \brief           MAC modulation and band enum definitions.
 */
typedef enum {
    LMAC15P4_SUBG_FSK = 1,   /*!< Sub 1GHz band FSK modulation */
    LMAC15P4_2P4G_OQPSK = 2, /*!< 2.4 GHz band OQPSK modulation */
    LMAC15P4_SUBG_OQPSK = 4, /*!< Sub 1GHz band OQPSK modulation */
} lmac15p4_modem_t;

/**
 * \brief           MAC 2.4 GHz band channel enum definitions.
 */
typedef enum {
    LMAC154_CHANNEL_11 = 0, /*!< 2.4 GHz band: 2405 MHz */
    LMAC154_CHANNEL_12,     /*!< 2.4 GHz band: 2410 MHz */
    LMAC154_CHANNEL_13,     /*!< 2.4 GHz band: 2415 MHz */
    LMAC154_CHANNEL_14,     /*!< 2.4 GHz band: 2420 MHz */
    LMAC154_CHANNEL_15,     /*!< 2.4 GHz band: 2425 MHz */
    LMAC154_CHANNEL_16,     /*!< 2.4 GHz band: 2430 MHz */
    LMAC154_CHANNEL_17,     /*!< 2.4 GHz band: 2435 MHz */
    LMAC154_CHANNEL_18,     /*!< 2.4 GHz band: 2440 MHz */
    LMAC154_CHANNEL_19,     /*!< 2.4 GHz band: 2445 MHz */
    LMAC154_CHANNEL_20,     /*!< 2.4 GHz band: 2450 MHz */
    LMAC154_CHANNEL_21,     /*!< 2.4 GHz band: 2455 MHz */
    LMAC154_CHANNEL_22,     /*!< 2.4 GHz band: 2460 MHz */
    LMAC154_CHANNEL_23,     /*!< 2.4 GHz band: 2465 MHz */
    LMAC154_CHANNEL_24,     /*!< 2.4 GHz band: 2470 MHz */
    LMAC154_CHANNEL_25,     /*!< 2.4 GHz band: 2475 MHz */
    LMAC154_CHANNEL_26,     /*!< 2.4 GHz band: 2480 MHz */
} lmac154_channel_t;

/**
 * \brief           MAC TX done status enum definitions.
 */
typedef enum {
    LMAC154_TX_SUCCESS = 0x0,      /*!< MAC TX success */
    LMAC154_TX_FAIL = 0x08,        /*!< MAC TX fail */
    LMAC154_TX_CSMACA_FAIL = 0x10, /*!< MAC TX fail due to CSMA-CA fail */
    LMAC154_TX_NO_ACK_FAIL = 0x20, /*!< MAC TX but doesn't get ack fail */
    LMAC154_TX_GET_ACK_FP_SUCCESS =
        0x40, /*!< MAC TX and get ack with pending bit success */
    LMAC154_TX_GET_ACK_SUCCESS = 0x80, /*!< MAC TX and get ack success */
} lmac154_tx_status_t;

/**
 * \brief           Soruce matching function control type.
 */
typedef enum {
    CLEAR_ALL = 0, /*!< Clear all entries */
    ADD_ADDRESS_PENDING =
        1, /*!< Set pending bit to 1 in ack frames to specified address */
    REMOVE_ADDRESS_PENDING =
        2, /*!< Set pending bit to 0 in ack frames to specified address */
    ADD_ADDRESS_LQI =
        3, /*!< Add LQI field in ack frames to specified address */
    REMOVE_ADDRESS_LQI =
        4, /*!< Remove LQI field in ack frames to specified address */
    ADD_ADDRESS_LINK_MARGIN =
        5, /*!< Add link margin field in ack frames to specified address */
    REMOVE_ADDRESS_LINK_MARGIN =
        6, /*!< Remove link margin field in ack frames to specified address */
    ADD_ADDRESS_RSSI =
        7, /*!< Add rssi field in ack frames to specified address */
    REMOVE_ADDRESS_RSSI =
        8, /*!< Remove rssi field in ack frames to specified address */
} src_match_ctrl_t;

/**
 * \brief           MAC RX done call back function.
 * \param[in]       packet_length: RX packet length.
 * \param[in]       pdata: address of RX packet data.
 * \param[in]       crc_status: [0: CRC correct; others: CRC error].
 * \param[in]       rssi: received signal strength indication.
 * \param[in]       snr: signal-to-noise ratio.
 * \return          None
 */
typedef void(pf_rx_done_cb)(uint16_t packet_length, uint8_t* pdata,
                            uint8_t crc_status, uint8_t rssi, uint8_t snr);

/**
 * \brief           MAC TX done call back function.
 */
typedef void(pf_tx_done_cb)(uint32_t tx_status);

/**
 * \brief           MAC interrupt call back function structure.
 */
typedef struct {
    pf_rx_done_cb* rx_cb;
    pf_tx_done_cb* tx_cb;
} lmac15p4_callback_t;

/**************************************************************************************************
 *    GLOBAL PROTOTYPES
 *************************************************************************************************/
/**
 * \brief           MAC RX done call back function.
 * \param[in]       packet_length: RX packet length.
 * \param[in]       pdata: address of RX packet data.
 * \param[in]       crc_status: [0: CRC correct; others: CRC error].
 * \param[in]       rssi: received signal strength indication.
 * \param[in]       snr: signal-to-noise ratio.
 * \return          None
 */
void lmac15p4_RxDoneEvent(uint16_t packet_length, uint8_t* pdata,
                          uint8_t crc_status, uint8_t rssi, uint8_t snr);

/**
 * \brief           MAC TX done call back function.
 * \param[in]       tx_status: TX status, see lmac154_tx_status_t.
 * \return          None
 */
void lmac15p4_TxDoneEvent(uint32_t tx_status);

/**
 * \brief           MAC initialization function.
 * \param[in]       modem: modulation type.
 * \param[in]       band_type: band type, only used in SubG init initialization.
 * \return          0
 */
int lmac15p4_init(lmac15p4_modem_t modem, uint8_t band_type);

/**
 * \brief           MAC interrupt call back function register function.
 * \param[in]       pan_idx: 0 ~ 1, usually set to 0, set to 1 only when using Multi-PAN function
 * \param[in]       callback_set: address of MAC interrupt call back function structure.
 * \return          None
 */
void lmac15p4_cb_set(uint32_t pan_idx, lmac15p4_callback_t* callback_set);

/**
 * \brief           MAC RSSI reading function.
 * \param[in]       None
 * \return          RSSI: received signal strength indication in -dBm.
 */
uint8_t lmac15p4_read_rssi(void);

/**
 * \brief           Get FW version, temporarily not used.
 */
char* lmac15p4_get_version(void);

/**
 * \brief           MAC TX data function.
 * \param[in]       pan_idx: 0 ~ 1, usually set to 0, set to 1 only when using Multi-PAN function
 * \param[in]       tx_data_address: address of TX data.
 * \param[in]       packet_length: length of TX data.
 * \param[in]       mac_control: TX control byte.
 * \param[in]       mac_dsn: MAC sequence number.
 * \return          TX queue write status: [0: success; 1: TX queue full fail]
 */
int lmac15p4_tx_data_send(uint32_t pan_idx, uint8_t* tx_data_address,
                          uint16_t packet_length, uint8_t mac_control,
                          uint8_t mac_dsn);

/**
 * \brief           Setting MAC address filter.
 * \param[in]       pan_idx: 0 ~ 1, usually set to 0, set to 1 only when using Multi-PAN function
 * \param[in]       mac_promiscuous_mode: whether to activate FW MAC address filter. [0: Yes; 1: No]
 * \param[in]       short_source_address: MAC 16-bit short address
 * \param[in]       long_source_address_0: the 4 most significant bytes of MAC 64-bit extended address
 * \param[in]       long_source_address_1: the 4 least significant bytes of MAC 64-bit extended address
 * \param[in]       pan_id: MAC 16-bit PAN ID
 * \param[in]       is_coordinator: whether this device is a coordinator
 * \return          None
 */
void lmac15p4_address_filter_set(uint32_t pan_idx, uint8_t mac_promiscuous_mode,
                                 uint16_t short_source_address,
                                 uint32_t long_source_address_0,
                                 uint32_t long_source_address_1,
                                 uint16_t pan_id, uint8_t is_coordinator);

/**
 * \brief           Setting MAC PIB attributes.
 * \param[in]       a_unit_backoff_period: the number of symbols forming the basic time period used by the CSMA-CA algorithm
 * \param[in]       mac_ack_wait_duration: time to wait for an ack frame after sending a packet requesting an ack
 * \param[in]       mac_max_BE: the maximum value of the backoff exponent (BE) in the CSMA-CA algorithm
 * \param[in]       mac_max_CSMA_backoffs: the maximum number of back offs due to cca failures
 * \param[in]       mac_max_frame_total_wait_time: time to wait for a normal frame after receiving an ack with frame pending bit 1
 * \param[in]       mac_max_frame_retries: The maximum number of retries due to not receiving ack
 * \param[in]       mac_min_BE: the minimum value of the backoff exponent (BE) in the CSMA-CA algorithm
 * \return          None
 */
void lmac15p4_mac_pib_set(uint32_t a_unit_backoff_period,
                          uint32_t mac_ack_wait_duration, uint8_t mac_max_BE,
                          uint8_t mac_max_CSMA_backoffs,
                          uint32_t mac_max_frame_total_wait_time,
                          uint8_t mac_max_frame_retries, uint8_t mac_min_BE);

/**
 * \brief           Setting PHY PIB attributes.
 * \param[in]       a_turnaround_time: RX to TX or TX to RX turnaround time
 * \param[in]       phy_cca_mode: CCA mode, see cca_mode_t
 * \param[in]       phy_cca_threshold: power threshold (-dBm) used by CCA
 * \param[in]       phy_cca_duration: CCA duration
 * \return          None
 */
void lmac15p4_phy_pib_set(uint16_t a_turnaround_time, uint8_t phy_cca_mode,
                          uint8_t phy_cca_threshold, uint16_t phy_cca_duration);

/**
 * \brief           Enable/disable automatic acking function
 * \param[in]       auto_ack_enable: whether to activate automatic acking function
 * \return          None
 */
void lmac15p4_auto_ack_set(uint32_t auto_ack_enable);

/**
 * \brief           Enable/disable RF RX
 * \param[in]       auto_state_enable: whether to enable RX
 * \return          None
 */
void lmac15p4_auto_state_set(uint32_t auto_state_enable);

/**
 * \brief           Setting RF frequency to 2.4G channel
 * \param[in]       ch: 2.4G channel, see lmac154_channel_t
 * \return          None
 */
void lmac15p4_channel_set(lmac154_channel_t ch);

/**
 * \brief           Setting pending bit for automatic acking function
 * \param[in]       pan_idx: 0 ~ 1, usually set to 0, set to 1 only when using Multi-PAN function
 * \param[in]       pending_bit_enable: pending bit in ack frame
 * \return          None
 */
void lmac15p4_ack_pending_bit_set(uint32_t pan_idx,
                                  uint32_t pending_bit_enable);

/**
 * \brief           Start/stop TX continuous wave
 * \param[in]       tx_enable: [0: stop; 1: start]
 * \return          None
 */
void lmac15p4_tx_continuous_wave_send(uint32_t tx_enable);

/**
 * \brief           Enable/disable source matching function
 * \param[in]       pan_idx: 0 ~ 1, usually set to 0, set to 1 only when using Multi-PAN function
 * \param[in]       enable: whether to enable source matching function
 * \return          None
 */
void lmac15p4_src_match_ctrl(uint32_t pan_idx, uint32_t enable);

/**
 * \brief           Short address control for source matching function
 * \param[in]       control_type: control type, see src_match_ctrl_t
 * \param[in]       short_addr: address of short address
 * \return          None
 */
void lmac15p4_src_match_short_entry(uint8_t control_type, uint8_t* short_addr);

/**
 * \brief           Extended address control for source matching function
 * \param[in]       control_type: control type, see src_match_ctrl_t
 * \param[in]       extended_addr: address of extended address
 * \return          None
 */
void lmac15p4_src_match_extended_entry(uint8_t control_type,
                                       uint8_t* extended_addr);

/**
 * \brief           Setting CSL IE control and parameters
 * \param[in]       csl_receiver_ctrl: whether to add CSL IE in ack frame
 * \param[in]       csl_period: CSL period (used to calculate CSL phase)
 * \return          None
 */
void lmac15p4_csl_receiver_ctrl(uint8_t csl_receiver_ctrl, uint16_t csl_period);

/**
 * \brief           Getting CSL accuracy, temporarily not used (fixed value)
 * \param[in]       csl_accuracy: address for storing CSL accuracy
 * \return          None
 */
void lmac15p4_csl_accuracy_get(uint8_t* csl_accuracy);

/**
 * \brief           Getting CSL uncertainty, temporarily not used (fixed value)
 * \param[in]       csl_accuracy: address for storing CSL uncertainty
 * \return          None
 */
void lmac15p4_csl_uncertainty_get(uint8_t* csl_uncertainty);

/**
 * \brief           Updating CSL sample time
 * \param[in]       csl_sample_time: new CSL sample time
 * \return          None
 */
void lmac15p4_csl_sample_time_update(uint32_t csl_sample_time);

/**
 * \brief           Reading received ack frame
 * \param[in]       ack_data: address for storing ack data
 * \param[in]       ack_time: address for storing ack RX time
 * \param[in]       is2bytephr: whether PHR is two-byte
 * \return          Ack packet length
 */
uint32_t lmac15p4_read_ack(uint8_t* ack_data, uint8_t* ack_time,
                           uint8_t is2bytephr, uint8_t* rx_rssi);
void lmac15p4_key_set(uint32_t pan_idx, uint8_t* key);

/**
 * \brief           Reading RTC time
 * \param[in]       rtc_time: address for storing RTC time
 * \return          None
 */
void lmac15p4_rtc_time_read(uint32_t* rtc_time);

/**
 * \brief           Reading received packets RX time
 * \param[in]       rx_cnt: RX done times % 5 (0 ~ 4)
 * \param[in]       rx_rtc_time: address for storing RTC time
 * \return          None
 */
void lmac15p4_rx_rtc_time_read(uint8_t rx_cnt, uint32_t* rx_rtc_time);

/**
 * \brief           Reading frame_counter
 * \param[in]       None
 * \return          frame_counter
 */
uint32_t lmac15p4_frame_counter_get(void);

/**
 * \brief           Setting two-channel scan parameters
 * \param[in]       eanble: whether to enable two-channel scan RX
 * \param[in]       ch1: scanning channel 1, see lmac154_channel_t
 * \param[in]       ch2: scanning channel 2, see lmac154_channel_t
 * \return          None
 */
void lmac15p4_2ch_scan_set(uint32_t eanble, lmac154_channel_t ch1,
                           lmac154_channel_t ch2);

/*@}*/ /* end of 15P4_group lmac15p4 */

#ifdef __cplusplus
}
#endif
#endif