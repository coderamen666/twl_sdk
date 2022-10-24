/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - multiboot
  File:     dispfunc.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef _DISPFUNC_H_
#define _DISPFUNC_H_

#include <nitro.h>

/* Palette color for text */
enum
{
    WHITE,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    PURPLE,
    LIGHT_GREEN,
    HI_YELLOW,
    HI_CYAN,
    HI_WHITE,
    HI_BLUE,
    HI_PURPLE,
    HI_LIGHT_GREEN,
    BROWN,
    HI_BROWN,
    COLOR_NUM
};

/* Constant array for each type of rendering */
extern const u32 sampleCharData[8 * 0xe0];
extern const u16 samplePlttData[16][16];

/*
	=================================================================================

	OBJ character drawing

	=================================================================================
*/

// --- global value
extern GXOamAttr oamBak[128];

//      Set OBJ
void    ObjSet(int objNo, int x, int y, int charNo, int paletteNo);

//      Set OBJ character string
void    ObjSetString(int startobjNo, int x, int y, char *string, int paletteNo);

//      Clears OBJ
void    ObjClear(int objNo);

//      Clears OBJ (specified range)
void    ObjClearRange(int start, int end);

//      Set message
void    ObjSetMessage(int objNo, char *message);

/*
	=================================================================================

	BG character drawing (BG0 fixed)

	=================================================================================
*/

// --- Definition

// --- Initialize
void    BgInitForPrintStr(void);

// --- For main LCD
void    BgPrintf(s16 x, s16 y, u8 palette, char *text, ...);
void    BgPutString(s16 x, s16 y, u8 palette, char *text);
void    BgPutStringN(s16 x, s16 y, u8 palette, char *text, u32 num);
void    BgPutChar(s16 x, s16 y, u8 palette, char c);
void    BgClear(void);

// --- For sub LCD
void    BgPrintfSub(s16 x, s16 y, u8 palette, char *text, ...);
void    BgPutStringSub(s16 x, s16 y, u8 palette, char *text);
void    BgPutStringNSub(s16 x, s16 y, u8 palette, char *text, u32 num);
void    BgPutCharSub(s16 x, s16 y, u8 palette, char c);
void    BgClearSub(void);

// --- V-blank process
void    BgScrSetVblank(void);

// --- Message print
void    BgSetMessage(u8 palette, char *message, ...);
void    BgSetMessageSub(u8 palette, char *message, ...);
void    BgSetMessageLogSub(u8 palette, char *text);

// --- Get screen buffer
u16    *BgGetScreenBuf(void);
u16    *BgGetScreenBufSub(void);

#endif /*       _DISPFUNC_H_    */
