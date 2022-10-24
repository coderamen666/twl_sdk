/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - sharedFont
  File:     main.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-06#$
  $Rev: 10986 $
  $Author: ooshimay $
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <twl/na.h>
#include <DEMO.h>

static void PrintBootType();

static void InitDEMOSystem();
static void InitInteruptSystem();
static void InitAllocSystem();
static void InitFileSystem();

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  Main function.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
    OS_Init();
    InitInteruptSystem();
    InitFileSystem();
    InitAllocSystem();
    InitDEMOSystem();
    OS_Printf("*** start sharedFont demo\n");

    PrintBootType();

    // Load the system's internal font(s)
    {
        BOOL bSuccess;
        int sizeTable;
        int sizeFont;
        void* pBufferTable;
        void* pBufferFont;

        // Initialize the system internal font API
        bSuccess = NA_InitSharedFont();
        SDK_ASSERT( bSuccess );

        // Get the management information buffer size
        sizeTable = NA_GetSharedFontTableSize();
        SDK_ASSERT( sizeTable >= 0 );
        OS_TPrintf("shared font table size = %d bytes\n", sizeTable);

        pBufferTable = OS_Alloc((u32)sizeTable);
        SDK_POINTER_ASSERT(pBufferTable);

        // Load the management information
        bSuccess = NA_LoadSharedFontTable(pBufferTable);
        SDK_ASSERT( bSuccess );

        // Get the font size
        sizeFont = NA_GetSharedFontSize(NA_SHARED_FONT_WW_M);
        SDK_ASSERT( sizeFont >= 0 );
        OS_TPrintf("shared font(M) size = %d bytes\n", sizeFont);

        pBufferFont = OS_Alloc((u32)sizeFont);
        SDK_POINTER_ASSERT(pBufferFont);

        // Load the font
        bSuccess = NA_LoadSharedFont(NA_SHARED_FONT_WW_M, pBufferFont);
        SDK_ASSERT( bSuccess );

        OS_TPrintf("shared font loaded\n");

        /*
        TWL-System is needed in order to use the loaded fonts...
        {
            NNSG2dFont font;

            NNS_G2dFontInitUTF16(&font, pBufferFont);
            ...
        }
        */
    }

    OS_Printf("*** End of demo\n");
    DEMO_DrawFlip();
    OS_WaitVBlankIntr();

    OS_Terminate();
}


/*---------------------------------------------------------------------------*
  Name:         PrintBootType

  Description:  Prints the BootType.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PrintBootType()
{
    const OSBootType btype = OS_GetBootType();

    switch( btype )
    {
    case OS_BOOTTYPE_ROM:   OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_ROM\n"); break;
    case OS_BOOTTYPE_NAND:  OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_NAND\n"); break;
    default:
        {
            OS_Warning("unknown BootType(=%d)", btype);
        }
        break;
    }
}

/*---------------------------------------------------------------------------*
  Name:         InitDEMOSystem

  Description:  Configures display settings for console screen output.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitDEMOSystem()
{
    // Initialize screen display
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:         InitInteruptSystem

  Description:  Initializes interrupts.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitInteruptSystem()
{
    // Enable master interrupt flag
    (void)OS_EnableIrq();

    // Allow IRQ interrupts
    (void)OS_EnableInterrupts();
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocSystem

  Description:  Creates a heap and makes OS_Alloc usable.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocSystem()
{
    void* newArenaLo;
    OSHeapHandle hHeap;

    // Initialize the main arena's allocation system
    newArenaLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetMainArenaLo(newArenaLo);

    // Create a heap in the main arena
    hHeap = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hHeap);
}

/*---------------------------------------------------------------------------*
  Name:         InitFileSystem

  Description:  Initializes the file system and makes the ROM accessible.
                The InitInteruptSystem function must have been called before this function is.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitFileSystem()
{
    // Initialize file system
    FS_Init( FS_DMA_NOT_USE );
}
