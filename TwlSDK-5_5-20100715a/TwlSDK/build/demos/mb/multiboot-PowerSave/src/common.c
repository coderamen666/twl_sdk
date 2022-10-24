/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot-PowerSave
  File:     common.c

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/


#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif
#include "common.h"


/*****************************************************************************/
/* Variables */

BOOL    g_power_save_mode;


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         ReadKeySetTrigger

  Description:  Detect key trigger.

  Arguments:    None.

  Returns: The bit set of key input that was NOT pressed the last time this function was called but was pressed during this function call.
                
 *---------------------------------------------------------------------------*/
u16 ReadKeySetTrigger(void)
{
    /* Measures to handle A button presses inside IPL */
    static u16 cont = 0xffff;
    u16     keyset = PAD_Read();
    u16     trigger = (u16)(keyset & (keyset ^ cont));
    cont = keyset;
    return trigger;
}
