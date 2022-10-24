/*---------------------------------------------------------------------------*
  Project:  TwlSDK - RTC - include
  File:     rtc.h

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
#ifndef	NITRO_RTC_H_
#define	NITRO_RTC_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include	<nitro/rtc/common/type.h>
#include	<nitro/rtc/common/fifo.h>

#ifdef	SDK_ARM7

#include	<nitro/rtc/ARM7/control.h>
#include	<nitro/rtc/ARM7/instruction.h>
#include	<nitro/rtc/ARM7/gpio.h>

#else  // SDK_ARM9

#include	<nitro/rtc/ARM9/api.h>
#include	<nitro/rtc/ARM9/convert.h>
#include	<nitro/rtc/ARM9/swclock.h>

#endif

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* NITRO_RTC_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
