/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - snd - common
  File:     alarm.h

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

#ifndef NITRO_SND_COMMON_ALARM_H_
#define NITRO_SND_COMMON_ALARM_H_

#include <nitro/types.h>
#include <nitro/os.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
	macro definition
 ******************************************************************************/

#define SND_ALARM_NUM 8
#define SND_ALARM_MIN 0
#define SND_ALARM_MAX ( SND_ALARM_NUM - 1 )

#define SND_ALARM_PRESCALER 32

/******************************************************************************
	type definition
 ******************************************************************************/

typedef void (*SNDAlarmHandler) (void *);

/******************************************************************************
	structure definition
 ******************************************************************************/

typedef struct SNDAlarm
{
    u8      enable;
    u8      id;
    u8      count;
    u8      padding;

    struct
    {
        OSTick  tick;
        OSTick  period;
    }
    setting;

    OSAlarm alarm;
}
SNDAlarm;

/******************************************************************************
	public function declaration
 ******************************************************************************/

void    SND_AlarmInit(void);

#ifdef SDK_ARM7

void    SND_SetupAlarm(int alarmNo, OSTick tick, OSTick period, int id);

void    SND_StartAlarm(int alarmNo);
void    SND_StopAlarm(int alarmNo);

#endif /* SDK_ARM7 */

/******************************************************************************
	private function declaration
 ******************************************************************************/

#ifdef SDK_ARM9

void    SNDi_IncAlarmId(int alarmNo);

u8      SNDi_SetAlarmHandler(int alarmNo, SNDAlarmHandler handler, void *arg);

void    SNDi_CallAlarmHandler(int alarmNo);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SND_COMMON_ALARM_H_ */
