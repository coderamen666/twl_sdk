/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2di_Dma.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef G2DI_DMA_H_
#define G2DI_DMA_H_

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dDmaCopy16

  Description:  Copies the DMA.

  Arguments:    dmaNo:  DMA number
                        (When GX_DMA_NOT_USE is specified, CPU copying is used instead of DMA copying.)
                          
                src:    transmission source address
                dest:   transmission destination address
                size:   size of the transmission

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNSi_G2dDmaCopy16( u32 dmaNo, const void* src, void* dest, u32 size )
{
    // When GX_DMA_NOT_USE is specified in dmaNo, CpuCopy is used.
    if( dmaNo != GX_DMA_NOT_USE )
    {
        MI_DmaCopy16( dmaNo, src, dest, size );
    }else{
        MI_CpuCopy16( src, dest, size );
    }
}
                      

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dDmaFill32

  Description:  Fills the memory with the specified data.

  Arguments:    dmaNo:  DMA number
                        (When GX_DMA_NOT_USE is specified, CpuFill is used instead of DmaFill.)
                          
                dest:   transmission destination address
                data:   transmission data
                size:   size of the transmission

  Returns:      None.
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNSi_G2dDmaFill32( u32 dmaNo, void* dest, u32 data, u32 size )
{
    // When GX_DMA_NOT_USE is specified in dmaNo, CpuFill is used.
    if( dmaNo != GX_DMA_NOT_USE )
    {
        MI_DmaFill32( dmaNo, dest, data, size );
    }else{
        MI_CpuFill32( dest, data, size );
    }
}

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // G2DI_DMA_H_

