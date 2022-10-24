/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_byteAccess.c

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

#include <nitro/misc.h>
#include <nitro/mi/byteAccess.h>

/*---------------------------------------------------------------------------*
  Name:         MI_ReadByte

  Description:  read bytedata

  Arguments:    address ... address to read bytedata

  Returns:      byte data
 *---------------------------------------------------------------------------*/
#ifdef	SDK_TEG
u8 MI_ReadByte(const void *address)
{
    if ((u32)address & 1)
    {
        return (u8)(*(u16 *)((u32)address & ~1) >> 8);
    }
    else
    {
        return (u8)(*(u16 *)address & 0xff);
    }
}
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_WriteByte

  Description:  write bytedata

  Arguments:    address ... address to write bytedata

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef	SDK_TEG
void MI_WriteByte(void *address, u8 value)
{
    u16     val = *(u16 *)((u32)address & ~1);

    if ((u32)address & 1)
    {
        *(u16 *)((u32)address & ~1) = (u16)(((value & 0xff) << 8) | (val & 0xff));
    }
    else
    {
        *(u16 *)((u32)address & ~1) = (u16)((val & 0xff00) | (value & 0xff));
    }
}
#endif
