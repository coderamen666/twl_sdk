/*---------------------------------------------------------------------------*
  Project:  TwlSDK - rtc - include
  File:     rtc.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_RTC_H_
#define TWL_RTC_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include    <nitro/rtc.h>
#include    <twl/rtc/common/type_ex.h>
#include    <twl/rtc/common/fifo_ex.h>

#ifdef  SDK_ARM7
#include    <twl/rtc/ARM7/instruction_ex.h>

#else   /* SDK_ARM9 */
#include    <twl/rtc/ARM9/api_ex.h>

#endif

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_RTC_H_ */
