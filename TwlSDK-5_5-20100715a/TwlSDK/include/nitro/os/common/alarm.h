/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     alarm.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_ALARM_H_
#define NITRO_OS_ALARM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os/common/thread.h>
#include <nitro/os/common/tick.h>


//--------------------------------------------------------------------------------
//---- Alarm Handler
typedef void (*OSAlarmHandler) (void *);


//---- struct of Alarm
//typedef struct OSiAlarm OSAlarm;   // this is decleared in thread.h
struct OSiAlarm
{
    OSAlarmHandler handler;
    void   *arg;

    u32     tag;
    OSTick  fire;
    OSAlarm *prev;
    OSAlarm *next;

    //---- for periodic alarm
    OSTick  period;
    OSTick  start;
};

//---- Alarm resource
typedef struct OSAlarmResource
{
	int num;
}
OSAlarmResource;

//--------------------------------------------------------------------------------
/*---------------------------------------------------------------------------*
  Name:         OS_InitAlarm

  Description:  initalize alarm system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_InitAlarm(void);


/*---------------------------------------------------------------------------*
  Name:         OS_EndAlarm

  Description:  end alarm system

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_EndAlarm(void);


/*---------------------------------------------------------------------------*
  Name:         OS_IsAlarmAvailable

  Description:  check alarm system is available

  Arguments:    None

  Returns:      if available, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    OS_IsAlarmAvailable(void);


/*---------------------------------------------------------------------------*
  Name:         OS_CreateAlarm

  Description:  Create alarm

  Arguments:    alarm       pointer to alarm to be initialized

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_CreateAlarm(OSAlarm *alarm);


/*---------------------------------------------------------------------------*
  Name:         OS_SetAlarm

  Description:  Set alarm as a relative time

  Arguments:    alarm       pointer to alarm to be set
                tick        ticks to count before firing
                handler     alarm handler to be called
                arg         argument of handler

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_SetAlarm(OSAlarm *alarm, OSTick tick, OSAlarmHandler handler, void *arg);


/*---------------------------------------------------------------------------*
  Name:         OS_SetPeriodicAlarm

  Description:  set periodic alarm 

  Arguments:    alarm          pointer to alarm to be set
                start          origin of the period in absolute time
                period         ticks to count for each period
                handler        alarm handler to be called
                arg            argument of handler

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_SetPeriodicAlarm(OSAlarm *alarm, OSTick start, OSTick period, OSAlarmHandler handler,
                            void *arg);


/*---------------------------------------------------------------------------*
  Name:         OS_SetAlarmTag

  Description:  set tag which is used OS_CancelAlarms

  Arguments:    alarm        alarm to be set tag
                tag          tagNo

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_SetAlarmTag(OSAlarm *alarm, u32 tag);


/*---------------------------------------------------------------------------*
  Name:         OS_CancelAlarm

  Description:  Cancel alarm

  Arguments:    alarm       pointer to alarm to be canceled

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_CancelAlarm(OSAlarm *alarm);


/*---------------------------------------------------------------------------*
  Name:         OS_CancelAlarms

  Description:  cancel alarms which have specified tag

  Arguments:    tag          tagNo. to be cancelled. not 0

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_CancelAlarms(u32 tag);


/*---------------------------------------------------------------------------*
  Name:         OS_CancelAllAlarms

  Description:  cancel all alarms

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_CancelAllAlarms(void);


//================================================================================
//        FOR DEBUG
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetNumberOfAlarm

  Description:  get number of alarm

  Arguments:    None

  Returns:      number of alarm
 *---------------------------------------------------------------------------*/
int     OS_GetNumberOfAlarm(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetAlarmResource

  Description:  store resources of alarm to specified pointer

  Arguments:    resource       pointer to store alarm resources

  Returns:      TRUE  ... success (always return this now)
                FALSE ... fail
 *---------------------------------------------------------------------------*/
BOOL    OS_GetAlarmResource(OSAlarmResource *resource);


//================================================================================
//  The following definitions or declarations are for internal use.
//  Don't call these from use program.
struct OSiAlarmQueue
{
    OSAlarm *head;
    OSAlarm *tail;
};
struct OSiAlarmQueue *OSi_GetAlarmQueue(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_ALARM_H_ */
#endif
