/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - Renderer_2LCD
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
//      This sample performs dual-screen display (the upper screen and the touch screen) using the renderer module.
//
//      A visibility culling function is set for the renderer in order to avoid unnecessary rendering of cells outside of each screen.
//
//  Using the Demo
//      START Button: Switches the cell output method between the object and the software sprite.
//                      
//      X Button: Resets the cell and surface positions.
//      +Control Pad: Moves the cell.
//      Y Button + Control Pad: Moves the main surface.
//      B Button + +Control Pad: Moves the subsurface.
//
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"

//------------------------------------------------------------------------------
// Constant Definitions
#define SCREEN_WIDTH                256                     // screen width
#define SCREEN_HEIGHT               192                     // screen height


#define INIT_OUTPUT_TYPE            NNS_G2D_SURFACETYPE_MAIN2D   // Initial value for output method

#define NUM_OF_OAM                  128                 // Number of OAMs allocated to OAM manager
#define NUM_OF_AFFINE               (NUM_OF_OAM / 4)    // Number of affine parameters allocated to OAM Manager

#define CHARA_BASE                  0x0                 // Character image base address
#define PLTT_BASE                   0x0                 // Palette base address

#define SURFACE_MAIN_INIT_POS ((NNSG2dFVec2){0 << FX32_SHIFT, 0 << FX32_SHIFT})
#define SURFACE_SUB_INIT_POS ((NNSG2dFVec2){0 << FX32_SHIFT, SCREEN_HEIGHT << FX32_SHIFT})

#define CELL_A_INIT_POS ((NNSG2dFVec2){150 << FX32_SHIFT, 120 << FX32_SHIFT})


static const NNSG2dFVec2 CELL_B_POS = {100 << FX32_SHIFT, 120 << FX32_SHIFT};
static const NNSG2dFVec2 CELL_C_POS = {100 << FX32_SHIFT, 300 << FX32_SHIFT};


//------------------------------------------------------------------------------
// Structure definitions


//------------------------------------------------------------------------------
// Global Variables

static NNSG2dImageProxy             sImageProxy;    // Character/texture proxy for cell
static NNSG2dImagePaletteProxy      sPaletteProxy;  // Palette proxy for cell

static NNSG2dOamManagerInstance     sMainOamMan;    // OAM Manager for main screen
static NNSG2dOamManagerInstance     sSubOamMan;     // OAM Manager for subscreen

static NNSG2dAnimBankData*          spAnimBank;     // Animation Bank
static NNSG2dCellAnimation*         spCellAnim;     // cell animation

//------------------------------------------------------------------------------
// Prototype Declarations



/*---------------------------------------------------------------------------*
  Name:         CallBackAddOam*

  Description:  Function called internally by NNS_G2dDraw* functions to add an OAM.
                It passes OAMs to the OAM Manager.

  Arguments:    pOam:           Pointer to OAM to be added.
                affineIndex:    Affine index used by this OAM.
                                May have a value of 32 or greater.
                                If affine is not used,
                                NNS_G2D_OAM_AFFINE_IDX_NONE is specified.
                bDoubleAffine:  Whether in double affine mode.

  Returns:      Returns TRUE if successful; returns FALSE otherwise.
 *---------------------------------------------------------------------------*/
// For main screen
static BOOL CallBackAddOamMain( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine )
{
#pragma unused( bDoubleAffine )
    SDK_NULL_ASSERT( pOam );

    return NNS_G2dEntryOamManagerOamWithAffineIdx( &sMainOamMan, pOam, affineIndex );
}

// For subscreen
static BOOL CallBackAddOamSub( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine )
{
#pragma unused( bDoubleAffine )
    SDK_NULL_ASSERT( pOam );

    return NNS_G2dEntryOamManagerOamWithAffineIdx( &sSubOamMan, pOam, affineIndex );
}

/*---------------------------------------------------------------------------*
  Name:         CallBackAddAffine*

  Description:  This function is called to add affine parameters in the
                NNS_G2dDraw* functions.
                It passes affine parameters to the OAM Manager.

  Arguments:    mtx:        Pointer to affine transform matrix to be added.

  Returns:      Returns the AffineIndex for referencing added affine parameters
                
 *---------------------------------------------------------------------------*/
// For main screen
static u16 CallBackAddAffineMain( const MtxFx22* mtx )
{
    SDK_NULL_ASSERT( mtx );
    return NNS_G2dEntryOamManagerAffine( &sMainOamMan, mtx );
}

// For subscreen
static u16 CallBackAddAffineSub( const MtxFx22* mtx )
{
    SDK_NULL_ASSERT( mtx );
    return NNS_G2dEntryOamManagerAffine( &sSubOamMan, mtx );
}

/*---------------------------------------------------------------------------*
  Name:         CallBackCulling

  Description:  A function called internally by NNS_G2dDraw* functions
                NNS_G2dDraw* functions.
                Using the radius of the bounding sphere included in the cell data,
                evaluates whether the cell is within the surface.
                
                Caution:
                From the bounding sphere calculation algorithm, the radius of the cell bounding sphere used with the sample is approximately 32 (approximately 64 with diameter),
                which is significantly larger than the actual display size. (In fact, it seems to be 36.)
                Therefore, the visibility culling process determines it as visible, and it is wrapped around to the other side of the screen for the display.

                There are various possible countermeasures.

                A. Implements the special culling function that takes the material-specific conditions into consideration.
                B. Perform culling by using the cell bounding rectangle information. (For details, see the converter manual.)
                C. Performs culling for each OBJ.

                This sample does the following.
                A. Implements the special culling function that takes the material-specific conditions into consideration.
                And then,
                
                Assumes that all cells have objects placed from the cell origin in the positive direction on the y-axis.
                Whereas normally processes that would require
                const fx32  minY = py - R;
                const fx32  maxY = py + R;
                now,
                the following,
                const fx32  minY = py - R;
                const fx32  maxY = py;
                are used.


  Arguments:    pCell:      Pointer to information on the cell to be evaluated.
                pMtx:       Coordinate transform matrix applied to the cell to be evaluated.
                pViewRect:  View rectangle for output surface.

  Returns:      TRUE if cell is to be displayed; FALSE otherwise.
 *---------------------------------------------------------------------------*/
static BOOL CallBackCulling(
                const NNSG2dCellData*   pCell,
                const MtxFx32*          pMtx,
                const NNSG2dViewRect*   pViewRect)
{
    // Get radius of bounding sphere calculated by converter
    const fx32  R = NNSi_G2dGetCellBoundingSphereR( pCell ) * FX32_ONE;

    // Find the origin of the cell's display position
    const fx32  px = pMtx->_20 - pViewRect->posTopLeft.x;
    const fx32  py = pMtx->_21 - pViewRect->posTopLeft.y;

    // Obtain the rectangle that encompasses the cell bounding sphere
    // Note: this is assuming that all cells have OBJs placed from the cell origin in the positive direction on the Y-axis.
#if 1
    const fx32  minY = py - R;
    const fx32  maxY = py;
#else
    const fx32  minY = py - R;
    const fx32  maxY = py + R;              
#endif 

    const fx32  minX = px - R;
    const fx32  maxX = px + R;


    if( (maxY > 0) && (minY < pViewRect->sizeView.y) )
    {
        if( (maxX > 0) && (minX < pViewRect->sizeView.x) )
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         InitOamManager

  Description:  This function carries out the initialization of the OAM Manager.

  Arguments:    pOamManager:    Pointer to the OAM manager to be initialized.
                type:           Specifies which graphics engine the initialization will target.
                                

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitOamManager(NNSG2dOamManagerInstance* pOamManager, NNSG2dOamType type)
{
    BOOL result;
    SDK_NULL_ASSERT( pOamManager );

    result = NNS_G2dGetNewManagerInstance( pOamManager, 0, NUM_OF_OAM - 1, type );
    SDK_ASSERT( result );
    result = NNS_G2dInitManagerInstanceAffine( pOamManager, 0, NUM_OF_AFFINE - 1 );
    SDK_ASSERT( result );
}

/*---------------------------------------------------------------------------*
  Name:         InitRenderer

  Description:  Initializes Renderer and Surface.

  Arguments:    pRenderer:      Pointer to Renderer to be initialized.
                pMainSurface:   Pointer to the main screen surface initialized.
                pSubSurface:    Pointer to the subscreen surface initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitRenderer(
                NNSG2dRendererInstance* pRenderer,
                NNSG2dRenderSurface*    pMainSurface,
                NNSG2dRenderSurface*    pSubSurface)
{
    SDK_NULL_ASSERT( pRenderer );
    SDK_NULL_ASSERT( pMainSurface );
    SDK_NULL_ASSERT( pSubSurface );

    NNS_G2dInitRenderSurface( pMainSurface );
    NNS_G2dInitRenderSurface( pSubSurface );

    // Initializes the main screen surface.
    {
        NNSG2dViewRect* pRect = &(pMainSurface->viewRect);

        // Display rectangle
        pRect->posTopLeft = SURFACE_MAIN_INIT_POS;
        pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
        pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

        // Callback Functions
        pMainSurface->pFuncOamRegister          = CallBackAddOamMain;
        pMainSurface->pFuncOamAffineRegister    = CallBackAddAffineMain;
        pMainSurface->pFuncVisibilityCulling    = CallBackCulling;

        // display destination
        pMainSurface->type                      = NNS_G2D_SURFACETYPE_MAIN2D;
    }

    // Initializes the subscreen surface
    {
        NNSG2dViewRect* pRect = &(pSubSurface->viewRect);

        // Display rectangle
        pRect->posTopLeft = SURFACE_SUB_INIT_POS;
        pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
        pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

        // Callback Functions
        pSubSurface->pFuncOamRegister           = CallBackAddOamSub;
        pSubSurface->pFuncOamAffineRegister     = CallBackAddAffineSub;
        pSubSurface->pFuncVisibilityCulling     = CallBackCulling;

        // display destination
        pSubSurface->type                       = NNS_G2D_SURFACETYPE_SUB2D;
    }

    // Initializes the renderer.
    {
        NNS_G2dInitRenderer( pRenderer );

        // Surface registration
        NNS_G2dAddRendererTargetSurface( pRenderer, pMainSurface );
        NNS_G2dAddRendererTargetSurface( pRenderer, pSubSurface );

        // Proxy registration
        NNS_G2dSetRendererImageProxy( pRenderer, &sImageProxy, &sPaletteProxy );

        // Specify the Z offset increment
        NNS_G2dSetRendererSpriteZoffset( pRenderer, - FX32_ONE );
    }
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Loads the animation data, cell bank, character data and palette data from a file, and then loads the character and palette data into VRAM.
                
                

  Arguments:    None.

  Returns:      Pointer to the loaded cell bank.
 *---------------------------------------------------------------------------*/
static NNSG2dCellDataBank* LoadResources(void)
{
    NNSG2dCellDataBank* pCellBank;
    void* pBuf;

    NNS_G2dInitImageProxy( &sImageProxy );
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );

    //------------------------------------------------------------------------------
    // load cell data
    {
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/Renderer_2LCD.NCER" );
        SDK_NULL_ASSERT( pBuf );
        // Because the cell bank is used in main memory until the end,
        // this pBuf is not freed.
    }

    //------------------------------------------------------------------------------
    // load animation data
    {
        // This pBuf is not deallocated because cell animation data is used in main memory until the end.
        // 
        pBuf = G2DDemo_LoadNANR( &spAnimBank, "data/Renderer_2LCD.NANR" );
        SDK_NULL_ASSERT( pBuf );

        //
        // Initialize the cell animation instance
        //
        {
            spCellAnim = G2DDemo_GetNewCellAnimation(1);
            SDK_NULL_ASSERT( spCellAnim );

            SDK_NULL_ASSERT( NNS_G2dGetAnimSequenceByIdx(spAnimBank, 0) );

            NNS_G2dInitCellAnimation(
                spCellAnim,
                NNS_G2dGetAnimSequenceByIdx(spAnimBank, 0),
                pCellBank );
        }
    }

    //------------------------------------------------------------------------------
    // load character data for 2D
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/Renderer_2LCD.NCGR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For Main 2D Graphics Engine.
        NNS_G2dLoadImage2DMapping(
            pCharData,
            CHARA_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sImageProxy );

        // Loading For Sub 2D Graphics Engine.
        NNS_G2dLoadImage2DMapping(
            pCharData,
            CHARA_BASE,
            NNS_G2D_VRAM_TYPE_2DSUB,
            &sImageProxy );

        // Because character data was copied into VRAM,
        // this pBuf is freed. Same below.
        G2DDemo_Free( pBuf );
    }

    // load character data for 3D (software sprite)
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCBR( &pCharData, "data/Renderer_2LCD.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 3D Graphics Engine.
        NNS_G2dLoadImage2DMapping(
            pCharData,
            CHARA_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sImageProxy );

        G2DDemo_Free( pBuf );
    }

    //------------------------------------------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/Renderer_2LCD.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For Main 2D Graphics Engine.
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sPaletteProxy);

        // Loading For Sub 2D Graphics Engine.
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_2DSUB,
            &sPaletteProxy);

        // Loading For 3D Graphics Engine.
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sPaletteProxy);

        G2DDemo_Free( pBuf );
    }

    return pCellBank;
}


/*---------------------------------------------------------------------------*
  Name:         ProcessInput

  Description:  Performs processing according to key input.

  Arguments:    pCellPos:       Pointer to the cell position.
                pOutputType:    Pointer to a buffer where the output method information is held.
                pMainPos:       Pointer to the subscreen surface position.
                pSubPos:        Pointer to the main screen surface position.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ProcessInput(NNSG2dFVec2* pCellPos,
                            NNSG2dSurfaceType* pOutputType,
                            NNSG2dFVec2* pMainPos,
                            NNSG2dFVec2* pSubPos)
{
    int dx = 0;
    int dy = 0;
    SDK_NULL_ASSERT( pCellPos );
    SDK_NULL_ASSERT( pOutputType );

    G2DDemo_ReadGamePad();

    // Obtains the +Control Pad input.
    {
        if ( G2DDEMO_IS_PRESS(PAD_KEY_UP) )
        {
            dy--;
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_DOWN) )
        {
            dy++;
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
        {
            dx--;
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
        {
            dx++;
        }
    }

    if( G2DDEMO_IS_PRESS(PAD_BUTTON_Y) )
    {
        // Moves main screen surface
        pMainPos->x += (dx << FX32_SHIFT);
        pMainPos->y += (dy << FX32_SHIFT);
    }
    else if( G2DDEMO_IS_PRESS(PAD_BUTTON_B) )
    {
        // Move subscreen surface
        pSubPos->x += (dx << FX32_SHIFT);
        pSubPos->y += (dy << FX32_SHIFT);
    }
    else
    {
        // Move cell
        pCellPos->x += (dx << FX32_SHIFT);
        pCellPos->y += (dy << FX32_SHIFT);
    }

    // Reset
    if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_X) )
    {
        *pMainPos = SURFACE_MAIN_INIT_POS;
        *pSubPos  = SURFACE_SUB_INIT_POS;
        *pCellPos = CELL_A_INIT_POS;
    }

    // Toggle Output Type
    if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_START) )
    {
        *pOutputType = (*pOutputType == NNS_G2D_SURFACETYPE_MAIN2D) ?
                            NNS_G2D_SURFACETYPE_MAIN3D:
                            NNS_G2D_SURFACETYPE_MAIN2D;
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintSurfaceRect

  Description:  Display the surface position information.

  Arguments:    x:          display position x.
                y:          display position y.
                pSurface:   Pointer to surface to be displayed.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintSurfaceRect(u8 x, u8 y, const NNSG2dRenderSurface* pSurface)
{
    const NNSG2dViewRect* pRect = &(pSurface->viewRect);

    G2DDemo_PrintOutf(x, y, "(%3d, %3d)-(%3d, %3d)",
        pRect->posTopLeft.x >> FX32_SHIFT,
        pRect->posTopLeft.y >> FX32_SHIFT,
        (pRect->posTopLeft.x + pRect->sizeView.x - FX32_ONE) >> FX32_SHIFT,
        (pRect->posTopLeft.y + pRect->sizeView.y - FX32_ONE) >> FX32_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    NNSG2dRendererInstance  render;         // Renderer for rendering
    NNSG2dRenderSurface     mainSurface;    // Main screen surface
    NNSG2dRenderSurface     subSurface;     // subscreen surface
    NNSG2dCellDataBank*     pCellBank;      // Cell data


    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();

        NNS_G2dInitOamManagerModule();
        InitOamManager( &sMainOamMan, NNS_G2D_OAMTYPE_MAIN );
        InitOamManager( &sSubOamMan, NNS_G2D_OAMTYPE_SUB );
        InitRenderer( &render, &mainSurface, &subSurface );
        pCellBank = LoadResources();
    }

    // start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    // Main loop
    while( TRUE )
    {
        static NNSG2dFVec2 cellAPos = CELL_A_INIT_POS;  // Cell A drawing position


        // Read input and update cell and surface positions and main screen output method.
        {
            ProcessInput(
                &cellAPos,
                &(mainSurface.type),
                &(mainSurface.viewRect.posTopLeft),
                &(subSurface.viewRect.posTopLeft));
        }

        // Render
        //
        // Render cell using Renderer module
        //
        {
            NNS_G2dBeginRendering( &render );
                // Cell A: Default displays in main screen. Can be moved using key.
                NNS_G2dPushMtx();
                    NNS_G2dTranslate( cellAPos.x, cellAPos.y, 0 );
                    NNS_G2dDrawCellAnimation( spCellAnim );
                NNS_G2dPopMtx();

                // Cell B: Default displays in main screen. Position fixed.
                NNS_G2dPushMtx();
                    NNS_G2dTranslate( CELL_B_POS.x, CELL_B_POS.y, 0 );
                    NNS_G2dDrawCellAnimation( spCellAnim );
                NNS_G2dPopMtx();

                // Cell C: Default displays in subscreen. Position fixed.
                NNS_G2dPushMtx();
                    NNS_G2dTranslate( CELL_C_POS.x, CELL_C_POS.y, 0 );
                    NNS_G2dDrawCellAnimation( spCellAnim );
                NNS_G2dPopMtx();
            NNS_G2dEndRendering();
        }

        // Output display information
        {
            G2DDemo_PrintOutf(0, 0, "cell pos: (%3d, %3d)",
                cellAPos.x >> FX32_SHIFT, cellAPos.y >> FX32_SHIFT);
            G2DDemo_PrintOut(0, 1, "main surface:");
            PrintSurfaceRect(1, 2, &mainSurface);
            G2DDemo_PrintOut(0, 3, "sub surface:");
            PrintSurfaceRect(1, 4, &subSurface);

            G2DDemo_PrintOut(0, 23, (mainSurface.type == NNS_G2D_SURFACETYPE_MAIN2D) ?
                   "OBJ           ": "SoftwareSprite");
        }

        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();


        //
        // Write buffer contents to HW
        //
        {
            // Display information
            G2DDemo_PrintApplyToHW();

            // Cell
            NNS_G2dApplyAndResetOamManagerBuffer( &sMainOamMan );
            NNS_G2dApplyAndResetOamManagerBuffer( &sSubOamMan );
        }

        NNS_G2dTickCellAnimation( spCellAnim, FX32_ONE );
    }
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Handles VBlank interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );                   // Checking VBlank interrupt
}


