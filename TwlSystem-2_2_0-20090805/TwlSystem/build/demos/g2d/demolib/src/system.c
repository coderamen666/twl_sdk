/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - demolib
  File:     system.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include "g2d_demolib.h"
#include <nnsys/g2d/g2d_Softsprite.h>
#include <nnsys/g2d/g2d_Load.h>

// Size of resource pool
#define SIZE_OF_RES_POOL        1000

// Round up to match alignment
#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

// Round down to match alignment
#define ROUND_DOWN(value, alignment) \
    ((u32)(value) & ~(alignment-1))

G2DDemoGamePad         gGamePad;         // GamePad
NNSFndHeapHandle       gHeapHandle;      // Heap handle

/*---------------------------------------------------------------------------*
    Initialization-related.
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Init3DStuff_

  Description:  Initializes the graphics engine for software sprites.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Init3DStuff_(void)
{
    G3X_Init();                                            // initialize the 3D graphics states
    G3X_InitMtxStack();                                    // initialize the matrix stack

    // Main screen
    {
        GX_SetBankForTex(GX_VRAM_TEX_0_A);                          // VRAM-A for texture images
        GX_SetBankForOBJ(GX_VRAM_OBJ_128_B);                        // VRAM-B for OBJ
        GX_SetBankForBG(GX_VRAM_BG_128_C);                          // VRAM-C for BGs
        GX_SetBankForTexPltt(GX_VRAM_TEXPLTT_0123_E);               // VRAM-E for texture palettes
        GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS,                    // Graphics mode
                           GX_BGMODE_4,                             // BGMODE is 4
                           GX_BG0_AS_3D);                           // BG #0 is for 3D
        GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D );             // 2D mapping mode
        GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_OBJ );  // display BG0 & OBJ
    }

    // Sub-screen
    {
        GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);            // VRAM-D for OBJ
        GXS_SetGraphicsMode(GX_BGMODE_0);                      // BGMODE is 0
        GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);        // 2D mapping mode
        GXS_SetVisiblePlane(GX_PLANEMASK_OBJ);                 // Display only OBJ
    }


    G2_SetBG0Priority(1);

    G3X_SetShading(GX_SHADING_TOON);
    G3X_AntiAlias(TRUE);
    G3X_AlphaBlend(TRUE);

    // An alpha blending target screen specification is needed because alpha blending with another background screen can occur even when 3D screen special effects are turned off.
    // 
    // Because the SDK has no API for specifying only the planes to be subject to alpha blending, use G2_SetBlendAlpha instead.
    // In this case, the last 2 arguments to G2_SetBlendAlpha are ignored.
    G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0, GX_BLEND_PLANEMASK_BD, 0, 0);

    // clear color
    G3X_SetClearColor(GX_RGB(0, 0, 0), 0, 0x7fff, 0, FALSE );

    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_Z);
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_CommonInit

  Description:  Initializes the OS, graphics engine, heap, and file system.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_CommonInit(void)
{
    // Common initialization.
    {
        OS_Init();

        FX_Init();

        GX_Init();

        GX_DispOff();
        GXS_DispOff();

        OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
        (void)OS_EnableIrqMask( OS_IE_V_BLANK );
        (void)OS_EnableIrq();

        (void)GX_VBlankIntr(TRUE);
    }

    // Clear VRAM
    {
        //---------------------------------------------------------------------------
        // All VRAM banks to LCDC
        //---------------------------------------------------------------------------
        GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

        //---------------------------------------------------------------------------
        // Clear all LCDC space
        //---------------------------------------------------------------------------
        MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

        //---------------------------------------------------------------------------
        // Disable the banks on LCDC
        //---------------------------------------------------------------------------
        (void)GX_DisableBankForLCDC();

        MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);      // Clear OAM
        MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);        // Clear the standard palette

        MI_CpuFillFast((void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE); // Clear OAM
        MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);  // Clear the standard palette
    }

    //------------------------------------------------------------------------------
    // Initializing the 3D Sprite Engine.
    //------------------------------------------------------------------------------
    Init3DStuff_();
    NNS_G2dSetupSoftwareSpriteCamera();


    //------------------------------------------------------------------------------
    // misc.
    {
        *(GXRgb*)(HW_BG_PLTT) = GX_RGB(15, 15, 15);
        *(GXRgb*)(HW_DB_BG_PLTT) = GX_RGB(15, 15, 15);
        OS_InitTick();
    }
    //------------------------------------------------------------------------------
    // Allocate heap
    {
        u32   arenaLow      = ROUND_UP  (OS_GetMainArenaLo(), 16);
        u32   arenaHigh     = ROUND_DOWN(OS_GetMainArenaHi(), 16);
        u32   heapSize      = arenaHigh - arenaLow;
        void* heapMemory    = OS_AllocFromMainArenaLo(heapSize, 16);

        gHeapHandle = NNS_FndCreateExpHeap(heapMemory, heapSize);
        SDK_ASSERT( gHeapHandle != NNS_FND_HEAP_INVALID_HANDLE );
    }

    //------------------------------------------------------------------------------
    // File System
    {
        (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);

        /* Initialize file-system */
        FS_Init( FS_DMA_NOT_USE );

        /* Always preload FS table for faster directory access. */
        {
            const u32   need_size = FS_GetTableSize();
            void    *p_table = G2DDemo_Alloc(need_size);
            SDK_ASSERT(p_table != NULL);
            (void)FS_LoadTable(p_table, need_size);
        }
    }
    
    // Dummy read of GamePad.
    G2DDemo_ReadGamePad();
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_CameraSetup

  Description:  Perform common camera settings.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_CameraSetup(void)
{
    {
        // equivalent to unit matrix
        VecFx32 Eye = { 0, 0, 0 };                  // eye position
        VecFx32 vUp = { 0, FX32_ONE, 0 };           // up
        VecFx32 at = { 0, 0, -FX32_ONE };           // viewpoint

        // Matrix mode is changed to GX_MTXMODE_POSITION_VECTOR internally,
        // and the camera matrix is loaded to the current matrix.
        G3_LookAt(&Eye, &vUp, &at, NULL);
    }
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_MaterialSetup

  Description:  Performs shared material settings.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_MaterialSetup(void)
{
    // for software sprite-setting
    {
        G3_MaterialColorDiffAmb(GX_RGB(31, 31, 31),        // diffuse
                                GX_RGB(16, 16, 16),        // ambient
                                TRUE                       // use diffuse as vtx color if TRUE
                                );

        G3_MaterialColorSpecEmi(GX_RGB(16, 16, 16),        // specular
                                GX_RGB(0, 0, 0),           // emission
                                FALSE                      // use shininess table if TRUE
                                );

        G3_PolygonAttr(GX_LIGHTMASK_NONE,                  // no lights
                       GX_POLYGONMODE_MODULATE,            // modulation mode
                       GX_CULL_NONE,                       // cull back
                       0,                                  // polygon ID(0 - 63)
                       31,                                 // alpha(0 - 31)
                       0                                   // OR of GXPolygonAttrMisc's value
                       );
    }
}


/*---------------------------------------------------------------------------*
    Other
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_GetNewCellAnimation

  Description:  Gets NNSG2dCellAnimation from pool.

  Arguments:    num:  Number of instances of NNSG2dCellAnimation to be obtained.

  Returns:      Pointer to NNSG2dCellAnimation that was obtained.
 *---------------------------------------------------------------------------*/
NNSG2dCellAnimation* G2DDemo_GetNewCellAnimation( u16 num )
{
    static NNSG2dCellAnimation        sPoolAnimatedObj[SIZE_OF_RES_POOL];
    static u16                        sNumAnimatedObj = 0;

    const u16 retIdx = sNumAnimatedObj;

    sNumAnimatedObj += num;
    SDK_ASSERT( sNumAnimatedObj <= SIZE_OF_RES_POOL );

    return &sPoolAnimatedObj[ retIdx ];
}

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_GetNewNode

  Description:  Gets NNSG2dNode from pool.

  Arguments:    num:  Number of instances of NNSG2dNode to be obtained.

  Returns:      Pointer to NNSG2dNode that was obtained.
 *---------------------------------------------------------------------------*/
NNSG2dNode* G2DDemo_GetNewNode( u16 num )
{
    static NNSG2dNode                 sPoolObjNode[SIZE_OF_RES_POOL];
    static u16                        sNumUsedPoolObjNode = 0;

    const u16 retIdx = sNumUsedPoolObjNode;

    sNumUsedPoolObjNode += num;
    SDK_ASSERT( sNumUsedPoolObjNode <= SIZE_OF_RES_POOL );

    return &sPoolObjNode[ retIdx ];
}

/*---------------------------------------------------------------------------*
  Name:         getNewMultiCellAnim_

  Description:  Gets NNSG2dMultiCellAnimation from pool.

  Arguments:    num:  Number of instances of NNSG2dMultiCellAnimation to be obtained.

  Returns:      Pointer to NNSG2dMultiCellAnimation that was obtained.
 *---------------------------------------------------------------------------*/
static NNSG2dMultiCellAnimation*     GetNewMultiCellAnim_( u16 num )
{
    static NNSG2dMultiCellAnimation   sMcAnimPool_[SIZE_OF_RES_POOL];
    static u16                        sNumUsedMcAnimPool = 0;

    const u16 retIdx = sNumUsedMcAnimPool;

    sNumUsedMcAnimPool += num;
    SDK_ASSERT( sNumUsedMcAnimPool <= SIZE_OF_RES_POOL );

    return &sMcAnimPool_[retIdx];
}


/*---------------------------------------------------------------------------*
  Name:         G2DDemo_GetNewMultiCellAnimation

  Description:  Gets and initializes new NNSG2dMultiCellAnimation.

  Arguments:    numNode:            Number of nodes
                pAnimBank:          Animation data bank
                pCellDataBank:      Cell data bank
                pMultiCellDataBank: Multicell data bank

  Returns:      Pointer to the NNSG2dMultiCellAnimation that has already been taken from the pool and initialized.
                
 *---------------------------------------------------------------------------*/
/* Previous version that uses the old API
NNSG2dMultiCellAnimation*     G2DDemo_GetNewMultiCellAnimation
(
    u16                                numNode,
    const NNSG2dCellAnimBankData*      pAnimBank,
    const NNSG2dCellDataBank*          pCellDataBank,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank
)
{
    // Get NNSG2dMultiCellAnimation from pool
    NNSG2dMultiCellAnimation* pMCAnim = GetNewMultiCellAnim_( 1 );

    // Get number of instances of NNSG2dNode and NNSG2dCellAnimation suitable for numNode
    NNSG2dNode*               pNodeArray     = G2DDemo_GetNewNode( numNode );
    NNSG2dCellAnimation*      pCellAnimArray = G2DDemo_GetNewCellAnimation( numNode );

    NNS_G2dInitMCAnimation( pMCAnim,              // Multicell animation
                                  pNodeArray,           // Node array
                                  pCellAnimArray,       // Cell animation array
                                  numNode,              // Number of nodes
                                  pAnimBank,            // Animation data bank
                                  pCellDataBank,        // Cell data bank
                                  pMultiCellDataBank ); // Multicell data bank
    return pMCAnim;
}
*/

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_GetNewMultiCellAnimation

  Description:  Gets and initializes new NNSG2dMultiCellAnimation.

  Arguments:    pAnimBank:          Animation data bank
                pCellDataBank:      Cell data bank
                pMultiCellDataBank: Multicell data bank
                mcType:             Type of the multicell entity
                
  Returns:      Pointer to the NNSG2dMultiCellAnimation that has already been taken from the pool and initialized.
                
 *---------------------------------------------------------------------------*/
NNSG2dMultiCellAnimation*     G2DDemo_GetNewMultiCellAnimation
(
    const NNSG2dCellAnimBankData*      pAnimBank,
    const NNSG2dCellDataBank*          pCellDataBank ,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank,
    NNSG2dMCType                       mcType
)
{
    const u32 szWork = NNS_G2dGetMCWorkAreaSize( pMultiCellDataBank, mcType );
    void* pWorkMem   = NNS_FndAllocFromExpHeap( gHeapHandle, szWork );
    
    
    // Gets NNSG2dMultiCellAnimation from the pool
    NNSG2dMultiCellAnimation* pMCAnim = GetNewMultiCellAnim_( 1 );

    // Initialization
    NNS_G2dInitMCAnimationInstance( pMCAnim, 
                                    pWorkMem, 
                                    pAnimBank, 
                                    pCellDataBank, 
                                    pMultiCellDataBank, 
                                    mcType );
    return pMCAnim;
}




/*---------------------------------------------------------------------------*
    GamePad-related
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         G2DDemo_ReadGamePad

  Description:  Reads key and requests the trigger and release-trigger.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G2DDemo_ReadGamePad(void)
{
    u16 status = PAD_Read();

    gGamePad.trigger = (u16)(status          & (status ^ gGamePad.button));
    gGamePad.release = (u16)(gGamePad.button & (status ^ gGamePad.button));
    gGamePad.button  = status;
}

