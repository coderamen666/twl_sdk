/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - fake_child
  File:     disp.c

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

//----------------------------------------------------------------------
// Basic string display functionality
//----------------------------------------------------------------------

#ifdef SDK_TWL
#include	<twl.h>
#else
#include	<nitro.h>
#endif
#include "disp.h"


//============================================================================
//  Prototype Declarations
//============================================================================
static void VramClear(void);
static void ObjInitForPrintStr(void);
static void BgInitForPrintStr(void);


//============================================================================
//  Constant Definitions
//============================================================================

#define BGSTR_MAX_LENGTH        32

//============================================================================
//  Variable Definitions
//============================================================================

/* Virtual screen */
static u16 vscr[32 * 32];

/* Temporary OAM for V-Blank transfer */
static GXOamAttr oamBak[128];


/* For various rendering usages */
extern const u32 sampleFontCharData[8 * 0xe0];
extern const u16 samplePlttData[16][16];


//============================================================================
//  Function Definitions
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         BgInitForPrintStr

  Description:  Initializes BG character drawing (Fixed to BG0, VRAM-C, BG mode 0).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void BgInitForPrintStr(void)
{
    GX_SetBankForBG(GX_VRAM_BG_128_C);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf800,      /* SCR base block 31 */
                     GX_BG_CHARBASE_0x00000,    /* CHR base block 0 */
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    G2_SetBG0Offset(0, 0);

    GX_LoadBG0Char(sampleFontCharData, 0, sizeof(sampleFontCharData));
    GX_LoadBGPltt(samplePlttData, 0, sizeof(samplePlttData));

    MI_CpuFillFast((void *)vscr, 0, sizeof(vscr));
    DC_FlushRange(vscr, sizeof(vscr));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(vscr, 0, sizeof(vscr));
}

/*---------------------------------------------------------------------------*
  Name:         ObjInitForPrintStr

  Description:  OBJ initialization (VRAM-B, 2D mode).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ObjInitForPrintStr(void)
{
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_B);
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);
    MI_DmaFill32(3, oamBak, 0xc0, sizeof(oamBak));

    GX_LoadOBJ(sampleFontCharData, 0, sizeof(sampleFontCharData));
    GX_LoadOBJPltt(samplePlttData, 0, sizeof(samplePlttData));

}

/*---------------------------------------------------------------------------*
  Name:         VramClear

  Description:  Clear VRAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VramClear(void)
{
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

}

/*---------------------------------------------------------------------------*
  Name:         DispInit

  Description:  Initializes drawing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DispInit(void)
{
    /* Initialize drawing setting */
    VramClear();
    // Initialize OBJs
    ObjInitForPrintStr();
    // Initialize BG
    BgInitForPrintStr();

    GX_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_OBJ);

}


/*---------------------------------------------------------------------------*
  Name:         DispOn

  Description:  Displays screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DispOn(void)
{
    /* Start display */
    GX_DispOn();
    GXS_DispOn();
}

/*---------------------------------------------------------------------------*
  Name:         DispOff

  Description:  Hides screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DispOff(void)
{
    /* Start display */
    GX_DispOff();
    GXS_DispOff();
}




/*---------------------------------------------------------------------------*
  Name:         DispVBlankFunc

  Description:  Drawing V-Blank processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DispVBlankFunc(void)
{
    //---- OAM updating
    DC_FlushRange(oamBak, sizeof(oamBak));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    MI_DmaCopy32(3, oamBak, (void *)HW_OAM, sizeof(oamBak));

    //---- BG screen update
    DC_FlushRange(vscr, sizeof(vscr));
    /* I/O register is accessed using DMA operation, so cache wait is not needed */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(vscr, 0, sizeof(vscr));
}




/*---------------------------------------------------------------------------*
  Name:         ObjSet

  Description:  Configures OBJ drawing settings.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ObjSet(s32 objNo, s32 x, s32 y, s32 charNo, s32 paletteNo)
{
    G2_SetOBJAttr((GXOamAttr *)&oamBak[objNo],
                  x,
                  y,
                  0,
                  GX_OAM_MODE_NORMAL,
                  FALSE,
                  GX_OAM_EFFECT_NONE, GX_OAM_SHAPE_8x8, GX_OAM_COLOR_16, charNo, paletteNo, 0);
}

/*---------------------------------------------------------------------------*
  Name:         ObjSetString

  Description:  Draw characters with OBJ.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ObjSetString(s32 startobjNo, s32 x, s32 y, const char *string, s32 paletteNo)
{
    s32     i;
    for (i = 0; (string[i] != '\0') && (i < 24); i++)
    {
        ObjSet(startobjNo + i, x + i * 8, y, (s32)string[i], paletteNo);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ObjClear

  Description:  Clears OBJ.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ObjClear(s32 objNo)
{
    ObjSet(objNo, 256 * 8, 192 * 8, 0, 0);
}

/*---------------------------------------------------------------------------*
  Name:         ObjClearRange

  Description:  Clears OBJ (specified range).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ObjClearRange(s32 start, s32 end)
{
    s32     i;
    for (i = start; i <= end; i++)
    {
        ObjClear(i);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ObjSetMessage

  Description:  Sets message (Displays 24 characters in the center bottom of screen).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ObjSetMessage(const char *message)
{
    ObjSetString(MESSAGE_OBJNO, 4 * 8, 22 * 8, message, 7);
}


/*---------------------------------------------------------------------------*
  Name:         BgPutStringN

  Description:  Outputs N characters in BG.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgPutStringN(s32 x, s32 y, s32 palette, const char *text, s32 num)
{
    s32     i;
    if (num > BGSTR_MAX_LENGTH)
    {
        num = BGSTR_MAX_LENGTH;
    }

    for (i = 0; i < num; i++)
    {
        if (text[i] == '\0')
        {
            break;
        }
        BgPutChar(x + i, y, palette, text[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         BgPutChar

  Description:  Outputs 1 BG character.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgPutChar(s32 x, s32 y, s32 palette, s8 c)
{
    vscr[((y * 32) + x) % (32 * 32)] = (u16)((palette << 12) | c);
}

/*---------------------------------------------------------------------------*
  Name:         BgPutString

  Description:  Outputs 32 BG characters.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgPutString(s32 x, s32 y, s32 palette, const char *text)
{
    BgPutStringN(x, y, palette, text, BGSTR_MAX_LENGTH);
}

/*---------------------------------------------------------------------------*
  Name:         BgPrintStr

  Description:  BG formatted output.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgPrintStr(s32 x, s32 y, s32 palette, const char *text, ...)
{
    char    temp[(BGSTR_MAX_LENGTH + 1) * 2];
    va_list vlist;

    MI_CpuClear8(temp, sizeof(temp));
    va_start(vlist, text);
    (void)vsnprintf(temp, sizeof(temp) - 1, text, vlist);
    va_end(vlist);
    BgPutString(x, y, palette, temp);
}

/*---------------------------------------------------------------------------*
  Name:         BgSetMessage

  Description:  Sets BG message.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgSetMessage(s32 palette, const char *text, ...)
{
    char    temp[(BGSTR_MAX_LENGTH + 1) * 2];
    va_list vlist;

    MI_CpuClear8(temp, sizeof(temp));
    va_start(vlist, text);
    (void)vsnprintf(temp, sizeof(temp) - 1, text, vlist);
    va_end(vlist);
    BgPutString(4, 22, palette, temp);
}

/*---------------------------------------------------------------------------*
  Name:         BgClear

  Description:  Clears BG.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgClear(void)
{
    MI_CpuClearFast(vscr, sizeof(vscr));
}


/*---------------------------------------------------------------------------*
  Name:         BgColorString

  Description:  Changes the color of character strings printed on the virtual screen.

  Arguments:    x: X-coordinate (x 8 dots ) from which to start color change
                y: Y-coordinate (x 8 dots ) from which to start color change
                length: Number of characters to continue the color change for
                palette: Specify text color by palette number

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BgColorString(s16 x, s16 y, s16 length, u8 palette)
{
    s32     i;
    u16     temp;
    s32     index;

    if (length < 0)
    {
        return;
    }

    for (i = 0; i < length; i++)
    {
        index = ((y * 32) + x + i) % (32 * 32);
        temp = vscr[index];
        temp &= 0x0fff;
        temp |= (palette << 12);
        vscr[index] = temp;
    }
}
