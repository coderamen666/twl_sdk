/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - mcs - basic - nitro
  File:     nns_util.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include "nns_util.h"
#include <nnsys/misc.h>

GamePad          gGamePad;      // GamePad
NNSFndHeapHandle gAppHeap;      // Application heap (Use expanded heap)


/*---------------------------------------------------------------------------*
  Name:         InitMemory

  Description:  Creates the application heap.
                This heap uses the TWL-System expanded heap.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
InitMemory(void)
{
    u32   arenaLow      = ROUND_UP  (OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = ROUND_DOWN(OS_GetMainArenaHi(), 16);
    u32   appHeapSize   = arenaHigh - arenaLow;
    void* appHeapMemory = OS_AllocFromMainArenaLo(appHeapSize, 16);

    gAppHeap = NNS_FndCreateExpHeap(appHeapMemory, appHeapSize     );
}


/*---------------------------------------------------------------------------*
  Name:         LoadFile

  Description:  Allocates a buffer and loads a file.

  Arguments:    filename:  Name of the file to load

  Returns:      If data is read successfully, this function returns a pointer to the buffer storing the contents of the file.
                When this buffer is no longer needed it must be deallocated with the NNS_FndFreeToExpHeap function.
                
                Returns NULL if it failed to load.
 *---------------------------------------------------------------------------*/
static void*
LoadFile(const char* filename)
{
    FSFile  file;
    void*   dataBuf = NULL;
    BOOL    bSuccess;

    SDK_NULL_ASSERT(filename);

    FS_InitFile(&file);
    bSuccess = FS_OpenFile(&file, filename);

    if(bSuccess)
    {
        const u32 fileSize = FS_GetLength(&file);

        dataBuf = NNS_FndAllocFromExpHeapEx(gAppHeap, fileSize, 16);
        NNS_NULL_ASSERT(dataBuf);

        if(fileSize != FS_ReadFile(&file, dataBuf, (s32)fileSize))
        {
            NNS_FndFreeToExpHeap(gAppHeap, dataBuf);
            dataBuf = NULL;
        }
        else
        {
            DC_FlushRange(dataBuf, fileSize);
        }

        bSuccess = FS_CloseFile(&file);
        NNS_ASSERT(bSuccess);
    }else{
        OS_Warning("Can't find the file : %s ", filename);
    }

    return dataBuf;
}

/*---------------------------------------------------------------------------*
  Name:         LoadPicture

  Description:  Transfers data loaded from the file to VRAM: character, color palette and screen data.
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
LoadPicture(void)
{
    void* chrData = LoadFile("/data/dog256.chr");
    void* scnData = LoadFile("/data/dog256.scn");
    void* palData = LoadFile("/data/dog256.pal");

    // Load data into VRAM
    GX_LoadBG0Char(chrData, 0, 0x10000);
    GX_LoadBG0Scr (scnData, 0, 0x00800);
    GX_LoadBGPltt (palData, 0, 0x00200);
}


/*---------------------------------------------------------------------------*
  Name:         ReadGamePad

  Description:  Reads key and requests the trigger and release-trigger.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ReadGamePad(void)
{
    u16 status = PAD_Read();

    gGamePad.trigger = (u16)(status          & (status ^ gGamePad.button));
    gGamePad.release = (u16)(gGamePad.button & (status ^ gGamePad.button));
    gGamePad.button  = status;
}
