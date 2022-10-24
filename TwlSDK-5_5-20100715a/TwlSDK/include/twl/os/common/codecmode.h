/*---------------------------------------------------------------------------*
  Project:  TwlSDK - os - include
  File:     codecmode.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-08#$
  $Rev: 8904 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#ifndef TWL_OS_COMMON_CODECMODE_H_
#define TWL_OS_COMMON_CODECMODE_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include    <twl/types.h>
#ifdef  SDK_ARM9
#include    <twl/memorymap.h>
#else
#include    <twl/memorymap_sp.h>
#endif

typedef enum
{
    OS_CODECMODE_NITRO = 0,
    OS_CODECMODE_TWL   = 1
}
OSCodecMode;

#define OS_IsCodecTwlMode OSi_IsCodecTwlMode

//---- internal
#ifdef SDK_TWL
BOOL OSi_IsCodecTwlMode(void);
#else // SDK_NITRO
#define OSi_IsCodecTwlMode()  (FALSE)
#endif

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_OS_COMMON_CODECMODE_H_ */
