/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - OamManagerEx2
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
//      This sample uses the extended OAM manager to render a cell animation.
//      It combines use of the extended OAM manger and the OAM manager.
//
//  Using the Demo
//      A: Plays back an explosion animation.
//
// ============================================================================


#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


//-----------------------------------------------------------------------------
//
// Effect structure
//
struct MyEffectObject;
typedef struct MyEffectObject
{
    
    NNSG2dCellAnimation*        pCellAnm;   // Cell animation
    NNSG2dFVec2                 pos;        // Position
    BOOL                        bActive;    // Is it active?
    struct MyEffectObject*      pNext;      // Next effect
    
}MyEffectObject;


//-----------------------------------------------------------------------------
#define LENGTH_EFFECT_ARRAY     32              // Length of array used for the effect structure pool

#define NUM_OF_OAM              16              // Number of OAMs controlled by OAM Manager
#define NUM_OF_AFFINE           (NUM_OF_OAM/4)  // Number of affine parameters controlled by OAM Manager

#define TEX_BASE                0x0             // Texture base address
#define TEX_PLTT_BASE           0x0             // Texture palette base address


#define NUM_OF_OAM_CHUNK        256             // Number of OAM chunks
                                                // (Maximum number of OAMs that can be registered for the extended OAM manager)
#define NUM_OF_CHUNK_LIST      (u8) 1           // Priority
#define NUM_OF_AFFINE_PROXY     1               // Number of affine proxies
                                                // (Maximum number of affine parameters that can be registered to the OAM manager)
                                                //  

//-----------------------------------------------------------------------------
// Global Variables

static MyEffectObject    effectArray_[LENGTH_EFFECT_ARRAY]; // Array used for the effect structure pool

static MyEffectObject*   pActiveEffectListHead = NULL;      // List of active effects
static MyEffectObject*   pFreeEffectListHead   = NULL;      // List of inactive effects

// Chunk
// The container for the list storing the OAM.
static NNSG2dOamChunk           oamChunk_[NUM_OF_OAM_CHUNK];

// Chunk List
// Maintains a chunk list for each display priority level.
// The same number of lists are required as the number of display priority levels used.
static NNSG2dOamChunkList       oamChunkList_[NUM_OF_CHUNK_LIST];

// Affine Parameter Proxy
// Extends the AffineIndex determination until affine parameters are stored and reflected in the hardware.
// Not used in this demo.
static NNSG2dAffineParamProxy   affineProxy_[NUM_OF_AFFINE_PROXY];


//
// OAM manager, extended OAM manager
//
static NNSG2dOamManagerInstance         oamManager_;
static NNSG2dOamManagerInstanceEx       oamExManager_;


    
//------------------------------------------------------------------------------
// Prototype Declarations


/*---------------------------------------------------------------------------*
  Name:         CallBackFunction

  Description:  This function is called back from the animation controller.
                
                Pointers to effect structures that playback animation are set as parameters.
                
                The active flag of this effect structure is set to FALSE inside the callback function.
                

  Arguments:    param:          The user parameters specified when the callback function was registered.
                                
                currentFrame:   When the callback type is NNS_G2D_ANMCALLBACKTYPE_LAST_FRM, the animation controller's currentTime is passed in fx32 format.
                                
                                
                                
                                In all other cases, the current frame number will be passed in u16 format.
                                

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CallBackFunction( u32 param, fx32 currentFrame )
{
#pragma unused( currentFrame )
    
    MyEffectObject*     pE = (MyEffectObject*)param;
    pE->bActive = FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         CallBackGetOamCapacity

  Description:  Callback invoked from the extended OAM manager; conveys the number of OAMs available for use in the extended OAM manager.
                

  Arguments:    None.

  Returns:      Number of available OAMs.
 *---------------------------------------------------------------------------*/
static u16 CallBackGetOamCapacity(void)
{
    return NNS_G2dGetOamManagerOamCapacity( &oamManager_ );
}



/*---------------------------------------------------------------------------*
  Name:         CallBackGetAffineCapacity

  Description:  Callback invoked from the extended OAM manager; conveys the number of affine parameter storage regions available for use in the extended OAM manager.
                

  Arguments:    None.

  Returns:      Number of usable affine parameter storage areas.
 *---------------------------------------------------------------------------*/
static u16 CallBackGetAffineCapacity(void)
{
    return 0;
}



/*---------------------------------------------------------------------------*
  Name:         CallBackEntryNewOam

  Description:  This function is called back from the extended OAM manager and outputs the OAMs.

  Arguments:    pOam:   Pointer to the OAM to be output.
                index:  OBJ number of the pOam.

  Returns:      If the OAM output succeeded or not.
 *---------------------------------------------------------------------------*/
static BOOL CallBackEntryNewOam(const GXOamAttr* pOam, u16 index )
{
#pragma unused( index )
    return NNS_G2dEntryOamManagerOam( &oamManager_, pOam, 1 );
}



/*---------------------------------------------------------------------------*
  Name:         CallBackEntryNewAffine

  Description:  This function is called back from the extended OAM manager and registers affine parameters.
                

  Arguments:    pMtx:   Affine parameters to be registered.
                index:  pMtx AffineIndex

  Returns:      pMtx AffineIndex actually registered.
 *---------------------------------------------------------------------------*/
static u16 CallBackEntryNewAffine( const MtxFx22* pMtx, u16 index )
{
#pragma unused( pMtx )
#pragma unused( index )
	// This function always returns invalid values.
    return NNS_G2D_OAMEX_HW_ID_NOT_INIT;
}

//------------------------------------------------------------------------------
static void InitApp()
{
    BOOL        result = TRUE;
    
    //
    // Initializes the OAM Manager.
    //
    {
        // Initialization of the OAM Manager module.
        NNS_G2dInitOamManagerModule();
        // Initializes the OAM manager instance
        result &= NNS_G2dGetNewOamManagerInstanceAsFastTransferMode( &oamManager_, 
                                                                     0, 
                                                                     NUM_OF_OAM,
                                                                     NNS_G2D_OAMTYPE_MAIN );    
    }
    
    
    // Initialize the extended OAM manager instance
    {
        NNSG2dOamExEntryFunctions       funcs;// !
        
        result &= NNS_G2dGetOamManExInstance( &oamExManager_, 
                                                oamChunkList_, NUM_OF_CHUNK_LIST,
                                                NUM_OF_OAM_CHUNK, oamChunk_,
                                                NUM_OF_AFFINE_PROXY, affineProxy_ );

        // Register the group of functions called back by NNS_G2dApplyOamManExToBaseModule().
        funcs.getOamCapacity    = CallBackGetOamCapacity;
        funcs.getAffineCapacity = CallBackGetAffineCapacity;
        funcs.entryNewOam       = CallBackEntryNewOam;
        funcs.entryNewAffine    = CallBackEntryNewAffine;

        NNS_G2dSetOamManExEntryFunctions( &oamExManager_, &funcs );
    }
    SDK_ASSERT( result );
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  This function carries out the following initializations:
                  Reading of cell animation data
                  Initialization of cell animation
                  Reading of character data and palette data

  Arguments:    ppAnimBank: Pointer for receiving the pointer to the cell animation data.
                            
                ppCellAnim: Pointer for receiving the pointer to the cell animation.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources_( )
{
    //
    // Initialization of cell related resources.
    //
    {
        void* pBuf;
        
        NNSG2dCellDataBank*          pCellBank  = NULL;
        NNSG2dAnimBankData*          pAnimBank  = NULL;
        
        // Load cell data and cell animation data.
        // These pBuf pointers are not deallocated because cell animation data is used in main memory until the end.
        // 
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/bomb.NCER" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNANR( &pAnimBank, "data/bomb.NANR" );
        SDK_NULL_ASSERT( pBuf );
        SDK_NULL_ASSERT( NNS_G2dGetAnimSequenceByIdx( pAnimBank, 0) );
        
        //
        // Initialize the cell animation instance
        //
        {
            
            NNSG2dCellAnimation*         pCellAnm = NULL;
            int i = 0;
            
            for( i = 0; i < LENGTH_EFFECT_ARRAY; i++ )
            {
                effectArray_[i].pCellAnm = G2DDemo_GetNewCellAnimation(1);
                pCellAnm = effectArray_[i].pCellAnm;
                
                
                SDK_NULL_ASSERT( pCellAnm );
                
                NNS_G2dInitCellAnimation( pCellAnm, NNS_G2dGetAnimSequenceByIdx( pAnimBank, 0), pCellBank );
                
                //
                // Callback setup
                //
                NNS_G2dSetAnimCtrlCallBackFunctor(
                    NNS_G2dGetCellAnimationAnimCtrl(pCellAnm),
                    NNS_G2D_ANMCALLBACKTYPE_LAST_FRM,
                    (u32)&effectArray_[i],
                    CallBackFunction );
            }
        }
    }

    //
    // Initialization related to VRAM
    //
    {
        void* pBuf;

        // Load character data for 2D
        {
            NNSG2dCharacterData*        pCharData;

            pBuf = G2DDemo_LoadNCGR( &pCharData, "data/bomb.NCGR" );
            SDK_NULL_ASSERT( pBuf );
            
            // Loading for 2D Graphics Engine
            DC_FlushRange( pCharData->pRawData, pCharData->szByte );
            GX_LoadOBJ( (void*)pCharData->pRawData, TEX_BASE, pCharData->szByte );
                
            // Because character data was copied into VRAM,
            // this pBuf is freed. Same below.
            G2DDemo_Free( pBuf );
        }

        //------------------------------------------------------------------------------
        // Load palette data
        {
            NNSG2dPaletteData*        pPlttData;

            pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/bomb.NCLR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading for 2D Graphics Engine
            DC_FlushRange( pPlttData->pRawData, pPlttData->szByte );
            GX_LoadOBJPltt( (void*)pPlttData->pRawData, TEX_PLTT_BASE, pPlttData->szByte );
            
            G2DDemo_Free( pBuf );
        }
    }
}

//------------------------------------------------------------------------------
// Reset the effect registration list for the scene
static void ResetEffectList_()
{
    int i;
    for( i = 0; i < LENGTH_EFFECT_ARRAY - 1; i++ )
    {
        effectArray_[i].bActive = FALSE;
        effectArray_[i].pNext = &effectArray_[i+1];
        NNS_G2dSetCellAnimationCurrentFrame( effectArray_[i].pCellAnm, 0 );
    }
    effectArray_[LENGTH_EFFECT_ARRAY - 1].bActive = FALSE;
    effectArray_[LENGTH_EFFECT_ARRAY - 1].pNext = NULL;
    
    pActiveEffectListHead   = NULL;
    pFreeEffectListHead     = effectArray_;
}

//------------------------------------------------------------------------------
// Add a new effect to the scene
static void AddNewEffect_()
{
    MyEffectObject*             pE = pFreeEffectListHead;
    if( pE )
    {
        SDK_ASSERT( pE->bActive == FALSE );
        
        pE->bActive = TRUE;
        pE->pos.x = (fx32)((OS_GetTick() % 255) * FX32_ONE);
        pE->pos.y = (fx32)((OS_GetTick() % 192) * FX32_ONE);
        
        
        // Remove from the free list
        pFreeEffectListHead = pE->pNext;
        
        // Add to the beginning of the active list
        pE->pNext             = pActiveEffectListHead;
        pActiveEffectListHead = pE;
    }
}

//------------------------------------------------------------------------------
// Updates the scene.
static void TickScene_()
{
    MyEffectObject*             pE = pActiveEffectListHead;
    MyEffectObject*             pPreE = NULL;
    MyEffectObject*             pNext = NULL;
    
    
    while( pE )
    {    
        // Update of the cell animation
        NNS_G2dTickCellAnimation( pE->pCellAnm, FX32_ONE );
        pE      = pE->pNext;
    }
    
    pE = pActiveEffectListHead;
    while( pE )
    {
        pNext = pE->pNext;
        if( !pE->bActive )    
        {    
            // Remove from the active list
            if( pPreE )
            {
                pPreE->pNext          = pNext;
            }else{
                pActiveEffectListHead = pNext;
            }
            // Add to the beginning of the free list
            pE->pNext           = pFreeEffectListHead;
            pFreeEffectListHead = pE;
            
            pE      = pNext;
        }else{
            pPreE   = pE;
            pE      = pNext;
        }
    }   
}

//------------------------------------------------------------------------------
// Render one explosion effect.
static u16 DrawOneEffect_( const MyEffectObject* pE )
{
    {
        int i;
        u16                     numOamBuffer;       // number of remaining OAMs
        static GXOamAttr        temp[NUM_OF_OAM];   // Temporary OAM buffer
        const NNSG2dCellData*   pCell;              // Cell to display

        pCell = NNS_G2dGetCellAnimationCurrentCell( pE->pCellAnm );
        SDK_NULL_ASSERT( pCell );

        //
        // Writes an OBJ list equivalent to the cell to the temporary buffer.
        //
        numOamBuffer = NNS_G2dMakeCellToOams(
                            temp,           // output Oam buffer
                            NUM_OF_OAM,     // output buffer length
                            pCell,          // Cell to output
                            NULL,           // affine conversion
                            &pE->pos,       // offset position
                            NULL,           // Affine Index
                            FALSE );        // double affine?

        SDK_ASSERT( numOamBuffer < NUM_OF_OAM);

        for( i = 0; i < numOamBuffer; i++ )
        {
            if( !NNS_G2dEntryOamManExOam( &oamExManager_, &temp[i], 0 ) )
            {   
                return (u16)i;
            }
        }
        return numOamBuffer;
    }
    
}

//------------------------------------------------------------------------------
// Render the scene.
// The number of OAMs that have been used is the return value.
//
static u16 DrawScene_()
{
    u16     totalOam = 0;
    MyEffectObject*     pE;
    pE = pActiveEffectListHead;
    while( pE )
    {
        if( pE->bActive )
        {
            totalOam += DrawOneEffect_( pE );
        }
        pE = pE->pNext;
    }
    
    return totalOam;
}

//------------------------------------------------------------------------------
// Count the number of explosion effects
static u16 GetNumEffect_( MyEffectObject* pListHead )
{
    u16 count = 0;
    MyEffectObject*     pE = pListHead;
    while( pE )
    {
        count++;
        pE = pE->pNext;
    }
    return count;
}

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_PrintInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        
        InitApp();
        LoadResources_();
        ResetEffectList_();
    }

    // Start display
    {
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    //----------------------------------------------------
    // Main loop
    while( TRUE )
    {
        static OSTick               time;                           // Used in OAM Manager performance measurement.
        u16     totalOam = 0;
        //
        // Pad handling.
        //
        {
            G2DDemo_ReadGamePad();
            // Adds new effects to the scene.
            if (G2DDEMO_IS_TRIGGER(PAD_BUTTON_A ))
            {
                AddNewEffect_();
            }
            
            if (G2DDEMO_IS_TRIGGER(PAD_BUTTON_B ))
            {
                ResetEffectList_();
            }
            
            if (G2DDEMO_IS_TRIGGER(PAD_BUTTON_L ))
            {
                OS_Printf( "num-active = %d\n", GetNumEffect_( pActiveEffectListHead ) );
                OS_Printf( "num-free   = %d\n", GetNumEffect_( pFreeEffectListHead ) );
            }
        }
        
        // Render the scene.
        // The number of OAMs that have been used is the return value.
        totalOam = DrawScene_();
        

        //
        // Debug character string drawing
        //
        {
            G2DDemo_PrintOutf( 0, 0, "Total-OAM:%2d", totalOam );
            G2DDemo_PrintOutf( 0, 2, "Push A to show cell animation.", totalOam );
        }
        
        //
        // Write manager contents to HW.
        // Also it measures the time required to do this and outputs it as debug information.
        //            
        {
            // Wait for V-Blank
            G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
            SVC_WaitVBlankIntr();
            
            time = OS_GetTick();
            {
                NNS_G2dApplyOamManExToBaseModule( &oamExManager_ );
                NNS_G2dResetOamManExBuffer( &oamExManager_ );
                
                NNS_G2dApplyOamManagerToHW( &oamManager_ );
                NNS_G2dResetOamManagerBuffer( &oamManager_ );
            }
            time = OS_GetTick() - time;
        }
        
        //
        // Writes the animation information to HW   
        //
        G2DDemo_PrintApplyToHW();   
        
        //
        // Updates the scene.
        // 
        TickScene_();   
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

