/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */


/**
 * \file            mp sector.c
 * \brief          mass production driver 
 */
#include <stdio.h>
#include <string.h>
#include "mcu.h"
#include "mp_sector.h"
#include "flashctl.h"
#include "sysfun.h"


#define OTP_MINUS_CHECK(x) (x<0?-1:1)

//static mp_sector_cal_t otp_cal;

volatile uint8_t mp_cal_vbatadc_flag = 0;
volatile uint16_t mp_cal_vbatadc_v1 = 0;
volatile uint16_t mp_cal_vbatadc_adc1 = 0;
volatile uint16_t mp_cal_vbatadc_v2 = 0;
volatile uint16_t mp_cal_vbatadc_adc2 = 0;

volatile uint8_t mp_cal_aioadc_flag = 0;
volatile uint16_t mp_cal_aioadc_v1 = 0;
volatile uint16_t mp_cal_aioadc_adc1 = 0;
volatile uint16_t mp_cal_aioadc_v2 = 0;
volatile uint16_t mp_cal_aioadc_adc2 = 0;

volatile uint8_t mp_cal_vcmadc_flag = 0;
volatile uint8_t mp_cal_vcmadc_enable = 0;
volatile uint16_t mp_cal_vcmadc_adc1 = 0;

volatile uint8_t mp_cal_tempadc_flag = 0;
volatile uint16_t mp_cal_tempadc_adc1 = 0;

volatile uint8_t mp_cal_tempk_flag = 0;
volatile uint16_t mp_cal_tempk_value = 0;

volatile uint8_t mp_cal_ana_flag = 0;
volatile uint16_t mp_cal_ana_v1 = 0;
volatile uint8_t mp_cal_ana_vosel1 = 0;
volatile uint16_t mp_cal_ana_v2 = 0;
volatile uint8_t mp_cal_ana_vosel2 = 0;

volatile uint8_t mp_cal_bod_flag = 0;
volatile uint16_t mp_cal_bod_v1 = 0;
volatile uint8_t mp_cal_bod_vosel1 = 0;
volatile uint16_t mp_cal_bod_v2 = 0;
volatile uint8_t mp_cal_bod_vosel2 = 0;

volatile uint8_t mp_cal_agc_flag = 0;
volatile uint16_t mp_cal_3p3_adc = 0;
volatile uint16_t mp_cal_1p8_adc = 0;
volatile uint16_t mp_cal_temp_adc = 0;

mp_sector_valid_t mp_sector_valid = MP_SECTOR_INVALID;



#define  CHECK_RESULT(X)   (X)<=4?0:1

#if defined(CONFIG_RT584HA4)
static uint8_t MpSectorAry[4096] __attribute__((aligned(4)));
static uint8_t use_ram_buffer_4096k = 0;
#else
uint8_t MpSectorAry[4096];
#endif
/**
* \ingroup mp_sector_group
* \brief  Otp to mp pmu value check
* \return None
*/
uint32_t fttomppmurangecheck(uint32_t id, int32_t targetvosel)
{
    //uint8_t voselresult;

    switch (id)
    {
    case MP_ID_DCDC:
    case MP_ID_LDOMV:

        if ((targetvosel >= MP_ID_VOSEL_MIN) && (targetvosel <= MP_ID_DCDC_LDOMV_VOSEL_MAX))
        {
            return STATUS_SUCCESS;
        }
        break;

    case MP_ID_LDOANA:
    case MP_ID_LDODIG:
    case MP_ID_RETLDO:

        if ((targetvosel >= MP_ID_VOSEL_MIN) && (targetvosel <= MP_ID_LDOANA_DIG_RET_VOSEL_MAX))
        {
            return STATUS_SUCCESS;
        }
        break;

    case MP_ID_ANA_COM:

        if ((targetvosel == MP_ID_BOD_VOSEL_MAX))
        {
            return STATUS_SUCCESS;
        }
        break;

    case MP_ID_BOD:

        if (targetvosel == MP_ID_BOD_VOSEL_MAX)
        {
            return STATUS_SUCCESS;
        }

        break;
    }


    return STATUS_ERROR;
}
/**
* \ingroup mp_sector_group
* \brief
* \return None
*/
/**
* \ingroup mp_sector_group
* \brief
* \return None
*/
uint32_t fttovoselcal(uint32_t mp_id, float target_voltage, ft_cal_regulator_t *ft_pmu_cal)
{
    uint32_t  vosel_result = 0;
    float  voltage_mul_vosel = 0.0;
    float  voltage_sub = 0.0;
    float  voltage_result = 0.0;
    uint32_t    vosel_uncondition = 0;
    uint32_t    result_status = STATUS_SUCCESS;
    int16_t voltage_1, voltage_2;
    int8_t      vosel_1, vosel_2;

    voltage_1 = ft_pmu_cal->voltage_1;
    voltage_2 = ft_pmu_cal->voltage_2;
    vosel_1  = ft_pmu_cal->vosel_1;
    vosel_2  = ft_pmu_cal->vosel_2;

    if ((mp_id >= MP_ID_DCDC) && (mp_id <= MP_ID_LDODIG))
    {

        //printf(" Dec2Hex((vosel_2-vosel_1)*[Target_voltage - voltage_1]/[voltage_2-voltage_1]+vosel_1)\r\n");
        //printf(" vosel_1 =0x%X ,vosel_2 =0x%X ,voltage_1=0x%X, voltage_2=0x%X , target_voltage=%f \r\n",vosel_1,vosel_2,voltage_1,voltage_2,target_voltage);


        voltage_mul_vosel = (vosel_2 - vosel_1) * (target_voltage - voltage_1);
        //printf(" voltage_mul_vosel (((vosel_2-vosel_1)*(target_voltage-voltage_1)))=%.1f \r\n",voltage_mul_vosel);
        voltage_sub = ((voltage_2 - voltage_1));
        //printf(" voltage_sub ((voltage_2-voltage_1))=%.1f \r\n",voltage_sub);
        voltage_result = (voltage_mul_vosel / voltage_sub) * 10;

        vosel_uncondition = (uint32_t)voltage_result;

        voltage_result = (voltage_result / 10);

        //printf(" voltage_result (voltage_mul_vosel/voltage_sub)=%.1f \r\n",voltage_result);
        vosel_result = ((uint32_t)(voltage_result) + vosel_1);

        if (vosel_uncondition != 0)
        {
            vosel_result += 1;
        }

        if (fttomppmurangecheck(mp_id, vosel_result) == STATUS_SUCCESS)
        {
            result_status = fttomppmurangecheck(mp_id, vosel_result) | vosel_result | BIT31;
        }
        else
        {

            if (mp_id == MP_ID_LDODIG || mp_id == MP_ID_LDOANA)
            {
                vosel_result = 0x6; //HW default value;
            }
            else
            {
                vosel_result = 0xE; //HW default value;
            }

            result_status = vosel_result | BIT31;
        }
        //printf(" vosel_result =%d \r\n",vosel_result);

    }
    else if (mp_id == MP_ID_RETLDO)
    {
        //printf(" MP_ID_RETLDO \r\n");
        //printf(" Dec2Hex ((vosel_1-vosel_2)*[Target_voltage-voltage_2]/[voltage_1-voltage_2]+vosel_2)\r\n");
        //printf(" vosel_1 =0x%X ,vosel_2 =0x%X ,voltage_1=0x%X, voltage_2=0x%X , target_voltage=%f \r\n",vosel_1,vosel_2,voltage_1,voltage_2,target_voltage);
        voltage_mul_vosel = (vosel_1 - vosel_2) * (target_voltage - voltage_2);
        //printf(" voltage_mul_vosel (((vosel_1-vosel_2)*(target_voltage-voltage_2)))=%.1f \r\n",voltage_mul_vosel);
        voltage_sub = (voltage_1 - voltage_2) ;
        //printf(" voltage_sub ((voltage_1 - voltage_2))=%f \r\n",voltage_sub);
        voltage_result = (voltage_mul_vosel / voltage_sub) * 10;

        vosel_uncondition = (uint32_t)voltage_result;

        voltage_result = voltage_result / 10;
        //printf(" voltage_result (voltage_mul_vosel/voltage_sub)=%.1f \r\n",voltage_result);

        vosel_result = ((uint32_t)(voltage_result) + vosel_2);

#if 0
        if (vosel_uncondition != 0)
        {
            vosel_result += 1;
        }
#endif
        //printf(" vosel_result=((uint32_t)(voltage_result) + vosel_2)=%d \r\n",vosel_result);

        if (fttomppmurangecheck(mp_id, vosel_result) == STATUS_SUCCESS)
        {
            result_status = fttomppmurangecheck(mp_id, vosel_result) | vosel_result | BIT31;
        }
        else
        {
            vosel_result = 0; //HW default value;
            result_status = vosel_result | BIT31;
        }
    }
    else
    {
        if (mp_id == MP_ID_RETLDO)
        {
            vosel_result = 0; //HW default value;
        }
        else if (mp_id == MP_ID_LDODIG || mp_id == MP_ID_LDOANA)
        {
            vosel_result = 0x6; //HW default value;
        }
        else
        {
            vosel_result = 0xE; //HW default value;
        }

        result_status = vosel_result | BIT31;
    }

    return result_status;
}


uint32_t fttoanabodvoselcal(uint32_t mp_id, float target_voltage, ft_cal_regulator_t *ft_pmu_cal)
{
    uint32_t  vosel_result = 0;
    float  voltage_mul_vosel = 0.0;
    float  voltage_sub = 0.0;
    float  voltage_result = 0.0;
    uint32_t    vosel_uncondition = 0;
    uint32_t    result_status = STATUS_SUCCESS;
    int16_t voltage_1, voltage_2;
    int8_t      vosel_1, vosel_2;


    if (mp_id == MP_ID_ANA_COM || mp_id == MP_ID_BOD)
    {
        voltage_1 = ft_pmu_cal->voltage_1;
        voltage_2 = ft_pmu_cal->voltage_2;
        vosel_1  = ft_pmu_cal->vosel_1;
        vosel_2  = ft_pmu_cal->vosel_2;

        voltage_mul_vosel = (vosel_2 - vosel_1) * (target_voltage - voltage_1);
        voltage_sub = ((voltage_2 - voltage_1));
        voltage_result = (voltage_mul_vosel / voltage_sub) * 10;

        vosel_uncondition = (uint32_t)voltage_result;

        voltage_result = (voltage_result / 10);

        vosel_result = ((uint32_t)(voltage_result) + vosel_1);

        if (vosel_uncondition != 0)
        {
            vosel_result += 1;
        }

        result_status = vosel_result | BIT31;
    }


    return result_status;
}

/**
* \ingroup mp_sector_group
* \brief
* \return None
*/
void ftloadadcvalue(uint32_t mp_id, mp_cal_adc_t *mp_adc_cal, ft_cal_adc_t *ft_adc_cal)
{

    if (mp_id == MP_ID_VBAT_ADC)
    {
        mp_adc_cal->flag = ft_adc_cal->flag;
        mp_adc_cal->voltage_1 = ft_adc_cal->voltage_1;
        mp_adc_cal->voltage_2 = ft_adc_cal->voltage_3;
        mp_adc_cal->adc_1 = ft_adc_cal->adc_1;
        mp_adc_cal->adc_2 = ft_adc_cal->adc_3;
        mp_adc_cal->target_voltage_1 = 2000;
        mp_adc_cal->target_voltage_2 = 2500;
        mp_adc_cal->target_voltage_3 = 3300;
    }
    else if (mp_id == MP_ID_AIO_ADC)
    {
        mp_adc_cal->flag = ft_adc_cal->flag;
        mp_adc_cal->voltage_1 = ft_adc_cal->voltage_1;
        mp_adc_cal->voltage_2 = ft_adc_cal->voltage_3;
        mp_adc_cal->adc_1 = ft_adc_cal->adc_1;
        mp_adc_cal->adc_2 = ft_adc_cal->adc_3;
        mp_adc_cal->target_voltage_1 = 900;
        mp_adc_cal->target_voltage_2 = 1800;
        mp_adc_cal->target_voltage_3 = 3300;
    }
    else if (mp_id == MP_ID_VCM_ADC)
    {
        mp_adc_cal->flag = ft_adc_cal->flag;
        mp_adc_cal->adc_1 = ft_adc_cal->voltage_1;
    }
    else if (mp_id == MP_ID_TEMP_ADC)
    {
        mp_adc_cal->flag = ft_adc_cal->flag;
        mp_adc_cal->adc_1 = ft_adc_cal->adc_1;

    }
    else if (mp_id == MP_ID_TEMPK)
    {
        mp_adc_cal->flag = ft_adc_cal->flag;
        mp_adc_cal->adc_1 = ft_adc_cal->adc_1;

    }
}

void ftload_ana_bod_value(uint32_t mp_id, mp_cal_regulator_t *mp_cal, ft_cal_regulator_t *ft_ana_bod_cal)
{
    if (mp_id == MP_ID_ANA_COM)
    {
        mp_cal->flag = ft_ana_bod_cal->flag;
        mp_cal->voltage_1 = ft_ana_bod_cal->voltage_1;
        mp_cal->voltage_2 = ft_ana_bod_cal->voltage_2;
        mp_cal->vosel_1 = ft_ana_bod_cal->vosel_1;
        mp_cal->vosel_2 = ft_ana_bod_cal->vosel_2;
        mp_cal->target_voltage_1 = 2000;
        mp_cal->target_voltage_2 = 2100;
        mp_cal->target_voltage_3 = 2200;

    }
    else if (mp_id == MP_ID_BOD)
    {
        mp_cal->flag = ft_ana_bod_cal->flag;
        mp_cal->voltage_1 = ft_ana_bod_cal->voltage_1;
        mp_cal->voltage_2 = ft_ana_bod_cal->voltage_2;
        mp_cal->vosel_1 = ft_ana_bod_cal->vosel_1;
        mp_cal->vosel_2 = ft_ana_bod_cal->vosel_2;
        mp_cal->target_voltage_1 = 2000;
        mp_cal->target_voltage_2 = 2100;
        mp_cal->target_voltage_3 = 2200;
    }
}

/**
* \ingroup mp_sector_group
* \brief
* \return None
*/
void ftloadagcvalue( uint32_t mp_id, mp_cal_agc_adc_t *mp_agc_cal, ft_cal_agc_adc_t *ft_agc_cal)
{
    if (mp_id == MP_ID_AGC)
    {
        mp_agc_cal->flag = ft_agc_cal->flag;
        mp_agc_cal->adc_1p8 = ft_agc_cal->adc_1p8;
        mp_agc_cal->adc_3p3 = ft_agc_cal->adc_3p3;
        mp_agc_cal->adc_temp = ft_agc_cal->adc_temp;
    }

}
/**
* \ingroup OtpToMpAdcCal
* \brief
* \return None
*/
void fttompadccal(mp_cal_adc_t *cal_adc)
{

    uint32_t adc_result = 0;
    uint32_t adc_condition = 0;

    //Target_ADC=ADC_1+(target_voltage-voltage_1)*(ADC_3-ADC1)/(voltage_3-voltage_1) ; reference OTP to Mp_sector programe guid ppt

    adc_result = (cal_adc->target_voltage_1 - cal_adc->voltage_1) * (cal_adc->adc_2 - cal_adc->adc_1) * (10) / (cal_adc->voltage_2 - cal_adc->voltage_1);

    adc_condition =  CHECK_RESULT((adc_result % 10));

    cal_adc->target_adc_1 = cal_adc->adc_1 + (adc_result / 10) + adc_condition;

    adc_result = (cal_adc->target_voltage_2 - cal_adc->voltage_1) * (cal_adc->adc_2 - cal_adc->adc_1) * (10) / (cal_adc->voltage_2 - cal_adc->voltage_1);

    adc_condition =  CHECK_RESULT((adc_result % 10));

    cal_adc->target_adc_2 = cal_adc->adc_1 + (adc_result / 10) + adc_condition;

    adc_result = (cal_adc->target_voltage_3 - cal_adc->voltage_1) * (cal_adc->adc_2 - cal_adc->adc_1) * (10) / (cal_adc->voltage_2 - cal_adc->voltage_1);

    adc_condition =  CHECK_RESULT((adc_result % 10));

    cal_adc->target_adc_3 = cal_adc->adc_1 + (adc_result / 10) + adc_condition;

}
/**
* \ingroup adc range chack
* \brief
* \return None
*/
uint32_t ftadcrangecheck( uint32_t mp_id, mp_cal_adc_t *mp_adc_cal_reg)
{




    return STATUS_SUCCESS;
}
/**
* \ingroup vcm temp adc range check
* \brief
* \return None
*/
uint32_t ftvcmtempadcrangecheck(uint32_t mp_id, mp_cal_vcm_adc_t *mp_cal_reg, mp_cal_temp_adc_t *tmp_mp_cal_reg)
{
    uint8_t  Tccompenstion = 1;
    uint16_t adcMax = 4095;

    if (mp_id == MP_ID_VCM_ADC)
    {
        if (mp_cal_reg->enable > Tccompenstion)
        {
            return STATUS_INVALID_PARAM;
        }
        if (mp_cal_reg->adc_1 > adcMax)
        {
            return STATUS_INVALID_PARAM;
        }

        mp_cal_reg->flag = 1;   //update flag to 1

    }
    else
    {
        if (tmp_mp_cal_reg->adc_1 > adcMax)
        {
            return STATUS_INVALID_PARAM;
        }
        tmp_mp_cal_reg->flag = 1;    //update flag to 1
    }


    return STATUS_SUCCESS;
}
/**
* \brief get mp sector information value
* \return None
*/
uint32_t getmpsectorinfo(mp_sector_inf_t *MpSectorInf)
{
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
    uint32_t flash_size_id = (flash_get_deviceinfo() >> FLASH_SIZE_ID_SHIFT);
#else
    //need to use nsc function
#endif

    if (flash_size_id == FLASH_512K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        MpSectorInf->ver = MP_SECTOR_INFO_512K->MP_SECTOR_VERSION;
        MpSectorInf->size = MP_SECTOR_INFO_512K->MP_SECTOR_SIZE;
        MpSectorInf->cal = MP_SECTOR_INFO_512K->MP_SECTOR_CALIBRATION;
        MpSectorInf->cal_data_sector_size = MP_SECTOR_INFO_512K->CAL_DATA_SECTOR_SIZE;
        MpSectorInf->cal_data_sector_addr = MP_SECTOR_INFO_512K->CAL_DATA_SECTOR_ADDR + FLASH_SECURE_MODE_BASE_ADDR;

        if (MpSectorInf->cal_data_sector_addr != MP_SECTOR_CAL_ADDR_512K)
        {
            MpSectorInf->cal_data_sector_addr = (uint32_t)NULL;
            return STATUS_INVALID_PARAM;
        }
#else
        //need to use nsc function
#endif

    }
    else if (flash_size_id == FLASH_1024K)
    {

#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        MpSectorInf->ver = MP_SECTOR_INFO_1024K->MP_SECTOR_VERSION;
        MpSectorInf->size = MP_SECTOR_INFO_1024K->MP_SECTOR_SIZE;
        MpSectorInf->cal = MP_SECTOR_INFO_1024K->MP_SECTOR_CALIBRATION;
        MpSectorInf->cal_data_sector_size = MP_SECTOR_INFO_1024K->CAL_DATA_SECTOR_SIZE;
        MpSectorInf->cal_data_sector_addr = MP_SECTOR_INFO_1024K->CAL_DATA_SECTOR_ADDR + FLASH_SECURE_MODE_BASE_ADDR;

        if (MpSectorInf->cal_data_sector_addr != MP_SECTOR_CAL_ADDR_1024K)
        {
            MpSectorInf->cal_data_sector_addr = (uint32_t)NULL;
            return STATUS_INVALID_PARAM;
        }
#else
        //need to use nsc function
#endif
    }
    else if (flash_size_id == FLASH_2048K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        MpSectorInf->ver = MP_SECTOR_INFO_2048K->MP_SECTOR_VERSION;
        MpSectorInf->size = MP_SECTOR_INFO_2048K->MP_SECTOR_SIZE;
        MpSectorInf->cal = MP_SECTOR_INFO_2048K->MP_SECTOR_CALIBRATION;
        MpSectorInf->cal_data_sector_size = MP_SECTOR_INFO_2048K->CAL_DATA_SECTOR_SIZE;
        MpSectorInf->cal_data_sector_addr = MP_SECTOR_INFO_2048K->CAL_DATA_SECTOR_ADDR + FLASH_SECURE_MODE_BASE_ADDR;

        if (MpSectorInf->cal_data_sector_addr != MP_SECTOR_CAL_ADDR_2048K)
        {
            MpSectorInf->cal_data_sector_addr = (uint32_t)NULL;
            return STATUS_INVALID_PARAM;
        }
#else
        //need to use nsc function
#endif
    } else if (flash_size_id == FLASH_4096K) {

            #if defined(CONFIG_RT584HA4)
				uint32_t i;
				for (i = 0; i < 16; i++) {
					uint32_t page_addr = 0x103FF000u + (i * 256u);
					
					flash_read_page((uint32_t)(MpSectorAry + (i * 256u)),page_addr);
				}
				
				use_ram_buffer_4096k = 1;
        
				if (use_ram_buffer_4096k) {
					uint8_t *base = (uint8_t *)MpSectorAry;
					MpSectorInf->ver  = *(volatile uint32_t *)(base + 0x0FF8u);
					MpSectorInf->size = *(volatile uint32_t *)(base + 0x0FFCu);
					MpSectorInf->cal  = *(volatile uint32_t *)(base + 0x0FF4u);
					MpSectorInf->cal_data_sector_size = *(volatile uint32_t *)(base + 0x0FC4u);
					/* MP sector stores offset -> convert to absolute */
					MpSectorInf->cal_data_sector_addr = *(volatile uint32_t *)(base + 0x0FC0u);
					MpSectorInf->cal_data_sector_addr += FLASH_SECURE_MODE_BASE_ADDR;
					
					//printf("MpSectorInf->ver=0x%.8X\r\n",MpSectorInf->ver);
					//printf("MpSectorInf->size=0x%.8X\r\n",MpSectorInf->size);
					//printf("cal=0x%.8X\r\n\r\n",MpSectorInf->cal);
					//printf("cal_data_sector_size=0x%.8X\r\n",MpSectorInf->cal_data_sector_size);
					//printf("cal_data_sector_addr=0x%.8X\r\n",MpSectorInf->cal_data_sector_addr);
				} else {
					MpSectorInf->ver  = MP_SECTOR_INFO_4096K->MP_SECTOR_VERSION;
					MpSectorInf->size = MP_SECTOR_INFO_4096K->MP_SECTOR_SIZE;
					MpSectorInf->cal  = MP_SECTOR_INFO_4096K->MP_SECTOR_CALIBRATION;
					MpSectorInf->cal_data_sector_size = MP_SECTOR_INFO_4096K->CAL_DATA_SECTOR_SIZE;
				}

				{
					uint32_t expected = MP_SECTOR_CAL_ADDR_4096K;
					if (MpSectorInf->cal_data_sector_addr != expected) {
						MpSectorInf->cal_data_sector_addr = 0;
						return STATUS_INVALID_PARAM;
					}
				}

            #else

                    #if defined(CONFIG_FLASHCTRL_SECURE_EN)
                            MpSectorInf->ver = MP_SECTOR_INFO_4096K->MP_SECTOR_VERSION;
                            MpSectorInf->size = MP_SECTOR_INFO_4096K->MP_SECTOR_SIZE;
                            MpSectorInf->cal = MP_SECTOR_INFO_4096K->MP_SECTOR_CALIBRATION;
                            MpSectorInf->cal_data_sector_size = MP_SECTOR_INFO_4096K->CAL_DATA_SECTOR_SIZE;
                            MpSectorInf->cal_data_sector_addr = MP_SECTOR_INFO_4096K->CAL_DATA_SECTOR_ADDR + FLASH_SECURE_MODE_BASE_ADDR;
                    
                            if (MpSectorInf->cal_data_sector_addr != MP_SECTOR_CAL_ADDR_4096K)
                            {
                                MpSectorInf->cal_data_sector_addr = (uint32_t)NULL;
                                return STATUS_INVALID_PARAM;
                            }
                    #else
                            //need to use nsc function
                    #endif 

            #endif 
    } else {
        return STATUS_INVALID_PARAM;
    }

    return STATUS_SUCCESS;
}
/**
* \brief  Otp to mp pmu value check
*/
void mpcaldcdcinit(mp_cal_regulator_t *mp_cal_reg)
{
    uint8_t flag;
    //uint8_t select;
    uint8_t target_vosel[3];

    flag = mp_cal_reg->flag;
    //select = mp_cal_reg->select;


    target_vosel[0] = mp_cal_reg->target_vosel_1;
    target_vosel[1] = mp_cal_reg->target_vosel_2;
    target_vosel[2] = mp_cal_reg->target_vosel_3;

    if ((flag == 1) || (flag == 2))
    {
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_normal =  target_vosel[0];


#if defined(CONFIG_RF_POWER_14DBM) || defined(CONFIG_RF_POWER_20DBM) || defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_BASIC_EXAMPLE) || defined(CONFIG_HELLOWORLD)

#if defined(CONFIG_RF_POWER_14DBM)
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = target_vosel[2];
#elif defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = target_vosel[0];
#else
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = target_vosel[0];
#endif

#else

        txpower_default_cfg_t       txpwrlevel;

        txpwrlevel = sys_txpower_getdefault();

        if (txpwrlevel == TX_POWER_14DBM_DEF)
        {

            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = target_vosel[2];
        }
        else if (txpwrlevel == TX_POWER_0DBM_DEF || txpwrlevel == TX_POWER_20DBM_DEF)
        {

            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = target_vosel[0];
        }
        else
        {

            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = target_vosel[0];
        }

#endif

        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_light = target_vosel[0];


    }
}
/**
* \ingroup mp_sector_group
* \brief Function according to the mp_cal_reg parameters to initial the LDO registers
* \param[in] mp_cal_regulator_t *mp_cal_reg
* \return None
*/
void mpcalldomvinit(mp_cal_regulator_t *mp_cal_reg)
{
    uint8_t flag;
    //uint8_t select;
    uint8_t target_vosel[3];;

    flag = mp_cal_reg->flag;
    //select = mp_cal_reg->select;


    target_vosel[0] = mp_cal_reg->target_vosel_1;
    target_vosel[1] = mp_cal_reg->target_vosel_2;
    target_vosel[2] = mp_cal_reg->target_vosel_3;

    if ((flag == 1) || (flag == 2))
    {

        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_normal =  target_vosel[0];    //1250mv

#if defined(CONFIG_RF_POWER_14DBM) || defined(CONFIG_RF_POWER_20DBM) || defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_BASIC_EXAMPLE) || defined(CONFIG_HELLOWORLD)

#if defined(CONFIG_RF_POWER_14DBM)
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = target_vosel[2];      //1600mv
#elif defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = target_vosel[0];      //1250mv
#else
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = target_vosel[0];      //1250mv
#endif

#else
        txpower_default_cfg_t       txpwrlevel;

        txpwrlevel = sys_txpower_getdefault();
        if (txpwrlevel == TX_POWER_14DBM_DEF)
        {
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = target_vosel[2];      //1600mv
        }
        else if (txpwrlevel == TX_POWER_0DBM_DEF || txpwrlevel == TX_POWER_20DBM_DEF)
        {
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = target_vosel[0];      //1250mv
        }
        else
        {
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = target_vosel[0];      //1250mv
        }

#endif

        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_light = target_vosel[0];      //1250mv
    }
}
/**
* \ingroup mp_sector_group
* \brief Function according to the mp_cal_reg parameters to initial the IOLDO registers
* \param[in] mp_cal_regulator_t *mp_cal_reg
* \return None
*/
void mpcalldoanainit(mp_cal_regulator_t *mp_cal_reg)
{
    uint8_t flag;
    //uint8_t select;
    uint8_t target_vosel = 0;

    flag = mp_cal_reg->flag;
    //select = mp_cal_reg->select;
    target_vosel = mp_cal_reg->target_vosel_1;


    if ((flag == 1) || (flag == 2))
    {
        PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_normal = target_vosel;
        PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_heavy = (target_vosel + 1);
    }
}
/**
* \ingroup mp_sector_group
* \brief Function according to the mp_cal_reg parameters to initial the SLDO registers
* \param[in] mp_cal_regulator_t *mp_cal_reg
* \return None
*/
void mpcalldodiginit(mp_cal_regulator_t *mp_cal_reg)
{
    uint8_t flag;
    //uint8_t select;
    uint8_t target_vosel = 0;

    flag = mp_cal_reg->flag;
    //select = mp_cal_reg->select;
    target_vosel = mp_cal_reg->target_vosel_1;

    if ((flag == 1) || (flag == 2))
    {
        PMU_CTRL->pmu_core_vosel.bit.ldodig_vosel = target_vosel;
    }

}
/**
* \ingroup mp_sector_group
* \brief Function according to the mp_cal_reg parameters to initial the SIOLDO registers
* \param[in] mp_cal_regulator_t mp_cal_reg
* \return None
*/
void mpcalretldoinit(mp_cal_regulator_t *mp_cal_reg)
{
    uint8_t flag;
    uint8_t select;
    uint8_t target_vosel = 0;

    flag = mp_cal_reg->flag;
    select = mp_cal_reg->select;

    switch (select)
    {
    case 1:
        target_vosel = mp_cal_reg->target_vosel_1;
        break;

    case 2:
        target_vosel = mp_cal_reg->target_vosel_2;
        break;

    case 3:
        target_vosel = mp_cal_reg->target_vosel_3;
        break;

    default:
        flag = 0;
        break;
    }

    if ((flag == 1) || (flag == 2))
    {
        PMU_CTRL->pmu_core_vosel.bit.sldo_vosel_sp = target_vosel;
    }
}

/*
uint32_t MpCalVbatAdc(sadc_value_t adc_val)
{
    int32_t cal_vol = 0;

    if ((mp_cal_vbatadc_flag == 1) || (mp_cal_vbatadc_flag == 2))
    {
        cal_vol = (adc_val - mp_cal_vbatadc_adc1);
        cal_vol *= (mp_cal_vbatadc_v2 - mp_cal_vbatadc_v1);
        cal_vol /= (mp_cal_vbatadc_adc2 - mp_cal_vbatadc_adc1);
        cal_vol += mp_cal_vbatadc_v1;
    }

    return cal_vol;
}
*/
/**
*
* \brief Mp sector read vbat adc value
*/
uint32_t mpcalvbatadcread(mp_cal_adc_t *mp_cal_adc)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_vbatadc_flag == 1) || (mp_cal_vbatadc_flag == 2))
    {
        mp_cal_adc->voltage_1 = mp_cal_vbatadc_v1;
        mp_cal_adc->adc_1 = mp_cal_vbatadc_adc1;
        mp_cal_adc->voltage_2 = mp_cal_vbatadc_v2;
        mp_cal_adc->adc_2 = mp_cal_vbatadc_adc2;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}
/**
*
* \brief Mp sector read aio adc value
*/
uint32_t mpcalaioadcread(mp_cal_adc_t *mp_cal_adc)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_aioadc_flag == 1) || (mp_cal_aioadc_flag == 2))
    {
        mp_cal_adc->voltage_1 = mp_cal_aioadc_v1;
        mp_cal_adc->adc_1 = mp_cal_aioadc_adc1;
        mp_cal_adc->voltage_2 = mp_cal_aioadc_v2;
        mp_cal_adc->adc_2 = mp_cal_aioadc_adc2;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}
/**
*
* \brief Mp sector read vcm adc value
*/
uint32_t mpcalvcmadcread(mp_cal_vcm_adc_t *mp_cal_vcmadc)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_vcmadc_flag == 1) || (mp_cal_vcmadc_flag == 2))
    {
        mp_cal_vcmadc->enable = mp_cal_vcmadc_enable;
        mp_cal_vcmadc->adc_1 = mp_cal_vcmadc_adc1;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}
/**
*
* \brief Mp sector read temp adc value
*/
uint32_t mpcaltempadcread(mp_cal_temp_adc_t *mp_cal_tempadc)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_tempadc_flag == 1) || (mp_cal_tempadc_flag == 2))
    {
        mp_cal_tempadc->adc_1 = mp_cal_tempadc_adc1;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}
/**
*
* \brief Mp sector read kt value
*/
uint32_t mpcalktread(mp_temp_k_t *mp_cal_k)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_tempk_flag == 1) || (mp_cal_tempk_flag == 2))
    {
        mp_cal_k->ktvalue = mp_cal_tempk_value;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}

/**
*
* \brief Mp sector read ana value
*/
uint32_t mpcalanaread(mp_cal_regulator_t *mp_cal_ana)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_ana_flag == 1) || (mp_cal_ana_flag == 2))
    {
        mp_cal_ana->voltage_1 = mp_cal_ana_v1;
        mp_cal_ana->vosel_1 = mp_cal_ana_vosel1;
        mp_cal_ana->voltage_2 = mp_cal_ana_v2;
        mp_cal_ana->vosel_2 = mp_cal_ana_vosel2;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}

/**
*
* \brief Mp sector read BOD value
*/
uint32_t mpcalbodread(mp_cal_regulator_t *mp_cal_bod)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_bod_flag == 1) || (mp_cal_bod_flag == 2))
    {
        mp_cal_bod->voltage_1 = mp_cal_bod_v1;
        mp_cal_bod->vosel_1 = mp_cal_bod_vosel1;
        mp_cal_bod->voltage_2 = mp_cal_bod_v2;
        mp_cal_bod->vosel_2 = mp_cal_bod_vosel2;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}
/**
*
* \brief Mp sector Read AGC Value
*/
uint32_t mpcalagcread(mp_cal_agc_adc_t *mp_cal_agc)
{
    uint32_t read_status = STATUS_INVALID_PARAM;

    if ((mp_cal_agc_flag == 1) || (mp_cal_agc_flag == 2))
    {

        mp_cal_agc->adc_1p8 = mp_cal_1p8_adc;
        mp_cal_agc->adc_3p3 = mp_cal_3p3_adc ;
        mp_cal_agc->adc_temp = mp_cal_temp_adc;

        read_status = STATUS_SUCCESS;
    }

    return read_status;
}

/**
*
* \brief Mp sector initinal vbat adc Value
*/
void mpcalvbatadcinit(mp_cal_adc_t *mp_cal_adc)
{
    mp_cal_vbatadc_flag = mp_cal_adc->flag;
    mp_cal_vbatadc_v1 = mp_cal_adc->voltage_1;
    mp_cal_vbatadc_adc1 = mp_cal_adc->adc_1;
    mp_cal_vbatadc_v2 = mp_cal_adc->voltage_2;
    mp_cal_vbatadc_adc2 = mp_cal_adc->adc_2;
}
/**
*
* \brief Mp sector initinal aio adc Value
*/
void mpcalaioadcinit(mp_cal_adc_t *mp_cal_adc)
{
    mp_cal_aioadc_flag = mp_cal_adc->flag;
    mp_cal_aioadc_v1 = mp_cal_adc->voltage_1;
    mp_cal_aioadc_adc1 = mp_cal_adc->adc_1;
    mp_cal_aioadc_v2 = mp_cal_adc->voltage_2;
    mp_cal_aioadc_adc2 = mp_cal_adc->adc_2;
}
/**
*
* \brief Mp sector initinal vcm adc Value
*/
void mpcalvcmadcinit(mp_cal_vcm_adc_t *mp_cal_vcmadc)
{
    mp_cal_vcmadc_flag = mp_cal_vcmadc->flag;
    mp_cal_vcmadc_enable = mp_cal_vcmadc->enable;
    mp_cal_vcmadc_adc1 = mp_cal_vcmadc->adc_1;
}
/**
*
* \brief Mp sector initinal Temp adc Value
*/
void mpcaltempadcinit(mp_cal_temp_adc_t *mp_cal_tempadc)
{
    mp_cal_tempadc_flag = mp_cal_tempadc->flag;
    mp_cal_tempadc_adc1 = mp_cal_tempadc->adc_1;
}

/**
*
* \brief Mp sector initinal Temp k Value
*/
void mpcalktreadinit(mp_temp_k_t *mp_cal_k)
{
    mp_cal_tempk_flag = mp_cal_k->flag;
    mp_cal_tempk_value = mp_cal_k->ktvalue;
}

/**
*
* \brief Mp sector initinal ANA Value
*/
void mpcalanacomreadinit(mp_cal_regulator_t *mp_cal_anacom)
{
    mp_cal_ana_flag = mp_cal_anacom->flag;
    mp_cal_ana_v1 = mp_cal_anacom->voltage_1;
    mp_cal_ana_vosel1 = mp_cal_anacom->vosel_1;
    mp_cal_ana_v2 =  mp_cal_anacom->voltage_2;
    mp_cal_ana_vosel2 = mp_cal_anacom->vosel_2;
}
/**
*
* \brief Mp sector initinal BOD Value
*/
void mpcalbodreadinit(mp_cal_regulator_t *mp_cal_bod)
{
    mp_cal_bod_flag =  mp_cal_bod->flag;
    mp_cal_bod_v1 =     mp_cal_bod->voltage_1;
    mp_cal_bod_vosel1 = mp_cal_bod->vosel_1;
    mp_cal_bod_v2 =     mp_cal_bod->voltage_2;
    mp_cal_bod_vosel2 = mp_cal_bod->vosel_2;
}
/**
*
* \brief Mp sector initinal AGC Value
*/
void mpcalagcreadinit(mp_cal_agc_adc_t *mp_cal_agc)
{
    mp_cal_agc_flag = mp_cal_agc->flag;
    mp_cal_3p3_adc = mp_cal_agc->adc_3p3;
    mp_cal_1p8_adc = mp_cal_agc->adc_1p8;
    mp_cal_temp_adc = mp_cal_agc->adc_temp;
}

/**
*
* \brief Mp sector initinal AGC Value
*/
uint8_t mpsectorreadtxpwrcfg()
{

    uint32_t read_addr = 0;
    uint8_t txpwrlevel = 0;
    if (flash_size() == FLASH_1024K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        read_addr = 0x100FFFD8;
#else
        read_addr = 0x000FFFD8;
#endif
    }
    else if (flash_size() == FLASH_2048K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        read_addr = 0x101FFFD8;
#else
        read_addr = 0x001FFFD8;
#endif
    }
    else if (flash_size() == FLASH_4096K)
    {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        read_addr = 0x103FFFD8;
#else
        read_addr = 0x003FFFD8;
#endif
    }

    uint32_t start = read_addr;      // FD8
    uint32_t limit = start + 8;      // one-past-end: FD8~FDF 共 8 bytes
    uint32_t addr;
    
    for (addr = start; addr < limit; addr++)
    {
        txpwrlevel = flash_read_byte(addr);
    
        if ((txpwrlevel == TX_POWER_20DBM_DEF) ||
            (txpwrlevel == TX_POWER_14DBM_DEF) ||
            (txpwrlevel == TX_POWER_0DBM_DEF))
        {
            break;  // find data
        }
        else
        {
            txpwrlevel = TX_POWER_14DBM_DEF; // 
        }
    }
    
    if (addr >= limit) {
        txpwrlevel = TX_POWER_14DBM_DEF;
    }

    set_sys_txpower_default(txpwrlevel);

    return txpwrlevel;
}


uint32_t mpsectorwritetxpwrcfg(uint8_t updatetxpwrlevel)
{


    uint32_t read_addr = 0, i = 0;
    uint8_t txpwrlevel = 0;
	uint8_t buf[8];
    if ((updatetxpwrlevel == TX_POWER_20DBM_DEF) || (updatetxpwrlevel == TX_POWER_14DBM_DEF) || (updatetxpwrlevel == TX_POWER_0DBM_DEF))
    {

        if (flash_size() == FLASH_1024K)
        {
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
            read_addr = 0x100FFFD8;
#else
            read_addr = 0x000FFFD8;
#endif
        }
        else if (flash_size() == FLASH_2048K)
        {
#if FLASHCTRL_SECURE_EN==1
            read_addr = 0x101FFFD8;
#else
            read_addr = 0x001FFFD8;
#endif
        }
        else if (flash_size() == FLASH_4096K)
        {
#if FLASHCTRL_SECURE_EN==1
            read_addr = 0x103FFFD8;
#else
            read_addr = 0x003FFFD8;
#endif
        }
        uint32_t start = read_addr;      // start addres FD8
        uint32_t limit = start + 8;      // FD8~FDF 共 8 bytes
        txpwrlevel = 0xFF;
        uint32_t addr;

        /* scan 8 bytes */
        for (addr = start; addr < limit; addr++)
        {
            txpwrlevel = flash_read_byte(addr);
        
            /* find value */
            if (txpwrlevel == updatetxpwrlevel)
            {
                return STATUS_INVALID_PARAM;
            }
        
            /* new address */
            if (txpwrlevel == 0xFF)
            {
                break;
            }
        }

        /* New address */
        if (txpwrlevel == 0xFF)
        {
            if (addr < limit)
            {
         
                flash_write_mpsector_txpwrcfgbyte(addr, updatetxpwrlevel);
        
                if (addr != start)
                {
                    flash_write_mpsector_txpwrcfgbyte(addr - 1, 0x00);
                }
				
                set_sys_txpower_default(updatetxpwrlevel);
        
                return STATUS_SUCCESS;
            }
            else
            {
                return STATUS_INVALID_REQUEST;   // over address
            }
        }
        else
        {
            return STATUS_ERROR;                 // record full
        }
    }
    else   //unknow tx power type;
    {

        return STATUS_INVALID_PARAM;
    }



    return STATUS_SUCCESS;
}

/**
* \ingroup mp_sector_group
* \brief Function according to the mp_cal_reg parameters to initial the PMU->PMU_COMP0.bit.AUX_COMP_VSEL registers
* \param[in] mp_cal_xtaltrim
*            \arg mp_sector_head_t head;
*            \arg flag
*            \arg xo_trim;
* \return None
*/
void mpcalcrystaltriminit(mp_cal_xtal_trim_t *mp_cal_xtaltrim)
{

    uint16_t target_xo_trim;

    target_xo_trim = mp_cal_xtaltrim->xo_trim;

    if ((mp_cal_xtaltrim->flag == 1) || (mp_cal_xtaltrim->flag == 2))
    {
        PMU_CTRL->pmu_soc_pmu_xtal1.bit.xosc_cap_ini = target_xo_trim;
    }
}

mp_sector_head_t *getspecvalidmpid(uint32_t spec_mp_id);
mp_sector_head_t *getnullmpid(void);

uint32_t mpcalrftrimwrite(uint32_t mp_id, MPK_RF_TRIM_T *mp_cal_rf)
{
    uint32_t i;
    uint32_t write_status = STATUS_SUCCESS;
    uint32_t write_addr;
    uint32_t write_cnt;
    uint8_t write_byte;
    uint8_t *pWriteByte;

    mp_sector_head_t *valid_rf_trim;
    valid_rf_trim = getspecvalidmpid(mp_id);
	if (valid_rf_trim == NULL || mp_cal_rf == NULL) {
	    return STATUS_INVALID_REQUEST;
	}
	
    pWriteByte = (uint8_t *)mp_cal_rf;

    //write_cnt = mp_cal_rf->head.mp_cnt;
    write_cnt = sizeof(MPK_RF_TRIM_T);
    write_addr = (uint32_t)valid_rf_trim;
	
	#if defined(CONFIG_RT584HA4)
	    /* RT584HA4: valid_rf_trim is a pointer into MpSectorAry (RAM),
	     * must convert to flash absolute address by offset mapping.
	     */
	    {
	        mp_sector_inf_t MpInf;
	        uint32_t offset;
	
	        memset(&MpInf, 0, sizeof(MpInf));
			if (getmpsectorinfo(&MpInf) != STATUS_SUCCESS || MpInf.cal_data_sector_addr == 0) {
			    return STATUS_ERROR;
			}

	        /* Calculate offset within MpSectorAry */
	        offset = (uint32_t)((uint8_t *)valid_rf_trim - (uint8_t *)MpSectorAry);
	
	        /* Bounds check: must stay within MP sector 4KB */
	        if (offset >= sizeof(MpSectorAry)) {
	            return STATUS_ERROR;
	        }
	        if ((offset + write_cnt) > sizeof(MpSectorAry)) {
	            return STATUS_INVALID_PARAM; /* would overflow MP sector */
	        }
	
	        /* Flash address to write */
	        write_addr = MpInf.cal_data_sector_addr + offset;
	    }
	#else
	    /* Non-RT584HA4: valid_rf_trim is a pointer in flash memory map already */
	    write_addr = (uint32_t)valid_rf_trim;
	#endif
	
    if (write_addr == 0)
    {
        return STATUS_ERROR;
    }

    for (i = 0; i < write_cnt; i++)
    {
        write_byte = *pWriteByte;
        pWriteByte++;

#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        flash_write_mpsector_rftrimbyte((write_addr + i), write_byte);
        while (flash_check_busy());
#else
        //need to use nsc function
#endif

    }

    //     if (valid_rf_trim != NULL)
    //     {
    //         write_addr = (uint32_t)&valid_rf_trim->mp_valid;

    // #if FLASHCTRL_SECURE_EN==1
    //         flash_write_byte(write_addr, MP_INVALID);
    //         while (flash_check_busy());
    // #else
    //         //need to use nsc function
    // #endif
    //     }


    return write_status;
}

uint32_t mpcalrftrimread(uint32_t mp_id, uint32_t byte_cnt, uint8_t *mp_sec_data)
{
	    uint32_t i;
	    uint32_t read_status = STATUS_SUCCESS;
	
	    uint8_t *pValidByte;
	    uint8_t *pReadByte;
	
	    mp_sector_head_t *valid_mp_sec;
	    valid_mp_sec = getspecvalidmpid(mp_id);
	
	    do
	    {
	        if (mp_sec_data == NULL || byte_cnt == 0U)
	        {
	            read_status = STATUS_INVALID_PARAM;
	            break;
	        }
	
	        if (valid_mp_sec == NULL)
	        {
	            read_status = STATUS_INVALID_REQUEST;
	            break;
	        }
	
	#if defined(CONFIG_RT584HA4)
	        {
	            mp_sector_inf_t MpInf;
	            uint32_t offset;
	            uint32_t flash_addr;
	
	            memset(&MpInf, 0, sizeof(MpInf));
	            if (getmpsectorinfo(&MpInf) != STATUS_SUCCESS || MpInf.cal_data_sector_addr == 0U)
	            {
	                read_status = STATUS_ERROR;
	                break;
	            }
	
	            /* valid_mp_sec points into MpSectorAry (RAM) on RT584HA4 */
	            offset = (uint32_t)((uint8_t *)valid_mp_sec - (uint8_t *)MpSectorAry);
	
	            /* Bounds check within 4KB MP sector buffer */
	            if (offset >= sizeof(MpSectorAry))
	            {
	                read_status = STATUS_ERROR;
	                break;
	            }
	            if ((offset + byte_cnt) > sizeof(MpSectorAry))
	            {
	                read_status = STATUS_INVALID_PARAM;
	                break;
	            }
	
	            flash_addr = MpInf.cal_data_sector_addr + offset;
	
	            /* Read from flash to ensure latest data (not stale RAM snapshot) */
	            for (i = 0; i < byte_cnt; i++)
	            {
	#if defined(CONFIG_FLASHCTRL_SECURE_EN)
	                mp_sec_data[i] = flash_read_byte(flash_addr + i);
	#else
	                /* need to use nsc function */
	                mp_sec_data[i] = 0xFF;
	#endif
	            }
	        }
	#else
	        /* Non-RT584HA4: valid_mp_sec is already mapped to flash region */
	        pValidByte = (uint8_t *)valid_mp_sec;
	        pReadByte  = (uint8_t *)mp_sec_data;
	
	        for (i = 0; i < byte_cnt; i++)
	        {
	            *pReadByte++ = *pValidByte++;
	        }
	#endif
	
	    } while (0);
	
	    return read_status;
}
/*
mp_sector_head_t * GetFirstValidMpId(mp_sector_head_t *pMpHead)
{
//    uint32_t get_status = 0;
    uint32_t mp_next = 0;

    mp_sector_head_t *pSearchHead;

    pSearchHead = pMpHead;

    do
    {
        if (pSearchHead->mp_id == MP_ID_NULL)
        {
            pMpHead = NULL;
//            get_status = 0;
            break;
        }

        if (pSearchHead->mp_valid == MP_VALID)
        {
            pMpHead = pSearchHead;
//            get_status = 1;
            break;
        }

        mp_next = (uint32_t)pSearchHead;
        mp_next += pSearchHead->mp_cnt;
        pSearchHead = (mp_sector_head_t *)mp_next;
    } while (1);

//    return get_status;
    return pMpHead;
}

uint32_t GetNextValidMpId(mp_sector_head_t *pMpHead)
{
}
*/
/**
 * \ingroup mp_sector_group
 * \brief Function to get the Mp sector id
 * \param[in] pMpHead
 *            \arg mp_id    id
 *            \arg mp_valid vaild flag
 *            \arg mp_cnt   counter value
 * \return pNextHead next vaild the point
 */
mp_sector_head_t *getnextmpid(mp_sector_head_t *pMpHead)
{
    uint32_t mp_next = 0;
    mp_sector_head_t *pNextHead = NULL;

    if ((pMpHead->mp_id != MP_ID_NULL) && (pMpHead->mp_cnt != 0))
    {
        mp_next = (uint32_t)pMpHead;
        mp_next += pMpHead->mp_cnt;
        pNextHead = (mp_sector_head_t *)mp_next;
    }

    return pNextHead;
}
/**
 * \ingroup mp_sector_group
 * \brief Function to get first vaild the Mp sector id
 * \param[in] pMpHead
 *            \arg mp_id    id
 *            \arg mp_valid vaild flag
 *            \arg mp_cnt   counter value
 * \return pNextHead vaild the point
 */
mp_sector_head_t *getfirstvalidmpid(mp_sector_head_t *pMpHead)
{
    mp_sector_head_t *pValidHead = NULL;

    while ((pMpHead != NULL) && (pMpHead->mp_id != MP_ID_NULL))
    {
        if (pMpHead->mp_valid == MP_VALID)
        {
            pValidHead = pMpHead;
            break;
        }

        pMpHead = getnextmpid(pMpHead);
    }

    return pValidHead;
}
/**
 * \ingroup mp_sector_group
 * \brief Function to get next vaild the Mp sector id
 * \param[in] pMpHead
 *            \arg mp_id    id
 *            \arg mp_valid vaild flag
 *            \arg mp_cnt   counter value
 * \return pValidHead vaild the point
 */
mp_sector_head_t *getnextvalidmpid(mp_sector_head_t *pMpHead)
{
    mp_sector_head_t *pValidHead = NULL;

    if (pMpHead != NULL)
    {
        pValidHead = getfirstvalidmpid(getnextmpid(pMpHead));
    }

    return pValidHead;
}

/**
 * \ingroup mp_sector_group
 * \brief Function to get a vaild point
 * \param[in] spec_mp_id
 *
 * \return pMpHead vaild the point
 */
mp_sector_head_t *getspecvalidmpid(uint32_t spec_mp_id)
{
    mp_sector_head_t *pMpHead;
    mp_sector_inf_t MpInf;
    memset(&MpInf, '\0', sizeof(mp_sector_inf_t));

    getmpsectorinfo(&MpInf);
	
	#if defined(CONFIG_RT584HA4)
	pMpHead = (mp_sector_head_t *)(MpSectorAry);
	#else
	pMpHead = (mp_sector_head_t *)((mp_sector_cal_t *)(MpInf.cal_data_sector_addr));
	#endif
   
    while (pMpHead != NULL)
    {
        if (pMpHead->mp_id == spec_mp_id)
        {
            break;
        }

        pMpHead = getnextvalidmpid(pMpHead);
    }

    return pMpHead;
}
/**
 * \ingroup mp_sector_group
 * \brief Function to get a vaild point
 * \return pNullMpHead vaild the point
 */
mp_sector_head_t *getnullmpid(void)
{
    uint32_t mp_next = 0;
    mp_sector_head_t *pMpHead;
    mp_sector_head_t *pNullMpHead = NULL;
    mp_sector_inf_t MpInf;
    memset(&MpInf, '\0', sizeof(mp_sector_inf_t));

    getmpsectorinfo(&MpInf);
	#if defined(CONFIG_RT584HA4)
	pMpHead = (mp_sector_head_t *)(MpSectorAry);
	#else
	pMpHead = (mp_sector_head_t *)((mp_sector_cal_t *)(MpInf.cal_data_sector_addr));
	#endif

    while (pMpHead != NULL)
    {
        if (pMpHead->mp_id == MP_ID_NULL)
        {
            pNullMpHead = pMpHead;
            break;
        }

        if (pMpHead->mp_cnt == 0)
        {
            break;
        }

        mp_next = (uint32_t)pMpHead;
        mp_next += pMpHead->mp_cnt;
        pMpHead = (mp_sector_head_t *)mp_next;
    }

    return pNullMpHead;
}


/**
 * \ingroup
 * \brief mp calibration data initinal function
 * \param[in] cal_sector
 */
void mpcalibrationinit(mp_sector_cal_t *cal_sector)
{
    mp_sector_head_t *pMpHead;

    pMpHead = getfirstvalidmpid((mp_sector_head_t *)cal_sector);

    while (pMpHead != NULL)
    {
        switch (pMpHead->mp_id)
        {
        case MP_ID_DCDC:
            mpcaldcdcinit((mp_cal_regulator_t *)&cal_sector->DCDC);
            break;

        case MP_ID_LDOMV:
            mpcalldomvinit((mp_cal_regulator_t *)&cal_sector->LDOMV);
            break;

        case MP_ID_LDOANA:
            mpcalldoanainit((mp_cal_regulator_t *)&cal_sector->LDOANA);
            break;

        case MP_ID_LDODIG:
            mpcalldodiginit((mp_cal_regulator_t *)&cal_sector->LDODIG);
            break;

        case MP_ID_RETLDO:
            mpcalretldoinit((mp_cal_regulator_t *)&cal_sector->RETLDO);
            break;

        case MP_ID_POWER_FAIL:
            //MpCalPowerfailInit((mp_cal_regulator_t *)&cal_sector->POWER_FAIL);
            break;

        case MP_ID_CRYSTAL_TRIM:
            mpcalcrystaltriminit((mp_cal_xtal_trim_t *)&cal_sector->CRYSTAL_TRIM);
            break;

        default:
            break;
        }

        pMpHead = getnextvalidmpid(pMpHead);
    }
}


uint32_t mpsectorinit(void)
{
    uint32_t mp_sector_check = 0;
    uint32_t status = STATUS_SUCCESS;
    mp_sector_valid = MP_SECTOR_INVALID;

    mp_sector_inf_t MpInf;
    memset(&MpInf, '\0', sizeof(mp_sector_inf_t));

    getmpsectorinfo(&MpInf);


    if (((MpInf.ver & MP_SECTOR_VERSION_CODE_MASK) == MP_SECTOR_VERSION_CODE) &&
            (MpInf.cal_data_sector_size <= MP_SECTOR_CAL_SIZE))
    {
        mp_sector_check = 1;
    }
    else
    {

        status = fttompcalibration();
        return status;
    }


    do
    {
        if (mp_sector_check == 1)
        {
            if (((MpInf.ver == MP_SECTOR_VERSION_ID_V1) && (MpInf.cal == MP_SECTOR_CALIBRATION_TOOL_V1)) ||
                    (MpInf.cal == MP_SECTOR_CALIBRATION_TOOL_V2))
            {
                mp_sector_valid = MP_SECTOR_VALID_MPTOOL;

                break;
            }


            if (((MpInf.ver == MP_SECTOR_VERSION_ID_V1) && (MpInf.cal == MP_SECTOR_CALIBRATION_SW_V1)) ||
                    (MpInf.cal == MP_SECTOR_CALIBRATION_SW_V2))
            {
                #if 0 //need default layout
                status = mpsectorcalswupdate();
                #endif
                mp_sector_valid = MP_SECTOR_VALID_SWDEFAULT;
                break;
            }

            if (((MpInf.ver == MP_SECTOR_VERSION_ID_V1) && (MpInf.cal == MP_SECTOR_CALIBRATION_SW_V1)) ||
                    (MpInf.cal == MP_SECTOR_CALIBRATION_SW_V2))
            {
                mp_sector_valid = MP_SECTOR_VALID_SWDEFAULT;

                break;
            }

            mp_sector_valid = MP_SECTOR_VALID_SWCAL;
        }

    } while (0);

    switch (mp_sector_valid)
    {
    case MP_SECTOR_VALID_MPTOOL:
    case MP_SECTOR_VALID_SWDEFAULT:

		#if defined(CONFIG_RT584HA4)
        mpcalibrationinit((mp_sector_cal_t *)(MpSectorAry));
		#else
		mpcalibrationinit((mp_sector_cal_t *)(MpInf.cal_data_sector_addr));
		#endif
        
        break;

    case MP_SECTOR_VALID_SWCAL:
        status  = fttompcalibration();
        break;

    default:
        break;
    }

    return status;
}


void mpcalibrationadcinit(mp_sector_cal_t *cal_sector)
{
    mp_sector_head_t *pMpHead;

    pMpHead = getfirstvalidmpid((mp_sector_head_t *)cal_sector);

    while (pMpHead != NULL)
    {
        switch (pMpHead->mp_id)
        {
        case MP_ID_VBAT_ADC:
            mpcalvbatadcinit((mp_cal_adc_t *)&cal_sector->VBAT_ADC);
            break;

        case MP_ID_AIO_ADC:
            mpcalaioadcinit((mp_cal_adc_t *)&cal_sector->AIO_ADC);
            break;

        case MP_ID_VCM_ADC:
            mpcalvcmadcinit((mp_cal_vcm_adc_t *)&cal_sector->VCM_ADC);
            break;

        case MP_ID_TEMP_ADC:
            mpcaltempadcinit((mp_cal_temp_adc_t *)&cal_sector->TEMP_ADC);
            break;

        case MP_ID_TEMPK:
            mpcalktreadinit((mp_temp_k_t *)&cal_sector->TEMP_K);
            break;

        case MP_ID_ANA_COM:
            mpcalanacomreadinit((mp_cal_regulator_t *)&cal_sector->ANA_COM);
            break;

        case MP_ID_BOD:
            mpcalbodreadinit((mp_cal_regulator_t *)&cal_sector->BOD_ADC);
            break;

        case MP_ID_AGC:
            mpcalagcreadinit((mp_cal_agc_adc_t *)&cal_sector->AGC_ADC);
            break;

        default:
            break;
        }


        pMpHead = getnextvalidmpid(pMpHead);
    }
}


void mpcaladcinit(void)
{
    uint32_t  mp_sector_check = 0;

    mp_sector_inf_t MpInf;
    memset(&MpInf, '\0', sizeof(mp_sector_inf_t));

    getmpsectorinfo(&MpInf);

    do
    {
        if (mp_sector_valid != MP_SECTOR_INVALID)
        {
            break;
        }


        if (((MpInf.ver & MP_SECTOR_VERSION_CODE_MASK) == MP_SECTOR_VERSION_CODE) &&
                (MpInf.cal_data_sector_size <= MP_SECTOR_CAL_SIZE))
        {
            mp_sector_check = 1;
        }

        if (mp_sector_check == 1)
        {
            if (((MpInf.ver == MP_SECTOR_VERSION_ID_V1) && (MpInf.cal == MP_SECTOR_CALIBRATION_TOOL_V1)) ||
                    (MpInf.cal == MP_SECTOR_CALIBRATION_TOOL_V2))
            {
                mp_sector_valid = MP_SECTOR_VALID_MPTOOL;

                break;
            }
        }

        if (fttompcalibration() == STATUS_SUCCESS)
        {
            mp_sector_valid = MP_SECTOR_VALID_SWCAL;

            break;
        }

        if (mp_sector_check == 1)
        {
            if (((MpInf.ver == MP_SECTOR_VERSION_ID_V1) && (MpInf.cal == MP_SECTOR_CALIBRATION_SW_V1)) ||
                    (MpInf.cal == MP_SECTOR_CALIBRATION_SW_V2))
            {
                mp_sector_valid = MP_SECTOR_VALID_SWDEFAULT;

                break;
            }
        }

    } while (0);

    switch (mp_sector_valid)
    {
    case MP_SECTOR_VALID_MPTOOL:
    case MP_SECTOR_VALID_SWDEFAULT:
        
		#if defined(CONFIG_RT584HA4)
        mpcalibrationadcinit((mp_sector_cal_t *)(MpSectorAry));
		#else
		mpcalibrationadcinit((mp_sector_cal_t *)(MpInf.cal_data_sector_addr));
		#endif
        
        break;

    case MP_SECTOR_VALID_SWCAL:
        //MpCalibrationAdcInit((mp_sector_cal_t *)&otp_cal);
        break;

    default:
        break;
    }
}

uint32_t mpsectorcheckft(uint8_t ft_offset)
{

    //uint32_t     i = 0, offset = 0;
    uint8_t      ft_rd_buf_addr[256], ft_filed_flag;


    if (flash_read_otp_sec_page((uint32_t)ft_rd_buf_addr) != STATUS_SUCCESS)
    {
        return  STATUS_INVALID_REQUEST;
    }

    ft_filed_flag = ft_rd_buf_addr[ft_offset];

    return ft_filed_flag;
}


uint32_t mpsectorcalswupdate()
{

    uint32_t                    i = 0, offset = 0;
    uint8_t                     ft_rd_buf_addr[256];
    uint32_t                    result, status;
    ft_cal_regulator_t          ft_pmu_temp;
    ft_cal_adc_t                ft_adc;
    ft_cal_agc_adc_t            ft_agc;
    //ft_cal_vcm_temp_adc_t       ft_vcm;
    ft_cal_img_k_t              ft_img_k;
    mp_cal_regulator_t          mp_cal_adc;
    mp_cal_adc_t                mp_adc;
    //mp_cal_vcm_adc_t            mp_vcm;
    //mp_cal_temp_adc_t           mp_temp_adc;
    //mp_temp_k_t                 mp_temp_k;
    mp_cal_agc_adc_t            mp_agc;
    mp_sector_inf_t  MpInf;
    mp_sector_info_t *MpInfp;
    mp_sector_cal_t *MpCal;

    status = STATUS_SUCCESS;

    memset(&MpInf, '\0', sizeof(mp_sector_inf_t));
    getmpsectorinfo(&MpInf);

	#if !defined(CONFIG_RT584HA4)
	memcpy(MpSectorAry, (uint32_t *)(MpInf.cal_data_sector_addr), sizeof(MpSectorAry));
	#endif
	
    MpCal = (mp_sector_cal_t *)MpSectorAry;
    MpInfp = (mp_sector_info_t *)(MpSectorAry + 0xFC0);

    //read data form FT(security Page)
    if (flash_read_otp_sec_page((uint32_t)ft_rd_buf_addr) != STATUS_SUCCESS)
    {
        status |= STATUS_INVALID_PARAM;
    }
    else
    {
        //MP_ID_DCDC
        ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_DCDC_OFFSET);

        if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
        {
            MpCal->DCDC.flag = 2;
            MpCal->DCDC.voltage_1 = ft_pmu_temp.voltage_1;
            MpCal->DCDC.voltage_2 = ft_pmu_temp.voltage_2;
            MpCal->DCDC.vosel_1 = ft_pmu_temp.vosel_1;
            MpCal->DCDC.vosel_2 = ft_pmu_temp.vosel_2;
            MpCal->DCDC.select = 1;
            MpCal->DCDC.target_voltage_1 = 1250;
            result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1.25v;
            MpCal->DCDC.target_vosel_1 = GET_BYTE0(result);
            MpCal->DCDC.target_voltage_2 = 1350;
            result = fttovoselcal(MP_ID_DCDC, 1350, &ft_pmu_temp); //Target volatge 1.35v;
            MpCal->DCDC.target_vosel_2 = GET_BYTE0(result);
            MpCal->DCDC.target_voltage_3 = 1600;
            result = fttovoselcal(MP_ID_DCDC, 1600, &ft_pmu_temp); //Target volatge 1.60v;
            MpCal->DCDC.target_vosel_3 = GET_BYTE0(result);

        }
        else if ((ft_pmu_temp.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_LDOMV
        ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_LDOMV_OFFSET);

        if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
        {
            MpCal->LDOMV.flag = 2;
            MpCal->LDOMV.voltage_1 = ft_pmu_temp.voltage_1;
            MpCal->LDOMV.voltage_2 = ft_pmu_temp.voltage_2;
            MpCal->LDOMV.vosel_1 = ft_pmu_temp.vosel_1;
            MpCal->LDOMV.vosel_2 = ft_pmu_temp.vosel_2;
            MpCal->LDOMV.select = 1;
            MpCal->LDOMV.target_voltage_1 = 1250;
            result = fttovoselcal(MP_ID_LDOMV, 1250, &ft_pmu_temp); //Target volatge 1.25v;
            MpCal->LDOMV.target_vosel_1 = GET_BYTE0(result);
            MpCal->LDOMV.target_voltage_2 = 1350;
            result = fttovoselcal(MP_ID_LDOMV, 1350, &ft_pmu_temp); //Target volatge 1.35v;
            MpCal->LDOMV.target_vosel_2 = GET_BYTE0(result);
            MpCal->LDOMV.target_voltage_3 = 1600;
            result = fttovoselcal(MP_ID_LDOMV, 1600, &ft_pmu_temp); //Target volatge 1.60v;
            MpCal->LDOMV.target_vosel_3 = GET_BYTE0(result);
        }
        else if ((ft_pmu_temp.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }
        //MP_ID_LDOANA
        ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_LDOANA_OFFSET);

        if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
        {
            MpCal->LDOANA.flag = 2;
            MpCal->LDOANA.voltage_1 = ft_pmu_temp.voltage_1;
            MpCal->LDOANA.voltage_2 = ft_pmu_temp.voltage_2;
            MpCal->LDOANA.vosel_1 = ft_pmu_temp.vosel_1;
            MpCal->LDOANA.vosel_2 = ft_pmu_temp.vosel_2;
            MpCal->LDOANA.select = 1;
            MpCal->LDOANA.target_voltage_1 = 1100;
            result = fttovoselcal(MP_ID_LDOANA, 1100, &ft_pmu_temp); //Target volatge 1.25v;
            MpCal->LDOANA.target_vosel_1 = GET_BYTE0(result);

        }
        else if ((ft_pmu_temp.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_LDODIG
        ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_LDODIG_OFFSET);

        if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
        {
            MpCal->LDODIG.flag = 2;
            MpCal->LDODIG.voltage_1 = ft_pmu_temp.voltage_1;
            MpCal->LDODIG.voltage_2 = ft_pmu_temp.voltage_2;
            MpCal->LDODIG.vosel_1 = ft_pmu_temp.vosel_1;
            MpCal->LDODIG.vosel_2 = ft_pmu_temp.vosel_2;
            MpCal->LDODIG.select = 1;

            MpCal->LDODIG.target_voltage_1 = 1100;
            result = fttovoselcal(MP_ID_LDODIG, 1100, &ft_pmu_temp); //Target volatge 1.25v;
            MpCal->LDODIG.target_vosel_1 = GET_BYTE0(result);
        }
        else if ((ft_pmu_temp.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_RETLDO
        ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_REDLDO_OFFSET);

        if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
        {

            MpCal->RETLDO.flag = 2;
            MpCal->RETLDO.voltage_1 = ft_pmu_temp.voltage_1;
            MpCal->RETLDO.voltage_2 = ft_pmu_temp.voltage_2;
            MpCal->RETLDO.vosel_1 = ft_pmu_temp.vosel_1;
            MpCal->RETLDO.vosel_2 = ft_pmu_temp.vosel_2;
            MpCal->RETLDO.select = 1;
            MpCal->RETLDO.target_voltage_1 = 700;
            MpCal->RETLDO.target_voltage_2 = 800;
            MpCal->RETLDO.target_voltage_3 = 680;

            result = fttovoselcal(MP_ID_RETLDO, 700, &ft_pmu_temp);
            MpCal->RETLDO.target_vosel_1 = GET_BYTE0(result);
            result = fttovoselcal(MP_ID_RETLDO, 800, &ft_pmu_temp);
            MpCal->RETLDO.target_vosel_2 = GET_BYTE0(result);
            result = fttovoselcal(MP_ID_RETLDO, 680, &ft_pmu_temp);
            MpCal->RETLDO.target_vosel_3 = GET_BYTE0(result);
        }
        else if ((ft_pmu_temp.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_VBAT_ADC
        ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_VBAT_OFFSET);

        if (ft_adc.flag == 1) // 1:update 0:don't update
        {
            ftloadadcvalue(MP_ID_VBAT_ADC, &mp_adc, &ft_adc);
            fttompadccal(&mp_adc);

            MpCal->VBAT_ADC.flag = 2;
            MpCal->VBAT_ADC.adc_1 = mp_adc.adc_1;
            MpCal->VBAT_ADC.adc_2 = mp_adc.adc_2;
            MpCal->VBAT_ADC.voltage_1 =  mp_adc.voltage_1;
            MpCal->VBAT_ADC.voltage_2 =  mp_adc.voltage_2;
            MpCal->VBAT_ADC.target_adc_1 = mp_adc.target_adc_1;
            MpCal->VBAT_ADC.target_adc_2 = mp_adc.target_adc_2;
            MpCal->VBAT_ADC.target_adc_3 = mp_adc.target_adc_3;
            MpCal->VBAT_ADC.target_voltage_1 = mp_adc.target_voltage_1;
            MpCal->VBAT_ADC.target_voltage_2 = mp_adc.target_voltage_2;
            MpCal->VBAT_ADC.target_voltage_3 = mp_adc.target_voltage_3;
        }
        else if ((ft_adc.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }
        //MP_ID_AIO_ADC
        ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_AIO_OFFSET);

        if (ft_adc.flag == 1) // 1:update 0:don't update
        {
            ftloadadcvalue(MP_ID_AIO_ADC, &mp_adc, &ft_adc);
            fttompadccal(&mp_adc);

            MpCal->AIO_ADC.flag = 2;
            MpCal->AIO_ADC.adc_1 = mp_adc.adc_1;
            MpCal->AIO_ADC.adc_2 = mp_adc.adc_2;
            MpCal->AIO_ADC.voltage_1 =  mp_adc.voltage_1;
            MpCal->AIO_ADC.voltage_2 =  mp_adc.voltage_2;
            MpCal->AIO_ADC.target_adc_1 = mp_adc.target_adc_1;
            MpCal->AIO_ADC.target_adc_2 = mp_adc.target_adc_2;
            MpCal->AIO_ADC.target_adc_3 = mp_adc.target_adc_3;
            MpCal->AIO_ADC.target_voltage_1 = mp_adc.target_voltage_1;
            MpCal->AIO_ADC.target_voltage_2 = mp_adc.target_voltage_2;
            MpCal->AIO_ADC.target_voltage_3 = mp_adc.target_voltage_3;
        }
        else if ((ft_adc.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }
        //MP_ID_VCM_ADC
        ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_VCM_OFFSET);

        if (ft_adc.flag == 1) // 1:update 0:don't update
        {
            ftloadadcvalue(MP_ID_VCM_ADC, &mp_adc, &ft_adc);

            MpCal->VCM_ADC.flag = mp_adc.flag;
            MpCal->VCM_ADC.adc_1 = mp_adc.adc_1;
            MpCal->VCM_ADC.enable = 0;

        }
        else if ((ft_adc.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_TEMP_ADC
        ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_TEMP_ADC_OFFSET);
        if (ft_adc.flag == 1) // 1:update 0:don't update
        {
            MpCal->TEMP_ADC.flag = ft_adc.flag;
            MpCal->TEMP_ADC.adc_1 = ft_adc.voltage_1;
        }
        else if ((ft_adc.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_0V_ADC
        ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_0V_ADC_OFFSET);
        if (ft_adc.flag == 1) // 1:update 0:don't update
        {
            MpCal->TEMP_K.flag = ft_adc.flag;
            MpCal->TEMP_K.ktvalue = ft_adc.voltage_1;
        }
        else if ((ft_adc.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_ANA_COM
        ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_ANA_OFFSET);
        if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
        {
            ftload_ana_bod_value(MP_ID_ANA_COM, &mp_cal_adc, &ft_pmu_temp);

            MpCal->ANA_COM.flag = 2;
            MpCal->ANA_COM.voltage_1 = mp_cal_adc.voltage_1;
            MpCal->ANA_COM.voltage_2 = mp_cal_adc.voltage_2;
            MpCal->ANA_COM.vosel_1 =  mp_cal_adc.vosel_1;
            MpCal->ANA_COM.vosel_2 =  mp_cal_adc.vosel_2;
            MpCal->ANA_COM.target_voltage_1 =  mp_cal_adc.target_voltage_1;
            MpCal->ANA_COM.target_voltage_2 =  mp_cal_adc.target_voltage_2;
            MpCal->ANA_COM.target_voltage_3 =  mp_cal_adc.target_voltage_3;

            result = fttoanabodvoselcal(MP_ID_ANA_COM, 2000, &ft_pmu_temp);
            MpCal->ANA_COM.target_vosel_1 =  GET_BYTE0(result);
            result = fttoanabodvoselcal(MP_ID_ANA_COM, 2100, &ft_pmu_temp);
            MpCal->ANA_COM.target_vosel_2 =  GET_BYTE0(result);
            result = fttoanabodvoselcal(MP_ID_ANA_COM, 2200, &ft_pmu_temp);
            MpCal->ANA_COM.target_vosel_3 =  GET_BYTE0(result);

        }
        else if ((ft_pmu_temp.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }
        //MP_ID_BOD
        ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_BOD_OFFSET);
        if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
        {
            ftload_ana_bod_value(MP_ID_BOD, &mp_cal_adc, &ft_pmu_temp);

            MpCal->BOD_ADC.flag = 2;
            MpCal->BOD_ADC.voltage_1 = mp_cal_adc.voltage_1;
            MpCal->BOD_ADC.voltage_2 = mp_cal_adc.voltage_2;
            MpCal->BOD_ADC.vosel_1 =  mp_cal_adc.vosel_1;
            MpCal->BOD_ADC.vosel_2 =  mp_cal_adc.vosel_2;
            MpCal->BOD_ADC.target_voltage_1 =  mp_cal_adc.target_voltage_1;
            MpCal->BOD_ADC.target_voltage_2 =  mp_cal_adc.target_voltage_2;
            MpCal->BOD_ADC.target_voltage_3 =  mp_cal_adc.target_voltage_3;

            result = fttoanabodvoselcal(MP_ID_BOD, 2000, &ft_pmu_temp);
            MpCal->BOD_ADC.target_vosel_1 =  GET_BYTE0(result);
            result = fttoanabodvoselcal(MP_ID_BOD, 2100, &ft_pmu_temp);
            MpCal->BOD_ADC.target_vosel_2 =  GET_BYTE0(result);
            result = fttoanabodvoselcal(MP_ID_BOD, 2200, &ft_pmu_temp);
            MpCal->BOD_ADC.target_vosel_3 =  GET_BYTE0(result);
        }
        else if ((ft_pmu_temp.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        //MP_ID_AGC
        ft_agc = *(ft_cal_agc_adc_t *)(ft_rd_buf_addr + FT_AGC_OFFSET);
        if (ft_agc.flag == 1) // 1:update 0:don't update
        {
            ftloadagcvalue(MP_ID_AGC, &mp_agc, &ft_agc);

            MpCal->AGC_ADC.flag = 2;
            MpCal->AGC_ADC.adc_1p8 = ft_agc.adc_1p8;
            MpCal->AGC_ADC.adc_3p3 = ft_agc.adc_3p3;
            MpCal->AGC_ADC.adc_temp =  ft_agc.adc_temp;
        }
        //else if ((ft_agc.flag > 1))
        //{
        //    status |= STATUS_NO_INIT;
        //}
        //MP_ID_IMAGE
        ft_img_k = *(ft_cal_img_k_t *)(ft_rd_buf_addr + FT_RX_IMAGE_K);


        /* For RF Power on Calibration */
        {
            memset((void *) & (MpCal->RF_TRIM_584_SUBG0.flag), 0xff, sizeof(mp_cal_rf_trim_1_t) - sizeof(mp_sector_head_t));

            MpCal->RF_TRIM_584_SUBG0.flag = 2;
            MpCal->RF_TRIM_584_SUBG0.mode = 2;
        }

        if (ft_img_k.flag == 1) // 1:update 0:don't update
        {
            //need to load img k data
            MpCal->RF_TRIM_584_SUBG0.rx_iq_gain = ft_img_k.rx_iq_again;
            MpCal->RF_TRIM_584_SUBG0.rx_iq_gain_sel = ft_img_k.rx_iq_again_select;
            MpCal->RF_TRIM_584_SUBG0.rx_iq_phase = ft_img_k.rx_iq_phase;
            MpCal->RF_TRIM_584_SUBG0.rx_iq_phase_sel = ft_img_k.rx_iq_phase_select;
        }
        else if ((ft_img_k.flag > 1))
        {
            status |= STATUS_NO_INIT;
        }

        /*
        //MpCal->CRYSTAL_TRIM.flag = 1;
        //MpCal->CRYSTAL_TRIM.xo_trim = 29;
        */
        /*change sw calibration to tool calibration*/
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
        MpInfp->CAL_DATA_SECTOR_ADDR = MpInf.cal_data_sector_addr - FLASH_SECURE_MODE_BASE_ADDR;
#else
        MpInfp->CAL_DATA_SECTOR_ADDR = MpInf.cal_data_sector_addr;
#endif
        MpInfp->CAL_DATA_SECTOR_SIZE = MpInf.cal_data_sector_size;
        MpInfp->MP_SECTOR_CALIBRATION = MP_SECTOR_CALIBRATION_TOOL;
        MpInfp->MP_SECTOR_VERSION = MpInf.ver;//
        MpInfp->MP_SECTOR_SIZE = MpInf.size;

        /*Erase Mp sector Table*/
        flash_erase_mpsector();

        /*Upadte to Mp sector Table*/
        for (i = 0; i < 16; i++)
        {
            flash_write_mp_sector((uint32_t)(MpSectorAry + offset), (MpInf.cal_data_sector_addr + offset));
            offset += 0x100;
        }

        mpsectorwritetxpwrcfg(sys_txpower_getdefault());
    }
    return status;
}


/**
* @ingroup mp_sector_group
* @brief  read otp value and calibration to mp_sector
* @return None
*/

uint32_t fttompcalibration()
{

    static uint8_t ft_rd_buf_addr[256];
    uint32_t result;
    ft_cal_regulator_t         ft_pmu_temp;
    ft_cal_adc_t               ft_adc;
    ft_cal_agc_adc_t           ft_agc;
    //ft_cal_vcm_temp_adc_t      ft_vcm;
    ft_cal_img_k_t              ft_img_k;
    mp_cal_regulator_t          mp_cal_adc;
    mp_cal_adc_t                mp_adc;
    mp_cal_vcm_adc_t            mp_vcm;
    mp_cal_temp_adc_t           mp_temp_adc;
    mp_temp_k_t                 mp_temp_k;
    mp_cal_agc_adc_t            mp_agc;

#if defined(CONFIG_RF_POWER_14DBM) || defined(CONFIG_RF_POWER_20DBM) || defined(CONFIG_RF_POWER_0DBM)
#elif defined(CONFIG_BASIC_EXAMPLE) || defined(CONFIG_HELLOWORLD)
#else
    txpower_default_cfg_t       txpwrlevel;
    txpwrlevel = sys_txpower_getdefault();
#endif
#if defined(CONFIG_FLASHCTRL_SECURE_EN)
    if (flash_read_otp_sec_page((uint32_t)ft_rd_buf_addr) != STATUS_SUCCESS)
    {
        return  STATUS_INVALID_PARAM;
    }
#else
    //need to use nsc function
#endif


    //MP_ID_DCDC
    ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_DCDC_OFFSET);

    if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
    {

        result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1.25v;
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_normal =  GET_BYTE0(result);

#if defined(CONFIG_RF_POWER_14DBM) || defined(CONFIG_RF_POWER_20DBM) || defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_BASIC_EXAMPLE) || defined(CONFIG_HELLOWORLD)

#if defined(CONFIG_RF_POWER_14DBM)
        result = fttovoselcal(MP_ID_DCDC, 1600, &ft_pmu_temp); //Target volatge 1250;
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = GET_BYTE0(result);

#elif defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
        result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1250;
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = GET_BYTE0(result);
#else
        result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1250;
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = GET_BYTE0(result);
#endif

#else



        if (txpwrlevel == TX_POWER_14DBM_DEF)
        {
            result = fttovoselcal(MP_ID_DCDC, 1600, &ft_pmu_temp); //Target volatge 1250;
            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = GET_BYTE0(result);
        }
        else if (txpwrlevel == TX_POWER_0DBM_DEF || txpwrlevel == TX_POWER_20DBM_DEF)
        {
            result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1250;
            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = GET_BYTE0(result);
        }
        else
        {
            result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1250;
            PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_heavy = GET_BYTE0(result);
        }
#endif
        result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1.25v;
        PMU_CTRL->pmu_dcdc_vosel.bit.dcdc_vosel_light = GET_BYTE0(result);

    }

    //MP_ID_LDOMV
    ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_LDOMV_OFFSET);

    if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
    {

        result = fttovoselcal(MP_ID_LDOMV, 1250, &ft_pmu_temp); //Target volatge 1.25v;
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_normal =  GET_BYTE0(result);

#if defined(CONFIG_RF_POWER_14DBM) || defined(CONFIG_RF_POWER_20DBM) || defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_BASIC_EXAMPLE) || defined(CONFIG_HELLOWORLD)

#if defined(CONFIG_RF_POWER_14DBM)
        result = fttovoselcal(MP_ID_LDOMV, 1600, &ft_pmu_temp); //Target volatge 1250;
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = GET_BYTE0(result);
#elif defined(CONFIG_RF_POWER_0DBM) || defined(CONFIG_RF_POWER_20DBM)
        result = fttovoselcal(MP_ID_LDOMV, 1250, &ft_pmu_temp); //Target volatge 1250;
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = GET_BYTE0(result);
#else
        result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1250;
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = GET_BYTE0(result);
#endif

#else
        if (txpwrlevel == TX_POWER_14DBM_DEF)
        {
            result = fttovoselcal(MP_ID_LDOMV, 1600, &ft_pmu_temp); //Target volatge 1250;
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = GET_BYTE0(result);
        }
        else if (txpwrlevel == TX_POWER_0DBM_DEF || txpwrlevel == TX_POWER_20DBM_DEF)
        {
            result = fttovoselcal(MP_ID_LDOMV, 1250, &ft_pmu_temp); //Target volatge 1250;
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = GET_BYTE0(result);
        }
        else
        {
            result = fttovoselcal(MP_ID_DCDC, 1250, &ft_pmu_temp); //Target volatge 1250;
            PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_heavy = GET_BYTE0(result);
        }
#endif
        result = fttovoselcal(MP_ID_LDOMV, 1250, &ft_pmu_temp); //Target volatge 1.25v;
        PMU_CTRL->pmu_ldomv_vosel.bit.ldomv_vosel_light = GET_BYTE0(result);

    }



    //MP_ID_LDOANA
    ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_LDOANA_OFFSET);

    if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
    {
        result = fttovoselcal(MP_ID_LDOANA, 1100, &ft_pmu_temp); //Target volatge 1.25v;

        PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_normal = GET_BYTE0(result);
        PMU_CTRL->pmu_rfldo.bit.ldoana_vtune_heavy = GET_BYTE0(result) + 1;
    }



    //MP_ID_LDODIG
    ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_LDODIG_OFFSET);

    if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
    {
        result = fttovoselcal(MP_ID_LDODIG, 1100, &ft_pmu_temp); //Target volatge 1.25v;
        PMU_CTRL->pmu_core_vosel.bit.ldodig_vosel = GET_BYTE0(result);
    }


    //MP_ID_RETLDO
    ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_REDLDO_OFFSET);

    if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
    {
        result = 0;
        result = fttovoselcal(MP_ID_RETLDO, 700, &ft_pmu_temp);
        PMU_CTRL->pmu_core_vosel.bit.sldo_vosel_sp = GET_BYTE0(result);
    }


    //MP_ID_VBAT_ADC
    ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_VBAT_OFFSET);

    if (ft_adc.flag == 1) // 1:update 0:don't update
    {
        ftloadadcvalue(MP_ID_VBAT_ADC, &mp_adc, &ft_adc);
        fttompadccal(&mp_adc);
        mpcalvbatadcinit(&mp_adc);
    }

    //MP_ID_AIO_ADC
    ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_AIO_OFFSET);

    if (ft_adc.flag == 1) // 1:update 0:don't update
    {
        ftloadadcvalue(MP_ID_AIO_ADC, &mp_adc, &ft_adc);
        fttompadccal(&mp_adc);
        mpcalaioadcinit(&mp_adc);
    }

    //MP_ID_VCM_ADC
    ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_VCM_OFFSET);

    if (ft_adc.flag == 1) // 1:update 0:don't update
    {
        ftloadadcvalue(MP_ID_VCM_ADC, &mp_adc, &ft_adc);
        mp_vcm.enable = 0;
        mp_vcm.flag = mp_adc.flag;
        mp_vcm.adc_1 = mp_adc.adc_1;
        mpcalvcmadcinit(&mp_vcm);
    }

    //MP_ID_TEMP_ADC
    ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_TEMP_ADC_OFFSET);
    if (ft_adc.flag == 1) // 1:update 0:don't update
    {
        mp_temp_adc.flag = ft_adc.flag;
        mp_temp_adc.adc_1 = ft_adc.voltage_1;
        mpcaltempadcinit(&mp_temp_adc);
    }

    //MP_ID_TEMP_ADC
    ft_adc = *(ft_cal_adc_t *)(ft_rd_buf_addr + FT_0V_ADC_OFFSET);
    if (ft_adc.flag == 1) // 1:update 0:don't update
    {
        mp_temp_k.flag = ft_adc.flag;
        mp_temp_k.ktvalue = ft_adc.voltage_1;
        //need add 0v adc calibration
        mpcalktreadinit(&mp_temp_k);
    }


    //MP_ID_ANA_COM
    ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_ANA_OFFSET);
    if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
    {
        ftload_ana_bod_value(MP_ID_ANA_COM, &mp_cal_adc, &ft_pmu_temp);

        result = fttoanabodvoselcal(MP_ID_ANA_COM, 1200, &ft_pmu_temp);
        mp_cal_adc.target_voltage_1 = 1200;
        mp_cal_adc.target_vosel_1 =  GET_BYTE0(result);
        result = fttoanabodvoselcal(MP_ID_ANA_COM, 1100, &ft_pmu_temp);
        mp_cal_adc.target_voltage_2 = 1100;
        mp_cal_adc.target_vosel_2 =  GET_BYTE0(result);
        result = fttoanabodvoselcal(MP_ID_ANA_COM, 1300, &ft_pmu_temp);
        mp_cal_adc.target_voltage_3 = 1300;
        mp_cal_adc.target_vosel_3 =  GET_BYTE0(result);
        mpcalanacomreadinit(&mp_cal_adc);
    }

    //MP_ID_BOD
    ft_pmu_temp = *(ft_cal_regulator_t *)(ft_rd_buf_addr + FT_BOD_OFFSET);
    if (ft_pmu_temp.flag == 1) // 1:update 0:don't update
    {

        ftload_ana_bod_value(MP_ID_BOD, &mp_cal_adc, &ft_pmu_temp);

        result = fttoanabodvoselcal(MP_ID_BOD, 1200, &ft_pmu_temp);
        mp_cal_adc.target_voltage_1 = 1200;
        mp_cal_adc.target_vosel_1 =  GET_BYTE0(result);
        result = fttoanabodvoselcal(MP_ID_BOD, 1100, &ft_pmu_temp);
        mp_cal_adc.target_voltage_2 = 1100;
        mp_cal_adc.target_vosel_2 =  GET_BYTE0(result);
        result = fttoanabodvoselcal(MP_ID_BOD, 1300, &ft_pmu_temp);
        mp_cal_adc.target_voltage_3 = 1300;
        mp_cal_adc.target_vosel_3 =  GET_BYTE0(result);

        mpcalbodreadinit(&mp_cal_adc);
    }


    //MP_ID_AGC
    ft_agc = *(ft_cal_agc_adc_t *)(ft_rd_buf_addr + FT_AGC_OFFSET);
    if (ft_agc.flag == 1) // 1:update 0:don't update
    {
        ftloadagcvalue(MP_ID_AGC, &mp_agc, &ft_agc);
        mpcalagcreadinit(&mp_agc);
    }


    ft_img_k = *(ft_cal_img_k_t *)(ft_rd_buf_addr + FT_RX_IMAGE_K);
    if (ft_img_k.flag == 1) // 1:update 0:don't update
    {
        //need to load img k data
    }

    return STATUS_SUCCESS;

}








