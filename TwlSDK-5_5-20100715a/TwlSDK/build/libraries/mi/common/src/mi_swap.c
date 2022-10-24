/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_swap.c

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

#include <nitro/types.h>
#include <nitro/mi/swap.h>


//  MI_Swap* will be used for control common resource exclusively among processers
//  or modules. Beside, it's used for realization spin lock system.
//
//  notice: you cannot access main memory by byte unless via cache.
//  so, use MI_SwapWord generally for accessing main memory ,not MI_SwapByte.


//---- This code will be compiled in ARM-Mode
#include <nitro/code32.h>

/*---------------------------------------------------------------------------*
  Name:         MI_SwapWord

  Description:  swap data and memory

  Arguments:    setData      data to swap
                destp        memory address to swap

  Returns:      swapped memory data
 *---------------------------------------------------------------------------*/
asm u32 MI_SwapWord( register u32 setData, register vu32* destp )
{
        swp     r0, r0, [r1]
        bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         MI_SwapByte

  Description:  swap data and memory

  Arguments:    setData      data to swap
                destp        memory address to swap

  Returns:      swapped memory data
 *---------------------------------------------------------------------------*/
asm u8  MI_SwapByte( register u32 setData, register vu8* destp )
{
        swpb    r0, r0, [r1]
        bx      lr
}

//---- end limitation of ARM-Mode
#include <nitro/codereset.h>
