/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - SimpleFontView
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
//      Displays the glyph image inside the font resource.
//      If you delete the fonts.NFTR file within the data directory, insert another font, and then rebuild, the content of the substituted font will be displayed.
//      
//      Do not delete the fontd.NFTR file inside the data directory.
//
//  Using the Demo
//      Left arrow, right arrow: Moves 1 character
//      Up arrow, down arrow: Moves 1 line
//      B Button: Enables key repeat while this button is depressed
//      X Button: Changes the scaling 4-->1-->2-->3-->4-->1-->...
// ============================================================================

#include "g2d_textdemolib.h"
#include <string.h>
#include <nnsys/gfd.h>

#define SCREEN_WIDTH        256                     // screen width
#define SCREEN_HEIGHT       192                     // screen height

#define CHARACTER_WIDTH     8
#define CHARACTER_HEIGHT    8



#define LIMIT_GLYPH_INDEX(gi)   \
    while( gi > giMax )         \
    {                           \
        gi -= giMax + 1;        \
    }                           \
    while( gi < 0 )             \
    {                           \
        gi += giMax + 1;        \
    }


//------------------------------------------------------------------------------
// Global Variables
NNSG2dCharCanvas    gCc;        // CharCanvas for display of glyph list / glyph index
NNSG2dTextCanvas    gTxn;       // TextCanvas for glyph list display
NNSG2dTextCanvas    gITxn;      // TextCanvas for glyph index display

NNSG2dCharCanvas    gDCc;       // CharCanvas for glyph image expanded display
NNSG2dTextCanvas    gDTxn;      // TextCanvas for glyph image expanded display

NNSG2dFont          gFont;      // display target font
NNSG2dFont          gInfoFont;  // font for information display

int                 giCenter;   // glyph index of the center of the glyph list
int                 giMax;      // Maximum glyph index value of the gFont
int                 giLine;     // Number of glyphs to be displayed in the glyph list

static GXCharBGText256          gBG1OffScreen;      // for displaying glyph list / glyph index
static GXCharBGAffine256        gBG2OffScreen;      // for glyph image expanded display
static NNSGfdVramTransferTask   gTransferTask[2];   // for off-screen transmission

const static int BG1_CANVAS_WIDTH   = 32;
const static int BG1_CANVAS_HEIGHT  = 24;
const static int BG1_CANVAS_LEFT    =  0;
const static int BG1_CANVAS_TOP     =  0;
const static int BG1_CANVAS_OFFSET  =  1;

const static int BG2_CANVAS_WIDTH   = 16;
const static int BG2_CANVAS_HEIGHT  = 12;
const static int BG2_CANVAS_LEFT    =  0;
const static int BG2_CANVAS_TOP     =  0;
const static int BG2_CANVAS_OFFSET  =  1;

const static int CHAR_SPACE         =  5;
const static int MARGIN             =  5;

const static int REPEAT_THRESHOLD   = 22;

static int scale = 4;
static char fontName[FS_FILE_NAME_MAX + 1];



//****************************************************************************
// wrapper
//****************************************************************************
static inline GetCharWidth(const NNSG2dCharWidths* pWidths)
{
    return pWidths->charWidth;
}


//****************************************************************************
// Initialize etc.
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         ClearAreaSafe

  Description:  Calls the NNS_G2dCharCanvasClearArea function once the arguments have been stored in an appropriate range.
                

  Arguments:    pCC:    Pointer to CharCanvas.
                cl:     Color number used in filling
                x:      x-coordinate of the upper-left corner of the rectangle
                y:      y-coordinate of the upper-left corner of the rectangle
                w:      rectangle width
                h:      rectangle height

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearAreaSafe(
    const NNSG2dCharCanvas* pCC,
    int cl,
    int x,
    int y,
    int w,
    int h
)
{
    // Maximum values that can be designated to an argument
    const int x_max = pCC->areaWidth * CHARACTER_WIDTH;
    const int y_max = pCC->areaHeight * CHARACTER_HEIGHT;

    if( x < 0 )
    {
        x = 0;
    }
    if( y < 0 )
    {
        y = 0;
    }
    if( x_max <= x )
    {
        x = x_max - 1;
    }
    if( y_max <= y )
    {
        y = y_max - 1;
    }
    if( x_max < w )
    {
        w = x_max;
    }
    if( y_max < h )
    {
        h = y_max;
    }
    if( x_max < x + w )
    {
        w = x_max - x;
    }
    if( y_max < y + h )
    {
        h = y_max - y;
    }
    if( w < 0 || h < 0 )
    {
        return;
    }

    NNS_G2dCharCanvasClearArea(pCC, cl, x, y, w, h);
}



/*---------------------------------------------------------------------------*
  Name:         LoadFont

  Description:  Loads the font.
                This looks for files with the extension *.NFTR and loads the first one that is found.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadFont(void)
{
    BOOL bSuccess;
    FSFile dir;
    FSDirEntry entry;

    FS_InitFile(&dir);

    bSuccess = FS_FindDir(&dir, "/data");
    SDK_ASSERT( bSuccess );

    // Search inside the directory
    while( FS_ReadDir(&dir, &entry) != FALSE )
    {
        if( entry.name_len >= 5 )
        {
            // The extension is *.NFTR, and
            if( strcmp( entry.name + entry.name_len - 5, ".NFTR" ) == 0 )
            {
                // the font is not a font for debug display
                if( strcmp( entry.name, "fontd.NFTR" ) != 0 )
                {
                    char fname[6 + FS_FILE_NAME_MAX + 1] = "/data/";

                    (void)strcat(fname, entry.name);
                    (void)strcpy(fontName, entry.name);

                    // load the font
                    TXT_LoadFont( &gFont, fname );
                    return;
                }
            }
        }
    }

    SDK_ASSERTMSG(FALSE, "There are no fonts.");
}



/*---------------------------------------------------------------------------*
  Name:         InitScreenCommon

  Description:  Performs initialization that is common among each BG screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitScreenCommon(void)
{
    // backdrop color + information display character color + frame color + maximum number of font gradations (up to 32 gradations)
    const int max_colors = 32;
    static u16 colorPalette[1 + 1 + max_colors] =
    {
        GX_RGB(0, 0, 31), GX_RGB(9, 27, 17), GX_RGB(27, 2, 2)
    };

    // load the information display font
    TXT_LoadFont( &gInfoFont, DEBUG_FONTRESOURCE_NAME );

    // load the display target font
    LoadFont();

    // create a color palette based on the font that was loaded
    {
        const int nColors = MATH_IMin((1 << NNS_G2dFontGetBpp(&gFont)), max_colors);
        int i;

        for( i = 0; i < nColors; ++i )
        {
            int level = ((nColors - 1 - i) * (max_colors - 1) / (nColors - 1));

            colorPalette[i+3] = GX_RGB(level, level, level);
        }
    }

    // load the color palette
    GX_LoadBGPltt(colorPalette, 0, sizeof(colorPalette));
}



/*---------------------------------------------------------------------------*
  Name:         InitEnumScreen

  Description:  Initializes the canvas for the display of the character list / glyph index

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitEnumScreen(void)
{
    const static int margin = 1;

    // Set BG1 to the text BG
    G2_SetBG1Control(
        GX_BG_SCRSIZE_TEXT_256x256,
        GX_BG_COLORMODE_256,
        GX_BG_SCRBASE_0x0000,
        GX_BG_CHARBASE_0x00000,
        GX_BG_EXTPLTT_01
    );

    // Make BG 1 visible
    CMN_SetPlaneVisible(GX_PLANEMASK_BG1);

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gCc,
        &gBG1OffScreen,
        BG1_CANVAS_WIDTH,
        BG1_CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_256
    );

    // screen configuration
    NNS_G2dMapScrToCharText(
        G2_GetBG1ScrPtr(),
        BG1_CANVAS_WIDTH,
        BG1_CANVAS_HEIGHT,
        BG1_CANVAS_LEFT,
        BG1_CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        BG1_CANVAS_OFFSET,
        0
    );

    // Clear CharCanvas
    NNS_G2dCharCanvasClear(&gCc, 0);

    // TextCanvas initialization
    NNS_G2dTextCanvasInit(&gTxn, &gCc, &gFont, margin, margin);
    NNS_G2dTextCanvasInit(&gITxn, &gCc, &gInfoFont, margin, margin);

    // get the maximum value of the glyph index
    giMax = (int)NNS_G2D_FONT_MAX_GLYPH_INDEX(&gFont) - 1;

    // calculate the number of glyph lists to display
    {
        const int cw = NNS_G2dFontGetMaxCharWidth(&gFont);

        giLine = ((SCREEN_WIDTH/2) / (cw + CHAR_SPACE)) * 2 + 1;
    }
}



/*---------------------------------------------------------------------------*
  Name:         UpdateScreenScale

  Description:  Applies the change in the glyph image scaling.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void UpdateScreenScale(void)
{
    const int cw = NNS_G2dFontGetMaxCharWidth(&gFont);
    const int ch = NNS_G2dFontGetHeight(&gFont);
    const int ew = cw + MARGIN * 2;
    const int eh = ch + MARGIN * 2;
    const int info_height = MARGIN + ch + MARGIN;
    const int x = (SCREEN_WIDTH - ew * scale + 1) / 2;
    const int y = (SCREEN_HEIGHT - eh * scale + 1 - info_height) / 2;
    MtxFx22 mtx;

    // affine matrix to scale up by the scale factor
    mtx._00 = FX32_ONE / scale;
    mtx._01 = 0;
    mtx._10 = 0;
    mtx._11 = FX32_ONE / scale;


    // Set BG2 to the affine matrix
    G2_SetBG2Affine(&mtx, 0, 0, -x, -y);
}



/*---------------------------------------------------------------------------*
  Name:         InitDoubleScaleScreen

  Description:  Canvas initialization for glyph image expanded display.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitDoubleScaleScreen(void)
{
    const static int margin = 1;

    // Set BG 2 to the affine BG
    G2_SetBG2ControlAffine(
        GX_BG_SCRSIZE_AFFINE_256x256,
        GX_BG_AREAOVER_XLU,
        GX_BG_SCRBASE_0x0800,
        GX_BG_CHARBASE_0x10000
    );

    UpdateScreenScale();

    // Make BG2 visible
    CMN_SetPlaneVisible(GX_PLANEMASK_BG2);


    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gDCc,
        &gBG2OffScreen,
        BG2_CANVAS_WIDTH,
        BG2_CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_256
    );

    // screen configuration
    NNS_G2dMapScrToCharAffine(
        &( ((GXScrAffine32x32*)G2_GetBG2ScrPtr())->scr[BG2_CANVAS_TOP][BG2_CANVAS_LEFT] ),
        BG2_CANVAS_WIDTH,
        BG2_CANVAS_HEIGHT,
        NNS_G2D_AFFINE_BG_WIDTH_256,
        BG2_CANVAS_OFFSET
    );

    // Clear CharCanvas
    NNS_G2dCharCanvasClear(&gDCc, 0);

    // TextCanvas initialization
    NNS_G2dTextCanvasInit(&gDTxn, &gDCc, &gFont, margin, margin);
}



/*---------------------------------------------------------------------------*
  Name:         UpdateDisplay

  Description:  Performs rendering.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void UpdateDisplay(void)
{
    const u8 barColor       = 1;    // green  GX_RGB(9, 27, 17)
    const u8 borderColor    = 2;    // orange  GX_RGB(27, 17, 9)
    const u8 bgColor        = 3;    // white (top of auto-generation)
    const u8 txtColor       = 4;    // 2nd and subsequent positions of auto-generation

    const int ch = NNS_G2dFontGetHeight(&gFont);
    const int cw = NNS_G2dFontGetMaxCharWidth(&gFont);
    const int dw = cw + CHAR_SPACE;

    const int l_px = (SCREEN_WIDTH - (giLine * dw - CHAR_SPACE)) / 2;
    const int l_py = SCREEN_HEIGHT - (ch + MARGIN);


    {
        // Extended display
        {
            NNSG2dGlyph g;

            NNS_G2dFontGetGlyphFromIndex(&g, &gFont, (u16)(giCenter));

            {
                // adjust so that the glyph comes to the center
            	const int cWidth = GetCharWidth(g.pWidths);
                const int e_pxc = MARGIN + g.pWidths->left + (cw - cWidth) / 2;
                const int bar_x = e_pxc - g.pWidths->left;
                const int bar_y = MARGIN + ch + 1;

                ClearAreaSafe(&gDCc, bgColor, 0, 0, (cw + MARGIN * 2), (ch + MARGIN * 2) );
                NNS_G2dCharCanvasDrawGlyph(&gDCc, &gFont, e_pxc, MARGIN, txtColor, &g);

                // Use NNS_G2dCharCanvasClearArea to draw a width line
                ClearAreaSafe(&gDCc, barColor, bar_x, bar_y, cWidth, 1);
            }
        }

        // display glyph list
        {
            int i;

            ClearAreaSafe(&gCc, bgColor, 0, (u16)(l_py - MARGIN), SCREEN_WIDTH, (u16)(ch + MARGIN * 2));

            // display a number of glyphs equal to giLine with giCenter as the center
            for( i = 0; i < giLine; ++i )
            {
                NNSG2dGlyph g;
                int gi = giCenter + i - giLine / 2;

                LIMIT_GLYPH_INDEX(gi);

                NNS_G2dFontGetGlyphFromIndex(&g, &gFont, (u16)(gi));

                {
                    // Correction x = line offset + character position + left space + relative character offset inside the font
                    const int pxc = l_px + dw * i + g.pWidths->left + (cw - GetCharWidth(g.pWidths)) / 2;

                    NNS_G2dCharCanvasDrawGlyph(&gCc, &gFont, pxc, l_py, txtColor, &g);
                }
            }

            // Use NNS_G2dCharCanvasClearArea to draw a frame
            {
                int border_x = l_px + dw * (giLine / 2) - 2;
                int border_y = l_py - 2;
                int border_w = cw + 2;
                int border_h = ch + 2;

                ClearAreaSafe(&gCc, borderColor, border_x,                  border_y,                   border_w + 2,   1);
                ClearAreaSafe(&gCc, borderColor, border_x,                  border_y + border_h + 1,    border_w + 2,   1);

                ClearAreaSafe(&gCc, borderColor, border_x,                  border_y + 1,               1,              border_h);
                ClearAreaSafe(&gCc, borderColor, border_x + border_w + 1,   border_y + 1,               1,              border_h);
            }
        }
    }

    // register VRAM transmission stack
    {
        (void)NNS_GfdRegisterNewVramTransferTask(
            NNS_GFD_DST_2D_BG1_CHAR_MAIN,
            sizeof(GXCharFmt256) * BG1_CANVAS_OFFSET,
            &gBG1OffScreen,
            sizeof(GXCharFmt256) * BG1_CANVAS_WIDTH * BG1_CANVAS_HEIGHT );

        (void)NNS_GfdRegisterNewVramTransferTask(
            NNS_GFD_DST_2D_BG2_CHAR_MAIN,
            sizeof(GXCharFmt256) * BG2_CANVAS_OFFSET,
            &gBG2OffScreen,
            sizeof(GXCharFmt256) * BG2_CANVAS_WIDTH * BG2_CANVAS_HEIGHT );
    }
}



/*---------------------------------------------------------------------------*
  Name:         PrintSampleInfo

  Description:  Displays the sample information to the lower screen.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintSampleInfo(void)
{
    const static char* encodingName[] =
    {
        "UTF-8",
        "UTF-16",
        "Shift_JIS",
        "CP1252",
    };
    NNSG2dGlyph g;
    int right;

    NNS_G2dFontGetGlyphFromIndex(&g, &gFont, (u16)(giCenter));
	right = GetCharWidth(g.pWidths) - (g.pWidths->left + g.pWidths->glyphWidth);

    DTX_PrintLine(
        "Simple Font Viewer Sample\n"
        "operation\n"
        "  left, right  move one character\n"
        "  up, down     move one line\n"
        "  X            change scale\n"
    );

    DTX_PrintLine("font file name:  %s", fontName);
    DTX_PrintLine("font encoding:   %s", encodingName[NNSi_G2dFontGetEncoding(&gFont)]);
    DTX_PrintLine("font cell size:  %5d x %5d", NNS_G2dFontGetCellWidth(&gFont), NNS_G2dFontGetCellHeight(&gFont));
    DTX_PrintLine("font bpp:        %5d", NNS_G2dFontGetBpp(&gFont));
    DTX_PrintLine("font baseline:   %5d", NNS_G2dFontGetBaselinePos(&gFont));
    DTX_PrintLine("font linefeed:   %5d", NNS_G2dFontGetLineFeed(&gFont));
    DTX_PrintLine("font alter char: %5d", NNS_G2dFontGetAlternateGlyphIndex(&gFont));
    DTX_PrintLine("glyph index/sum: %5d / %5d", giCenter, giMax);
    DTX_PrintLine("glyph widths:     %4d + %3d + %4d = %4d",
        g.pWidths->left, g.pWidths->glyphWidth, right, g.pWidths->charWidth);
    DTX_PrintLine("scale:              x%d", scale);
}



/*---------------------------------------------------------------------------*
  Name:         SampleInit

  Description:  Performs initializations for the display.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleInit(void)
{
    InitScreenCommon();
    InitEnumScreen();
    InitDoubleScaleScreen();

    // for off-screen transmission
    NNS_GfdInitVramTransferManager(gTransferTask, ARY_SIZEOF(gTransferTask));

    // initial display position
    giCenter = 0;
}



/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  Performs the rendering process for each frame.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleMain(void)
{
    static int iCenter = -1;

    // Render only when updated
    if( giCenter != iCenter )
    {
        iCenter = giCenter;
        PrintSampleInfo();
        UpdateDisplay();
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

        // display initialization
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
        static u32 repeat_count = 0;

        CMN_ReadGamePad();

        // change scale
        if( CMN_IsTrigger(PAD_BUTTON_X) )
        {
            scale ++;
            if( scale > 4 )
            {
                scale = 1;
            }
            UpdateScreenScale();
            PrintSampleInfo();
        }

        // change the displayed glyph
        if( CMN_IsPress(PAD_PLUS_KEY_MASK) )
        {
#define REPEAT(key)     ( (repeat_count >= REPEAT_THRESHOLD) ? CMN_IsPress(key): CMN_IsTrigger(key) )
            if( REPEAT(PAD_KEY_LEFT) )
            {
                giCenter--;
            }
            if( REPEAT(PAD_KEY_RIGHT) )
            {
                giCenter++;
            }
            if( REPEAT(PAD_KEY_UP) )
            {
                giCenter -= giLine;
            }
            if( REPEAT(PAD_KEY_DOWN) )
            {
                giCenter += giLine;
            }

            LIMIT_GLYPH_INDEX(giCenter);

            repeat_count++;
        }
        else
        {
            repeat_count = 0;
        }

        // Rendering
        SampleMain();

        CMN_WaitVBlankIntr();

        // Display the information output
        DTX_Reflect();

        // transmit the off-screen buffer
        NNS_GfdDoVramTransfer();
    }
}
