/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - DoubleBuffering
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
//      Displays character strings with double buffering ON or OFF.
//      It always takes time to render a character string, so the display will flicker unless double buffering is used.
//      
//
//  Using the Demo
//      A: Switches between double buffering ON/OFF.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include "g2d_textdemolib.h"



#define CANVAS_WIDTH        30      // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       22      // Height of character rendering area (in character units)
#define CANVAS_LEFT         1       // X position of character drawing area (in characters)
#define CANVAS_TOP          1       // Y position of character rendering area (in character units)

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Starting address for the string to use


//------------------------------------------------------------------------------
// Global Variables

// For onscreen display
NNSG2dCharCanvas        gOnCanvas;
NNSG2dTextCanvas        gOnTextCanvas;

// For offscreen display
#define TRANS_TASK_NUM  1
NNSG2dCharCanvas        gOffCanvas;
NNSG2dTextCanvas        gOffTextCanvas;

  // offscreen buffer
  //   Character array of the same size as CharCanvas
GXCharFmt16             gOffBuffer[CANVAS_HEIGHT][CANVAS_WIDTH];
NNSGfdVramTransferTask  gTrasTask[TRANS_TASK_NUM];

// Shared
NNSG2dFont              gFont;




//****************************************************************************
// Initialize etc.
//****************************************************************************


/*---------------------------------------------------------------------------*
  Name:         InitCanvasShare

  Description:  Initializes the on/off shared resources.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitCanvasShare(void)
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

    // load the font
    TXT_LoadFont(&gFont, TXT_FONTRESOURCE_NAME);

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
  Name:         InitOnScreenCanvas

  Description:  Initializes the canvas for onscreen display.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
// Initialization during onscreen display
static void InitOnScreenCanvas(void)
{
    GXCharFmt16* const charBase = (GXCharFmt16*)G2_GetBG1CharPtr();

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gOnCanvas,
        charBase + CHARACTER_OFFSET,    // VRAM address
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // TextCanvas initialization
    NNS_G2dTextCanvasInit(
        &gOnTextCanvas,
        &gOnCanvas,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );
}



/*---------------------------------------------------------------------------*
  Name:         InitOffScreenCanvas

  Description:  Initializes the canvas for offscreen display.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
// Initialization for offscreen display
static void InitOffScreenCanvas(void)
{
    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gOffCanvas,
        gOffBuffer,             // Pass to the offscreen buffer rather than the VRAM address
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // TextCanvas initialization
    NNS_G2dTextCanvasInit(
        &gOffTextCanvas,
        &gOffCanvas,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );

    // Uses the VRAM transmission manager in transmitting the offscreen buffer
    NNS_GfdInitVramTransferManager(gTrasTask, TRANS_TASK_NUM);
}



/*---------------------------------------------------------------------------*
  Name:         SampleInit

  Description:  Initializes the sample render.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleInit(void)
{
    // Initialize shared resources.
    InitCanvasShare();

    // Initialization for onscreen display
    InitOnScreenCanvas();

    // Initialization for offscreen display
    InitOffScreenCanvas();
}



/*---------------------------------------------------------------------------*
  Name:         SampleDraw

  Description:  Renders the sample.

  Arguments:    pTextCanvas:    pointer to the TextCanvas

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleDraw(NNSG2dTextCanvas *pTextCanvas)
{
    static int y = 0;

    // Clear the canvas
    NNS_G2dCharCanvasClear(NNS_G2dTextCanvasGetCharCanvas(pTextCanvas), TXT_COLOR_NULL);

    // Render
    NNS_G2dTextCanvasDrawText(
        pTextCanvas,
        0, y,
        TXT_COLOR_BLACK,
        TXT_DRAWTEXT_FLAG_DEFAULT,
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyz\n"
    );

    // A little lower.
    y ++;
    if( y > 100 )
    {
        y = 0;
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
    BOOL bDoubleBuffering = FALSE;

    // Initializing App.
    {
        // Initialization of SDK and demo library
        OS_Init();
        TXT_Init();

        // Configure the background
        TXT_SetupBackground();

        // Sample initialization
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

        {
            // Switches rendering on and off.
            if( CMN_IsTrigger(PAD_BUTTON_A) )
            {
                bDoubleBuffering = ! bDoubleBuffering;
            }
            DTX_PrintLine(
                "Double Buffering Sample\n"
                "operation\n"
                "  A    on/off double buffering\n"
            );
            DTX_PrintLine(bDoubleBuffering ? "DoubleBuffering: ON": "DoubleBuffering: OFF");
        }

    //----------------------------------
    // During the LCD display period

        if( bDoubleBuffering )
        {
            // Rendering is always possible with offscreen display as long as there is no transmission taking place.
            SampleDraw( &gOffTextCanvas );

            // Register the transmission task during the VBlank interval.
            (void)NNS_GfdRegisterNewVramTransferTask(
                NNS_GFD_DST_2D_BG1_CHAR_MAIN,
                sizeof(GXCharFmt16) * CHARACTER_OFFSET,
                gOffBuffer,
                sizeof(gOffBuffer)
            );
        }
        else
        {
            // Nothing is done with onscreen display
        }

    //
    //----------------------------------

        CMN_WaitVBlankIntr();
        DTX_Reflect();

    //----------------------------------
    // During the VBlank period

        if( bDoubleBuffering )
        {
            // The rendered image is transmitted to VRAM during the VBlank with offscreen display.
            NNS_GfdDoVramTransfer();
        }
        else
        {
            // It is rendered in VRAM during a VBlank with onscreen display.
            SampleDraw( &gOnTextCanvas );
        }

    //
    //----------------------------------
    }
}

