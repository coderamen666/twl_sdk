/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - fnd
  File:     expheap.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_FND_EXPHEAP_H_
#define NNS_FND_EXPHEAP_H_

#include <nnsys/fnd/heapcommon.h>

#ifdef __cplusplus
extern "C" {
#endif


/* =======================================================================
    Constant Definitions
   ======================================================================== */

// Memory allocation direction
enum
{
    NNS_FND_EXPHEAP_ALLOC_DIR_FRONT,    // Allocate from front
    NNS_FND_EXPHEAP_ALLOC_DIR_REAR      // Allocate from back
};

// Memory allocation mode
enum
{
    /*
        When this attribute value is set, the memory block is allocated from the first available region having at least the size of the memory block to be allocated.
        
        
    */
    NNS_FND_EXPHEAP_ALLOC_MODE_FIRST,

    /*
        When this attribute value is set, an available region having a size nearest to that of the memory block to be allocated is searched for, and the memory block is allocated from that available region.
        
        
    */
    NNS_FND_EXPHEAP_ALLOC_MODE_NEAR
};


/* =======================================================================
    Type Definitions
   ======================================================================== */

typedef struct NNSiFndExpHeapMBlockHead NNSiFndExpHeapMBlockHead;

// Header information for memory block
struct NNSiFndExpHeapMBlockHead
{
    u16                         signature;      // Signature
    u16                         attribute;      // Attribute
                                                // [8:groupID]
                                                // [7:alignment]
                                                // [1:temporary flag]

    u32                         blockSize;      // Block size (data area only)

    NNSiFndExpHeapMBlockHead*   pMBHeadPrev;    // Previous block
    NNSiFndExpHeapMBlockHead*   pMBHeadNext;    // Next block
};

typedef struct NNSiFndExpMBlockList NNSiFndExpMBlockList;

// Memory block list
struct NNSiFndExpMBlockList
{
    NNSiFndExpHeapMBlockHead*   head;   // Pointer for memory block linked to header
    NNSiFndExpHeapMBlockHead*   tail;   // Pointer to the memory block linked to the tail of the expanded heap
};

typedef struct NNSiFndExpHeapHead NNSiFndExpHeapHead;

// Header information for expanded heap
struct NNSiFndExpHeapHead
{
    NNSiFndExpMBlockList        mbFreeList;     // Free list
    NNSiFndExpMBlockList        mbUsedList;     // Used list

    u16                         groupID;        // Current group ID (lower 8 bits only)
    u16                         feature;        // Attribute
};

// Callback function type called for every memory block
typedef void        (*NNSFndHeapVisitor)(
                        void*               memBlock,
                        NNSFndHeapHandle    heap,
                        u32                 userParam);

/* =======================================================================
    Macro Functions
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCreateExpHeap

  Description:  Creates an expanded heap.

  Arguments:    startAddress: Start address of heap area
                size:         Size of heap area

  Returns:      If the function succeeds, a handle for the created expanded heap is returned.
                If the function fails, NNS_FND_HEAP_INVALID_HANDLE is returned.
 *---------------------------------------------------------------------------*/
#define             NNS_FndCreateExpHeap(startAddress, size) \
                        NNS_FndCreateExpHeapEx(startAddress, size, 0)

/*---------------------------------------------------------------------------*
  Name:         NNS_FndAllocFromExpHeap

  Description:  Allocates a memory block from the expanded heap.
                Alignment of the memory block is 4-byte fixed.

  Arguments:    heap:   Handle for the expanded heap.
                size:   Size of the memory block to be allocated (in bytes)

  Returns:      Returns a pointer to the allocated memory block when that allocation succeeded.
                
                If the operation fails, NULL is returned.
 *---------------------------------------------------------------------------*/
#define             NNS_FndAllocFromExpHeap(heap, size) \
                        NNS_FndAllocFromExpHeapEx(heap, size, NNS_FND_HEAP_DEFAULT_ALIGNMENT)

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetAllocatableSizeForExpHeap

  Description:  Gets a memory block of the maximum allocatable size from the expanded heap.
                Alignment of the memory block is 4-byte fixed.

  Arguments:    heap:     Handle for the expanded heap.

  Returns:      Returns the maximum allocatable size from the expanded heap (in bytes).
 *---------------------------------------------------------------------------*/
#define             NNS_FndGetAllocatableSizeForExpHeap(heap) \
                        NNS_FndGetAllocatableSizeForExpHeapEx(heap, NNS_FND_HEAP_DEFAULT_ALIGNMENT)


/* =======================================================================
    Function Prototypes
   ======================================================================== */

#if ! defined(NNS_FINALROM)

    void                NNSi_FndDumpExpHeap(
                            NNSFndHeapHandle    heap);

// #if ! defined(NNS_FINALROM)
#endif

NNSFndHeapHandle    NNS_FndCreateExpHeapEx(
                        void*   startAddress,
                        u32     size,
                        u16     optFlag);

void                NNS_FndDestroyExpHeap(
                        NNSFndHeapHandle    heap);

void*               NNS_FndAllocFromExpHeapEx(
                        NNSFndHeapHandle    heap,
                        u32                 size,
                        int                 alignment);

u32                 NNS_FndResizeForMBlockExpHeap(
                        NNSFndHeapHandle    heap,
                        void*               memBlock,
                        u32                 size);

void                NNS_FndFreeToExpHeap(
                        NNSFndHeapHandle    heap,
                        void*               memBlock);

u32                 NNS_FndGetTotalFreeSizeForExpHeap(
                        NNSFndHeapHandle    heap);

u32                 NNS_FndGetAllocatableSizeForExpHeapEx(
                        NNSFndHeapHandle    heap,
                        int                 alignment);

u16                 NNS_FndSetAllocModeForExpHeap(
                        NNSFndHeapHandle    heap,
                        u16                 mode);

u16                 NNS_FndGetAllocModeForExpHeap(
                        NNSFndHeapHandle    heap);

u16                 NNS_FndSetGroupIDForExpHeap(
                        NNSFndHeapHandle    heap,
                        u16                 groupID);

u16                 NNS_FndGetGroupIDForExpHeap(
                        NNSFndHeapHandle    heap);

void                NNS_FndVisitAllocatedForExpHeap(
                        NNSFndHeapHandle    heap,
                        NNSFndHeapVisitor   visitor,
                        u32                 userParam);

u32                 NNS_FndGetSizeForMBlockExpHeap(
                        const void*         memBlock);

u16                 NNS_FndGetGroupIDForMBlockExpHeap(
                        const void*         memBlock);

u16                 NNS_FndGetAllocDirForMBlockExpHeap(
                        const void*         memBlock);

#if defined(NNS_FINALROM)

    #define             NNS_FndCheckExpHeap(heap, optFlag)                      (TRUE)

    #define             NNS_FndCheckForMBlockExpHeap(memBlock, heap, optFlag)   (TRUE)

// #if defined(NNS_FINALROM)
#else

    BOOL                NNS_FndCheckExpHeap(
                            NNSFndHeapHandle    heap,
                            u32                 optFlag);

    BOOL                NNS_FndCheckForMBlockExpHeap(
                            const void*         memBlock,
                            NNSFndHeapHandle    heap,
                            u32                 optFlag);

// #if defined(NNS_FINALROM)
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_FND_EXPHEAP_H_ */
#endif
