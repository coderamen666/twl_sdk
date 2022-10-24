/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - MultiCellAnimation
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
//      Example of rendering and updating multicell animation
//      Initializes instances using different methods and compares the performances.
//
//  Using the Demo
//      +Control Pad key, up/down: Change playback speed
//      A: Toggle between play and stop
//      B: Toggle between forward and reverse playback
//      X: Change the type of the multicell animation instance
//      Y: Multicell animation reset
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


#define NUM_OF_OAM              128                  // Number of OAMs controlled by OAM Manager
#define NUM_OF_AFFINE           (NUM_OF_OAM/4)      // Number of affine parameters controlled by OAM Manager

#define ANIM_SPEED_UNIT         (FX32_ONE >> 3)     // Difference in animation speed

//
// Type of the multicell entity
// Use when accessing the array pMCAnim_
//
typedef enum 
{
    MCANIM_SHARE_CELLANIM = 0x0,
    MCANIM_DONOT_SHARE_CELLANIM = 0x1
    
}MyMCAnimType;

static const char* mcAnimTypeStr [] =
{
    "     ShareCellAnim",
    "DoNotShareCellAnim"
};

static const int CvtTblToNNSG2dMCType [] = 
{
    NNS_G2D_MCTYPE_SHARE_CELLANIM,
    NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM
};


// Cell display location
static const NNSG2dFVec2           CELL_POSITION = {120 << FX32_SHIFT, 150 << FX32_SHIFT};
//
// MultiCellAnimation instance
// Initializes instances using two different methods.
// 
static NNSG2dMultiCellAnimation   mcAnim[2];


// Temporary OAM attribute buffer
static GXOamAttr                tempOamBuffer[NUM_OF_OAM];

//
// Multicell bank
//
static NNSG2dMultiCellDataBank*        pMCBank = NULL;     // Multicell data


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
    
    // Sets the sequence to play back to the multicell animation
    {
        const NNSG2dMultiCellAnimSequence* pSequence = NULL;    
        // Obtains the sequence to play back
        pSequence = NNS_G2dGetAnimSequenceByIdx( pMCABank, 0 );
        SDK_ASSERT( pSequence );

        NNS_G2dSetAnimSequenceToMCAnimation( pMCAnim, pSequence);
    }
}

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  This function constructs a multicell animation and loads palette data.
                

  Arguments:    ppMCAnim:   Pointer that receives the pointer to the constructed multicell animation.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources()
{
    void* pBuf;

    //
    // Initialization related to Multicell
    //
    {
        NNSG2dCellDataBank*             pCellBank = NULL;   // Cell data
        NNSG2dCellAnimBankData*         pAnimBank = NULL;   // cell animation
        NNSG2dMultiCellAnimBankData*    pMCABank = NULL;    // Multicell animation

        // Load the cell data, cell animation, multicell data and multicell animation.
        // 
        // They are used on main memory until the end, so pBuf is not released.
        pBuf = G2DDemo_LoadNCER( &pCellBank, "data/MultiCellAnimation.NCER" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNANR( &pAnimBank, "data/MultiCellAnimation.NANR" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNMCR( &pMCBank, "data/MultiCellAnimation.NMCR" );
        SDK_NULL_ASSERT( pBuf );
        pBuf = G2DDemo_LoadNMAR( &pMCABank, "data/MultiCellAnimation.NMAR" );
        SDK_NULL_ASSERT( pBuf );

        
        
        //
        // Initializes the multicell animation instances
        // Initializes two instances using different methods.
        //
        InitMultiCellInstance_( &mcAnim[MCANIM_SHARE_CELLANIM],
                                pAnimBank,
                                pCellBank,
                                pMCBank,
                                pMCABank,
                                NNS_G2D_MCTYPE_SHARE_CELLANIM  );

        
        InitMultiCellInstance_( &mcAnim[MCANIM_DONOT_SHARE_CELLANIM],
                                pAnimBank,
                                pCellBank,
                                pMCBank,
                                pMCABank,
                                NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM  );
    }

    //
    // Initialization related to VRAM
    //
    {
        // Loading NCG (Character Data)
        {
            NNSG2dCharacterData*        pCharData = NULL;

            pBuf = G2DDemo_LoadNCGR( &pCharData, "data/MultiCellAnimation.NCGR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading For 2D Graphics Engine
            DC_FlushRange( pCharData->pRawData, pCharData->szByte );
            GX_LoadOBJ( (void*)pCharData->pRawData, 0, pCharData->szByte );

            // Because character data was copied into VRAM,
            // this pBuf is freed. Same below.
            G2DDemo_Free( pBuf );
        }

        // Loading NCL (Palette Data)
        {
            NNSG2dPaletteData*        pPltData = NULL;

            pBuf = G2DDemo_LoadNCLR( &pPltData, "data/MultiCellAnimation.NCLR" );
            SDK_NULL_ASSERT( pBuf );

            // Loading For 2D Graphics Engine
            DC_FlushRange( pPltData->pRawData, pPltData->szByte );
            GX_LoadOBJPltt( (void*)pPltData->pRawData, 0, pPltData->szByte );

            G2DDemo_Free( pBuf );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         ProcessInput

  Description:  Loads the pad input and changes animation parameters according to the input.
                

  Arguments:    pAnimSpeed: Pointer to the animation speed.
                            Changes the speed with up and down keys.
                pAnimCtrl:  Pointer to the animation controller.
                            Use A/B Buttons to stop/reverse.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ProcessInput(fx32* pAnimSpeed, NNSG2dAnimController* pAnimCtrl)
{
    G2DDemo_ReadGamePad();

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
}
//--------------------------------------------------------------------------------------------------------
//Multicell rendering
static void AppDraw
( 
    const NNSG2dMultiCellAnimation* pMcAnim, 
    NNSG2dOamManagerInstance*       pOamMgr 
)
{
    // Render
    //
    // Renders multicell in the low level rendering method.
    // There is also a rendering method that uses G2dRenderer of the upper level module.
    //
    //
    {
        u16                             numOamDrawn = 0;                  // number of remaining OAMs
        
        const NNSG2dMultiCellInstance*  pMultiCell = &(pMcAnim->multiCellInstance); // Multicell to display
        SDK_NULL_ASSERT( pMultiCell );

        //
        // Writes an OBJ list equivalent to multicell to temporary buffer.
        //
        numOamDrawn = NNS_G2dMakeSimpleMultiCellToOams(
                            tempOamBuffer,  // output Oam buffer
                            NUM_OF_OAM,     // output buffer length
                            pMultiCell,     // output multicell
                            NULL,           // affine conversion
                            &CELL_POSITION, // offset position
                            NULL,           // Affine Index
                            FALSE );        // double affine?

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
    MyMCAnimType                mcAnimType = MCANIM_SHARE_CELLANIM;
        
    // Initialize App.
    {
        G2DDemo_CommonInit();
        G2DDemo_PrintInit();
        InitOamManager( &oamManager );
        LoadResources();
        
        // start display
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
        static fx32                    animSpeed    = FX32_ONE; // Animation speed
        static OSTick                  time         = 0;
        NNSG2dMultiCellAnimation*      pMcAnim      = &mcAnim[mcAnimType];
        
        //
        //  ------ Pad handling.
        //
        {
            ProcessInput(&animSpeed, NNS_G2dGetMCAnimAnimCtrl(pMcAnim));
            
            // ---- Switches the type of multicell animation instance.
            if (G2DDEMO_IS_TRIGGER(PAD_BUTTON_X ))
            {
                if( mcAnimType == MCANIM_SHARE_CELLANIM )
                {
                    mcAnimType = MCANIM_DONOT_SHARE_CELLANIM;
                }else{
                    mcAnimType = MCANIM_SHARE_CELLANIM;
                }
            }
            
            // ---- Reset
            if (G2DDEMO_IS_PRESS(PAD_BUTTON_Y ))
            {
                NNS_G2dRestartMCAnimation( pMcAnim );
            }
        }

        //  ------ Render
        //
        // Render multicell animation using a low level rendering API.
        // There is also a rendering method that uses G2dRenderer of the upper level module.
        //
        //
        AppDraw( pMcAnim, &oamManager );
        
        
        //  ------ Update the animation
        // Measures the time that it took to update the one being drawn.
        {
            u32 i;
            for( i = 0; i < 2; i++ )
            {
                if( i == mcAnimType ) 
                {
                    time = OS_GetTick();
                        NNS_G2dTickMCAnimation( &mcAnim[i], animSpeed );
                    time = OS_GetTick()- time;
                }else{
                        NNS_G2dTickMCAnimation( &mcAnim[i], animSpeed );
                }
            }
        }
        
        
        // ------ Draw the debug string
        {
            const NNSG2dAnimController* pAnimCtrl = NNS_G2dGetMCAnimAnimCtrl(pMcAnim);

            G2DDemo_PrintOutf(0, 0, "speed: %7.3f", (float)animSpeed / FX32_ONE );
            G2DDemo_PrintOutf(0, 1, "frame: %3d / %3d",
                pAnimCtrl->pCurrent - pAnimCtrl->pAnimSequence->pAnmFrameArray,
                pAnimCtrl->pAnimSequence->numFrames );
            
            G2DDemo_PrintOutf( 0, 2, "TIMEForUpdate     :%06ld usec", OS_TicksToMicroSeconds(time) );
            
            // Size of work memory required to initialize the current instance
            G2DDemo_PrintOutf( 0, 3, "szWorkMem:%06ld byte", 
                               NNS_G2dGetMCWorkAreaSize( pMCBank, 
                               (NNSG2dMCType)CvtTblToNNSG2dMCType[ mcAnimType ] ) );
                               
            // Type of current instance
            G2DDemo_PrintOutf( 0, 4, "Type     :%s", mcAnimTypeStr[mcAnimType] );
            
            G2DDemo_PrintOut(19, 0, "Active:");
            G2DDemo_PrintOut(28, 0, NNS_G2dIsAnimCtrlActive(pAnimCtrl) ? "yes": " no");
            G2DDemo_PrintOut(19, 1, "Reverse:");
            G2DDemo_PrintOut(28, 1, pAnimCtrl->bReverse ? "yes": " no");
        }

        // ------ Wait for V-Blank
        SVC_WaitVBlankIntr();
        G2DDemo_PrintApplyToHW();   // Writes the animation information to HW

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

