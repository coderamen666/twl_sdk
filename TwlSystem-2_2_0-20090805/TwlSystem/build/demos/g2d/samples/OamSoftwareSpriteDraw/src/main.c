/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - OamSoftwareSpriteDraw
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
//      This is an OAM software sprite rendering sample.
//      It compares the differences in rendering load between each format.
//      It renders NUM_OBJ_X * NUM_OBJ_Y sprites.
//  Using the Demo
//
//      A Button: Switches the OBJ displayed (If OBJs exist)
//      B Button: Switches the render format.
//      Up/Down/Left/Right: Adjusts the number of rendered sprites.
//
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"

//------------------------------------------------------------------------------
// Constant Definitions
#define AFFINE_IDX                  3                       // Affine Index for cell
#define CELL_ROTATE_UNIT            0xFF                    // Units of cell rotations
#define CELL_SCALE_UNIT             ((FX32_ONE >> 7) * 3)   // Units of cell scaling
                                                            // The scaling value is ensured to be not near 0
#define CELL_INIT_POS_X             (120 << FX32_SHIFT)     // Initial X position of the cell
#define CELL_INIT_POS_Y             (120 << FX32_SHIFT)     // Initial Y position of the cell
#define NUM_OBJ_X                   48                       
#define NUM_OBJ_Y                   32
#define STEP_OBJ                    10

#define INIT_OUTPUT_TYPE            NNS_G2D_OAMTYPE_MAIN    // Initial value for output method

#define NUM_OF_OAM                  128                     // Number of OAMs allocated to OAM manager
#define NUM_OF_AFFINE               (NUM_OF_OAM / 4)        // Number of affine parameters allocated to OAM Manager

#define TEX_BASE                    0x0                     // Texture base address
#define TEX_PLTT_BASE               0x0                     // Texture palette base address



//------------------------------------------------------------------------------
// Structure definitions



//------------------------------------------------------------------------------
// Global Variables

static NNSG2dImageProxy         sImageProxy;    // Character/texture proxy for cell
static NNSG2dImagePaletteProxy  sPaletteProxy;  // Palette proxy for cell

//------------------------------------------------------------------------------
// Prototype Declarations

void VBlankIntr(void);
static NNSG2dCellDataBank* LoadResources(void);

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
// Rendering Type
typedef enum MyDrawMetod
{
    MyDrawMetod_Normal,         // Normal
    MyDrawMetod_Fast,           // fast
    MyDrawMetod_UsingCache,     // use cache, fast
    MyDrawMetod_Max
}MyDrawMetod;

const char* strDrawMetod[]=
{
    "Normal    ",
    "Fast      ",
    "UsingCache",
    "NG",
};

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

    //------------------------------------------------------------------------------
    // load cell data
    {
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/OamSoftwareSpriteDraw.NCER" );
        SDK_NULL_ASSERT( pBuf );
        // Because the cell bank is used in main memory until the end,
        // this pBuf is not freed.
    }


    // load character data for 3D (software sprite)
    {
        NNSG2dCharacterData* pCharData;

        pBuf = G2DDemo_LoadNCBR( &pCharData, "data/OamSoftwareSpriteDraw.NCBR" );
        SDK_NULL_ASSERT( pBuf );

        // Loading For 3D Graphics Engine.
        NNS_G2dLoadImage2DMapping(
            pCharData,
            TEX_BASE,
            NNS_G2D_VRAM_TYPE_3DMAIN,
            &sImageProxy );

        G2DDemo_Free( pBuf );
    }

    //------------------------------------------------------------------------------
    // load palette data
    {
        NNSG2dPaletteData* pPlttData;

        pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/OamSoftwareSpriteDraw.NCLR" );
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
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    NNSG2dCellDataBank*         pCellBank;      // Cell data bank

    // Initialize App.
    {
        BOOL result = TRUE;

        G2DDemo_CommonInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        G2DDemo_PrintInit();
        
        pCellBank = LoadResources();
        G2_SetBG1Priority( 0 );
        G2_SetBG0Priority( 1 );
    }

    // start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    //----------------------------------------------------
    // Main loop
    while( TRUE )
    {
        static CellInfo cellInfo = 
        {
            0,                                  // Number of cell to render
            0,                                  // Rotation angle
            {FX32_ONE, FX32_ONE},               // Scale
            {CELL_INIT_POS_X, CELL_INIT_POS_Y}  // Position of cell rendering
        };
        static NNSG2dOamType        outputType = INIT_OUTPUT_TYPE;  // cell output method
        static OSTick time;
        static MyDrawMetod          drawMethod = MyDrawMetod_Normal;
        static int numX = 24, numY = 16;
        //
        // Key operations
        //
        {
            G2DDemo_ReadGamePad();

            // Change current Cell
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_A) )
            {
                cellInfo.cellIdx++;
                if( cellInfo.cellIdx >= pCellBank->numCells )
                {
                    cellInfo.cellIdx = 0;
                }
            }
            
            if ( G2DDEMO_IS_TRIGGER(PAD_BUTTON_B) )
            {
                drawMethod++;
                drawMethod %= MyDrawMetod_Max;
            }
            
            if ( G2DDEMO_IS_PRESS(PAD_KEY_UP) )
            {
                numY--;
            }
            if ( G2DDEMO_IS_PRESS(PAD_KEY_DOWN) )
            {
                numY++;
            }
            if ( G2DDEMO_IS_PRESS(PAD_KEY_LEFT) )
            {
                numX--;
            }
            if ( G2DDEMO_IS_PRESS(PAD_KEY_RIGHT) )
            {
                numX++;
            }
            
        }   

        if( NUM_OBJ_X < numX )
        {
            numX = NUM_OBJ_X;
        }
        
        if( 0 > numX )
        {
            numX = 0;
        }
        
        if( NUM_OBJ_Y < numY )
        {
            numY = NUM_OBJ_Y;
        }
        
        if( 0 > numY )
        {
            numY = 0;
        }  

        // Render
        //
        // Renders the OBJ.
        // Calculates the time required for rendering as well
        //
        //
        time = OS_GetTick();
        {
            GXOamAttr      tempOam;
            int i, j;
            
            // Renders the number 0 OBJ.
            NNS_G2dCopyCellAsOamAttr( NNS_G2dGetCellDataByIdx( pCellBank, cellInfo.cellIdx ), 0, &tempOam );
            NNS_G2dSetupSoftwareSpriteCamera();
            
            switch( drawMethod )
            {
            //------------------------------------------------------------------------------
            // normal rendering
            case MyDrawMetod_Normal:
                for( i = 0; i < numX; i ++ )
                {
                    for( j = 0;j < numY; j++ )
                    {
                        NNS_G2dDrawOneOam3DDirectWithPos( (s16)(STEP_OBJ*i), (s16)(STEP_OBJ*j), (s16)(-1), &tempOam, 
                                           &sImageProxy.attr,
                                           NNS_G2dGetImageLocation( &sImageProxy, NNS_G2D_VRAM_TYPE_3DMAIN ),
                                           NNS_G2dGetImagePaletteLocation( &sPaletteProxy, NNS_G2D_VRAM_TYPE_3DMAIN ) );
                    }
                }
                break;
            //------------------------------------------------------------------------------
            // Fast Render: The current matrix is not saved to stack or restored.
            case MyDrawMetod_Fast:
                G3_PushMtx();
                for( i = 0; i < numX; i ++ )
                {
                    for( j = 0;j < numY; j++ )
                    {
                        // The current matrix is not saved so its initialization needs to be done prior to rendering.
                        G3_Identity();
                        NNS_G2dDrawOneOam3DDirectWithPosFast( (s16)(STEP_OBJ*i), (s16)(STEP_OBJ*j), (s16)(-1), &tempOam, 
                                           &sImageProxy.attr,
                                           NNS_G2dGetImageLocation( &sImageProxy, NNS_G2D_VRAM_TYPE_3DMAIN ),
                                           NNS_G2dGetImagePaletteLocation( &sPaletteProxy, NNS_G2D_VRAM_TYPE_3DMAIN ) );
                    }
                }
                G3_PopMtx(1);
                break;
            //------------------------------------------------------------------------------
            // Fast Rendering Using Cache: Fastest speed, but can only render OBJs with the same image
            // (different sizes can be used)                
            case MyDrawMetod_UsingCache: 
                NNS_G2dSetOamSoftEmuSpriteParamCache( &tempOam, 
                                           &sImageProxy.attr,
                                           NNS_G2dGetImageLocation( &sImageProxy, NNS_G2D_VRAM_TYPE_3DMAIN ),
                                           NNS_G2dGetImagePaletteLocation( &sPaletteProxy, NNS_G2D_VRAM_TYPE_3DMAIN ) );    
                G3_PushMtx();
                for( i = 0; i < numX; i ++ )
                {
                    for( j = 0;j < numY; j++ )
                    {
                        // The current matrix is not saved so its initialization needs to be done prior to rendering.
                        G3_Identity();
                        NNS_G2dDrawOneOam3DDirectUsingParamCacheFast( (s16)(STEP_OBJ*i), (s16)(STEP_OBJ*j), (s16)(-1), &tempOam );
                    }
                }
                G3_PopMtx(1);
                break;
            }
        }
        time = OS_GetTick() - time;

        // Output display information
        {
            
            G2DDemo_PrintOutf( 0, 21, "TIME:%06ld usec\n", OS_TicksToMicroSeconds(time) );
            G2DDemo_PrintOut ( 0, 22, strDrawMetod[drawMethod] );
            G2DDemo_PrintOutf( 0, 23, "NUM_SPRITE:%04ld usec\n", numX * numY );
        }

        //
        // Write manager contents to HW.
        //
        {
            // Wait for V-Blank
            G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
            SVC_WaitVBlankIntr();
            
            // Display information
            G2DDemo_PrintApplyToHW();
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

