/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - libraries -
  File:     wxc_scheduler.c

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-16#$
  $Rev: 9661 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

#include <nitro/wxc/common.h>
#include <nitro/wxc/scheduler.h>


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WXCi_InitScheduler

  Description:  Initializes the mode-switch scheduler.

  Arguments:    p: WXCScheduler structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_InitScheduler(WXCScheduler * p)
{
    /*
     * Initialize using the default schedule, which seems to be appropriate, based on experience.
     * This setting is usually changed only for debugging.
     */
    static const BOOL default_table[WXC_SCHEDULER_PATTERN_MAX][WXC_SCHEDULER_SEQ_MAX] = {
        {TRUE, FALSE, TRUE, TRUE},
        {FALSE, TRUE, TRUE, TRUE},
        {FALSE, TRUE, TRUE, TRUE},
        {TRUE, FALSE, TRUE, TRUE},
    };
    p->seq = (int)((OS_GetTick() >> 0) % WXC_SCHEDULER_SEQ_MAX);
    p->pattern = (int)((OS_GetTick() >> 2) % WXC_SCHEDULER_PATTERN_MAX);
    p->start = 0;
    p->child_mode = FALSE;
    MI_CpuCopy32(default_table, p->table, sizeof(default_table));
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_SetChildMode

  Description:  Sets scheduler to operate with child device fixed.

  Arguments:    p: WXCScheduler structure
                enable: If it can only be run on the child side, TRUE

  Returns:      None.
 *---------------------------------------------------------------------------*/
void WXCi_SetChildMode(WXCScheduler * p, BOOL enable)
{
    p->child_mode = enable;
}

/*---------------------------------------------------------------------------*
  Name:         WXCi_UpdateScheduler

  Description:  Updates mode switch scheduler.

  Arguments:    p: WXCScheduler structure

  Returns:      Return TRUE if currently in parent mode. Return FALSE if currently in child mode.
 *---------------------------------------------------------------------------*/
BOOL WXCi_UpdateScheduler(WXCScheduler * p)
{
    if (++p->seq >= WXC_SCHEDULER_SEQ_MAX)
    {
        p->seq = 0;
        if (++p->pattern >= WXC_SCHEDULER_PATTERN_MAX)
        {
            p->pattern = 0;
        }
        if (p->pattern == p->start)
        {
            /* Supposed to be random number */
            p->start = (int)(OS_GetTick() % WXC_SCHEDULER_PATTERN_MAX);
            p->pattern = p->start;
        }
    }
    return p->table[p->pattern][p->seq] && !p->child_mode;
}
