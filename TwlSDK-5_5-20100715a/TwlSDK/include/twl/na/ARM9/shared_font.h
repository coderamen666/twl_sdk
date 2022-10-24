/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NA - include
  File:     shared_font.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_NA_ARM9_SHARED_FONT_H_
#define TWL_NA_ARM9_SHARED_FONT_H_

#include	<twl/misc.h>
#include	<twl/types.h>
#include    <nitro/os/common/emulator.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/


typedef enum NASharedFontIndex
{
    NA_SHARED_FONT_WW_L = 0,
    NA_SHARED_FONT_WW_M = 1,
    NA_SHARED_FONT_WW_S = 2,
	NA_SHARED_FONT_MAX  = 3,
	
	NA_SHARED_FONT_CN_L = NA_SHARED_FONT_MAX,
    NA_SHARED_FONT_CN_M = 4,
    NA_SHARED_FONT_CN_S = 5,
    NA_SHARED_FONT_KR_L = 6,
    NA_SHARED_FONT_KR_M = 7,
    NA_SHARED_FONT_KR_S = 8,
    NA_SHARED_FONT_CN_KR_MAX = 9
}
NASharedFontIndex;


BOOL    NAi_InitSharedFont(void);
int     NAi_GetSharedFontTableSize(void);
BOOL    NAi_LoadSharedFontTable(void* pBuffer);
int     NAi_GetSharedFontSize(NASharedFontIndex index);
BOOL    NAi_LoadSharedFont(NASharedFontIndex index, void* pBuffer);

static inline BOOL NA_InitSharedFont(void)
    { return OS_IsRunOnTwl() ? NAi_InitSharedFont(): FALSE; }
static inline int  NA_GetSharedFontTableSize(void)
    { return OS_IsRunOnTwl() ? NAi_GetSharedFontTableSize(): -1; }
static inline BOOL NA_LoadSharedFontTable(void* pBuffer)
    { return OS_IsRunOnTwl() ? NAi_LoadSharedFontTable(pBuffer): FALSE; }
static inline int  NA_GetSharedFontSize(NASharedFontIndex index)
    { return OS_IsRunOnTwl() ? NAi_GetSharedFontSize(index): -1; }
static inline BOOL NA_LoadSharedFont(NASharedFontIndex index, void* pBuffer)
    { return OS_IsRunOnTwl() ? NAi_LoadSharedFont(index, pBuffer): FALSE; }

// The following functions are for internal use only
const u8*   NAi_GetSharedFontName(NASharedFontIndex index);
u32         NAi_GetSharedFontTimestamp(void);


/*===========================================================================*/

#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* TWL_NA_ARM9_SHARED_FONT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
