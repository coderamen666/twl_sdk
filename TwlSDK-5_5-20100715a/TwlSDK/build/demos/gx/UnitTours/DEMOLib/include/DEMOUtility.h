/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOUtility.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
  
 *---------------------------------------------------------------------------*/
#ifndef DEMO_UTILITY_H_
#define DEMO_UTILITY_H_
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void    DEMO_Set3DDefaultMaterial(BOOL bUsediffuseAsVtxCol, BOOL bUseShininessTbl);
void    DEMO_Set3DDefaultShininessTable();

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif /* DEMO_UTILITY_H_ */
