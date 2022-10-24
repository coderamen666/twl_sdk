/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - OamManagerEx3
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
//      This sample demonstrates how the extended OAM Manager and the renderer can be combined and used.
//      -OBJs are displayed using time-division and the apparent number of displayed OBJs is greater than the actual number of OAMs used.
//      - OBJs are displayed according to priority by specifying a rendering order for the OBJs.
//      - If two OBJ have the same display priority, the demo explains the rendering order of affine-transformed OAM and normal OAM.
//      - The algorithm that displays time percentages changes its display based on the amount of hardware used (in terms of the number of objects and affine parameters) and priority.
//        
//
//  Using the Demo
//      +Control Pad Left/Right: Moves character horizontally.
//      +Control Pad Up/Down: Moves character forward and backward.
//      A: Increases the number of OBJ used by the OMA manager.
//      B: Decreases the number of OBJ used by the OMA manager.
//      X: Increases the number of affine parameters used by the OMA manager.
//      Y: Decreases the number of affine parameters used by the OMA manager.
//      START: Switches the extended OAM manager rendering order.          
//      SELECT: Sets the rendering priority of the right-end cell to a different value than other two cells.
//
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"

// Size of screen
#define SCREEN_WIDTH            256
#define SCREEN_HEIGHT           192




#define NUM_OF_OAM_CHUNK        32   // Number of OAM chunks held by the extended OAM manager
#define NUM_OF_AFFINE_PROXY     32   // Number of affine parameter proxies held by the extended OAM manager
#define NUM_OF_ORDERING_TBL     2    // Length of the ordering table maintained by the extended OAM manager
                                     // (= the number of priorities that can be specified)
                                     

// Load address offset for resource
#define CHARA_BASE              0x0000
#define PLTT_BASE               0x0000



//-----------------------------------------------------------------------------
// Structure definitions

// The structure that indicates position
typedef struct Position
{
    s16 x;
    s16 y;
} Position;



//-----------------------------------------------------------------------------
// Global Variables

//-----------------------------------------------------------------------------
// Extended OAM manager
static NNSG2dOamManagerInstanceEx       myOamMgrEx_;
// Chunk
// The container for the list storing OAM.
static NNSG2dOamChunk                   sOamChunk[NUM_OF_OAM_CHUNK];
// Chunk List
// Maintains a chunk list for each display priority level.
// The same number of lists are required as the number of display priority levels used.
static NNSG2dOamChunkList               sOamChunkList[NUM_OF_ORDERING_TBL];
// Affine Parameter Proxy
// Extends the AffineIndex determination until affine parameters are stored and reflected in the hardware.
static NNSG2dAffineParamProxy           sAffineProxy[NUM_OF_AFFINE_PROXY];



//-----------------------------------------------------------------------------
// OAM Manager
static NNSG2dOamManagerInstance         myOamMgr_;

//-----------------------------------------------------------------------------
// Related to the renderer
static NNSG2dRendererInstance           myRenderer_;
static NNSG2dRenderSurface              myRenderSurface_;
static NNSG2dImageProxy                 myImageProxy_;
static NNSG2dImagePaletteProxy          myPaletteProxy_;


//-----------------------------------------------------------------------------
// 
// cell animation
static NNSG2dCellAnimation*             pMyCellAnim_;     

// Used when registering an extended OAM manager
// Display priority
static u8                               oamPriority_ = 0; 

// The number of OAM attributes used by the extended OAM manager
static u16 numOamReservedForManager_           =   8;     
// The number of affine parameters used by the extended OAM manager
static u16 numOamAffineReservedForManager_     =   1;     

// The number of OAMs used by the extended OAM manager
static u16 numOamUsedByManager_         = 0;              
// The number of affine parameters used by the extended OAM manager
static u16 numOamAffineUsedByManager_   = 0;              

//-----------------------------------------------------------------------------
// Prototype Declarations
void NitroMain(void);
void VBlankIntr(void);

//------------------------------------------------------------------------------
// OAM registration function being used by the renderer
//
// Calls the extended OAM manager's OAM registration function.
//
static BOOL RndrCBFuncEntryOam_
( 
    const GXOamAttr* pOam, 
    u16 affineIndex, 
    BOOL /*bDoubleAffine*/ 
)
{
    SDK_NULL_ASSERT( pOam );
    
    numOamUsedByManager_++;
    
    if( NNS_G2D_OAM_AFFINE_IDX_NONE == affineIndex )
    {
        return NNS_G2dEntryOamManExOam( &myOamMgrEx_, pOam, oamPriority_ );
    }else{
        return NNS_G2dEntryOamManExOamWithAffineIdx( &myOamMgrEx_, pOam, oamPriority_, affineIndex );
    }
}

//------------------------------------------------------------------------------
// OAM affine parameter registration function being used by the renderer
//
// Calls the extended OAM manager's OAM affine parameter registration function.
//
static u16 RndrCBFuncEntryOamAffine_( const MtxFx22* mtx )
{
    SDK_NULL_ASSERT( mtx );
    
    numOamAffineUsedByManager_++;
    
    return NNS_G2dEntryOamManExAffine( &myOamMgrEx_, mtx );
}




//------------------------------------------------------------------------------
// Group of functions used when the extended OAM manger registers OAM attributes and affine parameters for the external module.
// 
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Gets the number of OAM attributes that the extended OAM manager can use.
// The value obtained by this function represents the actual number of OAM attributes that can be loaded into hardware OAM.
// Be aware that this is different from the number of OAM chunks (NUM_OF_OAM_CHUNK).
// 
// When the number of registered chunks is greater than the value returned by OamMgrExCBGetCapacityOam_(), the extended OAM manager performs time-division display.
// 
static u16 OamMgrExCBGetCapacityOam_(void)
{
    return numOamReservedForManager_;
}
//------------------------------------------------------------------------------
// Gets the number of affine parameters that the extended OAM manager can use.
static u16 OamMgrExCBGetCapacityOamAffine_(void)
{
    return numOamAffineReservedForManager_;
}

static BOOL OamMgrExCBEntryOam_(const GXOamAttr* pOam, u16 /*index*/)
{
    return NNS_G2dEntryOamManagerOam( &myOamMgr_, pOam, 1 );
}

static u16 OamMgrExCBEntryOamAffine_( const MtxFx22* pMtx, u16 /*index*/ )
{
    return NNS_G2dEntryOamManagerAffine( &myOamMgr_, pMtx );
}



//------------------------------------------------------------------------------
static void LoadResources(void)
{
    NNSG2dCellDataBank* pCellBank;
    void*               pBuf;

    NNSG2dAnimBankData*          pAnimBank = NULL;
    
    NNS_G2dInitImageProxy( &myImageProxy_ );
    NNS_G2dInitImagePaletteProxy( &myPaletteProxy_ );

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
        pBuf = G2DDemo_LoadNANR( &pAnimBank, "data/Renderer_2LCD.NANR" );
        SDK_NULL_ASSERT( pBuf );

        //
        // Initialize the cell animation instance
        //
        {
            pMyCellAnim_ = G2DDemo_GetNewCellAnimation(1);
            SDK_NULL_ASSERT( pMyCellAnim_ );

            SDK_NULL_ASSERT( NNS_G2dGetAnimSequenceByIdx( pAnimBank, 0 ) );

            NNS_G2dInitCellAnimation(
                pMyCellAnim_,
                NNS_G2dGetAnimSequenceByIdx(pAnimBank, 0),
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
            &myImageProxy_ );

        // Because character data was copied into VRAM,
        // this pBuf is freed. Same below.
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
            &myPaletteProxy_ );

        G2DDemo_Free( pBuf );
    }
}

//------------------------------------------------------------------------------
// Perform the process that initializes the application
static void InitApp_()
{
    
    //
    // Load the data resources for rendering. Also, initialize the data that manages VRAM.
    //
    LoadResources();
    
    //
    // Initializes the extended OAM manager.
    // In the example below, the OAM manager module is used as a subcontractor module for the extended OAM manager's OAM registration tasks.
    // 
    // Because it is used this way, the OAM manager module is also initialized.
    //
    {
        BOOL                        bSuccess;
        

        //
        // Initializes the OAM manager.
        //
        NNS_G2dInitOamManagerModule();
        bSuccess = NNS_G2dGetNewOamManagerInstanceAsFastTransferMode( &myOamMgr_, 0, 128, NNS_G2D_OAMTYPE_MAIN );
        SDK_ASSERT( bSuccess );

        

        // Initialize the extended OAM manager instance
        bSuccess = NNS_G2dGetOamManExInstance(
                        &myOamMgrEx_,
                        sOamChunkList,
                        (u8)NUM_OF_ORDERING_TBL,
                        NUM_OF_OAM_CHUNK,
                        sOamChunk,
                        NUM_OF_AFFINE_PROXY,
                        sAffineProxy);

        SDK_ASSERT( bSuccess );
        
        // Register the group of functions called back by NNS_G2dApplyOamManExToBaseModule().
        // In this sample, the OAM manager API is used to implement the callback functions.    
        {
            NNSG2dOamExEntryFunctions   funcs;
            funcs.getOamCapacity    = OamMgrExCBGetCapacityOam_;
            funcs.getAffineCapacity = OamMgrExCBGetCapacityOamAffine_;
            funcs.entryNewOam       = OamMgrExCBEntryOam_;
            funcs.entryNewAffine    = OamMgrExCBEntryOamAffine_;

            NNS_G2dSetOamManExEntryFunctions( &myOamMgrEx_, &funcs );
        }
    }
    
    //
    // Initialize the renderer module.
    // In this sample, the extended OAM manager is used as the module to perform the OAM registration process.
    //
    {
        // renderer initialization
        NNS_G2dInitRenderer( &myRenderer_ );
        NNS_G2dInitRenderSurface( &myRenderSurface_ );
        
        // Initializes the main screen surface.
        {
            NNSG2dViewRect* pRect = &(myRenderSurface_.viewRect);

            // Display rectangle
            pRect->posTopLeft.x = 0;
            pRect->posTopLeft.y = 0;
            pRect->sizeView.x = FX32_ONE * SCREEN_WIDTH;
            pRect->sizeView.y = FX32_ONE * SCREEN_HEIGHT;

            // Callback Functions
            myRenderSurface_.pFuncOamRegister          = RndrCBFuncEntryOam_;
            myRenderSurface_.pFuncOamAffineRegister    = RndrCBFuncEntryOamAffine_;

            // display destination
            myRenderSurface_.type                      = NNS_G2D_SURFACETYPE_MAIN2D;
        }
        
        NNS_G2dAddRendererTargetSurface( &myRenderer_, &myRenderSurface_ );
        
        // Proxy registration
        NNS_G2dSetRendererImageProxy( &myRenderer_, &myImageProxy_, &myPaletteProxy_ );
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
    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_PrintInit();
        
        InitApp_();
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
        // Position of character
        static Position pos = { 64, 128 };

        static OSTick          timeForApply;
        static u16             rotation = 0x0;
        static u8              priority = 0;
        
        static NNSG2dOamExDrawOrder drawOrderType = NNSG2D_OAMEX_DRAWORDER_BACKWARD;
        
        // Input process
        {
            G2DDemo_ReadGamePad();

            // Move character
            if( G2DDEMO_IS_PRESS( PAD_KEY_LEFT ) )
            {
                pos.x--;
            }
            if( G2DDEMO_IS_PRESS( PAD_KEY_RIGHT ) )
            {
                pos.x++;
            }
            if( G2DDEMO_IS_PRESS( PAD_KEY_UP ) )
            {
                pos.y--;
            }
            if( G2DDEMO_IS_PRESS( PAD_KEY_DOWN ) )
            {
                pos.y++;
            }
            
            if( G2DDEMO_IS_PRESS( PAD_BUTTON_L ) )
            {
                rotation -= 0xFF;
            }
            if( G2DDEMO_IS_PRESS( PAD_BUTTON_R ) )
            {
                rotation += 0xFF;
            }
            
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_START ) )
            {
                //
                // Switch the drawing order of the extended OAM manager
                //
                if( drawOrderType == NNSG2D_OAMEX_DRAWORDER_BACKWARD )
                {   
                    drawOrderType = NNSG2D_OAMEX_DRAWORDER_FORWARD;
                }else{
                    drawOrderType = NNSG2D_OAMEX_DRAWORDER_BACKWARD;
                }
                NNSG2d_SetOamManExDrawOrderType( &myOamMgrEx_, drawOrderType );
            }
            
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_A ) )
            {
                if( numOamReservedForManager_+1 < 127 )
                {
                    numOamReservedForManager_++;
                }
            }
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_B ) )
            {
                if( numOamReservedForManager_ > 1 )
                {
                    numOamReservedForManager_--;
                }
            }
            
            
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_X ) )
            {
                if( numOamAffineReservedForManager_+1 < 31)
                {
                    numOamAffineReservedForManager_++;
                }
            }
            
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_Y ) )
            {
                if( numOamAffineReservedForManager_ > 0 )
                {
                    numOamAffineReservedForManager_--;
                }
            }
            
            if( G2DDEMO_IS_TRIGGER( PAD_BUTTON_SELECT ) )
            {
                if( priority == 0 )
                {
                    priority = 1;
                }else{
                    priority = 0;
                }
            }
        }
        
        // Render
        {
            NNS_G2dBeginRendering( &myRenderer_ );
                NNS_G2dPushMtx();
                    NNS_G2dTranslate( FX32_ONE*pos.x, FX32_ONE*pos.y, 0 );                        
                    // Left
                    oamPriority_ = 0;
                    NNS_G2dTranslate( FX32_ONE*30, 0, 0 );
                    NNS_G2dDrawCellAnimation( pMyCellAnim_ );
                    
                    // Center
                    oamPriority_ = 0;
                    NNS_G2dTranslate( FX32_ONE*30, 0, 0 );
                    NNS_G2dScale( FX32_ONE + (FX32_ONE >> 1), FX32_ONE + (FX32_ONE >> 1), 0 );
                    NNS_G2dDrawCellAnimation( pMyCellAnim_ );
                    
                    // Right
                    oamPriority_ = priority;
                    NNS_G2dTranslate( FX32_ONE*30, 0, 0 );
                    if( rotation != 0 )
                    {
                        NNS_G2dRotZ( FX_SinIdx( rotation ), FX_CosIdx( rotation ) );
                    }
                    NNS_G2dDrawCellAnimation( pMyCellAnim_ );
                NNS_G2dPopMtx();
            NNS_G2dEndRendering();
        }

        // Output debug string
        {
            G2DDemo_PrintOutf(0, 0, "OAM(used/Max)=(%3d/%3d)", 
                                    numOamUsedByManager_, 
                                    numOamReservedForManager_ );
                                    
            G2DDemo_PrintOutf(0, 1, "AFF(used/Max)=(%3d/%3d)", 
                                    numOamAffineUsedByManager_, 
                                    numOamAffineReservedForManager_ );        
                  
            G2DDemo_PrintOutf(0, 2, "DrawOrderType = %s", 
                                    (drawOrderType == NNSG2D_OAMEX_DRAWORDER_BACKWARD) ?
                                    "Backward":
                                    "Forward " );        
        }
        
        // Wait for V-Blank
        SVC_WaitVBlankIntr();

        //
        // Write buffer contents to HW
        //
        {
            timeForApply = OS_GetTick();
            // Write display information
            G2DDemo_PrintApplyToHW();

            
            // Write OBJ
            NNS_G2dApplyOamManExToBaseModule( &myOamMgrEx_ );

            // Reset the extended OAM manager.
            // Even after reset, the information relating to time division is retained.
            NNS_G2dResetOamManExBuffer( &myOamMgrEx_ );
            timeForApply = OS_GetTick() - timeForApply;
            
            NNS_G2dApplyOamManagerToHW( &myOamMgr_ );
            NNS_G2dResetOamManagerBuffer( &myOamMgr_ );
            
            numOamUsedByManager_         = 0;
            numOamAffineUsedByManager_   = 0;
        }
        
        // Update the animation.
        NNS_G2dTickCellAnimation( pMyCellAnim_, FX32_ONE );
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
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );    // Checking VBlank interrupt
}


