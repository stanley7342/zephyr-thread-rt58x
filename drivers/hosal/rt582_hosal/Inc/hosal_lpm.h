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
#ifndef HOSAL_LPM_H
#define HOSAL_LPM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lpm.h"


/**
 * \defgroup HOSAL_LPM Hosal lpm
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal lpm definitions, structures, and functions
 * @{
 */

/**
 * \brief          hosal low power mode const defineds
 */
#define HOSAL_LPM_PARAM_NONE             0
#define HOSAL_LPM_MASK                   1
#define HOSAL_LPM_GET_MASK               2
#define HOSAL_LPM_SUBSYSTEM_MASK         3
#define HOSAL_LPM_UNMASK                 4
#define HOSAL_LPM_SUBSYSTEM_UNMASK       5
#define HOSAL_LPM_SRAM0_RETAIN           6
#define HOSAL_LPM_SET_POWER_LEVEL        7
#define HOSAL_LPM_GET_POWER_LEVEL        8
#define HOSAL_LPM_ENABLE_WAKE_UP_SOURCE  9
#define HOSAL_LPM_DISABLE_WAKE_UP_SOURCE 10
#define HOSAL_LPM_PLATFORM_WAKE_UP       11
#define HOSAL_LPM_ENTER_LOW_POWER        12
#define HOSAL_LPM_SUBSYSTEM_SRAM_DEEP_SLEEP_INIT 13
#define HOSAL_LPM_SUBSYSTEM_ENTER_LOW_POWER        14
#define HOSAL_LPM_SUBSYSTEM_DISABLE_LDO_MODE       15

/**
 * \brief          hosal low power mask defined 
 */
#define HOSAL_LOW_POWER_NO_MASK                     LOW_POWER_NO_MASK              
#define HOSAL_LOW_POWER_MASK_BIT_TASK_HCI           LOW_POWER_MASK_BIT_TASK_HCI     /**< bit0 */
#define HOSAL_LOW_POWER_MASK_BIT_TASK_HOST    		  LOW_POWER_MASK_BIT_TASK_HOST    /**< bit1 */
#define HOSAL_LOW_POWER_MASK_BIT_TASK_BLE_APP 		  LOW_POWER_MASK_BIT_TASK_BLE_APP /**< bit2 */
#define HOSAL_LOW_POWER_MASK_BIT_TASK_ADC     		  LOW_POWER_MASK_BIT_TASK_ADC     /**< bit3 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD4               LOW_POWER_MASK_BIT_RESERVED4    /**< bit4 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD5               LOW_POWER_MASK_BIT_RESERVED5    /**< bit5 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD6               LOW_POWER_MASK_BIT_RESERVED6    /**< bit6 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD7               LOW_POWER_MASK_BIT_RESERVED7    /**< bit7 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD8               LOW_POWER_MASK_BIT_RESERVED8    /**< bit8 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD9               LOW_POWER_MASK_BIT_RESERVED9    /**< bit9 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD10              LOW_POWER_MASK_BIT_RESERVED10   /**< bit10 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD11              LOW_POWER_MASK_BIT_RESERVED11   /**< bit11 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD12              LOW_POWER_MASK_BIT_RESERVED12   /**< bit12 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD13              LOW_POWER_MASK_BIT_RESERVED13   /**< bit13 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD14              LOW_POWER_MASK_BIT_RESERVED14   /**< bit14 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD15              LOW_POWER_MASK_BIT_RESERVED15   /**< bit15 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD16              LOW_POWER_MASK_BIT_RESERVED16   /**< bit16 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD17              LOW_POWER_MASK_BIT_RESERVED17   /**< bit17 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD18              LOW_POWER_MASK_BIT_RESERVED18   /**< bit18 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD19              LOW_POWER_MASK_BIT_RESERVED19   /**< bit19 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD20              LOW_POWER_MASK_BIT_RESERVED20   /**< bit20 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD21              LOW_POWER_MASK_BIT_RESERVED21   /**< bit21 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD22              LOW_POWER_MASK_BIT_RESERVED22   /**< bit22 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD23              LOW_POWER_MASK_BIT_RESERVED23   /**< bit23 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD24              LOW_POWER_MASK_BIT_RESERVED24   /**< bit24 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD25              LOW_POWER_MASK_BIT_RESERVED25   /**< bit25 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD26              LOW_POWER_MASK_BIT_RESERVED26   /**< bit26 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD27              LOW_POWER_MASK_BIT_RESERVED27   /**< bit27 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD28              LOW_POWER_MASK_BIT_RESERVED28   /**< bit28 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD29              LOW_POWER_MASK_BIT_RESERVED29   /**< bit29 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD30              LOW_POWER_MASK_BIT_RESERVED30   /**< bit30 */
#define HOSAL_LOW_POWER_MASK_BIT_RVD31              LOW_POWER_MASK_BIT_RESERVED31   /**< bit31 */

/**
 * \brief          hosal low power mask defined 
 */
#define HOSAL_COMM_SUBSYS_WAKEUP_NO_MASK            COMM_SUBSYS_WAKEUP_NO_MASK            
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD0      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED0  /**< bit0 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD1      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED1  /**< bit1 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD2      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED2  /**< bit2 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD3      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED3  /**< bit3 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD4      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED4  /**< bit4 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD5      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED5  /**< bit5 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD6      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED6  /**< bit6 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD7      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED7  /**< bit7 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD8      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED8  /**< bit8 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD9      COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED9  /**< bit9 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD10     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED10 /**< bit10 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD11     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED11 /**< bit11 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD12     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED12 /**< bit12 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD13     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED13 /**< bit13 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD14     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED14 /**< bit14 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD15     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED15 /**< bit15 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD16     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED16 /**< bit16 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD17     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED17 /**< bit17 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD18     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED18 /**< bit18 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD19     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED19 /**< bit19 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD20     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED20 /**< bit20 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD21     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED21 /**< bit21 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD22     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED22 /**< bit22 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD23     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED23 /**< bit23 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD24     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED24 /**< bit24 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD25     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED25 /**< bit25 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD26     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED26 /**< bit26 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD27     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED27 /**< bit27 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD28     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED28 /**< bit28 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD29     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED29 /**< bit29 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD30     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED30 /**< bit30 */
#define HOSAL_COMM_SUBSYS_WAKEUP_MASK_BIT_RVD31     COMM_SUBSYS_WAKEUP_MASK_BIT_RESERVED31 /**< bit31 */

 
#define   HOSAL_COMMUMICATION_SUBSYSTEM_PWR_STATE_TRANSITION  COMMUMICATION_SUBSYSTEM_PWR_STATE_TRANSITION  /*!< sub system transition mode */
#define   HOSAL_COMMUMICATION_SUBSYSTEM_PWR_STATE_DEEP_SLEEP COMMUMICATION_SUBSYSTEM_PWR_STATE_DEEP_SLEEP   /*!< sub system enter sleep mode */
#define   HOSAL_COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP COMMUMICATION_SUBSYSTEM_PWR_STATE_SLEEP             /*!< sub system enter deep sleep mode */
#define   HOSAL_COMMUMICATION_SUBSYSTEM_PWR_STATE_NORMAL  COMMUMICATION_SUBSYSTEM_PWR_STATE_NORMAL         /*!< sub system normal mode */ 

 /**
 * \brief            hosal_low_power_platform_enter_mode struct defined 
 */
#define   HOSAL_LOW_POWER_PLATFORM_ENTER_SLEEP  LOW_POWER_PLATFORM_ENTER_SLEEP        /*!< platform system enter sleep mode */
#define   HOSAL_LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP  LOW_POWER_PLATFORM_ENTER_DEEP_SLEEP    /*!< platform system enter deep sleep mode */


/**
 * \brief            hosal low power level enum defined 
 */
#define   HOSAL_LOW_POWER_LEVEL_NORMAL  LOW_POWER_LEVEL_NORMAL  /*!< platform system: run, communication system: run */
#define   HOSAL_LOW_POWER_LEVEL_SLEEP0  LOW_POWER_LEVEL_SLEEP0  /*!< platform system: sleep, communication system: run */
#define   HOSAL_LOW_POWER_LEVEL_SLEEP1  LOW_POWER_LEVEL_SLEEP1  /*!< platform system: sleep, communication system: sleep */
#define   HOSAL_LOW_POWER_LEVEL_SLEEP2  LOW_POWER_LEVEL_SLEEP2  /*!< platform system: sleep, communication system: deep sleep */
#define   HOSAL_LOW_POWER_LEVEL_SLEEP3  LOW_POWER_LEVEL_SLEEP3  /*!< platform system: deep sleep, communication system: deep sleep */
#define   HOSAL_LPM_SLEEP  LOW_POWER_LEVEL_SLEEP0
#define   HOSAL_LPM_DEEP_SLEEP  LOW_POWER_LEVEL_SLEEP3


/**
 * \brief            hosal low power wakeup defined 
 */
#define   HOSAL_LOW_POWER_WAKEUP_NULL          LOW_POWER_WAKEUP_NULL        	/*!< Low power mode wake-up source: Null */
#define   HOSAL_LOW_POWER_WAKEUP_RTC_TIMER     LOW_POWER_WAKEUP_RTC_TIMER       /*!< Low power mode wake-up source: RTC Timer */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO          LOW_POWER_WAKEUP_GPIO            /*!< Low power mode wake-up source: GPIO */
#define   HOSAL_LOW_POWER_WAKEUP_COMPARATOR    LOW_POWER_WAKEUP_COMPARATOR      /*!< Low power mode wake-up source: Analog Comparator */
#define   HOSAL_LOW_POWER_WAKEUP_SLOW_TIMER    LOW_POWER_WAKEUP_SLOW_TIMER       /*!< Low power mode wake-up source: 32KHz Timers */
#define   HOSAL_LOW_POWER_WAKEUP_UART_RX       LOW_POWER_WAKEUP_UART_RX         /*!< Low power mode wake-up source: UART Rx Break Signal */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO0         LOW_POWER_WAKEUP_GPIO0           /*!< Low power mode wake-up source: GPIO0 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO1         LOW_POWER_WAKEUP_GPIO1           /*!< Low power mode wake-up source: GPIO1 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO2         LOW_POWER_WAKEUP_GPIO2           /*!< Low power mode wake-up source: GPIO2 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO3         LOW_POWER_WAKEUP_GPIO3           /*!< Low power mode wake-up source: GPIO3 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO4         LOW_POWER_WAKEUP_GPIO4           /*!< Low power mode wake-up source: GPIO4 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO5         LOW_POWER_WAKEUP_GPIO5           /*!< Low power mode wake-up source: GPIO5 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO6         LOW_POWER_WAKEUP_GPIO6           /*!< Low power mode wake-up source: GPIO6 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO7         LOW_POWER_WAKEUP_GPIO7           /*!< Low power mode wake-up source: GPIO7 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO8         LOW_POWER_WAKEUP_GPIO8           /*!< Low power mode wake-up source: GPIO8 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO9         LOW_POWER_WAKEUP_GPIO9           /*!< Low power mode wake-up source: GPIO9 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO10        LOW_POWER_WAKEUP_GPIO10          /*!< Low power mode wake-up source: GPIO10 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO11        LOW_POWER_WAKEUP_GPIO11          /*!< Low power mode wake-up source: GPIO11 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO12        LOW_POWER_WAKEUP_GPIO12          /*!< Low power mode wake-up source: GPIO12 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO13        LOW_POWER_WAKEUP_GPIO13          /*!< Low power mode wake-up source: GPIO13 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO14        LOW_POWER_WAKEUP_GPIO14          /*!< Low power mode wake-up source: GPIO14 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO15        LOW_POWER_WAKEUP_GPIO15          /*!< Low power mode wake-up source: GPIO15 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO16        LOW_POWER_WAKEUP_GPIO16          /*!< Low power mode wake-up source: GPIO16 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO17        LOW_POWER_WAKEUP_GPIO17          /*!< Low power mode wake-up source: GPIO17 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO18        LOW_POWER_WAKEUP_GPIO18          /*!< Low power mode wake-up source: GPIO18 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO19        LOW_POWER_WAKEUP_GPIO19          /*!< Low power mode wake-up source: GPIO19 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO20        LOW_POWER_WAKEUP_GPIO20          /*!< Low power mode wake-up source: GPIO20 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO21        LOW_POWER_WAKEUP_GPIO21          /*!< Low power mode wake-up source: GPIO21 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO22        LOW_POWER_WAKEUP_GPIO22          /*!< Low power mode wake-up source: GPIO22 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO23        LOW_POWER_WAKEUP_GPIO23          /*!< Low power mode wake-up source: GPIO23 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO24        LOW_POWER_WAKEUP_GPIO24          /*!< Low power mode wake-up source: GPIO24 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO25        LOW_POWER_WAKEUP_GPIO25          /*!< Low power mode wake-up source: GPIO25 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO26        LOW_POWER_WAKEUP_GPIO26          /*!< Low power mode wake-up source: GPIO26 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO27        LOW_POWER_WAKEUP_GPIO27          /*!< Low power mode wake-up source: GPIO27 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO28        LOW_POWER_WAKEUP_GPIO28          /*!< Low power mode wake-up source: GPIO28 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO29        LOW_POWER_WAKEUP_GPIO29          /*!< Low power mode wake-up source: GPIO29 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO30        LOW_POWER_WAKEUP_GPIO30          /*!< Low power mode wake-up source: GPIO30 */
#define   HOSAL_LOW_POWER_WAKEUP_GPIO31        LOW_POWER_WAKEUP_GPIO31          /*!< Low power mode wake-up source: GPIO31 */
#define   HOSAL_LOW_POWER_WAKEUP_UART0_RX      LOW_POWER_WAKEUP_UART0_RX        /*!< Low power mode wake-up source: UART0 Rx Break Signal */
#define   HOSAL_LOW_POWER_WAKEUP_UART1_RX      LOW_POWER_WAKEUP_UART1_RX        /*!< Low power mode wake-up source: UART1 Rx Break Signal */
#define   HOSAL_LOW_POWER_WAKEUP_UART2_RX      LOW_POWER_WAKEUP_UART2_RX        /*!< Low power mode wake-up source: UART2 Rx Break Signal */

/**
 * \brief           low power mode initinal value;
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_lpm_init(void);

/**
 * \brief           low power mode io control
 * \param[in]       ctl: control command
 * \param[in]       para: set low power mode io control paramater value
 * \return          function status
 */
int hosal_lpm_ioctrl(int ctl, uint32_t para);

/**
 * \brief           get low power mode io control paramater value
 * \param[in]       ctl: control command
 * \param[in]       para: get low power mode io control paramater value
 * \return          function status
 */
int hosal_get_lpm_ioctrl(int ctl, uint32_t* para);

/*@}*/ /* end of RT58X_HOSAL HOSAL_LPM */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_LPM_H */
