/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_timer.c

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
#include <nitro/os.h>


//----------------------------------------------------------------------
#define OSi_TM_E_MASK      REG_OS_TM0CNT_H_E_MASK
#define OSi_TM_I_MASK      REG_OS_TM0CNT_H_I_MASK
#define OSi_TM_CH_MASK     REG_OS_TM1CNT_H_CH_MASK

#define OSi_TIMER_MASK(preScale)   ((u16)(OSi_TM_E_MASK | OSi_TM_I_MASK | (preScale)))
#define OSi_CASCADE_MASK           ((u16)(OSi_TM_E_MASK | OSi_TM_I_MASK | OSi_TM_CH_MASK))

//---- if OS reserved each timer, bit=1 (LSB is TIMER0)
static u16 OSi_TimerReserved = 0;

u16     OSi_IsTimerReserved(int timerNum);
void    OSi_SetTimerReserved(int timerNum);
void    OSi_UnsetTimerReserved(int timerNum);

static void OS_StopTimerAndUnsetHandler(OSTimer id);
static void OSi_TimerIntr0(void);
static void OSi_TimerIntr1(void);
static void OSi_TimerIntr2(void);

static void (*OSi_IntrTable[])(void) = { OSi_TimerIntr0, OSi_TimerIntr1, OSi_TimerIntr2 };
static OSIrqMask OSi_MaskTable[] = { OS_IE_TIMER0, OS_IE_TIMER1, OS_IE_TIMER2 };

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_IsTimerReserved

  Description:  check if specified timer is reserved for OS

  Arguments:    timerNum : timerNo (0-3)

  Returns:      non-0 if reserved
 *---------------------------------------------------------------------------*/
u16 OSi_IsTimerReserved(int timerNum)
{
    return (u16)(OSi_TimerReserved & (1 << timerNum));
}

/*---------------------------------------------------------------------------*
  Name:         OSi_SetTimerReserved

  Description:  set specified timer to reserved for OS

  Arguments:    timerNum : timerNo (0-3)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_SetTimerReserved(int timerNum)
{
    OSi_TimerReserved |= (1 << timerNum);
}

/*---------------------------------------------------------------------------*
  Name:         OSi_UnsetTimerReserved

  Description:  unset specified timer to reserved for OS

  Arguments:    timerNum : timerNo (0-3)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_UnsetTimerReserved(int timerNum)
{
    OSi_TimerReserved &= ~(1 << timerNum);
}

//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_StartTimer / OS_StartTimer32 / OS_StartTimer48 /OS_StartTimer64

  Description:  set timer(s) and start

  Arguments:    id         timerNo
                count      count value to be set to timer
                preScale   preScale

  Returns:      None
 *---------------------------------------------------------------------------*/
//   use 1 timer, 16bit counter,  timer<id> interrupt occurs by overflow
//
void OS_StartTimer(OSTimer id, u16 count, OSTimerPrescaler preScale)
{
    SDK_ASSERT(OS_TIMER_0 <= id && id <= OS_TIMER_3);
    SDK_ASSERT(OS_TIMER_PRESCALER_1 <= preScale && preScale <= OS_TIMER_PRESCALER_1024);
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(id));

	//---- set a count register
    OS_SetTimerCount(id, (u16)~count);

	//---- set a control register
    OS_SetTimerControl(id, OSi_TIMER_MASK(preScale));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   use 2 timers, 32bit counter, timer<id+1> interrupt occurs by overflow
//
void OS_StartTimer32(OSTimer32 id, u32 count, OSTimerPrescaler preScale)
{
    SDK_ASSERT(OS_TIMER32_01 <= id && id <= OS_TIMER32_23);
    SDK_ASSERT(OS_TIMER_PRESCALER_1 <= preScale && preScale <= OS_TIMER_PRESCALER_1024);
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(id));
    SDK_ASSERT(!OSi_IsTimerReserved(id + 1));

	//---- set count registers
    OS_SetTimerCount((OSTimer)((int)id + 1), (u16)((~count >> 16) & 0xffff));
    OS_SetTimerCount((OSTimer)id, (u16)(~count & 0xffff));

	//---- set a timer interrupt handler
    OS_SetIrqFunction(OSi_MaskTable[(int)id], OSi_IntrTable[(int)id]);

	//---- set control registers
    OS_SetTimerControl((OSTimer)((int)id + 1), OSi_CASCADE_MASK );
    OS_SetTimerControl((OSTimer)id, OSi_TIMER_MASK(preScale));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   use 3 timers, 48bit counter, timer<id+2> interrupt occurs by overflow
//
void OS_StartTimer48(OSTimer48 id, u64 count, OSTimerPrescaler preScale)
{
    SDK_ASSERT(OS_TIMER48_012 <= id && id <= OS_TIMER48_123);
    SDK_ASSERT(OS_TIMER_PRESCALER_1 <= preScale && preScale <= OS_TIMER_PRESCALER_1024);
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(id));
    SDK_ASSERT(!OSi_IsTimerReserved(id + 1));
    SDK_ASSERT(!OSi_IsTimerReserved(id + 2));

	//---- set count registers
    OS_SetTimerCount((OSTimer)((int)id + 2), (u16)((~count >> 32) & 0xffff));
    OS_SetTimerCount((OSTimer)((int)id + 1), (u16)((~count >> 16) & 0xffff));
    OS_SetTimerCount((OSTimer)id, (u16)(~count & 0xffff));

	//---- set timer interrupt handlers
    OS_SetIrqFunction(OSi_MaskTable[(int)id+1], OSi_IntrTable[(int)id+1]);
    OS_SetIrqFunction(OSi_MaskTable[(int)id], OSi_IntrTable[(int)id]);

	//---- set control registers
    OS_SetTimerControl((OSTimer)((int)id + 2), OSi_CASCADE_MASK );
    OS_SetTimerControl((OSTimer)((int)id + 1), OSi_CASCADE_MASK );
    OS_SetTimerControl((OSTimer)id, OSi_TIMER_MASK(preScale));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   use all 4 timers, 64bit counter, timer3 interrupt occurs by overflow
//
void OS_StartTimer64(u64 count, OSTimerPrescaler preScale)
{
    SDK_ASSERT(OS_TIMER_PRESCALER_1 <= preScale && preScale <= OS_TIMER_PRESCALER_1024);
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_0));
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_1));
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_2));
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_3));

	//---- set count registers
    OS_SetTimerCount(OS_TIMER_3, (u16)((~count >> 48) & 0xffff));
    OS_SetTimerCount(OS_TIMER_2, (u16)((~count >> 32) & 0xffff));
    OS_SetTimerCount(OS_TIMER_1, (u16)((~count >> 16) & 0xffff));
    OS_SetTimerCount(OS_TIMER_0, (u16)(~count & 0xffff));

	//---- set timer interrupt handlers
    OS_SetIrqFunction(OS_IE_TIMER2, OSi_TimerIntr2);
    OS_SetIrqFunction(OS_IE_TIMER1, OSi_TimerIntr1);
	OS_SetIrqFunction(OS_IE_TIMER0, OSi_TimerIntr0);

	//---- set control registers
    OS_SetTimerControl(OS_TIMER_3, OSi_CASCADE_MASK );
    OS_SetTimerControl(OS_TIMER_2, OSi_CASCADE_MASK );
    OS_SetTimerControl(OS_TIMER_1, OSi_CASCADE_MASK );
    OS_SetTimerControl(OS_TIMER_0, OSi_TIMER_MASK(preScale));
}

/*---------------------------------------------------------------------------*
  Name:         OS_StopTimer / OS_StopTimer32 / OS_StopTimer48 / OS_StopTimer64

  Description:  stop timer(s)

  Arguments:    id         timerNo

  Returns:      None
 *---------------------------------------------------------------------------*/
//  stop a timer
//
void OS_StopTimer(OSTimer id)
{
    SDK_ASSERT(OS_TIMER_0 <= id && id <= OS_TIMER_3);
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(id));

    OS_SetTimerControl(id, 0);
}

static void OS_StopTimerAndUnsetHandler(OSTimer id)
{
    OS_SetIrqFunction(OSi_MaskTable[(int)id], NULL);
	OS_StopTimer(id);
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//  stop 2 timers
//
void OS_StopTimer32(OSTimer32 id)
{
    SDK_ASSERT(OS_TIMER32_01 <= id && id <= OS_TIMER32_23);
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(id));
    SDK_ASSERT(!OSi_IsTimerReserved(id + 1));

	//---- clear timer setting
    OS_StopTimerAndUnsetHandler((OSTimer)id);
    OS_StopTimer((OSTimer)((int)id + 1));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//  stop 3 timers
//
void OS_StopTimer48(OSTimer48 id)
{
    SDK_ASSERT(OS_TIMER48_012 <= id && id <= OS_TIMER48_123);
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(id));
    SDK_ASSERT(!OSi_IsTimerReserved(id + 1));
    SDK_ASSERT(!OSi_IsTimerReserved(id + 2));

	//---- clear timer setting
    OS_StopTimerAndUnsetHandler((OSTimer)id);
    OS_StopTimerAndUnsetHandler((OSTimer)((int)id + 1));
    OS_StopTimer((OSTimer)((int)id + 2));
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//  stop all 4 timers
//
void OS_StopTimer64(void)
{
    //---- check if system reserved
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_0));
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_1));
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_2));
    SDK_ASSERT(!OSi_IsTimerReserved(OS_TIMER_3));

	//---- clear timer setting
    OS_StopTimerAndUnsetHandler(OS_TIMER_0);
    OS_StopTimerAndUnsetHandler(OS_TIMER_1);
    OS_StopTimerAndUnsetHandler(OS_TIMER_2);
    OS_StopTimer(OS_TIMER_3);
}

/*---------------------------------------------------------------------------*
  Name:         OSi_TimerIntr0 / OSi_TimerIntr1 / OSi_TimerIntr2

  Description:  timer 0/1/2 interrupt handler

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void OSi_TimerIntr0()
{
    OS_SetTimerCount( OS_TIMER_0, 0 );
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
static void OSi_TimerIntr1()
{
    OS_SetTimerCount( OS_TIMER_1, 0 );
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
static void OSi_TimerIntr2()
{
    OS_SetTimerCount( OS_TIMER_2, 0 );
}
