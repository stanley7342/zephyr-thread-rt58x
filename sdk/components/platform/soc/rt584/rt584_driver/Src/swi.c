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

#include <stdio.h>
#include "mcu.h"
#include "swi.h"


static swi_isr_handler_t swi0_reg_handler[MAX_NUMBER_OF_SWI];
static swi_isr_handler_t swi1_reg_handler[MAX_NUMBER_OF_SWI];
/**
* \brief            Get software interrupt data
*/
uint32_t swi_get_int_data(uint32_t id, uint32_t *data) {

    swi_t* swi_base[2]={SWI0,SWI1};
	swi_t* swi=swi_base[id];
		 
		 
    if (id >= SWI_MAX)
    {
        return STATUS_INVALID_PARAM;
    }
		
    *data = swi->data;


    return STATUS_SUCCESS;
}

/**
* \brief            Set software interrupt data
*/
uint32_t swi_set_int_data(uint32_t id, uint32_t data) {

    swi_t* swi_base[2]={SWI0,SWI1};
	swi_t* swi=swi_base[id];
		 
    if (id >= SWI_MAX)
    {
        return STATUS_INVALID_PARAM;
    }
		
	swi->data = data; 

    return STATUS_SUCCESS;
}

uint32_t swi_int_trigger(uint32_t id, uint32_t trig_bit) {

    swi_t* swi_base[2]={SWI0,SWI1};
	swi_t* swi=swi_base[id];

    if (id >= SWI_MAX)
    {
        return STATUS_INVALID_PARAM;
    }

	NVIC_EnableIRQ((IRQn_Type)(SWI0_IRQn + id));

	swi->enable_irq |= (1 << trig_bit);

     return STATUS_SUCCESS;

}

uint32_t swi_int_callback_register(uint32_t id, uint32_t trig_bit, swi_isr_handler_t swi_cb_fun) {
	
    if (id >= SWI_MAX)
    {
        return STATUS_INVALID_PARAM;
    }

    if(id==0)
    {
        swi0_reg_handler[trig_bit] = swi_cb_fun;
    }
    else if(id==1)
    {
        swi1_reg_handler[trig_bit] = swi_cb_fun;

    }
    else
    {
       return STATUS_INVALID_PARAM;

    }

      return STATUS_SUCCESS;
}

uint32_t swi_int_enable(uint32_t id)
{

	
    if (id >= SWI_MAX)
    {
        return STATUS_INVALID_PARAM;
    }

	NVIC_EnableIRQ((IRQn_Type)(SWI0_IRQn + id));

		 
    return STATUS_SUCCESS;
}

uint32_t swi_int_clear(uint32_t id, uint32_t swi_id) {

    swi_t* swi_base[2]={SWI0,SWI1};
	swi_t* swi=swi_base[id];
		
    if (swi_id >= MAX_NUMBER_OF_SWI)
    {
        return STATUS_INVALID_PARAM;
    }

	swi->clear_irq |= (1 << swi_id);

     return STATUS_SUCCESS;
}


static uint32_t swi_int_callback_unregister(uint32_t id, uint32_t trig_bit) {
	
    swi_t* swi_base[2]={SWI0,SWI1};
	swi_t* swi=swi_base[id];

	
	if(id==0)
    {
        swi0_reg_handler[trig_bit] = NULL;
    }
    else if(id==1)
    {
        swi1_reg_handler[trig_bit] = NULL;
    }
    else
    {
        return STATUS_INVALID_PARAM;

    }	

      return STATUS_SUCCESS;
}

uint32_t swi_int_callback_clear(uint32_t swi_id) {
    
    uint32_t i;

    for (i = 0; i < MAX_NUMBER_OF_SWI; i++)
    {
        swi_int_callback_unregister(swi_id, i);
    }

  return STATUS_SUCCESS;
}

uint32_t swi_int_disable(uint32_t id, uint32_t trig_bit) {

    swi_t* swi_base[2]={SWI0,SWI1};
	swi_t* swi=swi_base[id];
		
    if (trig_bit >= MAX_NUMBER_OF_SWI) {
        
        return STATUS_INVALID_PARAM;
    }

    swi_int_callback_unregister(id, trig_bit);

	swi->clear_irq |= (1 << trig_bit);
		
     return STATUS_SUCCESS;
}

void __swi_generic_notify_handler(uint32_t id) {

    swi_isr_handler_t  cb_isr_fun = NULL;
    swi_t* swi_base[2]={SWI0,SWI1};
	swi_t* swi=swi_base[id];
    uint32_t i = 0, get_status = 0;

    for (i = 0; i < MAX_NUMBER_OF_SWI; i++) {

        get_status = (swi->irq_state) & (1 << i);

        if (get_status != 0) {

            swi->clear_irq |= (1 << i);

            if(id==0)
            {
                cb_isr_fun = swi0_reg_handler[i];
            }
            else if(id==1)
            {
                cb_isr_fun =  swi1_reg_handler[i];
            }

            if (cb_isr_fun != NULL) {
                /*
                * call register Software Interrupt callback
                * Please Notice: the CB is ISR context switch too!
                */
                cb_isr_fun(i);
            }
        }
    }
}

/**
* \brief            SWI 0 interrupt hander
*/
void swi0_handler(void) {

    __swi_generic_notify_handler(0);
}

/**
* \brief            SWI 1 interrupt hander 
*/
void swi1_handler(void) {
	
    __swi_generic_notify_handler(1);
}


