/*---------------------------------------------------------------------------*
  Project:  TwlSDK - screenshot test - Ext
  File:     ext_keycontrol.h

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

#include <nitro/types.h>
#include <nitro/ext.h>


/*---------------------------------------------------------------------------*
  Name:         EXT_AutoKeys

  Description:  Key input

  Arguments:    mode: Current display mode
  
  Returns:      Type of current capture mode.
                (See GXCaptureMode for more detail.)
 *---------------------------------------------------------------------------*/
void EXT_AutoKeys(const EXTKeys *sequence, u16 *cont, u16 *trig)
{
    // Key sequence counter
    static u16 absolute_cnt = 0;
    static u16 last_key = 0;
    u16     cnt;

    cnt = absolute_cnt;
    while (cnt >= sequence->count)
    {
        // If there is a sequence with a count value of 0, regard it as the end
        if (sequence->count == 0)
        {
            return;
        }
        cnt -= sequence->count;
        sequence++;
    }

    *cont |= sequence->key;
    *trig = (u16)(~last_key & *cont);
    last_key = *cont;
    absolute_cnt++;
}
