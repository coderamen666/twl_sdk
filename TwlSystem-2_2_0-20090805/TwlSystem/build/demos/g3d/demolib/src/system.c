/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g3d - demolib
  File:     system.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "g3d_demolib/system.h"

#define DEFAULT_DMA_NUMBER      MI_DMA_MAX_NUM
#define SYSTEM_HEAP_SIZE        64*1024


G3DDemoGamePad   G3DDemo_GamePad;       // GamePad
NNSFndHeapHandle G3DDemo_SysHeap;       // System heap    (Use expanded heap)
NNSFndHeapHandle G3DDemo_AppHeap;       // Application heap (Use expanded heap)


/*---------------------------------------------------------------------------*
  Name:         G3DDemo_VBlankIntr

  Description:  V-Blank callback routine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
G3DDemo_VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_InitSystem

  Description:  Initializes NITRO.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
G3DDemo_InitSystem(void)
{
    OS_Init();
    FX_Init();
    GX_SetPower(GX_POWER_ALL);
    GX_Init();
    OS_InitTick();

    GX_DispOff();
    GXS_DispOff();

    OS_SetIrqFunction(OS_IE_V_BLANK, G3DDemo_VBlankIntr);

    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();

    FS_Init(DEFAULT_DMA_NUMBER);

    (void)GX_VBlankIntr(TRUE);         // To generate V-Blank interrupt request

    // Dummy read of GamePad.
    G3DDemo_ReadGamePad();
}

#define SYSTEM_HEAP_SIZE        64*1024

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_InitVRAM

  Description:  Performs the initialization of VRAM.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
G3DDemo_InitVRAM(void)
{
    // Assign all banks to LCDC
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);

    // Clear entire LCDC space
    MI_CpuClearFast( (void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE );
    
    // Disable bank assigned to LCDC
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast ((void*)HW_OAM    , 192, HW_OAM_SIZE   );  // Clear OAM
    MI_CpuFillFast ((void*)HW_DB_OAM , 192, HW_DB_OAM_SIZE);  // Clear OAM

    MI_CpuClearFast((void*)HW_PLTT   , HW_PLTT_SIZE   );      // Clear palette
    MI_CpuClearFast((void*)HW_DB_PLTT, HW_DB_PLTT_SIZE);      // Clear the standard palette
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_InitMemory

  Description:  Creates system heap and application heap.
                Both heaps use the TWL-System expanded heap.
                
                An area the size of SYSTEM_HEAP_SIZE is allocated from the main arena
                as memory for the system heap and allocates the remainder of the main arena
                as memory for the application heap.
                
                We assume the system heap will be used by system programs of the game system.
                Use the application heap to load data used by the game.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
G3DDemo_InitMemory(void)
{
    void* sysHeapMemory = OS_AllocFromMainArenaLo(SYSTEM_HEAP_SIZE, 16);

    u32   arenaLow      = G3DDEMO_ROUND_UP  (OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = G3DDEMO_ROUND_DOWN(OS_GetMainArenaHi(), 16);
    u32   appHeapSize   = arenaHigh - arenaLow;
    void* appHeapMemory = OS_AllocFromMainArenaLo(appHeapSize, 16);

    G3DDemo_SysHeap = NNS_FndCreateExpHeap(sysHeapMemory, SYSTEM_HEAP_SIZE);
    G3DDemo_AppHeap = NNS_FndCreateExpHeap(appHeapMemory, appHeapSize      );
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_LoadFile

  Description:  Load file
  
  Arguments:    path        Path to file

  Returns:      Address where file should be loaded
 *---------------------------------------------------------------------------*/
void* G3DDemo_LoadFile(const char *path)
{
    FSFile file;
    void*  memory;

    FS_InitFile(&file);
    if (FS_OpenFile(&file, path))
    {
        u32 fileSize = FS_GetLength(&file);

        memory = NNS_FndAllocFromExpHeapEx(G3DDemo_AppHeap, fileSize, 16);
        if (memory == NULL)
        {
            OS_Printf("no enough memory.\n");
        }
        else
        {
            if (FS_ReadFile(&file, memory, (s32)fileSize) != fileSize)   // If file is too big to be loaded
            {
                NNS_FndFreeToExpHeap(G3DDemo_AppHeap, memory);
                memory = NULL;
                OS_Printf("file reading failed.\n");
            }
        }
        (void)FS_CloseFile(&file);
    }
    return memory;
}

/*---------------------------------------------------------------------------*
  Name:         G3DDemo_ReadGamePad

  Description:  Reads key, and requests the trigger and release-trigger.
                
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void G3DDemo_ReadGamePad(void)
{
    u16 status = PAD_Read();

    G3DDemo_GamePad.trigger = (u16)(status                 & (status ^ G3DDemo_GamePad.button));
    G3DDemo_GamePad.release = (u16)(G3DDemo_GamePad.button & (status ^ G3DDemo_GamePad.button));
    G3DDemo_GamePad.button  = status;
}
