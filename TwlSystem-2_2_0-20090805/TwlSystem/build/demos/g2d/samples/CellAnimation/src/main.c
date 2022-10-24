/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - CellAnimation
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
//      Example of rendering and updating cell animation
//
//  Using the Demo
//      +Control Pad key, up/down: Change playback speed
//      +Control Pad left/right: Change playback sequence
//      A: Toggle between play and stop
//      B: Toggle between forward and reverse playback
//      X: Switch callback type
//      Y: Restart playback
//  Description
//      Plays back cell animation. Also indicates various types of playback control features and callback calling features.
//      
// ============================================================================


#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


//------------------------------------------------------------------------------
// Constants
#define NUM_OF_OAM              48                      // Number of OAMs controlled by OAM Manager

#define ANIM_SPEED_UNIT         (FX32_ONE >> 3)         // Difference in animation speed

#define CALLBACK_PARAM          0x12345678              // Callback function parameter
#define CALL_CALLBACK_FRAME     3                       // The frame number that invokes the callback function for NNS_G2D_ANMCALLBACKTYPE_SPEC_FRM.
                                                        // 
                                                        
#define TEX_BASE                0x0                     // Texture base address
#define TEX_PLTT_BASE           0x0                     // Texture palette base address

//------------------------------------------------------------------------------
// Global members
static NNSG2dOamManagerInstance objOamManager;          // OAM manager for OBJ output

static NNSG2dImageProxy         sImageProxy;            // Character/texture proxy for cell
static NNSG2dImagePaletteProxy  sPaletteProxy;          // Palette proxy for cell

// Callback type names
static const char* CALL_BACK_TYPE_NAME[] = {
    "NONE    ",     // NNS_G2D_ANMCALLBACKTYPE_NONE
    "LAST_FRM",     // NNS_G2D_ANMCALLBACKTYPE_LAST_FRM
    "SPEC_FRM",     // NNS_G2D_ANMCALLBACKTYPE_SPEC_FRM
    "EVER_FRM",     // NNS_G2D_ANMCALLBACKTYPE_EVER_FRM
};

// Cell display location
static const NNSG2dFVec2        CELL_POSITION = {120 << FX32_SHIFT, 120 << FX32_SHIFT};

// Number of times the animation callback has been called
static int                      callCount = 0;

// Animation Callback Type
static NNSG2dAnmCallbackType    callBackType = NNS_G2D_ANMCALLBACKTYPE_NONE;


/*---------------------------------------------------------------------------*
  Name:         CallBackFunction

  Description:  This function is called back from the animation controller.

  Arguments:    param:          The user parameters specified when the callback function was registered.
                                
                currentFrame:   When the callback type is NNS_G2D_ANMCALLBACKTYPE_LAST_FRM, the animation controller's currentTime is passed in fx32 format.
                                
                                
                                
                                In all other cases, the current frame number will be passed in u16 format.
                                

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CallBackFunction( u32 param, fx32 currentFrame )
{
#pragma unused( param )
#pragma unused( currentFrame )

    // Updates the number of times the callback has been called
    callCount++;
    
    OS_Printf("CallBackFunction is called: param=0x%p, frame=0x%p\n", param, currentFrame);
}

/*---------------------------------------------------------------------------*
  Name:         InitOamManager

  Description:  Initializes the OAM manager system.

  Arguments:    pOamManager:    Pointer to the OAM manager initialized for object output
                                
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitOamManager( NNSG2dOamManagerInstance* pObjManager )
{
    BOOL    result;

    // Initializes the Oam Manager system
    NNS_G2dInitOamManagerModule();

    
    result = NNS_G2dGetNewOamManagerInstanceAsFastTransferMode( pObjManager, 
                                                                0, 
                                                                NUM_OF_OAM, 
                                                                NNS_G2D_OAMTYPE_MAIN );    
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
static void LoadResources(NNSG2dAnimBankData** ppAnimBank, NNSG2dCellAnimation** ppCellAnim)
{
    void* pBuf;

    NNS_G2dInitImageProxy( &sImageProxy );
    NNS_G2dInitImagePaletteProxy( &sPaletteProxy );

    //
    // Initialization of cell related resources.
    //
    {
        NNSG2dCellDataBank*          pCellBank = NULL;

        // Load cell data and cell animation data.
        // These pBuf pointers are not deallocated because cell animation data is used in main memory until the end.
        // 
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/CellAnimation.NCER" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNANR( ppAnimBank, "data/CellAnimation.NANR" );
        SDK_NULL_ASSERT( pBuf );

        //
        // Initialize the cell animation instance
        //
        {
            *ppCellAnim = G2DDemo_GetNewCellAnimation(1);
            SDK_NULL_ASSERT( *ppCellAnim );

            SDK_NULL_ASSERT( NNS_G2dGetAnimSequenceByIdx(*ppAnimBank, 0) );

            NNS_G2dInitCellAnimation(
                *ppCellAnim,
                NNS_G2dGetAnimSequenceByIdx(*ppAnimBank, 0),
                pCellBank );
        }
    }

    //
    // Initialization related to VRAM
    //
    {
        //------------------------------------------------------------------------------
        // load character data for 2D
        {
            NNSG2dCharacterData*        pCharData;

            pBuf = G2DDemo_LoadNCGR( &pCharData, "data/CellAnimation.NCGR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading for 2D Graphics Engine
            DC_FlushRange( pCharData->pRawData, pCharData->szByte );
            GX_LoadOBJ( (void*)pCharData->pRawData, TEX_BASE, pCharData->szByte );
            
            // Because character data was copied into VRAM,
            // this pBuf is freed. Same below.
            G2DDemo_Free( pBuf );
        }

        //------------------------------------------------------------------------------
        // load palette data
        {
            NNSG2dPaletteData*        pPlttData;

            pBuf = G2DDemo_LoadNCLR( &pPlttData, "data/CellAnimation.NCLR" );
            SDK_NULL_ASSERT( pBuf );

            DC_FlushRange( pPlttData->pRawData, pPlttData->szByte );
            GX_LoadOBJPltt( (void*)pPlttData->pRawData, TEX_PLTT_BASE, pPlttData->szByte );
            
            G2DDemo_Free( pBuf );

        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ChangeCallBackType

  Description:  Changes the callback type.

  Arguments:    pAnimCtrl:      Pointer to the animation controller for which the callback type is to be changed.
                                
                callBackType:   New callback type.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ChangeCallBackType
(
    NNSG2dAnimController*   pAnimCtrl,
    NNSG2dAnmCallbackType   callBackType
)
{
    //
    // Initialize = reset
    //
    NNS_G2dInitAnimCtrlCallBackFunctor( pAnimCtrl );


    switch( callBackType )
    {
    case NNS_G2D_ANMCALLBACKTYPE_NONE:
        break;

    case NNS_G2D_ANMCALLBACKTYPE_LAST_FRM:
    case NNS_G2D_ANMCALLBACKTYPE_EVER_FRM:
        NNS_G2dSetAnimCtrlCallBackFunctor(
            pAnimCtrl,
            callBackType,
            CALLBACK_PARAM,
            CallBackFunction );
        break;

    case NNS_G2D_ANMCALLBACKTYPE_SPEC_FRM:
        NNS_G2dSetAnimCtrlCallBackFunctorAtAnimFrame(
            pAnimCtrl,
            CALLBACK_PARAM,
            CallBackFunction,
            CALL_CALLBACK_FRAME );
        break;

    default:
        SDK_ASSERT(FALSE);
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         ProcessInput

  Description:  Loads the pad input and changes animation parameters according to the input.
                

  Arguments:    pIndex:         Pointer to the sequence index.
                                Use the left/right keys to switch the sequence.
                pAnimSpeed:     Pointer to the animation speed.
                                Changes the speed with up and down keys.
                pAnimCtrl:      Pointer to the animation controller.
                                Use A/B Buttons to stop/reverse.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ProcessInput(u16* pIndex, fx32* pAnimSpeed, NNSG2dAnimController* pAnimCtrl )
{
    G2DDemo_ReadGamePad();

    // change animation sequence.
    {
        if(G2DDEMO_IS_TRIGGER(PAD_KEY_RIGHT))
        {
            (*pIndex)++;
        }

        if(G2DDEMO_IS_TRIGGER(PAD_KEY_LEFT))
        {
            (*pIndex)--;
        }
    }

    // change animation speed.
    {
        if (G2DDEMO_IS_TRIGGER(PAD_KEY_UP))
        {
            *pAnimSpeed +=ANIM_SPEED_UNIT;
        }

        if (G2DDEMO_IS_TRIGGER(PAD_KEY_DOWN))
        {
            *pAnimSpeed -= ANIM_SPEED_UNIT;

            if( *pAnimSpeed < 0 )
            {
                *pAnimSpeed = 0;
            }
        }
    }

    // Playback control
    {
        if (G2DDEMO_IS_TRIGGER(PAD_BUTTON_B ))
        {
            pAnimCtrl->bReverse = ! pAnimCtrl->bReverse;
        }

        if (G2DDEMO_IS_TRIGGER(PAD_BUTTON_A ))
        {
            pAnimCtrl->bActive = ! pAnimCtrl->bActive;
        }
    }

    // Change the callback type
    if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_X) )
    {
        callBackType++;
        if( callBackType >= AnmCallbackType_MAX )
        {
            callBackType = NNS_G2D_ANMCALLBACKTYPE_NONE;
        }
        ChangeCallBackType(pAnimCtrl, callBackType);
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
    NNSG2dAnimBankData*         pAnimBank = NULL;   // Cell animation data
    NNSG2dCellAnimation*        pCellAnim = NULL;   // CellAnimation instance
    

    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_PrintInit();
        G2DDemo_CameraSetup();
        G2DDemo_MaterialSetup();
        InitOamManager( &objOamManager );
        LoadResources( &pAnimBank, &pCellAnim );
        
        // start display
        SVC_WaitVBlankIntr();
        GX_DispOn();
        GXS_DispOn();
    }

    //----------------------------------------------------
    // Main loop
    while( TRUE )
    {
        static u16      currentSeqIdx = 0;       // Rendering sequence number
        static fx32     animSpeed     = FX32_ONE;// Animation speed

        //-------------------------------------------------
        // Pad input
        //
        {
            const u16 oldSeqIndex = currentSeqIdx;
            const u16 numSequence = NNS_G2dGetNumAnimSequence( pAnimBank );

            ProcessInput(
                &currentSeqIdx,
                &animSpeed,
                NNS_G2dGetCellAnimationAnimCtrl(pCellAnim) );

            if( (s16)currentSeqIdx >= numSequence )
            {
                currentSeqIdx= 0;
            }

            if( (s16)currentSeqIdx < 0 )
            {
                currentSeqIdx = (u16)(numSequence - 1);
            }
             
            //
            // Will respond appropriately if there are changes
            //
            if( oldSeqIndex != currentSeqIdx )
            {
                const NNSG2dAnimSequence* pSeq = NULL;

                pSeq = NNS_G2dGetAnimSequenceByIdx( pAnimBank, currentSeqIdx );
                SDK_NULL_ASSERT( pSeq );
                NNS_G2dSetCellAnimationSequence( pCellAnim, pSeq );
                NNS_G2dStartAnimCtrl( NNS_G2dGetCellAnimationAnimCtrl( pCellAnim ) );
            }
            
            // Restart playback
            if( G2DDEMO_IS_TRIGGER(PAD_BUTTON_Y) )
            {
                NNS_G2dRestartCellAnimation( pCellAnim );
            }
        }
        
        
        //-------------------------------------------------
        // Render
        //
        {
            u16                     numOamDrawn = 0;  // Number of OAMs that have been rendered
            static GXOamAttr        temp[NUM_OF_OAM]; // Temporary OAM buffer
            const NNSG2dCellData*   pCell = NULL;     // Cell to display

            pCell = NNS_G2dGetCellAnimationCurrentCell( pCellAnim );
            SDK_NULL_ASSERT( pCell );

            //
            // Writes an OBJ list equivalent to the cell to the temporary buffer.
            //
            numOamDrawn = NNS_G2dMakeCellToOams(
                                temp,           // Output Oam buffer
                                NUM_OF_OAM,     // Output buffer length
                                pCell,          // Cell to output
                                NULL,           // Affine conversion
                                &CELL_POSITION, // Offset position
                                NULL,           // Affine Index
                                FALSE );        // Double affine?

            SDK_ASSERT( numOamDrawn < NUM_OF_OAM);

            // Registers OBJ list to OAM Manager
            (void)NNS_G2dEntryOamManagerOam( &objOamManager, temp, numOamDrawn );
        }

        //-------------------------------------------------
        // Debug character string drawing
        //
        {
            const NNSG2dAnimController* pAnimCtrl = NNS_G2dGetCellAnimationAnimCtrl(pCellAnim);
            
            G2DDemo_PrintOutf(0, 0, "sequence: %3d / %3d", currentSeqIdx, NNS_G2dGetNumAnimSequence( pAnimBank ));
            G2DDemo_PrintOutf(0, 1, "speed:    %7.3f", (float)animSpeed / FX32_ONE );
            G2DDemo_PrintOutf(0, 2, "frame:    %3d / %3d",
                pAnimCtrl->pCurrent - pAnimCtrl->pAnimSequence->pAnmFrameArray,
                pAnimCtrl->pAnimSequence->numFrames );
            G2DDemo_PrintOutf(0, 3, "time:     %3d / %3d",
                pAnimCtrl->currentTime >> FX32_SHIFT,
                pAnimCtrl->pCurrent->frames);
            G2DDemo_PrintOut(22, 0, "Active:");
            G2DDemo_PrintOut(29, 1, NNS_G2dIsAnimCtrlActive(pAnimCtrl) ? "yes": " no");
            G2DDemo_PrintOut(22, 2, "Reverse:");
            G2DDemo_PrintOut(29, 3, pAnimCtrl->bReverse ? "yes": " no");
            
            G2DDemo_PrintOutf(0, 21, "callback type:  %s",
                CALL_BACK_TYPE_NAME[callBackType]);
            G2DDemo_PrintOutf(0, 22, "callback count: %4d", 0);
            
            G2DDemo_PrintOutf(16, 22, "%5d", callCount);
        }
        
        //-------------------------------------------------
        // Wait for V-Blank
        G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
        SVC_WaitVBlankIntr();
            
        //-------------------------------------------------
        // Write manager contents to HW.
        //            
        {
            NNS_G2dApplyOamManagerToHW( &objOamManager );
            NNS_G2dResetOamManagerBuffer( &objOamManager );
        }

        //-------------------------------------------------
        // Writes the debug character string to HW.   
        //
        G2DDemo_PrintApplyToHW();   
        
        //-------------------------------------------------
        // Updates the animation.
        // 
        {
            NNS_G2dTickCellAnimation( pCellAnim, animSpeed );
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

