/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - DrawTaggedText
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
//      This is a sample showing the use of NNS_G2dTextCanvasDrawTaggedText().
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

// color number for character string embedding
#define WHITE   "\x1"
#define BLACK   "\x2"
#define RED     "\x3"
#define GREEN   "\x4"
#define BLUE    "\x5"
#define CYAN    "\x6"
#define MAGENTA "\x7"
#define YELLOW  "\x8"


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont          gFont;          // Font
NNSG2dCharCanvas    gCanvas;        // CharCanvas
NNSG2dTextCanvas    gTextCanvas;    // TextCanvas




//****************************************************************************
// Initialize etc.
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         SimpleTagCallback

  Description:  NNSG2dTagCallback sample.

  Arguments:    c:      Character code for the reason the callback occurred.
                pInfo:  pointer to the render information structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SimpleTagCallback(u16 c, NNSG2dTagCallbackInfo* pInfo)
{
    if( c == 0x1B )
    {
        const char* pos = (const char*)pInfo->str;

        switch( *pos++ )
        {
        // 0x1B C 0xXX
        case 'C':   // Color
            {
                pInfo->clr = *(u8*)pos;
                pos++;
            }
            break;

        // 0x1B B
        case 'B':   // callBack
            {
                NNS_G2D_POINTER_ASSERT( pInfo->cbParam );

                pos++;  // alignment for NNS_G2D_UNICODE
                pInfo->str = (const NNSG2dChar*)pos;

                ((NNSG2dTagCallback)pInfo->cbParam)( c, pInfo );
            }
            return;

        // 0x1B H 0xXX
        case 'H':   // Horizontal
            {
                s8 hSpace;

                hSpace = *(s8*)pos;
                pos++;

                NNS_G2dTextCanvasSetHSpace(&(pInfo->txn), hSpace);
            }
            break;

        // 0x1B V 0xXX
        case 'V':   // Vertical
            {
                s8 vSpace;

                vSpace = *(s8*)pos;
                pos++;

                NNS_G2dTextCanvasSetVSpace(&(pInfo->txn), vSpace);
           }
            break;

        // 0x1B X 0xXX
        case 'X':   // X
            {
                pInfo->x += *(s8*)pos;
                pos++;
            }
            break;

        // 0x1B Y 0xXX
        case 'Y':   // Y
            {
                pInfo->y += *(s8*)pos;
                pos++;
            }
            break;

        default:
            OS_Warning("NNS_G2dTextCanvasDrawTaggedText: Unknown Tag: %c (0x%02X)\n", *(u8*)(pos-1), *(u8*)(pos-1));
            pos++;  // alignment for NNS_G2D_UNICODE
            break;
        }

        pInfo->str = (const NNSG2dChar*)pos;
    }
}



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
    TXT_LoadFont( &gFont, TXT_FONTRESOURCE_NAME );

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        pCharBase + cOffset,
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
        cOffset,
        TXT_CPALETTE_MAIN
    );
}




/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  This is the main process of the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define TAG_BEGIN       "\x1B"
#define CHG_COLOR(clr)  TAG_BEGIN  "C" clr
#define CHG_LFEED(h)    TAG_BEGIN  "L" #h
#define CHG_HSP(h)      TAG_BEGIN  "H" #h
#define CHG_VSP(v)      TAG_BEGIN  "V" #v
#define ADD_X(x)        TAG_BEGIN  "X" #x
#define ADD_Y(y)        TAG_BEGIN  "Y" #y

static void SampleMain(void)
{
    const static char text[] =
        "change color to " CHG_COLOR(RED) "red " CHG_COLOR(GREEN) "green " CHG_COLOR(CYAN) "cyan" CHG_COLOR(BLACK) "\n"
        CHG_HSP(\x5) "change hspace to 5" CHG_HSP(\x1) "\n"
        ADD_X(\x32) ADD_Y(\x32) "jump x+50 y+50";

    InitScreen();
    InitCanvas();

    // Clear background
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);

    // Draw tagged strings
    NNS_G2dTextCanvasDrawTaggedText(
        &gTextCanvas,           // TextCanvas to render
        0,                      // starting render coordinate
        0,                      // starting render coordinate
        TXT_COLOR_BLACK,        // Character color
        text,                   // Character string to render
        SimpleTagCallback,      // tag process callback
        NULL                    // parameter for tag process callback
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

