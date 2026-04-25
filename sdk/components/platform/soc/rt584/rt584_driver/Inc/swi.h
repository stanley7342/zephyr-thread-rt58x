/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           swi.h
 * \brief          software interrupt defineds header file
 */
/*
 * This file is part of library_name.
 * Author:
 */


#ifndef SWI_H
#define SWI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * \defgroup        SWI Swi
 * \ingroup         RT584_DRIVER
 * \brief           Define Swi definitions, structures, and functions
 * @{
 */


/**
 * \brief           software interrupt const define
 */
#define MAX_NUMBER_OF_SOFT_INT              2
#define MAX_NUMBER_OF_SWI                   32

/**
 * \brief           software interrupt enum define
 */
typedef enum {
    SWI_ID_0 = 0,                                /*!< swi0 idefinitions */
    SWI_ID_1 = 1,                                /*!< swi1 idefinitions */
    SWI_MAX
} swi_id_t;

/**
 * \brief           
 */
typedef void (*swi_isr_handler_t)(uint32_t id);

/**
 * \brief trigger Software interrupt
 */
uint32_t swi_int_trigger(uint32_t id, uint32_t swi_trig_id);


/**
 * \brief trigger Software interrupt register callback
 */
uint32_t swi_int_callback_register(uint32_t id, uint32_t trig_bit,swi_isr_handler_t swi_cb_fun);

/**
 * \brief clear Software interrupt
 */
uint32_t swi_int_clear(uint32_t id, uint32_t swi_trig_id);

/**
 * \brief enable Software interrupt
 */
uint32_t swi_int_enable(uint32_t id);

/**
 * \brief disable Software interrupt
 */
uint32_t swi_int_disable(uint32_t id, uint32_t swi_trig_id);

/**
 * \brief clear Software interrupt callback function
 */
uint32_t swi_int_callback_clear(uint32_t id);

/**
 * \brief get Software interrupt data
 */
uint32_t swi_get_int_data(uint32_t id, uint32_t* data);

/**
 * \brief set Software interrupt data
 */
uint32_t swi_set_int_data(uint32_t id, uint32_t data);

/*@}*/ /* end of RT584_DRIVER SWI */

#ifdef __cplusplus
}
#endif

#endif /* End of SWI_H */
