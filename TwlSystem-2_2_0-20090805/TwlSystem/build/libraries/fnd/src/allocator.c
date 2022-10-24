/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     allocator.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/misc.h>
#include <nnsys/fnd/expheap.h>
#include <nnsys/fnd/frameheap.h>
#include <nnsys/fnd/unitheap.h>
#include <nnsys/fnd/allocator.h>

/* ========================================================================
    static functions
   ======================================================================== */

/* ------------------------------------------------------------------------
    for Exp Heap
   ------------------------------------------------------------------------ */

static void*
AllocatorAllocForExpHeap(
    NNSFndAllocator*    pAllocator,
    u32                 size
)
{
    NNSFndHeapHandle const heap = pAllocator->pHeap;
    int const alignment = (int)pAllocator->heapParam1;
    return NNS_FndAllocFromExpHeapEx(heap, size, alignment);
}

static void
AllocatorFreeForExpHeap(
    NNSFndAllocator*    pAllocator,
    void*               memBlock
)
{
    NNSFndHeapHandle const heap = pAllocator->pHeap;
    NNS_FndFreeToExpHeap(heap, memBlock);
}

/* ------------------------------------------------------------------------
    for Frame Heap
   ------------------------------------------------------------------------ */

static void*
AllocatorAllocForFrmHeap(
    NNSFndAllocator*    pAllocator,
    u32                 size
)
{
    NNSFndHeapHandle const heap = pAllocator->pHeap;
    int const alignment = (int)pAllocator->heapParam1;
    return NNS_FndAllocFromFrmHeapEx(heap, size, alignment);
}

/*
    Because memory deallocation cannot occur at the memory block level in the frame heap, this implementation does nothing.
    
*/
static void
AllocatorFreeForFrmHeap(
    NNSFndAllocator*    /*pAllocator*/,
    void*               /*memBlock*/
)
{
}


/* ------------------------------------------------------------------------
    for Unit Heap
   ------------------------------------------------------------------------ */

/*
    Returns NULL because a size that goes beyond the unit heap's memory block size cannot be allocated.
    
*/
static void*
AllocatorAllocForUnitHeap(
    NNSFndAllocator*    pAllocator,
    u32                 size
)
{
    NNSFndHeapHandle const heap = pAllocator->pHeap;

    if (size > NNS_FndGetMemBlockSizeForUnitHeap(heap))
    {
        return NULL;
    }

    return NNS_FndAllocFromUnitHeap(heap);
}

static void
AllocatorFreeForUnitHeap(
    NNSFndAllocator*    pAllocator,
    void*               memBlock
)
{
    NNSFndHeapHandle const heap = pAllocator->pHeap;
    NNS_FndFreeToUnitHeap(heap, memBlock);
}


/* ------------------------------------------------------------------------
    for SDK heap
   ------------------------------------------------------------------------ */

static void*
AllocatorAllocForSDKHeap(
    NNSFndAllocator*    pAllocator,
    u32                 size
)
{
    OSHeapHandle const heap = (int)pAllocator->pHeap;
    OSArenaId const id = (OSArenaId)pAllocator->heapParam1;
    return OS_AllocFromHeap(id, heap, size);
}

static void
AllocatorFreeForSDKHeap(
    NNSFndAllocator*    pAllocator,
    void*               memBlock
)
{
    OSHeapHandle const heap = (int)pAllocator->pHeap;
    OSArenaId const id = (OSArenaId)pAllocator->heapParam1;
    OS_FreeToHeap(id, heap, memBlock);
}



/* ========================================================================
    External Functions
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndAllocFromAllocator

  Description:  Allocates memory blocks from the allocator.

                In actual fact, allocation method depends on the implementation of both the allocator and the memory manager associated with it.
                

  Arguments:    pAllocator:  Address of the NNSFndAllocator structure
                size:        Memory block size (in bytes)

  Returns:      If memory block allocation succeeds, returns the beginning address of the block.
                Returns NULL if allocation fails.
 *---------------------------------------------------------------------------*/
void*
NNS_FndAllocFromAllocator(
    NNSFndAllocator*    pAllocator,
    u32                 size
)
{
    NNS_ASSERT(pAllocator);
    return (*pAllocator->pFunc->pfAlloc)(pAllocator, size);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndFreeToAllocator

  Description:  Deallocates a memory block and returns it to the allocator.

                In actual fact, the return method depends on the implementation of both the allocator and the memory manager associated with it.
                

  Arguments:    pAllocator:  Address of the NNSFndAllocator structure
                memBlock:    Pointer to the memory block to be deallocated

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndFreeToAllocator(
    NNSFndAllocator*    pAllocator,
    void*               memBlock
)
{
    NNS_ASSERT(pAllocator && memBlock);
    (*pAllocator->pFunc->pfFree)(pAllocator, memBlock);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndInitAllocatorForExpHeap

  Description:  Initializes the allocator so it can allocate and deallocate memory from the expanded heap.
                The alignment value for all memory blocks allocated with the allocator becomes the value specified in the alignment argument.
                

  Arguments:    pAllocator:  Address of the NNSFndAllocator structure
                heap:        Handle for the expanded heap.
                alignment:   Alignment value to apply to each allocated memory block

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndInitAllocatorForExpHeap(
    NNSFndAllocator*    pAllocator,
    NNSFndHeapHandle    heap,
    int                 alignment
)
{
    static const NNSFndAllocatorFunc sAllocatorFunc =
    {
        AllocatorAllocForExpHeap,
        AllocatorFreeForExpHeap,
    };

    pAllocator->pFunc = &sAllocatorFunc;
    pAllocator->pHeap = heap;
    pAllocator->heapParam1 = (u32)alignment;
    pAllocator->heapParam2 = 0; // not used
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndInitAllocatorForFrmHeap

  Description:  Initializes the allocator so it can allocate and deallocate memory from the frame heap.
                The alignment value for all memory blocks allocated with the allocator becomes the value specified in the alignment argument.
                

  Arguments:    pAllocator:  Address of the NNSFndAllocator structure
                heap:        Handle for the frame heap.
                alignment:   Alignment value to apply to each allocated memory block

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndInitAllocatorForFrmHeap(
    NNSFndAllocator*    pAllocator,
    NNSFndHeapHandle    heap,
    int                 alignment
)
{
    static const NNSFndAllocatorFunc sAllocatorFunc =
    {
        AllocatorAllocForFrmHeap,
        AllocatorFreeForFrmHeap,
    };

    pAllocator->pFunc = &sAllocatorFunc;
    pAllocator->pHeap = heap;
    pAllocator->heapParam1 = (u32)alignment;
    pAllocator->heapParam2 = 0; // not used
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndInitAllocatorForUnitHeap

  Description:  Initializes the allocator so it can allocate and deallocate memory from the unit heap.
                It is not possible to allocated a memory block that is larger than the unit heap memory block size.
                In such a case, the NS_FndAllocFromAllocator() function will return NULL.

  Arguments:    pAllocator:  Address of the NNSFndAllocator structure
                heap:        Unit heap handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndInitAllocatorForUnitHeap(
    NNSFndAllocator*    pAllocator,
    NNSFndHeapHandle    heap
)
{
    static const NNSFndAllocatorFunc sAllocatorFunc =
    {
        AllocatorAllocForUnitHeap,
        AllocatorFreeForUnitHeap,
    };

    pAllocator->pFunc = &sAllocatorFunc;
    pAllocator->pHeap = heap;
    pAllocator->heapParam1 = 0; // not used
    pAllocator->heapParam2 = 0; // not used
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndInitAllocatorForSDKHeap

  Description:  The allocator is initialized to allocate and deallocate memory from heaps created with the NITRO-SDK's OS_CreateHeap() function.
                

  Arguments:    pAllocator:  Address of the NNSFndAllocator structure
                id:          Arena ID for the arena that the heap is located in
                heap:        Heap handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndInitAllocatorForSDKHeap(
    NNSFndAllocator*    pAllocator,
    OSArenaId           id,
    OSHeapHandle        heap
)
{
    static const NNSFndAllocatorFunc sAllocatorFunc =
    {
        AllocatorAllocForSDKHeap,
        AllocatorFreeForSDKHeap,
    };

    pAllocator->pFunc = &sAllocatorFunc;
    pAllocator->pHeap = (void*)heap;
    pAllocator->heapParam1 = id;
    pAllocator->heapParam2 = 0; // not used
}

