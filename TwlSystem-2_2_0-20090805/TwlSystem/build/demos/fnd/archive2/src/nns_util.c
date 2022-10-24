/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - fnd - archive2
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
  Name:         LoadCompressedArchive

  Description:  Expands the compressed archive specified by the path name into memory and
                mounts it in the file system.
                ntrcomp.exe is used to compress files.

                The memory used to load the archive and the memory used for the NNSFndArchive structure,
                required to mount the archive, are allocated from the application heap.
                

                Compressed data is allocated memory from the back of the expanded heap. Since memory is allocated for data from the front of the extended heap after expansion, fragmention does not occur after the memory storing compressed data is deallocated.
                
                

  Arguments:    name:   Name used to identify the archive on the file system
                path :   Path name of the archive.

  Returns:      If archive load is successful, returns a pointer to a NNSFndArchive structure.
                If unsuccessful, returns NULL.
 *---------------------------------------------------------------------------*/
NNSFndArchive*
LoadCompressedArchive(const char* name, const char* path)
{
    FSFile          file;
    u8*             compData;
    NNSFndArchive*  archive = NULL;

    FS_InitFile(&file);
    if (FS_OpenFile(&file, path))
    {
        u32 fileSize = FS_GetLength(&file);

        // Memory to store compressed data is allocated from the back of the expanded heap.
        // (If a negative number is specified for alignment, it is allocated from the back.)
        compData = NNS_FndAllocFromExpHeapEx(gAppHeap, fileSize, -16);
        
        if (compData == NULL)
        {
            (void)FS_CloseFile(&file);
            return NULL;
        }
        if (FS_ReadFile(&file, compData, (s32)fileSize) != fileSize)
        {
            NNS_FndFreeToExpHeap(gAppHeap, compData);
            (void)FS_CloseFile(&file);
            return NULL;
        }
        (void)FS_CloseFile(&file);
    }

    {
        // The NNSFndArchive structure and archive data are stored in a single memory block.
        u32 memorySize = ROUND_UP(sizeof(NNSFndArchive), 16) + ROUND_UP(MI_GetUncompressedSize(compData), 16);

        // Memory is allocated from the front of the heap for decompressed archive data.
        u8* memory = NNS_FndAllocFromExpHeapEx(gAppHeap, memorySize, 16);

        if (memory != NULL)
        {
            u8* binary = memory + ROUND_UP(sizeof(NNSFndArchive), 16);

            // Expand the archive.
            switch (MI_GetCompressionType(compData))
            {
                case MI_COMPRESSION_LZ      : MI_UncompressLZ16   (compData, binary);       break;
                case MI_COMPRESSION_HUFFMAN : MI_UncompressHuffman(compData, binary);       break;
                case MI_COMPRESSION_RL      : MI_UncompressRL16   (compData, binary);       break;
                default                     : SDK_ASSERT(FALSE);                            break;
            }
            DC_FlushRange(binary, MI_GetUncompressedSize(compData));

            if (NNS_FndMountArchive((NNSFndArchive*)memory, name, binary))
            {
                archive = (NNSFndArchive*)memory;
            }
            else
            {
                NNS_FndFreeToExpHeap(gAppHeap, memory);
            }
        }
        // Free the compressed data.
        NNS_FndFreeToExpHeap(gAppHeap, compData);
    }
    return archive;
}

/*---------------------------------------------------------------------------*
  Name:         RemoveArchive

  Description:  Deletes the specified archive from memory.
  
                This unmounts a designated archive from the file system,
                and deallocates the memory into which the archive was loaded.
                
  Arguments:    archive:    pointer to the NNS archive structure

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
