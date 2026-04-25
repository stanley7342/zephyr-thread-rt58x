/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file           sec_ctrl_reg.h
 * \brief          Secure Control register definition header file
 */
/*
 * This file is part of library_name.
 * Author:
 */
#ifndef SEC_CONTROL_REG_H
#define SEC_CONTROL_REG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum SEC_IADU_Type {
    /******  CM33 Specific Sec attribuite Numbers *************************************************/
    //attribuite 0
    SYS_CTRL_IADU_Type          = 0,
    GPIO_IADU_Type              = 1,
    RTC_IADU_Type               = 4,
    DPD_CTRL_IADU_Type          = 5,
    SOC_PMU_IADU_Type           = 6,
    FLASH_CONTROL_IADU_Type     = 9,
    TIMER0_IADU_Type            = 10,
    TIMER1_IADU_Type            = 11,
    TIMER2_IADU_Type            = 12,
    SLOWETIMER0_IADU_Type       = 13,
    SLOWETIMER1_IADU_Type       = 14,
    WDT_IADU_Type               = 16,
    UART0_IADU_Type             = 18,
    UART1_IADU_Type             = 19,
    I2C_S_IADU_Type             = 24,
    RT569_AHB_IADU_Type         = 26,
    RCO32K_CAL_IADU_Type        = 28,
    BOD_COMP_IADU_Type          = 29,
    AUX_COMP_IADU_Type          = 30,
    RCO1M_CAL_IADU_Type         = 31,
    //attribuite 1
    QSPI0_IADU_Type             = 32,
    QSPI1_IADU_Type             = 33,
    IRM_IADU_Type               = 36,
    UART2_IADU_Type             = 37,
    PWM_IADU_Type               = 38,
    XDMA_IADU_Type              = 40,
    DMA0_IADU_Type              = 41,
    DMA1_IADU_Type              = 42,
    I2C_M0_IADU_Type            = 43,
    I2C_M1_IADU_Type            = 44,
    I2S0_M_IADU_Type            = 45,
    SADC_IADU_Type              = 47,
    SWI0_IADU_Type              = 48,
    SWI1_IADU_Type              = 49,
    //attribuite 2
    CRYPTO_IADU_Type            = 64,
    PUF_OTP_IADU_Type           = 65,
} SEC_IADU_Type;


/**
 * \brief           Secure interrupt enable register at offet 0x30
 */
typedef union sec_int_en_ctrl_s {
    struct sec_int_en_ctrl_b {
        uint32_t sec_en_rom_err_int    : 1;     /*!< */
        uint32_t sec_en_flash_err_int  : 1;     /*!< */
        uint32_t sec_en_ram_err_int    : 1;     /*!< */
        uint32_t sec_en_peri_err_int   : 1;     /*!< */
        uint32_t sec_en_crypto_err_int : 1;     /*!< */
        uint32_t sec_en_puf_err_int    : 1;     /*!< */
        uint32_t reserved1             : 26;    /*!< */
    } bit;
    uint32_t reg;
} sec_int_en_ctrl_t;

/**
 * \brief           Secure interrupt clear register at offet 0x34
 */
typedef union sec_int_clr_ctrl_s {
    struct sec_int_clr_ctrl_b {
        uint32_t sec_clr_rom_err_int    : 1;    /*!< */
        uint32_t sec_clr_flash_err_int  : 1;    /*!< */
        uint32_t sec_clr_ram_err_int    : 1;    /*!< */
        uint32_t sec_clr_peri_err_int   : 1;    /*!< */
        uint32_t sec_clr_crypto_err_int : 1;    /*!< */
        uint32_t sec_clr_puf_err_int    : 1;    /*!< */
        uint32_t reserved1              : 26;   /*!< */
    } bit;
    uint32_t reg;
} sec_int_clr_ctrl_t;

/**
 * \brief           Secure interrupt status register at offet 0x38
 */
typedef union sec_int_status_ctrl_s {
    struct sec_int_status_ctrl_b {
        uint32_t sec_status_rom_err_int    : 1;     /*!< */
        uint32_t sec_status_flash_err_int  : 1;     /*!< */
        uint32_t sec_status_ram_err_int    : 1;     /*!< */
        uint32_t sec_status_peri_err_int   : 1;     /*!< */
        uint32_t sec_status_crypto_err_int : 1;     /*!< */
        uint32_t sec_status_puf_err_int    : 1;     /*!< */
        uint32_t reserved1                 : 26;    /*!< */
    } bit;
    uint32_t reg;
} sec_int_status_ctrl_t;

/**
 * \brief          Secure control total register 
 */
typedef struct {
    __IO uint32_t              sec_flash_sec_size;      /*!< 0x00 */
    __IO uint32_t              sec_flash_nsc_start;     /*!< 0x04 */
    __IO uint32_t              sec_flash_nsc_stop;      /*!< 0x08 */
    __IO uint32_t              sec_flash_ns_stop;       /*!< 0x0C */
    __IO uint32_t              sec_ram_sec_size;        /*!< 0x10 */
    __IO uint32_t              sec_ram_nsc_start;       /*!< 0x14 */
    __IO uint32_t              sec_ram_nsc_stop;        /*!< 0x18 */
    __IO uint32_t              sec_ram_ns_stop;         /*!< 0x1C */
    __IO uint32_t              sec_peri_attr[3];        /*!< 0x20~28 */
    __IO uint32_t              sec_idau_ctrl;           /*!< 0x2C */
    __IO sec_int_en_ctrl_t     sec_int_en;              /*!< 0x30 */
    __IO sec_int_clr_ctrl_t    sec_int_clr;             /*!< 0x34 */
    __IO sec_int_status_ctrl_t sec_int_status;          /*!< 0x38 */
    __IO uint32_t              resv1;                   /*!< 0x3C */
    __IO uint32_t              sec_mcu_debug;           /*!< 0x40 */
    __IO uint32_t              sec_lock_mcu_ctrl;       /*!< 0x44 */
    __IO uint32_t              sec_otp_write_key;       /*!< 0x48 */
} sec_ctrl_t;


#define ENABLE_IDAU_SEC_CTRL               1
#define DISABLE_IDAU_SEC_CTRL              0


#define SEC_ROM_ERR_INT                 (1<<0)
#define SEC_FLASH_ERR_INT               (1<<1)
#define SEC_RAM_ERR_INT                 (1<<2)
#define SEC_PERI_ERR_INT                (1<<3)
#define SEC_CRYPTO_ERR_INT              (1<<4)


#define SEC_DBGEN                       (1)
#define SEC_SPIDEN                      (2)
#define SEC_NIDEN                       (4)
#define SEC_SPNIDEN                     (8)


#define SEC_LOCK_SVTAIRCR               (1<<0)
#define SEC_LOCK_NSVTOR                 (1<<1)
#define SEC_LOCK_SMPU                   (1<<1)
#define SEC_LOCK_NSMPU                  (1<<3)
#define SEC_LOCK_SAU                    (1<<4)


/*Attribute setting*/

/*For REG SEC_PERI_ATTR0*/
#define SEC_IDAU_SYS_CTRL                   BIT0            /*  (1UL << 0)   */
#define SEC_IDAU_GPIO                       BIT1            /*  (1UL << 1)   */
#define SEC_IDAU_SEC_CTRL                   BIT3            /*  (1UL << 3)    */
#define SEC_IDAU_RTC                        BIT4            /*  (1UL << 4)    */
#define SEC_IDAU_DPD_CTRL                   BIT5            /*  (1UL << 5)    */
#define SEC_IDAU_SOC_PMU                    BIT6            /*  (1UL << 6)    */
#define SEC_IDAU_FLASH_CTRL                 BIT9            /*  (1UL << 9)    */
#define SEC_IDAU_TIMER0                     BIT10           /*  (1UL << 10)   */
#define SEC_IDAU_TIMER1                     BIT11           /*  (1UL << 11)   */
#define SEC_IDAU_TIMER2                     BIT12           /*  (1UL << 12)   */
#define SEC_IDAU_SLOWTIMER0                 BIT13           /*  (1UL << 13)   */
#define SEC_IDAU_SLOWTIMER1                 BIT14           /*  (1UL << 14)   */
#define SEC_IDAU_WDT                        BIT16           /*  (1UL << 16)   */
#define SEC_IDAU_UART0                      BIT18           /*  (1UL << 18)   */
#define SEC_IDAU_UART1                      BIT19           /*  (1UL << 19)   */
#define SEC_IDAU_I2C_SLAVE                  BIT24           /*  (1UL << 24)   */
#define SEC_IDAU_COMM_AHB                   BIT26           /*  (1UL << 26)   */
#define SEC_IDAU_RCO32_CAL                  BIT28           /*  (1UL << 28)   */
#define SEC_IDAU_BOC_CMP                    BIT29           /*  (1UL << 29)   */
#define SEC_IDAU_AUX_CMP                    BIT30           /*  (1UL << 30)   */
#define SEC_IDAU_RCO1M_CAL                  BIT31           /*  (1UL << 31)   */
/*For REG SEC_PERI_ATTR1*/
#define SEC_IDAU_QSPI0                      BIT0            /*  (1UL << 0)    */
#define SEC_IDAU_QSPI1                      BIT1            /*  (1UL << 1)    */
#define SEC_IDAU_IRM                        BIT4            /*  (1UL << 4)    */
#define SEC_IDAU_UART2                      BIT5            /*  (1UL << 5)    */
#define SEC_IDAU_PWM                        BIT6            /*  (1UL << 6)    */
#define SEC_IDAU_XDMA                       BIT8            /*  (1UL << 8)    */
#define SEC_IDAU_DMA0                       BIT9            /*  (1UL << 9)    */
#define SEC_IDAU_DMA1                       BIT10           /*  (1UL << 10)   */
#define SEC_IDAU_I2C_MASTER0                BIT11           /*  (1UL << 11)   */
#define SEC_IDAU_I2C_MASTER1                BIT12           /*  (1UL << 12)   */
#define SEC_IDAU_I2S0_MASTER                BIT13           /*  (1UL << 13)   */
#define SEC_IDAU_SADC                       BIT16           /*  (1UL << 15)   */
#define SEC_IDAU_SW_IRQ0                    BIT16           /*  (1UL << 16)   */
#define SEC_IDAU_SW_IRQ1                    BIT17           /*  (1UL << 17)   */
#define SEC_PERI_ATTRI1                                     0
/*For REG SEC_PERI_ATTR2*/
#define SEC_IDAU_CRYPTO                     BIT0            /*  (1UL << 0)    */
#define SEC_IDAU_OTP                        BIT1            /*  (1UL << 1)    */


#define SEC_WRITE_OTP_MAGIC_KEY         (0x28514260)

#ifdef __cplusplus
}
#endif

#endif /* End of SEC_CONTROL_REG_H */
