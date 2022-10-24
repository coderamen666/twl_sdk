/*---------------------------------------------------------------------------*
  Project:  NitroSDK - GX - 
  File:     gxasm.c

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

#include <nitro/code32.h>
#include "../include/gxasm.h"

asm void GX_SendFifo48B(register const void* pSrc, register void* pDest)
{
    ldmia   r0!, {r2,r3,r12}           // r0-r3, r12 need not be saved.
    stmia   r1,  {r2,r3,r12}
    ldmia   r0!, {r2,r3,r12}
    stmia   r1,  {r2,r3,r12}
    ldmia   r0!, {r2,r3,r12}
    stmia   r1,  {r2,r3,r12}
    ldmia   r0!, {r2,r3,r12}
    stmia   r1,  {r2,r3,r12}

    bx      lr
}

asm void GX_SendFifo64B(register const void* pSrc, register void* pDest)
{
    stmfd   sp!, {r4-r8}               // r0-r3, r12 need not be saved.

    ldmia   r0!, {r2-r8, r12}
    stmia   r1,  {r2-r8, r12}
    ldmia   r0!, {r2-r8, r12}
    stmia   r1,  {r2-r8, r12}

    ldmfd   sp!, {r4-r8}
    bx      lr
}

asm void GX_SendFifo128B(register const void* pSrc, register void* pDest)
{
    stmfd   sp!, {r4-r8}               // r0-r3, r12 need not be saved.

    ldmia   r0!, {r2-r8, r12}
    stmia   r1,  {r2-r8, r12}
    ldmia   r0!, {r2-r8, r12}
    stmia   r1,  {r2-r8, r12}
    ldmia   r0!, {r2-r8, r12}
    stmia   r1,  {r2-r8, r12}
    ldmia   r0!, {r2-r8, r12}
    stmia   r1,  {r2-r8, r12}

    ldmfd   sp!, {r4-r8}
    bx      lr
}
