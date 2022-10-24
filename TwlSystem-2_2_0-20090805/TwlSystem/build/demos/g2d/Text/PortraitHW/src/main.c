/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - PortraitHW
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
//      The font converted for the horizontal screen is used and rendered, and the background is rotated to be displayed for the vertical screen.
//      
//
//  Using the Demo
//      Rotates BG so that the direction in which the +Control Pad is pushed will become the top of the screen.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include "g2d_textdemolib.h"


#define CHARACTER_WIDTH     8
#define CHARACTER_HEIGHT    8

#define CANVAS_WIDTH        21      // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       13      // Height of character rendering area (in character units)
#define CANVAS_LEFT         2       // X position of character drawing area (in characters)
#define CANVAS_TOP          7       // Y position of character rendering area (in character units)

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


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont              gFont;          // Font
NNSG2dCharCanvas        gCanvas;        // CharCanvas
NNSG2dTextCanvas        gTextCanvas;    // TextCanvas


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
    // Configure BG 3
    // Background rotation can only be used when the affine background is a 256x16 palette background.
    // 
    G2_SetBG3Control256x16Pltt(
        GX_BG_SCRSIZE_256x16PLTT_256x256,   // screen size: 256x256
        GX_BG_AREAOVER_XLU,                 // Area over processing
        GX_BG_SCRBASE_0x0000,               // screen base
        GX_BG_CHARBASE_0x00000              // character base
    );

    // Make BG 3 visible
    CMN_SetPlaneVisible( GX_PLANEMASK_BG3 );

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
    GXCharFmt256* pChrBase = (GXCharFmt256*)G2_GetBG3CharPtr();
    GXScrFmtText* pScrBase = (GXScrFmtText*)G2_GetBG3ScrPtr();
    int cOffset = CHARACTER_OFFSET;

    // Clears the screen
    MI_CpuClear16(G2_GetBG3ScrPtr(), sizeof(GXScrFmtText) * BG_WIDTH * BG_HEIGHT);

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        pChrBase + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_256
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
    NNS_G2dMapScrToChar256x16Pltt(
        pScrBase + CANVAS_TOP * BG_WIDTH
                 + CANVAS_LEFT,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_256x16PLTT_BG_WIDTH_256,
        cOffset,
        0           // specify number zero, 256 color palette
    );
}



/*---------------------------------------------------------------------------*
  Name:         SampleDraw

  Description:  Draws a sample character string.

  Arguments:    x:      The x-position at which to draw the string.
                y:      The y-position at which to draw the string.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleDraw(int x, int y)
{
    // Clear background
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

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
    // Load fonts
    TXT_LoadFont( &gFont, TXT_FONTRESOURCE_NAME );

    // screen configuration
    InitScreen();

    // Build the canvas
    InitCanvas();

    // Draw text
    SampleDraw(0, 0);
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
        int centerX, centerY;
        MtxFx22 m;

        switch( rotation )
        {
        case DIR_H_0:
            {
                MTX_Rot22(&m, FX32_SIN0,   FX32_COS0);
                centerX      = 0;
                centerY      = 0;
            }
            break;

        case DIR_H_90:
            {
                MTX_Rot22(&m, FX32_SIN90, FX32_COS90);
                centerX      = GX_LCD_SIZE_X / 2;
                centerY      = GX_LCD_SIZE_X / 2;
            }
            break;

        case DIR_H_180:
            {
                MTX_Rot22(&m, FX32_SIN180, FX32_COS180);
                centerX      = GX_LCD_SIZE_X / 2;
                centerY      = GX_LCD_SIZE_Y / 2;
            }
            break;

        case DIR_H_270:
            {
                MTX_Rot22(&m, FX32_SIN270, FX32_COS270);
                centerX      = GX_LCD_SIZE_Y / 2;
                centerY      = GX_LCD_SIZE_Y / 2;
            }
            break;

        default:
            return;
        }

        // Set BG rotation
        G2_SetBG3Affine(&m, centerX, centerY, 0, 0);


        //---- Display message
        DTX_PrintLine("PortraitHW demo");
        DTX_PrintLine("  operation:");
        DTX_PrintLine("    +: change direction");
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

