/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     swap.h

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

#ifndef NITRO_MI_SWAP_H_
#define NITRO_MI_SWAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

/*---------------------------------------------------------------------------*
  Name:         MI_SwapWord / MI_SwapByte

  Description:  swap data and memory

  Arguments:    setData      data to swap
                destp        memory address to swap

  Returns:      swapped memory data

  *Notice: Cannot access to main memory by byte unless cache.
           Use MI_SwapWord() not MI_SwapByte basically.
 *---------------------------------------------------------------------------*/
//---- by word
u32     MI_SwapWord(u32 setData, volatile u32 *destp);

//---- by byte
u8      MI_SwapByte(u32 setData, volatile u8 *destp);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_SWAP_H_ */
#endif
