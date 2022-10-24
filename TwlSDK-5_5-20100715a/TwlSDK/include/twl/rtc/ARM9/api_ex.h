/*---------------------------------------------------------------------------*
  Project:  TwlSDK - rtc - include
  File:     api_ex.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_RTC_ARM9_API_EX_H_
#define TWL_RTC_ARM9_API_EX_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include    <nitro/rtc/common/type.h>

/*---------------------------------------------------------------------------*
    Function Definitions
 *---------------------------------------------------------------------------*/
/* Functions for controlling added TWL features */
RTCResult   RTCi_GetCounterAsync(u32 *count, RTCCallback callback, void *arg);
RTCResult   RTCi_GetCounter(u32 *count);
RTCResult   RTCi_GetFoutAsync(u16 *fout, RTCCallback callback, void *arg);
RTCResult   RTCi_GetFout(u16 *fout);
RTCResult   RTCi_SetFoutAsync(const u16 *fout, RTCCallback callback, void *arg);
RTCResult   RTCi_SetFout(const u16 *fout);


/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_RTC_ARM9_API_EX_H_ */
