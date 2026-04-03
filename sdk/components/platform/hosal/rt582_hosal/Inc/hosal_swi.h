/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_swi.h
 * \brief           hosal_swi software interrupt include file
 */

/*
 * This file is part of library_name.
 * Author:         ives.lee
 */
#ifndef HOSAL_SWI_H
#define HOSAL_SWI_H



#ifdef __cplusplus
extern "C" {
#endif

#include "swi.h"


/**
 * \defgroup HOSAL_SWI Hosal swi
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal swi definitions, structures, and functions
 * @{
 */


/**
 * \brief           software interrupt typedef callback function
 */
typedef void (*hosal_swi_cb)(uint32_t id);

/**
 * \brief           software interrupt id defined
 */
typedef enum{
    HOSAL_SWI0_ID = 0,
	HOSAL_SWI_ID_MAX = 1,
}hosal_swi_port_t;

/**
 * \brief           softwarer interrupt id number
 */
typedef enum {
    HOSAL_TRIG_0 = 0,                         /**< software interrupt identifier  0. */
    HOSAL_TRIG_1 = 1,                         /**< software interrupt identifier  1. */
    HOSAL_TRIG_2 = 2,                         /**< software interrupt identifier  2. */
    HOSAL_TRIG_3 = 3,                         /**< software interrupt identifier  3. */
    HOSAL_TRIG_4 = 4,                         /**< software interrupt identifier  4. */
    HOSAL_TRIG_5 = 5,                         /**< software interrupt identifier  5. */
    HOSAL_TRIG_6 = 6,                         /**< software interrupt identifier  6. */
    HOSAL_TRIG_7 = 7,                         /**< software interrupt identifier  7. */
    HOSAL_TRIG_8 = 8,                         /**< software interrupt identifier  8. */
    HOSAL_TRIG_9 = 9,                         /**< software interrupt identifier  9. */
    HOSAL_TRIG_10 = 10,                       /**< software interrupt identifier  10. */
    HOSAL_TRIG_11 = 11,                       /**< software interrupt identifier  11. */
    HOSAL_TRIG_12 = 12,                       /**< software interrupt identifier  12. */
    HOSAL_TRIG_13 = 13,                       /**< software interrupt identifier  13. */
    HOSAL_TRIG_14 = 14,                       /**< software interrupt identifier  14. */
    HOSAL_TRIG_15 = 15,                       /**< software interrupt identifier  15. */
    HOSAL_TRIG_16 = 16,                       /**< software interrupt identifier  16. */
    HOSAL_TRIG_17 = 17,                       /**< software interrupt identifier  17. */
    HOSAL_TRIG_18 = 18,                       /**< software interrupt identifier  18. */
    HOSAL_TRIG_19 = 19,                       /**< software interrupt identifier  19. */
    HOSAL_TRIG_20 = 20,                       /**< software interrupt identifier  20. */
    HOSAL_TRIG_21 = 21,                       /**< software interrupt identifier  21. */
    HOSAL_TRIG_22 = 22,                       /**< software interrupt identifier  22. */
    HOSAL_TRIG_23 = 23,                       /**< software interrupt identifier  23. */
    HOSAL_TRIG_24 = 24,                       /**< software interrupt identifier  24. */
    HOSAL_TRIG_25 = 25,                       /**< software interrupt identifier  25. */
    HOSAL_TRIG_26 = 26,                       /**< software interrupt identifier  26. */
    HOSAL_TRIG_27 = 27,                       /**< software interrupt identifier  27. */
    HOSAL_TRIG_28 = 28,                       /**< software interrupt identifier  28. */
    HOSAL_TRIG_29 = 29,                       /**< software interrupt identifier  29. */
    HOSAL_TRIG_30 = 30,                       /**< software interrupt identifier  30. */
    HOSAL_TRIG_31 = 31,                       /**< software interrupt identifier  31. */
    HOSAL_MAX_NUMBER_OF_SWI = MAX_NUMBER_OF_SWI,    /**< Max SWI Number 32. */
} hosal_trig_sel_t;


/**
 * \brief           Software interrupt initialize
 * \param[in]       NONE 
 * \return          return function status
 */
int hosal_swi_uninit(void);

/**
 * \brief           Software interrupt uninitializ
 * \param[in]       NONE 
 * \return          return function status
 */
int hosal_swi_init(void);

/**
 * \brief           Trigger software interrupt
 * \param[in]       swi_id: software interrupt identification 
 * \return          return function status
 */
int hosal_swi_trigger(uint32_t swi_port, uint32_t swi_trig_bit);

/**
 * \brief           Software interrupt register callback fucntion
 * \param[in]       swi_id:  software interrupt identification 
 * \param[in]       hosal_swi_callback_fn: call back function
 * \return          return function status
 */
int hosal_swi_callback_register(uint32_t swi_port, uint32_t swi_trig_bit, hosal_swi_cb cb);

/**
 * \brief           Software interrupt unregister callback fucntion
 * \param[in]       swi_id: First number
 * \return          return function status
 */
int hosal_swi_callback_unregister(uint32_t swi_port, uint32_t swi_trig_bit);

/*@}*/ /* end of RT58X_HOSAL HOSAL_SWI */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_SWI_H */
