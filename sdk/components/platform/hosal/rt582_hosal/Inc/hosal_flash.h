/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/**
 * \file            hosal_flash.h
 * \brief           hosal_flash include file
 */

/*
 * This file is part of library_name.
 * Author:  ives.lee 
 */
#ifndef HOSAL_FLASH_H
#define HOSAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "flashctl.h"
#include "flash_bp_protect.h"


/**
 * \defgroup HOSAL_FLASH Hosal flash
 * \ingroup RT58X_HOSAL
 * \brief  Define Hosal flash definitions, structures, and functions
 * @{
 */
/**
 * \brief          Debug printf
 */
#define HOSAL_FLASH_DEBUG       0 
#if (HOSAL_FLASH_DEBUG==1)
#define HOSAL_FLASH_DEBUG_LOG(...)  do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while (0)
#else
#define HOSAL_FLASH_DEBUG_LOG(...)  do { } while (0)
#endif
/**
 * \brief           Status Register definitions
 */
#define  HOSAL_FLASH_STATUS_RW1     FLASH_STATUS_RW1                /*!< Status Register 1  */
#define  HOSAL_FLASH_STATUS_RW2     FLASH_STATUS_RW2                 /*!< Status Register 2  */
#define  HOSAL_FLASH_STATUS_RW3     FLASH_STATUS_RW3                /*!< Status Register 3  */

/**
 * \brief           Status Register Byte 1 and Byte 2, this define for writing status Bytes2
 */
#define  HOSAL_FLASH_STATUS_RW1_2   FLASH_STATUS_RW1_2
/**
* \brief flash read command
 */
#define HOSAL_FLASH_READ_BYTE				0x00
#define HOSAL_FLASH_READ_PAGE				0x01
#define HOSAL_FLASH_SECURITY_READ			0x02
#define HOSAL_FLASH_READ_STATUS				0x03
#define HOSAL_FLASH_READ_ID					0x04

/**
* \brief flash write command
 */
#define HOSAL_FLASH_WRITE_BYTE				0x10
#define HOSAL_FLASH_WRITE_PAGE				0x11
#define HOSAL_FLASH_SECURITY_WRITE			0x12
#define HOSAL_FLASH_WRITE_STATUS			0x13

/**
 * \brief flash erase command 
 */
#define HOSAL_FLASH_ERASE_PAGE				0x20
#define HOSAL_FLASH_ERASE_SECTOR			0x21
#define HOSAL_FLASH_ERASE_32K_SECTOR		0x22
#define HOSAL_FLASH_ERASE_64K_SECTOR		0x23
#define HOSAL_FLASH_ERASE_CHIP				0x24
#define HOSAL_FLASH_ERASE_SECURITY_PAGE		0x25

/**
 * \brief flash io control command 
 */
#define HOSAL_FLASH_ENABLE_SUSPEND			0x30
#define HOSAL_FLASH_DISABLE_SUSPEND			0x31
#define HOSAL_FLASH_CACHE					0x32
#define HOSAL_FLASH_BUSY            		0x33
#define HOSAL_FLASH_GET_INFO				0x34


/* ================= HOSAL Flash BP Control Commands ================= */

typedef enum {
    HOSAL_FLASH_BP_PROTECT_ENABLE,      // Enable Flash Block Protection (BP) mechanism
    HOSAL_FLASH_BP_GET_DEVICE_INFO,     // Get flash device information
    HOSAL_FLASH_BP_SET_REGION,          // Configure block protection region
    HOSAL_FLASH_BP_GET_STATUS,          // Retrieve current BP protection status
    HOSAL_FLASH_BP_SET_CMP_REGION,      // Configure CMP inverted protection region
    HOSAL_FLASH_BP_VERIFY_STATUS,       // Verify flash status registers (SR1/SR2)
    HOSAL_FLASH_BP_REMOVE_ALL,          // Remove all BP protections
    HOSAL_FLASH_BP_PRINT_STATUS,        // Print current BP protection information
    HOSAL_FLASH_BP_CHECK_ADDRESS,       // Check whether a specific address is protected
} hosal_flash_bp_ctl_t;

/* ================= HOSAL Flash BP Mode ================= */
typedef struct {
    flash_bp_mode_t bp_value;        // Raw BP value (use FLASH_BP_xxx definitions)
    bool    cmp_enabled;     // CMP mode flag (true = inverted, false = normal)
} hosal_flash_bp_mode_t;

/* ================= HOSAL Flash BP Status ================= */
typedef struct {
    flash_bp_mode_t  bp_value;        // Current raw BP value
    bool     cmp_enabled;     // Current CMP mode status
    uint32_t start_addr;      // Protected region start address (inclusive)
    uint32_t end_addr;        // Protected region end address (exclusive)
    uint32_t size;            // Protected region size in bytes
    const char *description;  // Human-readable protection description
} hosal_flash_bp_status_t;

/* ================= HOSAL Flash BP Address Check ================= */
typedef struct {
    uint32_t address;        // Address to be checked
    bool     is_protected;   // Output: true if the address is protected
} hosal_flash_bp_addr_check_t;
/* ================= HOSAL Flash BP Device Info ================= */
typedef struct {
    const void *device_ptr;  //  flash_bp_device_t
} hosal_flash_bp_device_info_t;
/**
* \brief            Flash size definitions.
*/
typedef enum {
    HOSAL_FLASH_NOT_SUPPORT = 0x00,
    HOSAL_FLASH_512K = 0x13,                          /*!< 512K size   */
    HOSAL_FLASH_1024K = 0x14,                         /*!< 1024K size   */
    HOSAL_FLASH_2048K = 0x15,                         /*!< 2048K size   */
    HOSAL_FLASH_4096K = 0x16,                         /*!< 4096K size   */
} hosal_flash_size_t;



/**
 * \brief               flash conifg struct table
 *                      This structure is used to calculate all point
 *                      related stuff
 */
typedef struct {
	uint32_t address;							 /*!< flash address */
	uint32_t length;							 /*!< flash data length  */	
	uint32_t buf_address;						 /*!< flash data buffer address  */
}hosal_flash_config_t;	

/**
 * \brief               flash conifg struct table
 *                      This structure is used to calculate all point
 *                      related stuff
 */
typedef struct {
    uint8_t require_mode;                       /*!< bitwise mode to indicate read/write operation */
    uint8_t status1;                            /*!< flash status1 for read/write */
    uint8_t status2;                            /*!< flash status2 for read/write */
    uint8_t status3;                            /*!< flash status3 for read/write */
} hosal_flash_status_t;

/**
 * \brief               sw portect enum
 *                      
 *                      
 */
typedef enum {
    HOSAL_FLASH_SW_PROTECT_ENABLE = 0,
    HOSAL_FLASH_SW_PROTECT_DISABLE,
    HOSAL_FLASH_SW_PROTECT_GET_STATUS,
} hosal_flash_sw_protect_ctl_t;
/**
 * \brief           enable flash
 * \param[in]       NONE
 * \return           function status
 */
int hosal_flash_enable_qe(void);

/**
 * \brief           hosal_flash_init
 * \param[in]       NONE
 * \return          function status
 */
int hosal_flash_init(void);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_read(int ctl, uint32_t address, uint8_t *buf);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_write(int ctl, uint32_t address, uint8_t *buf);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \return          function status
 */
int hosal_flash_erase(int ctl, uint32_t address);

/**
 * \brief           flash io control function
 * \param[in]       ctl: control command
 * \param[in]       address: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_ioctrl(int ctl, void *p_arg);

/**
 * \brief           flash io control function
 * \param[in]       length: flash address
 * \param[in]       buf: flash buffer address
 * \return          function status
 */
int hosal_flash_read_id(uint32_t length, uint8_t* buf);
/**
 * \brief Write data to flash memory.
 *
 * Writes a continuous block of data to flash starting from the given address.
 * The flash area must be erased before writing.
 *
 * \param[in] address  Flash start address.
 * \param[in] buf      Pointer to data buffer.
 * \param[in] len      Number of bytes to write.
 *
 * \retval STATUS_SUCCESS         Write successful.
 * \retval STATUS_INVALID_PARAM   Invalid parameter.
 * \retval STATUS_EBUSY           Flash is busy.
 */
int hosal_flash_write_n_bytes(uint32_t address,uint8_t* buf,uint32_t len);

/**
 * \brief read data frm flash memory.
 *
 *
 * \param[in] address  Flash start address.
 * \param[in] buf      Pointer to data buffer.
 * \param[in] len      Number of bytes to read.
 *
 * \retval STATUS_SUCCESS         Read successful.
 * \retval STATUS_INVALID_PARAM   Invalid parameter.
 * \retval STATUS_EBUSY           Flash is busy.
 */
int hosal_flash_read_n_bytes(uint32_t address,uint8_t* buf,uint32_t len);




/* ================= HOSAL Flash BP API ================= */

/**
 * \brief HOSAL Flash BP control functin
 * \param ctl control command
 * \param p_arg paramater point
 * \return HOSAL_STATUS_SUCCESS or error code
 * 
 * Paramater table
 * - HOSAL_FLASH_BP_PROTECT_ENABLE   : p_arg = NULL
 * - HOSAL_FLASH_BP_SET_REGION       : p_arg = hosal_flash_bp_mode_t*
 * - HOSAL_FLASH_BP_GET_STATUS       : p_arg = hosal_flash_bp_status_t*
 * - HOSAL_FLASH_BP_SET_CMP_REGION   : p_arg = hosal_flash_bp_mode_t*
 * - HOSAL_FLASH_BP_VERIFY_STATUS    : p_arg = NULL
 * - HOSAL_FLASH_BP_REMOVE_ALL       : p_arg = NULL
 * - HOSAL_FLASH_BP_PRINT_STATUS     : p_arg = NULL
 * - HOSAL_FLASH_BP_CHECK_ADDRESS    : p_arg = hosal_flash_bp_addr_check_t*
 */
int hosal_flash_bp_status_ctrl(hosal_flash_bp_ctl_t ctl, void *p_arg);

/**
 * \brief HOSAL Flash BP control functin
 * \param ctl control command
 * \param p_arg paramater point
 * \return HOSAL_STATUS_SUCCESS or error code
 * 
 * ctl command
 *   HOSAL_FLASH_SW_PROTECT_ENABLE
 *   HOSAL_FLASH_SW_PROTECT_DISABLE
 *   HOSAL_FLASH_SW_PROTECT_GET_STATUS
 */
int hosal_flash_sw_protect_ctrl(int ctl, void *p_arg);
/*@}*/ /* end of RT584_HOSAL HOSAL_FLASH */

#ifdef __cplusplus
}
#endif

#endif /* End of HOSAL_FLASH_H */
