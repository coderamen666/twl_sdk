/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - DrawLetter
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
//      This is a sample of using character rendering functions.
//      It renders the characters stepwise while changing the color of each character at a time.
//
//  Using the Demo
//      No operations.
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

NNSG2dFont                  gFont;          // Font
static NNSG2dCharCanvas     gCanvas;            // CharCanvas



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
    // Unlike the other demos, this uses an easy-to-understand font that has left spaces.
    TXT_LoadFont( &gFont, DEBUG_FONTRESOURCE_NAME );

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        pCharBase + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // Set the screen
    NNS_G2dMapScrToCharText(
        G2_GetBG1ScrPtr(),
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        cOffset,
        TXT_CPALETTE_USERCOLOR
    );
}



/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  This is the main process of the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define NEXT_COLOR(c)   ((u8)(((c) + 1) & 0xF))
static void SampleMain(void)
{
    // The character string to be displayed
    const char* const str = "abcdefghijklmnopqrstuvwxyz";
    const char* pos;
    const u8 bgColor = TXT_UCOLOR_GRAY;

    int x = 0;
    int y = 0;
    u8 clr = 0;
    NNSG2dGlyph glyph;

    InitScreen();
    InitCanvas();

    // Clear background
    NNS_G2dCharCanvasClear(&gCanvas, bgColor);

    // Render with DrawChar
    for( pos = str; *pos != '\0'; pos++ )
    {
        int width;

        // The return value of NNS_G2dCharCanvasDrawChar is the character width (=left space width + glyph width + right space width).
        width = NNS_G2dCharCanvasDrawChar(&gCanvas, &gFont, x, y, clr, *pos);

        // Calculate character width
        x += width;

        x += TEXT_HSPACE;
        y += TEXT_VSPACE;

        // Next color
        clr = NEXT_COLOR(clr);
        if( clr == bgColor )
        {
            clr = NEXT_COLOR(clr);
        }
    }

    // Render with DrawGlyph
    x = 0;
    for( pos = str; *pos != '\0'; pos++ )
    {
        NNS_G2dFontGetGlyph(&glyph, &gFont, *pos);
        NNS_G2dCharCanvasDrawGlyph(&gCanvas, &gFont, x, y, clr, &glyph);

        // Add only the glyph width
        x += glyph.pWidths->glyphWidth;

        x += TEXT_HSPACE;
        y += TEXT_VSPACE;

        // Next color
        clr = NEXT_COLOR(clr);
        if( clr == bgColor )
        {
            clr = NEXT_COLOR(clr);
        }
    }
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

