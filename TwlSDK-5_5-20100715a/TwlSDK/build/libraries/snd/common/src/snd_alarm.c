/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - snd - common
  File:     snd_alarm.c

  Copyright 2004-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#include <nitro/snd/common/alarm.h>

#include <nitro/pxi.h>
#include <nitro/snd/common/work.h>

/******************************************************************************
	structure definition
 ******************************************************************************/

#ifdef SDK_ARM9

typedef struct AlarmCallbackInfo
{
    void    (*func) (void *);
    void   *arg;
    u8      id;
    u8      pad;
    u16     pad2;
}
AlarmCallbackInfo;

#endif

/******************************************************************************
	static variable
 ******************************************************************************/

#ifdef SDK_ARM9

static AlarmCallbackInfo sCallbackTable[SND_ALARM_NUM];

#endif

/******************************************************************************
	static function declaration
 ******************************************************************************/

#ifdef SDK_ARM7

static void AlarmHandler(void *arg);

#endif

/******************************************************************************
	public function
 ******************************************************************************/

#ifdef SDK_ARM9

void SND_AlarmInit(void)
{
    int     alarmNo;

    for (alarmNo = 0; alarmNo < SND_ALARM_NUM; alarmNo++)
    {
        AlarmCallbackInfo *info;

        info = &sCallbackTable[alarmNo];

        info->func = NULL;
        info->arg = NULL;
        info->id = 0;
    }
}

#else

void SND_AlarmInit(void)
{
    int     alarmNo;

    for (alarmNo = 0; alarmNo < SND_ALARM_NUM; alarmNo++)
    {
        SNDi_Work.alarm[alarmNo].enable = FALSE;
        SNDi_Work.alarm[alarmNo].id = 0;
    }
}

void SND_SetupAlarm(int alarmNo, OSTick tick, OSTick period, int id)
{
    SNDAlarm *alarm;

    SDK_MINMAX_ASSERT(alarmNo, SND_ALARM_MIN, SND_ALARM_MAX);
    SDK_MINMAX_ASSERT(id, 0x00, 0xff);

    alarm = &SNDi_Work.alarm[alarmNo];

    if (alarm->enable)
    {
        OS_CancelAlarm(&alarm->alarm);
        alarm->enable = FALSE;
    }

    alarm->setting.tick = tick;
    alarm->setting.period = period;
    alarm->id = (u8)id;
    alarm->count = 0;
}

void SND_StartAlarm(int alarmNo)
{
    OSTick  tick;
    OSTick  period;
    SNDAlarm *alarm;
    void   *arg;

    SDK_MINMAX_ASSERT(alarmNo, SND_ALARM_MIN, SND_ALARM_MAX);

    alarm = &SNDi_Work.alarm[alarmNo];

    if (alarm->enable)
    {
        OS_CancelAlarm(&alarm->alarm);
        alarm->enable = FALSE;
    }

    tick = alarm->setting.tick;
    period = alarm->setting.period;
    arg = (void *)((alarm->id << 8) | alarmNo);

    OS_CreateAlarm(&alarm->alarm);

    if (period == 0)
    {
        OS_SetAlarm(&alarm->alarm, tick, AlarmHandler, arg);
    }
    else
    {
        OS_SetPeriodicAlarm(&alarm->alarm, OS_GetTick() + tick, period, AlarmHandler, arg);
    }

    alarm->enable = TRUE;
}

void SND_StopAlarm(int alarmNo)
{
    SNDAlarm *alarm;

    SDK_MINMAX_ASSERT(alarmNo, SND_ALARM_MIN, SND_ALARM_MAX);

    alarm = &SNDi_Work.alarm[alarmNo];

    if (!alarm->enable)
        return;

    OS_CancelAlarm(&alarm->alarm);

    alarm->id++;
    alarm->enable = FALSE;
}
#endif

/******************************************************************************
	static function
 ******************************************************************************/

#ifdef SDK_ARM7

static void AlarmHandler(void *arg)
{
    u32 msg = (u32)arg;
    u32 alarmNo = msg & 0xff;
    u32 id = (msg >> 8) & 0xff;
    SNDAlarm *alarm;
    
    SDK_MINMAX_ASSERT(alarmNo, SND_ALARM_MIN, SND_ALARM_MAX);
        
    alarm = &SNDi_Work.alarm[alarmNo];
    if ( id != alarm->id ) {
        return;
    }
    
    if ( alarm->count < 0xff ) {
        alarm->count++;
    }
    
    while (alarm->count > 0)
    {
        if (PXI_SendWordByFifo(PXI_FIFO_TAG_SOUND, msg, FALSE) < 0) {
            break;
        }
        alarm->count--;
    }
}

#endif

/******************************************************************************
	private function
 ******************************************************************************/

#ifdef SDK_ARM9

void SNDi_IncAlarmId(int alarmNo)
{
    AlarmCallbackInfo *info;

    SDK_MINMAX_ASSERT(alarmNo, SND_ALARM_MIN, SND_ALARM_MAX);

    info = &sCallbackTable[alarmNo];

    info->id++;
}

u8 SNDi_SetAlarmHandler(int alarmNo, SNDAlarmHandler handler, void *arg)
{
    AlarmCallbackInfo *info;

    SDK_MINMAX_ASSERT(alarmNo, SND_ALARM_MIN, SND_ALARM_MAX);

    info = &sCallbackTable[alarmNo];

    info->func = handler;
    info->arg = arg;
    info->id++;

    return info->id;
}

void SNDi_CallAlarmHandler(int msg)
{
    AlarmCallbackInfo *info;
    int     alarmNo = msg & 0xff;
    int     id = (msg >> 8) & 0xff;

    SDK_MINMAX_ASSERT(alarmNo, SND_ALARM_MIN, SND_ALARM_MAX);

    info = &sCallbackTable[alarmNo];

    if (id == info->id)
    {
        if (info->func != NULL)
        {
            info->func(info->arg);
        }
    }
}

#endif

/*====== End of snd_alarm.c ======*/
