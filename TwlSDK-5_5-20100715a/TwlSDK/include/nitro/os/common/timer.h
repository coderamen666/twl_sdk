/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     timer.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_TIMER_H_
#define NITRO_OS_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#ifndef SDK_TWL
#include <nitro/ioreg.h>
#else
#include <twl/ioreg.h>
#endif

//----------------------------------------------------------------------
//---- pre-scaler
typedef enum
{
    OS_TIMER_PRESCALER_1 = (0UL << REG_OS_TM0CNT_H_PS_SHIFT),   // x 1
    OS_TIMER_PRESCALER_64 = (1UL << REG_OS_TM0CNT_H_PS_SHIFT),  // x 64
    OS_TIMER_PRESCALER_256 = (2UL << REG_OS_TM0CNT_H_PS_SHIFT), // x 256
    OS_TIMER_PRESCALER_1024 = (3UL << REG_OS_TM0CNT_H_PS_SHIFT) // x 1024
}
OSTimerPrescaler;

//---- timer number
typedef enum
{
    OS_TIMER_0 = 0,
    OS_TIMER_1 = 1,
    OS_TIMER_2 = 2,
    OS_TIMER_3 = 3
}
OSTimer;

//---- timer number ( if use 32 bit timer )
typedef enum
{
    OS_TIMER32_01 = 0,
    OS_TIMER32_12 = 1,
    OS_TIMER32_23 = 2
}
OSTimer32;

//---- timer number ( if use 48 bit timer )
typedef enum
{
    OS_TIMER48_012 = 0,
    OS_TIMER48_123 = 1
}
OSTimer48;

//================================================================================
//            TIMER
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SetTimerCount

  Description:  set timer count

  Arguments:    id         timerNo
                count      count value to be set to timer

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OS_SetTimerCount(OSTimer id, u16 count)
{
    SDK_ASSERT(OS_TIMER_0 <= id && id <= OS_TIMER_3);
    *((REGType16 *)((u32)REG_TM0CNT_L_ADDR + id * 4)) = count;
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetTimerControl

  Description:  set timer control

  Arguments:    id         timerNo
                control    control value to be set to timer

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OS_SetTimerControl(OSTimer id, u16 control)
{
    SDK_ASSERT(OS_TIMER_0 <= id && id <= OS_TIMER_3);
    *((REGType16 *)((u32)REG_TM0CNT_H_ADDR + id * 4)) = control;
}

/*---------------------------------------------------------------------------*
  Name:         OS_StartTimer

  Description:  set timer(s) and start

  Arguments:    id         timerNo
                count      count value to be set to timer
                preScale   preScale

  Returns:      None
 *---------------------------------------------------------------------------*/
//
//   use 1 timer, 16bit counter,  timer<id> interrupt occurs by overflow
//
void    OS_StartTimer(OSTimer id, u16 count, OSTimerPrescaler preScale);
//
//   use 2 timers, 32bit counter, timer<id+1> interrupt occurs by overflow
//
void    OS_StartTimer32(OSTimer32 id, u32 count, OSTimerPrescaler preScale);
//
//   use 3 timers, 48bit counter, timer<id+2> interrupt occurs by overflow
//
void    OS_StartTimer48(OSTimer48 id, u64 count, OSTimerPrescaler preScale);
//
//   use all 4 timers, 64bit counter, timer3 interrupt occurs by overflow
//
void    OS_StartTimer64(u64 count, OSTimerPrescaler preScale);


/*---------------------------------------------------------------------------*
  Name:         OS_StopTimer

  Description:  stop timer(s)

  Arguments:    id         timerNo

  Returns:      None
 *---------------------------------------------------------------------------*/
//
//  stop a timer
//
void    OS_StopTimer(OSTimer id);
//
//  stop 2 timers
//
void    OS_StopTimer32(OSTimer32 id);
//
//  stop 3 timers
//
void    OS_StopTimer48(OSTimer48 id);
//
//  stop all 4 timers
//
void    OS_StopTimer64(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_TIMER_H_ */
#endif
