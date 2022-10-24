/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_event.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-21#$
  $Rev: 9385 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_EVENT_H__
#define NITRO_LIBRARIES_CARD_EVENT_H__


#include <nitro.h>


// These structures and functions are used to monitor software state changes without completion notifications.
// Currently, these are used only by the CARD_LockRom function but are split into a separate module.


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */


// This structure is used to monitor software state changes without completion notifications.
typedef struct CARDEventListener
{
    OSEvent     event[1];
    OSVAlarm    valarm[1];
    BOOL(*Condition)(void*);
    void       *userdata;
    u16         lockID;
    u8          padding[2];
}
CARDEventListener;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_PeekEventListener

  Description:  Determines event conditions.
                A V-count alarm is issued if the conditions are not met.

  Arguments:    arg: CARDEventListener structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_PeekEventListener(void *arg)
{
    CARDEventListener  *el = (CARDEventListener *)arg;
    if (el->Condition(el->userdata))
    {
        OS_SignalEvent(el->event, 0x00000001UL);
    }
    else
    {
        OS_SetVAlarm(el->valarm, HW_LCD_HEIGHT, OS_VALARM_DELAY_MAX, CARDi_PeekEventListener, el);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitEventListener

  Description:  Initializes an event listener structure.

  Arguments:    el: CARDEventListener structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_InitEventListener(CARDEventListener *el)
{
    OS_InitEvent(el->event);
    OS_CreateVAlarm(el->valarm);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetEventListener

  Description:   Configures an event in an event listener structure.

  Arguments:    el: CARDEventListener structure
                condition: Event conditions
                userdata: Arbitrary user-defined arguments that can be specified as event conditions

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_SetEventListener(CARDEventListener *el, BOOL(*condition)(void*), void *userdata)
{
    el->Condition = condition;
    el->userdata = userdata;
    CARDi_PeekEventListener(el);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitForEvent

  Description:   Waits for an event listener structure to complete.

  Arguments:    el: CARDEventListener structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_WaitForEvent(CARDEventListener *el)
{
    (void)OS_WaitEventEx(el->event, 0x00000001UL, OS_EVENT_MODE_AND, 0x00000001UL);
}


/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_LIBRARIES_CARD_EVENT_H__
