/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - Renderer_Callback1
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
//      This sample performs special processes unique to each game using the renderer module's object rendering callback.
//      
//
//      The display priority and location of character OBJs to be displayed on the screen are manipulated by callbacks.
//      
//
//  Using the Demo
//      +Control Pad: Moves the cell.
//      B Button: Disables the callback when pressed.
//
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"

//------------------------------------------------------------------------------
// Constant Definitions
#define SCREEN_WIDTH                256                     // screen width
#define SCREEN_HEIGHT               192                     // screen height

#define AFFINE_IDX                  3                       // Affine Index for cell
#define CELL_ROTATE_UNIT            0xFF                    // Units of cell rotations
#define CELL_SCALE_UNIT             ((FX32_ONE >> 7) * 3 )  // Units of cell scaling
                                                            // The scaling value is ensured to be not near 0
#define CELL_INIT_POS_X             (120 << FX32_SHIFT)     // Initial X position of the cell
#define CELL_INIT_POS_Y             (120 << FX32_SHIFT)     // Initial Y position of the cell

#define INIT_OUTPUT_TYPE            NNS_G2D_SURFACETYPE_MAIN2D   // Initial value for output method

#define NUM_OF_OAM                  128                 // Number of OAMs allocated to OAM manager

#define CHARA_BASE                  0x0                 // Character image base address
#define PLTT_BASE                   0x0                 // Palette base address



//------------------------------------------------------------------------------
// Structure definitions

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
static NNSG2dCellAnimation*         spCellAnim;     // cell animation

static NNSG2dScreenData*            pScrDataBG1;    // Screen data for BG
static NNSG2dSVec2                  randomDiff;     // Amount of random movement of head

//------------------------------------------------------------------------------
// Prototype Declarations

void VBlankIntr(void);
static NNSG2dCellDataBank* LoadResources(void);
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
                                NNS_G2D_OAM_AFFINE_IDX_NONE is specified.
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

  Returns:      Returns the AffineIndex for referencing added affine parameters
                
 *---------------------------------------------------------------------------*/
static u16 CallBackAddAffine( const MtxFx22* mtx )
{
    SDK_NULL_ASSERT( spCurrentOam );
    SDK_NULL_ASSERT( mtx );
    return NNS_G2dEntryOamManagerAffine( spCurrentOam, mtx );
}


/*---------------------------------------------------------------------------*
  Name:         CallBackBeforeOBJ

  Description:  Renderer OBJ rendering callback:
                Registered as the callback called prior to renderer object rendering, this callback carries out hacking specific to each game title.
                
                Here the position of the head part of the cell is moved and the rendering priority of the lower body is updated.

  Arguments:    pRend               renderer
                pSurface            surface
                pCell               cell
                oamIdx              OAM number
                pMtx                matrix

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CallBackBeforeOBJ
(
    struct NNSG2dRendererInstance*         pRend,
    struct NNSG2dRenderSurface*            pSurface,
    const NNSG2dCellData*                  pCell,
    u16                                    oamIdx,
    const MtxFx32*                         pMtx
)
{
#pragma unused( pSurface ) 
#pragma unused( pCell ) 
#pragma unused( pMtx ) 
    GXOamAttr*    pTempOam = &pRend->rendererCore.currentOam;
    //
    // write parameter
    //
    {
        // 0 -- 3 express the head.      
        if( oamIdx <= 3 )
        {
            // Move the position
            pTempOam->x += randomDiff.x;                    
            pTempOam->y += randomDiff.y;
        }
        
        // 4 and 5 express the lower body and 6 the shadow.
        if( oamIdx >= 4 )
        {
            // drawing priority change
            pTempOam->priority = 0;
        }
    }
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
    pSurface->type = NNS_G2D_SURFACETYPE_MAIN2D;

    NNS_G2dAddRendererTargetSurface( pRenderer, pSurface );
    NNS_G2dSetRendererImageProxy( pRenderer, &sImageProxy, &sPaletteProxy );
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
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/Rdr_CallBack1.NCER" );
        SDK_NULL_ASSERT( pBuf );
        // Because the cell bank is used in main memory until the end,
        // this pBuf is not freed.
    }

    //------------------------------------------------------------------------------
    // load animation data
    {
        // This pBuf is not deallocated because cell animation data is used in main memory until the end.
        // 
        pBuf = G2DDemo_LoadNANR( &spAnimBank, "data/Rdr_CallBack1.NANR" );
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

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/Rdr_CallBack1_OBJ.NCGR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 2D Graphics Engine
        NNS_G2dLoadImage1DMapping(
            pCharData,
            CHARA_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sImageProxy );

        // Because character data was copied into VRAM,
        // this pBuf is freed. Same below.
        G2DDemo_Free( pBuf );
    }
    
    //------------------------------------------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/Rdr_CallBack1_OBJ.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 2D Graphics Engine
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sPaletteProxy);

        G2DDemo_Free( pBuf );
    }
    
    //------------------------------------------------------------------------------
    // load screen data
    {
        NNSG2dPaletteData*      pPltData;
        NNSG2dCharacterData*    pChrData;
        NNSG2dScreenData*       pScrData;
        void*   pBuf2;
        void*   pBuf3;

        pBuf  = G2DDemo_LoadNSCR( &pScrDataBG1, "data/Rdr_CallBack1_BG0.NSCR" );
        pBuf2 = G2DDemo_LoadNCLR( &pPltData, "data/Rdr_CallBack1_BG.NCLR" );
        pBuf3 = G2DDemo_LoadNCGR( &pChrData, "data/Rdr_CallBack1_BG.NCGR" );
        
        SDK_NULL_ASSERT( pBuf );
        SDK_NULL_ASSERT( pBuf2 );
        SDK_NULL_ASSERT( pBuf3 );

        NNS_G2dBGSetup(
            NNS_G2D_BGSELECT_MAIN1,
            pScrDataBG1,
            pChrData,
            pPltData,
            GX_BG_SCRBASE_0x0000,
            GX_BG_CHARBASE_0x08000
        );
        G2_SetBG1Priority(0);
        
        pBuf  = G2DDemo_LoadNSCR( &pScrData, "data/Rdr_CallBack1_BG1.NSCR" );
        NNS_G2dBGSetup(
            NNS_G2D_BGSELECT_MAIN2,
            pScrData,
            pChrData,
            pPltData,
            GX_BG_SCRBASE_0x1800,
            GX_BG_CHARBASE_0x10000
        );
        G2_SetBG2Priority(1);
        G2DDemo_Free( pBuf2 );
        G2DDemo_Free( pBuf3 );
        
    }
    
    return pCellBank;
}

//------------------------------------------------------------------------------
// Obtains the character number for the screen data of the screen position.
static u16 GetBG1ScreenCharNo( int posX, int posY )
{
    const u16*  pData = (const u16*)pScrDataBG1->rawData;
    
    posX %= pScrDataBG1->screenWidth;
    posY %= pScrDataBG1->screenHeight;
    
    pData += (posY >> 3) * (pScrDataBG1->screenWidth >> 3) + (posX >> 3);
    
    // Returns the character number.
    return (u16)(*pData & 0x3FF);
}

//------------------------------------------------------------------------------
// Is the lower body of the cell located on the road?
static BOOL IsOnLoad( int x, int y )
{
    const u16 bgNo = GetBG1ScreenCharNo( x, y );
    
    // Is this the character number expressing the road?
    if( bgNo == 0x20 || bgNo == 0x21 || bgNo == 0x40 || bgNo == 0x41 )
    {
        return TRUE;
    }else{
        return FALSE;
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
    NNSG2dCellDataBank*         pCellBank;      // Cell data
    NNSG2dOamManagerInstance    oamManager;     // OAM manager for 2D rendering
    NNSG2dSurfaceType           outputType = INIT_OUTPUT_TYPE;


    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        
        GX_SetVisiblePlane( GX_PLANEMASK_BG0 | 
                            GX_PLANEMASK_BG1 | 
                            GX_PLANEMASK_BG2 | 
                            GX_PLANEMASK_OBJ );
        
        InitOamManager( &oamManager );
        InitRenderer( &render, &surface );
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
        static CellInfo cellInfo = {
            0,                                  // Number of cell to render
            0,                                  // Rotation angle
            {FX32_ONE, FX32_ONE},               // Scale
            {CELL_INIT_POS_X, CELL_INIT_POS_Y}  // Position of cell rendering
        };

        //
        // Reads the pad input and updates the cell display information and output method
        // 
        {
            G2DDemo_ReadGamePad();    
            //
            // cell movement
            //
            {
                if ( G2DDEMO_IS_PRESS(PAD_KEY_UP) )
                {
                    cellInfo.pos.y -= FX32_ONE;
                }
                if ( G2DDEMO_IS_PRESS(PAD_KEY_DOWN) )
                {
                    cellInfo.pos.y += FX32_ONE;
                }
                if ( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
                {
                    cellInfo.pos.x -= FX32_ONE;
                }
                if ( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
                {
                    cellInfo.pos.x += FX32_ONE;
                }
            }
            //
            // ON/OFF for OBJ unit callback
            //
            if( !G2DDEMO_IS_PRESS(PAD_BUTTON_B) )
            {
                const int x = cellInfo.pos.x >> FX32_SHIFT;
                const int y = cellInfo.pos.y >> FX32_SHIFT;
                const int OBJ_SIZE = 16;
                
                // If one of the four corners of the cell is on the road, then...
                if( IsOnLoad( x           , y               )  ||
                    IsOnLoad( x + OBJ_SIZE, y               )  ||
                    IsOnLoad( x           , y - OBJ_SIZE    )  ||
                    IsOnLoad( x + 16      , y - OBJ_SIZE)           )
                {
                    surface.pBeforeDrawOamBackFunc = CallBackBeforeOBJ;
                }else{
                    surface.pBeforeDrawOamBackFunc = NULL;
                }
            }else{
                surface.pBeforeDrawOamBackFunc = NULL;
            }
        }

        // Render
        //
        // Render cell using Renderer module
        //
        {
            spCurrentOam = &oamManager;

            NNS_G2dBeginRendering( &render );
                NNS_G2dPushMtx();

                    NNS_G2dTranslate( cellInfo.pos.x, cellInfo.pos.y, 0 );
                    NNS_G2dRotZ( FX_SinIdx( cellInfo.rotate ), FX_CosIdx( cellInfo.rotate ) );
                    NNS_G2dScale( cellInfo.scale.x, cellInfo.scale.y, FX32_ONE );

                    NNS_G2dDrawCellAnimation( spCellAnim );

                NNS_G2dPopMtx();
            NNS_G2dEndRendering();
        }
        
        
        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();


        //
        // Write buffer contents to HW
        //
        {
            // Cell
            NNS_G2dApplyOamManagerToHW( &oamManager );
            NNS_G2dResetOamManagerBuffer( &oamManager );
        }
        
        // Update
        {
            //
            // Update of amount head shakes
            //
            {
                randomDiff.x = 0;
                randomDiff.y = 0;
                
                if( OS_GetTick() % 5 == 0 )
                {
                    randomDiff.x = 3;
                }
                
                if( OS_GetTick() % 7 == 0 )
                {
                    randomDiff.y = 3;
                }
            }
            //
            // cell animation update
            //
            NNS_G2dTickCellAnimation( spCellAnim, FX32_ONE );
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


