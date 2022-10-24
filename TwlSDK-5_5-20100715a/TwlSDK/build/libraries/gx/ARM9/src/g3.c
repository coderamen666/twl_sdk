/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - 
  File:     g3.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <nitro/gx/g3.h>


/*---------------------------------------------------------------------------*
  Name:         G3_BeginMakeDL

  Description:  Initialize GXDLInfo before display list generation.

  Arguments:    info         a pointer to display list info.
                ptr          a pointer to the buffer where a display list is stored
                length       the length of the buffer(in bytes)

  Returns:      none
 *---------------------------------------------------------------------------*/
void G3_BeginMakeDL(GXDLInfo *info, void *ptr, u32 length)
{
    SDK_ALIGN4_ASSERT(ptr);
    SDK_ALIGN4_ASSERT(length);

    info->length = length;
    info->bottom = (u32 *)ptr;
    info->curr_cmd = (u8 *)ptr;
    info->curr_param = (u32 *)ptr + 1;
    info->param0_cmd_flg = FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         G3_EndMakeDL

  Description:  Finish generating a display list.

  Arguments:    info         a pointer to display list info

  Returns:      none
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>              // avoid byte access problems
u32 G3_EndMakeDL(GXDLInfo *info)
{
    u32     sz;
    if ((u8 *)info->bottom == info->curr_cmd)
    {
        // no display list generated
        return 0;
    }

    switch ((u32)info->curr_cmd & 3)
    {
        // padding
    case 0:
        SDK_ASSERT((u32)info->bottom < (u32)info->curr_cmd);
        return (u32)((u32)info->curr_cmd - (u32)info->bottom);
    case 1:
        *info->curr_cmd++ = 0;         // byte access
    case 2:
        *info->curr_cmd++ = 0;
    case 3:
        *info->curr_cmd++ = 0;
    };

    // code for hardware bug in geometry fifo.
    if (info->param0_cmd_flg)
    {
        *(u32 *)(info->curr_param++) = 0;
        info->param0_cmd_flg = FALSE;
    }

    info->curr_cmd = (u8 *)info->curr_param;

    SDK_ASSERT((u32)info->bottom < (u32)info->curr_cmd);
    SDK_TASSERTMSG(((u32)info->curr_cmd - (u32)info->bottom <= info->length),
                  "Buffer overflow ! : Current DL buffer doesn't have enough capacity for new commands\n");

    sz = (u32)((u32)info->curr_cmd - (u32)info->bottom);

    return sz;
}

#include <nitro/codereset.h>
