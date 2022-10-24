/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     event.h

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

#ifndef NITRO_OS_EVENT_H_
#define NITRO_OS_EVENT_H_

#include <nitro/os/common/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

//---------------- event mode
typedef enum
{
	OS_EVENT_MODE_AND = 0,
	OS_EVENT_MODE_OR  = 1
}
OSEventMode;


//---------------- event parameter
typedef struct
{
	vu32           flag;
	OSThreadQueue  queue;
}
OSEvent;


/*---------------------------------------------------------------------------*
  Name:         OS_InitEvent

  Description:  initialize event struct

  Arguments:    event   : pointer to event struct

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_InitEvent( OSEvent *event );

/*---------------------------------------------------------------------------*
  Name:         OS_WaitEvent

  Description:  sleep thread and wait event
                if event is NULL, just sleep.

  Arguments:    event   : pointer to event struct
                pattern : event pattern
                mode    : event mode
                           OS_EVENT_MODE_AND ... wait for all event bit
                           OS_EVENT_MODE_OR  ... wait for any event bit

  Returns:      none
 *---------------------------------------------------------------------------*/
u32 OS_WaitEvent(OSEvent* event, u32 pattern, OSEventMode mode );

/*---------------------------------------------------------------------------*
  Name:         OS_WaitEventEx

  Description:  sleep thread and wait event
                if event is NULL, just sleep.

  Arguments:    event    : pointer to event struct
                pattern  : event pattern
                mode     : event mode
                            OS_EVENT_MODE_AND ... wait for all event bit
                            OS_EVENT_MODE_OR  ... wait for any event bit
                clearBit : clear bits (in case of the end of waiting)

  Returns:      none
 *---------------------------------------------------------------------------*/
u32 OS_WaitEventEx(OSEvent* event, u32 pattern, OSEventMode mode, u32 clearBit );


//---- inline access
static inline u32 OS_WaitEvent_And( OSEvent* event, u32 pattern )
{
	return OS_WaitEventEx( event, pattern, OS_EVENT_MODE_AND, 0 );
}
static inline u32 OS_WaitEvent_Or(OSEvent* event, u32 pattern )
{
	return OS_WaitEventEx( event, pattern, OS_EVENT_MODE_OR, 0 );
}
static inline u32 OS_WaitEventEx_And( OSEvent* event, u32 pattern, u32 clearBit )
{
	return OS_WaitEventEx( event, pattern, OS_EVENT_MODE_AND, clearBit );
}
static inline u32 OS_WaitEventEx_Or(OSEvent* event, u32 pattern, u32 clearBit )
{
	return OS_WaitEventEx( event, pattern, OS_EVENT_MODE_OR, clearBit );
}

/*---------------------------------------------------------------------------*
  Name:         OS_SignalEvent

  Description:  set event bit and wakeup thread.
                if setPattern == 0, do nothing

  Arguments:    event      : pointer to event struct
                setPattern : bit pattern to set

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_SignalEvent(OSEvent* event, u32 setPattern);

/*---------------------------------------------------------------------------*
  Name:         OS_ClearEvent

  Description:  clear specified event bits

  Arguments:    event    : pointer to event struct
                clearBit : clear bits

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_ClearEvent(OSEvent* event, u32 clearBit);

/*---------------------------------------------------------------------------*
  Name:         OS_ClearAllEvent

  Description:  clear all event bits

  Arguments:    event  : pointer to event struct

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_ClearAllEvent(OSEvent* event);

/*---------------------------------------------------------------------------*
  Name:         OS_PollEvent

  Description:  poll event struct if condition matches

  Arguments:    event   : waiting condition
                pattern : event pattern
                mode    : event mode
                           OS_EVENT_MODE_AND ... wait for all event bit
                           OS_EVENT_MODE_OR  ... wait for any event bit

  Returns:      not 0  ... matched. returnd the event flag.
                0      ... not match.
 *---------------------------------------------------------------------------*/
u32 OS_PollEvent(OSEvent* event, u32 pattern, OSEventMode mode );

/*---------------------------------------------------------------------------*
  Name:         OS_PollEventEx

  Description:  poll event struct if condition matches

  Arguments:    event    : pointer to event struct
                pattern  : event pattern
                mode     : event mode
                            OS_EVENT_MODE_AND ... wait for all event bit
                            OS_EVENT_MODE_OR  ... wait for any event bit
                clearBit : clear bits (in case of matching)

  Returns:      not 0  ... matched. returnd the event flag.
                0      ... not match.
 *---------------------------------------------------------------------------*/
u32 OS_PollEventEx(OSEvent* event, u32 pattern, OSEventMode mode, u32 clearBit );

//---- inline access
static inline u32 OS_PollEvent_And( OSEvent* event, u32 pattern )
{
	return OS_PollEventEx( event, pattern, OS_EVENT_MODE_AND, 0 );
}
static inline u32 OS_PollEvent_Or(OSEvent* event, u32 pattern )
{
	return OS_PollEventEx( event, pattern, OS_EVENT_MODE_OR, 0 );
}
static inline u32 OS_PollEventEx_And( OSEvent* event, u32 pattern, u32 clearBit )
{
	return OS_PollEventEx( event, pattern, OS_EVENT_MODE_AND, clearBit );
}
static inline u32 OS_PollEventEx_Or(OSEvent* event, u32 pattern, u32 clearBit )
{
	return OS_PollEventEx( event, pattern, OS_EVENT_MODE_OR, clearBit );
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_EVENT_H_ */
#endif
