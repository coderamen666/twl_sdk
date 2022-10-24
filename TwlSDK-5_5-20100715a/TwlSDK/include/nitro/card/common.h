/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     common.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_COMMON_H_
#define NITRO_CARD_COMMON_H_


#include <nitro/card/types.h>

#include <nitro/memorymap.h>
#include <nitro/mi/dma.h>
#include <nitro/os.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

// Card thread's default priority level
#define	CARD_THREAD_PRIORITY_DEFAULT	4


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_Init

  Description:  Initializes the CARD library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_Init(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_IsAvailable

  Description:  Determines if the CARD library is usable.

  Arguments:    None.

  Returns:      TRUE if the CARD_Init function has already been called.
 *---------------------------------------------------------------------------*/
BOOL    CARD_IsAvailable(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_IsEnabled

  Description:  Determines if the Game Card is accessible.

  Arguments:    None.

  Returns:      TRUE if we have access rights to the Game Card.
 *---------------------------------------------------------------------------*/
BOOL    CARD_IsEnabled(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckEnabled

  Description:  Determines if we have access rights to the Game Card and performs a forced shutdown if we do not.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_CheckEnabled(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_Enable

  Description:  Sets access rights to the Game Card.

  Arguments:    enable: TRUE if we have access rights.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_Enable(BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetThreadPriority

  Description:  Gets the priority of asynchronous processing threads in the library.

  Arguments:    None.

  Returns:      Internal thread priority.
 *---------------------------------------------------------------------------*/
u32     CARD_GetThreadPriority(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_SetThreadPriority

  Description:  Sets the priority of asynchronous processing threads in the library.

  Arguments:    None.

  Returns:      Internal thread priority before it is set.
 *---------------------------------------------------------------------------*/
u32     CARD_SetThreadPriority(u32 prior);

/*---------------------------------------------------------------------------*
  Name:         CARD_GetResultCode

  Description:  Gets the result of the last CARD function to be called.

  Arguments:    None.

  Returns:      The result of the last CARD function to be called.
 *---------------------------------------------------------------------------*/
CARDResult CARD_GetResultCode(void);


/*---------------------------------------------------------------------------*
 * for internal use
 *---------------------------------------------------------------------------*/

// Internal CARD library event
typedef u32 CARDEvent;
#define CARD_EVENT_PULLEDOUT  0x00000001
#define CARD_EVENT_SLOTRESET  0x00000002

// Event hook callback and hook registration structure
typedef void (*CARDHookFunction)(void*, CARDEvent, void*);

typedef struct CARDHookContext
{
    struct CARDHookContext *next;
    void                   *userdata;
    CARDHookFunction        callback;
}
CARDHookContext;

/*---------------------------------------------------------------------------*
  Name:         CARDi_RegisterHook

  Description:  Registers an event hook within the CARD library.

  Arguments:    hook: Hook structure to use during registration
                callback: Callback function to invoke when the event occurs
                arg:     Callback argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_RegisterHook(CARDHookContext *hook, CARDHookFunction callback, void *arg);

/*---------------------------------------------------------------------------*
  Name:         CARDi_UnregisterHook

  Description:  Removes an event hook within the CARD library.

  Arguments:    hook: Hook structure used during registration

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_UnregisterHook(CARDHookContext *hook);

/*---------------------------------------------------------------------------*
  Name:         CARDi_NotifyEvent

  Description:  Notifies all hook functions of an internal CARD library event.

  Arguments:    event: The event that occurred
                arg: argument for each event

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_NotifyEvent(CARDEvent event, void *arg);


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_COMMON_H_
