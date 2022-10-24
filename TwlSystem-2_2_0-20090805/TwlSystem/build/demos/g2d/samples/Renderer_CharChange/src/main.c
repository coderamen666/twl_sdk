/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - Renderer_CharChange
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
//      Sample of cell animation rendering using the Renderer module
//        - Switches the image proxy (2 proxies) and renders using the renderer.
//        - One of the image proxies will replace a portion of the image data loaded into VRAM.
//         
//        - To that end, data is created using the -cr/ option (to specify the character file conversion region) for g2dcvtr.exe (the g2d binary converter).
//         
//         
//
//      Refer to the conversion option information denoted in this project's make file as well as the resource data within the data/src folder. That should make the process easier to understand.        
//      
//      
//
//  Controls
//                      
//      A Button      Switches the image proxy to be used  
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

#define NUM_OF_OAM                  128                     // Number of OAMs allocated to OAM manager

#define CHARA_BASE                  0x0                     // Character image base address
#define PLTT_BASE                   0x0                     // Palette base address

#define APP_NUM_IMG_PROXY           2                       // Number of image proxies

//------------------------------------------------------------------------------
// Global Variables

static NNSG2dImageProxy             sImageProxy[APP_NUM_IMG_PROXY];    // Character/texture proxy for cell
static NNSG2dImagePaletteProxy      sPaletteProxy;                     // Palette proxy for cell

static NNSG2dOamManagerInstance     oamManager;     // OAM manager being used by the renderer

static NNSG2dAnimBankData*          spAnimBank;     // Animation Bank
static NNSG2dCellAnimation*         spCellAnim;     // Cell animation
static NNSG2dCellDataBank*          spCellBank;     // Cell data bank

//------------------------------------------------------------------------------
// Prototype Declarations



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

    return NNS_G2dEntryOamManagerOamWithAffineIdx( &oamManager, pOam, affineIndex );
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
    return NNS_G2dEntryOamManagerAffine( &oamManager, mtx );
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
static void InitRenderer
(
    NNSG2dRendererInstance* pRenderer,
    NNSG2dRenderSurface*    pSurface
)
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
    
    // Initialize callback
    pSurface->pFuncOamRegister       = CallBackAddOam;
    pSurface->pFuncOamAffineRegister = CallBackAddAffine;

    pSurface->type = NNS_G2D_SURFACETYPE_MAIN2D;
    
    // Set surface to be rendered
    NNS_G2dAddRendererTargetSurface( pRenderer, pSurface );
    
    // Set image proxy and palette proxy
    NNS_G2dSetRendererImageProxy( pRenderer, &sImageProxy[0], &sPaletteProxy );
}


//------------------------------------------------------------------------------
static u32 InitImageProxy
( 
    NNSG2dImageProxy* pImageProxy,
    const char*       pNcgrFileName,
    u32               characterBaseOffset
)
{
    SDK_NULL_ASSERT( pNcgrFileName );
    NNS_G2dInitImageProxy( pImageProxy );
    
    //------------------------------------------------------------------------------
    // Load character data for 2D
    {
        NNSG2dCharacterData* pCharData  = NULL;
        void*                pBuf       = NULL;
        
        pBuf = G2DDemo_LoadNCGR( &pCharData, pNcgrFileName );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 2D Graphics Engine
        NNS_G2dLoadImage2DMapping(
            pCharData,
            characterBaseOffset,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            pImageProxy );
            
        // Because character data was copied into VRAM,
        // this pBuf is freed.
        G2DDemo_Free( pBuf );
        
        return pCharData->szByte;
    }
}

/*---------------------------------------------------------------------------*
  Name:         OverWrriteVramImage_

  Description:  Partially overwrites character data

  Arguments:    None.

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
static void OverWrriteVramImage_( void* pNcgrFile )
{
    NNSG2dCharacterPosInfo* pCharPosInfo = NULL;
    
    if( NNS_G2dGetUnpackedCharacterPosInfo( pNcgrFile, &pCharPosInfo ) )
    {
    
    }
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Loads the animation data, cell bank, character data and palette data from a file, and then loads the character and palette data into VRAM.
                
                

  Arguments:    None.

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
static void LoadResources(void)
{
    void* pBuf;

    
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );

    //------------------------------------------------------------------------------
    // load cell data
    {
        pBuf = G2DDemo_LoadNCER( &spCellBank, "data/PartialCharCvt.NCER" );
        SDK_NULL_ASSERT( pBuf );
        // Because the cell bank is used in main memory until the end,
        // this pBuf is not freed.
    }

    //------------------------------------------------------------------------------
    // load animation data
    {
        // This pBuf is not deallocated because cell animation data is used in main memory until the end.
        // 
        pBuf = G2DDemo_LoadNANR( &spAnimBank, "data/PartialCharCvt.NANR" );
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
                NNS_G2dGetAnimSequenceByIdx( spAnimBank, 0), spCellBank );
        }
    }

    //------------------------------------------------------------------------------
    // Load character data for 2D
    {
        int i = 0;
        u32 szLoaded = 0;
        
        const char* ncgrFileNames [] = 
        {
            "data/PartialCharCvt0.NCGR",
            "data/PartialCharCvt0.NCGR"
        };
        
        for( i = 0 ; i < APP_NUM_IMG_PROXY; i++ )
        { 
            szLoaded += InitImageProxy( &sImageProxy[i], 
                                        ncgrFileNames[i], 
                                        CHARA_BASE + szLoaded );
        }
        
        //
        // This function partially overwrites the VRAM storage area managed by sImageProxy[1].
        //
        {
            const u32 addr = NNS_G2dGetImageLocation( &sImageProxy[1], NNS_G2D_VRAM_TYPE_2DMAIN );

            NNSG2dCharacterData*    pCharData       = NULL;
            NNSG2dCharacterPosInfo* pCharPosInfo    = NULL;
            void*                   pBuf            = NULL;
        
            pBuf = G2DDemo_LoadNCGREx( &pCharData, &pCharPosInfo, "data/PartialCharCvt1.NCGR" );
            SDK_NULL_ASSERT( pBuf );
            
            DC_FlushAll();
            {
                const int szChar = ( pCharData->pixelFmt == GX_TEXFMT_PLTT16 ) ? 32 : 64;
                const int offset = ( pCharPosInfo->srcPosY * pCharPosInfo->srcW ) * szChar;
                GX_LoadOBJ( (void*)pCharData->pRawData, addr + offset, pCharData->szByte );
            }
            
            // Because character data was copied into VRAM,
            // this pBuf is freed.
            G2DDemo_Free( pBuf );
        }
    }
    

    
    //------------------------------------------------------------------------------
    // Load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/PartialCharCvt.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 2D Graphics Engine
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sPaletteProxy);

        G2DDemo_Free( pBuf );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    NNSG2dRendererInstance      render;         // Renderer for rendering
    NNSG2dRenderSurface         surface;        // Main screen surface
    int                         imgProxyIdx = 0;
    
    // Initializes the application.
    {
        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();
        InitOamManager( &oamManager );
        InitRenderer( &render, &surface );
        LoadResources();
    }

    // Starts rendering.
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    // Main loop
    while( TRUE )
    {
        // Processes pad input.
        {
            G2DDemo_ReadGamePad();
            
            // Change the image proxy to be used using the A Button
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_A) )
            {
                imgProxyIdx++;
                imgProxyIdx &= (APP_NUM_IMG_PROXY - 1);
                
                NNS_G2dSetRendererImageProxy( &render, 
                                              &sImageProxy[imgProxyIdx], 
                                              &sPaletteProxy );
            }
        }
        

        // Render
        //
        // Render cell using Renderer module
        //
        {
            NNS_G2dBeginRendering( &render );
                NNS_G2dPushMtx();
                    NNS_G2dTranslate( 120 * FX32_ONE, 120 * FX32_ONE, 0 );
                    NNS_G2dDrawCellAnimation( spCellAnim );
                NNS_G2dPopMtx();
            NNS_G2dEndRendering();
        }

        // Output display information
        {
            G2DDemo_PrintOutf(0, 0, "ImageProxyIdx : %d", imgProxyIdx );
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
            NNS_G2dApplyOamManagerToHW( &oamManager );
            NNS_G2dResetOamManagerBuffer( &oamManager );
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


