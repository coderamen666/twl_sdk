/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - OBJ1D
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
//      This is a sample of character rendering on the 1D mapping OBJ.
//      It displays the number of OBJ needed depending on the size of CharCanvas.
//      Note that characters might not be displayed correctly in OBJ1D, owing to the combination of the color mode, the VRAM mode and the length of the sides of the canvas.
//      
//        See also: NNS_G2dCharCanvasInitForOBJ1D() API reference
//
//  Using the Demo
//      A Button: Changes the color mode.
//      B Button: Changes the VRAM mode.
//      +Control Pad: Changes the size of CharCanvas.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/misc.h>
#include "g2d_textdemolib.h"


#define CANVAS_WIDTH        28      // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       10      // Height of character rendering area (in character units)
#define CANVAS_LEFT         13      // X position of character drawing area (in pixels)
#define CANVAS_TOP          17      // Y position of character drawing area (in pixels)

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Top character name of the character string to be used.

#define MAX_OBJ_NUM         128
#define REPEAT_THRESHOLD    22


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont              gFont;          // Font
NNSG2dCharCanvas        gCanvas;        // CharCanvas
NNSG2dTextCanvas        gTextCanvas;    // TextCanvas

const char* gErrMsg         = NULL;
int gCanvasWidth            = CANVAS_WIDTH;
int gCanvasHeight           = CANVAS_HEIGHT;
GXOamColorMode gColorMode   = GX_OAM_COLORMODE_16;
NNSG2dOBJVramMode gVramMode = NNS_G2D_OBJVRAMMODE_32K;

//****************************************************************************
// Initialize etc.
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         IsValidCharCanvasSize

  Description:  Determines whether the CharCanvas size can be displayed correctly.
                However, it does not determine excesses in size.

  Arguments:    areaWidth:  CharCanvas width (in character units).
                areaHeight: CharCanvas height (in character units).
                colorMode:  CharCanvas color mode.
                vramMode:   OBJ VRAM capacity

  Returns:      Returns TRUE if it can be displayed correctly.
 *---------------------------------------------------------------------------*/
static BOOL IsValidCharCanvasSize(
    int                 areaWidth,
    int                 areaHeight,
    GXOamColorMode      colorMode,
    NNSG2dOBJVramMode   vramMode
)
{
    NNS_G2D_ASSERT( (colorMode == GX_OAM_COLORMODE_16)
                 || (colorMode == GX_OAM_COLORMODE_256) );
    NNS_G2D_ASSERT( (vramMode == NNS_G2D_OBJVRAMMODE_32K)
                 || (vramMode == NNS_G2D_OBJVRAMMODE_64K)
                 || (vramMode == NNS_G2D_OBJVRAMMODE_128K)
                 || (vramMode == NNS_G2D_OBJVRAMMODE_256K) );

    // The size must be a positive number.
    if( areaWidth <= 0 || areaHeight <= 0 )
    {
        return FALSE;
    }

    if(    (colorMode == GX_OAM_COLORMODE_16 && vramMode == NNS_G2D_OBJVRAMMODE_128K)
        || (colorMode != GX_OAM_COLORMODE_16 && vramMode == NNS_G2D_OBJVRAMMODE_256K) )
    {
        // The case when it is impossible to have a combination of an odd number and a "value whose remainder is 3 when divided by 4."
        // This case has the same value as "possible if either value is even or if the remainder is not three when both are divided by 4," so determine based on this case.
        // 
        return ((areaWidth  % 2) == 0)
            || ((areaHeight % 2) == 0)
            || ((areaWidth  % 4) != 3 && (areaHeight % 4) != 3);
    }
    else if( colorMode == GX_OAM_COLORMODE_16 && vramMode == NNS_G2D_OBJVRAMMODE_256K )
    {
        // Both must be even numbers.
        return (areaWidth % 2) == 0 && (areaHeight % 2) == 0;
    }
    else
    {
        return TRUE;
    }
}




/*---------------------------------------------------------------------------*
  Name:         OffsetCharName

  Description:  Gets the address indicated by the character name with the 1D mapping OBJ.

  Arguments:    pCharBase:  The OBJ character base
                charName:   The target character name
                vramMode:   The VRAM mode

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void* OffsetCharName(
    void*               pCharBase,
    int                 charName,
    NNSG2dOBJVramMode   vramMode )
{
    u32 addr = (u32)pCharBase;
    addr += sizeof(GXCharFmt16) * (charName << vramMode);
    return (void*)addr;
}



/*---------------------------------------------------------------------------*
  Name:         SDKToNNSColorMode

  Description:  Converts the NITRO-SDK color mode enumerator to a TWL-System color mode enumerator.
                

  Arguments:    sdk:    A NITRO-SDK color mode enumerator

  Returns:      Returns the TWL-System color mode enumerator that corresponds to sdk.
 *---------------------------------------------------------------------------*/
static inline NNSG2dCharaColorMode SDKToNNSColorMode(GXOamColorMode sdk)
{
    static const NNSG2dCharaColorMode nns[] =
        { NNS_G2D_CHARA_COLORMODE_16, NNS_G2D_CHARA_COLORMODE_256 };

    NNS_MINMAX_ASSERT(sdk, 0, 1);
    return nns[sdk];
}



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

        if( CMN_IsTrigger(PAD_BUTTON_A) )
        {
            gColorMode = (gColorMode != GX_OAM_COLORMODE_16) ?
                            GX_OAM_COLORMODE_16:
                            GX_OAM_COLORMODE_256;
            bChanged = TRUE;
        }
        if( CMN_IsTrigger(PAD_BUTTON_B) )
        {
            gVramMode++;
            if( gVramMode > NNS_G2D_OBJVRAMMODE_256K )
            {
                gVramMode = NNS_G2D_OBJVRAMMODE_32K;
            }
            bChanged = TRUE;
        }
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
        if( MAX_OBJ_NUM < NNS_G2dCalcRequiredOBJ1D(gCanvasWidth, gCanvasHeight) )
        {
            // The maximum number of usable OBJs has been exceeded
            gCanvasWidth = old_cw;
            gCanvasHeight = old_ch;
            gErrMsg = "OBJ OVERFLOW";
            bChanged = TRUE;
        }
        {
            const int maxChr = ((1024 << gVramMode) >> gColorMode) - CHARACTER_OFFSET;

            // 
            if( maxChr < (gCanvasWidth + 1) * (gCanvasHeight + 1) )
            {
                // The maximum number of usable characters has been exceeded
                gCanvasWidth = old_cw;
                gCanvasHeight = old_ch;
                gErrMsg = "Character OVERFLOW";
                bChanged = TRUE;
            }
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
    void*      const pCharBase = G2_GetOBJCharPtr();
    GXOamAttr* const pObjBase = (GXOamAttr*)HW_OAM;
    const int cOffset = CHARACTER_OFFSET;
    int nObjs;


    // mapping mode settings
    switch( gVramMode )
    {
    case NNS_G2D_OBJVRAMMODE_32K:  GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);   break;
    case NNS_G2D_OBJVRAMMODE_64K:  GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_64K);   break;
    case NNS_G2D_OBJVRAMMODE_128K: GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);  break;
    case NNS_G2D_OBJVRAMMODE_256K: GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_256K);  break;
    default:                       GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);   break;
    }


    // CharCanvas initialization
    NNS_G2dCharCanvasInitForOBJ1D(
        &gCanvas,
        OffsetCharName(pCharBase, cOffset, gVramMode),
            // Correction since the character name boundaries differ depending on the size of the VRAM that can be referenced

        gCanvasWidth,
        gCanvasHeight,
        SDKToNNSColorMode(gColorMode)
    );

    // Clear the canvas
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

    // place the OBJ
    nObjs = NNS_G2dArrangeOBJ1D(
        pObjBase,
        gCanvasWidth,
        gCanvasHeight,
        CANVAS_LEFT,
        CANVAS_WIDTH,
        gColorMode,
        cOffset,
        gVramMode
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

    if( ! IsValidCharCanvasSize(gCanvasWidth, gCanvasHeight, gColorMode, gVramMode) )
    {
        gErrMsg = "INVALID CharCanvas size";
    }
}



/*---------------------------------------------------------------------------*
  Name:         SampleDraw

  Description:  Rendering for the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleDraw(void)
{
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
    static const char* colorModeString[] =
    {
        "GX_OAM_COLORMODE_16",
        "GX_OAM_COLORMODE_256"
    };
    static const char* vramModeString[] =
    {
        "NNS_G2D_OBJVRAMMODE_32K",
        "NNS_G2D_OBJVRAMMODE_64K",
        "NNS_G2D_OBJVRAMMODE_128K",
        "NNS_G2D_OBJVRAMMODE_256K"
    };

    DTX_PrintLine(
        "1D mapping OBJ CharCanvas Sample\n"
        "operation\n"
        "  A  change color mode\n"
        "  B  change vram mode\n"
        "  +  change canvas size\n"
    );
    DTX_PrintLine("color mode:  %s", colorModeString[gColorMode]);
    DTX_PrintLine("vram mode:   %s", vramModeString[gVramMode]);
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
    gErrMsg = NULL;

    if( HandleInput() )
    {
        PrintSampleInfo();

        ResetOAM();
        InitCanvas();

        SampleDraw();
        DC_StoreAll();

        if( gErrMsg != NULL )
        {
            DTX_PrintLine("\n\n\n\n     !!! %s !!!", gErrMsg);
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

