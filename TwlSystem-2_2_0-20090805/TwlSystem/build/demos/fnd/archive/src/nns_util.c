/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - fnd - archive
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

#define SYSTEM_HEAP_SIZE        64*1024

GamePad          gGamePad;      // GamePad
NNSFndHeapHandle gSysHeap;      // System heap    (Use expanded heap)
NNSFndHeapHandle gAppHeap;      // Application heap (Use expanded heap)


/*---------------------------------------------------------------------------*
  Name:         InitMemory

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
InitMemory(void)
{
    void* sysHeapMemory = OS_AllocFromMainArenaLo(SYSTEM_HEAP_SIZE, 16);

    u32   arenaLow      = ROUND_UP  (OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = ROUND_DOWN(OS_GetMainArenaHi(), 16);
    u32   appHeapSize   = arenaHigh - arenaLow;
    void* appHeapMemory = OS_AllocFromMainArenaLo(appHeapSize, 16);

    gSysHeap = NNS_FndCreateExpHeap(sysHeapMemory, SYSTEM_HEAP_SIZE);
    gAppHeap = NNS_FndCreateExpHeap(appHeapMemory, appHeapSize     );
}

/*---------------------------------------------------------------------------*
  Name:         LoadArchive

  Description:  Load the archive designated with the path name into memory, then mount it in the file system.
                This implementation of the function guarantees that a loaded archive exists in main memory, by calling the DC_FlushRange function.
                
                

                The memory used to load the archive and the memory used for the NNSFndArchive structure, required to mount the archive, are allocated from the application heap.
                
                

  Arguments:    name:   Name used to identify the archive on the file system
                path :   Path name of the archive.

  Returns:      If archive load is successful, returns a pointer to a NNSFndArchive structure.
                If unsuccessful, returns NULL.
 *---------------------------------------------------------------------------*/
NNSFndArchive*
LoadArchive(const char* name, const char* path)
{
    FSFile          file;
    NNSFndArchive*  archive = NULL;

    FS_InitFile(&file);
    if (FS_OpenFile(&file, path))
    {
        u32 binarySize = FS_GetLength(&file);
        u32 memorySize = ROUND_UP(sizeof(NNSFndArchive), 16) + ROUND_UP(binarySize, 16);

        u8* memory     = (u8*)NNS_FndAllocFromExpHeapEx(gAppHeap, memorySize, 16);

        if (memory != NULL)
        {
            u8* binary = memory + ROUND_UP(sizeof(NNSFndArchive), 16);

            if (FS_ReadFile(&file, binary, (s32)binarySize) == binarySize)
            {
                DC_FlushRange(binary, binarySize);

                if (NNS_FndMountArchive((NNSFndArchive*)memory, name, binary))
                {
                    archive = (NNSFndArchive*)memory;
                }
                else
                {
                    NNS_FndFreeToExpHeap(gAppHeap, memory);
                }
            }
        }
        (void)FS_CloseFile(&file);
    }
    return archive;
}

/*---------------------------------------------------------------------------*
  Name:         RemoveArchive

  Description:  Deletes the specified archive from memory.
  
                This unmounts a designated archive from the file system,
                and deallocates the memory into which the archive was loaded.
                
  Arguments:    archive:    Pointer to the NNS archive structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
RemoveArchive(NNSFndArchive* archive)
{
    (void)NNS_FndUnmountArchive(archive);
    NNS_FndFreeToExpHeap(gAppHeap, archive);
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
