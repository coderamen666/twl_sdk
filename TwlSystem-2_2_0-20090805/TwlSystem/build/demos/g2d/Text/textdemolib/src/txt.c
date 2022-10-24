/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - textdemolib
  File:     txt.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "g2d_textdemolib.h"


static NNSFndAllocator sAllocator;

#define GRAY(x) GX_RGB(x, x, x)

// Color palette shared by demos
GXRgb TXTColorPalette[TXT_NUM_CPALEETE * 16] =
{
    GX_RGB( 0,  0,  0), GX_RGB(31, 31, 31), GX_RGB( 0,  0,  0), GX_RGB(31,  0,  0),
    GX_RGB( 0, 31,  0), GX_RGB( 0,  0, 31), GX_RGB( 0, 31, 31), GX_RGB(31,  0, 31),
    GX_RGB(31, 31,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
    GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),

    GX_RGB( 0,  0,  0), GX_RGB(12, 16, 19), GX_RGB(23,  9,  0), GX_RGB(31,  0,  3),
    GX_RGB(31, 17, 31), GX_RGB(31, 18,  0), GX_RGB(30, 28,  0), GX_RGB(21, 31,  0),
    GX_RGB( 0, 20,  7), GX_RGB( 9, 27, 17), GX_RGB( 6, 23, 30), GX_RGB( 0, 11, 30),
    GX_RGB( 0,  0, 18), GX_RGB(17,  0, 26), GX_RGB(26,  0, 29), GX_RGB(31,  0, 18),

    GRAY(31),           GRAY(29),           GRAY(27),           GRAY(25),
    GRAY(23),           GRAY(21),           GRAY(19),           GRAY(17),
    GRAY(15),           GRAY(14),           GRAY(12),           GRAY(10),
    GRAY( 8),           GRAY( 6),           GRAY( 3),           GRAY( 0),
};

//****************************************************************************
//
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         AssignVramBanks

  Description:  Allocates VRAM.

                        A B C D E F G H I
                2DA BG      o o           256 KB    For demo and background
                    OBJ o o                256 KB    For demo
                2DB BG                o    32 KB    For information display
                    OBJ
                Not used          o o o   o 112 KB

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AssignVramBanks( void )
{
    GX_SetBankForBG(GX_VRAM_BG_256_CD);
    GX_SetBankForOBJ(GX_VRAM_OBJ_256_AB);

    GX_SetBankForSubBG(GX_VRAM_SUB_BG_32_H);
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  Handles VBlank interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void)
{
    OS_SetIrqCheckFlag( OS_IE_V_BLANK );                   // Checking VBlank interrupt
}


//****************************************************************************
//
//****************************************************************************


/*---------------------------------------------------------------------------*
  Name:         TXT_Init

  Description:  Shared initialization of the sample demos.
                    VRAM initialization
                    VRAM allocation
                    Memory allocator initialization
                    Initialize file system
                    Graphics mode configuration
                    Visibility screen configuration
                    Priority configuration
                    BG memory offset configuration

                Main screen BG
                    BG mode 4
                        BG0 text         For background     Visible
                        BG1 text         For demo     Not visible
                        BG2 affine         For demo     Not visible
                        BG3 affine expansion     For demo     Not visible
                        OBJ                 For demo     Not visible
                    BG VRAM 256 KB
                        00000-  192 KB  Character for demo
                        30000-   30 KB  Screen for demo
                        37800-    2 KB  Screen for background
                        38000-   32 KB  Character for background
                    OBJ VRAM 256 KB
                        00000-  256 KB  Character for demo

                Sub-screen
                    BG mode 0
                        BG0 text         For information output  Visible
                        BG1 Not used
                        BG2 Not used
                        BG3 Not used


  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_Init(void)
{
    // Common initialization.
    {
        OS_Init();
        FX_Init();
        GX_Init();

        GX_DispOff();
        GXS_DispOff();
    }

    {
        CMN_InitInterrupt();
        CMN_BeginVBlankIntr(VBlankIntr);
        CMN_ClearVram();
        CMN_InitAllocator( &sAllocator );
        CMN_InitFileSystem( &sAllocator );
    }

    AssignVramBanks();

    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_4, GX_BG0_AS_2D);
    GX_SetBGScrOffset(GX_BGSCROFFSET_0x30000);
    GX_SetBGCharOffset(GX_BGCHAROFFSET_0x00000);

    DTX_Init();
}



/*---------------------------------------------------------------------------*
  Name:         TXT_SetupBackground

  Description:  Loads and displays the background image to the main screen BG0.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_SetupBackground( void )
{
    void*                   pPltFile;
    void*                   pChrFile;
    void*                   pScnFile;
    NNSG2dPaletteData*      pPltData;
    NNSG2dCharacterData*    pChrData;
    NNSG2dScreenData*       pScnData;

    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(3);

    pPltFile = LoadNCLR( &pPltData, "/data/BG.NCLR", &sAllocator );
    SDK_NULL_ASSERT( pPltFile );

    pChrFile = LoadNCGR( &pChrData, "/data/BG.NCGR", &sAllocator );
    SDK_NULL_ASSERT( pChrFile );

    pScnFile = LoadNSCR( &pScnData, "/data/BG.NSCR", &sAllocator );
    SDK_NULL_ASSERT( pScnFile );

    NNS_G2dBGSetup(
        NNS_G2D_BGSELECT_MAIN0,
        pScnData,
        pChrData,
        pPltData,
        GX_BG_SCRBASE_0x7800,
        GX_BG_CHARBASE_0x38000
    );

    NNS_FndFreeToAllocator( &sAllocator, pPltFile );
    NNS_FndFreeToAllocator( &sAllocator, pChrFile );
    NNS_FndFreeToAllocator( &sAllocator, pScnFile );
}



/*---------------------------------------------------------------------------*
  Name:         TXT_Alloc

  Description:  Allocates memory.

  Arguments:    size:   The size of the memory to allocate.

  Returns:      Pointer to the allocated memory region.
 *---------------------------------------------------------------------------*/
void* TXT_Alloc(u32 size)
{
    return NNS_FndAllocFromAllocator( &sAllocator, size );
}



/*---------------------------------------------------------------------------*
  Name:         TXT_Free

  Description:  Deallocates the memory allocated with TXT_Alloc().

  Arguments:    ptr:    Pointer to the memory region to deallocate.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_Free(void* ptr)
{
    NNS_FndFreeToAllocator( &sAllocator, ptr );
}



/*---------------------------------------------------------------------------*
  Name:         TXT_LoadFont

  Description:  Loads a font from a file to memory.

  Arguments:    pFname: The font resource path.

  Returns:      The pointer to the loaded font.
 *---------------------------------------------------------------------------*/
void TXT_LoadFont( NNSG2dFont* pFont, const char* pFname )
{
    void* pBuf;

    pBuf = LoadNFTR( pFont, pFname, &sAllocator );
    if( pBuf == NULL )
    {
        OS_Panic("Fail to load font file(%s).", pFname);
    }

    return;
}



/*---------------------------------------------------------------------------*
  Name:         TXT_LoadFile

  Description:  Loads a file to memory.

  Arguments:    ppFile: Pointer to the buffer that receives the memory address where the file was loaded.
                        
                fpath:  The path of the file to load.

  Returns:      Returns the file size of the loaded file.
                If the file size is 0, this indicates that the file failed to load.
                In that case, the *ppFile value is invalid.
 *---------------------------------------------------------------------------*/
u32 TXT_LoadFile(void** ppFile, const char* fpath)
{
    return CMN_LoadFile(ppFile, fpath, &sAllocator);
}



/*---------------------------------------------------------------------------*
  Name:         TXT_SetCharCanvasOBJAttrs

  Description:  Configures all the parameters not configured by NNS_G2dArrangeOBJ* of successive OAMs.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TXT_SetCharCanvasOBJAttrs(
    GXOamAttr * oam,
    int num,
    int priority,
    GXOamMode mode,
    BOOL mosaic,
    GXOamEffect effect,
    int cParam,
    int rsParam
)
{
    int i;

    for( i = 0; i < num; i++ )
    {
        G2_SetOBJPriority(oam, priority);
        G2_SetOBJMode(oam, mode, cParam);
        G2_SetOBJEffect(oam, effect, rsParam);
        G2_OBJMosaic(oam, mosaic);

        oam++;
    }
}

