/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_event.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl/memorymap.h>
#else
#include <nitro/memorymap.h>
#endif

#include <nitro/os.h>


#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_begin.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_InitEvent

  Description:  initialize event struct

  Arguments:    event   : pointer to event struct

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_InitEvent( OSEvent *event )
{
	OS_InitThreadQueue( &event->queue );
	event->flag    = 0;
}

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
u32 OS_WaitEvent(OSEvent* event, u32 pattern, OSEventMode mode )
{
	return OS_WaitEventEx(event, pattern, mode, 0);
}

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
u32 OS_WaitEventEx(OSEvent* event, u32 pattern, OSEventMode mode, u32 clearBit )
{
	u32 retval = 0;
    OSIntrMode enable = OS_DisableInterrupts();

	SDK_ASSERT( event );

	switch( mode )
	{
		case OS_EVENT_MODE_AND:
			while( (event->flag & pattern) != pattern )
			{
				OS_SleepThread(&event->queue);
			}
			retval = event->flag;
			break;
		case OS_EVENT_MODE_OR:
			while( (event->flag & pattern) == 0 )
			{
				OS_SleepThread(&event->queue);
			}
			retval = event->flag;
			break;
	}

	//---- clear flag
	if ( retval )
	{
		event->flag &= ~clearBit;
	}

    (void)OS_RestoreInterrupts(enable);
	return retval;
}

/*---------------------------------------------------------------------------*
  Name:         OS_SignalEvent

  Description:  set event bit and wakeup thread.
                if setPattern == 0, do nothing

  Arguments:    event      : pointer to event struct
                setPattern : bit pattern to set

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_SignalEvent(OSEvent* event, u32 setPattern)
{
    OSIntrMode enable = OS_DisableInterrupts();
	SDK_ASSERT( event );

	if ( setPattern )
	{
		event->flag |= setPattern;
		OS_WakeupThread( &event->queue );
	}

	(void)OS_RestoreInterrupts(enable);
}

/*---------------------------------------------------------------------------*
  Name:         OS_ClearAllEvent

  Description:  clear all event bits

  Arguments:    event  : pointer to event struct

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_ClearAllEvent(OSEvent* event)
{
	SDK_ASSERT( event );
	event->flag = 0;
}

/*---------------------------------------------------------------------------*
  Name:         OS_ClearEvent

  Description:  clear specified event bits

  Arguments:    event    : pointer to event struct
                clearBit : clear bits

  Returns:      none
 *---------------------------------------------------------------------------*/
void OS_ClearEvent(OSEvent* event, u32 clearBit)
{
    OSIntrMode enable = OS_DisableInterrupts();
	SDK_ASSERT( event );

	event->flag &= ~clearBit;

	(void)OS_RestoreInterrupts(enable);
}

/*---------------------------------------------------------------------------*
  Name:         OS_PollEvent

  Description:  poll event struct if condition matches

  Arguments:    event   : waiting condition
                pattern : event pattern
                mode    : event mode
                           OS_EVENT_MODE_AND ... wait for all event bit
                           OS_EVENT_MODE_OR  ... wait for any event bit

  Returns:      not 0  ... matched. return the event flag.
                0      ... not match.
 *---------------------------------------------------------------------------*/
u32 OS_PollEvent(OSEvent* event, u32 pattern, OSEventMode mode )
{
	return OS_PollEventEx( event, pattern, mode, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         OS_PollEventEx

  Description:  poll event struct if condition matches

  Arguments:    event    : pointer to event struct
                pattern  : event pattern
                mode     : event mode
                            OS_EVENT_MODE_AND ... wait for all event bit
                            OS_EVENT_MODE_OR  ... wait for any event bit
                clearBit : clear bits (in case of matching)

  Returns:      not 0  ... matched. return the event flag.
                0      ... not match.
 *---------------------------------------------------------------------------*/
u32 OS_PollEventEx(OSEvent* event, u32 pattern, OSEventMode mode, u32 clearBit )
{
	u32 retval = 0;
    OSIntrMode enable = OS_DisableInterrupts();

	SDK_ASSERT( event );

	switch( mode )
	{
		case OS_EVENT_MODE_AND:
			if ( (event->flag & pattern) == pattern )
			{
				retval = event->flag;
			}
			break;
		case OS_EVENT_MODE_OR:
			if ( event->flag & pattern )
			{
				retval = event->flag;
			}
			break;
	}

	//---- clear flag
	if ( retval )
	{
		event->flag &= ~clearBit;
	}

    (void)OS_RestoreInterrupts(enable);
	return retval;
}

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_end.h>
#endif
