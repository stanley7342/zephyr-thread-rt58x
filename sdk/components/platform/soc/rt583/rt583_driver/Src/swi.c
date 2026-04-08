/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            swi.c
 * \brief           software interrupt driver file
 */
/*
 * Author:          ives.lee
 */

#include "swi.h"
#include "mcu.h"

static swi_isr_handler_t swi_reg_handler[MAX_NUMBER_OF_SWI];

/**
 * \brief           Software interrupt handler
 * \details         Clear the software interrupt status and handle the software interrupt routine
 */
void sw_handler(void) {
    uint32_t i;
    uint32_t swi_int_reg;

    swi_isr_handler_t cb_fun;
    do {
        swi_int_reg = SWI_INT_STATUS_GET();
        SWI_INT_CLEAR(swi_int_reg);
    } while (SWI_INT_STATUS_GET());

    // if (SWI_INT_STATUS_GET()) {
    // ASSERT();
    // }

    if (swi_int_reg) {

        for (i = 0; i < MAX_NUMBER_OF_SWI; i++) {

            cb_fun = swi_reg_handler[i];

            if ((swi_int_reg & (1 << i)) && (cb_fun != NULL)) {

                cb_fun(i);
            }
        }
    }

    return;
}

uint32_t swi_int_callback_register(uint32_t id, uint32_t trig_bit,
                                   swi_isr_handler_t swi_cb_fun) {

    if (id >= SWI_MAX) {
        return STATUS_INVALID_PARAM;
    }

    swi_reg_handler[trig_bit] = swi_cb_fun;

    return STATUS_SUCCESS;
}

/**
 * \brief           Unregister SWI interrupt service routine callback.
 * \param[in]       swi_id: Software interrupt ID
                        \arg SWI_ID_0 ~ SWI_ID_31
 */
uint32_t swi_int_callback_unregister(uint32_t id, uint32_t trig_bit) {

    uint32_t i;

    for (i = id; i < MAX_NUMBER_OF_SWI; i++) {

        swi_reg_handler[trig_bit] = NULL;
    }

    return STATUS_SUCCESS;
}

uint32_t swi_int_callback_clear(uint32_t id) {

    uint32_t i;

    for (i = 0; i < MAX_NUMBER_OF_SWI; i++) {

        swi_int_callback_unregister(id, i);
    }

    return STATUS_SUCCESS;
}

uint32_t swi_int_enable(uint32_t id, uint32_t swi_id,
                        swi_isr_handler_t swi_int_callback) {

    uint32_t swi_mask;

    if (swi_id < MAX_NUMBER_OF_SWI) {

        swi_int_callback_register(id, swi_id, swi_int_callback);

        swi_mask = (1 << swi_id);

        NVIC_EnableIRQ((IRQn_Type)(Soft_IRQn));

        SWI_INT_CLEAR(swi_mask);
        SWI_INT_ENABLE(swi_mask);
    }

    return STATUS_SUCCESS;
}

uint32_t swi_int_disable(uint32_t id, uint32_t trig_id) {
    uint32_t swi_mask;

    if (trig_id < MAX_NUMBER_OF_SWI) {

        swi_int_callback_unregister(id, trig_id);

        swi_mask = (1 << trig_id);

        SWI_INT_DISABLE(swi_mask);
        SWI_INT_CLEAR(swi_mask);

        if (SWI_INT_ENABLE_GET() == NULL) {
            NVIC_DisableIRQ((IRQn_Type)(Soft_IRQn));
        }
    }

    return STATUS_SUCCESS;
}

uint32_t swi_int_trigger(uint32_t id, uint32_t trig_id) {

    uint32_t swi_mask;

    if (trig_id < MAX_NUMBER_OF_SWI) {

        swi_mask = (1 << trig_id);

        SWI_INT_STATUS_SET(swi_mask);
    }

    return STATUS_SUCCESS;
}
