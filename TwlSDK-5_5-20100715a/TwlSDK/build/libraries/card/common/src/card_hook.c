/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_hook.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-21#$
  $Rev: 5625 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro/card/common.h>


/*---------------------------------------------------------------------------*/
/* Variables */

static CARDHookContext *CARDiHookChain = NULL;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_RegisterHook

  Description:  Registers an internal event hook for the CARD library

  Arguments:    hook: Hook structure used for registration
                callback: Callback function called when an event occurs
                arg: Callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_RegisterHook(CARDHookContext *hook, CARDHookFunction callback, void *arg)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    hook->callback = callback;
    hook->userdata = arg;
    hook->next = CARDiHookChain;
    CARDiHookChain = hook;
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_UnregisterHook

  Description:  Deallocates an internal event hook of the CARD library 

  Arguments:    hook: Hook structure used for registration.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_UnregisterHook(CARDHookContext *hook)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    CARDHookContext **pp;
    for (pp = &CARDiHookChain; *pp; pp = &(*pp)->next)
    {
        if (*pp == hook)
        {
            *pp = (*pp)->next;
            break;
        }
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_NotifyEvent

  Description:  Notifies an internal event for the CARD library 

  Arguments:    event: Occurred event
                arg: Argument for each event

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_NotifyEvent(CARDEvent event, void *arg)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    CARDHookContext **pp = &CARDiHookChain;
    while (*pp)
    {
        CARDHookContext *hook = *pp;
        if (hook->callback)
        {
            (*hook->callback)(hook->userdata, event, arg);
        }
        // Consider cases where the hook is deallocated in the callback
        if (*pp == hook)
        {
            pp = &(*pp)->next;
        }
    }
    (void)OS_RestoreInterrupts(bak);
}
