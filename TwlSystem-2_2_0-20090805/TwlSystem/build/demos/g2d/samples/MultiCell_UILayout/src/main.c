/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - MultiCell_UILayout
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
//      This sample processes the user interface (UI) for a game application, using multicells.
//      
//
//  Using the Demo
//      +Control Pad: Cursor movement
//      
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

// Header file that defines other names for animation sequence numbers output by the binary converter (using the -lbl option).
// 
#include "../data/win_pnl_NANR_LBLDEFS.h"
#include "g2d_demolib.h"


#define NUM_OF_OAM              128                 // Number of OAMs controlled by OAM Manager
#define NUM_OF_AFFINE           (NUM_OF_OAM/4)      // Number of affine parameters controlled by OAM Manager

// Multicell position
#define MY_MC_BASE_POS_X        128
#define MY_MC_BASE_POS_Y        96
// Multicell display position
static const NNSG2dFVec2           MULTICELL_POSITION 
            = {MY_MC_BASE_POS_X << FX32_SHIFT, MY_MC_BASE_POS_Y << FX32_SHIFT};
            
            
// Meaning of the animation frame number used in a cell animation as defined by the user
#define APP_UI_ANIMFRAME_IDX_NORMAL 0   // Normal status
#define APP_UI_ANIMFRAME_IDX_PUSHED 1   // Button being held down


//
// MultiCellAnimation instance
// Initializes instances using two different methods.
// 
static NNSG2dMultiCellAnimation   mcAnim;


// Temporary OAM attribute buffer
static GXOamAttr                tempOamBuffer[NUM_OF_OAM];

//
// Multicell bank
//
static NNSG2dMultiCellDataBank*        pMCBank = NULL;     // Multicell data
static NNSG2dCellDataBank*             pCellBank = NULL;   // Cell data
static NNSG2dCellAnimBankData*         pAnimBank = NULL;   // Cell animation
static NNSG2dMultiCellAnimBankData*    pMCABank = NULL;    // Multicell animation

// Position of the pointer
static NNSG2dSVec2          pointerPos_ = { 0, 0 };
static BOOL                 bHit_       = FALSE;


//------------------------------------------------------------------------------
// Prototype Declarations

static void InitOamManager(NNSG2dOamManagerInstance* pOamManager);
static void LoadResources();
static void ProcessInput(fx32* pAnimSpeed, NNSG2dAnimController* pAnimCtrl);
void VBlankIntr(void);



/*---------------------------------------------------------------------------*
  Name:         InitOamManager

  Description:  Initializes the OAM manager system and OAM manager.

  Arguments:    pOamManager:    Pointer to the OAM manager to be initialized.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitOamManager(NNSG2dOamManagerInstance* pOamManager)
{
    BOOL    result;

    // Initializes the Oam Manager system
    NNS_G2dInitOamManagerModule();

    result = NNS_G2dGetNewManagerInstance(
                pOamManager,
                0,
                NUM_OF_OAM - 1,
                NNS_G2D_OAMTYPE_MAIN );
    SDK_ASSERT( result );

    result = NNS_G2dInitManagerInstanceAffine( pOamManager, 0, NUM_OF_AFFINE - 1 );
    SDK_ASSERT( result );
}



/*---------------------------------------------------------------------------*
  Name:         InitMultiCellInstance_

  Description:  Initializes multicell instance.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitMultiCellInstance_
(
    NNSG2dMultiCellAnimation*       pMCAnim,
    NNSG2dCellAnimBankData*         pAnimBank,
    NNSG2dCellDataBank*             pCellBank,
    NNSG2dMultiCellDataBank*        pMCBank,
    NNSG2dMultiCellAnimBankData*    pMCABank,
    NNSG2dMCType                    mcType
)
{
    // Get work memory of the size needed for initialization
    const u32 szWork = NNS_G2dGetMCWorkAreaSize( pMCBank, mcType );
    void* pWorkMem   = G2DDemo_Alloc( szWork );
    
    // Initialization
    NNS_G2dInitMCAnimationInstance( pMCAnim, 
                                    pWorkMem, 
                                    pAnimBank, 
                                    pCellBank, 
                                    pMCBank, 
                                    mcType );
    
    SDK_NULL_ASSERT( pMCAnim );
    
    // Sets the sequence to play to the multicell animation
    {
        const NNSG2dMultiCellAnimSequence* pSequence = NULL;    
        // Obtains the sequence to play
        pSequence = NNS_G2dGetAnimSequenceByIdx( pMCABank, 0 );
        SDK_ASSERT( pSequence );

        NNS_G2dSetAnimSequenceToMCAnimation( pMCAnim, pSequence);
    }
}

/*---------------------------------------------------------------------------*
  Name:         IsRectIntersect_

  Description:  Determine whether or not the rectangle is colliding with the point.

  Arguments:    pRect:          Rectangle for which the determination is to be made
                
                pPointerPos:   Point for which the determination is to be made
                
                pRectPos:      Rectangle position

  Returns:      Do they collide? Returns TRUE if they collide.
 *---------------------------------------------------------------------------*/
static BOOL IsRectIntersect_
( 
    const NNSG2dCellBoundingRectS16*    pRect,
    const NNSG2dSVec2*                  pPointerPos, 
    const NNSG2dSVec2*                  pRectPos 
)
{
    NNSG2dSVec2 posPtr = (*pPointerPos);
    
    posPtr.x -= pRectPos->x; 
    posPtr.y -= pRectPos->y; 
    
    if( pRect->maxX >= posPtr.x && 
        pRect->minX <= posPtr.x )
    {
        if( pRect->maxY >= posPtr.y && 
            pRect->minY <= posPtr.y )
        {
            // Objects collide
            return TRUE;        
        }
    }
    // Objects do not collide
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         OnButtonHitCallBack_

  Description:  This callback function performs collision detection for the UI buttons.
                The result is used as an argument to NNS_G2dTraverseMCNodes().
                
                This function is used as the callback function invoked for all of the nodes within the multicell.
                

  Arguments:    userParamater:   User parameters.
                                 In this sample the value for a pointer to a flag variable that records whether a collision occurred is stored.
                                 
                
                pNodeData:      The node information
                                 The cell animation position information, etc., within the cell animation is stored.
                
                pCellAnim:      Cell animation (used during cell animation)
                
                nodeIdx:        Node number

  Returns:      Should calling further callbacks be stopped?
                Returns FALSE if subsequent callback calls are to be stopped.
 *---------------------------------------------------------------------------*/
static BOOL OnButtonHitCallBack_
( 
    u32                                 userParamater,
    const NNSG2dMultiCellHierarchyData* pNodeData,
    NNSG2dCellAnimation*                pCellAnim, 
    u16                                 nodeIdx 
)
{
#pragma unused( nodeIdx )

    NNSG2dSVec2       rectPos;
    
    const NNSG2dCellData*       pCD      = NNS_G2dGetCellAnimationCurrentCell( pCellAnim );
    const NNSG2dAnimController* pAnmCtrl = NNS_G2dGetCellAnimationAnimCtrl( pCellAnim );
    const NNSG2dAnimSequence*   pAnmSeq  = NNS_G2dGetAnimCtrlCurrentAnimSequence( pAnmCtrl );
    const u16                   seqIdx   = NNS_G2dGetAnimSequenceIndex( pAnimBank, pAnmSeq );
    
    const NNSG2dCellBoundingRectS16* pRect = NNS_G2dGetCellBoundingRect( pCD );
    
    rectPos.x = (s16)(pNodeData->posX + MY_MC_BASE_POS_X);
    rectPos.y = (s16)(pNodeData->posY + MY_MC_BASE_POS_Y);
    
    // Does this cell animation need collision detection?
    // 
    // The animation sequence numbers' defines, denoted in the ../data/win_pnl_NANR_LBLDEFS.h file, are used.
    // 
    // Specify the -lbl option to the converter to output this data.
    if( seqIdx != NANR_win_pnl_Win_bse )
    {
        if( IsRectIntersect_( pRect, &pointerPos_, &rectPos ) )
        {
            // A pointer to a flag is passed as a user parameter
            // The flag is updated here.
            BOOL* pHitFlag = (BOOL*)userParamater;
            OS_Printf("hit index = %d\n", seqIdx );
            *pHitFlag = TRUE;
            
            // Set pressed status for the cell animation image
            NNS_G2dSetCellAnimationCurrentFrame( pCellAnim, APP_UI_ANIMFRAME_IDX_PUSHED );            
            
            
            // Stop calling further callbacks.
            return FALSE;            
        }else{
            // Set normal status for the cell animation image
            NNS_G2dSetCellAnimationCurrentFrame( pCellAnim, APP_UI_ANIMFRAME_IDX_NORMAL );
        }
    }
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  This function constructs a multicell animation and loads character and palette data.
                

  Arguments:    ppMCAnim:   Pointer that receives the pointer to the constructed multicell animation.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources()
{
    void* pBuf;

    //
    // Multicell-related Initialization
    //
    {
        
        // Load the cell data, cell animation, multicell data and multicell animation.
        // 
        // They are used in main memory until the end, so pBuf is not released.
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/win_pnl.NCER" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNANR( &pAnimBank, "data/win_pnl.NANR" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNMCR( &pMCBank, "data/win_pnl.NMCR" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNMAR( &pMCABank, "data/win_pnl.NMAR" );
        SDK_NULL_ASSERT( pBuf );

        //
        // Initializes the multicell animation instances
        // Initializes two instances using different methods.
        //
        InitMultiCellInstance_( &mcAnim,
                                pAnimBank,
                                pCellBank,
                                pMCBank,
                                pMCABank,
                                NNS_G2D_MCTYPE_SHARE_CELLANIM  );
    }

    //
    // VRAM-related Initialization
    //
    {
        // Loading NCG (Character Data)
        {
            NNSG2dCharacterData*        pCharData = NULL;

            pBuf = G2DDemo_LoadNCGR( &pCharData, "data/win_pnl.NCGR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading for 2D Graphics Engine
            DC_FlushRange( pCharData->pRawData, pCharData->szByte );
            GX_LoadOBJ( (void*)pCharData->pRawData, 0, pCharData->szByte );

            // Because character data was copied into VRAM,
            // this pBuf is freed. Same below.
            G2DDemo_Free( pBuf );
        }

        // Loading NCL (Palette Data)
        {
            NNSG2dPaletteData*        pPltData = NULL;

            pBuf = G2DDemo_LoadNCLR( &pPltData, "data/win_pnl.NCLR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading for 2D Graphics Engine
            DC_FlushRange( pPltData->pRawData, pPltData->szByte );
            GX_LoadOBJPltt( (void*)pPltData->pRawData, 0, pPltData->szByte );

            G2DDemo_Free( pBuf );
        }
    }
}


//--------------------------------------------------------------------------------------------------------
// Render application
static void AppDraw
( 
    const NNSG2dMultiCellAnimation* pMcAnim, 
    NNSG2dOamManagerInstance*       pOamMgr 
)
{
    u16                             numOamDrawn = 0;                  
    // Render the pointer
    {
        const NNSG2dCellData* pCD = NNS_G2dGetCellDataByIdx( pCellBank, 0xd );
        NNSG2dFVec2           pointerPos; 
        
        pointerPos.x = pointerPos_.x << FX32_SHIFT;
        pointerPos.y = pointerPos_.y << FX32_SHIFT;
        
        //
        // Writes an OBJ list equivalent to multicell to temporary buffer.
        //
        numOamDrawn = NNS_G2dMakeCellToOams(
                            tempOamBuffer,              // Output Oam buffer
                            (u16)(NUM_OF_OAM - numOamDrawn),   // Output buffer length
                            pCD,                        // Cell to output
                            NULL,                       // Affine conversion
                            &pointerPos,                // Offset position
                            NULL,                       // Affine Index
                            FALSE );                    // Double affine?

        SDK_ASSERT( numOamDrawn < NUM_OF_OAM );

        // Registers OBJ list to OAM Manager
        (void)NNS_G2dEntryOamManagerOam( pOamMgr, tempOamBuffer, numOamDrawn );
    }
    
    // Multicell rendering
    //
    // Renders multicell with a low-level rendering method.
    // There is also a rendering method that uses the upper-level module's G2dRenderer.
    //
    //
    {
        
        
        const NNSG2dMultiCellInstance*  pMultiCell = &(pMcAnim->multiCellInstance); // Multicell to display
        SDK_NULL_ASSERT( pMultiCell );

        //
        // Writes an OBJ list equivalent to multicell to a temporary buffer.
        //
        numOamDrawn = NNS_G2dMakeSimpleMultiCellToOams(
                            tempOamBuffer,       // Output Oam buffer
                            NUM_OF_OAM,          // Output buffer length
                            pMultiCell,          // Output multicell
                            NULL,                // Affine conversion
                            &MULTICELL_POSITION, // Offset position
                            NULL,                // Affine Index
                            FALSE );             // Double affine?

        SDK_ASSERT( numOamDrawn < NUM_OF_OAM );

        // Registers OBJ list to OAM Manager
        (void)NNS_G2dEntryOamManagerOam( pOamMgr, tempOamBuffer, numOamDrawn );
    }
    
    
}



   

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    NNSG2dOamManagerInstance    oamManager;     // Oam manager instance
    
        
    // Initialize app.
    {
        G2DDemo_CommonInit();
        G2DDemo_PrintInit();
        InitOamManager( &oamManager );
        LoadResources();
        
        // Start display
        {
            SVC_WaitVBlankIntr();
            GX_DispOn();
            GXS_DispOn();
        }
    }

    //----------------------------------------------------
    // Main loop
    while( TRUE )
    {
        NNSG2dMultiCellAnimation*      pMcAnim      = &mcAnim;
        
        //
        //  ------ Pad handling.
        //
        {
            G2DDemo_ReadGamePad();

            // change animation speed.
            {
                if (G2DDEMO_IS_PRESS(PAD_KEY_UP))
                {
                    pointerPos_.y -= 1;
                }
                
                if (G2DDEMO_IS_PRESS(PAD_KEY_DOWN))
                {
                    pointerPos_.y += 1;
                }
                
                if (G2DDEMO_IS_PRESS(PAD_KEY_LEFT))
                {
                    pointerPos_.x -= 1;
                }
                
                if (G2DDEMO_IS_PRESS(PAD_KEY_RIGHT))
                {
                    pointerPos_.x += 1;
                }
            }
        }
        
        //  ------ Collision Detection
        // For each node within a multicell, collision detection and the calling of a callback that processes the reaction at collision occurs.
        // 
        // Pass a pointer to the flag as a user parameter and update the flag if a collision has been detected within the callback.
        // 
        bHit_       = FALSE;
        NNS_G2dTraverseMCNodes( &pMcAnim->multiCellInstance, OnButtonHitCallBack_, (u32)&bHit_ );

        //  ------ Render
        AppDraw( pMcAnim, &oamManager );
        
        
        
        // ------ Draw the debug string
        {
            if( bHit_ )
            {
                G2DDemo_PrintOutf(0, 0, "Hit");
            }else{
                G2DDemo_PrintOutf(0, 0, "   ");
            }
        }

        // ------ Wait for V-Blank
        SVC_WaitVBlankIntr();
        G2DDemo_PrintApplyToHW();   // Writes the debug character string data to HW.

        // ------ Write manager contents to HW
        {
            NNS_G2dApplyOamManagerToHW          ( &oamManager );
            NNS_G2dResetOamManagerBuffer        ( &oamManager );
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

