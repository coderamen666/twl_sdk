/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - Entity
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
//      Displays cell animation using entities.
//      Toggle color palette to change character color.
//
//  Using the Demo
//      START Button: Switches the cell output method between the object and the software sprite.
//                      
//      A Button: Switches animation sequences.
//      X Button: Switches color palettes.
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>
#include <string.h>

#include "g2d_demolib.h"


#define SCREEN_WIDTH                256     // Screen width
#define SCREEN_HEIGHT               192     // Screen height

#define NUM_OF_OAM                 128      // Number of OAMs managed by OAM manager
#define NUM_OF_AFFINE               1       // Number of affine parameters managed by OAM manager
#define NUM_OF_ALT_PLT              4       // Number of palettes that can be used for palette replacement
#define PLT_SWAP_TARGET             8       // Number of the palette to be replaced



//------------------------------------------------------------------------------
// Global Variables

// OAM Manager
static NNSG2dOamManagerInstance         sOamManager;

//------------------------------------------------------------------------------
// Prototype Declarations

static inline u16 GetNumOfEntitySequence( NNSG2dEntity* pEntity );
static BOOL CallBackAddOam( const GXOamAttr* pOam, u16 affineIndex, BOOL bDoubleAffine );
static u16 CallBackAddAffine( const MtxFx22* mtx );
static void LoadEntity(
    NNSG2dEntity* pEntity,
    const char* pathbase
);
static void LoadResources(
                NNSG2dEntity*               pEntity,
                NNSG2dImageProxy*           pImgProxy,
                NNSG2dImagePaletteProxy*    pPltProxy);
static void InitializeOamManager(NNSG2dOamManagerInstance* pManager);
static void InitRenderer(
                NNSG2dRendererInstance*     pRenderer,
                NNSG2dRenderSurface*        pSurface,
                NNSG2dImageProxy*           pImgProxy,
                NNSG2dImagePaletteProxy*    pPltProxy);


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
    
    return NNS_G2dEntryOamManagerOamWithAffineIdx( &sOamManager, pOam, affineIndex );
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

    return NNS_G2dEntryOamManagerAffine( &sOamManager, mtx );
}


/*---------------------------------------------------------------------------*
  Name:         LoadEntity

  Description:  Reads resource file and builds an entity.

  Arguments:    pEntity          :    Pointer to entity that takes the entity that was read.
                pathbase:   Pointer to text string for entity that represents name of each resource file, excluding its extension.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadEntity(
    NNSG2dEntity* pEntity,
    const char* pathbase
){
    size_t length = strlen( pathbase );
    char*                   filepath;
    char*                   extPos;
    NNSG2dCellDataBank*     pCellBank = NULL;
    NNSG2dAnimBankData*     pAnimBank = NULL;
    NNSG2dEntityDataBank*   pEntityBank = NULL;
    SDK_NULL_ASSERT( pEntity );
    SDK_NULL_ASSERT( pathbase );


    filepath = (char*)G2DDemo_Alloc( length + 6 );
    SDK_NULL_ASSERT( filepath );

    extPos = filepath + length;
    (void)strcpy( filepath, pathbase );

    // load entity resource
    {
        void* pBuf;

        (void)strcpy( extPos, ".NCER" );
        pBuf = G2DDemo_LoadNCER( &pCellBank, filepath );
        SDK_NULL_ASSERT( pBuf );

        (void)strcpy( extPos, ".NANR" );
        pBuf = G2DDemo_LoadNANR( &pAnimBank, filepath );
        SDK_NULL_ASSERT( pBuf );

        (void)strcpy( extPos, ".NENR" );
        pBuf = G2DDemo_LoadNENR( &pEntityBank, filepath );
        SDK_NULL_ASSERT( pBuf );
    }

    // construct an entity
    {
        NNSG2dCellAnimation *pCellAnim = G2DDemo_GetNewCellAnimation(1);
        SDK_NULL_ASSERT( pCellAnim );

        NNS_G2dInitCellAnimation(
            pCellAnim,
            NNS_G2dGetAnimSequenceByIdx( pAnimBank, 0 ),
            pCellBank );

        NNS_G2dInitEntity(
            pEntity,
            pCellAnim,
            NNS_G2dGetEntityDataByIdx( pEntityBank, 0 ),
            pAnimBank );

        // The above processing alone does not properly initialize the entity animation sequence.
        // The initial value must be specified using NNS_G2dSetEntityCurrentAnimation.
        NNS_G2dSetEntityCurrentAnimation( pEntity, 0 );

        SDK_ASSERT( NNS_G2dIsEntityValid( pEntity ) );
    }

    G2DDemo_Free( filepath );
}



/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Reads the resource.

  Arguments:    pEntity:    Pointer to entity that takes the entity that was read.
                pImgProxy:  Image proxy that stores the character information that was read.
                pPltProxy:  Palette proxy that stores the palette information that was read.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources(
                NNSG2dEntity*               pEntity,
                NNSG2dImageProxy*           pImgProxy,
                NNSG2dImagePaletteProxy*    pPltProxy)
{
    void* pBuf;

    NNS_G2dInitImageProxy( pImgProxy );
    NNS_G2dInitImagePaletteProxy( pPltProxy );

    // load resources.

    //------------------------------------------------------------------------------
    // load entity
    {
        LoadEntity( pEntity, "data/Entity" );
    }

    //------------------------------------------------------------------------------
    // load character data for 2D
    {
        NNSG2dCharacterData*        pCharData = NULL;

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/Entity.NCGR" );
        SDK_NULL_ASSERT( pBuf );

        // For 2D Graphics Engine
        NNS_G2dLoadImage2DMapping( pCharData, 0, NNS_G2D_VRAM_TYPE_2DMAIN, pImgProxy );

        G2DDemo_Free( pBuf );
    }

    // load character data for 3D (for software sprite)
    {
        NNSG2dCharacterData*        pCharData = NULL;

        pBuf = G2DDemo_LoadNCBR( &pCharData, "data/Entity.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        // For 3D Graphics Engine
        NNS_G2dLoadImage2DMapping( pCharData, 0, NNS_G2D_VRAM_TYPE_3DMAIN, pImgProxy );

        G2DDemo_Free( pBuf );
    }


    //------------------------------------------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData*        pPltData = NULL;

        pBuf = G2DDemo_LoadNCLR( &pPltData, "data/Entity.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // For 3D Graphics Engine
        NNS_G2dLoadPalette( pPltData, 0, NNS_G2D_VRAM_TYPE_3DMAIN, pPltProxy );
        // For 2D Graphics Engine
        NNS_G2dLoadPalette( pPltData, 0, NNS_G2D_VRAM_TYPE_2DMAIN, pPltProxy );

        G2DDemo_Free( pBuf );
    }
}



/*---------------------------------------------------------------------------*
  Name:         InitRenderer

  Description:  Initializes Renderer and Surface.

  Arguments:    pRenderer:  Pointer to Renderer to be initialized.
                pSurface:   Pointer to surface initialized and specified for renderer.
                pImgProxy:  Pointer to ImageProxy specified for renderer.
                pPltProxy:  Pointer to PaletteProxy specified for renderer.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitRenderer(
                NNSG2dRendererInstance*     pRenderer,
                NNSG2dRenderSurface*        pSurface,
                NNSG2dImageProxy*           pImgProxy,
                NNSG2dImagePaletteProxy*    pPltProxy)
{
    NNSG2dViewRect* pRect = &(pSurface->viewRect);
    SDK_NULL_ASSERT( pRenderer );
    SDK_NULL_ASSERT( pSurface );
    SDK_NULL_ASSERT( pImgProxy );
    SDK_NULL_ASSERT( pPltProxy );

    NNS_G2dInitRenderer( pRenderer );
    NNS_G2dInitRenderSurface( pSurface );

	// Specify extent of surface
    pRect->posTopLeft.x = FX32_ONE * 0;
    pRect->posTopLeft.y = FX32_ONE * 0;
    pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
    pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

    pSurface->pFuncOamRegister       = CallBackAddOam;
    pSurface->pFuncOamAffineRegister = CallBackAddAffine;
    pSurface->type = NNS_G2D_SURFACETYPE_MAIN3D;

	// Specify for Renderer
    NNS_G2dAddRendererTargetSurface( pRenderer, pSurface );
    NNS_G2dSetRendererImageProxy( pRenderer, pImgProxy, pPltProxy );
}



/*---------------------------------------------------------------------------*
  Name:         GetNumOfEntitySequence

  Description:  Gets the number of sequences registered for the entity.

  Arguments:    pEntity:    Entity to be examined.

  Returns:      Number of sequences registered for the entity.
*---------------------------------------------------------------------------*/
static inline u16 GetNumOfEntitySequence( NNSG2dEntity* pEntity )
{
    return pEntity->pEntityData->animData.numAnimSequence;
}



/*---------------------------------------------------------------------------*
  Name:         InitializeOamManager

  Description:  Initializes the OAM manager.

  Arguments:    pManager:   OAM manager to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeOamManager(NNSG2dOamManagerInstance* pManager)
{
    BOOL    bSuccess;

    bSuccess = NNS_G2dGetNewManagerInstance( pManager, 0, NUM_OF_OAM - 1, NNS_G2D_OAMTYPE_MAIN );
    SDK_ASSERT( bSuccess );

    bSuccess = NNS_G2dInitManagerInstanceAffine( pManager, 0, NUM_OF_AFFINE - 1 );
    SDK_ASSERT( bSuccess );
}



/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    static NNSG2dEntity                 entity;     // Entity
    static NNSG2dRendererInstance       renderer;   // Renderer
    static NNSG2dRenderSurface          surface;    // Surface
    static NNSG2dImageProxy             imgProxy;   // ImageProxy
    static NNSG2dImagePaletteProxy      pltProxy;   // PaletteProxy
    static NNSG2dPaletteSwapTable       pltSwpTbl;  // Palette swap table

    // Initialize App.
    {
        u16 i;
        u16* pTbl = pltSwpTbl.paletteIndex;

        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();

        LoadResources( &entity, &imgProxy, &pltProxy );
        NNS_G2dInitOamManagerModule();
        InitializeOamManager( &sOamManager );
        InitRenderer( &renderer, &surface, &imgProxy, &pltProxy );

    	// Initialize palette swap table
        for( i = 0; i < NNS_G2D_NUM_COLOR_PALETTE; ++i )
        {
            pTbl[i] = i;
        }
        NNS_G2dSetEntityPaletteTable( &entity, &pltSwpTbl );
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
        static u16      rotate      = 0;
        static fx32     animSpeed   = FX32_ONE;
        static u16      animIdx    = 0;
        static u16      pltIdx     = 0;

        //
        // Key handling.
        //
        {
            G2DDemo_ReadGamePad();

            // Switch sequences
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_A ) )
            {
                animIdx++;
                if( animIdx >= GetNumOfEntitySequence( &entity ) )
                {
                    animIdx = 0;
                }
                NNS_G2dSetEntityCurrentAnimation( &entity, animIdx );
            }

            // Switch rendering mode
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_START ) )
            {
                if( surface.type == NNS_G2D_SURFACETYPE_MAIN2D )
                {
                    surface.type = NNS_G2D_SURFACETYPE_MAIN3D;
                }else{
                    surface.type = NNS_G2D_SURFACETYPE_MAIN2D;
                }
            }

            // Switch palettes
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_X ) )
            {
                pltIdx++;
                if( pltIdx > NUM_OF_ALT_PLT )
                {
                    pltIdx = 0;
                }

                if( pltIdx == 0 )
                {
                	// Do not replace
                    pltSwpTbl.paletteIndex[PLT_SWAP_TARGET] = PLT_SWAP_TARGET;
                }
                else
                {
                    pltSwpTbl.paletteIndex[PLT_SWAP_TARGET] = pltIdx;
                }
            }
        }

        //
        // Draw
        //
        {
            NNS_G2dBeginRendering( &renderer );
                NNS_G2dPushMtx();
                    NNS_G2dTranslate( FX32_ONE * 120, FX32_ONE * 100, 0 );
                    NNS_G2dDrawEntity( &entity );
                NNS_G2dPopMtx();
            NNS_G2dEndRendering();
        }

        // Output display status
        {
            G2DDemo_PrintOutf(0, 0, "sequence no.: %d", animIdx);
            G2DDemo_PrintOutf(0, 1, "palette swap: %d", pltIdx);

            G2DDemo_PrintOut(0, 23, (surface.type == NNS_G2D_SURFACETYPE_MAIN2D) ?
                                "OBJ           ": "SoftwareSprite");
        }

        // Reflect SoftwareSprite output
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);

        // Wait for V-blank
        SVC_WaitVBlankIntr();

        //
        // copy Oam datas from manager's buffer to 2D HW.
        //
        {
            // Reflect BG (display status string)
            G2DDemo_PrintApplyToHW();

            // Reflect OBJ
            NNS_G2dApplyOamManagerToHW( &sOamManager );
            NNS_G2dResetOamManagerBuffer( &sOamManager );
        }

        //
        // Tick animation
        //
        {
            NNS_G2dTickEntity( &entity, animSpeed );
        }
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


