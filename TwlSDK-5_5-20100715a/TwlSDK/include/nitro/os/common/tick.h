/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     tick.h

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

#ifndef NITRO_OS_TICK_H_
#define NITRO_OS_TICK_H_

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


//---- unit of tick
typedef u64 OSTick;

//---- conversion tick count <-> real time count
#define OS_SYSTEM_CLOCK                HW_SYSTEM_CLOCK

#define OSi_BOUND_SEC1	 			   128154       // 2^8 / (OS_SYSTEM_CLOCK/1000) = 128154.4
#define OSi_BOUND_SEC2 				   128  // 2^8 / OS_SYSTEM_CLOCK = 128.2
#define OSi_BOUND_TICK1				   67108        // 2^8 / (64*1000) = 67108.9
#define OSi_BOUND_TICK2 			   67108863     // 2^8 / 64 = 67108864

//---- sec to tick
#define  OS_MicroSecondsToTicks(   usec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u64)(usec)) / 64 / 1000 ))
#define  OS_MicroSecondsToTicks32( usec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u32)(usec)) / 64 / 1000 ))

#define  OS_MilliSecondsToTicks(   msec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u64)(msec)) / 64 ))
#define  OS_MilliSecondsToTicks32( msec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u32)(msec)) / 64 ))

#define  OS_SecondsToTicks(         sec ) ((OSTick)( (OS_SYSTEM_CLOCK * (u64)(sec)) / 64 ))
#define  OS_SecondsToTicks32(       sec ) ((OSTick)( (OS_SYSTEM_CLOCK * (u32)(sec)) / 64 ))

//---- tick to sec
#define  OS_TicksToMicroSeconds(   tick ) ( ((u64)(tick) * 64 * 1000) / (OS_SYSTEM_CLOCK/1000) )
#define  OS_TicksToMicroSeconds32( tick ) ( ((u32)(tick) * 64 * 1000) / (OS_SYSTEM_CLOCK/1000) )

#define  OS_TicksToMilliSeconds(   tick ) ( ((u64)(tick) * 64) / (OS_SYSTEM_CLOCK/1000) )
#define  OS_TicksToMilliSeconds32( tick ) ( ((u32)(tick) * 64) / (OS_SYSTEM_CLOCK/1000) )

#define  OS_TicksToSeconds(        tick ) ( ((u64)(tick) * 64) / OS_SYSTEM_CLOCK )
#define  OS_TicksToSeconds32(      tick ) ( ((u32)(tick) * 64) / OS_SYSTEM_CLOCK )

/*---------------------------------------------------------------------------*
  Name:         OS_InitTick

  Description:  Initialize tick system

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_InitTick(void);


/*---------------------------------------------------------------------------*
  Name:         OS_IsTickAvailable

  Description:  check tick system is available

  Arguments:    None

  Returns:      if available, TRUE.
 *---------------------------------------------------------------------------*/
BOOL    OS_IsTickAvailable(void);


/*---------------------------------------------------------------------------*
  Name:         OS_GetTick

  Description:  get tick value

  Arguments:    None

  Returns:      tick value
 *---------------------------------------------------------------------------*/
OSTick  OS_GetTick(void);


/*---------------------------------------------------------------------------*
  Name:         OS_GetTickLo

  Description:  get tick value (only u16 part)

  Arguments:    None

  Returns:      sytem clock value (only u16 part)
 *---------------------------------------------------------------------------*/
u16     OS_GetTickLo(void);


/*---------------------------------------------------------------------------*
  Name:         OS_SetTick

  Description:  set tick value

  Arguments:    count     value of tick to be set

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetTick(OSTick count);



#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_TICK_H_ */
#endif
