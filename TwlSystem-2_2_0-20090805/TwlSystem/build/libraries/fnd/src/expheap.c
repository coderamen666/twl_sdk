/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     expheap.c

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
#include <nnsys/fnd/expheap.h>
#include <nnsys/fnd/config.h>
#include "heapcommoni.h"


/* ========================================================================
    Macro Constants
   ======================================================================== */

// signature for free memory block
#define MBLOCK_FREE_SIGNATURE   ('FR')

// signature for the memory block being used
#define MBLOCK_USED_SIGNATURE   ('UD')

// maximum value for the group ID
#define MAX_GROUPID             0xff

// default value for the group ID
#define DEFAULT_GROUPID         0

// minimum value for alignment
#define MIN_ALIGNMENT           4

// default memory allocation mode
#define DEFAULT_ALLOC_MODE      NNS_FND_EXPHEAP_ALLOC_MODE_FIRST

// minimum size to register as a free block (size not including header)
#define MIN_FREE_BLOCK_SIZE     4
// #define MIN_FREE_BLOCK_SIZE 16


/* ========================================================================
    Structure definitions
   ======================================================================== */
typedef struct NNSiMemRegion NNSiMemRegion;

struct NNSiMemRegion
{
    void*       start;
    void*       end;
};


/* ========================================================================
    Macro Functions
   ======================================================================== */

#if ! defined(NNS_FINALROM)

    // for warning output when checking the heap
    #define HEAP_WARNING(exp, ...) \
        (void) ((exp) && (OS_Printf(__VA_ARGS__), 0))

// #if ! defined(NNS_FINALROM)
#endif


/* ========================================================================
    static functions
   ======================================================================== */

/* ------------------------------------------------------------------------
    pointer operations
   ------------------------------------------------------------------------ */

static NNS_FND_INLINE void*
MaxPtr(void* a, void* b)
{
    return NNSiGetUIntPtr(a) > NNSiGetUIntPtr(b) ? a: b;
}

static NNS_FND_INLINE BOOL
IsValidExpHeapHandle(NNSFndHeapHandle handle)
{
    if (handle == NNS_FND_HEAP_INVALID_HANDLE)
    {
        return FALSE;
    }

    {
        NNSiFndHeapHead* pHeapHd = handle;
        return pHeapHd->signature == NNSI_EXPHEAP_SIGNATURE;
    }
}

/*---------------------------------------------------------------------------*
  Name:         GetExpHeapHeadPtrFromHeapHead

  Description:  Gets the pointer to the expanded heap header from the pointer to the heap header.

  Arguments:    pHHead:  pointer to the heap header

  Returns:      Returns the pointer to the expanded heap header.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndExpHeapHead*
GetExpHeapHeadPtrFromHeapHead(NNSiFndHeapHead* pHHead)
{
    return AddU32ToPtr(pHHead, sizeof(NNSiFndHeapHead));
}

/*---------------------------------------------------------------------------*
  Name:         GetHeapHeadPtrFromExpHeapHead

  Description:  Gets the pointer to the heap header from the pointer to the expanded heap header.

  Arguments:    pEHHead:  Pointer to the expanded heap header.

  Returns:      Returns the pointer to the heap header.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndHeapHead*
GetHeapHeadPtrFromExpHeapHead(NNSiFndExpHeapHead* pEHHead)
{
    return SubU32ToPtr(pEHHead, sizeof(NNSiFndHeapHead));
}

/*---------------------------------------------------------------------------*
  Name:         GetExpHeapHeadPtrFromHandle

  Description:  Gets the pointer to the expanded heap header from the expanded heap handle.

  Arguments:    heap:  expanded heap handle

  Returns:      Returns the pointer to the expanded heap header.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndExpHeapHead*
GetExpHeapHeadPtrFromHandle(NNSFndHeapHandle heap)
{
    return GetExpHeapHeadPtrFromHeapHead(heap);
}

/*---------------------------------------------------------------------------*
  Name:         GetMemPtrForMBlock

  Description:  Gets a pointer to the memory block from the pointer to the NNSiFndExpHeapMBlockHead structure.
                

  Arguments:    pMBlkHd:  pointer to the NNSiFndExpHeapMBlockHead structure

  Returns:      Returns the pointer to the memory block.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE void*
GetMemPtrForMBlock(NNSiFndExpHeapMBlockHead* pMBlkHd)
{
    return AddU32ToPtr(pMBlkHd, sizeof(NNSiFndExpHeapMBlockHead));
}

static NNS_FND_INLINE const void*
GetMemCPtrForMBlock(const NNSiFndExpHeapMBlockHead* pMBlkHd)
{
    return AddU32ToCPtr(pMBlkHd, sizeof(NNSiFndExpHeapMBlockHead));
}

/*---------------------------------------------------------------------------*
  Name:         GetMBlockHeadPtr

  Description:  Gets a pointer to the NNSiFndExpHeapMBlockHead structure from a pointer to the memory block.
                
                Gets the pointer to the memory block.

  Arguments:    memBlock:  pointer to the memory block

  Returns:      Returns the pointer to the NNSiFndExpHeapMBlockHead structure.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndExpHeapMBlockHead*
GetMBlockHeadPtr(void* memBlock)
{
    return SubU32ToPtr(memBlock, sizeof(NNSiFndExpHeapMBlockHead));
}

static NNS_FND_INLINE const NNSiFndExpHeapMBlockHead*
GetMBlockHeadCPtr(const void* memBlock)
{
    return SubU32ToCPtr(memBlock, sizeof(NNSiFndExpHeapMBlockHead));
}

static NNS_FND_INLINE void*
GetMBlockEndAddr(NNSiFndExpHeapMBlockHead* pMBHead)
{
    return AddU32ToPtr(GetMemPtrForMBlock(pMBHead), pMBHead->blockSize);
}

/* ------------------------------------------------------------------------
    NNSiFndExpHeapMBlockHead structure access functions
   ------------------------------------------------------------------------ */

/*---------------------------------------------------------------------------*
  Name:         GetAlignmentForMBlock

  Description:  Gets the alignment value for the NNSiFndExpHeapMBlockHead structure.

  Arguments:    pMBlkHd:  pointer to the NNSiFndExpHeapMBlockHead structure

  Returns:      Returns the alignment value for the NNSiFndExpHeapMBlockHead structure.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE u16
GetAlignmentForMBlock(const NNSiFndExpHeapMBlockHead* pMBlkHd)
{
    return (u16)NNSi_FndGetBitValue(pMBlkHd->attribute, 8, 7);
}

/*---------------------------------------------------------------------------*
  Name:         SetAlignmentForMBlock

  Description:  Sets the alignment value for the NNSiFndExpHeapMBlockHead structure.

  Arguments:    pMBlkHd:    pointer to the NNSiFndExpHeapMBlockHead structure
                alignment:  alignment value to be set

  Returns:      None.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE void
SetAlignmentForMBlock(
    NNSiFndExpHeapMBlockHead*   pMBlkHd,
    u16                         alignment
)
{
    NNSi_FndSetBitValue(pMBlkHd->attribute, 8, 7, alignment);
}

static NNS_FND_INLINE u16
GetGroupIDForMBlock(const NNSiFndExpHeapMBlockHead* pMBHead)
{
    return (u16)NNSi_FndGetBitValue(pMBHead->attribute, 0, 8);
}

static NNS_FND_INLINE void
SetGroupIDForMBlock(
    NNSiFndExpHeapMBlockHead*   pMBHead,
    u16                         id
)
{
    NNSi_FndSetBitValue(pMBHead->attribute, 0, 8, id);
}

static NNS_FND_INLINE u16
GetAllocDirForMBlock(const NNSiFndExpHeapMBlockHead* pMBHead)
{
    return (u16)NNSi_FndGetBitValue(pMBHead->attribute, 15, 1);
}

static NNS_FND_INLINE void
SetAllocDirForMBlock(
    NNSiFndExpHeapMBlockHead*   pMBHead,
    u16                         mode
)
{
    NNSi_FndSetBitValue(pMBHead->attribute, 15, 1, mode);
}


/* ------------------------------------------------------------------------
    NNSiFndExpHeapHead structure access functions
   ------------------------------------------------------------------------ */

/*---------------------------------------------------------------------------*
  Name:         GetAllocMode

  Description:  Gets the memory allocation mode for the expanded heap.

  Arguments:    pEHHead:  Pointer to the expanded heap header.

  Returns:      Returns the memory allocation mode for the expanded heap.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE u16
GetAllocMode(NNSiFndExpHeapHead* pEHHead)
{
    return (u16)NNSi_FndGetBitValue(pEHHead->feature, 0, 1);
}

/*---------------------------------------------------------------------------*
  Name:         SetAllocMode

  Description:  Sets the memory allocation mode for the expanded heap.

  Arguments:    pEHHead:  Pointer to the expanded heap header.
                mode:     Memory allocation mode.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE void
SetAllocMode(
    NNSiFndExpHeapHead* pEHHead,
    u16                 mode
)
{
    NNSi_FndSetBitValue(pEHHead->feature, 0, 1, mode);
}

static void
GetRegionOfMBlock(
    NNSiMemRegion*              region,
    NNSiFndExpHeapMBlockHead*   block
)
{
    region->start = SubU32ToPtr(block, GetAlignmentForMBlock(block));
    region->end   = GetMBlockEndAddr(block);
}


/* ------------------------------------------------------------------------
    memory block list operations
   ------------------------------------------------------------------------ */

static NNSiFndExpHeapMBlockHead*
RemoveMBlock(
    NNSiFndExpMBlockList*       list,
    NNSiFndExpHeapMBlockHead*   block
)
{
    NNSiFndExpHeapMBlockHead *const prev = block->pMBHeadPrev;
    NNSiFndExpHeapMBlockHead *const next = block->pMBHeadNext;

    // previous reference link
    if (prev)
    {
        prev->pMBHeadNext = next;
    }
    else
    {
        list->head = next;
    }

    // next reference link
    if (next)
    {
        next->pMBHeadPrev = prev;
    }
    else
    {
        list->tail = prev;
    }

    return prev;
}

static NNSiFndExpHeapMBlockHead*
InsertMBlock(
    NNSiFndExpMBlockList*       list,
    NNSiFndExpHeapMBlockHead*   target,
    NNSiFndExpHeapMBlockHead*   prev
)
{
    NNSiFndExpHeapMBlockHead* next;

    // previous reference link
    target->pMBHeadPrev = prev;
    if (prev)
    {
        next = prev->pMBHeadNext;
        prev->pMBHeadNext = target;
    }
    else
    {
        next = list->head;
        list->head = target;
    }

    // next reference link
    target->pMBHeadNext = next;
    if (next)
    {
        next->pMBHeadPrev = target;
    }
    else
    {
        list->tail = target;
    }

    return target;
}

/*---------------------------------------------------------------------------*
  Name:         AppendMBlock

  Description:  Adds a memory block to the end of the list.

  Arguments:    link:   list to be added to
                block:  memory block to add

  Returns:      None.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE void
AppendMBlock(
    NNSiFndExpMBlockList*       list,
    NNSiFndExpHeapMBlockHead*   block
)
{
    (void)InsertMBlock(list, block, list->tail);
}

/*---------------------------------------------------------------------------*
  Name:         InitMBlock

  Description:  Initializes a memory block.

  Arguments:    pRegion:    pointer to the structure representing the region used for the memory block
                signature:  memory block signature

  Returns:      Returns the pointer to the initialized memory block.
 *---------------------------------------------------------------------------*/
static NNSiFndExpHeapMBlockHead*
InitMBlock(
    const NNSiMemRegion*    pRegion,
    u16                     signature
)
{
    NNSiFndExpHeapMBlockHead* block = pRegion->start;

    block->signature = signature;
    block->attribute = 0;
    block->blockSize = GetOffsetFromPtr(GetMemPtrForMBlock(block), pRegion->end);
    block->pMBHeadPrev = NULL;
    block->pMBHeadNext = NULL;

    return block;
}

/*---------------------------------------------------------------------------*
  Name:         InitFreeMBlock

  Description:  Initializes the memory block for use as a free block.

  Arguments:    pRegion:    pointer to the structure representing the region used for the memory block

  Returns:      Returns the pointer to the initialized memory block.
 *---------------------------------------------------------------------------*/
static NNS_FND_INLINE NNSiFndExpHeapMBlockHead*
InitFreeMBlock(
    const NNSiMemRegion*    pRegion
)
{
    return InitMBlock(pRegion, MBLOCK_FREE_SIGNATURE);
}

/*---------------------------------------------------------------------------*
  Name:         InitExpHeap

  Description:  Initializes the expanded heap.

  Arguments:    startAddress:  memory start address for the expanded heap
                endAddress:    terminal address for the memory for the expanded heap incremented by one
                optFlag:       Option flag

  Returns:      Returns the pointer to the heap header.
 *---------------------------------------------------------------------------*/
static NNSiFndHeapHead*
InitExpHeap(
    void*   startAddress,
    void*   endAddress,
    u16     optFlag
)
{
    NNSiFndHeapHead* pHeapHd = startAddress;
    NNSiFndExpHeapHead* pExpHeapHd = GetExpHeapHeadPtrFromHeapHead(pHeapHd);

    NNSi_FndInitHeapHead(           // common heap initializations
        pHeapHd,
        NNSI_EXPHEAP_SIGNATURE,
        AddU32ToPtr(pExpHeapHd, sizeof(NNSiFndExpHeapHead)),    // heapStart
        endAddress,                                             // heapEnd
        optFlag);

    pExpHeapHd->groupID = DEFAULT_GROUPID;      // Group ID
    pExpHeapHd->feature = 0;
    SetAllocMode(pExpHeapHd, DEFAULT_ALLOC_MODE);

    // create a free block
    {
        NNSiFndExpHeapMBlockHead* pMBHead;
        NNSiMemRegion region;
        region.start = pHeapHd->heapStart;
        region.end   = pHeapHd->heapEnd;
        pMBHead = InitFreeMBlock(&region);

        // block list
        pExpHeapHd->mbFreeList.head = pMBHead;
        pExpHeapHd->mbFreeList.tail = pMBHead;
        pExpHeapHd->mbUsedList.head = NULL;
        pExpHeapHd->mbUsedList.tail = NULL;

        return pHeapHd;
    }
}

/*---------------------------------------------------------------------------*
  Name:         AllocUsedBlockFromFreeBlock

  Description:  Allocates a new memory block from free blocks.

  Arguments:    pEHHead:      Pointer to the expanded heap header.
                pMBHeadFree:  Pointer to the free block header.
                mblock:       Address for the memory block to be allocated.
                size:         Size of the memory block to be allocated.
                direction:    Allocation direction.

  Returns:      Returns a pointer to the start of the allocated memory block.
 *---------------------------------------------------------------------------*/
static void*
AllocUsedBlockFromFreeBlock(
    NNSiFndExpHeapHead*         pEHHead,
    NNSiFndExpHeapMBlockHead*   pMBHeadFree,
    void*                       mblock,
    u32                         size,
    u16                         direction
)
{
    NNSiMemRegion freeRgnT;
    NNSiMemRegion freeRgnB;
    NNSiFndExpHeapMBlockHead* pMBHeadFreePrev;

    GetRegionOfMBlock(&freeRgnT, pMBHeadFree);
    freeRgnB.end   = freeRgnT.end;
    freeRgnB.start = AddU32ToPtr(mblock, size);
    freeRgnT.end   = SubU32ToPtr(mblock, sizeof(NNSiFndExpHeapMBlockHead));

    pMBHeadFreePrev = RemoveMBlock(&pEHHead->mbFreeList, pMBHeadFree);  // delete the free block for the time being

    // when there is no space for creating a free block
    if (GetOffsetFromPtr(freeRgnT.start, freeRgnT.end) < sizeof(NNSiFndExpHeapMBlockHead) + MIN_FREE_BLOCK_SIZE)
    {
        freeRgnT.end = freeRgnT.start;  // include the alignment value for the block being used
    }
    else
    {
        pMBHeadFreePrev = InsertMBlock(&pEHHead->mbFreeList, InitFreeMBlock(&freeRgnT), pMBHeadFreePrev);
    }

    // when there is no space for creating a free block
    if (GetOffsetFromPtr(freeRgnB.start, freeRgnB.end) < sizeof(NNSiFndExpHeapMBlockHead) + MIN_FREE_BLOCK_SIZE)
    {
        freeRgnB.start= freeRgnB.end;   // include the block being used
    }
    else
    {
        (void)InsertMBlock(&pEHHead->mbFreeList, InitFreeMBlock(&freeRgnB), pMBHeadFreePrev);
    }

    // fill the memory for debugging
    FillAllocMemory(GetHeapHeadPtrFromExpHeapHead(pEHHead), freeRgnT.end, GetOffsetFromPtr(freeRgnT.end, freeRgnB.start));

    // create a new block
    {
        NNSiFndExpHeapMBlockHead* pMBHeadNewUsed;
        NNSiMemRegion region;

        region.start = SubU32ToPtr(mblock, sizeof(NNSiFndExpHeapMBlockHead));
        region.end   = freeRgnB.start;

        pMBHeadNewUsed = InitMBlock(&region, MBLOCK_USED_SIGNATURE);
        SetAllocDirForMBlock(pMBHeadNewUsed, direction);
        SetAlignmentForMBlock(pMBHeadNewUsed, (u16)GetOffsetFromPtr(freeRgnT.end, pMBHeadNewUsed));
        SetGroupIDForMBlock(pMBHeadNewUsed, pEHHead->groupID);
        AppendMBlock(&pEHHead->mbUsedList, pMBHeadNewUsed);
    }

    return mblock;
}

/*---------------------------------------------------------------------------*
  Name:         AllocFromHead

  Description:  Allocates the memory block from the start of the heap.

  Arguments:    pHeapHd:    pointer to the heap header
                size:       Size of the memory block to be allocated.
                alignment:  alignment value

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
static void*
AllocFromHead(
    NNSiFndHeapHead*    pHeapHd,
    u32                 size,
    int                 alignment
)
{
    NNSiFndExpHeapHead* pExpHeapHd = GetExpHeapHeadPtrFromHeapHead(pHeapHd);

    // Allocate the first one found?
    const BOOL bAllocFirst = GetAllocMode(pExpHeapHd) == NNS_FND_EXPHEAP_ALLOC_MODE_FIRST;

    NNSiFndExpHeapMBlockHead* pMBlkHd      = NULL;
    NNSiFndExpHeapMBlockHead* pMBlkHdFound = NULL;
    u32 foundSize = 0xffffffff;
    void* foundMBlock = NULL;

    // search for free block
    for (pMBlkHd = pExpHeapHd->mbFreeList.head; pMBlkHd; pMBlkHd = pMBlkHd->pMBHeadNext)
    {
        void *const mblock    = GetMemPtrForMBlock(pMBlkHd);
        void *const reqMBlock = NNSi_FndRoundUpPtr(mblock, alignment);
        const u32 offset      = GetOffsetFromPtr(mblock, reqMBlock);    // generated offset

        if ( pMBlkHd->blockSize >= size + offset
         &&  foundSize > pMBlkHd->blockSize )
        {
            pMBlkHdFound  = pMBlkHd;
            foundSize     = pMBlkHd->blockSize;
            foundMBlock   = reqMBlock;

            if (bAllocFirst || foundSize == size)
            {
                break;
            }
        }
    }

    if (! pMBlkHdFound) // no blocks matching the conditions were found
    {
        return NULL;
    }

    return AllocUsedBlockFromFreeBlock( // allocate a region from the free block that was found
            pExpHeapHd,
            pMBlkHdFound,
            foundMBlock,
            size,
            NNS_FND_EXPHEAP_ALLOC_DIR_FRONT);
}

/*---------------------------------------------------------------------------*
  Name:         AllocFromTail

  Description:  Allocates a memory block from the end of the heap.

  Arguments:    pHeapHd:    pointer to the heap header
                size:       Size of the memory block to be allocated.
                alignment:  alignment value

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
static void*
AllocFromTail(
    NNSiFndHeapHead*    pHeapHd,
    u32                 size,
    int                 alignment
)
{
    NNSiFndExpHeapHead* pExpHeapHd = GetExpHeapHeadPtrFromHeapHead(pHeapHd);

    // Allocate the first one found?
    const BOOL bAllocFirst = GetAllocMode(pExpHeapHd) == NNS_FND_EXPHEAP_ALLOC_MODE_FIRST;

    NNSiFndExpHeapMBlockHead* pMBlkHd      = NULL;
    NNSiFndExpHeapMBlockHead* pMBlkHdFound = NULL;
    u32 foundSize = 0xffffffff;
    void* foundMBlock = NULL;

    // search for free block
    for (pMBlkHd = pExpHeapHd->mbFreeList.tail; pMBlkHd; pMBlkHd = pMBlkHd->pMBHeadPrev)
    {
        void *const mblock    = GetMemPtrForMBlock(pMBlkHd);
        void *const mblockEnd = AddU32ToPtr(mblock, pMBlkHd->blockSize);
        void *const reqMBlock = NNSi_FndRoundDownPtr(SubU32ToPtr(mblockEnd, size), alignment);  // aligned address

        if ( ComparePtr(reqMBlock, mblock) >= 0
         &&  foundSize > pMBlkHd->blockSize )
        {
            pMBlkHdFound = pMBlkHd;
            foundSize    = pMBlkHd->blockSize;
            foundMBlock  = reqMBlock;

            if (bAllocFirst || foundSize == size)
            {
                break;
            }
        }
    }

    if (! pMBlkHdFound) // no blocks matching the conditions were found
    {
        return NULL;
    }

    return AllocUsedBlockFromFreeBlock( // allocate a region from the free block that was found
            pExpHeapHd,
            pMBlkHdFound,
            foundMBlock,
            size,
            NNS_FND_EXPHEAP_ALLOC_DIR_REAR);
}

/*---------------------------------------------------------------------------*
  Name:         RecycleRegion

  Description:  Incorporates an empty region into a free memory block.
                If it is adjacent to a free block, the free block is expanded.
                When neither adjacent to a free block nor having enough size for a free block, used as an alignment value for used blocks adjacent to the end.
                
                If there is no used block adjacent to the end of the empty region, the function fails.

  Arguments:    pEHHead:  Pointer to the expanded heap header.
                pRegion:  pointer to the empty region

  Returns:      Returns TRUE if the function is successful.
                Returns FALSE if it fails.
 *---------------------------------------------------------------------------*/
static BOOL
RecycleRegion(
    NNSiFndExpHeapHead*     pEHHead,
    const NNSiMemRegion*    pRegion
)
{
    NNSiFndExpHeapMBlockHead* pBlkPrFree  = NULL;   // immediately preceding free block
    NNSiMemRegion freeRgn = *pRegion;

    // search for free area next to the specified one
    {
        NNSiFndExpHeapMBlockHead* pBlk;

        for (pBlk = pEHHead->mbFreeList.head; pBlk; pBlk = pBlk->pMBHeadNext)
        {
            if (pBlk < pRegion->start)
            {
                pBlkPrFree = pBlk;
                continue;
            }

            if (pBlk == pRegion->end)   // Is the block adjacent to the end?
            {
                // combine the available regions
                freeRgn.end = GetMBlockEndAddr(pBlk);
                (void)RemoveMBlock(&pEHHead->mbFreeList, pBlk);

                // pad the header with NoUse
                FillNoUseMemory(GetHeapHeadPtrFromExpHeapHead(pEHHead), pBlk, sizeof(NNSiFndExpHeapMBlockHead));
            }
            break;
        }
    }

    // Is the immediately preceding free block adjacent to the front?
    if (pBlkPrFree && GetMBlockEndAddr(pBlkPrFree) == pRegion->start)
    {
        // combine the available regions
        freeRgn.start = pBlkPrFree;
        pBlkPrFree = RemoveMBlock(&pEHHead->mbFreeList, pBlkPrFree);
    }

    if (GetOffsetFromPtr(freeRgn.start, freeRgn.end) < sizeof(NNSiFndExpHeapMBlockHead)) // size is not suitable for use as a block
    {
        return FALSE;   // arrive here when attempting to shrink to a small size using the NNS_FndResizeForMBlockExpHeap function and when there are no free blocks at the end
                        // 
    }

    // fill the memory for debugging
    FillFreeMemory(GetHeapHeadPtrFromExpHeapHead(pEHHead), pRegion->start, GetOffsetFromPtr(pRegion->start, pRegion->end));

    (void)InsertMBlock(&pEHHead->mbFreeList,
        InitFreeMBlock(&freeRgn),   // create the free block
        pBlkPrFree);

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         CheckMBlock

  Description:  Checks whether the header contents for the memory block are valid.

  Arguments:    pMBHead:    pointer to the header for the memory block to be checked
                pHeapHd:    pointer to the expanded heap header
                signature:  memory block signature
                heapType:   memory block type (used or free)
                flag:       Flag.

  Returns:      Return TRUE if the content of the memory block header is appropriate. Otherwise return FALSE.
                
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static BOOL
    CheckMBlock(
        const NNSiFndExpHeapMBlockHead* pMBHead,
        NNSiFndHeapHead*                pHeapHd,
        u16                             signature,
        const char*                     heapType,
        u32                             flag
    )
    {
        const BOOL bPrint = 0 != (flag & NNS_FND_HEAP_ERROR_PRINT);
        const void *const memBlock = GetMemCPtrForMBlock(pMBHead);

        if (pHeapHd)
        {
            if ( NNSiGetUIntPtr(pMBHead) < NNSiGetUIntPtr(pHeapHd->heapStart)
              || NNSiGetUIntPtr(memBlock) > NNSiGetUIntPtr(pHeapHd->heapEnd)
            )
            {
                HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Bad %s memory block address. - address %08X, heap area [%08X - %08X)\n",
                    heapType, memBlock, pHeapHd->heapStart, pHeapHd->heapEnd);
                return FALSE;
            }
        }
        else
        {
            if (NNSiGetUIntPtr(pMBHead) >= 0x11000000)
            {
                HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Bad %s memory block address. - address %08X\n",
                    heapType, memBlock);
                return FALSE;
            }
        }

        if (pMBHead->signature != signature)    // Is the signature different?
        {
            HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Bad %s memory block signature. - address %08X, signature %04X\n",
                heapType, memBlock, pMBHead->signature);
            return FALSE;
        }

        if (pMBHead->blockSize >= 0x01000000)   // Size too big?
        {
            HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Too large %s memory block. - address %08X, block size %08X\n",
                heapType, memBlock, pMBHead->blockSize);
            return FALSE;
        }

        if (pHeapHd)
        {
            if (NNSiGetUIntPtr(memBlock) + pMBHead->blockSize > NNSiGetUIntPtr(pHeapHd->heapEnd))
            {
                HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " wrong size %s memory block. - address %08X, block size %08X\n",
                    heapType, memBlock, pMBHead->blockSize);
                return FALSE;
            }
        }

        return TRUE;
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         CheckUsedMBlock

  Description:  Checks whether the header contents for the memory block being used are valid.

  Arguments:    pMBHead:  pointer to the header for the memory block to be checked
                pHeapHd:  pointer to the expanded heap header
                flag:     Flag.

  Returns:      Return TRUE if the content of the memory block header is appropriate. Otherwise return FALSE.
                
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static NNS_FND_INLINE BOOL
    CheckUsedMBlock(
        const NNSiFndExpHeapMBlockHead* pMBHead,
        NNSiFndHeapHead*                pHeapHd,
        u32                             flag
    )
    {
        return CheckMBlock(pMBHead, pHeapHd, MBLOCK_USED_SIGNATURE, "used", flag);
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         CheckFreeMBlock

  Description:  Checks whether the header contents for the free memory block are valid.

  Arguments:    pMBHead:  pointer to the header for the memory block to be checked
                pHeapHd:  pointer to the expanded heap header
                flag:     Flag.

  Returns:      Return TRUE if the content of the memory block header is appropriate. Otherwise return FALSE.
                
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static NNS_FND_INLINE BOOL
    CheckFreeMBlock(
        const NNSiFndExpHeapMBlockHead* pMBHead,
        NNSiFndHeapHead*                pHeapHd,
        u32                             flag
    )
    {
        return CheckMBlock(pMBHead, pHeapHd, MBLOCK_FREE_SIGNATURE, "free", flag);
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         CheckMBlockPrevPtr

  Description:  Checks whether the link to before the memory block is correct.

  Arguments:    pMBHead:      pointer to the header for the memory block to be checked
                pMBHeadPrev:  pointer to the header for the memory block prior to the one being checked
                flag:         Flag.

  Returns:      Return TRUE when the link to before the memory block is correct. Otherwise return FALSE.
                
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static BOOL
    CheckMBlockPrevPtr(
        const NNSiFndExpHeapMBlockHead* pMBHead,
        const NNSiFndExpHeapMBlockHead* pMBHeadPrev,
        u32                             flag
    )
    {
        const BOOL bPrint = 0 != (flag & NNS_FND_HEAP_ERROR_PRINT);

        if (pMBHead->pMBHeadPrev != pMBHeadPrev)
        {
            HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Wrong link memory block. - address %08X, previous address %08X != %08X\n",
                GetMemCPtrForMBlock(pMBHead), pMBHead->pMBHeadPrev, pMBHeadPrev);
            return FALSE;
        }

        return TRUE;
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         CheckMBlockNextPtr

  Description:  Checks whether the link to after the memory block is correct.

  Arguments:    pMBHead:      pointer to the header for the memory block to be checked
                pMBHeadNext:  pointer to the header for the memory block after the one being checked
                flag:         Flag.

  Returns:      Return TRUE when the link to after the memory block is correct. Otherwise return FALSE.
                
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static BOOL
    CheckMBlockNextPtr(
        const NNSiFndExpHeapMBlockHead* pMBHead,
        const NNSiFndExpHeapMBlockHead* pMBHeadNext,
        u32                             flag
    )
    {
        const BOOL bPrint = 0 != (flag & NNS_FND_HEAP_ERROR_PRINT);

        if (pMBHead->pMBHeadNext != pMBHeadNext)
        {
            HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Wrong link memory block. - address %08X, next address %08X != %08X\n",
                GetMemCPtrForMBlock(pMBHead), pMBHead->pMBHeadNext, pMBHeadNext);
            return FALSE;
        }

        return TRUE;
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         CheckMBlockLinkTail

  Description:  Checks whether the memory block pointer is at the start or end of the memory block list.

  Arguments:    pMBHead:      pointer to the header for the memory block to be checked
                pMBHeadTail:  pointer to the memory block at the start or end of the memory block list
                headType:     string indicating the start or end
                flag:         Flag.

  Returns:      Return TRUE when the memory block pointer is at the start or end of the memory block list. Otherwise return FALSE.
                
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static BOOL
    CheckMBlockLinkTail(
        const NNSiFndExpHeapMBlockHead* pMBHead,
        const NNSiFndExpHeapMBlockHead* pMBHeadTail,
        const char*                     heapType,
        u32                             flag
    )
    {
        const BOOL bPrint = 0 != (flag & NNS_FND_HEAP_ERROR_PRINT);

        if (pMBHead != pMBHeadTail)
        {
            HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Wrong memory brock list %s pointer. - address %08X, %s address %08X != %08X\n",
                heapType, GetMemCPtrForMBlock(pMBHead), heapType, pMBHead, pMBHeadTail);
            return FALSE;
        }

        return TRUE;
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         IsValidUsedMBlock

  Description:  Checks whether the memory block being used is appropriate.

  Arguments:    memBlock:  memory block to be checked
                heap:      handle for the expanded heap containing the memory block
                           No check is performed for whether the memory block is included in the heap when NULL is specified.
                           

  Returns:      Returns TRUE if there is no problem with the specified memory block.
                Returns FALSE if there is a problem.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    static BOOL
    IsValidUsedMBlock(
        const void*         memBlock,
        NNSFndHeapHandle    heap
    )
    {
        NNSiFndHeapHead* pHeapHd = heap;

        if (! memBlock)
        {
            return FALSE;
        }

        return CheckUsedMBlock(GetMBlockHeadCPtr(memBlock), pHeapHd, 0);
    }

// #if ! defined(NNS_FINALROM)
#endif

/* ========================================================================
    external functions (non-public)
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNSi_FndDumpExpHeap

  Description:  Shows internal expanded heap information.
                This function is used for debugging.

  Arguments:    heap:    Handle for the expanded heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    void
    NNSi_FndDumpExpHeap(NNSFndHeapHandle heap)
    {
        NNS_ASSERT(IsValidExpHeapHandle(heap));

        {
            u32  usedSize = 0;
            u32  usedCnt = 0;
            u32  freeSize = 0;
            u32  freeCnt = 0;

            NNSiFndHeapHead* pHeapHd = heap;
            NNSiFndExpHeapHead* pExpHeapHd = GetExpHeapHeadPtrFromHandle(pHeapHd);

            NNSi_FndDumpHeapHead(pHeapHd);

            OS_Printf("     attr  address:   size    gid aln   prev_ptr next_ptr\n");   // header line

            // ---------------- UsedBlock dump ----------------
            OS_Printf("    (Used Blocks)\n" );

            if (pExpHeapHd->mbUsedList.head == NULL)
            {
                OS_Printf("     NONE\n");
            }
            else
            {
                NNSiFndExpHeapMBlockHead* pMBHead;

                for (pMBHead = pExpHeapHd->mbUsedList.head; pMBHead; pMBHead = pMBHead->pMBHeadNext)
                {
                    if (pMBHead->signature != MBLOCK_USED_SIGNATURE)
                    {
                        OS_Printf("    xxxxx %08x: --------  --- ---  (-------- --------)\nabort\n", pMBHead);
                        break;
                    }

                    OS_Printf("    %s %08x: %8d  %3d %3d  (%08x %08x)\n",
                        GetAllocDirForMBlock(pMBHead) == NNS_FND_EXPHEAP_ALLOC_DIR_REAR ? " rear" : "front",
                        GetMemPtrForMBlock(pMBHead),
                        pMBHead->blockSize,
                        GetGroupIDForMBlock( pMBHead ),
                        GetAlignmentForMBlock( pMBHead ),
                        pMBHead->pMBHeadPrev ? GetMemPtrForMBlock(pMBHead->pMBHeadPrev): NULL,
                        pMBHead->pMBHeadNext ? GetMemPtrForMBlock(pMBHead->pMBHeadNext): NULL
                    );

                    // ---- amount used
                    usedSize += sizeof(NNSiFndExpHeapMBlockHead) + pMBHead->blockSize + GetAlignmentForMBlock(pMBHead);

                    usedCnt ++;
                }
            }

            // ---------------- FreeBlock dump ----------------
            OS_Printf("    (Free Blocks)\n" );

            if (pExpHeapHd->mbFreeList.head == NULL)
            {
                OS_Printf("     NONE\n" );
            }
            else
            {
                NNSiFndExpHeapMBlockHead* pMBHead;

                for (pMBHead = pExpHeapHd->mbFreeList.head; pMBHead; pMBHead = pMBHead->pMBHeadNext)
                {
                    if (pMBHead->signature != MBLOCK_FREE_SIGNATURE)
                    {
                        OS_Printf("    xxxxx %08x: --------  --- ---  (-------- --------)\nabort\n", pMBHead);
                        break;
                    }

                    OS_Printf("    %s %08x: %8d  %3d %3d  (%08x %08x)\n",
                        " free",
                        GetMemPtrForMBlock(pMBHead),
                        pMBHead->blockSize,
                        GetGroupIDForMBlock( pMBHead ),
                        GetAlignmentForMBlock( pMBHead ),
                        pMBHead->pMBHeadPrev ? GetMemPtrForMBlock(pMBHead->pMBHeadPrev): NULL,
                        pMBHead->pMBHeadNext ? GetMemPtrForMBlock(pMBHead->pMBHeadNext): NULL
                    );

                    freeCnt ++;
                }
            }

            OS_Printf("\n");

            {
                u32 heapSize  = GetOffsetFromPtr(pHeapHd->heapStart, pHeapHd->heapEnd); // heap size (data region size)
                OS_Printf("    %d / %d bytes (%6.2f%%) used (U:%d F:%d)\n",
                                   usedSize, heapSize, 100.0 * usedSize / heapSize, usedCnt, freeCnt);
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
  Name:         NNS_FndCreateExpHeapEx

  Description:  Creates an expanded heap.

  Arguments:    startAddress: Start address of heap area
                size:         Size of heap area
                optFlag:      Option flag

  Returns:      If the function succeeds, a handle for the created expanded heap is returned.
                If the function fails, NNS_FND_HEAP_INVALID_HANDLE is returned.
 *---------------------------------------------------------------------------*/
NNSFndHeapHandle
NNS_FndCreateExpHeapEx(
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
     ||  GetOffsetFromPtr(startAddress, endAddress) < sizeof(NNSiFndHeapHead) + sizeof(NNSiFndExpHeapHead)
                                                        + sizeof(NNSiFndExpHeapMBlockHead) + MIN_ALIGNMENT
    )
    {
        return NNS_FND_HEAP_INVALID_HANDLE;
    }

    {   // initialization of the expanded heap
        NNSiFndHeapHead* pHeapHd = InitExpHeap(startAddress, endAddress, optFlag);
        return pHeapHd;  // the pointer to the heap header is used as the handle value
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndDestroyExpHeap

  Description:  Destroys the expanded heap.

  Arguments:    heap: Handle for the expanded heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndDestroyExpHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));

    NNSi_FndFinalizeHeap(heap);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndAllocFromExpHeapEx

  Description:  Allocates a memory block from the expanded heap.
                The memory block alignment can be specified.
                If a negative alignment value is specified, an available region is searched for from the back of the heap.

  Arguments:    heap:      Handle for the expanded heap.
                size:      Size of the memory block to be allocated (in bytes)
                alignment: Alignment of the memory block to be allocated
                           4, 8, 16, 32, -4, -8, -16 or -32 may be specified.

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
void*
NNS_FndAllocFromExpHeapEx(
    NNSFndHeapHandle    heap,
    u32                 size,
    int                 alignment)
{
    void* memory = NULL;

    NNS_ASSERT(IsValidExpHeapHandle(heap));

    // check alignment
    NNS_ASSERT(alignment % MIN_ALIGNMENT == 0);
    NNS_ASSERT(MIN_ALIGNMENT <= abs(alignment) && abs(alignment) <= 32);

    if (size == 0)
    {
        size = 1;
    }

    size = NNSi_FndRoundUp(size, MIN_ALIGNMENT);    // size actually allocated

    if (alignment >= 0)     // allocate from the front
    {
        memory = AllocFromHead(heap, size, alignment);
    }
    else                    // allocate from the back
    {
        memory = AllocFromTail(heap, size, -alignment);
    }

    return memory;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndResizeForMBlockExpHeap

  Description:  Changes the size of the memory block allocated from the expanded heap.

  Arguments:    heap:     Handle for the expanded heap.
                memBlock: Pointer to the memory block to be resized
                size:     New size to be allocated (in bytes)

  Returns:      Returns the size of the resized memory block (in bytes), if the function is successful.
                Returns 0 if the function fails.
 *---------------------------------------------------------------------------*/
u32
NNS_FndResizeForMBlockExpHeap(
    NNSFndHeapHandle    heap,
    void*               memBlock,
    u32                 size
)
{
    NNSiFndExpHeapHead* pEHHead;
    NNSiFndExpHeapMBlockHead* pMBHead;

    NNS_ASSERT(IsValidExpHeapHandle(heap));
    NNS_ASSERT(IsValidUsedMBlock(memBlock, heap));

    pEHHead = GetExpHeapHeadPtrFromHandle(heap);
    pMBHead = GetMBlockHeadPtr(memBlock);

    size = NNSi_FndRoundUp(size, MIN_ALIGNMENT);
    if (size == pMBHead->blockSize)  // when the block size is not changed
    {
        return size;
    }

    // for expanding the new area
    if (size > pMBHead->blockSize)
    {
        void* crUsedEnd = GetMBlockEndAddr(pMBHead);   // end address for the used block
        NNSiFndExpHeapMBlockHead* block;

        // search for the next free block
        for (block = pEHHead->mbFreeList.head; block; block = block->pMBHeadNext)
        {
            if (block == crUsedEnd)
            {
                break;
            }
        }

        // there is no next free block or the size is inadequate
        if (! block || size > pMBHead->blockSize + sizeof(NNSiFndExpHeapMBlockHead) + block->blockSize)
        {
            return 0;
        }

        {
            NNSiMemRegion rgnNewFree;
            void* oldFreeStart;
            NNSiFndExpHeapMBlockHead* nextBlockPrev;

            // get the free block region, and temporarily remove the free block
            GetRegionOfMBlock(&rgnNewFree, block);
            nextBlockPrev = RemoveMBlock(&pEHHead->mbFreeList, block);

            oldFreeStart = rgnNewFree.start;
            rgnNewFree.start = AddU32ToPtr(memBlock, size); // region position to be newly freed

            // when the remainder is smaller than the memory block size
            if (GetOffsetFromPtr(rgnNewFree.start, rgnNewFree.end) < sizeof(NNSiFndExpHeapMBlockHead))
            {
                rgnNewFree.start = rgnNewFree.end;  // absorbed into the used block
            }

            pMBHead->blockSize = GetOffsetFromPtr(memBlock, rgnNewFree.start);  // change the target block size

            // when the remainder is equal to or larger than the memory block size
            if (GetOffsetFromPtr(rgnNewFree.start, rgnNewFree.end) >= sizeof(NNSiFndExpHeapMBlockHead))
            {
                (void)InsertMBlock(&pEHHead->mbFreeList, InitFreeMBlock(&rgnNewFree), nextBlockPrev);   // create a new free block
            }

            FillAllocMemory(  // expanded portion fill
                heap,
                oldFreeStart,
                GetOffsetFromPtr(oldFreeStart, rgnNewFree.start));
        }
    }
    // when the new area is reduced
    else
    {
        NNSiMemRegion rgnNewFree;
        const u32 oldBlockSize = pMBHead->blockSize;

        rgnNewFree.start = AddU32ToPtr(memBlock, size); // region position to be newly freed
        rgnNewFree.end   = GetMBlockEndAddr(pMBHead);   // end address for the used block

        pMBHead->blockSize = size;  // change the target block size

        if (! RecycleRegion(pEHHead, &rgnNewFree))    // try to return to the free list
        {
            pMBHead->blockSize = oldBlockSize;  // restore to original form if failed
        }
    }

    return pMBHead->blockSize;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndFreeToExpHeap

  Description:  Returns the memory block to the expanded heap.

  Arguments:    heap:     Handle for the expanded heap.
                memBlock: Pointer to the memory block to be returned

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndFreeToExpHeap(
    NNSFndHeapHandle    heap,
    void*               memBlock
)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));

    {
        NNSiFndHeapHead* pHeapHd = heap;
        NNSiFndExpHeapHead* pExpHeapHd = GetExpHeapHeadPtrFromHandle(pHeapHd);
        NNSiFndExpHeapMBlockHead* pMBHead = GetMBlockHeadPtr(memBlock);
        NNSiMemRegion region;

        // Is it included in this heap?
        NNS_ASSERT(pHeapHd->heapStart <= memBlock && memBlock < pHeapHd->heapEnd);

        GetRegionOfMBlock(&region, pMBHead);
        (void)RemoveMBlock(&pExpHeapHd->mbUsedList, pMBHead);   // remove from the list being used
        (void)RecycleRegion(pExpHeapHd, &region);   // add the specified size from the specified address to the free region
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetTotalFreeSizeForExpHeap

  Description:  Gets the total size of the available regions of the expanded heap.

  Arguments:    heap:     Handle for the expanded heap.

  Returns:      Returns the total size of the available regions in the expanded heap (in bytes).
 *---------------------------------------------------------------------------*/
u32
NNS_FndGetTotalFreeSizeForExpHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));

    {
        u32 sumSize = 0;
        NNSiFndExpHeapHead* pEHHead = GetExpHeapHeadPtrFromHandle(heap);
        NNSiFndExpHeapMBlockHead* pMBHead;

        for(pMBHead = pEHHead->mbFreeList.head; pMBHead; pMBHead = pMBHead->pMBHeadNext)
        {
            sumSize += pMBHead->blockSize;
        }

        return sumSize;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetAllocatableSizeForExpHeapEx

  Description:  Gets a memory block of the maximum allocatable size from the expanded heap.
                The memory block alignment can be specified.

  Arguments:    heap:      Handle for the expanded heap.
                alignment: Alignment of the memory block to be allocated
                           The values 4, 8, 16, or 32 can be specified.

  Returns:      Returns the maximum allocatable size from the expanded heap (in bytes).
 *---------------------------------------------------------------------------*/
u32
NNS_FndGetAllocatableSizeForExpHeapEx(
    NNSFndHeapHandle    heap,
    int                 alignment
)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));

    // check alignment
    NNS_ASSERT(alignment % MIN_ALIGNMENT == 0);
    NNS_ASSERT(MIN_ALIGNMENT <= abs(alignment) && abs(alignment) <= 32);

    alignment = abs(alignment); // convert to a positive value just to be sure

    {
        NNSiFndExpHeapHead* pEHHead = GetExpHeapHeadPtrFromHandle(heap);
        u32 maxSize = 0;
        u32 offsetMin = 0xFFFFFFFF;
        NNSiFndExpHeapMBlockHead* pMBlkHd;

        for (pMBlkHd = pEHHead->mbFreeList.head; pMBlkHd; pMBlkHd = pMBlkHd->pMBHeadNext)
        {
            // memory block position giving consideration to the alignment
            void* baseAddress = NNSi_FndRoundUpPtr(GetMemPtrForMBlock(pMBlkHd), alignment);

            if (NNSiGetUIntPtr(baseAddress) < NNSiGetUIntPtr(GetMBlockEndAddr(pMBlkHd)))
            {
                const u32 blockSize = GetOffsetFromPtr(baseAddress, GetMBlockEndAddr(pMBlkHd));
                // empty area due to the alignment
                const u32 offset = GetOffsetFromPtr(GetMemPtrForMBlock(pMBlkHd), baseAddress);

                /*
                    when the size is large, or if the size is the same but the wasted space is smaller, the memory block is updated
                    
                 */
                if ( maxSize < blockSize
                 ||  (maxSize == blockSize && offsetMin > offset)
                )
                {
                    maxSize = blockSize;
                    offsetMin= offset;
                }
            }
        }

        return maxSize;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndSetAllocModeForExpHeap

  Description:  Sets the memory allocation mode for the expanded heap.

  Arguments:    heap:  Handle for the expanded heap.
                mode:  Memory allocation mode.

  Returns:      Returns the memory allocation mode for the previous expanded heap.
 *---------------------------------------------------------------------------*/
u16
NNS_FndSetAllocModeForExpHeap(
    NNSFndHeapHandle    heap,
    u16                 mode
)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));

    {
        NNSiFndExpHeapHead *const pEHHead = GetExpHeapHeadPtrFromHandle(heap);
        u16 oldMode = GetAllocMode(pEHHead);
        SetAllocMode(pEHHead, mode);

        return oldMode;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetAllocModeForExpHeap

  Description:  Gets the memory allocation mode for the expanded heap.

  Arguments:    heap:    Handle for the expanded heap.

  Returns:      Returns the memory allocation mode for the expanded heap.
 *---------------------------------------------------------------------------*/
u16
NNS_FndGetAllocModeForExpHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));
    return GetAllocMode(GetExpHeapHeadPtrFromHandle(heap));
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndSetGroupIDForExpHeap

  Description:  Sets the group ID for the expanded heap.

  Arguments:    heap:    Handle for the expanded heap.
                groupID: Group ID value to be set.

  Returns:      Returns the previous group ID value.
 *---------------------------------------------------------------------------*/
u16
NNS_FndSetGroupIDForExpHeap(
    NNSFndHeapHandle    heap,
    u16                 groupID)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));
    NNS_ASSERT(groupID <= MAX_GROUPID);

    {
        NNSiFndExpHeapHead* pEHHead = GetExpHeapHeadPtrFromHandle(heap);
        u16 oldGroupID = pEHHead->groupID;
        pEHHead->groupID = groupID;

        return oldGroupID;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetGroupIDForExpHeap

  Description:  Gets the group ID for the expanded heap.

  Arguments:    heap:    Handle for the expanded heap.

  Returns:      Returns the group ID value.
 *---------------------------------------------------------------------------*/
u16
NNS_FndGetGroupIDForExpHeap(NNSFndHeapHandle heap)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));

    return GetExpHeapHeadPtrFromHandle(heap)->groupID;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndVisitAllocatedForExpHeap

  Description:  Invokes the user-specified function for all memory blocks allocated from the expanded heap.
                
                The order of the memory blocks called by the visitor function is the order in which they were allocated.

                The visitor type NNSFndHeapVisitor is defined as below.

                    typedef void (*NNSFndHeapVisitor)(
                                    void*               memBlock,
                                    NNSFndHeapHandle    heap,
                                    u32                 userParam);

                                        memBlock:   pointer to the memory block
                                        heap:       heap that includes the memory block
                                        userParam:  user parameter

  Arguments:    heap:       Handle for the expanded heap.
                visitor:    Function called for each memory block
                userParam:  User-specified parameter passed to the visitor function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
NNS_FndVisitAllocatedForExpHeap(
    NNSFndHeapHandle    heap,
    NNSFndHeapVisitor   visitor,
    u32                 userParam
)
{
    NNS_ASSERT(IsValidExpHeapHandle(heap));
    SDK_NULL_ASSERT(visitor);

    {
        NNSiFndExpHeapMBlockHead* pMBHead = GetExpHeapHeadPtrFromHandle(heap)->mbUsedList.head;

        while (pMBHead)
        {
            NNSiFndExpHeapMBlockHead* pMBHeadNext = pMBHead->pMBHeadNext;
            (*visitor)(GetMemPtrForMBlock(pMBHead), heap, userParam);
            pMBHead = pMBHeadNext;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetSizeForMBlockExpHeap

  Description:  Gets the size of the memory block that was allocated from the expanded heap.

  Arguments:    memBlock:  pointer to the memory block for getting the size

  Returns:      Returns the size of the specified memory block (in bytes).
 *---------------------------------------------------------------------------*/
u32
NNS_FndGetSizeForMBlockExpHeap(const void* memBlock)
{
    NNS_ASSERT(IsValidUsedMBlock(memBlock, NULL));

    return GetMBlockHeadCPtr(memBlock)->blockSize;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetGroupIDForMBlockExpHeap

  Description:  Gets the group ID for the memory block allocated from the expanded heap.

  Arguments:    memBlock:  pointer to the memory block for getting the group ID

  Returns:      Returns the group ID for the specified memory block.
 *---------------------------------------------------------------------------*/
u16
NNS_FndGetGroupIDForMBlockExpHeap(const void* memBlock)
{
    NNS_ASSERT(IsValidUsedMBlock(memBlock, NULL));

    return GetGroupIDForMBlock(GetMBlockHeadCPtr(memBlock));
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetAllocDirForMBlockExpHeap

  Description:  Gets the allocation direction for the memory block allocated from the expanded heap.

  Arguments:    memBlock:  pointer to the memory block

  Returns:      Returns the allocation direction for the specified memory block.
 *---------------------------------------------------------------------------*/
u16
NNS_FndGetAllocDirForMBlockExpHeap(const void* memBlock)
{
    NNS_ASSERT(IsValidUsedMBlock(memBlock, NULL));

    return GetAllocDirForMBlock(GetMBlockHeadCPtr(memBlock));
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCheckExpHeap

  Description:  Checks whether the expanded heap has been destroyed.

  Arguments:    heap:     Handle for the expanded heap.
                optFlag:  Flag.

  Returns:      Returns TRUE if the heap is normal.
                Returns FALSE if the heap has an error.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    BOOL
    NNS_FndCheckExpHeap(
        NNSFndHeapHandle    heap,
        u32                 optFlag
    )
    {
        const BOOL bPrint = 0 != (optFlag & NNS_FND_HEAP_ERROR_PRINT);
        u32  totalBytes  = 0;

        if (! IsValidExpHeapHandle(heap))
        {
            HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Invalid heap handle. - %08X\n", heap);
            return FALSE;
        }

        {
            NNSiFndHeapHead *const pHeapHd = heap;
            NNSiFndExpHeapHead *const pExpHeapHd = GetExpHeapHeadPtrFromHeapHead(pHeapHd);
            NNSiFndExpHeapMBlockHead* pMBHead = NULL;
            NNSiFndExpHeapMBlockHead* pMBHeadPrev = NULL;

            // used block
            for (pMBHead = pExpHeapHd->mbUsedList.head; pMBHead; pMBHeadPrev = pMBHead, pMBHead = pMBHead->pMBHeadNext)
            {
                if ( ! CheckUsedMBlock(pMBHead, pHeapHd, optFlag)
                  || ! CheckMBlockPrevPtr(pMBHead, pMBHeadPrev, optFlag)   // Is the pointer to the previous block the same as the pointer to the memory block in the previous loop?
                )
                {
                    return FALSE;
                }

                // calculate size occupied
                totalBytes += sizeof(NNSiFndExpHeapMBlockHead) + pMBHead->blockSize + GetAlignmentForMBlock(pMBHead);
            }

            if (! CheckMBlockLinkTail(pMBHeadPrev, pExpHeapHd->mbUsedList.tail, "tail", optFlag))  // Is the last block indicating the pointer to the final block?
            {
                return FALSE;
            }

            // free block
            pMBHead = NULL;
            pMBHeadPrev = NULL;
            for (pMBHead = pExpHeapHd->mbFreeList.head; pMBHead; pMBHeadPrev = pMBHead, pMBHead = pMBHead->pMBHeadNext)
            {
                if ( ! CheckFreeMBlock(pMBHead, pHeapHd, optFlag)
                  || ! CheckMBlockPrevPtr(pMBHead, pMBHeadPrev, optFlag)   // Is the pointer to the previous block the same as the pointer to the memory block in the previous loop?
                )
                {
                    return FALSE;
                }

                // calculate size occupied
                totalBytes += sizeof(NNSiFndExpHeapMBlockHead) + pMBHead->blockSize;
            }

            if (! CheckMBlockLinkTail(pMBHeadPrev, pExpHeapHd->mbFreeList.tail, "tail", optFlag))  // Is the last block indicating the pointer to the final block?
            {
                return FALSE;
            }

            // Display all results.
            if (totalBytes != GetOffsetFromPtr(pHeapHd->heapStart, pHeapHd->heapEnd))
            {
                HEAP_WARNING(bPrint, "[NNS Foundation " "Exp" " Heap]" " Incorrect total memory block size. - heap size %08X, sum size %08X\n",
                    GetOffsetFromPtr(pHeapHd->heapStart, pHeapHd->heapEnd), totalBytes);
                return FALSE;
            }

            return TRUE;
        }
    }

// #if ! defined(NNS_FINALROM)
#endif

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCheckForMBlockExpHeap

  Description:  This function checks if the memory block of the expanded heap was destroyed.

  Arguments:    memBlock:  Pointer to the memory block to be checked.
                heap:      Handle for the expanded heap.
                optFlag:   Flag.

  Returns:      Returns TRUE if the memory block is valid.
                Returns FALSE if the memory block has an error.
 *---------------------------------------------------------------------------*/
#if ! defined(NNS_FINALROM)

    BOOL
    NNS_FndCheckForMBlockExpHeap(
        const void*         memBlock,
        NNSFndHeapHandle    heap,
        u32                 optFlag
    )
    {
        const NNSiFndExpHeapMBlockHead* pMBHead = NULL;
        NNSiFndHeapHead *const pHeapHd = heap;

        if (! memBlock)
        {
            return FALSE;
        }

        pMBHead = GetMBlockHeadCPtr(memBlock);

        if (! CheckUsedMBlock(pMBHead, pHeapHd, optFlag))
        {
            return FALSE;
        }

        if (pMBHead->pMBHeadPrev)
        {
            if ( ! CheckUsedMBlock(pMBHead->pMBHeadPrev, pHeapHd, optFlag)     // check of signature and size of previous block
              || ! CheckMBlockNextPtr(pMBHead->pMBHeadPrev, pMBHead, optFlag)  // Is the previous block's pointer to the next block indicating the current block?
            )
            {
                return FALSE;
            }
        }
        else
        {
            if (pHeapHd)
            {
                // When prev is NULL, the head pointer of the list should indicate the current (block).
                if (! CheckMBlockLinkTail(pMBHead, GetExpHeapHeadPtrFromHeapHead(pHeapHd)->mbUsedList.head, "head", optFlag))
                {
                    return FALSE;
                }
            }
        }

        if (pMBHead->pMBHeadNext)
        {
            if ( ! CheckUsedMBlock(pMBHead->pMBHeadNext, pHeapHd, optFlag)     // check of signature and size of next block
              || ! CheckMBlockPrevPtr(pMBHead->pMBHeadNext, pMBHead, optFlag)  // Is the next block's pointer to the previous block indicating the current block?
            )
            {
                return FALSE;
            }
        }
        else
        {
            if (pHeapHd)
            {
                // When next is NULL, the tail pointer of the list should indicate the current (block).
                if (! CheckMBlockLinkTail(pMBHead, GetExpHeapHeadPtrFromHeapHead(pHeapHd)->mbUsedList.tail, "tail", optFlag))
                {
                    return FALSE;
                }
            }
        }

        return TRUE;
    }

// #if ! defined(NNS_FINALROM)
#endif
