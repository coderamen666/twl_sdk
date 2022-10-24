/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_exMemory.c

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
  Name:         MI_SetAgbCartridgeFastestRomCycle

  Description:  set Cartridge ROM access cycles for AGB. (8-4 cycle)

  Arguments:    prev1st : pointer to be store the previous 1st ROM access cycle.
                          if NULL, no effect.
                prev2nd : pointer to be store the previous 2nd ROM access cycle.
                          if NULL, no effect.

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_SetAgbCartridgeFastestRomCycle(MICartridgeRomCycle1st *prev1st,
                                       MICartridgeRomCycle2nd *prev2nd)
{
    //---- store previous value
    if (prev1st)
    {
        *prev1st = MI_GetCartridgeRomCycle1st();
    }
    if (prev2nd)
    {
        *prev2nd = MI_GetCartridgeRomCycle2nd();
    }

    //---- set cycles for AGB cartridge setting
    MI_SetCartridgeRomCycle1st(MI_CTRDG_ROMCYCLE1_8);
    MI_SetCartridgeRomCycle2nd(MI_CTRDG_ROMCYCLE2_4);
}
