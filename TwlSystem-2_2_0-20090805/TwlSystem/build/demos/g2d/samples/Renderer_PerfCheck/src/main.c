/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - Renderer_PerfCheck
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
//      Compares the performance of the Renderer module.
//
//      This demo draws a multicell animation using the renderer and displays the time required to draw it.
//       
//
//      The compile switch USE_OPZ_API (specified value 0) is defined.
//       When USE_OPZ_API is defined as 1, the NNS_G2dBeginRenderingEx function, which supports optimized hints, is used.
//       
//       (However, builds are not possible with versions that do not support the NNS_G2dBeginRenderingEx function.)
//       
//
//      - Pushing the A Button switches to rendering with the NNS_G2dMakeSimpleMultiCellToOams function, enabling the fastest possible rendering.
//       
//       Software sprite rendering is not carried out during NNS_G2dMakeSimpleMultiCellToOams() rendering.
//       
//
// 
//  Using the Demo
//      Start Button: Switches the cell output method between OBJ and Software Sprite.
//                      
//      X Button: Resets scaling, movement, and rotation status of the cell.
//      +Control Pad: Moves the cell.
//      A Button: Switches between rendering that uses the renderer and rendering that uses the NNS_G2dMakeSimpleMultiCellToOams function.
//                      
//                      
//
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"

//------------------------------------------------------------------------------
// Constant Definitions
#define SCREEN_WIDTH                256                     // Screen width
#define SCREEN_HEIGHT               192                     // Screen height

#define CELL_INIT_POS_X             (120 << FX32_SHIFT)     // Initial x position of the cell
#define CELL_INIT_POS_Y             (120 << FX32_SHIFT)     // Initial y position of the cell

#define INIT_OUTPUT_TYPE            NNS_G2D_SURFACETYPE_MAIN2D   // Initial value for output method

#define NUM_OF_OAM                  128      // Number of OAMs allocated to OAM manager

#define CHARA_BASE                  0x0      // Character image base address
#define PLTT_BASE                   0x0      // Palette base address

//
// If this definition is set to 1, the new optimized API added from the 2004/11/10 version is used.
// (However, compiles are not possible with versions where optimized API is not supported.
//   Operations for the affine parameters are not reflected.)
//
#define USE_OPZ_API 1

//------------------------------------------------------------------------------
// Structure Definitions

typedef struct CellInfo
{
    u16          cellIdx;   // Number of cell to render
    u16          rotate;    // Rotation angle
    NNSG2dFVec2  scale;     // Scale
    NNSG2dFVec2  pos;       // Position of cell rendering
} CellInfo;


//------------------------------------------------------------------------------
// Global Variables

static NNSG2dImageProxy             sImageProxy;    // Character/texture proxy for cell
static NNSG2dImagePaletteProxy      sPaletteProxy;  // Palette proxy for cell

static NNSG2dOamManagerInstance*    spCurrentOam;   // Manager of OAM subject to callback processing

static NNSG2dAnimBankData*          spAnimBank;     // Animation Bank
static NNSG2dCellAnimation*         spCellAnim;     // Cell animation


//------------------------------------------------------------------------------
// Prototype Declarations

void VBlankIntr(void);
static void ProcessInput(CellInfo* pCellInfo, NNSG2dSurfaceType* pOutputType, u16 numCells);
static void MakeAffineMatrix(MtxFx22* pMtx, MtxFx22* pMtx2D, const CellInfo* pCellInfo);
static BOOL CallBackAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine );
static u16 CallBackAddAffine( const MtxFx22* mtx );
static void InitOamManager(NNSG2dOamManagerInstance*   pObjOamManager);
static void InitRenderer(
                NNSG2dRendererInstance* pRenderer,
                NNSG2dRenderSurface*    pSurface);


/*---------------------------------------------------------------------------*
  Name:         CallBackAddOam

  Description:  Function called internally by NNS_G2dDraw* functions to add an OAM.

  Arguments:    pOam:           Pointer to OAM to be added.
                affineIndex:    Affine index used by this OAM.
                                May have a value of 32 or greater.
                                If affine is not used,
                                specify NNS_G2D_OAM_AFFINE_IDX_NONE.
                bDoubleAffine:  Whether in double affine mode.

  Returns:      Returns TRUE if successful; returns FALSE otherwise.
 *---------------------------------------------------------------------------*/
static BOOL CallBackAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine )
{
#pragma unused( bDoubleAffine )
    SDK_NULL_ASSERT( spCurrentOam );
    SDK_NULL_ASSERT( pOam );

    return NNS_G2dEntryOamManagerOamWithAffineIdx( spCurrentOam, pOam, affineIndex );
}

/*---------------------------------------------------------------------------*
  Name:         CallBackAddAffine

  Description:  This function is called to add affine parameters in the
                NNS_G2dDraw* functions.

  Arguments:    mtx:        Pointer to affine transform matrix to be added.

  Returns:      Returns the AffineIndex for referencing added affine parameters.
                
 *---------------------------------------------------------------------------*/
static u16 CallBackAddAffine( const MtxFx22* mtx )
{
    SDK_NULL_ASSERT( spCurrentOam );
    SDK_NULL_ASSERT( mtx );
    return NNS_G2dEntryOamManagerAffine( spCurrentOam, mtx );
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

    result = NNS_G2dGetNewOamManagerInstanceAsFastTransferMode(
                pOamManager, 0, NUM_OF_OAM, NNS_G2D_OAMTYPE_MAIN );
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
                NNSG2dRendererInstance* pRenderer,
                NNSG2dRenderSurface*    pSurface)
{
    NNSG2dViewRect* pRect = &(pSurface->viewRect);
    SDK_NULL_ASSERT( pRenderer );
    SDK_NULL_ASSERT( pSurface );

    NNS_G2dInitRenderer( pRenderer );
    NNS_G2dInitRenderSurface( pSurface );
    

    pRect->posTopLeft.x = FX32_ONE * 0;
    pRect->posTopLeft.y = FX32_ONE * 0;
    pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
    pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

    pSurface->pFuncOamRegister       = CallBackAddOam;
    pSurface->pFuncOamAffineRegister = CallBackAddAffine;

    NNS_G2dAddRendererTargetSurface( pRenderer, pSurface );
    NNS_G2dSetRendererImageProxy( pRenderer, &sImageProxy, &sPaletteProxy );
    NNS_G2dSetRendererSpriteZoffset( pRenderer, -FX32_ONE >> 2 );

}


/*---------------------------------------------------------------------------*
  Name:         ProcessInput

  Description:  Changes the cell display status according to the key input.
                Also switches the displayed cell and resets its status.

  Arguments:    pCellInfo:      Pointer to CellInfo reflecting the key input
                pOutputType:    Pointer to a buffer where the output method information is held
                numCells:       Number of cells that can be selected

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ProcessInput(CellInfo* pCellInfo, NNSG2dSurfaceType* pOutputType, u16 numCells)
{
    SDK_NULL_ASSERT( pCellInfo );
    SDK_NULL_ASSERT( pOutputType );

    G2DDemo_ReadGamePad();

    // Change current Cell
    if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_A) )
    {
       pCellInfo->cellIdx++;
        if( pCellInfo->cellIdx >= numCells )
        {
            pCellInfo->cellIdx = 0;
        }
    }

    // Move cell
    if( ! G2DDEMO_IS_PRESS(PAD_BUTTON_B) && ! G2DDEMO_IS_PRESS(PAD_BUTTON_Y) )
    {
        if ( G2DDEMO_IS_PRESS(PAD_KEY_UP) )
        {
            pCellInfo->pos.y -= FX32_ONE;
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_DOWN) )
        {
            pCellInfo->pos.y += FX32_ONE;
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
        {
            pCellInfo->pos.x -= FX32_ONE;
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
        {
            pCellInfo->pos.x += FX32_ONE;
        }
    }

    // Reset
    if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_X) )
    {
        pCellInfo->rotate      = 0;
        pCellInfo->scale       = (NNSG2dFVec2){FX32_ONE, FX32_ONE};
        pCellInfo->pos         = (NNSG2dFVec2){CELL_INIT_POS_X, CELL_INIT_POS_Y};
    }

    // Toggle output type
    if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_START) )
    {
        *pOutputType = (*pOutputType == NNS_G2D_SURFACETYPE_MAIN2D) ?
                            NNS_G2D_SURFACETYPE_MAIN3D:
                            NNS_G2D_SURFACETYPE_MAIN2D;
    }
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  This function constructs a multicell animation and loads character and palette data.
                

  Arguments:    ppMCAnim:   Pointer that receives the pointer to the constructed multicell animation.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources(NNSG2dMultiCellAnimation** ppMCAnim)
{
    void* pBuf;
    
    NNS_G2dInitImageProxy( &sImageProxy );
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );
    
    //
    // Multicell-related Initialization
    //
    {
        NNSG2dCellDataBank*             pCellBank = NULL;   // Cell data
        NNSG2dCellAnimBankData*         pAnimBank = NULL;   // Cell animation
        NNSG2dMultiCellDataBank*        pMCBank = NULL;     // Multicell data
        NNSG2dMultiCellAnimBankData*    pMCABank = NULL;    // Multicell animation

        // Load the cell data, cell animation, multicell data and multicell animation.
        // 
        // They are used in main memory until the end, so pBuf is not released.
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/MultiCellAnimation.NCER" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNANR( &pAnimBank, "data/MultiCellAnimation.NANR" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNMCR( &pMCBank, "data/MultiCellAnimation.NMCR" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNMAR( &pMCABank, "data/MultiCellAnimation.NMAR" );
        SDK_NULL_ASSERT( pBuf );


        //
        // Initializes the multicell animation instances
        //
        {
            const NNSG2dMultiCellAnimSequence* pSequence;

            // Obtains the sequence to play
            pSequence = NNS_G2dGetAnimSequenceByIdx( pMCABank, 0 );
            SDK_ASSERT( pSequence );

            // Constructs the multicell animation
            *ppMCAnim = G2DDemo_GetNewMultiCellAnimation( pAnimBank,
                                                          pCellBank,
                                                          pMCBank,
                                                          NNS_G2D_MCTYPE_SHARE_CELLANIM );
            SDK_NULL_ASSERT( *ppMCAnim );


            // Sets the sequence to play to the multicell animation
            NNS_G2dSetAnimSequenceToMCAnimation( *ppMCAnim, pSequence);
        }
    }

    //
    // VRAM-related Initialization
    //
    {
        //------------------------------------------------------------------------------
        // Load character data for 2D
        {
            NNSG2dCharacterData* pCharData;

            pBuf = G2DDemo_LoadNCGR( &pCharData, "data/MultiCellAnimation.NCGR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading for 2D Graphics Engine
            NNS_G2dLoadImage2DMapping(
                pCharData,
                CHARA_BASE,
                NNS_G2D_VRAM_TYPE_2DMAIN,
                &sImageProxy );

            // Because character data was copied into VRAM,
            // this pBuf is freed. Same below.
            G2DDemo_Free( pBuf );
        }

        // Load character data for 3D (software sprite)
        {
            NNSG2dCharacterData* pCharData;

            pBuf = G2DDemo_LoadNCBR( &pCharData, "data/MultiCellAnimation.NCBR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading for 3D Graphics Engine.
            NNS_G2dLoadImage2DMapping(
                pCharData,
                CHARA_BASE,
                NNS_G2D_VRAM_TYPE_3DMAIN,
                &sImageProxy );

            G2DDemo_Free( pBuf );
        }
        
        //------------------------------------------------------------------------------
        // Load palette data
        {
            NNSG2dPaletteData* pPlttData;

            pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/MultiCellAnimation.NCLR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading for 2D Graphics Engine
            NNS_G2dLoadPalette(
                pPlttData,
                PLTT_BASE,
                NNS_G2D_VRAM_TYPE_2DMAIN,
                &sPaletteProxy);

            // Loading for 3D Graphics Engine.
            NNS_G2dLoadPalette(
                pPlttData,
                PLTT_BASE,
                NNS_G2D_VRAM_TYPE_3DMAIN,
                &sPaletteProxy);

            G2DDemo_Free( pBuf );
        }
    }
}

//------------------------------------------------------------------------------
// Renders multicells using the renderer module functions.
// 
static void DrawUsingRendererAPI
(
    NNSG2dRendererInstance*         pRender,       // Renderer for rendering
    const NNSG2dFVec2*              pPos,          // Position data
    const NNSG2dMultiCellAnimation* pMCAnim        // MultiCellAnimation instance
)
{
    //
    // When USE_OPZ_API is defined, optimized hints are provided.
    //  At this point, specify:
    // Do not use affine transformation.
    // Do not change parameters.  
    //            
#if USE_OPZ_API
    NNS_G2dBeginRenderingEx( pRender, NNS_G2D_RDR_OPZHINT_NOT_SR | 
                                      NNS_G2D_RDR_OPZHINT_LOCK_PARAMS );
#else// USE_OPZ_API
    NNS_G2dBeginRendering( pRender );
#endif//USE_OPZ_API

        NNS_G2dPushMtx();

            NNS_G2dTranslate( pPos->x, pPos->y, 0 );
            NNS_G2dDrawMultiCellAnimation( pMCAnim );

        NNS_G2dPopMtx();
    NNS_G2dEndRendering();
}

//------------------------------------------------------------------------------
// Renders multicells using the NNS_G2dMakeSimpleMultiCellToOams function.
// 
static void DrawUsingSimpleAPI
(
    NNSG2dOamManagerInstance*       pOamManager,   // OAM manager for 2D rendering
    const NNSG2dFVec2*              pPos,          // Position data
    const NNSG2dMultiCellAnimation* pMCAnim        // MultiCellAnimation instance
)
{
    static GXOamAttr    temp[NUM_OF_OAM];   
    u16                 numOamDrawn = 0;           // Number of OAMs to be rendered
    //
    // Writes an OBJ list equivalent to multicell to a temporary buffer.
    //
    numOamDrawn = NNS_G2dMakeSimpleMultiCellToOams(
                        temp,                           // Output Oam buffer
                        NUM_OF_OAM,                     // Output buffer length
                        &pMCAnim->multiCellInstance,    // Output multicell
                        NULL,                           // Affine conversion
                        pPos,                           // Offset position
                        NULL,                           // Affine index
                        FALSE );                        // Double affine?

    SDK_ASSERT( numOamDrawn < NUM_OF_OAM );

    // Registers OBJ list to OAM Manager
    (void)NNS_G2dEntryOamManagerOam(
            pOamManager,
            temp,
            numOamDrawn );
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    // Renderer for rendering
    NNSG2dRendererInstance      render;         
    // Main screen surface
    NNSG2dRenderSurface         surface;        
    // OAM manager for 2D rendering
    NNSG2dOamManagerInstance    oamManager;     
    // Type of rendering method
    NNSG2dSurfaceType           outputType = INIT_OUTPUT_TYPE;
    // multicell animation instance
    NNSG2dMultiCellAnimation*   pMCAnim = NULL; 
    
    //------------------------------------------
    // Initialize animation
    {
        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();
        InitOamManager( &oamManager );
        InitRenderer( &render, &surface );
        LoadResources( &pMCAnim );
        
        spCurrentOam = &oamManager;
    }

    // Start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    // Main loop
    while( TRUE )
    {
        // Draw using a renderer?
        static BOOL     bUsingRenderer = TRUE;
        // Time required to draw
        static OSTick   timeDraw       = 0;
        // Cell information regarding location, etc.
        static CellInfo cellInfo = {
            0,                                  // Number of cell to render
            0,                                  // Rotation angle
            {FX32_ONE, FX32_ONE},               // Scale
            {CELL_INIT_POS_X, CELL_INIT_POS_Y}  // Position of cell rendering
        };

        //------------------------------------------
        // Update display information and output method for cell for which input is read
        {
            ProcessInput(&cellInfo, &outputType, 0);
            surface.type = outputType;
            
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_A) )
            {
                bUsingRenderer = bUsingRenderer^TRUE;
            }
        }
        

        //------------------------------------------
        // Render
        // Calculate the time required for rendering
        {
            timeDraw = OS_GetTick();
            
            // Use the renderer?
            if( bUsingRenderer )
            {
                DrawUsingRendererAPI( &render, &cellInfo.pos, pMCAnim );
            }else{
                //
                // Not processed if software sprite rendering.
                //
                if( outputType != NNS_G2D_SURFACETYPE_MAIN3D )
                {
                    DrawUsingSimpleAPI( &oamManager, &cellInfo.pos, pMCAnim );
                }
            }
                
            timeDraw = OS_GetTick() - timeDraw;
        }
        
        
        //------------------------------------------
        // Output display information
        {
            // Top screen
            G2DDemo_PrintOutf(0, 0, "pos:   x=%3d     y=%3d",
                cellInfo.pos.x >> FX32_SHIFT, cellInfo.pos.y >> FX32_SHIFT);
            
            // Bottom screen
            G2DDemo_PrintOutf( 0, 21, "TIME-draw  :%06ld usec\n", OS_TicksToMicroSeconds(timeDraw) );
            G2DDemo_PrintOut(0, 22, (outputType == NNS_G2D_SURFACETYPE_MAIN2D) ?
                   "OBJ           ": "SoftwareSprite");
            G2DDemo_PrintOut(0, 23, (bUsingRenderer) ?
                   "UsingRenderer           ": "UsingMakeCellToOams");
            
            // Software sprite rendering is not carried out during NNS_G2dMakeSimpleMultiCellToOams() rendering.
            if( outputType != NNS_G2D_SURFACETYPE_MAIN2D && !bUsingRenderer )
            {
                G2DDemo_PrintOutf( 5, 10, "Not supported." );
            }else{
                G2DDemo_PrintOutf( 5, 10, "              " );
            }
        }
        
        //------------------------------------------
        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();

        //------------------------------------------
        //
        // Write buffer contents to HW
        //
        {
            // Display information
            G2DDemo_PrintApplyToHW();

            // Cell
            NNS_G2dApplyOamManagerToHW( &oamManager );
            NNS_G2dResetOamManagerBuffer( &oamManager );
        }
        
        //------------------------------------------
        //
        // Update animation
        //
        NNS_G2dTickMCAnimation( pMCAnim, FX32_ONE );
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


