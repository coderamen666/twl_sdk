/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - PortraitSW
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
//      Screen rendering for vertical screen.
//      This demo renders a vertical screen using a converted font.
//
//  Using the Demo
//      Switches the font and its display position, taking the direction pressed on the +Control Pad as the top of the screen.
//      
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include "g2d_textdemolib.h"


#define CHARACTER_WIDTH     8
#define CHARACTER_HEIGHT    8

#define CANVAS_WIDTH        28      // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       20      // Height of character rendering area (in character units)
#define CANVAS_LEFT         2       // X position of character drawing area (in characters)
#define CANVAS_TOP          2       // Y position of character rendering area (in character units)

#define BG_WIDTH            32      // Width of the BG for drawing strings   (character units)
#define BG_HEIGHT           32      // Height of the BG for drawing strings (character units)

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Starting address for the string to use


//------------------------------------------------------------------------------
// Types

typedef enum Direction
{
    DIR_H_0,    // Horizontal writing, from the left of the screen to the right
    DIR_H_90,   // Horizontal writing, from the top of the screen to the bottom
    DIR_H_180,  // Horizontal writing, from the right of the screen to the left
    DIR_H_270,  // Horizontal writing, from the bottom of the screen to the top
    DIR_NULL
}
Direction;

typedef struct FontInfo
{
    const char* name;
    Direction   dir;
    NNSG2dFont  res;
}
FontInfo;


//------------------------------------------------------------------------------
// Global Variables

NNSG2dCharCanvas        gCanvas;        // CharCanvas
NNSG2dTextCanvas        gTextCanvas;    // TextCanvas

FontInfo gFonts[] =
{
    {"/data/fontu8.NFTR",     DIR_H_0 },
    {"/data/fontu8_90.NFTR",  DIR_H_90 },
    {"/data/fontu8_180.NFTR", DIR_H_180 },
    {"/data/fontu8_270.NFTR", DIR_H_270 },
};

// the text to display
const char* const sSampleText = "abcdefghijklmnopqrstuvwxyz\n"
                                "The quick brown fox jumps over\n"
                                "the lazy dog\n";



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
    GXCharFmt16*  pChrBase = (GXCharFmt16*) G2_GetBG1CharPtr();
    GXScrFmtText* pScrBase = (GXScrFmtText*)G2_GetBG1ScrPtr();
    int cOffset = CHARACTER_OFFSET;

    // Clears the screen
    MI_CpuClear16(G2_GetBG1ScrPtr(), sizeof(GXScrFmtText) * BG_WIDTH * BG_HEIGHT);

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        pChrBase + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // TextCanvas initialization
    NNS_G2dTextCanvasInit(
        &gTextCanvas,
        &gCanvas,
        &gFonts[0].res,
        TEXT_HSPACE,
        TEXT_VSPACE
    );

    // Set the screen
    NNS_G2dMapScrToCharText(
        pScrBase,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        cOffset,
        TXT_CPALETTE_MAIN
    );
}



/*---------------------------------------------------------------------------*
  Name:         SampleDraw

  Description:  Draws a sample character string.

  Arguments:    x:      The x-position at which to draw the string.
                y:      The y-position at which to draw the string.
                pFont:  Font used for drawing.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleDraw(int x, int y, const NNSG2dFont* pFont)
{
    // Clear background
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

    // Change font
    NNS_G2dTextCanvasSetFont(&gTextCanvas, pFont);

    // Draw text
    NNS_G2dTextCanvasDrawText(&gTextCanvas, x, y, TXT_COLOR_BLACK,
            TXT_DRAWTEXT_FLAG_DEFAULT, sSampleText);
}



/*---------------------------------------------------------------------------*
  Name:         SampleInit

  Description:  Processing for the initial settings of the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleInit(void)
{
    int i;

    // Load fonts
    for( i = 0; i < ARY_SIZEOF(gFonts); ++i )
    {
        TXT_LoadFont( &gFonts[i].res, gFonts[i].name );
    }


    // screen configuration
    InitScreen();

    // Build the canvas
    InitCanvas();
}



/*---------------------------------------------------------------------------*
  Name:         SearchFont

  Description:  Search for the font for use with the specified direction

  Arguments:    pfis:   Array of font data
                dir:    Target direction

  Returns:      Returns the font that was found.
 *---------------------------------------------------------------------------*/
static const NNSG2dFont* SearchFont(const FontInfo* pfis, Direction dir)
{
    int i;

    for( i = 0; i < ARY_SIZEOF(gFonts); ++i )
    {
        if( pfis[i].dir == dir )
        {
            return &pfis[i].res;
        }
    }

    return NULL;
}



/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  This is the main process of the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleMain(void)
{
    static Direction rotation = DIR_NULL;
    BOOL bUpdated = FALSE;

    //---- Change the display direction based on input.
    if( CMN_IsTrigger(PAD_KEY_UP) || rotation == DIR_NULL )
    {
        rotation = DIR_H_0;
        bUpdated = TRUE;
    }
    if( CMN_IsTrigger(PAD_KEY_DOWN) )
    {
        rotation = DIR_H_180;
        bUpdated = TRUE;
    }
    if( CMN_IsTrigger(PAD_KEY_LEFT) )
    {
        rotation = DIR_H_270;
        bUpdated = TRUE;
    }
    if( CMN_IsTrigger(PAD_KEY_RIGHT) )
    {
        rotation = DIR_H_90;
        bUpdated = TRUE;
    }

    //---- Applies the change if the direction was changed.
    if( bUpdated )
    {
        static const int canvas_width  = CANVAS_WIDTH  * CHARACTER_WIDTH;
        static const int canvas_height = CANVAS_HEIGHT * CHARACTER_HEIGHT;
        const NNSG2dFont* pFont = SearchFont(gFonts, rotation);
        int x = 0;
        int y = 0;

        NNS_G2D_POINTER_ASSERT( pFont );

        //---- Adjusts the starting point depending on the direction.
        switch( rotation )
        {
        case DIR_H_270: y = canvas_height;  break;
        case DIR_H_90:  x = canvas_width;  break;
        case DIR_H_180: x = canvas_width;
                        y = canvas_height;  break;
        }

        //---- Draws a sample character string using the fonts divided by direction.
        SampleDraw(x, y, pFont);


        //---- Display message
        DTX_PrintLine("PortraitSW demo");
        DTX_PrintLine("  operation:");
        DTX_PrintLine("    +: change  direction");
        DTX_PrintLine("");
        DTX_PrintLine("direction:");
        switch( rotation )
        {
        case DIR_H_0:   DTX_PrintLine("  left to right"); break;
        case DIR_H_90:  DTX_PrintLine("  top to bottom"); break;
        case DIR_H_180: DTX_PrintLine("  right to left"); break;
        case DIR_H_270: DTX_PrintLine("  bottom to up");  break;
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

        // preprocessing for demo
        SampleInit();
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
        SampleMain();

        // Display the information output
        DTX_Reflect();
    }
}

