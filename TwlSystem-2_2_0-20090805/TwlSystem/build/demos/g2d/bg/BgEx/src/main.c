/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - bg - BgEx
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
//      Display the BG using the compressed palette and partial character data.
//      Upper screen BG3: Normal BG
//      Upper screen BG1: BG that used the compressed palette
//      Lower screen BG0: BG that used the partial character
//      Lower screen BG2: BG that used the compressed palette and partial character
//
//  Using the Demo
//      None.
// ============================================================================

#include <nitro.h>
#include <nnsys/g2d.h>
#include <nnsys/fnd.h>
#include <string.h>

#include "loader.h"


#define BG_DATA_DIR             "/data/"

#define BG_FILENAME_NORMAL      BG_DATA_DIR "BG_normal"
    // Normal BG resource filename

#define BG_FILENAME_CMP         BG_DATA_DIR "BG_cmp"
    // Resource filename for BGs that use compressed palettes

#define BG_FILENAME_PART        BG_DATA_DIR "BG_part"
    // Resource filename for BGs that use partial characters

#define BG_FILENAME_CMP_PART    BG_DATA_DIR "BG_cmp_part"
    // Resource filename of BG that uses compressed palette and partial characters


// Round up to match alignment
#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

// Round down to match alignment
#define ROUND_DOWN(value, alignment) \
    ((u32)(value) & ~(alignment-1))


//Structure in which BG resource files have been collected
typedef struct BGData
{
    void*                       pPltFile;   // Pointer to palette file
    void*                       pChrFile;   // Pointer to the character file
    void*                       pScnFile;   // Pointer to the screen file
    NNSG2dPaletteData*          pPltData;   // Pointer to the palette data
    NNSG2dCharacterData*        pChrData;   // Pointer to the character data
    NNSG2dScreenData*           pScnData;   // Pointer to the screen data
    NNSG2dCharacterPosInfo*     pPosInfo;   // Pointer to the partial character position data
    NNSG2dPaletteCompressInfo*  pCmpInfo;   // Pointer to the palette compression data
}
BGData;



//------------------------------------------------------------------------------
// Global Variables

NNSFndAllocator     gAllocator;         // Memory allocator

//------------------------------------------------------------------------------
// Prototype Declarations
static void VBlankIntr(void);








//****************************************************************************
// Initialization process
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         InitInterrupt

  Description:  Initializes interrupt processing.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInterrupt( void )
{
    // Disable all individual interrupt flags
    (void)OS_SetIrqMask(0);

    // Enable master interrupt flag
    (void)OS_EnableIrq();
}



/*---------------------------------------------------------------------------*
  Name:         InitAllocator

  Description:  Initialize the sole allocator for the application.

  Arguments:    pAllocator: Pointer to the allocator to initialize.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocator( NNSFndAllocator* pAllocator )
{
    u32   arenaLow      = ROUND_UP  (OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = ROUND_DOWN(OS_GetMainArenaHi(), 16);
    u32   heapSize      = arenaHigh - arenaLow;
    void* heapMemory    = OS_AllocFromMainArenaLo(heapSize, 16);
    NNSFndHeapHandle       heapHandle;
    SDK_NULL_ASSERT( pAllocator );

    heapHandle = NNS_FndCreateExpHeap(heapMemory, heapSize);
    SDK_ASSERT( heapHandle != NNS_FND_HEAP_INVALID_HANDLE );

    NNS_FndInitAllocatorForExpHeap(pAllocator, heapHandle, 4);
}



/*---------------------------------------------------------------------------*
  Name:         InitFileSystem

  Description:  Initializes the file system.

  Arguments:    pAllocator: Pointer to the allocator

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitFileSystem( NNSFndAllocator* pAllocator )
{
    SDK_NULL_ASSERT( pAllocator );

    // Enables interrupts for the communications FIFO with the ARM7
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    // Initialize file system
    FS_Init( FS_DMA_NOT_USE );

    // File table cache
    if( pAllocator != NULL )
    {
        const u32   need_size = FS_GetTableSize();
        void    *p_table = NNS_FndAllocFromAllocator( pAllocator, need_size );
        SDK_ASSERT(p_table != NULL);

        (void)FS_LoadTable(p_table, need_size);
    }
}



/*---------------------------------------------------------------------------*
  Name:         ClearVram

  Description:  Clears the VRAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ClearVram( void )
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



/*---------------------------------------------------------------------------*
  Name:         AssignVramBanks

  Description:  Allocates VRAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void AssignVramBanks( void )
{
    GX_SetBankForBG(GX_VRAM_BG_256_AB);                         // VRAM-AB for BGs
    GX_SetBankForBGExtPltt(GX_VRAM_BGEXTPLTT_0123_E);           // VRAM-E for BG Ext Plt

    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);                   // VRAM-C for BGs
    GX_SetBankForSubBGExtPltt(GX_VRAM_SUB_BGEXTPLTT_0123_H);    // VRAM-H for BG Ext Plt
}



/*---------------------------------------------------------------------------*
  Name:         Init

  Description:  Performs initialization for the sample.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void Init( void )
{
    // Common initialization.
    {
        OS_Init();
        GX_Init();

        InitInterrupt();
    }

    {
        ClearVram();
        InitAllocator( &gAllocator );
        InitFileSystem( &gAllocator );
    }

    AssignVramBanks();
}









//****************************************************************************
// Main process of sample
//****************************************************************************

/*---------------------------------------------------------------------------*
  Name:         CatFileExt

  Description:  Returns a filename where the specified extension has been concatenated with the specified filename.
 
  Arguments:    fname:  Filename
                fext:   File extension.

  Returns:      Pointer to the buffer used to store the string given by concatenating fname and fext.
                Because this buffer is statically allocated within the function, the next time the function is called, the buffer's content will be destroyed.
                
 *---------------------------------------------------------------------------*/
static const char* CatFileExt(const char* fname, const char* fext)
{
    static char buf[FS_FILE_NAME_MAX + 1];
    SDK_NULL_ASSERT( fname );
    SDK_NULL_ASSERT( fext );
	SDK_ASSERT( strlen(fname) + strlen(fext) <= FS_FILE_NAME_MAX );

    (void)strcpy(buf, fname);
    (void)strcat(buf, fext);
    return buf;
}



/*---------------------------------------------------------------------------*
  Name:         LoadBGData

  Description:  Main function.

  Arguments:    pData:  Pointer to the BGData structure that stores pointers to loaded background resources.
                        
                fname:  File name of BG resources to be loaded (not including extension)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void LoadBGData( BGData* pData , const char* fname)
{
    SDK_NULL_ASSERT( pData );
    SDK_NULL_ASSERT( fname );

    // load from file
    {
        // color palette
        pData->pPltFile = LoadNCLREx(
            &pData->pPltData,
            &pData->pCmpInfo,
            CatFileExt(fname, ".NCLR"),
            &gAllocator
        );
        SDK_NULL_ASSERT( pData->pPltFile );

        // character
        pData->pChrFile = LoadNCGREx(
            &pData->pChrData,
            &pData->pPosInfo,
            CatFileExt(fname, ".NCGR"),
            &gAllocator
        );
        SDK_NULL_ASSERT( pData->pChrFile );

        // screen
        pData->pScnFile = LoadNSCR(
            &pData->pScnData,
            CatFileExt(fname, ".NSCR"),
            &gAllocator
        );
        SDK_NULL_ASSERT( pData->pScnFile );
    }
}



/*---------------------------------------------------------------------------*
  Name:         UnloadBGData

  Description:  Frees the memory used by BG resources loaded using LoadBGData.

  Arguments:    pData:  Pointer to the BGData structure used to store pointers to BG resources.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void UnloadBGData( BGData* pData )
{
    SDK_NULL_ASSERT( pData );
    SDK_NULL_ASSERT( pData->pPltFile );
    SDK_NULL_ASSERT( pData->pChrFile );
    SDK_NULL_ASSERT( pData->pScnFile );
    NNS_FndFreeToAllocator( &gAllocator, pData->pPltFile );
    NNS_FndFreeToAllocator( &gAllocator, pData->pChrFile );
    NNS_FndFreeToAllocator( &gAllocator, pData->pScnFile );
}



/*---------------------------------------------------------------------------*
  Name:         SetupBG

  Description:  Displays the BG using those BG resources specified for the specified BG.

  Arguments:    bg:         target background screen
                scrBase:    Screen base block.
                charBase:   Character base block.
                bgFileName: File name of BG resources to be loaded (excluding extension)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SetupBG(
    NNSG2dBGSelect bg,
    GXBGScrBase scrBase,
    GXBGCharBase charBase,
    const char* bgFileName
)
{
    BGData bgData;

    LoadBGData(&bgData, bgFileName);
    NNS_G2dBGSetupEx(
        bg,
        bgData.pScnData,
        bgData.pChrData,
        bgData.pPltData,
        bgData.pPosInfo,
        bgData.pCmpInfo,
        scrBase,
        charBase
    );
    UnloadBGData(&bgData);
}



/*---------------------------------------------------------------------------*
  Name:         BGSetup

  Description:  Displays BG.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void BGSetup( void )
{
    // set up registers
    {
        GX_SetVisiblePlane ( GX_PLANEMASK_BG1 | GX_PLANEMASK_BG3 );
        GXS_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG2 );

        G2_SetBG1Priority(0);
        G2_SetBG3Priority(1);
        G2S_SetBG0Priority(0);
        G2S_SetBG2Priority(1);
    }

    // load bg
    {
        SetupBG(
            NNS_G2D_BGSELECT_MAIN3,
            GX_BG_SCRBASE_0x0000,
            GX_BG_CHARBASE_0x08000,
            BG_FILENAME_NORMAL
        );
        SetupBG(
            NNS_G2D_BGSELECT_MAIN1,
            GX_BG_SCRBASE_0x0800,
            GX_BG_CHARBASE_0x10000,
            BG_FILENAME_CMP
        );
        SetupBG(
            NNS_G2D_BGSELECT_SUB0,
            GX_BG_SCRBASE_0x0000,
            GX_BG_CHARBASE_0x08000,
            BG_FILENAME_PART
        );
        SetupBG(
            NNS_G2D_BGSELECT_SUB2,
            GX_BG_SCRBASE_0x0800,
            GX_BG_CHARBASE_0x10000,
            BG_FILENAME_CMP_PART
        );
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
        Init();
        BGSetup();
    }

    {
        GX_DispOn();
        GXS_DispOn();
    }

    // Main loop
    while( TRUE )
    {
    }
}

