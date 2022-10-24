/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - PXI
  File:     compparam.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_PXI_COMMON_COMPPARAM_H_
#define NITRO_PXI_COMMON_COMPPARAM_H_

#ifndef SDK_TWL
#include <nitro/types.h>
#include <nitro/memorymap.h>
#else
#include <twl/types.h>
#include <twl/memorymap.h>
#endif

#include <nitro/mi.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         PXI_SetComponentParam

  Description:  Set component parameter for other processor sync
                - This parameter should be set before OS_InitLock() for safety
                FYI. OS_InitLock are called in OS_Init, take care!!!
 
  Arguments:    param : parameter to be set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void PXI_SetComponentParam(u32 param)
{
    MI_WriteWord(HW_COMPONENT_PARAM, param);
}

/*---------------------------------------------------------------------------*
  Name:         PXI_GetComponentParam

  Description:  Get component parameter from other processor

  Arguments:    None

  Returns:      parameter
 *---------------------------------------------------------------------------*/
static inline u32 PXI_GetComponentParam(void)
{
    return MI_ReadWord(HW_COMPONENT_PARAM);
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/* NITRO_PXI_COMMON_COMPPARAM_H_ */
#endif
