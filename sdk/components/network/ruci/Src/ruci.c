/*
 * Copyright (c) 2022-2025 Rafael Microelectronics Inc. All rights reserved.
 * 
 * SPDX-License-Identifier: LicenseRef-RafaelMicro-Proprietary-1.0
 *
 */

/******************************************************************************
*
* @File  RFB_ruci.c
* @Version
* $Revision:
* $Date:
* @Brief
* @Note
*
******************************************************************************/

/******************************************************************************
* INCLUDES
******************************************************************************/
#include "ruci.h"



/*******************************************************************************
*   GLOBAL FUNCTIONS
*******************************************************************************/
#if (RUCI_ENDIAN_INVERSE)

void ruci_endian_convert(
    uint8_t  *pDataIn,
    uint8_t  ParaNum,
    const uint8_t  *pTypeMapIn,
    const uint8_t  *pElemNumMapIn
)
{
    uint8_t  ParaIdx, ElemIdx;
    uint8_t  pDwTemp[4];
    uint8_t  *pData = pDataIn;
    const uint8_t  *pTypeMap = pTypeMapIn;
    const uint8_t  *pElemNumMap = pElemNumMapIn;

    for (ParaIdx = 0; ParaIdx < ParaNum; ParaIdx++)
    {
        switch (*pTypeMap)
        {
        case (4):
            // Swap 4 bytes ([byte0, byte1, byte2, byte3] ->
            // [byte3, byte2, byte1, byte0])
            for (ElemIdx = 0; ElemIdx < (*pElemNumMap); ElemIdx++)
            {
                pDwTemp[0] = pData[0];
                pDwTemp[1] = pData[1];
                pDwTemp[2] = pData[2];
                pDwTemp[3] = pData[3];
                pData[0] = pDwTemp[3];
                pData[1] = pDwTemp[2];
                pData[2] = pDwTemp[1];
                pData[3] = pDwTemp[0];
                pData += 4;
            }
            break;

        case (2):
            // Swap 2 bytes ([byte0, byte1] -> [byte1, byte0])
            for (ElemIdx = 0; ElemIdx < (*pElemNumMap); ElemIdx++)
            {
                pDwTemp[0] = pData[0];
                pDwTemp[1] = pData[1];
                pData[0] = pDwTemp[1];
                pData[1] = pDwTemp[0];
                pData += 2;
            }
            break;

        case (1):
            // Do nothing
            pData += (*pElemNumMap);
            break;

        default:
            break;
        }

        pTypeMap++;
        pElemNumMap++;
    }
}

#endif /* RUCI_ENDIAN_INVERSE */
