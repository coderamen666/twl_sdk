/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - fnd
  File:     unitheap.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_FND_UNITHEAP_H_
#define NNS_FND_UNITHEAP_H_

#include <nnsys/fnd/heapcommon.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================
    Type Definitions
   ======================================================================== */

typedef struct NNSiFndUntHeapMBlockHead NNSiFndUntHeapMBlockHead;

// Header information for memory block
struct NNSiFndUntHeapMBlockHead
{
    NNSiFndUntHeapMBlockHead*  pMBlkHdNext;    // Next block
};


typedef struct NNSiFndUntMBlockList NNSiFndUntMBlockList;

// Memory block list
struct NNSiFndUntMBlockList
{
    NNSiFndUntHeapMBlockHead*  head;           // Pointer for memory block linked to header
};


typedef struct NNSiFndUntHeapHead NNSiFndUntHeapHead;

// Header information for unit heap
struct NNSiFndUntHeapHead
{
    NNSiFndUntMBlockList    mbFreeList;     // Free list
    u32                     mBlkSize;       // Memory block size
};


/* ========================================================================
    Macro Functions
   ======================================================================== */

/*---------------------------------------------------------------------------*
  Name:         NNS_FndCreateUnitHeap

  Description:  Creates unit heap

  Arguments:    startAddress:  Start address of heap area
                heapSize:      Size of heap area
                memBlockSize:  Memory block size

  Returns:      If the function succeeds, a handle for the created unit heap is returned.
                If function fails, NNS_FND_INVALID_HEAP_HANDLE is returned.
 *---------------------------------------------------------------------------*/
#define                 NNS_FndCreateUnitHeap(startAddress, heapSize, memBlockSize) \
                            NNS_FndCreateUnitHeapEx(startAddress, heapSize, memBlockSize, NNS_FND_HEAP_DEFAULT_ALIGNMENT, 0)


/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetMemBlockSizeForUnitHeap

  Description:  Gets the memory block size for unit heap.

  Arguments:    heap:  Unit heap handle

  Returns:      Returns memory block size for unit heap.
 *---------------------------------------------------------------------------*/
#define                 NNS_FndGetMemBlockSizeForUnitHeap(heap) \
                            (((const NNSiFndUntHeapHead*)((const u8*)((const void*)(heap)) + sizeof(NNSiFndHeapHead)))->mBlkSize)


/* ========================================================================
    Function Prototypes
   ======================================================================== */

#if ! defined(NNS_FINALROM)

    void                    NNSi_FndDumpUnitHeap(
                                NNSFndHeapHandle    heap);

// #if ! defined(NNS_FINALROM)
#endif

NNSFndHeapHandle        NNS_FndCreateUnitHeapEx(
                            void*   startAddress,
                            u32     heapSize,
                            u32     memBlockSize,
                            int     alignment,
                            u16     optFlag);

void                    NNS_FndDestroyUnitHeap(
                            NNSFndHeapHandle    heap);

void*                   NNS_FndAllocFromUnitHeap(
                        	NNSFndHeapHandle    heap);

void                    NNS_FndFreeToUnitHeap(
                            NNSFndHeapHandle    heap,
                            void*               memBlock);

u32                     NNS_FndCountFreeBlockForUnitHeap(
                            NNSFndHeapHandle    heap);


u32                     NNS_FndCalcHeapSizeForUnitHeap(
                            u32     memBlockSize,
                            u32     memBlockNum,
                            int     alignment);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_FND_UNITHEAP_H_ */
#endif
