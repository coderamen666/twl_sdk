/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - g2d - Text - textdemolib
  File:     cmn.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "g2d_textdemolib.h"


// Game Pad
CMNGamePad CMNGamePadState;



/*---------------------------------------------------------------------------*
  Name:         CMN_InitInterrupt

  Description:  Initializes the interrupt.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CMN_InitInterrupt( void )
{
    // disable all individual interrupt flags
    (void)OS_SetIrqMask(0);

    // enable master interrupt flag
    (void)OS_EnableIrq();
}



/*---------------------------------------------------------------------------*
  Name:         CMN_BeginVBlankIntr

  Description:  Starts the V-blank interrupt.

  Arguments:    vBlankFunc: Pointer to the V-blank interrupt handler

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CMN_BeginVBlankIntr( OSIrqFunction vBlankFunc )
{
    // interrupt handler registration
    OS_SetIrqFunction(OS_IE_V_BLANK, vBlankFunc);

    // permit VBlank interrupts
    (void)OS_EnableIrqMask( OS_IE_V_BLANK );

    // Start V-blank interrupt generation
    (void)GX_VBlankIntr(TRUE);
}



/*---------------------------------------------------------------------------*
  Name:         CMN_InitAllocator

  Description:  Initializes an allocator so that the entire arena is covered.

  Arguments:    pAllocator: Pointer to the uninitialized allocator.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CMN_InitAllocator( NNSFndAllocator* pAllocator )
{
    u32   arenaLow      = MATH_ROUNDUP  ((u32)OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = MATH_ROUNDDOWN((u32)OS_GetMainArenaHi(), 16);
    u32   heapSize      = arenaHigh - arenaLow;
    void* heapMemory    = OS_AllocFromMainArenaLo(heapSize, 16);
    NNSFndHeapHandle    heapHandle;
    SDK_NULL_ASSERT( pAllocator );

    heapHandle = NNS_FndCreateExpHeap(heapMemory, heapSize);
    SDK_ASSERT( heapHandle != NNS_FND_HEAP_INVALID_HANDLE );

    NNS_FndInitAllocatorForExpHeap(pAllocator, heapHandle, 4);
}



/*---------------------------------------------------------------------------*
  Name:         CMN_InitFileSystem

  Description:  Enables the file system.
                In addition, loads file data into memory.

  Arguments:    pAllocator: Pointer to a valid allocator.

  Returns:      None.
 *---------------------------------------------------------------------------*/
// Prepare file system
void CMN_InitFileSystem( NNSFndAllocator* pAllocator )
{
    SDK_NULL_ASSERT( pAllocator );

    // Enables interrupts for the communications FIFO with the ARM7
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);

    // Initialize file system
    FS_Init( FS_DMA_NOT_USE );

    // file table cache
    if( pAllocator != NULL )
    {
        const u32   need_size = FS_GetTableSize();
        void    *p_table = NNS_FndAllocFromAllocator( pAllocator, need_size );
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }
}



/*---------------------------------------------------------------------------*
  Name:         CMN_ClearVram

  Description:  Clears the VRAM.
                All VRAM must be unallocated.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
// Clear VRAM
void CMN_ClearVram( void )
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
  Name:         CMN_ReadGamePad

  Description:  Reads the controller input and stores it in an internal buffer.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CMN_ReadGamePad(void)
{
    u16 status = PAD_Read();

    CMNGamePadState.trigger = (u16)(status                  & (status ^ CMNGamePadState.button));
    CMNGamePadState.release = (u16)(CMNGamePadState.button  & (status ^ CMNGamePadState.button));
    CMNGamePadState.button  = status;
}



/*---------------------------------------------------------------------------*
  Name:         CMN_LoadFile

  Description:  Loads a file to memory.
                When the file data is no longer needed, it is deallocated with the CMN_UnloadFile( *ppFile, pAlloc ) function.
                
                

  Arguments:    ppFile: Pointer to the buffer that receives the memory address where the file was loaded.
                        
                fpath:  The path of the file to load.
                pAlloc: Pointer to the allocator

  Returns:      Returns the file size of the loaded file.
                If the file size is 0, this indicates that the file failed to load.
                In that case, the *ppFile value is invalid.
 *---------------------------------------------------------------------------*/
u32 CMN_LoadFile(void** ppFile, const char* fpath, NNSFndAllocator* pAlloc)
{
    BOOL bSuccess;
    FSFile f;
    u32 length;
    u32 read;

    SDK_NULL_ASSERT( ppFile );
    SDK_NULL_ASSERT( fpath );
    SDK_NULL_ASSERT( pAlloc );

    FS_InitFile(&f);

    bSuccess = FS_OpenFile(&f, fpath);
    if( ! bSuccess )
    {
        OS_Warning("file (%s) not found", fpath);
        return 0;
    }

    length = FS_GetLength(&f);
    *ppFile = NNS_FndAllocFromAllocator(pAlloc, length);
    if( *ppFile == NULL )
    {
        OS_Warning("cant allocate memory for file: %s", fpath);
        return 0;
    }

    read = (u32)FS_ReadFile(&f, *ppFile, (s32)length);
    if( read != length )
    {
        OS_Warning("fail to load file: %s", fpath);
        NNS_FndFreeToAllocator(pAlloc, *ppFile);
        return 0;
    }

    bSuccess = FS_CloseFile(&f);
    if( ! bSuccess )
    {
        OS_Warning("fail to close file: %s", fpath);
    }

    return length;
}



/*---------------------------------------------------------------------------*
  Name:         CMN_UnloadFile

  Description:  Deallocates the file data.

  Arguments:    pFile :  Pointer to the file data
                pAlloc: Pointer to the allocator used in file loading

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CMN_UnloadFile(void* pFile, NNSFndAllocator* pAlloc)
{
    NNS_FndFreeToAllocator(pAlloc, pFile);
}



/*---------------------------------------------------------------------------*
  Name:         CMN_LoadArchive

  Description:  Load the archive designated with the path name into memory, then mount it in the file system.
                
                When the archive is no longer needed, it is deallocated with the CMN_RemoveArchive( [return value], pAllocator ) function.
                
                

  Arguments:    name:       name used to identify the archive on the file system
                path :       Path name of the archive.
                pAllocator: Pointer to the allocator

  Returns:      If archive load is successful, returns a pointer to a NNSFndArchive structure.
                If unsuccessful, returns NULL.
 *---------------------------------------------------------------------------*/
NNSFndArchive*
CMN_LoadArchive(const char* name, const char* path, NNSFndAllocator* pAllocator)
{
    FSFile          file;
    NNSFndArchive*  archive = NULL;

    SDK_NULL_ASSERT(name);
    SDK_NULL_ASSERT(path);
    SDK_NULL_ASSERT(pAllocator);

    FS_InitFile(&file);
    if (FS_OpenFile(&file, path))
    {
        u32 binarySize = FS_GetLength(&file);
        u32 memorySize = MATH_ROUNDUP(sizeof(NNSFndArchive), 16) + MATH_ROUNDUP(binarySize, 16);

        u8* memory     = (u8*)NNS_FndAllocFromAllocator(pAllocator, memorySize);

        if (memory != NULL)
        {
            u8* binary = memory + MATH_ROUNDUP(sizeof(NNSFndArchive), 16);

            if ((u32)FS_ReadFile(&file, binary, (s32)binarySize) == binarySize)
            {
                if (NNS_FndMountArchive((NNSFndArchive*)memory, name, binary))
                {
                    archive = (NNSFndArchive*)memory;
                }
            }
        }
        (void)FS_CloseFile(&file);
    }
    return archive;
}



/*---------------------------------------------------------------------------*
  Name:         CMN_RemoveArchive

  Description:  Deletes the specified archive from memory.

                This unmounts a designated archive from the file system,
                and deallocates the memory into which the archive was loaded.

  Arguments:    archive:    pointer to the NNS archive structure
                pAllocator: Pointer to the allocator used in loading the archive.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
CMN_RemoveArchive(NNSFndArchive* archive, NNSFndAllocator* pAllocator)
{
    SDK_NULL_ASSERT(archive);
    SDK_NULL_ASSERT(pAllocator);

    (void)NNS_FndUnmountArchive(archive);
    NNS_FndFreeToAllocator(pAllocator, archive);
}

