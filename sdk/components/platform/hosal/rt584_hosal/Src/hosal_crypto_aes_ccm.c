/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_aes_ccm.c
 * \brief           hosal_crytpo_aes_ccm file
 */

/*
 * This file is part of library_name.
 * Author:
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mcu.h"
#include "hosal_crypto_aes_ccm.h"


int hosal_crypto_aes_ccm_init(void) {

    int status;

    status = STATUS_SUCCESS;

    crypto_lib_init();

    return status;
}


int hosal_crypto_aes_ccm_operation(hosal_aes_ccm_dev_t* aes_ccm_dev) {

    int status;
    struct aes_ccm_encryption_packet  ccm_input_packet;
    struct aes_ccm_decryption_packet  ccm_payload_packet;
    struct aes_ctx ctx;

    status = STATUS_SUCCESS;

    aes_acquire(&ctx);

    aes_key_init(&ctx, aes_ccm_dev->key_ptr, aes_ccm_dev->bit);

    aes_load_round_key(&ctx);

    if (aes_ccm_dev->bit == HOSAL_AES_128_BIT) {
        if (aes_ccm_dev->crypto_operation == HOSAL_AES_CCM_ENCRYPT) {

            ccm_input_packet.nonce = (uint8_t*)aes_ccm_dev->nonce;
            ccm_input_packet.hdr = (uint8_t*) aes_ccm_dev->hdr;
            ccm_input_packet.hdr_len = aes_ccm_dev->hdr_len;
            ccm_input_packet.data = (uint8_t*) aes_ccm_dev->data;
            ccm_input_packet.data_len = aes_ccm_dev->data_len;
            ccm_input_packet.mlen = aes_ccm_dev->mlen;
            ccm_input_packet.out_buf =  aes_ccm_dev->out_buf;
            ccm_input_packet.out_buf_len = aes_ccm_dev->out_buf_len;

            status = aes_ccm_encryption( &ccm_input_packet);

        } else if (aes_ccm_dev->crypto_operation == HOSAL_AES_CCM_DECRYPT) {

            ccm_payload_packet.payload_buf = aes_ccm_dev->payload_buf;
            ccm_payload_packet.nonce = (uint8_t*) aes_ccm_dev->nonce;
            ccm_payload_packet.hdr_len = aes_ccm_dev->hdr_len;
            ccm_payload_packet.data_len = aes_ccm_dev->data_len;
            ccm_payload_packet.mlen = aes_ccm_dev->mlen;
            ccm_payload_packet.out_buf = aes_ccm_dev->out_buf;
            ccm_payload_packet.out_buf_len = aes_ccm_dev->out_buf_len;

            status = aes_ccm_decryption_verification( &ccm_payload_packet);
        }
    }

    aes_release(&ctx);

    return status;
}










