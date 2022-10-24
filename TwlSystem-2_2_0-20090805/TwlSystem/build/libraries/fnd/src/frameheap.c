/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     frameheap.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <stdlib.h>
#include <nitro.h>
#include <nnsys/misc.h>
#include <nnsys/fnd/frameheap.h>
#include <nnsys/fnd/config.h>
#include "heapcommoni.h"

/* ========================================================================
    Macro Constants
   ======================================================================== */

// minimum value for alignment
#define MIN_ALIGNMENT           4


/* ========================================================================
    static functions
   ======================================================================== */

static NNS_FND_INLINE BOOL
IsValidFrmHeapHandle(NNSFndHeapHandle handle)
{
    if(handle == NNS_FND_HEAP_INVALID_HANDLE)
    {
        return FALSE;
    }

    {
        NNSiFndHeapHead* pHeapHd = handle;
        return pHeapHd->signature == NNSI_FRMHEAP_SIGNATURE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetFrmHeapHeadPtrFromHeapHead

  Description:  Gets pointer to frame heap header from pointer to heap header.

  Arguments:    pHHead:  pointer to the heap header

  Returns:      Returns the pointer to the frame heap header.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndFrmHeapHead*
GetFrmHeapHeadPtrFromHeapHead(NNSiFndHeapHead* pHHead)
{
    return AddU32ToPtr(pHHead, sizeof(NNSiFndHeapHead));
}

/*---------------------------------------------------------------------------*
  Name:         GetHeapHeadPtrFromFrmHeapHead

  Description:  Gets heap header pointer from frame heap header pointer.

  Arguments:    pFrmHeapHd:  Pointer to the frame heap header.

  Returns:      Returns the pointer to the heap header.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndHeapHead*
GetHeapHeadPtrFromFrmHeapHead(NNSiFndFrmHeapHead* pFrmHeapHd)
{
    return SubU32ToPtr(pFrmHeapHd, sizeof(NNSiFndHeapHead));
}

/*---------------------------------------------------------------------------*
  Name:         InitFrameHeap

  Description:  Initializes the frame heap.

  Arguments:    startAddress:  starting address of memory used as frame heap
                endAddress:    ending address + 1 of the memory to be used as the frame heap
                optFlag:       Option flag

  Returns:      Returns the pointer to the heap header.
 *---------------------------------------------------------------------------*/
static NNSiFndHeapHead*
InitFrameHeap(
    void*   startAddress,
    void*   endAddress,
    u16     optFlag
)
{
    NNSiFndHeapHead* pHeapHd = startAddress;
    NNSiFndFrmHeapHead* pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(pHeapHd);

    NNSi_FndInitHeapHead(       // common heap initializations
        pHeapHd,
        NNSI_FRMHEAP_SIGNATURE,
        AddU32ToPtr(pFrmHeapHd, sizeof(NNSiFndFrmHeapHead)),    // heapStart
        endAddress,                                             // heapEnd
        optFlag);

    pFrmHeapHd->headAllocator = pHeapHd->heapStart;
    pFrmHeapHd->tailAllocator = pHeapHd->heapEnd;

    pFrmHeapHd->pState = NULL;   // status saving state location

    return pHeapHd;
}


/*---------------------------------------------------------------------------*
  Name:         AllocFromHead

  Description:  Allocates the memory block from the start of the heap.
                There is an alignment specification.

  Arguments:    pHHead:  pointer to the heap header
                size:    Size of the memory block to be allocated.
                alignment:  alignment value

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
static void*
AllocFromHead(
    NNSiFndFrmHeapHead* pFrmHeapHd,
    u32                 size,
    int                 alignment
)
{
    void* newBlock = NNSi_FndRoundUpPtr(pFrmHeapHd->headAllocator, alignment);
    void* endAddress = AddU32ToPtr(newBlock, size);

    if(NNSiGetUIntPtr(endAddress) > NNSiGetUIntPtr(pFrmHeapHd->tailAllocator))
    {
        return NULL;
    }

    FillAllocMemory(  // fill memory
        GetHeapHeadPtrFromFrmHeapHead(pFrmHeapHd),
        pFrmHeapHd->headAllocator,
        GetOffsetFromPtr(pFrmHeapHd->headAllocator, endAddress));

    pFrmHeapHd->headAllocator = endAddress;

    return newBlock;
}

/*---------------------------------------------------------------------------*
  Name:         AllocFromTail

  Description:  Allocates a memory block from the end of the heap.
                There is an alignment specification.

  Arguments:    pHHead:     pointer to the heap header
                size:       Size of the memory block to be allocated.
                alignment:  alignment value

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
static void*
AllocFromTail(
    NNSiFndFrmHeapHead* pFrmHeapHd,
    u32                 size,
    int                 alignment
)
{
    void* newBlock = NNSi_FndRoundDownPtr(SubU32ToPtr(pFrmHeapHd->tailAllocator, size), alignment);

    if(NNSiGetUIntPtr(newBlock) < NNSiGetUIntPtr(pFrmHeapHd->headAllocator))
    {
        return NULL;
    }

    FillAllocMemory(  // fill memory
        GetHeapHeadPtrFromFrmHeapHead(pFrmHeapHd),
        newBlock,
        GetOffsetFromPtr(newBlock, pFrmHeapHd->tailAllocator));

    pFrmHeapHd->tailAllocator = newBlock;

    return newBlock;
}

/*---------------------------------------------------------------------------*
  Name:         FreeHead

  Description:  Deallocates memory blocks allocated from the head of the heap area all at once.

  Arguments:    pHeapHd:  pointer to heap header

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
FreeHead(NNSiFndHeapHead* pHeapHd)
{
    NNSiFndFrmHeapHead* pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(pHeapHd);

    FillFreeMemory(
        pHeapHd,
        pHeapHd->heapStart,
        GetOffsetFromPtr(pHeapHd->heapStart, pFrmHeapHd->headAllocator));

    pFrmHeapHd->headAllocator = pHeapHd->heapStart;
    pFrmHeapHd->pState = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         FreeTail

  Description:  Deallocates the memory blocks allocated from the heap area all at once.

  Arguments:    pHeapHd:  pointer to heap header

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
FreeTail(NNSiFndHeapHead* pHeapHd)
{
    NNSiFndFrmHeapHead* pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(pHeapHd);

    FillFreeMemory(
        pHeapHd,
        pFrmHeapHd->tailAllocator,
        GetOffsetFromPtr(pFrmHeapHd->tailAllocator, pHeapHd->heapEnd));

    /*
        Reset the end allocation pointer for save information to avoid reusing deallocated memory blocks when allocated heaps are restored.
        
     */
    {
        NNSiFndFrmHeapState* pState;
        for (pState = pFrmHeapHd->pState; pState; pState = pState->pPrevState)
        {
            pState->tailAllocator = pHeapHd->heapEnd;
        }
    }

    pFrmHeapHd->tailAllocator = pHeapHd->heapEnd;
}

/*---------------------------------------------------------------------------*
  Name:         PrintSize

  Description:  Outputs the size and percentage.

  Arguments:    size:       target size
                wholeSize:  whole size

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static void
    PrintSize(
        u32     size,
        u32     wholeSize
    )
    {
        OS_Printf("%9d (%6.2f%%)", size, 100.0 * size / wholeSize);
    }

// #if ! defined(NNS_FINALROM)
#endif


/* ========================================================================
    external functions (non-public)
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNSi_FndGetFreeStartForFrmHeap

  Description:  Gets the start address of the free area of the frame heap.

  Arguments:    heap: Handle for the frame heap.

  Returns:      Returns the start address of the free area of the frame heap.
 *---------------------------------------------------------------------------*/
void*
NNSi_FndGetFreeStartForFrmHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    return GetFrmHeapHeadPtrFromHeapHead(heap)->headAllocator;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_FndGetFreeEndForFrmHeap

  Description:  Gets the end address of the free area of the frame heap.

  Arguments:    heap: Handle for the frame heap.

  Returns:      Returns the end address + 1 of the free area of the frame heap.
 *---------------------------------------------------------------------------*/
void*
NNSi_FndGetFreeEndForFrmHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    return GetFrmHeapHeadPtrFromHeapHead(heap)->tailAllocator;
}


/*---------------------------------------------------------------------------*
  Name:         NNSi_FndDumpFrmHeap

  Description:  Displays the information in the frame heap.
                This function is used for debugging.

  Arguments:    heap:    Handle for the frame heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    void
    NNSi_FndDumpFrmHeap(NNSFndHeapHandle heap)
    {
        NNS_ASSERT(IsValidFrmHeapHandle(heap));

        {
            NNSiFndHeapHead *const pHeapHd = heap;
            NNSiFndFrmHeapHead *const pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(pHeapHd);
            const u32 heapSize = GetOffsetFromPtr(pHeapHd->heapStart, pHeapHd->heapEnd);

            NNSi_FndDumpHeapHead(pHeapHd);

            OS_Printf(  "     head [%p - %p) ", pHeapHd->heapStart, pFrmHeapHd->headAllocator);
            PrintSize(GetOffsetFromPtr(pHeapHd->heapStart, pFrmHeapHd->headAllocator), heapSize);
            OS_Printf("\n     free                           ");
            PrintSize(GetOffsetFromPtr(pFrmHeapHd->headAllocator, pFrmHeapHd->tailAllocator), heapSize);
            OS_Printf("\n     tail [%p - %p) ", pFrmHeapHd->tailAllocator, pHeapHd->heapEnd);
            PrintSize(GetOffsetFromPtr(pFrmHeapHd->tailAllocator, pHeapHd->heapEnd), heapSize);
            OS_Printf("\n");

            if (pFrmHeapHd->pState)
            {
                NNSiFndFrmHeapState* pState;

                OS_Printf("    state : [tag]   [head]      [tail]\n");

                for (pState = pFrmHeapHd->pState; pState; pState = pState->pPrevState)
                {
                    OS_Printf("            '%c%c%c%c' : %p %p\n", pState->tagName >>24, (pState->tagName >>16) & 0xFF, (pState->tagName >>8) & 0xFF, pState->tagName & 0xFF,
                                            pState->headAllocator, pState->tailAllocator);
                }
            }

            OS_Printf("\n");
        }
    }

// #if ! defined(NNS_FINALROM)
#endif


/* ========================================================================
    external functions (public)
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCreateFrmHeapEx

  Description:  Creates a frame heap.

  Arguments:    startAddress: Start address of heap area
                size:         Size of heap area
                optFlag:      Option flag

  Returns:      Returns the handle for the created frame heap if the function succeeds.
                If function fails, NNS_FND_INVALID_HEAP_HANDLE is returned.

  Memo:         This is essentially not thread-safe.
                To make it a thread heap, either add an argument that specifies heap attributes or have it controlled with a function that sets those attributes.
                
 *---------------------------------------------------------------------------*/
NNSFndHeapHandle
NNS_FndCreateFrmHeapEx(
    void*   startAddress,
    u32     size,
    u16     optFlag
)
{
    void* endAddress;

    SDK_NULL_ASSERT(startAddress);

    endAddress   = NNSi_FndRoundDownPtr(AddU32ToPtr(startAddress, size), MIN_ALIGNMENT);
    startAddress = NNSi_FndRoundUpPtr(startAddress, MIN_ALIGNMENT);

    if ( NNSiGetUIntPtr(startAddress) > NNSiGetUIntPtr(endAddress)
     ||  GetOffsetFromPtr(startAddress, endAddress) < sizeof(NNSiFndHeapHead) + sizeof(NNSiFndFrmHeapHead)
    )
    {
        return NNS_FND_HEAP_INVALID_HANDLE;
    }

    {   // initialize frame heap
        NNSiFndHeapHead* pHHead = InitFrameHeap(startAddress, endAddress, optFlag);
        return pHHead;  // the pointer to the heap header is used as the handle value
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNS_FndDestroyFrmHeap

  Description:  Destroys the frame heap.

  Arguments:    heap: Handle for the frame heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndDestroyFrmHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    NNSi_FndFinalizeHeap(heap);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndAllocFromFrmHeapEx

  Description:  Allocates a memory block from the frame heap.
                The memory block alignment can be specified.
                If a negative alignment value is specified, an available region is searched for from the back of the heap.

  Arguments:    heap:      Handle for the frame heap.
                size:      Size of the memory block to be allocated (in bytes)
                alignment: Alignment of the memory block to be allocated
                           4, 8, 16, 32, -4, -8, -16 or -32 may be specified.

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
void*
NNS_FndAllocFromFrmHeapEx(
    NNSFndHeapHandle    heap,
    u32                 size,
    int                 alignment
)
{
    void* memory = NULL;
    NNSiFndFrmHeapHead* pFrmHeapHd;

    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    // check alignment
    NNS_ASSERT(alignment % MIN_ALIGNMENT == 0);
    NNS_ASSERT(MIN_ALIGNMENT <= abs(alignment) && abs(alignment) <= 32);

    pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(heap);

    if (size == 0)
    {
        size = 1;
    }

    size = NNSi_FndRoundUp(size, MIN_ALIGNMENT);

    if (alignment >= 0)   // allocate from the front of the heap
    {
        memory = AllocFromHead(pFrmHeapHd, size, alignment);
    }
    else                    // allocate from the end of the heap
    {
        memory = AllocFromTail(pFrmHeapHd, size, -alignment);
    }

    return memory;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndFreeToFrmHeap

  Description:  This function returns the memory block to the frame heap.

  Arguments:    heap: Handle for the frame heap.
                mode: method of returning memory block

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndFreeToFrmHeap(
    NNSFndHeapHandle    heap,
    int                 mode
)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    if (mode & NNS_FND_FRMHEAP_FREE_HEAD)
    {
        FreeHead(heap);
    }

    if (mode & NNS_FND_FRMHEAP_FREE_TAIL)
    {
        FreeTail(heap);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetAllocatableSizeForFrmHeapEx

  Description:  Gets the maximum allocatable size in the frame heap.
                The memory block alignment can be specified.

  Arguments:    heap:      Handle for the frame heap.
                alignment: Alignment of the memory block to be allocated
                           The values 4, 8, 16, or 32 can be specified.

  Returns:      Returns the maximum allocatable size in the frame heap (in bytes).
 *---------------------------------------------------------------------------*/
u32
NNS_FndGetAllocatableSizeForFrmHeapEx(
    NNSFndHeapHandle    heap,
    int                 alignment
)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    // check alignment
    NNS_ASSERT(alignment % MIN_ALIGNMENT == 0);
    NNS_ASSERT(MIN_ALIGNMENT <= abs(alignment) && abs(alignment) <= 32);

    alignment = abs(alignment); // convert to a positive value just to be sure

    {
        const NNSiFndFrmHeapHead* pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(heap);
        const void* block = NNSi_FndRoundUpPtr(pFrmHeapHd->headAllocator, alignment);

        if (NNSiGetUIntPtr(block) > NNSiGetUIntPtr(pFrmHeapHd->tailAllocator))
        {
            return 0;
        }

        return GetOffsetFromPtr(block, pFrmHeapHd->tailAllocator);
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNS_FndRecordStateForFrmHeap

  Description:  This function records the current use status of the frame heap.
                You can restore the recorded memory usage status later.
                20 bytes are used to record the status.

  Arguments:    heap:     Handle for the frame heap.
                tagName:  tag name given to the status record

  Returns:      Returns TRUE if the function is successful.
                Returns FALSE if unsuccessful.
 *---------------------------------------------------------------------------*/
BOOL
NNS_FndRecordStateForFrmHeap(
    NNSFndHeapHandle    heap,
    u32                 tagName
)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    {
        NNSiFndFrmHeapHead* pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(heap);
        void* oldHeadAllocator = pFrmHeapHd->headAllocator;

        // allocate memory for saving information
        NNSiFndFrmHeapState* pState = AllocFromHead(pFrmHeapHd, sizeof(NNSiFndFrmHeapState), MIN_ALIGNMENT);
        if (! pState)
        {
            return FALSE;
        }

        // store the current status
        pState->tagName       = tagName;
        pState->headAllocator = oldHeadAllocator;
        pState->tailAllocator = pFrmHeapHd->tailAllocator;
        pState->pPrevState    = pFrmHeapHd->pState;

        pFrmHeapHd->pState = pState;

        return TRUE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndFreeByStateToFrmHeap

  Description:  This function restores the frame heap memory block to the recorded status.
                Use the specified tag name to return to the memory use status immediately before the NNS_FndRecordStateForFrmHeap function was called.
                
                If 0 is specified for the tag name the status is returned to that in effect right before the last call of NNS_FndRecordStateForFrmHeap().
                

                When restoring with a specified tag name, any information that was recorded by subsequent calls of the NNS_FndRecordStateForFrmHeap function is lost.
                
                

  Arguments:    heap:     Handle for the frame heap.
                tagName:  tag name given to the status record

  Returns:      Returns TRUE if the function is successful.
                Returns FALSE if unsuccessful.
 *---------------------------------------------------------------------------*/
BOOL
NNS_FndFreeByStateToFrmHeap(
    NNSFndHeapHandle    heap,
    u32                 tagName
)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    {
        NNSiFndFrmHeapHead* pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(heap);
        NNSiFndFrmHeapState* pState = pFrmHeapHd->pState;

        if (tagName != 0)   // tag name is specified
        {
            for(; pState; pState = pState->pPrevState)
            {
                if(pState->tagName == tagName)
                    break;
            }
        }

        if (! pState)
        {
            return FALSE;
        }

        pFrmHeapHd->headAllocator = pState->headAllocator;
        pFrmHeapHd->tailAllocator = pState->tailAllocator;

        pFrmHeapHd->pState = pState->pPrevState;

        return TRUE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndAdjustFrmHeap

  Description:  This function deallocates an available region of the frame heap from the heap region and reduces the heap region by that amount.
                
                There must not be memory blocks allocated from the back of heap memory.
                

  Arguments:    heap:     Handle for the frame heap.

  Returns:      Returns the frame heap size in bytes after reduction, if the function is successful.
                
                Returns 0 if unsuccessful.
 *---------------------------------------------------------------------------*/
u32
NNS_FndAdjustFrmHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidFrmHeapHandle(heap));

    {
        NNSiFndHeapHead* pHeapHd = heap;
        NNSiFndFrmHeapHead* pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(pHeapHd);

        // The function fails if there are any memory blocks allocated from the end.
        if(0 < GetOffsetFromPtr(pFrmHeapHd->tailAllocator, pHeapHd->heapEnd))
        {
            return 0;
        }

        pFrmHeapHd->tailAllocator = pHeapHd->heapEnd = pFrmHeapHd->headAllocator;

        return GetOffsetFromPtr(heap, pHeapHd->heapEnd);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndResizeForMBlockFrmHeap

  Description:  This function changes the size of the memory block allocated from the frame heap.

                Memory blocks for which the size is to be changed must be allocated from the beginning of a heap's available region to its end.
                

  Arguments:    heap:      Handle for the frame heap.
                memBlock:  Pointer to the memory block to be resized
                newSize:   New size to be allocated (in bytes)
                           If a value less than 4 is specified, processing is performed as if 4 was specified.

  Returns:      Returns the size of the resized memory block (in bytes), if the function is successful.
                Returns 0 if the function fails.
 *---------------------------------------------------------------------------*/
u32
NNS_FndResizeForMBlockFrmHeap(
    NNSFndHeapHandle    heap,
    void*               memBlock,
    u32                 newSize
)
{
    NNSiFndHeapHead* pHeapHd = NULL;
    NNSiFndFrmHeapHead* pFrmHeapHd = NULL;

    NNS_ASSERT(IsValidFrmHeapHandle(heap));
    NNS_ASSERT(memBlock == NNSi_FndRoundDownPtr(memBlock, MIN_ALIGNMENT));  // check if at the minimum alignment

    pHeapHd = heap;
    pFrmHeapHd = GetFrmHeapHeadPtrFromHeapHead(pHeapHd);

    NNS_ASSERT(
            ComparePtr(pHeapHd->heapStart, memBlock) <= 0
        &&  ComparePtr(pFrmHeapHd->headAllocator, memBlock) > 0);           // be sure that memory blocks exist at the front
    NNS_ASSERT(
            pFrmHeapHd->pState == NULL
        ||  ComparePtr(pFrmHeapHd->pState, memBlock) < 0);                  // be sure that there is no status saved after the memory block

    /*
        Do not allow newSize to be 0.
        This is because if newSize is 0, the memory block specified by memBlock has ceased to exist.
    */
    if (newSize == 0)
    {
        newSize = 1;
    }
    newSize = NNSi_FndRoundUp(newSize, MIN_ALIGNMENT);

    {
        const u32 oldSize = GetOffsetFromPtr(memBlock, pFrmHeapHd->headAllocator);
        void* endAddress = AddU32ToPtr(memBlock, newSize);

        if (newSize == oldSize)  // when the block size is not changed
        {
            return newSize;
        }

        if (newSize > oldSize)  // during enlargement
        {
            if (ComparePtr(endAddress, pFrmHeapHd->tailAllocator) > 0)  // when the size is insufficient
            {
                return 0;
            }

            FillAllocMemory(heap, pFrmHeapHd->headAllocator, newSize - oldSize);
        }
        else                    // during reduction
        {
            FillFreeMemory(heap, endAddress, oldSize - newSize);
        }

        pFrmHeapHd->headAllocator = endAddress;

        return newSize;
    }
}
