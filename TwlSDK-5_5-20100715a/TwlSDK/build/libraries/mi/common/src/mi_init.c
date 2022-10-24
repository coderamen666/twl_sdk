/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_init.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-01#$
  $Rev: 9448 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#ifdef SDK_TWL
#include <twl/mi/common/dma.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_Init

  Description:  initialize MI functions

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_Init(void)
{
#ifdef SDK_ARM9
    //---- Init Work RAM (allocate both wram (block0/1) to ARM7)
    MI_SetWramBank(MI_WRAM_ARM7_ALL);
#endif

    if ( OS_IsRunOnTwl() )
    {
#ifdef SDK_TWL
        MI_InitNDma();
#endif
    }

    //---- dummy DMA
    MI_StopDma(0);
}
