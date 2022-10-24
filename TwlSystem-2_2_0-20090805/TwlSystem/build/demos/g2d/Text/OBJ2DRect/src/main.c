/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - OBJ2DRect
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
//      This is a sample of character rendering on the 2D mapping OBJ.
//      It displays the number of OBJ needed depending on the size of CharCanvas.
//
//  Using the Demo
//      +Control Pad: Changes the size of CharCanvas.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include "g2d_textdemolib.h"


#define CANVAS_WIDTH        28      // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       10      // Height of character rendering area (in character units)
#define CANVAS_LEFT         13      // X position of character drawing area (in pixels)
#define CANVAS_TOP          17      // Y position of character drawing area (in pixels)

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    32 * 1  // Starting address for the string to use

#define REPEAT_THRESHOLD    22


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont              gFont;          // Font
NNSG2dCharCanvas        gCanvas;        // CharCanvas
NNSG2dTextCanvas        gTextCanvas;    // TextCanvas

int gCanvasWidth    = CANVAS_WIDTH;
int gCanvasHeight   = CANVAS_HEIGHT;


//****************************************************************************
// Initialize etc.
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         ResetOAM

  Description:  Resets the OAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ResetOAM(void)
{
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
}



/*---------------------------------------------------------------------------*
  Name:         HandleInput

  Description:  Reads in input.

  Arguments:    None.

  Returns:      TRUE if the internal state changes because of the input.
 *---------------------------------------------------------------------------*/
static BOOL HandleInput(void)
{
    BOOL bChanged = FALSE;

#define REPEAT(key)   ((repeat_count >= REPEAT_THRESHOLD) ? CMN_IsPress(key): CMN_IsTrigger(key))
    {
        static int repeat_count = 0;
        int old_cw = gCanvasWidth;
        int old_ch = gCanvasHeight;

        if( REPEAT(PAD_KEY_UP) )
        {
            gCanvasHeight--;
        }
        if( REPEAT(PAD_KEY_DOWN) )
        {
            gCanvasHeight++;
        }
        if( REPEAT(PAD_KEY_LEFT) )
        {
            gCanvasWidth--;
        }
        if( REPEAT(PAD_KEY_RIGHT) )
        {
            gCanvasWidth++;
        }
        if( gCanvasWidth < 1 )
        {
            gCanvasWidth = 1;
        }
        if( gCanvasHeight < 1 )
        {
            gCanvasHeight = 1;
        }
        if( 32 < gCanvasWidth )
        {
            gCanvasWidth = 32;
        }
        if( 31 < gCanvasHeight )
        {
            gCanvasHeight = 31;
        }

        if( gCanvasWidth != old_cw || gCanvasHeight != old_ch )
        {
            bChanged = TRUE;
        }
        if( CMN_IsPress(PAD_PLUS_KEY_MASK) )
        {
            repeat_count++;
        }
        else
        {
            repeat_count = 0;
        }
    }

    return bChanged;
}



/*---------------------------------------------------------------------------*
  Name:         InitScreen

  Description:  Initializes the OBJ display.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitScreen(void)
{
    // Mapping mode configuration  2D 32 KB
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

    // Make the OBJ visible
    CMN_SetPlaneVisible( GX_PLANEMASK_OBJ );

    // Configure color palette
    GX_LoadOBJPltt(TXTColorPalette, 0, sizeof(TXTColorPalette));
}



/*---------------------------------------------------------------------------*
  Name:         InitCanvas

  Description:  Initializes the character string render.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitCanvas(void)
{
    GXCharFmt16* const pCharBase = (GXCharFmt16*)G2_GetOBJCharPtr();
    GXOamAttr* const pObjBase = (GXOamAttr*)HW_OAM;
    const int cOffset = CHARACTER_OFFSET;
    int nObjs;

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForOBJ2DRect(
        &gCanvas,
        pCharBase + cOffset,
        gCanvasWidth,
        gCanvasHeight,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // place the OBJ
    nObjs = NNS_G2dArrangeOBJ2DRect(
        pObjBase,
        gCanvasWidth,
        gCanvasHeight,
        CANVAS_LEFT,
        CANVAS_WIDTH,
        GX_OAM_COLORMODE_16,
        cOffset
    );

    // configure the OBJ
    TXT_SetCharCanvasOBJAttrs(
        pObjBase,               // pointer to the OAM array
        nObjs,                  // the number of target OAMs
        0,                      // priority
        GX_OAM_MODE_NORMAL,     // OBJ Mode
        FALSE,                  // mosaic
        GX_OAM_EFFECT_NONE,     // OBJ effect
        TXT_CPALETTE_MAIN,      // palette number
        0                       // affine parameter index
    );

    // TextCanvas initialization
    NNS_G2dTextCanvasInit(
        &gTextCanvas,
        &gCanvas,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );

    // The number of OBJ in use
    DTX_PrintLine("using OBJ:   %3d", nObjs);
}



/*---------------------------------------------------------------------------*
  Name:         SampleDraw

  Description:  Rendering for the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleDraw(void)
{
    // Clear the canvas
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

    // character rendering
    NNS_G2dTextCanvasDrawText(&gTextCanvas, 1, 1, TXT_COLOR_BLACK, TXT_DRAWTEXT_FLAG_DEFAULT,
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n"
        "bcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789a\n"
        "cdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ab\n"
        "defghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abc\n"
        "efghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcd\n"
        "fghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcde\n"
        "ghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdef\n"
        "hijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefg\n"
        "ijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefgh\n"
        "jklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghi\n"
        "klmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghij\n"
        "lmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijk\n"
        "mnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijkl\n"
        "nopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklm\n"
        "opqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmn\n"
        "pqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmno\n"
        "qrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnop\n"
        "rstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopq\n"
        "stuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqr\n"
        "tuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrs\n"
        "uvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrst\n"
        "vwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstu\n"
        "wxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuv\n"
        "xyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvw\n"
        "yzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwx\n"
    );
}



/*---------------------------------------------------------------------------*
  Name:         PrintSampleInfo

  Description:  Outputs the sample information to the bottom screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintSampleInfo(void)
{
    DTX_PrintLine(
        "2D mapping OBJ CharCanvas Sample\n"
        "operation\n"
        "  +  change canvas size\n"
    );
    DTX_PrintLine("canvas size: %2d x %2d", gCanvasWidth, gCanvasHeight);
}



/*---------------------------------------------------------------------------*
  Name:         SampleInit

  Description:  This is the initialization process for the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleInit(void)
{
    // Loads the font
    TXT_LoadFont( &gFont, TXT_FONTRESOURCE_NAME );

    PrintSampleInfo();

    InitScreen();
    InitCanvas();
    SampleDraw();
}



/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  This is the main process of the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleMain(void)
{
    if( HandleInput() )
    {
        PrintSampleInfo();

        ResetOAM();
        InitCanvas();

        SampleDraw();
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

        // Main process of sample
        SampleMain();

        CMN_WaitVBlankIntr();

        // Display the information output
        DTX_Reflect();
    }
}

