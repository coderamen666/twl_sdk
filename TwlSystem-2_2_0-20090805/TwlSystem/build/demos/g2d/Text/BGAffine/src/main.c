/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - BGAffine
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
//      Character string display sample on the affine BG (256-color mode)
//      An affine background can only use 256 characters. This sample is not able to create a CharCanvas large enough to display all the characters.
//      
//      
//
//  Using the Demo
//      No operations.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include "g2d_textdemolib.h"


// Be aware that only 256 characters can be used with the affine BG
// This means that the sum of CANVAS*_WIDTH * CANVAS*_HEIGHT must be 256 or lower.

#define CANVAS0_WIDTH       10      // Width of character rendering area (in character units)
#define CANVAS0_HEIGHT      5       // Height of character rendering area (in character units)
#define CANVAS0_LEFT        1       // X position of character drawing area (in characters)
#define CANVAS0_TOP         1       // Y position of character rendering area (in character units)

#define CANVAS1_WIDTH       20      // Width of character rendering area (in character units)
#define CANVAS1_HEIGHT      6       // Height of character rendering area (in character units)
#define CANVAS1_LEFT        3       // X position of character drawing area (in characters)
#define CANVAS1_TOP         15      // Y position of character rendering area (in character units)

#define CANVAS2_WIDTH       10      // Width of character rendering area (in character units)
#define CANVAS2_HEIGHT      6      // Height of character rendering area (in character units)
#define CANVAS2_LEFT        15      // X position of character drawing area (in characters)
#define CANVAS2_TOP         3       // Y position of character rendering area (in character units)

#define TEXT_HSPACE0        4       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE0        4       // Line spacing when rendering strings (in pixels)

#define TEXT_HSPACE1        1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE1        1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Starting address for the string to use


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont           gFont;             // Font
NNSG2dCharCanvas     gCanvas[3];        // CharCanvas x3
NNSG2dTextCanvas     gTextCanvas[3];    // TextCanvas x3



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
    // Configure BG 2
    G2_SetBG2ControlAffine(
        GX_BG_SCRSIZE_AFFINE_256x256,   // screen size: 256x256
        GX_BG_AREAOVER_XLU,             // External region: transparent
        GX_BG_SCRBASE_0x0000,           // screen base
        GX_BG_CHARBASE_0x00000          // character base
    );

    // Make BG2 visible
    CMN_SetPlaneVisible( GX_PLANEMASK_BG2 );

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
    // Pointer to BG character base
    GXCharFmt256* pCharBase = (GXCharFmt256*)G2_GetBG2CharPtr();
    // Pointer to BG screen base
    GXScrAffine32x32* pScr = (GXScrAffine32x32*)G2_GetBG1ScrPtr();
    int cOffset = CHARACTER_OFFSET;

    // Loads the font
    TXT_LoadFont( &gFont, TXT_FONTRESOURCE_NAME );

    // CharCanvas initialization and BG screen configuration
    // CharCanvas 0
    NNS_G2dCharCanvasInitForBG(
        &gCanvas[0],                    // Pointer to CharCanvas
        pCharBase + cOffset,            // Pointer to the top of the character string to be used
        CANVAS0_WIDTH,                  // CharCanvas width
        CANVAS0_HEIGHT,                 // CharCanvas height
        NNS_G2D_CHARA_COLORMODE_256     // color mode
    );
    NNS_G2dMapScrToCharAffine(
        &( pScr->scr[CANVAS0_TOP][CANVAS0_LEFT] ),  // Pointer to the screen of the CharCanvas display position
        CANVAS0_WIDTH,                              // CharCanvas width
        CANVAS0_HEIGHT,                             // CharCanvas height
        NNS_G2D_AFFINE_BG_WIDTH_256,                // Screen width
        cOffset                                     // Character number of the top of the character string to be used
    );

    // Add the number of characters that CharCanvas 0 uses to the offset
    cOffset += CANVAS0_WIDTH * CANVAS0_HEIGHT;

    NNS_G2dCharCanvasInitForBG(
        &gCanvas[1],
        pCharBase + cOffset,
        CANVAS1_WIDTH,
        CANVAS1_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_256
    );
    NNS_G2dMapScrToCharAffine(
        &( pScr->scr[CANVAS1_TOP][CANVAS1_LEFT] ),
        CANVAS1_WIDTH,
        CANVAS1_HEIGHT,
        NNS_G2D_AFFINE_BG_WIDTH_256,
        cOffset
    );

    // Add the number of characters that CharCanvas 1 uses to the offset
    cOffset += CANVAS1_WIDTH * CANVAS1_HEIGHT;

    NNS_G2dCharCanvasInitForBG(
        &gCanvas[2],
        pCharBase + cOffset,
        CANVAS2_WIDTH,
        CANVAS2_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_256
    );
    NNS_G2dMapScrToCharAffine(
        &( pScr->scr[CANVAS2_TOP][CANVAS2_LEFT] ),
        CANVAS2_WIDTH,
        CANVAS2_HEIGHT,
        NNS_G2D_AFFINE_BG_WIDTH_256,
        cOffset
    );

    // TextCanvas initialization
    // A TextCanvas can be prepared for each CharCanvas, and multiple TextCanvas instances can share one CharCanvas as well.
    // 
    // When using one TextCanvas for multiple CharCanvas instances, use it by replacing its CharCanvas as you go.
    // 

    // 1) Share a single CharCanvas over more than one TextCanvas
    // CharCanvas 1 is shared between TextCanvas 0 and 1
    {
        NNS_G2dTextCanvasInit(
            &gTextCanvas[0],        // Pointer to the TextCanvas
            &gCanvas[1],            // Pointer to the render destination CharCanvas
            &gFont,                 // Pointer to the font used in rendering
            TEXT_HSPACE0,           // Space between characters
            TEXT_VSPACE0            // Space between lines
        );
        NNS_G2dTextCanvasInit(
            &gTextCanvas[1],
            &gCanvas[1],
            &gFont,
            TEXT_HSPACE1,
            TEXT_VSPACE1
        );
    }

    // 2) Prepare a TextCanvas for each CharCanvas
    // CharCanvas 2 is used exclusively by TextCanvas 2
    {
        NNS_G2dTextCanvasInit(
            &gTextCanvas[2],
            &gCanvas[2],
            &gFont,
            TEXT_HSPACE0,
            TEXT_VSPACE0
        );
    }
}



/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  This is the main process of the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleMain(void)
{
    InitScreen();
    InitCanvas();

    // Completely fill in each CharCanvas with TXT_COLOR_WHITE
    NNS_G2dCharCanvasClear(&gCanvas[0], TXT_COLOR_WHITE);
    NNS_G2dCharCanvasClear(&gCanvas[1], TXT_COLOR_WHITE);
    NNS_G2dCharCanvasClear(&gCanvas[2], TXT_COLOR_WHITE);

    // Render character string
    // Render TextCanvas 0 (->CharCanvas 1)
    NNS_G2dTextCanvasDrawText(&gTextCanvas[0], 0, 0, TXT_COLOR_CYAN, TXT_DRAWTEXT_FLAG_DEFAULT,
        "TextCanvas0 CharCanvas1\n"
        "abcdefg"
    );

    // Render TextCanvas 1 (->CharCanvas 1)
    NNS_G2dTextCanvasDrawText(&gTextCanvas[1], 0, 30, TXT_COLOR_MAGENTA, TXT_DRAWTEXT_FLAG_DEFAULT,
        "TextCanvas1 CharCanvas1\n"
        "hijklmn"
    );

    // Render TextCanvas 2 (->CharCanvas 2)
    NNS_G2dTextCanvasDrawText(&gTextCanvas[2], 0, 0, TXT_COLOR_GREEN, TXT_DRAWTEXT_FLAG_DEFAULT,
        "TextCanvas2\n"
        "CharCanvas2\n"
        "opqrstu"
    );


    // 3) Use a single TextCanvas over more than one CharCanvas
    {
        // Swap the CharCanvas of TextCanvas 1
        NNS_G2dTextCanvasSetCharCanvas(&gTextCanvas[1], &gCanvas[0]);

        // Render TextCanvas 1 (->CharCanvas 0)
        NNS_G2dTextCanvasDrawText(&gTextCanvas[1], 0, 0, TXT_COLOR_BLUE, TXT_DRAWTEXT_FLAG_DEFAULT,
            "TextCanvas1\n"
            "CharCanvas0\n"
            "vwxyz"
        );
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

