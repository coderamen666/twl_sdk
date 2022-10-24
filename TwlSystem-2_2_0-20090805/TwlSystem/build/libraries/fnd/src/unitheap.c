/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     unitheap.c

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
#include <nnsys/fnd/unitheap.h>
#include <nnsys/fnd/config.h>
#include "heapcommoni.h"


/* ========================================================================
    Macro Constants
   ======================================================================== */

// Minimum value for alignment
#define MIN_ALIGNMENT           4


/* ========================================================================
    static functions
   ======================================================================== */

/* ------------------------------------------------------------------------
    Memory block list operations
   ------------------------------------------------------------------------ */

static NNSiFndUntHeapMBlockHead*
PopMBlock(NNSiFndUntMBlockList* list)
{
    NNSiFndUntHeapMBlockHead* block = list->head;
    if (block)
    {
        list->head = block->pMBlkHdNext;
    }

    return block;
}

/*---------------------------------------------------------------------------*
  Name:         PushMBlock

  Description:  Adds a memory block to the top of the list.

  Arguments:    link:   List to be added to
                block:  Memory block to add

  Returns:      None.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE void
PushMBlock(
    NNSiFndUntMBlockList*       list,
    NNSiFndUntHeapMBlockHead*   block
)
{
    block->pMBlkHdNext = list->head;
    list->head = block;
}


/*---------------------------------------------------------------------------*
  Name:         GetUnitHeapHeadPtrFromHeapHead

  Description:  Gets the pointer to the unit heap header from the pointer to heap header.

  Arguments:    pHeapHd:  Pointer to the heap header

  Returns:      Returns the pointer to the unit heap header.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndUntHeapHead*
GetUnitHeapHeadPtrFromHeapHead(NNSiFndHeapHead* pHeapHd)
{
    return AddU32ToPtr(pHeapHd, sizeof(NNSiFndHeapHead));
}

static NNS_FND_INLINE BOOL
IsValidUnitHeapHandle(NNSFndHeapHandle handle)
{
    if(handle == NNS_FND_HEAP_INVALID_HANDLE)
    {
        return FALSE;
    }

    {
        NNSiFndHeapHead* pHeapHd = handle;
        return pHeapHd->signature == NNSI_UNTHEAP_SIGNATURE;
    }
}


/* ========================================================================
    External functions (non-public)
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNSi_FndDumpUnitHeap

  Description:  Displays the information in the unit heap.
                This function is used for debugging.

  Arguments:    heap:    Unit heap handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    void
    NNSi_FndDumpUnitHeap(NNSFndHeapHandle heap)
    {
        NNS_ASSERT(IsValidUnitHeapHandle(heap));

        {
            NNSiFndHeapHead *const pHeapHd = heap;
            NNSiFndUntHeapHead *const pUnitHeapHd = GetUnitHeapHeadPtrFromHeapHead(pHeapHd);
            const u32 heapSize = GetOffsetFromPtr(pHeapHd->heapStart, pHeapHd->heapEnd);

        	const u32 freeSize = NNS_FndCountFreeBlockForUnitHeap(heap) * pUnitHeapHd->mBlkSize;
        	const u32 usedSize = heapSize - freeSize;

            NNSi_FndDumpHeapHead(pHeapHd);

        	OS_Printf( "    %d / %d bytes (%6.2f%%) used\n",
        											usedSize, heapSize, 100.0f * usedSize / heapSize);
        }
    }

// #if ! defined(NNS_FINALROM)
#endif


/* ========================================================================
    External Functions (public)
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCreateUnitHeapEx

  Description:  Creates unit heap

  Arguments:    startAddress:  Start address of heap area
                heapSize:      Size of heap area
                memBlockSize:  Memory block size
                alignment:     Alignment of the memory block
                               The values 4, 8, 16, or 32 can be specified.
                optFlag:       Option flag

  Returns:      If the function succeeds, a handle for the created unit heap is returned.
                If function fails, NNS_FND_INVALID_HEAP_HANDLE is returned.
 *---------------------------------------------------------------------------*/
NNSFndHeapHandle
NNS_FndCreateUnitHeapEx(
    void*   startAddress,
    u32     heapSize,
    u32     memBlockSize,
    int     alignment,
    u16     optFlag
)
{
    NNSiFndHeapHead* pHeapHd;
    void* heapEnd;

    SDK_NULL_ASSERT(startAddress);

    // Check alignment
    NNS_ASSERT(alignment % MIN_ALIGNMENT == 0);
    NNS_ASSERT(MIN_ALIGNMENT <= alignment && alignment <= 32);

    pHeapHd = NNSi_FndRoundUpPtr(startAddress, MIN_ALIGNMENT);
    heapEnd = NNSi_FndRoundDownPtr(AddU32ToPtr(startAddress, heapSize), MIN_ALIGNMENT);

    if (ComparePtr(pHeapHd, heapEnd) > 0)
    {
        return NNS_FND_HEAP_INVALID_HANDLE;
    }

    memBlockSize = NNSi_FndRoundUp(memBlockSize, alignment);    // actual block size

    {
        NNSiFndUntHeapHead* pUntHeapHd = GetUnitHeapHeadPtrFromHeapHead(pHeapHd);
        void* heapStart = NNSi_FndRoundUpPtr(AddU32ToPtr(pUntHeapHd, sizeof(NNSiFndUntHeapHead)), alignment);
        u32 elementNum;

        if (ComparePtr(heapStart, heapEnd) > 0)
        {
            return NNS_FND_HEAP_INVALID_HANDLE;
        }

        elementNum = GetOffsetFromPtr(heapStart, heapEnd) / memBlockSize;
        if (elementNum == 0)
        {
            return NNS_FND_HEAP_INVALID_HANDLE;
        }

        heapEnd = AddU32ToPtr(heapStart, elementNum * memBlockSize);

        NNSi_FndInitHeapHead(           // common heap initializations
            pHeapHd,
            NNSI_UNTHEAP_SIGNATURE,
            heapStart,
            heapEnd,
            optFlag);

        pUntHeapHd->mbFreeList.head = heapStart;
        pUntHeapHd->mBlkSize = memBlockSize;

        {
            NNSiFndUntHeapMBlockHead* pMBlkHd = pUntHeapHd->mbFreeList.head;
            int i;

            for (i = 0; i < elementNum - 1; ++i, pMBlkHd = pMBlkHd->pMBlkHdNext)
            {
                pMBlkHd->pMBlkHdNext = AddU32ToPtr(pMBlkHd, memBlockSize);
            }

            pMBlkHd->pMBlkHdNext = NULL;
        }

        return pHeapHd;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndDestroyUnitHeap

  Description:  Destroys the unit heap.

  Arguments:    heap: Unit heap handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndDestroyUnitHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidUnitHeapHandle(heap));

    NNSi_FndFinalizeHeap(heap);
}


/*---------------------------------------------------------------------------*
  Name:         NNS_FndAllocFromUnitHeap

  Description:  Allocates a memory block from the unit heap.

  Arguments:    heap:   Unit heap handle

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
void*
NNS_FndAllocFromUnitHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidUnitHeapHandle(heap));

    {
        NNSiFndUntHeapHead* pUntHeapHd = GetUnitHeapHeadPtrFromHeapHead(heap);
        NNSiFndUntHeapMBlockHead* pMBlkHd = PopMBlock(&pUntHeapHd->mbFreeList);

        if (pMBlkHd)
        {
            FillAllocMemory(heap, pMBlkHd, pUntHeapHd->mBlkSize);
        }

        return pMBlkHd;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndFreeToUnitHeap

  Description:  This function returns the memory block to the unit heap.

  Arguments:    heap:     Unit heap handle
                memBlock: Pointer to the memory block to be returned

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndFreeToUnitHeap(
    NNSFndHeapHandle    heap,
    void*               memBlock
)
{
    NNS_ASSERT(IsValidUnitHeapHandle(heap));

    {
        NNSiFndUntHeapHead* pUntHeapHd = GetUnitHeapHeadPtrFromHeapHead(heap);

        FillFreeMemory(heap, memBlock, pUntHeapHd->mBlkSize);

        PushMBlock(&pUntHeapHd->mbFreeList, memBlock);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCountFreeBlockForUnitHeap

  Description:  Gets the number of free memory blocks in the unit heap.

  Arguments:    heap:     Unit heap handle

  Returns:      Returns the number of free memory blocks in the unit heap.
 *---------------------------------------------------------------------------*/
u32
NNS_FndCountFreeBlockForUnitHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidUnitHeapHandle(heap));

    {
        NNSiFndUntHeapHead* pUntHeapHd = GetUnitHeapHeadPtrFromHeapHead(heap);
        NNSiFndUntHeapMBlockHead* pMBlkHd = pUntHeapHd->mbFreeList.head;
        u32 cnt = 0;

        for (; pMBlkHd; pMBlkHd = pMBlkHd->pMBlkHdNext)
        {
            ++cnt;
        }

        return cnt;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCalcHeapSizeForUnitHeap

  Description:  Gets the required heap size based on the size and number of memory blocks.

  Arguments:    memBlockSize:  Size of memory blocks (in bytes)
                memBlockNum:   Total number of memory blocks to be allocated
                alignment:     Alignment of the memory block

  Returns:      Returns the required heap size.
 *---------------------------------------------------------------------------*/
u32
NNS_FndCalcHeapSizeForUnitHeap(
    u32     memBlockSize,
    u32     memBlockNum,
    int     alignment
)
{
    return
          // Size that the heap uses internally
          sizeof(NNSiFndHeapHead) + sizeof(NNSiFndUntHeapHead)

          // Maximum size required for adjusting the alignment
        + (alignment - 4)

          // Size required by all units
        + memBlockNum * NNSi_FndRoundUp(memBlockSize, alignment);
}
