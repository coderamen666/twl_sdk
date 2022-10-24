/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - VRamAnimation
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
//      Example of rendering and updating of cell animation using VRAM transfer manager
//      - The -vta option is being designated to the converter and data is being output in order to create the binary data for VRAM transmission.
//        (Be sure to check the MakeFile )
//
//  Using the Demo
//      START button: Switches the cell output method between OBJ and Software Sprite.
//                      
//      A Button: Switches the cell to be displayed (if the cells are available.)
//      X button: Resets scaling, movement, and rotation status of the cell.
//      +Control Pad: Moves the cell.
//      B Button + +Control Pad:
//                      Rotates the cell.
//      Y Button + +Control Pad:
//                      Adjusts cell scaling.
//
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


#define CHARNAME_TO_OFFSET(charname)    (charname * 32)     // Change character name to offset in VRAM

#define SCREEN_WIDTH                256                     // screen width
#define SCREEN_HEIGHT               192                     // screen height

#define AFFINE_IDX                  3                       // Affine Index for cell
#define CELL_ROTATE_UNIT            0xFF                    // Units of cell rotations
#define CELL_SCALE_UNIT             ((FX32_ONE >> 7) * 3 )  // Units of cell scaling
                                                            // The scaling value is ensured to be not near 0
#define CELL_INIT_POS_X             (120 << FX32_SHIFT)     // Initial X position of the cell
#define CELL_INIT_POS_Y             (120 << FX32_SHIFT)     // Initial Y position of the cell

#define INIT_OUTPUT_TYPE            NNS_G2D_SURFACETYPE_MAIN2D   // Initial value for output method

#define NUM_OF_OAM                  128                     // Number of OAMs allocated to OAM manager
#define NUM_OF_CELLANM              1                       // Number of cell animations displayed
#define NUM_OF_VRAMSTATE            NUM_OF_CELLANM          // Number of statuses handled by cell transfer manager
#define NUM_OF_VRAMTASK             NUM_OF_CELLANM          // Number of transfer tasks handled by VRAM transfer manager

#define PLTT_BASE                   0x0                     // Palette base address

#define CELL0_CHARNAME_BEGIN_MAIN   6                       // Starting number for main screen OAM used
#define CELL0_CHARNAME_BEGIN_SUB    0                       // Starting number for subscreen OAM used
#define CHAR_MAIN_OFFSET            CHARNAME_TO_OFFSET(CELL0_CHARNAME_BEGIN_MAIN)
#define CHAR_SUB_OFFSET             CHARNAME_TO_OFFSET(CELL0_CHARNAME_BEGIN_SUB)

#define TEX_OFFSET                  128                     // Offset of texture VRAM used



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

// Work area for VRAM transfer manager
static NNSGfdVramTransferTask       sTransferTaskArray[NUM_OF_VRAMTASK];

// Work area for cell transfer manager
static NNSG2dCellTransferState      sVramStateArray[NUM_OF_VRAMSTATE];

static u32                          sCellTransManHandle;    // Handle of cell transfer management object


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
  Name:         CallbackRegistTransferTask

  Description:  This is a function called inside an NNS_G2dDraw* function to register the character data's transmission task.
                

  Arguments:    type:       Type of VRAM area to which data to be transferred
                dstAddr :    Address offset to which data to be transferred
                pSrc:       Pointer to data to be transferred
                szByte:     Size of data to be transferred

  Returns:      Returns TRUE if successful; returns FALSE otherwise.
 *---------------------------------------------------------------------------*/
static BOOL CallbackRegistTransferTask(
                NNS_GFD_DST_TYPE type,
                u32 dstAddr,
                void* pSrc,
                u32 szByte )
{
    // Pass to VRAM transfer manager
    return NNS_GfdRegisterNewVramTransferTask(
                type,
                dstAddr,
                pSrc,
                szByte );
}

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
  Name:         InitOamManager

  Description:  Initializes the OAM manager system and initializes a single OAM manager instance.
                

  Arguments:    pOamManager:    Pointer to the OAM manager to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitOamManager(NNSG2dOamManagerInstance* pOamManager)
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

    // Display rectangle
    pRect->posTopLeft.x = FX32_ONE * 0;
    pRect->posTopLeft.y = FX32_ONE * 0;
    pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
    pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

    // Callback Functions
    pSurface->pFuncOamRegister       = CallBackAddOam;
    pSurface->pFuncOamAffineRegister = CallBackAddAffine;

    // Surface registration
    NNS_G2dAddRendererTargetSurface( pRenderer, pSurface );

    // Proxy registration
    NNS_G2dSetRendererImageProxy( pRenderer, &sImageProxy, &sPaletteProxy );

        // Specify the Z offset increment
    NNS_G2dSetRendererSpriteZoffset( pRenderer, - FX32_ONE );
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Loads the necessary data from a file.
                It also builds the cell animation.

  Arguments:    None.

  Returns:      Pointer to the loaded cell bank.
 *---------------------------------------------------------------------------*/
static NNSG2dCellDataBank* LoadResources(void)
{
    NNSG2dCharacterData* pCharData2D;
    NNSG2dCharacterData* pCharData3D;
    NNSG2dCellDataBank* pCellBank;
    void* pBuf;

    // Proxy initialization
    NNS_G2dInitImageProxy( &sImageProxy );
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );

    //------------------------------------------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/VRamAnimation.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 2D Graphics Engine
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sPaletteProxy);

        // Loading For 3D Graphics Engine.
        NNS_G2dLoadPalette(
            pPlttData,
            PLTT_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sPaletteProxy);

        // 
        // Deallocate pBuf because the palette data was copied into VRAM.
        G2DDemo_Free( pBuf );
    }

    //------------------------------------------------------------------------------
    // load character data for 2D
    {
        pBuf = G2DDemo_LoadNCGR( &pCharData2D, "data/VRamAnimation.NCGR" );
        SDK_NULL_ASSERT( pBuf );
        // 
        // This pBuf is not deallocated because this data is used in main memory until the end. Same below.

        // Unlike the other NNS_G2dLoad* functions,  NNS_G2dLoadImageVramTransfer
        // does not read character data into VRAM.
        // The character data attributes are just specified for ImageProxy and HW.
        // As a result, the character data must be maintained in main memory.
        NNS_G2dLoadImageVramTransfer(
            pCharData2D,
            CHAR_MAIN_OFFSET,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sImageProxy );
    }

    // load character data for 3D (software sprite)
    {
        pBuf = G2DDemo_LoadNCBR( &pCharData3D, "data/VRamAnimation.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        NNS_G2dLoadImageVramTransfer(
            pCharData3D,
            TEX_OFFSET,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sImageProxy );
    }

    //------------------------------------------------------------------------------
    // load cell data
    {
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/VRamAnimation.NCER" );
        SDK_NULL_ASSERT( pBuf );
    }


    //------------------------------------------------------------------------------
    // load animation data
    {
        pBuf = G2DDemo_LoadNANR( &spAnimBank, "data/VRamAnimation.NANR" );
        SDK_NULL_ASSERT( pBuf );
    }

    //------------------------------------------------------------------------------
    // Build cell animation
    {
        // Perform initialization for VRAM transfer
        {
            NNS_GfdInitVramTransferManager(
                sTransferTaskArray,
                NUM_OF_VRAMTASK
            );

            NNS_G2dInitCellTransferStateManager(
                sVramStateArray,
                NUM_OF_VRAMSTATE,
                CallbackRegistTransferTask          // <- (1)
            );

            // Get a new cell transfer status handling object.
            // When the object is no longer needed,
            // deallocate it with NNS_G2dFreeCellTransferStateHandle( handle ).
            sCellTransManHandle = NNS_G2dGetNewCellTransferStateHandle();
            SDK_ASSERT( sCellTransManHandle != NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE );
        }

        // Initialize the cell animation instance
        {
            spCellAnim = G2DDemo_GetNewCellAnimation(1);
            SDK_NULL_ASSERT( spCellAnim );

            SDK_NULL_ASSERT( NNS_G2dGetAnimSequenceByIdx(spAnimBank, 0) );
            SDK_ASSERT( NNS_G2dCellDataBankHasVramTransferData(pCellBank) );

            // If arguments 8 and 9 of NNS_G2dInitializeVramTransferedCellAnimation are both non-NULL,
            // the sizes of the 2 types of data must basically be equal.
            SDK_ASSERT( pCharData2D->szByte == pCharData3D->szByte );

            NNS_G2dInitCellAnimationVramTransfered(
                spCellAnim,
                NNS_G2dGetAnimSequenceByIdx(spAnimBank, 0),
                pCellBank,

                sCellTransManHandle,    // Manage the VRAM transfer for this cell animation
                                        // Handle of cell transfer management object
                                        // The 3 items below are passed to callback function (1) as dstAddr.
                                        // Normally the value specified is the same value specified for ImageProxy.
                TEX_OFFSET,             // Texture transfer offset
                CHAR_MAIN_OFFSET,       // Destination offset for main screen character transfer
                CHAR_SUB_OFFSET,        // Destination offset for subscreen character transfer
                                        // Either of the following 2 times can be NULL
                pCharData2D->pRawData,  // Character data
                pCharData3D->pRawData,  // Texture data
                pCharData2D->szByte     // Size of texture data or character data
            );
        }
    }

    return pCellBank;
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

    // Rotate Cell
    if( G2DDEMO_IS_PRESS(PAD_BUTTON_B) )
    {
        if ( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
        {
            pCellInfo->rotate -= CELL_ROTATE_UNIT;      // Counterclockwise
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
        {
            pCellInfo->rotate += CELL_ROTATE_UNIT;      // Clockwise
        }
    }

    // Zoom Cell
    if( G2DDEMO_IS_PRESS(PAD_BUTTON_Y) )
    {
        if ( G2DDEMO_IS_PRESS(PAD_KEY_UP) )
        {
            pCellInfo->scale.y += CELL_SCALE_UNIT;   // Expand in Y direction
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_DOWN) )
        {
            pCellInfo->scale.y -= CELL_SCALE_UNIT;   // Reduce in Y direction
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
        {
            pCellInfo->scale.x += CELL_SCALE_UNIT;   // Expand in X direction
        }
        if ( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
        {
            pCellInfo->scale.x -= CELL_SCALE_UNIT;   // Reduce in X direction
        }
    }

    // Move Cell
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

    // Toggle Output Type
    if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_START) )
    {
        *pOutputType = (*pOutputType == NNS_G2D_SURFACETYPE_MAIN2D) ?
                            NNS_G2D_SURFACETYPE_MAIN3D:
                            NNS_G2D_SURFACETYPE_MAIN2D;
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
        G2DDemo_PrintInit();
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


        // Update display information and output method for cell for which input is read
        ProcessInput(&cellInfo, &outputType, pCellBank->numCells);

        surface.type = outputType;

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

            // The transfer request for the cell transfer manager is sent to the VRAM transfer manager via the callback function.
            // 
            NNS_G2dUpdateCellTransferStateManager();
        }

        // Output display information
        {
            G2DDemo_PrintOutf(0, 0, "pos:   x=%3d     y=%3d",
                cellInfo.pos.x >> FX32_SHIFT, cellInfo.pos.y >> FX32_SHIFT);
            G2DDemo_PrintOutf(0, 1, "scale: x=%7.3f y=%7.3f",
                (double)cellInfo.scale.x / FX32_ONE, (double)cellInfo.scale.y / FX32_ONE);
            G2DDemo_PrintOutf(0, 2, "rot:   %5d (%6.2f)",
                cellInfo.rotate, 360.0 * cellInfo.rotate / 0x10000);
            G2DDemo_PrintOut(0, 23, (outputType == NNS_G2D_SURFACETYPE_MAIN2D) ?
                   "OBJ           ": "SoftwareSprite");
        }

        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();


        //
        // Write buffer contents to HW
        //
        {
            // Perform a transfer to VRAM
            NNS_GfdDoVramTransfer();

            // Write display information
            G2DDemo_PrintApplyToHW();

            // Write OAM
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


