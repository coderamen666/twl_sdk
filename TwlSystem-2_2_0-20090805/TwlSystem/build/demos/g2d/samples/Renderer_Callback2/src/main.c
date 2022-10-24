/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - Renderer_Callback2
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
//      This sample performs visual culling at the object level, using the renderer module's pre-object rendering callback.
//      
//      
//      The cell of a large pencil covering two screens is displayed.
//      The sample demo provides a debug display of the time required for processing by NNS_G2dDrawCellAnimation().
//
//  Using the Demo
//      START Button: Culling ON,OFF
//      X Button: Resets the cell and surface positions.
//      +Control Pad: Moves the cell.
//      Y Button + Control Pad: Moves the main surface.
//      A Button: Turns ON/OFF the cell rotation.
//
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


//------------------------------------------------------------------------------
// Constant Definitions
#define SCREEN_WIDTH        256                 // screen width
#define SCREEN_HEIGHT       192                 // screen height

#define NUM_OF_OAM          128                 // Number of OAMs allocated to OAM manager
#define NUM_OF_AFFINE       (NUM_OF_OAM / 4)    // Number of affine parameters allocated to OAM Manager

#define CHARA_BASE          0x0                 // Character image base address
#define PLTT_BASE           0x0                 // Palette base address

#define INIT_OUTPUT_TYPE    NNS_G2D_SURFACETYPE_MAIN2D   // Initial value for output method

// Initial position of surface
#define SURFACE_MAIN_INIT_POS ((NNSG2dFVec2){0 << FX32_SHIFT, 0 << FX32_SHIFT})             
// Initial position of surface
#define SURFACE_SUB_INIT_POS ((NNSG2dFVec2){0 << FX32_SHIFT, SCREEN_HEIGHT << FX32_SHIFT})  
// Initial position of cell
#define CELL_INIT_POS ((NNSG2dFVec2){150 << FX32_SHIFT, 120 << FX32_SHIFT})   



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

//------------------------------------------------------------------------------
// Determines if rectangle is in view.
static BOOL IsRectInView_
( 
    const fx32  px,
    const fx32  py,
    const fx32 _min_x, 
    const fx32 _max_x,
    const fx32 _min_y,
    const fx32 _max_y,
    const MtxFx32*          pMtx,
    const NNSG2dViewRect*   pViewRect
)
{
        s32 max_x, min_x;
    s32 max_y, min_y;
    s32 temp;
    
    // Calculates maximum/minimum XY display coordinates after scaling conversion
    min_x = FX_Mul(_min_x, pMtx->_00) + FX_Mul(_min_y, pMtx->_10);  // upper left
    max_x = min_x;
    
    temp  = FX_Mul(_min_x, pMtx->_00) + FX_Mul(_max_y, pMtx->_10);  // lower left
    if      (temp < min_x) { min_x = temp; }
    else if (temp > max_x) { max_x = temp; }
    temp  = FX_Mul(_max_x, pMtx->_00) + FX_Mul(_min_y, pMtx->_10);  // upper right
    if      (temp < min_x) { min_x = temp; }
    else if (temp > max_x) { max_x = temp; }
    temp  = FX_Mul(_max_x, pMtx->_00) + FX_Mul(_max_y, pMtx->_10);  // lower right
    if      (temp < min_x) { min_x = temp; }
    else if (temp > max_x) { max_x = temp; }
    
    min_y = FX_Mul(_min_x, pMtx->_01) + FX_Mul(_min_y, pMtx->_11);  // upper left
    max_y = min_y;
    
    temp  = FX_Mul(_min_x, pMtx->_01) + FX_Mul(_max_y, pMtx->_11);  // lower left
    if      (temp < min_y) { min_y = temp; }
    else if (temp > max_y) { max_y = temp; }
    
    temp  = FX_Mul(_max_x, pMtx->_01) + FX_Mul(_min_y, pMtx->_11);  // upper right
    if      (temp < min_y) { min_y = temp; }
    else if (temp > max_y) { max_y = temp; }
    
    temp  = FX_Mul(_max_x, pMtx->_01) + FX_Mul(_max_y, pMtx->_11);  // lower right
    if      (temp < min_y) { min_y = temp; }
    else if (temp > max_y) { max_y = temp; }
    
    // Adds the display position origin of the cell
    min_x  += px;
    max_x  += px;
    min_y  += py;
    max_y  += py;
    
    // Determines inside/outside of surface
    if( (max_y > 0) && (min_y < pViewRect->sizeView.y) )
    {
        if( (max_x > 0) && (min_x < pViewRect->sizeView.x) )
        {
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------
// Implements the OBJ culling function as the callback prior to OBJ drawing.
// You must note that there will be a higher processing load for culling, because there are more objects compared to cells.
// 
static void OBJCullingFunc_
(
    struct NNSG2dRendererInstance*         pRend,
    struct NNSG2dRenderSurface*            pSurface,
    const  NNSG2dCellData*                 pCell,
    u16                                    oamIdx,
    const MtxFx32*                         pMtx
)
{
#pragma unused( pCell )
#pragma unused( oamIdx )
    {
        const GXOamAttr* pOam = &pRend->rendererCore.currentOam;
        const GXOamShape shape = NNS_G2dGetOAMSize( pOam ); 
        
        const fx32 minX = NNS_G2dRepeatXinCellSpace( (s16)pOam->x ) * FX32_ONE ;
        const fx32 maxX = ( NNS_G2dRepeatXinCellSpace( (s16)pOam->x ) + NNS_G2dGetOamSizeX( &shape ) ) * FX32_ONE;
        const fx32 minY = NNS_G2dRepeatYinCellSpace( (s16)pOam->y ) * FX32_ONE;
        const fx32 maxY = ( NNS_G2dRepeatYinCellSpace( (s16)pOam->y ) + NNS_G2dGetOamSizeY( &shape ) ) * FX32_ONE;
        
        const fx32  px = pMtx->_20 - pSurface->viewRect.posTopLeft.x;
        const fx32  py = pMtx->_21 - pSurface->viewRect.posTopLeft.y;
                    
        if( IsRectInView_( px, py, minX, maxX, minY, maxY, pMtx, &pSurface->viewRect ) )
        {
            pRend->rendererCore.bDrawEnable = TRUE;
        }else{
            pRend->rendererCore.bDrawEnable = FALSE;
        }
    }   
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
        
        // display destination
        pMainSurface->type                      = NNS_G2D_SURFACETYPE_MAIN2D;
        
        pMainSurface->pBeforeDrawOamBackFunc = OBJCullingFunc_;
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
        
        // display destination
        pSubSurface->type                       = NNS_G2D_SURFACETYPE_SUB2D;
        
        pSubSurface->pBeforeDrawOamBackFunc = OBJCullingFunc_;
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
  Name:         LoadResource

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
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/Rdr_CallBack2.NCER" );
        SDK_NULL_ASSERT( pBuf );
        // Because the cell bank is used in main memory until the end,
        // this pBuf is not freed.
    }

    //------------------------------------------------------------------------------
    // load animation data
    {
        // This pBuf is not deallocated because cell animation data is used in main memory until the end.
        // 
        pBuf = G2DDemo_LoadNANR( &spAnimBank, "data/Rdr_CallBack2.NANR" );
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

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/Rdr_CallBack2.NCGR" );
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

    //------------------------------------------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/Rdr_CallBack2.NCLR" );
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
        G2DDemo_Free( pBuf );
    }

    return pCellBank;
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


    // Initializing App.
    {
        G2DDemo_CommonInit();
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
        static NNSG2dFVec2 cellAPos = CELL_INIT_POS;  // Cell A drawing position
        static u16         priority = 0;
        static u16         plttNo   = 0;
        static BOOL        bRotate = FALSE;
        static u16         rot = 0x0;
        static OSTick      time;

        //------------------------------------------------------
        // Loads the pad input and updates the data
        {
            int dx = 0;
            int dy = 0;
            
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
                mainSurface.viewRect.posTopLeft.x += (dx << FX32_SHIFT);
                mainSurface.viewRect.posTopLeft.y += (dy << FX32_SHIFT);
            }
            else
            {
                // Move cell
                cellAPos.x += (dx << FX32_SHIFT);
                cellAPos.y += (dy << FX32_SHIFT);
            }

            // Reset
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_X) )
            {
                cellAPos = CELL_INIT_POS;
                mainSurface.viewRect.posTopLeft = SURFACE_MAIN_INIT_POS;
                subSurface.viewRect.posTopLeft  = SURFACE_SUB_INIT_POS;
            }
            
            // Rotation ON/OFF           
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_A) )
            {
                bRotate = bRotate^TRUE;
            }
            
            //
            // ON/OFF for OBJ unit visible culling.
            //
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_START) )
            {
                if( mainSurface.pBeforeDrawOamBackFunc == NULL )
                {
                    mainSurface.pBeforeDrawOamBackFunc  = OBJCullingFunc_;
                    subSurface.pBeforeDrawOamBackFunc   = OBJCullingFunc_;
                }else{
                    mainSurface.pBeforeDrawOamBackFunc  = NULL;
                    subSurface.pBeforeDrawOamBackFunc   = NULL;
                }
            }
        }
        //------------------------------------------------------
        // Render
        //
        // Render cell using Renderer module
        //
        {
            NNS_G2dBeginRendering( &render );
                    
                // Movement possible with cell key operation.
                NNS_G2dPushMtx();
                    NNS_G2dTranslate( cellAPos.x, cellAPos.y, 0 );
                    
                    if( bRotate )
                    {
                        NNS_G2dRotZ( FX_SinIdx( rot ), FX_CosIdx( rot ) );
                    }
                    
                    // Measures time required for drawing.
                    time = OS_GetTick();
                        NNS_G2dDrawCellAnimation( spCellAnim );
                    time = OS_GetTick() - time;
                    
                NNS_G2dPopMtx();
                
            NNS_G2dEndRendering();
        }
        //------------------------------------------------------
        // Output display information
        {
            G2DDemo_PrintOutf( 0, 0, "TimeForDraw  : %06ld usec\n", OS_TicksToMicroSeconds(time) );
            G2DDemo_PrintOutf( 0, 1, "OBJ-Culling  : %s", ( mainSurface.pBeforeDrawOamBackFunc != NULL ) ? "ON ":"OFF");
            G2DDemo_PrintOutf( 0, 2, "Rotate       : %s", ( bRotate ) ? "ON ":"OFF");
        }
        //------------------------------------------------------
        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();

        //------------------------------------------------------
        //
        // Write buffer contents to HW
        //
        {
            // Cell
            NNS_G2dApplyAndResetOamManagerBuffer( &sMainOamMan );
            NNS_G2dApplyAndResetOamManagerBuffer( &sSubOamMan );
            
            // Display information
            G2DDemo_PrintApplyToHW();
        }
        
        //------------------------------------------------------
        // scene update
        rot += 0x100;
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


