/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - simple
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
    OS_Printf("*** start nandApp demo\n");

    PrintBootType();

    // Load file from ROM and display it
    // The ROM file system can also be used from NAND applications.
    {
        BOOL bSuccess;
        FSFile f;
        u32 fileSize;
        s32 readSize;
        void* pBuffer;

        FS_InitFile(&f);

        bSuccess = FS_OpenFileEx(&f, "build_time.txt", FS_FILEMODE_R);
        SDK_ASSERT( bSuccess );

        fileSize = FS_GetFileLength(&f);
        pBuffer = OS_Alloc(fileSize + 1);
        SDK_POINTER_ASSERT(pBuffer);

        readSize = FS_ReadFile(&f, pBuffer, (s32)fileSize);
        SDK_ASSERT( readSize == fileSize );

        bSuccess = FS_CloseFile(&f);
        SDK_ASSERT( bSuccess );

        ((char*)pBuffer)[fileSize] = '\0';
        OS_Printf("%s\n", pBuffer);
        OS_Free(pBuffer);
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
    // ESnable master interrupt flag
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
