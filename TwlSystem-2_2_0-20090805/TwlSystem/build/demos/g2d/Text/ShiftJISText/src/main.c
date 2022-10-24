/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - ShiftJISText
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

// ============================================================================
//  Explanation of the demo:
//      This is a sample of the ShiftJIS character string display method.
//
//  Using the Demo
//      None.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include "g2d_textdemolib.h"


#define CANVAS_WIDTH        22      // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       16      // Height of character rendering area (in character units)
#define CANVAS_LEFT         5       // X position of character drawing area (in characters)
#define CANVAS_TOP          4       // Y position of character rendering area (in character units)

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Starting address for the string to use


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont              gFont;          // Font
NNSG2dCharCanvas        gCanvas;        // CharCanvas
NNSG2dTextCanvas        gTextCanvas;    // TextCanvas



//****************************************************************************
// Initialize etc.
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         InitScreen

  Description:  Configures the BG screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitScreen(void)
{
    // Configure BG 1
    G2_SetBG1Control(
        GX_BG_SCRSIZE_TEXT_256x256,     // screen size: 256x256
        GX_BG_COLORMODE_16,             // 16-color color mode
        GX_BG_SCRBASE_0x0000,           // screen base
        GX_BG_CHARBASE_0x00000,         // character base
        GX_BG_EXTPLTT_01                // extended palette slot
    );

    // Make BG 1 visible
    CMN_SetPlaneVisible( GX_PLANEMASK_BG1 );

    // Configure color palette
    GX_LoadBGPltt(TXTColorPalette, 0, sizeof(TXTColorPalette));
}



/*---------------------------------------------------------------------------*
  Name:         InitCanvas

  Description:  Initializes the character string render.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitCanvas(void)
{
    GXCharFmt16* pCharBase = (GXCharFmt16*)G2_GetBG1CharPtr();
    int cOffset = CHARACTER_OFFSET;

    // Loads the font
    {
//      In the same manner as the other demos, the font can be loaded even if TXT_LoadFont is used.
//      This demo shows the use of NNS_G2dFontInitShiftJIS.
//        TXT_LoadFont( &gFont, TXT_SJIS_FONTRESOURCE_NAME );

        void* pFontFile;
        u32 size;

        size = TXT_LoadFile( &pFontFile, TXT_SJIS_FONTRESOURCE_NAME );
        NNS_G2D_ASSERT( size > 0 );

        NNS_G2dFontInitShiftJIS(&gFont, pFontFile);
        NNS_G2dPrintFont(&gFont);
    }

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        pCharBase + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // TextCanvas initialization
    NNS_G2dTextCanvasInit(
        &gTextCanvas,
        &gCanvas,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );

    // Set the screen
    NNS_G2dMapScrToCharText(
        G2_GetBG1ScrPtr(),
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        CHARACTER_OFFSET,
        TXT_CPALETTE_MAIN
    );
}



/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  Main processing of sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleMain(void)
{
    InitScreen();
    InitCanvas();

    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);
    NNS_G2dTextCanvasDrawText(&gTextCanvas, 0, 0,
        TXT_COLOR_BLACK, TXT_DRAWTEXT_FLAG_DEFAULT,
        "Shift_JIS\n"
        "0123456789\n"                                              // <- Numerals
        "abcdefg ABCDEFG\n"                                         // <- Alphabet
        "\x85\x75\x85\x76\x85\x77\x85\x78\x85\x79\x85\x7A\n"        // <- Western European characters JIS X 0213
        "\x82\xA0\x82\xA2\x82\xA4\x82\xA6\x82\xA8\x8A\xBF\x8E\x9A"  // <- Japanese characters
    );
}




//****************************************************************************
// Main
//****************************************************************************


/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Initializing App.
    {
        // Initialization of SDK and demo library
        OS_Init();
        TXT_Init();

        // Configure the background
        TXT_SetupBackground();

        // Main process of sample
        SampleMain();
    }

    // start display
    {
        CMN_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    // Main loop
    while( TRUE )
    {
        CMN_ReadGamePad();

        CMN_WaitVBlankIntr();

        // Display the information output
        DTX_Reflect();
    }
}

