/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - CanvasCell
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
//      Displays a CharCanvas built for a 1D mapping OBJ as a cell.
// ============================================================================


#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/g2d_Renderer.h>
#include "g2d_textdemolib.h"

#define SCREEN_WIDTH        256
#define SCREEN_HEIGHT       192

#define CANVAS_WIDTH        6       // Width of character rendering area (in character units)
#define CANVAS_HEIGHT       4       // Height of character rendering area (in character units)
#define CANVAS_LEFT         13      // X position of character drawing area (in pixels)
#define CANVAS_TOP          17      // Y position of character drawing area (in pixels)

#define TEXT_HSPACE         1       // Amount of space between characters when rendering strings (in pixels)
#define TEXT_VSPACE         1       // Line spacing when rendering strings (in pixels)

#define CHARACTER_OFFSET    1       // Top character name of the character string to be used.
                                    //

#define MAX_OBJ_NUM         128
#define REPEAT_THRESHOLD    22

#define CHARACTER_WIDTH     8
#define CHARACTER_HEIGHT    8

#define NUM_OF_OAM          128                 // Number of OAMs allocated to OAM manager


//------------------------------------------------------------------------------
// Global Variables

NNSG2dFont                  gFont;          // Font
NNSG2dCharCanvas            gCanvas;        // CharCanvas
NNSG2dTextCanvas            gTextCanvas;    // TextCanvas

NNSG2dRendererInstance      gRenderer;
NNSG2dRenderSurface         gSurface;
NNSG2dImageProxy            gImageProxy;
NNSG2dImagePaletteProxy     gPaletteProxy;
NNSG2dOamManagerInstance    gOamManager;

NNSG2dCellData*             gpCanvasCell;



//****************************************************************************
// Renderer-related processing
//****************************************************************************
/*---------------------------------------------------------------------------*
  Name:         CallBackAddOam

  Description:  Function called internally by NNS_G2dDraw* functions to add an OAM.

  Arguments:    pOam:           Pointer to OAM to be added.
                affineIndex:    Affine index used by this OAM.
                                May have a value of 32 or greater.
                                If affine is not used,
                                NNS_G2D_OAM_AFFINE_IDX_NONE is specified.
                bDoubleAffine:  Whether in double affine mode.

  Returns:      Returns TRUE if successful; returns FALSE otherwise.
 *---------------------------------------------------------------------------*/
static BOOL CallBackAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine )
{
#pragma unused( bDoubleAffine )
    SDK_NULL_ASSERT( pOam );

    return NNS_G2dEntryOamManagerOamWithAffineIdx( &gOamManager, pOam, affineIndex );
}

/*---------------------------------------------------------------------------*
  Name:         CallBackAddAffine

  Description:  This function is called to add affine parameters in the
                NNS_G2dDraw* functions.

  Arguments:    mtx:        Pointer to affine transform matrix to be added.

  Returns:      Returns the AffineIndex for referencing added affine parameters
                
 *---------------------------------------------------------------------------*/
static u16 CallBackAddAffine( const MtxFx22* mtx )
{
    SDK_NULL_ASSERT( mtx );
    return NNS_G2dEntryOamManagerAffine( &gOamManager, mtx );
}


/*---------------------------------------------------------------------------*
  Name:         InitOamManager

  Description:  Initializes the OAM manager system and initializes a single OAM manager instance.
                

  Arguments:    pOamManager:    Pointer to the OAM manager to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitOamManager(NNSG2dOamManagerInstance*   pOamManager)
{
    BOOL result;
    SDK_NULL_ASSERT( pOamManager );

    NNS_G2dInitOamManagerModule();

    result = NNS_G2dGetNewOamManagerInstance(
                pOamManager,
                0, NUM_OF_OAM,
                0, NUM_OF_OAM/4,
                NNS_G2D_OAMTYPE_MAIN
    );
    SDK_ASSERT( result );
}

/*---------------------------------------------------------------------------*
  Name:         InitRenderer

  Description:  Initializes Renderer and Surface.

  Arguments:    pRenderer:  Pointer to Renderer to be initialized.
                pSurface:   Pointer to Surface to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitRenderer(
                NNSG2dRendererInstance*     pRenderer,
                NNSG2dRenderSurface*        pSurface,
                NNSG2dImageProxy*           pImageProxy,
                NNSG2dImagePaletteProxy*    pPaletteProxy
)
{
    NNSG2dViewRect* pRect;

    SDK_NULL_ASSERT( pImageProxy );
    SDK_NULL_ASSERT( pPaletteProxy );
    SDK_NULL_ASSERT( pRenderer );
    SDK_NULL_ASSERT( pSurface );

    //---- Initialize ImageProxy with a dummy address
    NNS_G2dInitImageProxy( pImageProxy );
    NNS_G2dSetImageLocation( pImageProxy, NNS_G2D_VRAM_TYPE_2DMAIN, NULL );

    //---- Initialize PaletteProxy with a dummy address
    NNS_G2dInitImagePaletteProxy( pPaletteProxy );
    NNS_G2dSetImagePaletteLocation( pPaletteProxy, NNS_G2D_VRAM_TYPE_2DMAIN, NULL );

    //---- Initialize the surface
    NNS_G2dInitRenderSurface( pSurface );
    pRect = &(pSurface->viewRect);

    pRect->posTopLeft.x = FX32_ONE * 0;
    pRect->posTopLeft.y = FX32_ONE * 0;
    pRect->sizeView.x   = FX32_ONE * SCREEN_WIDTH;
    pRect->sizeView.y   = FX32_ONE * SCREEN_HEIGHT;

    pSurface->pFuncOamRegister          = CallBackAddOam;
    pSurface->pFuncOamAffineRegister    = CallBackAddAffine;
    pSurface->type                      = NNS_G2D_SURFACETYPE_MAIN2D;

    //---- Initialize the renderer
    NNS_G2dInitRenderer( pRenderer );
    NNS_G2dAddRendererTargetSurface( pRenderer, pSurface );
    NNS_G2dSetRendererImageProxy( pRenderer, pImageProxy, pPaletteProxy );
}



//****************************************************************************
// Initialize etc.
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         InitScreen

  Description:  Initializes the OBJ display.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitScreen(void)
{
    // Mapping mode configuration  1D 256 KB
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_1D_256K);

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
    const NNSG2dOBJVramMode vramMode = NNS_G2D_OBJVRAMMODE_256K;

    // CharCanvas initialization
    NNS_G2dCharCanvasInitForOBJ1D(
        &gCanvas,
        pCharBase + (cOffset << vramMode),
            // Correction since the character name boundaries differ depending on the size of the VRAM that can be referenced

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

    // Build cell
    {
        //---- Set rotation center to center of CharCanvas
        const int x                 = CANVAS_WIDTH  * CHARACTER_WIDTH  / 2;
        const int y                 = CANVAS_HEIGHT * CHARACTER_HEIGHT / 2;
        const int priority          = 0;
        const GXOamMode mode        = GX_OAM_MODE_NORMAL;
        const BOOL mosaic           = FALSE;
        const GXOamEffect effect    = GX_OAM_EFFECT_AFFINE_DOUBLE;
        const GXOamColorMode color  = GX_OAM_COLORMODE_16;
        const int cParam            = TXT_CPALETTE_MAIN;
        const BOOL makeBR           = FALSE;

        const size_t szByte         = NNS_G2dCharCanvasCalcCellDataSize1D(&gCanvas, makeBR);

        //---- Allocate memory for cell data
        gpCanvasCell = (NNSG2dCellData*)TXT_Alloc(szByte);
        SDK_NULL_ASSERT( gpCanvasCell );

        NNS_G2dCharCanvasMakeCell1D(
            gpCanvasCell,
            &gCanvas,
            x,
            y,
            priority,
            mode,
            mosaic,
            effect,
            color,
            cOffset,
            cParam,
            vramMode,
            makeBR
        );
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
    // Clear the canvas
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_NULL);

    // character rendering
    NNS_G2dTextCanvasDrawText(&gTextCanvas, 1, 1, TXT_COLOR_BLACK, TXT_DRAWTEXT_FLAG_DEFAULT,
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz\n"
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
        "1D mapping OBJ CharCanvas Cell Sample\n"
        "operation\n"
        "  (none)\n"
    );
    DTX_PrintLine("canvas size character:  %3d x %3d", CANVAS_WIDTH, CANVAS_HEIGHT);
    DTX_PrintLine("canvas size pixel:      %3d x %3d", CANVAS_WIDTH * 8, CANVAS_HEIGHT * 8);
    DTX_PrintLine("bounding sphere radius: %3d", NNS_G2dGetCellBoundingSphereR(gpCanvasCell));
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

    InitOamManager( &gOamManager );
    InitRenderer( &gRenderer, &gSurface, &gImageProxy, &gPaletteProxy );

    InitScreen();
    InitCanvas();

    SampleDraw();

    PrintSampleInfo();
}



/*---------------------------------------------------------------------------*
  Name:         SampleMain

  Description:  This is the main process of the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SampleMain(void)
{
    static fx32 x       = SCREEN_WIDTH /2 * FX32_ONE;
    static fx32 y       = SCREEN_HEIGHT/2 * FX32_ONE;
    static fx32 scale   = FX32_ONE;
    static u16  rot     = 0;
    static fx32 vx      = FX32_ONE;
    static fx32 vy      = FX32_ONE;
    static fx32 vs      = FX32_ONE/120;
    static u16  vr      = 0x10000/0xFF;

    //---- Update cell display parameters
    {
        const int bsr = NNS_G2dGetCellBoundingSphereR( gpCanvasCell );

        x       += vx;
        y       += vy;
//      scale   += vs;
        rot     += vr;

        //---- Reflected at end of display
        if( (x < bsr * scale) || (SCREEN_WIDTH * FX32_ONE - bsr * scale < x) )
        {
            vx *= -1;
        }
        if( (y < bsr * scale) || (SCREEN_HEIGHT * FX32_ONE - bsr * scale < y) )
        {
            vy *= -1;
        }
        if( (scale < FX32_ONE/2) || (FX32_ONE * 2 < scale) )
        {
            vs *= -1;
        }
    }

    //---- Render the CharCanvas cell
    NNS_G2dBeginRendering( &gRenderer );
    {
        NNS_G2dPushMtx();
        {
            NNS_G2dTranslate( x, y, 0 );
            NNS_G2dRotZ( FX_SinIdx(rot), FX_CosIdx(rot) );
            NNS_G2dScale( scale, scale, FX32_ONE );

            NNS_G2dDrawCell( gpCanvasCell );
        }
        NNS_G2dPopMtx();
    }
    NNS_G2dEndRendering();
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

        // Writes cell data
        NNS_G2dApplyOamManagerToHW( &gOamManager );
        NNS_G2dResetOamManagerBuffer( &gOamManager );

        // Display the information output
        DTX_Reflect();
    }
}

