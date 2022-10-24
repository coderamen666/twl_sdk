/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_dma_mainmem.c

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
  Name:         MI_DispMemDmaCopy

  Description:  main memory display DMA.

  Arguments:    dmaNo   : DMA channel No.
                src     : source address

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_DispMemDmaCopy(u32 dmaNo, const void *src)
{
    MIi_ASSERT_DMANO(dmaNo);
    MIi_ASSERT_SRC_ALIGN4(src);
    MIi_WARNING_ADDRINTCM(src, 0x18000);        // 0x18000=256*192*2(byte)

    MIi_CheckAnotherAutoDMA(dmaNo, MI_DMA_TIMING_DISP_MMEM);
    //---- check DMA0 source address
    MIi_CheckDma0SourceAddress(dmaNo, (u32)src, 0x18000, MI_DMA_SRC_INC);

    MIi_DmaSetParameters(dmaNo, (u32)src, (u32)REG_DISP_MMEM_FIFO_ADDR, MI_CNT_MMCOPY(0), 0);
}
#endif // SDK_ARM9
