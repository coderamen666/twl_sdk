/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-dataShare
  File:     disp.h

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/
#ifndef MB_DEMO_DISP_H_
#define MB_DEMO_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>

/*
 * Processing for general text rendering
 */

/* OBJ number assignment */
enum
{
    TITLE_STRING_OBJNO = 0,
    PARENT_STRING_OBJNO = 10,
    CHILD_STRING_OBJNO = 22,
    PLIST_STRING_OBJNO = 33,
    CLIST_STRING_OBJNO = 39,

    CURSOR_OBJNO = 45,
    PARENT_NAME_OBJNO = 46,
    FILENAME_OBJNO = 82,
    MESSAGE_OBJNO = 104
};

/* Palette color for text */
enum
{
    PLTT_BLACK = 0,
    PLTT_BLUE = 1,
    PLTT_RED = 2,
    PLTT_PURPLE = 3,
    PLTT_GREEN = 4,
    PLTT_CYAN = 5,
    PLTT_YELLOW = 6,
    PLTT_WHITE = 7
};



//============================================================================
//  Function Declarations
//============================================================================

void DispInit(void);
void DispOn(void);
void DispOff(void);
void DispVBlankFunc(void);
void BgPutChar(s32 x, s32 y, s32 palette, s8 c);
void BgPutStringN(s32 x, s32 y, s32 palette, const char *text, s32 num);
void BgPutString(s32 x, s32 y, s32 palette, const char *text);
void BgPrintStr(s32 x, s32 y, s32 palette, const char *text, ...);
void BgClear(void);



#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // MB_DEMO_DISP_H_
