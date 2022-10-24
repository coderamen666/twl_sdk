/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - MinimumCanvas
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
//      Constructs the smallest CharCanvas needed to display a text string, based on that string's size, and then renders it.
//      
//      When the size of the text string provided by the NNS_G2dFontGetTextRect function is invalid as a CharCanvas size, the smallest valid size that is larger than that is calculated to construct CharCanvas.
//      
//      
//
//  Using the Demo
//      A Button: Changes the color mode.
//      B Button: Changes the VRAM mode.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/misc.h>
#include "g2d_textdemolib.h"


#define ROUNDUP(x, div) (((x) + (div) - 1) & ~((div) - 1))

#define CHARACTER_WIDTH     8       // Width of character in pixels
#define CHARACTER_HEIGHT    8       // Height of character in pixels
#define SCREEN_WIDTH        256     // Screen width
#define SCREEN_HEIGHT       192     // Screen height

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Starting address for the string to use

#define MAX_OBJ_NUM         128     // Maximum number of OBJs
#define MAX_CHARA_NAME      1024    // Maximum character name value


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont              gFont;          // Font

GXOamColorMode gColorMode       = GX_OAM_COLORMODE_16;
NNSG2dOBJVramMode gVramMode     = NNS_G2D_OBJVRAMMODE_32K;

int gUsingCharas = 0;
int gUsingOam    = 0;

const static NNSG2dChar* sampleText[] =
{
    NNS_G2D_TRANSCODE("This sample program make temporary minimum\n"
                      "CharCanvas to draw a text and draw the\n"
                      "text to it."),
    NNS_G2D_TRANSCODE("sample text"),
    NNS_G2D_TRANSCODE("this text require\n11x3 size."),
    NNS_G2D_TRANSCODE("line 1\n" "line 2\n" "line 3\n" "line 4\n"),
    NNS_G2D_TRANSCODE("A"),
};

//****************************************************************************
// Initialization, etc.
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         GetMinValidCanvasSize

  Description:  Gets the smallest valid size for a CharCanvas that is larger than the given size.
                

  Arguments:    areaWidth:  CharCanvas width (in character units).
                areaHeight: CharCanvas height (in character units).
                colorMode:  CharCanvas color mode.
                vramMode:   OBJ VRAM capacity

  Returns:      Returns the smallest valid size for a CharCanvas that is larger than the given size.
                
 *---------------------------------------------------------------------------*/
static NNSG2dTextRect GetMinValidCanvasSize(
    int                 areaWidth,
    int                 areaHeight,
    GXOamColorMode      colorMode,
    NNSG2dOBJVramMode   vramMode
)
{
    NNSG2dTextRect rect;
    NNS_G2D_MIN_ASSERT( areaWidth,  1 );
    NNS_G2D_MIN_ASSERT( areaHeight, 1 );
    NNS_G2D_ASSERT( (colorMode == GX_OAM_COLORMODE_16)
                 || (colorMode == GX_OAM_COLORMODE_256) );
    NNS_G2D_ASSERT( (vramMode == NNS_G2D_OBJVRAMMODE_32K)
                 || (vramMode == NNS_G2D_OBJVRAMMODE_64K)
                 || (vramMode == NNS_G2D_OBJVRAMMODE_128K)
                 || (vramMode == NNS_G2D_OBJVRAMMODE_256K) );

    rect.width  = areaWidth;
    rect.height = areaHeight;

    if(    (colorMode == GX_OAM_COLORMODE_16 && vramMode == NNS_G2D_OBJVRAMMODE_128K)
        || (colorMode != GX_OAM_COLORMODE_16 && vramMode == NNS_G2D_OBJVRAMMODE_256K) )
    {
        // The case when it is impossible to have a combination of an odd number and a "value whose remainder is 3 when divided by 4."
        // If the conditions are met, it is okay to add 1 to the appropriate value.
        if( (rect.width % 2) != 0 && (rect.height % 2) != 0 )
        {
            if( (rect.width % 4) != 1 )
            {
                // width % 4 == 3 && height % 4 == 1,3
                rect.width++;
            }
            else if( (rect.height % 4) != 1 )
            {
                // width % 4 == 1 && height % 4 == 3
                rect.height++;
            }
        }
    }
    else if( colorMode == GX_OAM_COLORMODE_16 && vramMode == NNS_G2D_OBJVRAMMODE_256K )
    {
        // Both must be even numbers.
        rect.width  = ROUNDUP(rect.width,  2);
        rect.height = ROUNDUP(rect.height, 2);
    }

    return rect;
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

  Returns:      Returns the TWL-System color mode enumerator that corresponds to the SDK.
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

  Description:  Releases and resets all OAMs.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ResetOAM(void)
{
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    gUsingOam = 0;
}



/*---------------------------------------------------------------------------*
  Name:         ResetOBJChr

  Description:  Releases all OBJ characters.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ResetOBJChr(void)
{
    gUsingCharas = 0;
}



/*---------------------------------------------------------------------------*
  Name:         AllocCharacters

  Description:  This is a simple character manager.
                Allocates the specified number of characters and returns the first character name from the allocated character array.
                

  Arguments:    num:    The number of characters to allocate.

  Returns:      Returns the first character name from the allocated character string when allocation is successful.
                
                Returns a -1 if failed.
 *---------------------------------------------------------------------------*/
static int AllocCharacters(
    int num,
    GXOamColorMode colorMode,
    NNSG2dOBJVramMode vramMode )
{
    int ret;
    int realNum = ROUNDUP_DIV(num, (1 << vramMode));

    if( colorMode == GX_OAM_COLORMODE_256 )
    {
        realNum *= 2;
    }

    if( gUsingCharas + realNum > MAX_CHARA_NAME )
    {
        return -1;
    }

    ret = gUsingCharas;
    gUsingCharas += realNum;

    return ret;
}



/*---------------------------------------------------------------------------*
  Name:         AllocOAM

  Description:  This is a simple OAM manager.
                
                Allocates an OAM with the specified number and returns a pointer to the start of that allocated OAM array.

  Arguments:    num:    The number of OAMs to allocate.

  Returns:      Returns the first character name from the allocated character string when allocation is successful.
                
                Returns a NULL if failed.
 *---------------------------------------------------------------------------*/
static GXOamAttr* AllocOAM(int num)
{
    GXOamAttr* const pObjBase = (GXOamAttr*)HW_OAM;
    GXOamAttr* ret;

    if( gUsingOam + num > MAX_OBJ_NUM )
    {
        return NULL;
    }

    ret = pObjBase + gUsingOam;
    gUsingOam += num;

    return ret;
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
  Name:         PutText

  Description:  Displays the specified text to the specified location.

  Arguments:    x:          The position of the text to be displayed
                y:          The position of the text to be displayed
                pFont:      The font for drawing the text
                colorMode:  Specifies the color mode of the main OBJ.
                vramMode:   Specifies the VRAM mode of the main OBJ.
                color:      Specifies the text's color number.
                txt:        Specifies the text to display.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PutText(
    int                 x,
    int                 y,
    const NNSG2dFont*   pFont,
    GXOamColorMode      colorMode,
    NNSG2dOBJVramMode   vramMode,
    u8                  color,
    const NNSG2dChar*   txt
)
{
    void* const pCharBase = G2_GetOBJCharPtr();
    NNSG2dCharCanvas cc;
    NNSG2dTextCanvas tc;
    NNSG2dTextRect textRect;
    NNSG2dTextRect canvasSize;
    int areaWidth, areaHeight;
    GXOamAttr* pOam;
    int numOBJ;
    int charNo;

    // Finds the size of the text region
    textRect   = NNS_G2dFontGetTextRect(pFont, TEXT_HSPACE, TEXT_VSPACE, txt);
    areaWidth  = ROUNDUP_DIV(textRect.width,  CHARACTER_WIDTH);
    areaHeight = ROUNDUP_DIV(textRect.height, CHARACTER_HEIGHT);
    canvasSize = GetMinValidCanvasSize(areaWidth, areaHeight, colorMode, vramMode);

    // Allocates OAM and characters
    numOBJ = NNS_G2dCalcRequiredOBJ1D( canvasSize.width, canvasSize.height );
    charNo = AllocCharacters( canvasSize.width * canvasSize.height, colorMode, vramMode );
    pOam   = AllocOAM( numOBJ );
    NNS_G2D_ASSERT( charNo >= 0 );
    NNS_G2D_POINTER_ASSERT( pOam );

    // Initializes the CharCanvas for the 1D OBJ at the text region size
    NNS_G2dCharCanvasInitForOBJ1D(
        &cc,
        OffsetCharName(pCharBase, charNo, vramMode),
        canvasSize.width,
        canvasSize.height,
        SDKToNNSColorMode(colorMode)
    );

    // Place the OBJ for the CharCanvas
    (void)NNS_G2dArrangeOBJ1D(
        pOam,
        canvasSize.width,
        canvasSize.height,
        x, y,
        colorMode,
        charNo,
        vramMode
    );

    // Configure OBJ settings
    TXT_SetCharCanvasOBJAttrs(
        pOam,                  // Pointer to the OAM array
        numOBJ,                 // The number of target OAMs
        0,                      // Priority
        GX_OAM_MODE_NORMAL,     // OBJ Mode
        FALSE,                  // Mosaic
        GX_OAM_EFFECT_NONE,     // OBJ effect
        TXT_CPALETTE_MAIN,      // Palette number
        0                       // Affine parameter index
    );

    // Build the TextCanvas
    NNS_G2dTextCanvasInit(
        &tc,
        &cc,
        pFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );

    // Clear the canvas and render the character string
    NNS_G2dCharCanvasClear(
        &cc,
        TXT_COLOR_WHITE
    );
    NNS_G2dTextCanvasDrawText(
        &tc,
        0, 0,
        color,
        TXT_DRAWTEXT_FLAG_DEFAULT,
        txt
    );


    DTX_PrintLine("%3dx%3d  %2dx%2d  %2dx%2d  %4d  %3d",
        textRect.width, textRect.height,
        areaWidth, areaHeight,
        canvasSize.width, canvasSize.height,
        canvasSize.width * canvasSize.height, numOBJ
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
        "Text size dependents Canvas Sample\n"
        "operation\n"
        "  A  change color mode\n"
        "  B  change vram mode\n"
    );
    DTX_PrintLine("color mode:  %s", colorModeString[gColorMode]);
    DTX_PrintLine("vram mode:   %s", vramModeString[gVramMode]);
    DTX_PrintLine(
        "\n"
        "Size                    Using\n"
        "text     canvas adjust  char  OBJ"
    );
}



/*---------------------------------------------------------------------------*
  Name:         SampleDraw

  Description:  Rendering for the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleDraw(void)
{
    int i;

    PrintSampleInfo();

    switch( gVramMode )
    {
    case NNS_G2D_OBJVRAMMODE_32K:  GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);   break;
    case NNS_G2D_OBJVRAMMODE_64K:  GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_64K);   break;
    case NNS_G2D_OBJVRAMMODE_128K: GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_128K);  break;
    case NNS_G2D_OBJVRAMMODE_256K: GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_256K);  break;
    default:                       GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_32K);   break;
    }

    ResetOAM();
    ResetOBJChr();

    for( i = 0; i < ARY_SIZEOF(sampleText); ++i )
    {
        int x = (int)(SCREEN_WIDTH / ARY_SIZEOF(sampleText)) * i;
        int y = (int)(SCREEN_HEIGHT / ARY_SIZEOF(sampleText)) * i;
        u8 color = (u8)(TXT_COLOR_RED + i);

        PutText(x, y, &gFont, gColorMode, gVramMode, color, sampleText[i]);
    }
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

    InitScreen();
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
        SampleDraw();
        DC_StoreAll();
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
    // Initializing app.
    {
        // Initialization of SDK and demo library
        OS_Init();
        TXT_Init();

        // Configure the background
        TXT_SetupBackground();

        // Sample initialization
        SampleInit();
    }

    // Start display
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

