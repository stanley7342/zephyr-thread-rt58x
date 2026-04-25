/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_wdt.c
 * \brief           Hosal WDT driver file
 */
/*
 * Author:        
 */
#include <stdio.h>
#include <string.h>
#include "status.h"
#include "trng.h"
#include "hosal_otp.h"
#include "pufs_rt_regs.h"
#include "sysctrl.h"

int hosal_otp_init(void) {

	return STATUS_SUCCESS;
}


int hosal_otp_ioctrl(int ctl,uint32_t address ,void* data, uint32_t len)
{

    uint32_t puf_version, puf_feature;
	uint32_t status= STATUS_SUCCESS;
	uint32_t temp;
    switch(ctl) {

            case HOSAL_PUF_READ_VERSION:

            *(uint32_t*)(data) = rt_otp_version();

            break;

            case HOSAL_PUF_READ_UID:
            
            enable_secure_write_protect();

            status = rt_otp_read_data(PUF_UID, address, (uint32_t *)data, len); 

            disable_secure_write_protect();
            
            break;

            case HOSAL_PUF_GET_RAND:
				
            get_random_number(data,len);
			
            break;   

            case HOSAL_PUF_READ_OTP1:
				
            enable_secure_write_protect();

            status = rt_otp_read_data(PUF_OTP1, address, (uint32_t *)data, len);

            disable_secure_write_protect();

            break;   

            case HOSAL_PUF_WRITE_OTP1:
				
			enable_secure_write_protect();
            temp = *(uint32_t *)data;

            rt_otp_write_data(PUF_OTP1, address, temp);
       
			disable_secure_write_protect();
            break;    
            
			case HOSAL_PUF_GET_LOCK_OTP1:
		
            enable_secure_write_protect();

            status = get_otp_lckwd_state(address); 

            disable_secure_write_protect();
            break;  
			
			case HOSAL_PUF_SET_LOCK_OTP1:
				
            enable_secure_write_protect();

            set_otp_lckwd_readonly(address); 

			disable_secure_write_protect();
            
            break;  	

            case HOSAL_PUF_READ_OTP2:
				
            enable_secure_write_protect();

            status = rt_otp_read_data(PUF_OTP2, address, (uint32_t *)data, len);
            
            disable_secure_write_protect();
            break;    

            case HOSAL_PUF_WRITE_OTP2:
				
			enable_secure_write_protect();

            temp = *(uint32_t *)data;

            printf("write data=%8X\r\n",temp);
            printf("address=%d\r\n",address);
            rt_otp_write_data(PUF_OTP2, address, temp);

			disable_secure_write_protect();			
			
            break; 
		
			case HOSAL_PUF_GET_LOCK_OTP2:
            
			enable_secure_write_protect();	

            status = get_otp2_lckwd_state(address); 

			disable_secure_write_protect();			
			
            break;  
			
			case HOSAL_PUF_SET_LOCK_OTP2:
				
            enable_secure_write_protect();

            set_otp2_lckwd_readonly(address); 

			disable_secure_write_protect();
			
            break;  				

            default:
            
			break;
    }

        return status;
}
















