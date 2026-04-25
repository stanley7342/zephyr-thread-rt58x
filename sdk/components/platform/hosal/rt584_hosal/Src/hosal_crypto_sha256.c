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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "mcu.h"
#include "hosal_crypto_sha256.h"


int hosal_crypto_sha256_init(void) {

    int status = STATUS_SUCCESS;

    crypto_lib_init();

    return status;
}


int hosal_crypto_sha256_operation(hosal_sha256_dev_t* sha256_dev) {
    int status;
    sha256_context sha_cnxt;
    hkdf_context hkdf_sha256_dev;

    status = STATUS_SUCCESS;

    if (sha256_dev->crypto_operation == HOSAL_SHA256_HMAC) {
        hmac_sha256(sha256_dev->key_ptr, sha256_dev->key_length, sha256_dev->in_ptr,
                    sha256_dev->in_length, sha256_dev->out_ptr);
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_HKDF) {

        hkdf_sha256_dev.out_key = sha256_dev->out_ptr;
        hkdf_sha256_dev.out_len =  sha256_dev->out_len;

        hkdf_sha256_dev.secret = sha256_dev->in_ptr;
        hkdf_sha256_dev.secret_len = sha256_dev->in_length;

        hkdf_sha256_dev.salt =  sha256_dev->salt;
        hkdf_sha256_dev.salt_len = sha256_dev->salt_len;

        hkdf_sha256_dev.info = sha256_dev->info;
        hkdf_sha256_dev.info_len =  sha256_dev->info_len;

        status = hkdf_sha256(&hkdf_sha256_dev);
        
	 } else if (sha256_dev->crypto_operation == HOSAL_SHA256_DIGEST) {
        sha256_init(&sha256_dev->sha_cnxt);	
        sha256_starts(&sha256_dev->sha_cnxt, 0);
        sha256_update(&sha256_dev->sha_cnxt, sha256_dev->in_ptr,
                      sha256_dev->in_length); //test sha256("abc")
        sha256_finish(&sha256_dev->sha_cnxt, sha256_dev->out_ptr);              
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_DIGEST_INIT) {
        sha256_init(&sha256_dev->sha_cnxt);
    } else if (sha256_dev->crypto_operation == HOSAL_SHA256_DIGEST_STARTS) {
#if defined(CONFIG_RT584H) || defined(CONFIG_RT584HA4) || defined(CONFIG_RT584L) || defined(CONFIG_RF1301)
        sha256_starts(&sha256_dev->sha_cnxt, 0);
#endif
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

