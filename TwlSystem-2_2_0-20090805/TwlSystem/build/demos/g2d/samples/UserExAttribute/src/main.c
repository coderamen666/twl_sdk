/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - samples - UserExAttribute
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
//      This is a sample that shows how to use user expanded attribute information.
//      Use the converter's new feature, the -oua option, to output the user expanded attribute information.
//      
//      
//      
//      User expanded attribute information must be used to added expanded comments to cells and animation frames.
//      You therefore need to create data to which expanded comments can be appended with a new version of NITRO-CHARACTER.
//      
//
//      This sample gets many types of attribute values for each cell animation within a multicell and displays the results as debug output.
//      
//      
//      We have considered several applications in the actual game program, such as:
//          - A sound playback trigger
//          - Storage of a collision determination size
//        
//
//  Using the Demo
//      +Control Pad key, up/down: Change playback speed
//      Y: Change the method for acquiring user expanded attributes
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>

#include "g2d_demolib.h"


#define NUM_OF_OAM              128                  // Number of OAMs controlled by OAM Manager
#define NUM_OF_AFFINE           (NUM_OF_OAM/4)      // Number of affine parameters controlled by OAM Manager

#define ANIM_SPEED_UNIT         (FX32_ONE >> 3)     // Difference in animation speed

#define NUM_CELL_ANIMATION      10

//
// Pass the structure pointer to the data structure controller that is used when passing data to the animation controller's callback parameter.
// 
// 
//
typedef struct MyAnimCtrlData
{
    NNSG2dAnimController*       pAnmCtrl;       // Animation controller
    u16                         idx;            // Cell animation number
    u16                         pad16;          // Padding
    BOOL                        bAttributeIsNotZero;// Was a non-zero attribute found?
    
}MyAnimCtrlData;
static MyAnimCtrlData              animDataArray[NUM_CELL_ANIMATION];


// Cell display location
static const NNSG2dFVec2           MULTICELL_POSITION = {120 << FX32_SHIFT, 150 << FX32_SHIFT};
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
static NNSG2dCellAnimBankData*         pAnimBank = NULL;   // cell animation
static NNSG2dMultiCellAnimBankData*    pMCABank = NULL;    // Multicell animation

//
// User extended attribute-related
//
static const NNSG2dUserExCellAttrBank*  pCellAttrBank = NULL;
static const NNSG2dUserExAnimAttrBank*  pAnimAttrBank = NULL;

//
// Method for obtaining expanded attributes
//
typedef enum MY_GET_ATTRIBUTE_MODE
{
    MY_GET_ATTRIBUTE_MODE_CELL,    // Reference cell expanded attributes
    MY_GET_ATTRIBUTE_MODE_FRAME,   // Animation frame
                                   // Access extended attributes
    MY_GET_ATTRIBUTE_MODE_MAX

}MY_GET_ATTRIBUTE_MODE;
   
static MY_GET_ATTRIBUTE_MODE    attributeMode = MY_GET_ATTRIBUTE_MODE_CELL;


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

//------------------------------------------------------------------------------
// Callback function for setting cell animation during multicell
// Gets user expanded attributes in conjunction with the application's operation mode and saves attributes values when they are not a standard value.
// 
// 
//
static void CellAnimCallBackFunction( u32 param, fx32 currentFrame )
{
#pragma unused(currentFrame)
    
    MyAnimCtrlData*             pAnmCtrlData    = (MyAnimCtrlData*)param;
    const NNSG2dAnimController* pAnmCtrl        = pAnmCtrlData->pAnmCtrl;
     
    u32 attrVal;
    
    //
    // Depending on the method for obtaining the attributes...
    //
    switch( attributeMode )
    {
    // ----- Reference cell expanded attributes
    case MY_GET_ATTRIBUTE_MODE_CELL:
        {
            // Get the current cell number
            const u16 cellIdx = NNS_G2dGetAnimCtrlCurrentElemIdxVal( pAnmCtrl );
            // Get the attribute corresponding to the cell number
            const NNSG2dUserExCellAttr* pAttr 
                = NNS_G2dGetUserExCellAttr( pCellAttrBank, cellIdx );
            attrVal = NNS_G2dGetUserExCellAttrValue( pAttr );
            
            break;
        }
    // ----- Reference animation frame expanded attributes
    case MY_GET_ATTRIBUTE_MODE_FRAME:
        {
            // Get the current sequence number
            const NNSG2dAnimSequence* pSeq 
                = NNS_G2dGetAnimCtrlCurrentAnimSequence( pAnmCtrl );
            const u16 seqIdx 
                = NNS_G2dGetAnimSequenceIndex( pAnimBank, pSeq );
            // Get the sequence extended attribute corresponding to the animation sequence
            // (Although not used in this sample, it is also possible to get attribute values from sequence expanded attributes.)
            // 
            const NNSG2dUserExAnimSequenceAttr* pSeqAttr 
                = NNS_G2dGetUserExAnimSequenceAttr( pAnimAttrBank, seqIdx );
                
            // Get the animation frame number during playback
            const u16 currenFrmIdx = NNS_G2dGetAnimCtrlCurrentFrame( pAnmCtrl );
            // Get the frame expanded attribute from the sequence expanded attribute and the frame number.
            // 
            const NNSG2dUserExAnimFrameAttr*  pFrmAttr 
                = NNS_G2dGetUserExAnimFrameAttr( pSeqAttr, currenFrmIdx );                                                        
            attrVal = NNS_G2dGetUserExAnimFrmAttrValue( pFrmAttr );
            
            break;
        }
    }
        
    //
    // Record it when the obtained attribute value is not zero.
    // 
    //
    if( attrVal != 0 )
    { 
        pAnmCtrlData->bAttributeIsNotZero = TRUE;
    }else{
        pAnmCtrlData->bAttributeIsNotZero = FALSE;
    }
}

//------------------------------------------------------------------------------
// 
/*---------------------------------------------------------------------------*
  Name:         SetupMCCellAnimCallBackFunc

  Description:  Called to be passed as a parameter to the NNS_G2dTraverseMCCellAnims (multicell traversing) function and for each cell animation within the NNS_G2dTraverseMCCellAnims callback function.
                
                
                
                
                Callback functions are configured within this function for the animation controllers held by cell animations.
                
                 

  Arguments:    userParamaters: User parameters
                                (In this sample, pointers to animDataArray have been substituted)
                pCellAnim:      Cell animations in a multicell animation
                idx:           Cell animation number

  Returns:      Whether or not to cancel the callback call.
                If the callback invocation is cancelled, returns FALSE.
 *---------------------------------------------------------------------------*/
static BOOL SetupMCCellAnimCallBackFunc
(
    u32                  userParamaters,
    NNSG2dCellAnimation* pCellAnim, 
    u16                  idx 
)
{
#pragma unused( userParamaters )     
    
    //
    // In this sample, pointers to animDataArray have been substituted as userParameters.
    // 
    //
    MyAnimCtrlData* pAnmDataArray = (MyAnimCtrlData*)userParamaters;
    
    // Get the animation controller from the cell animation.
    // 
    NNSG2dAnimController* pAnmCtrl 
        = NNS_G2dGetCellAnimationAnimCtrl( pCellAnim );

    // Initializes the animation controller functor    
    NNS_G2dInitAnimCtrlCallBackFunctor( pAnmCtrl );
    
    // Stores data in the user-defined structure used as the callback functor parameters.
    // 
    pAnmDataArray[idx].pAnmCtrl = pAnmCtrl;
    pAnmDataArray[idx].idx      = idx;
    
    // Set a parameter and callback function to the animation controller functor.
    // 
    NNS_G2dSetAnimCtrlCallBackFunctor
    ( 
        pAnmCtrl,
        NNS_G2D_ANMCALLBACKTYPE_EVER_FRM,
        (u32)&pAnmDataArray[idx],
        CellAnimCallBackFunction 
    );
    
    return TRUE;    
} 

/*---------------------------------------------------------------------------*
  Name:         LoadResources

  Description:  This function carries out the building of the multi-cell animation and the loading of character and palette data.
                

  Arguments:    ppMCAnim:   Pointer that receives the pointer to the built multi-cell animation.
                            

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadResources()
{
    void* pBuf;

    //
    // Initialization related to Multicell
    //
    {
        
        // 
        // Load the cell data, cell animation, multicell data and multicell animation.
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
        // Get extended attribute bank
        // Data must be converted using the converter option -oua.
        // 
        //
        // Gets occur here from the cell and animation banks, but data can also be gotten in the same way from multicell and multicell animation banks.
        // 
        // 
        //
        // Get cell extended attribute bank
        pCellAttrBank = NNS_G2dGetUserExCellAttrBankFromCellBank( pCellBank ); 
        // Get animation extended attribute bank
        pAnimAttrBank = NNS_G2dGetUserExAnimAttrBank( pAnimBank );

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
        
        //
        // Call the callback function that traverses the cell animations that comprise a multicell and performs user-specific initialization for each cell animation.
        // 
        // 
        //
        // A pointer to animDataArray is passed as a user parameter.
        // This pointer is used to transfer the animDataArray data inside the callback.
        NNS_G2dTraverseMCCellAnims( &mcAnim.multiCellInstance,
                                    SetupMCCellAnimCallBackFunc,
                                    (u32)(&animDataArray) );
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
        u16                             numOamDrawn = 0;                  
        
        // Multicell to display
        const NNSG2dMultiCellInstance*  pMultiCell = &(pMcAnim->multiCellInstance); 
        SDK_NULL_ASSERT( pMultiCell );

        //
        // Writes an OBJ list equivalent to multicell to temporary buffer.
        //
        numOamDrawn = NNS_G2dMakeSimpleMultiCellToOams(
                            tempOamBuffer,         // output Oam buffer
                            NUM_OF_OAM,            // output buffer length
                            pMultiCell,            // output multicell
                            NULL,                  // affine conversion
                            &MULTICELL_POSITION,   // offset position
                            NULL,                  // Affine Index
                            FALSE );               // double affine?

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
        NNSG2dMultiCellAnimation*      pMcAnim      = &mcAnim;
        
        //
        //  ------ Pad handling.
        //
        {
            G2DDemo_ReadGamePad();

            // change animation speed.
            {
                if (G2DDEMO_IS_TRIGGER(PAD_KEY_UP))
                {
                    animSpeed +=ANIM_SPEED_UNIT;
                }

                if (G2DDEMO_IS_TRIGGER(PAD_KEY_DOWN))
                {
                    animSpeed -= ANIM_SPEED_UNIT;

                    if( animSpeed < 0 )
                    {
                        animSpeed = 0;
                    }
                }
            }
            
            // ---- Change the method of retrieving extended attributes.
            if (G2DDEMO_IS_TRIGGER(PAD_BUTTON_Y ))
            {
                attributeMode 
                    = (MY_GET_ATTRIBUTE_MODE)((++attributeMode) % MY_GET_ATTRIBUTE_MODE_MAX);
            }
        }

        //  ------ Render
        AppDraw( pMcAnim, &oamManager );
        //  ------ Update the animation
        NNS_G2dTickMCAnimation( &mcAnim, animSpeed );
        
        
        
        // ------ Render the debug string
        {
            const NNSG2dAnimController* pAnimCtrl = NNS_G2dGetMCAnimAnimCtrl(pMcAnim);

            G2DDemo_PrintOutf(0, 0, "speed: %7.3f", (float)animSpeed / FX32_ONE );
            G2DDemo_PrintOutf(0, 1, "frame: %3d / %3d",
                pAnimCtrl->pCurrent - pAnimCtrl->pAnimSequence->pAnmFrameArray,
                pAnimCtrl->pAnimSequence->numFrames );
            
            //
            // A small circle is displayed when an expanded attributed for a cell animation, having a value other than zero, has been recognized. In all other cases, a small 'x' is displayed.
            // 
            // 
            //
            {
                int i;
                G2DDemo_PrintOutf( 0, 3,      "-----------------" );
                G2DDemo_PrintOutf( 0, 4,      "attribute " );
                G2DDemo_PrintOutf( 0, 5,      "-----------------" );
                
                for( i = 0; i < NUM_CELL_ANIMATION; i++ )
                {
                    if( animDataArray[i].bAttributeIsNotZero == TRUE )
                    {
                        G2DDemo_PrintOutf( 0, 6+i, "%02d:o", i );
                    }else{
                        G2DDemo_PrintOutf( 0, 6+i, "%02d:x", i );
                    }
                }
                G2DDemo_PrintOutf( 0, 6 + NUM_CELL_ANIMATION, "----- " );
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

