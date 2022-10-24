/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_tick.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-15#$
  $Rev: 10752 $
  $Author: terui $
 *---------------------------------------------------------------------------*/
#include <nitro/os.h>

//----------------------------------------------------------------------
//---- timer control setting for tick
#define OSi_TICK_TIMERCONTROL  ( REG_OS_TM0CNT_H_E_MASK | REG_OS_TM0CNT_H_I_MASK | OS_TIMER_PRESCALER_64 )

//---- timer number tick uses
#define OSi_TICK_TIMER         OS_TIMER_0

//---- timer interrupt mask (must be same number with OSi_TICK_TIMER)
#define OSi_TICK_IE_TIMER      OS_IE_TIMER0

//---- flag for initialization tick
static u16 OSi_UseTick = FALSE;

//---- tick counter
vu64    OSi_TickCounter;

//---- flag for need to re-set timer
BOOL    OSi_NeedResetTimer = FALSE;


extern u16 OSi_IsTimerReserved(int timerNum);
extern void OSi_SetTimerReserved(int timerNum);

static void OSi_CountUpTick(void);
u32     OSi_GetTick_noProfile(void);


/*---------------------------------------------------------------------------*
  Name:         OS_InitTick

  Description:  initialize 'tick system'

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_InitTick(void)
{
    if (!OSi_UseTick)
    {
        OSi_UseTick = TRUE;

        //---- OS reserves OSi_TICK_TIMER timer
        SDK_ASSERT(!OSi_IsTimerReserved(OSi_TICK_TIMER));
        OSi_SetTimerReserved(OSi_TICK_TIMER);

        //---- setting timer
        OSi_TickCounter = 0;
        OS_SetTimerControl(OSi_TICK_TIMER, 0);
        OS_SetTimerCount((OSTimer)OSi_TICK_TIMER, (u16)0);
        OS_SetTimerControl(OSi_TICK_TIMER, (u16)OSi_TICK_TIMERCONTROL);

        //---- set interrupt callback
        OS_SetIrqFunction(OSi_TICK_IE_TIMER, OSi_CountUpTick);

        //---- enable timer interrupt
        (void)OS_EnableIrqMask(OSi_TICK_IE_TIMER);

        //---- need to reset
        OSi_NeedResetTimer = FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsTickAvailable

  Description:  check tick system is available

  Arguments:    None

  Returns:      if available, TRUE.
 *---------------------------------------------------------------------------*/
BOOL OS_IsTickAvailable(void)
{
    return OSi_UseTick;
}


/*---------------------------------------------------------------------------*
  Name:         OSi_CountUpTick

  Description:  timer interrupt handle. 

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static void OSi_CountUpTick(void)
{
    OSi_TickCounter++;

    //---- setting for timer
    if (OSi_NeedResetTimer)
    {
        OS_SetTimerControl(OSi_TICK_TIMER, 0);
        OS_SetTimerCount((OSTimer)OSi_TICK_TIMER, (u16)0);
        OS_SetTimerControl(OSi_TICK_TIMER, (u16)OSi_TICK_TIMERCONTROL);

        OSi_NeedResetTimer = FALSE;
    }

    //---- reset callback
    OSi_EnterTimerCallback(OSi_TICK_TIMER, (void (*)(void *))OSi_CountUpTick, 0);
}

#ifdef SDK_ARM7
#ifdef SDK_TWL
extern u32 MIC_GetDelayIF(void);
#endif
#endif
/*---------------------------------------------------------------------------*
  Name:         OS_GetTick

  Description:  get tick value

  Arguments:    None

  Returns:      tick value
 *---------------------------------------------------------------------------*/
OSTick OS_GetTick(void)
{
    vu16    countL;
    vu64    countH;

    OSIntrMode prev = OS_DisableInterrupts();
    SDK_ASSERT(OSi_UseTick);

    countL = *(REGType16 *)((u32)REG_TM0CNT_L_ADDR + OSi_TICK_TIMER * 4);
    countH = OSi_TickCounter & 0xffffffffffffULL;

    //---- check if timer interrupt bit is on
#ifdef SDK_ARM7
#ifdef SDK_TWL
    if (((reg_OS_IF | MIC_GetDelayIF()) & OSi_TICK_IE_TIMER) && !(countL & 0x8000))
#else
    if (reg_OS_IF & OSi_TICK_IE_TIMER && !(countL & 0x8000))
#endif
#else
    if (reg_OS_IF & OSi_TICK_IE_TIMER && !(countL & 0x8000))
#endif
    {
        countH++;
    }

    (void)OS_RestoreInterrupts(prev);

    return (countH << 16) | countL;
}

//---- for profiler
#pragma profile off
u32 OSi_GetTick_noProfile(void)
{
    vu16    countL;
    vu16    countH;

    OSIntrMode prev = OS_DisableInterrupts();
    SDK_ASSERT(OSi_UseTick);

    countL = *(REGType16 *)((u32)REG_TM0CNT_L_ADDR + OSi_TICK_TIMER * 4);
    countH = (u16)OSi_TickCounter;

    //---- check if timer interrupt bit is on
    if (reg_OS_IF & OSi_TICK_IE_TIMER && !(countL & 0x8000))
    {
        countH++;
    }

    (void)OS_RestoreInterrupts(prev);

    return (u32)(countH << 16) | countL;
}

#pragma profile reset

/*---------------------------------------------------------------------------*
  Name:         OS_GetTickLo

  Description:  get tick value (only u16 part)

  Arguments:    None

  Returns:      tick value (only u16 part)
 *---------------------------------------------------------------------------*/
u16 OS_GetTickLo(void)
{
    SDK_ASSERT(OSi_UseTick);
    return *(REGType16 *)((u32)REG_TM0CNT_L_ADDR + OSi_TICK_TIMER * 4);
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetTick

  Description:  set tick value

  Arguments:    count     value of tick to be set

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_SetTick(OSTick count)
{
    OSIntrMode prev;

    SDK_ASSERT(OSi_UseTick);
    prev = OS_DisableInterrupts();

    reg_OS_IF = OSi_TICK_IE_TIMER;

    OSi_NeedResetTimer = TRUE;

    OSi_TickCounter = (u64)(count >> 16);

    OS_SetTimerControl(OSi_TICK_TIMER, 0);
    OS_SetTimerCount((OSTimer)OSi_TICK_TIMER, (u16)(count & 0xffff));
    OS_SetTimerControl(OSi_TICK_TIMER, (u16)OSi_TICK_TIMERCONTROL);

    (void)OS_RestoreInterrupts(prev);
}
