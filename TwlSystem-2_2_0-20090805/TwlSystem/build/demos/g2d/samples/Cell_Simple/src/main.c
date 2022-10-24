/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - Cell_Simple
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
//      Sample demonstrating simple rendering of a cell
//
//  Features
//      Uses the OAM manager module.
//      Toggles between OBJ-based and software sprite-based rendering.
//      Does not animate cells.
//
//  Using the Demo
//      START button: Switches the cell output method between OBJ and Software Sprite.
//                      
//      A Button:      Switches the cell to be displayed (if there is a cell to switch to).
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


#define AFFINE_IDX                  3                       // Affine Index for cell
#define CELL_ROTATE_UNIT            0xFF                    // Units of cell rotations
#define CELL_SCALE_UNIT             ((FX32_ONE >> 7) * 3)   // Units of cell scaling
                                                            // The scaling value is ensured to be not near 0
#define CELL_INIT_POS_X             (120 << FX32_SHIFT)     // Initial X position of the cell
#define CELL_INIT_POS_Y             (120 << FX32_SHIFT)     // Initial Y position of the cell

#define INIT_OUTPUT_TYPE            NNS_G2D_OAMTYPE_MAIN    // Initial value for output method

#define NUM_OF_OAM                  128                     // Number of OAMs allocated to OAM manager
#define NUM_OF_AFFINE               (NUM_OF_OAM / 4)        // Number of affine parameters allocated to OAM Manager

#define TEX_BASE                    0x0                     // Texture base address
#define TEX_PLTT_BASE               0x0                     // Texture palette base address



//------------------------------------------------------------------------------
// Structure definitions

//------------------------------------------------------------------------------
//
// Cell information
//
typedef struct CellInfo
{
    u16          cellIdx;   // Number of cell to render
    u16          rotate;    // Rotation angle
    NNSG2dFVec2  scale;     // Scale
    NNSG2dFVec2  pos;       // Position of cell rendering
    
} CellInfo;


//------------------------------------------------------------------------------
// Global variables

static NNSG2dImageProxy             sImageProxy;    // Character/texture proxy for cell
static NNSG2dImagePaletteProxy      sPaletteProxy;  // Palette proxy for cell

static NNSG2dCellDataBank*          spCellBank = NULL;
static NNSG2dOamManagerInstance     objOamManager;  // OAM manager for OBJ output
static NNSG2dOamManagerInstance     ssOamManager;   // OAM manager for SoftwareSprite output

static GXOamAttr                    tempOamBuffer[NUM_OF_OAM];           // Temporary OAM buffer 

static CellInfo cellInfo = {
    0,                                  // Number of cell to render
    0,                                  // Rotation angle
    {FX32_ONE, FX32_ONE},               // Scale
    {CELL_INIT_POS_X, CELL_INIT_POS_Y}  // Position of cell rendering
};


//------------------------------------------------------------------------------
// Prototype Declarations
void VBlankIntr(void);

static NNSG2dCellDataBank* LoadResources(void);
static void ProcessInput(CellInfo* pCellInfo, NNSG2dOamType* pOutputType, u16 numCells);
static void MakeAffineMatrix(MtxFx22* pMtx, MtxFx22* pMtx2D, const CellInfo* pCellInfo);
static void InitApp( void );
static void AppDraw
( 
    NNSG2dOamManagerInstance*   pOamMgr, 
    const CellInfo*             pCellInfo,
    const MtxFx22*              pMtxAffineForCell,
    const MtxFx22*              pMtxAffineForOBJ
);
static void ApplayOamMgrToHW( NNSG2dOamType outputType );





/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  Loads the cell bank, character data and palette data from a file, and then loads the character and palette data into VRAM.
                
                

  Arguments:    None.

  Returns:      Pointer to the loaded cell bank.
 *---------------------------------------------------------------------------*/
static NNSG2dCellDataBank* LoadResources(void)
{
    NNSG2dCellDataBank* pCellBank;
    void* pBuf;

    NNS_G2dInitImageProxy( &sImageProxy );
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );

    //----------------------------------------------
    // load cell data
    {
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/Cell_Simple.NCER" );
        SDK_NULL_ASSERT( pBuf );
        // Because the cell bank is used in main memory until the end,
        // this pBuf is not freed.
    }

    //----------------------------------------------
    // load character data for 2D
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCGR( &pCharData, "data/Cell_Simple.NCGR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 2D Graphics Engine
        NNS_G2dLoadImage2DMapping(
            pCharData,
            TEX_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sImageProxy );

        // Because character data was copied into VRAM,
        // this pBuf is freed. Same below.
        G2DDemo_Free( pBuf );
    }
    //--------------------------------------------
    // load character data for 3D (software sprite)
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCBR( &pCharData, "data/Cell_Simple.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 3D Graphics Engine.
        NNS_G2dLoadImage2DMapping(
            pCharData,
            TEX_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sImageProxy );

        G2DDemo_Free( pBuf );
    }

    //--------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/Cell_Simple.NCLR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 2D Graphics Engine
        NNS_G2dLoadPalette(
            pPlttData,
            TEX_PLTT_BASE,
            NNS_G2D_VRAM_TYPE_2DMAIN,
            &sPaletteProxy);

        // Loading For 3D Graphics Engine.
        NNS_G2dLoadPalette(
            pPlttData,
            TEX_PLTT_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sPaletteProxy);

        G2DDemo_Free( pBuf );
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
static void ProcessInput(CellInfo* pCellInfo, NNSG2dOamType* pOutputType, u16 numCells)
{
    SDK_NULL_ASSERT( pCellInfo );

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
        if( *pOutputType == NNS_G2D_OAMTYPE_MAIN )
        {
            // Resets the OAM
            NNS_G2dResetOamManagerBuffer( &objOamManager );
            NNS_G2dApplyOamManagerToHW( &objOamManager );
            
            *pOutputType = NNS_G2D_OAMTYPE_SOFTWAREEMULATION;
        }else{
            *pOutputType = NNS_G2D_OAMTYPE_MAIN;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         MakeAffineMatrix

  Description:  Creates an Affine matrix based on the display information for the cell.

  Arguments:    pMtx:       Pointer to MtxFx22 that takes a normal affine matrix.
                pMtx2D:     Pointer to the MtxFx22 matrix that takes an affine matrix for the 2D graphics engine.
                            
                pCellInfo:  CellInfo represents the cell display information.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MakeAffineMatrix(MtxFx22* pMtx, MtxFx22* pMtx2D, const CellInfo* pCellInfo)
{
    MtxFx22 tmpMtx;

    // The rotation is common to the 2 matrices.
    G2DDemo_MTX_Rot22( &tmpMtx, FX_SinIdx( pCellInfo->rotate ), FX_CosIdx( pCellInfo->rotate ) );

    // Create normal affine matrix
    MTX_ScaleApply22(
        &tmpMtx,
        pMtx,
        pCellInfo->scale.x,
        pCellInfo->scale.y );

    // Create affine matrix for 2D graphics engine
    // A characteristic point of code is that the scaling component is set as an inverse number.
    // These are the 2D graphics engine specifications.
    // For details, see the programming manual.
    MTX_ScaleApply22(
        &tmpMtx,
        pMtx2D,
        (FX32_ONE * FX32_ONE) / pCellInfo->scale.x,
        (FX32_ONE * FX32_ONE) / pCellInfo->scale.y );
}

//------------------------------------------------------------------------------
// Initializes the application.
//------------------------------------------------------------------------------
static void InitApp( void )
{
    BOOL result = TRUE;
    
    //
    // Initializes the demo library
    //
    {
        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();
    }
    
    //
    // Initializes the OAM manager
    //
    // Buffer transfer performance falters, but a function for initializing an OAM manager instance within greater freedom is available as the NNS_G2dGetNewOamManagerInstance function.
    // 
    // 
    //
    {
        NNS_G2dInitOamManagerModule();
        
        // For OBJ
        {
            result = NNS_G2dGetNewOamManagerInstanceAsFastTransferMode
                     ( 
                        &objOamManager, 
                        0, 
                        NUM_OF_OAM, 
                        NNS_G2D_OAMTYPE_MAIN 
                     );
            SDK_ASSERT( result );
        }
        
        // For software sprites
        {
            result = NNS_G2dGetNewOamManagerInstanceAsFastTransferMode
                     ( 
                        &ssOamManager, 
                        0, 
                        NUM_OF_OAM, 
                        NNS_G2D_OAMTYPE_SOFTWAREEMULATION 
                     );
            SDK_ASSERT( result );
            // Specify the Z offset increment for software sprite.
            NNS_G2dSetOamManagerSpriteZoffsetStep( &ssOamManager, - FX32_ONE );
        }
    }
    
    //
    // Load the resources
    //
    spCellBank = LoadResources();
}

//------------------------------------------------------------------------------
// Render
//
// Render the cell using the low-level rendering method NNS_G2dMakeCellToOams().
// There is also a rendering method that uses G2dRenderer of the upper level module.
//
//
//       The NNS_G2dMakeCellToOams() function writes the cell to the specified buffer as an OBJ group that represents the cell.
//       In this sample, after writing to a temporary buffer, the content of that buffer is registered to the OAM manager module.
//       
//    
// Caution: There are two matrices that express the affine parameters: pMtxAffineForCell and pMtxAffineForOBJ.
//       
//       
//       The matrix represented by pMtxAffineForOBJ is used as the affine parameters to be accessed by the object.
//        As a result, when the OAM manager is the 2D type, a matrix for one part of the scale must be passed.
//       
//
//       The matrix that pMtxAffineForCell represents is used when converting the position of an object within a cell, without referencing from the 2D graphics engine. 
//       
//
static void AppDraw
( 
    NNSG2dOamManagerInstance*   pOamMgr, 
    const CellInfo*             pCellInfo,
    const MtxFx22*              pMtxAffineForCell,
    const MtxFx22*              pMtxAffineForOBJ
)
{
    // Number of OAMs that have been rendered
    u16                     numOamDrawn = 0;  
    // affine parameter settings (affine Index)
    const u16               affineIdx 
        = NNS_G2dEntryOamManagerAffine( pOamMgr, pMtxAffineForOBJ );
    // Cell to be displayed
    const NNSG2dCellData*   pCell
        = NNS_G2dGetCellDataByIdx( spCellBank, pCellInfo->cellIdx );                    
    SDK_NULL_ASSERT( pCell );
    
    //----------------------------------------------------
    // Writes an OBJ list equivalent to the cell to the temporary buffer.
    //
    numOamDrawn  = NNS_G2dMakeCellToOams(
                        tempOamBuffer,      // Output Oam buffer
                        NUM_OF_OAM,         // Output buffer length
                        pCell,              // Cell to output
                        pMtxAffineForCell,  // Affine transformation     <= Specify normal affine matrix for affine transform
                        &pCellInfo->pos,    // Cell position     This matrix is not passed to the 2D graphics engine and used for affine transformation of the cell.
                        affineIdx,          // Affine Index
                        TRUE );             // Double affine?

    SDK_ASSERT( numOamDrawn < NUM_OF_OAM );

    //---------------------------------------------------- 
    // The temporary OBJ list is registered for the OAM manager
    // 
    (void)NNS_G2dEntryOamManagerOam( pOamMgr,tempOamBuffer, numOamDrawn );
    
}

//------------------------------------------------------------------------------
//
// Write manager contents to HW.
//
// Processing branches for each outputType.
//
static void ApplayOamMgrToHW( NNSG2dOamType outputType )
{ 
    // Render cell with SoftwareSprite
    if( outputType == NNS_G2D_OAMTYPE_SOFTWAREEMULATION )
    {
        u32 texBase = NNS_G2dGetImageLocation( &sImageProxy, NNS_G2D_VRAM_TYPE_3DMAIN );
        u32 pltBase = NNS_G2dGetImagePaletteLocation( &sPaletteProxy, NNS_G2D_VRAM_TYPE_3DMAIN );
        //
        // In this sample, the manager content is written to hardware during a V-Blank, but for software sprite rendering, this process doesn't have to occur during a V-Blank.
        // 
        // 
        //
        NNS_G2dApplyOamManagerToHWSprite( &ssOamManager, &sImageProxy.attr, texBase, pltBase );
        NNS_G2dResetOamManagerBuffer( &ssOamManager );
                
    }else{
    // Render cell with OBJ
        NNS_G2dApplyOamManagerToHW( &objOamManager );
        NNS_G2dResetOamManagerBuffer( &objOamManager );
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
    //----------------------------------------------------
    // Initialize animation
    InitApp();
    // start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }


    //----------------------------------------------------
    // Main Loop
    while( TRUE )
    {
        static NNSG2dOamType outputType = INIT_OUTPUT_TYPE;  // Cell output method
        MtxFx22              mtxAffine;                      // Normal cell affine matrix
        MtxFx22              mtxAffine2D;                    // Matrix used for OBJ affine parameters
        
        //-------------------------------------------------
        // Update
        //
        // Update display information and output method for cell for which input is read
        ProcessInput( &cellInfo, &outputType, spCellBank->numCells);
        // Generate affine conversion matrix
        MakeAffineMatrix( &mtxAffine, &mtxAffine2D, &cellInfo );
        
        
        //-------------------------------------------------
        // Render
        //
        // You can switch between the OAM manager and object affine parameter matrices when displaying either as objects or as SoftwareSprite data.
        // 
        // 
        if( outputType == NNS_G2D_OAMTYPE_MAIN )
        {
            AppDraw( &objOamManager, &cellInfo, &mtxAffine, &mtxAffine2D );
        }else{               
            AppDraw( &ssOamManager, &cellInfo, &mtxAffine, &mtxAffine );
        }
        
        //-------------------------------------------------
        // Output display information
        {
            G2DDemo_PrintOutf(0, 0, "pos:   x=%3d     y=%3d",
                cellInfo.pos.x >> FX32_SHIFT, cellInfo.pos.y >> FX32_SHIFT);
            G2DDemo_PrintOutf(0, 1, "scale: x=%7.3f y=%7.3f",
                (double)cellInfo.scale.x / FX32_ONE, (double)cellInfo.scale.y / FX32_ONE);
            G2DDemo_PrintOutf(0, 2, "rot:   %5d (%6.2f)",
                cellInfo.rotate, 360.0 * cellInfo.rotate / 0x10000);
            G2DDemo_PrintOut(0, 23, (outputType == NNS_G2D_OAMTYPE_MAIN ) ?
                   "OBJ           ": "SoftwareSprite");
        }
        
        //-------------------------------------------------
        //
        // Wait for V-Blank
        {
            G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
            SVC_WaitVBlankIntr();
        
            // Reflect display information (manipulate BG data)
            G2DDemo_PrintApplyToHW();
        
            //-------------------------------------------------
            //
            // Writes manager contents to HW.
            //
            ApplayOamMgrToHW( outputType );
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

