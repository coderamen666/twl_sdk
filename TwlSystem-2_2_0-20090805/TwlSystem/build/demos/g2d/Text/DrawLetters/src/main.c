/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - DrawLetters
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
//      This is a sample that uses the render character string functions.
//      NNS_G2dTextCanvasDrawString
//      NNS_G2dTextCanvasDrawText
//      NNS_G2dTextCanvasDrawTextRect
//      This sample demonstrates the differences in rendering for the above three functions using a flag.
//
//  Using the Demo
//      References the lower screen when the demo is run
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include "g2d_textdemolib.h"

#define CHARACTER_WIDTH     8
#define CHARACTER_HEIGHT    8

#define CANVAS_WIDTH        22      // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       16      // Height of character rendering area (in character units)
#define CANVAS_LEFT         5       // X position of character drawing area (in characters)
#define CANVAS_TOP          4       // Y position of character rendering area (in character units)

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Starting address for the string to use

#define TRANS_TASK_NUM  1


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont              gFont;          // Font
NNSG2dCharCanvas        gCanvas;        // CharCanvas
NNSG2dTextCanvas        gTextCanvas;    // TextCanvas

// offscreen buffer
GXCharFmt16             gOffBuffer[CANVAS_HEIGHT][CANVAS_WIDTH];
NNSGfdVramTransferTask  gTrasTask[TRANS_TASK_NUM];

// the text to display
static const char sSampleText[] =
    "short string\n"
    "It's a long long long long string.\n"
    "\n"
    "after null line";



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
    // Loads the font
    TXT_LoadFont( &gFont, TXT_FONTRESOURCE_NAME );

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        &gOffBuffer,
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
  Name:         SampleInit

  Description:  Initializes the demo process.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleInit(void)
{
    InitScreen();
    InitCanvas();

    // Uses the VRAM transmission manager in transmitting the offscreen buffer
    NNS_GfdInitVramTransferManager(gTrasTask, TRANS_TASK_NUM);
}



/*---------------------------------------------------------------------------*
  Name:         SampleString

  Description:  NNS_G2dTextCanvasDrawString() sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleString(void)
{
    const char* str = sSampleText;
    const int linefeed = NNS_G2dFontGetLineFeed(&gFont) + NNS_G2dTextCanvasGetVSpace(&gTextCanvas);
    const int baselinePos = NNS_G2dFontGetBaselinePos(&gFont);
    int x, y;

    // information output
    {
        DTX_PrintLine("NNS_G2dTextCanvasDrawString Sample");
        DTX_PrintLine("  operation:");
        DTX_PrintLine("    X:     next mode");
        DTX_PrintLine("");
    }

    // Render position initial value
    x = 0;
    y = baselinePos;

    // clear the background
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

    // Render character string
    while( str != NULL )
    {
        NNS_G2dTextCanvasDrawString(
            &gTextCanvas,           // Pointer to the TextCanvas to render
            x,                      // starting render coordinate  x
            y - baselinePos,        // starting render coordinate  y
            TXT_COLOR_CYAN,         // Character color
            str,                    // Character string to render
            &str                    // Pointer to the buffer that receives the render end position
        );

        y += linefeed;
    }
}



/*---------------------------------------------------------------------------*
  Name:         SampleText

  Description:  NNS_G2dTextCanvasDrawText() sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleText(void)
{
    u32 flag = 0;

    // clear the background
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

    // Display the horizontal and vertical lines that pass through the base point
    {
        const int canvas_pw = CANVAS_WIDTH * CHARACTER_WIDTH;
        const int canvas_ph = CANVAS_HEIGHT * CHARACTER_HEIGHT;
        const int origin_x = canvas_pw / 2;
        const int origin_y = canvas_ph / 2;

        NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_RED, 0, origin_y, canvas_pw, 1);
        NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_RED, origin_x, 0, 1, canvas_ph);
    }

    // key input process and information output
    {
        DTX_PrintLine("NNS_G2dTextCanvasDrawText Sample");
        DTX_PrintLine("  operation:");
        DTX_PrintLine("    up:    origin bottom");
        DTX_PrintLine("    down:  origin top");
        DTX_PrintLine("    left:  origin right");
        DTX_PrintLine("    right: origin left");
        DTX_PrintLine("    Y:     left align");
        DTX_PrintLine("    A:     right align");
        DTX_PrintLine("    X:     next mode");
        DTX_PrintLine("");

        if( CMN_IsPress(PAD_KEY_UP) )
        {
            flag |= NNS_G2D_VERTICALORIGIN_BOTTOM;
            DTX_PrintLine("  NNS_G2D_VERTICALORIGIN_BOTTOM");
        }
        else if( CMN_IsPress(PAD_KEY_DOWN) )
        {
            flag |= NNS_G2D_VERTICALORIGIN_TOP;
            DTX_PrintLine("  NNS_G2D_VERTICALORIGIN_TOP");
        }
        else
        {
            flag |= NNS_G2D_VERTICALORIGIN_MIDDLE;
            DTX_PrintLine("  NNS_G2D_VERTICALORIGIN_MIDDLE");
        }

        if( CMN_IsPress(PAD_KEY_LEFT) )
        {
            flag |= NNS_G2D_HORIZONTALORIGIN_RIGHT;
            DTX_PrintLine("  NNS_G2D_HORIZONTALORIGIN_RIGHT");
        }
        else if( CMN_IsPress(PAD_KEY_RIGHT) )
        {
            flag |= NNS_G2D_HORIZONTALORIGIN_LEFT;
            DTX_PrintLine("  NNS_G2D_HORIZONTALORIGIN_LEFT");
        }
        else
        {
            flag |= NNS_G2D_HORIZONTALORIGIN_CENTER;
            DTX_PrintLine("  NNS_G2D_HORIZONTALORIGIN_CENTER");
        }

        if( CMN_IsPress(PAD_BUTTON_Y) )
        {
            flag |= NNS_G2D_HORIZONTALALIGN_LEFT;
            DTX_PrintLine("  NNS_G2D_HORIZONTALALIGN_LEFT");
        }
        else if( CMN_IsPress(PAD_BUTTON_A) )
        {
            flag |= NNS_G2D_HORIZONTALALIGN_RIGHT;
            DTX_PrintLine("  NNS_G2D_HORIZONTALALIGN_RIGHT");
        }
        else
        {
            flag |= NNS_G2D_HORIZONTALALIGN_CENTER;
            DTX_PrintLine("  NNS_G2D_HORIZONTALALIGN_CENTER");
        }
    }

    // Render character string
    NNS_G2dTextCanvasDrawText(
        &gTextCanvas,                           // Pointer to the TextCanvas to render
        CANVAS_WIDTH * CHARACTER_WIDTH / 2,     // Base point coordinates     CharCanvas center
        CANVAS_HEIGHT * CHARACTER_HEIGHT / 2,   // Base point coordinates     CharCanvas center
        TXT_COLOR_MAGENTA,                      // Character color
        flag,                                   // Render position flag
        sSampleText                             // Character string to render
    );
}



/*---------------------------------------------------------------------------*
  Name:         SampleTextRect

  Description:  NNS_G2dTextCanvasDrawTextRect() sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleTextRect(void)
{
    const int canvas_pw = CANVAS_WIDTH * CHARACTER_WIDTH;
    const int canvas_ph = CANVAS_HEIGHT * CHARACTER_HEIGHT;
    const int rect_x    = canvas_pw * 1/6;
    const int rect_y    = canvas_ph * 1/6;
    const int rect_w    = canvas_pw * 2/3;
    const int rect_h    = canvas_ph * 2/3;
    u32 flag = 0;

    // clear the background
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

    // display the base rectangle
    {
        NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_RED,
            rect_x - 1,      rect_y - 1,      rect_w + 2, 1);
        NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_RED,
            rect_x - 1,      rect_y + rect_h, rect_w + 2, 1);
        NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_RED,
            rect_x - 1,      rect_y,          1,          rect_h);
        NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_RED,
            rect_x + rect_w, rect_y,          1,          rect_h);
    }

    // key input process and information output
    {
        DTX_PrintLine("NNS_G2dTextCanvasDrawTextRect Sample");
        DTX_PrintLine("  operation:");
        DTX_PrintLine("    up:    top align");
        DTX_PrintLine("    down:  bottom align");
        DTX_PrintLine("    left:  left align");
        DTX_PrintLine("    right: right align");
        DTX_PrintLine("    X:     next mode");
        DTX_PrintLine("");

        if( CMN_IsPress(PAD_KEY_UP) )
        {
            flag |= NNS_G2D_VERTICALALIGN_TOP;
            DTX_PrintLine("  NNS_G2D_VERTICALALIGN_TOP");
        }
        else if( CMN_IsPress(PAD_KEY_DOWN) )
        {
            flag |= NNS_G2D_VERTICALALIGN_BOTTOM;
            DTX_PrintLine("  NNS_G2D_VERTICALALIGN_BOTTOM");
        }
        else
        {
            flag |= NNS_G2D_VERTICALALIGN_MIDDLE;
            DTX_PrintLine("  NNS_G2D_VERTICALALIGN_MIDDLE");
        }

        if( CMN_IsPress(PAD_KEY_LEFT) )
        {
            flag |= NNS_G2D_HORIZONTALALIGN_LEFT;
            DTX_PrintLine("  NNS_G2D_HORIZONTALALIGN_LEFT");
        }
        else if( CMN_IsPress(PAD_KEY_RIGHT) )
        {
            flag |= NNS_G2D_HORIZONTALALIGN_RIGHT;
            DTX_PrintLine("  NNS_G2D_HORIZONTALALIGN_RIGHT");
        }
        else
        {
            flag |= NNS_G2D_HORIZONTALALIGN_CENTER;
            DTX_PrintLine("  NNS_G2D_HORIZONTALALIGN_CENTER");
        }
    }

    // Draw a character string
    NNS_G2dTextCanvasDrawTextRect(
        &gTextCanvas,                       // Pointer to the TextCanvas to render
        rect_x,                             // Rectangle position coordinates
        rect_y,                             // Rectangle position coordinates
        rect_w,                             // Base rectangle width
        rect_h,                             // Base rectangle height
        TXT_COLOR_GREEN,                    // Character color
        flag,                               // Render position flag
        sSampleText                         // Character string to render
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
    // display mode
    enum
    {
        MODE_STRING,        // NNS_G2dTextCanvasDrawString   demo
        MODE_TEXT,          // NNS_G2dTextCanvasDrawText     demo
        MODE_TEXT_RECT,     // NNS_G2dTextCanvasDrawTextRect demo
        NUM_OF_MODE
    }
    mode = MODE_TEXT_RECT;

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

        // Change mode with the X button
        if( CMN_IsTrigger(PAD_BUTTON_X) )
        {
            mode ++;
            mode %= NUM_OF_MODE;
        }

        // branch processing according to mode
        switch( mode )
        {
        case MODE_STRING:       SampleString();     break;
        case MODE_TEXT:         SampleText();       break;
        case MODE_TEXT_RECT:    SampleTextRect();   break;
        }

        // register the transmission tasks
        (void)NNS_GfdRegisterNewVramTransferTask(
            NNS_GFD_DST_2D_BG1_CHAR_MAIN,
            sizeof(GXCharFmt16) * CHARACTER_OFFSET,
            gOffBuffer,
            sizeof(gOffBuffer)
        );

        CMN_WaitVBlankIntr();

        // Display the information output
        DTX_Reflect();

        // transmit offscreen buffer
        NNS_GfdDoVramTransfer();
    }
}

