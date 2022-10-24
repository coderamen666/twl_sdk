/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - demos - uncompressBLZ
  File:     main.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9555 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "DEMO.h"

#define BLZ_FILE_PATH   "/data/image1.bin_BLZ.bin"

static void InitAlloc(void)
{
    void   *tempLo;
    OSHeapHandle hh;

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}


void NitroMain(void)
{
    int    result;
    u32    comp_size, uncomp_size;
    FSFile file[1];
    u8*    buf;

    //---------------------------------------------------------------------------
    // Initialize:
    // Enable IRQ interrupts and initialize VRAM
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    InitAlloc();
    FS_Init(3);
    OS_InitTick();

    //---------------------------------------------------------------------------
    // Map VRAM bank A onto LCDC.
    //---------------------------------------------------------------------------
    GX_SetBankForLCDC(GX_VRAM_LCDC_A);

    OS_TPrintf("uncompress BLZ file demo.\n");

    // Get the the data size before and after decompression
    FS_InitFile(file);
    
    OS_TPrintf("open BLZ file...\n");
    if (FS_OpenFileEx(file, BLZ_FILE_PATH, FS_FILEMODE_R))
    {
        // Before decompression
        comp_size = FS_GetFileLength(file);
        
        // After decompression
        if (!FS_SeekFile(file, -4, FS_SEEK_END))
        {
            (void)FS_CloseFile(file);
            OS_Panic("seek file failed.\n");
        }
        
        // From the footer of the BLZ file, get the increase in data size after decompression
        (void)FS_ReadFile(file, &uncomp_size, sizeof(u32));
        uncomp_size += comp_size;
        OS_TPrintf("success.\n");
    }
    else
    {
        OS_Panic("BLZ file opening failed.\n");
    }
    
    // In main memory, allocate a buffer of the size of obtained data
    OS_TPrintf("allocate buffer after uncompressing...\n");
    if (NULL == (buf = OS_Alloc(uncomp_size)))
    {
        OS_Panic("failed.\n");
    }
    
    // Load the BLZ data into the allocated buffer
    OS_TPrintf("load BLZ file to main memory.\n");
    (void)FS_SeekFileToBegin(file);
    if (FS_ReadFile(file, buf, (s32)comp_size) == -1)
    {
        (void)FS_CloseFile(file);
        OS_Panic("load failed.\n");
    }
    (void)FS_CloseFile(file);
    
    // BLZ decompression
    OS_TPrintf("uncompress BLZ file...\n");
    if ((result = MI_SecureUncompressBLZ(buf, comp_size, uncomp_size)) < 0)
    {
        OS_Panic("failed. (error: %d)\n", result);
    }
    OS_TPrintf("success. (%d -> %d byte)\n", comp_size, uncomp_size);
    
    // From VRAM transfer to display
    OS_TPrintf("display uncompressed file.\n");
    MI_DmaCopy32(3, buf, (void *)HW_LCDC_VRAM_A, 256 * 192 * sizeof(unsigned short));
    
    GX_SetGraphicsMode(GX_DISPMODE_VRAM_A, (GXBGMode)0, (GXBG0As)0);
    DEMOStartDisplay();
    OS_WaitVBlankIntr();
    
    // Free the allocated buffer
    OS_Free(buf);
    
    OS_TPrintf("demo end.\n");
    OS_Terminate();
}

//---------------------------------------------------------------------------
// V-Blank interrupt function:
//
// Interrupt handlers are registered on the interrupt table by OS_SetIRQFunction.
// OS_EnableIrqMask selects IRQ interrupts to enable, and
// OS_EnableIrq enables IRQ interrupts.
// Notice that you have to call OS_SetIrqCheckFlag to check a V-Blank interrupt.
//---------------------------------------------------------------------------
void VBlankIntr(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // Checking V-Blank interrupt
}
