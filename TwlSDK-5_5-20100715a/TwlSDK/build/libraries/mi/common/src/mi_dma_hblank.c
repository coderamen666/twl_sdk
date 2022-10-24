/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_dma_hblank.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-16#$
  $Rev: 8957 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#ifdef SDK_ARM9
#include <nitro.h>
#include "../include/mi_dma.h"

/*---------------------------------------------------------------------------*
  Name:         MI_HBlankDmaCopy32

  Description:  HBlank DMA copy.
                32bit.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_HBlankDmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_H_BLANK);
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);
    if (size == 0)
    {
        return;
    }

    MI_WaitDma(dmaNo);
//    MIi_DmaSetParams(dmaNo, (u32)src, (u32)dest, MI_CNT_HBCOPY32(size));
    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_HBCOPY32(size), 0);
}

/*---------------------------------------------------------------------------*
  Name:         MI_HBlankDmaCopy16

  Description:  HBlank DMA copy.
                16bit.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_HBlankDmaCopy16(u32 dmaNo, const void *src, void *dest, u32 size)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_H_BLANK);
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);
    if (size == 0)
    {
        return;
    }

    MI_WaitDma(dmaNo);
    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_HBCOPY16(size), 0);
}

/*---------------------------------------------------------------------------*
  Name:         MI_HBlankDmaCopy32If

  Description:  HBlank DMA copy.
                32bit.
                occur interrupt when finish DMA.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_HBlankDmaCopy32If(u32 dmaNo, const void *src, void *dest, u32 size)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_ASSERT_DEST_ALIGN4(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_H_BLANK);
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);
    if (size == 0)
    {
        return;
    }

    MI_WaitDma(dmaNo);
    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_HBCOPY32_IF(size), 0);
}

/*---------------------------------------------------------------------------*
  Name:         MI_HBlankDmaCopy16If

  Description:  HBlank DMA copy.
                16bit.
                occur interrupt when finish DMA.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address
                dest    : destination address
                size    : transfer size (byte)

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_HBlankDmaCopy16If(u32 dmaNo, const void *src, void *dest, u32 size)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN2(src);
    MIi_ASSERT_DEST_ALIGN2(dest);
    MIi_WARNING_ADDRINTCM(src, size);
    MIi_WARNING_ADDRINTCM(dest, size);

    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_H_BLANK);
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_INC);
    if (size == 0)
    {
        return;
    }

    MI_WaitDma(dmaNo);
    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest, MI_CNT_HBCOPY16_IF(size), 0);
}
#endif // SDK_ARM9
