/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            ringbuffer.h
 * \brief           ring buffer header file
 */
/*
 * Author:          
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \defgroup RING_BUFFER Ring buffer
 * \ingroup RT58X_DRIVER
 * \brief  Define Ring buffer definitions, structures, and functions
 * @{
 */

#define bufsize_mask(X)    (X-1)

typedef struct {
    uint8_t* ring_buf;
    uint16_t bufsize_mask;                      /*buffer size should be 2^N, this value is buffersize-1 */
    volatile uint16_t wr_idx;                   /*Notice: this write index must be volatile*/
    volatile uint16_t rd_idx;                   /*Notice: this read index must be volatile*/
} ring_buf_t;

/*@}*/ /* end of RT58X_DRIVER RINGBUFFER */

#ifdef __cplusplus
}
#endif

#endif /* End of RINGBUFFER_H */
