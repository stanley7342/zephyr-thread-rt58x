/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_aux_com.c
 * \brief           hosal aux comparator driver 
 */
/*
 * Author:          Kc.tseng
 */

#include "mcu.h"
#include "hosal_comparator.h"
#include "hosal_status.h"

uint32_t hosal_comp_register_int_callback(void* callback_function) {
    comp_register_int_callback(callback_function);
    return HOSAL_STATUS_SUCCESS;
}

uint32_t hosal_comparator_interrupt_enable(void) {
    comp_int_enable();
    return HOSAL_STATUS_SUCCESS;
}

uint32_t hosal_comparator_interrupt_disable(void) {
    comp_int_disable();
    return HOSAL_STATUS_SUCCESS;
}

uint32_t hosal_comparator_enable(void) {
    comp_enable();
    return HOSAL_STATUS_SUCCESS;
}

uint32_t hosal_comparator_sleep_enable(void) {
    comp_enable_at_sleep();
        return HOSAL_STATUS_SUCCESS;
}

uint32_t hosal_comparator_deep_sleep_enable(void) {
     comp_enable_at_deepsleep();
     return HOSAL_STATUS_SUCCESS;
}

uint32_t hosal_comparator_open(hosal_comp_config_t* cfg, void* hosal_comparator_callback) {


    if (hosal_comparator_callback == NULL)
    {
        return HOSAL_STATUS_INVALID_PARAM;
    }

    HOSAL_COMP_SELREF_SEL(cfg->comp_selref);       /* Select the Comparator input N source */
    HOSAL_COMP_INT_POL(cfg->comp_int_pol);         /* Set the Comparator interrupt polarity */
    HOSAL_COMP_SELINPUT_SEL(cfg->comp_selinput);   /* Select the Comparator input P source */
    HOSAL_COMP_V_SEL(cfg->comp_v_sel);             /* Select the Comparator internal vdd div voltage */
    HOSAL_COMP_REF_SEL(cfg->comp_ref_sel);         /* Select the Comparator N input for comparision */
    HOSAL_COMP_CH_SEL(cfg->comp_ch_sel);           /* Select the Comparator P input for comparision */
    HOSAL_COMP_PW_SEL(cfg->comp_pw);               /* Select the Comparator current */
    HOSAL_COMP_HYS_SEL(cfg->comp_hys_sel);         /* Select the Comparator hys window */
    HOSAL_COMP_SWDIV_SEL(cfg->comp_swdiv);         /* Switch the Comparator vdd div type */
    HOSAL_COMP_DS_WAKEUP(cfg->comp_ds_wakeup);     /* Select the Comparator wakeup in DeepSleep */
    HOSAL_COMP_DS_INV(cfg->comp_ds_inv);           /* Select the invert of the Comparator output for waking up from DeepSleep */

    if (hosal_comparator_callback != NULL) {

        if (HOSAL_COMP_INT_POL_GET() == HOSAL_COMP_CONFIG_INT_POL_FALLING)
        {
            comp_int_callback_counter(1);
        }
        else if (HOSAL_COMP_INT_POL_GET() == HOSAL_COMP_CONFIG_INT_POL_RISING)
        {
            comp_int_callback_counter(2);
        }
        else
        {
            comp_int_callback_counter(3);
        }
        hosal_comp_register_int_callback(hosal_comparator_callback);
        hosal_comparator_interrupt_enable();
    }
    
    return HOSAL_STATUS_SUCCESS;
}




