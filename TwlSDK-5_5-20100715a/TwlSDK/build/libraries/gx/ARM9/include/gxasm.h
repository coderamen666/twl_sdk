/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - 
  File:     gxasm.h

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

#ifndef NITRO_GXASM_H_
#define NITRO_GXASM_H_

#include <nitro/mi/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

void    GX_SendFifo36B(register const void *pSrc, register void *pDest);
void    GX_SendFifo48B(register const void *pSrc, register void *pDest);
void    GX_SendFifo64B(register const void *pSrc, register void *pDest);
void    GX_SendFifo128B(register const void *pSrc, register void *pDest);



static inline void GX_SendFifo36B(register const void *pSrc, register void *pDest)
{
    MI_Copy36B(pSrc, pDest);
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
