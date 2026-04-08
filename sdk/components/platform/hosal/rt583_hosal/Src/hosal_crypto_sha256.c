/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_crytpo_sha256.c
 * \brief           hosal_crytpo_sha256 file
 */
/*
 * This file is part of library_name.
 * Author:
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "hosal_crypto_sha256.h"
#include "hosal_status.h"
#include "mcu.h"

int hosal_crypto_sha256_init(void) {

    int status = HOSAL_STATUS_SUCCESS;

    sha256_vector_init();

    return status;
}

int hosal_crypto_sha256_operation(hosal_sha256_dev_t* sha256_dev) {
    int status;
    status = HOSAL_STATUS_SUCCESS;

    if (sha256_dev->crypto_operation == HOSAL_SHA256_HMAC) {
        hmac_sha256(sha256_dev->key_ptr, sha256_dev->key_length,
                    sha256_dev->in_ptr, sha256_dev->in_length,
                    sha256_dev->out_ptr);
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_HKDF) {
        status = hkdf_sha256(sha256_dev->out_ptr, sha256_dev->out_len,
                             sha256_dev->in_ptr, sha256_dev->in_length,
                             sha256_dev->salt, sha256_dev->salt_len,
                             sha256_dev->info, sha256_dev->info_len);
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_DIGEST) {
        sha256_init(&sha256_dev->sha_cnxt);	
        sha256_update(&sha256_dev->sha_cnxt, sha256_dev->in_ptr,
                      sha256_dev->in_length); //test sha256("abc")
        sha256_finish(&sha256_dev->sha_cnxt, sha256_dev->out_ptr);                           
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_DIGEST_INIT) {
        sha256_init(&sha256_dev->sha_cnxt);
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_DIGEST_UPDATE) {
        sha256_update(&sha256_dev->sha_cnxt, sha256_dev->in_ptr,
                      sha256_dev->in_length); //test sha256("abc")
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_DIGEST_FINISH) {
        sha256_finish(&sha256_dev->sha_cnxt, sha256_dev->out_ptr);
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_PBKDF2_HMAC) {
        status = pbkdf2_hmac(sha256_dev->pbkdf2);
    } else {
        return -1;
    }

    return status;
}
