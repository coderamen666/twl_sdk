/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_dma_card.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "../include/mi_dma.h"


/*---------------------------------------------------------------------------*
  Name:         MIi_CardDmaCopy32

  Description:  CARD DMA copy.
                32bit, sync version.
                (This sets only DMA-control. CARD register must be set after)

  Arguments:    dmaNo: DMA channel number
                src: Source address
                dest: destination address
                size: Transfer size (bytes)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MIi_CardDmaCopy32(u32 dmaNo, const void *src, void *dest, u32 size)
{
    vu32   *dmaCntp;

    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_DEST_ALIGN4(dest);
    /* Size is not used in this function, but specify to make determinations */
    MIi_ASSERT_SRC_ALIGN512(size);
    MIi_WARNING_ADDRINTCM(dest, size);
    (void)size;

#ifdef SDK_ARM9
    /*
     * CARD DMA differs from other DMA in that there is no possibility of doing same types in parallel.
     * Although there is centralized management for the CARD library on the calling side, since check functionality is available anyway, set to MIi_DMA_TIMING_ANY to also detect same types.
     * 
     */
    MIi_CheckAnotherAutoDMA(dmaNo, MIi_DMA_TIMING_ANY);
#endif
    //---- Check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, size, MI_DMA_SRC_FIX);

    if (size == 0)
    {
        return;
    }

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
//    MIi_DmaSetParams(dmaNo, (u32)src, (u32)dest,
//                     (u32)(MI_CNT_CARDRECV32(4) | MI_DMA_CONTINUOUS_ON));
    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)dest,
						 (u32)(MI_CNT_CARDRECV32(4) | MI_DMA_CONTINUOUS_ON), 0);
    /*
     * Here, automatic startup was just turned on.
     * It will start up for the first time when commands are configured in the CARD register.
     */
}
