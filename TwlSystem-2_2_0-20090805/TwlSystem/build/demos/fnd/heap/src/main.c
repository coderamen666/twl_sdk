/*---------------------------------------------------------------------------*
  Project:  TWL-System - demos - fnd - heap
  File:     main.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/fnd.h>

#define HeapBufElementNum (4096/sizeof(u32))

static u32 sHeapBuf[HeapBufElementNum];

/*
    The structure that holds the group ID and the total size.

        This is used to count the size of the memory blocks with the same group ID using the function CalcGroupSizeForExpHeap().
        
 */
typedef struct TGroupSize TGroupSize;

struct TGroupSize
{
    u16     groupID;
    u16		padding;
    u32     size;
};

/*---------------------------------------------------------------------------*
  Name:         CalcGroupSizeForMBlockExpHeap

  Description:  
                The function called from the NNS_FndVisitAllocatedForExpHeap function for each memory block.
                When the memory block's group ID matches what was specified, the memory block's size is changed to a total value.
                

  Arguments:    memBlock:   Memory block
                heap:       The handle of the expanded heap
                userParam:  The pointer to the structure that holds the group ID and the total value

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
CalcGroupSizeForMBlockExpHeap(
    void*               memBlock,
    NNSFndHeapHandle    heap,
    u32                 userParam
)
{
    #pragma unused(heap)

    TGroupSize* pGrSize = (TGroupSize*)userParam;

    if (NNS_FndGetGroupIDForMBlockExpHeap(memBlock) == pGrSize->groupID)
    {
        pGrSize->size += NNS_FndGetSizeForMBlockExpHeap(memBlock);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CalcGroupSizeForExpHeap

  Description:  Totals the sizes of memory blocks having the specifed group ID among the expanded heap's memory blocks.
                

  Arguments:    heap: Handle for the expanded heap.

  Returns:      Returns the total size value of the memory blocks having the designated group ID.
 *---------------------------------------------------------------------------*/
static u32
CalcGroupSizeForExpHeap(
    NNSFndHeapHandle    heap,
    u16                 groupID
)
{
    TGroupSize groupSize;

    groupSize.groupID = groupID;
    groupSize.size = 0;

    NNS_FndVisitAllocatedForExpHeap(
        heap,
        CalcGroupSizeForMBlockExpHeap,
        (u32)&groupSize);

    return groupSize.size;
}

/*---------------------------------------------------------------------------*
  Name:         FreeDirForMBlockExpHeap

  Description:  
                The function called from the NNS_FndVisitAllocatedForExpHeap function for each memory block.
                Deallocates the memory if the direction of allocation for the memory block is the same as the value specified by userParam.
                

  Arguments:    memBlock:   Memory block
                heap:       Handle for the expanded heap.
                userParam:  The allocation direction of the memory block

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
FreeDirForMBlockExpHeap(
    void*               memBlock,
    NNSFndHeapHandle    heap,
    u32                 userParam
)
{
    if (NNS_FndGetAllocDirForMBlockExpHeap(memBlock) == userParam)
    {
        NNS_FndFreeToExpHeap(heap, memBlock);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FreeTailForExpHeap

  Description:  Frees all memory blocks allocated at the end of the expanded heap region.

  Arguments:    heap: Handle for the expanded heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
FreeTailForExpHeap(NNSFndHeapHandle heap)
{
    NNS_FndVisitAllocatedForExpHeap(
        heap,
        FreeDirForMBlockExpHeap,
        NNS_FND_EXPHEAP_ALLOC_DIR_REAR);
}


/*---------------------------------------------------------------------------*
  Name:         SampleExpHeap

  Description:  The sample of the expanded heap.

  Arguments:    heapAddress:  The starting address of the memory allocated to the heap.
                heapSize:     The size of the memory allocated to the heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SampleExpHeap(
    void*   heapAddress,
    u32     heapSize)
{
    void* pMBlocks[4];

    // Creation of the expanded heap
    NNSFndHeapHandle hExpHeap = NNS_FndCreateExpHeap(heapAddress, heapSize);

    // Normal memory block allocation
    pMBlocks[0] = NNS_FndAllocFromExpHeap(hExpHeap, 100);

    // Allocates from the end of the heap region. The default alignment value is 4.
    pMBlocks[2] = NNS_FndAllocFromExpHeapEx(hExpHeap, 100, -NNS_FND_HEAP_DEFAULT_ALIGNMENT);

    // Changes the group ID to 1
    (void)NNS_FndSetGroupIDForExpHeap(hExpHeap, 1);

    // Allocates the memory blocks. The alignment is 16.
    pMBlocks[1] = NNS_FndAllocFromExpHeapEx(hExpHeap, 200, 16);

    // Allocates from the end of the heap region. The alignment is 16.
    pMBlocks[3] = NNS_FndAllocFromExpHeapEx(hExpHeap, 200, -16);

    NNS_FndDumpHeap(hExpHeap);

    // Finds the total value of the size of the group 0 memory block.
    OS_Printf("[demo exp heap] group 0 size %d\n", CalcGroupSizeForExpHeap(hExpHeap, 0));

    // Finds the total value of the size of the group 1 memory block.
    OS_Printf("[demo exp heap] group 1 size %d\n", CalcGroupSizeForExpHeap(hExpHeap, 1));

    OS_Printf("\n");

    // Expansion of the memory block from 100 to 300
    (void)NNS_FndResizeForMBlockExpHeap(hExpHeap, pMBlocks[1], 300);

    // Deallocates the memory blocks allocated from the end of the heap region.
    FreeTailForExpHeap(hExpHeap);

    NNS_FndDumpHeap(hExpHeap);

    // Destroy the expanded heap
    NNS_FndDestroyExpHeap(hExpHeap);
}


/*---------------------------------------------------------------------------*
  Name:         SampleFrameHeap

  Description:  The sample of the frame heap.

  Arguments:    heapAddress:  The starting address of the memory allocated to the heap.
                heapSize:     The size of the memory allocated to the heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SampleFrameHeap(
    void*   heapAddress,
    u32     heapSize)
{
    void* pExpMBlock;
    void* pMBlocks[8];
    NNSFndHeapHandle hExpHeap;
    NNSFndHeapHandle hFrmHeap;
    u32 frmHeapSize;

    /*
        First create the expanded heap and then create the frame heap inside it.
     */
        // Creation of the expanded heap
        hExpHeap = NNS_FndCreateExpHeap(heapAddress, heapSize);

        // Creation of a memory block that appropriates the entire allocatable size
        pExpMBlock = NNS_FndAllocFromExpHeap(
                        hExpHeap,
                        NNS_FndGetAllocatableSizeForExpHeap(hExpHeap));

        // Creation of the frame heap
        hFrmHeap = NNS_FndCreateFrmHeap(
                        pExpMBlock,
                        NNS_FndGetSizeForMBlockExpHeap(pExpMBlock));

    /*
        Conduct the memory block allocation, state save, and restoration
     */
        // Normal memory block allocation
        pMBlocks[0] = NNS_FndAllocFromFrmHeap(hFrmHeap, 100);

        // Allocates from the end of the heap region. The default alignment value is 4.
        pMBlocks[1] = NNS_FndAllocFromFrmHeapEx(hFrmHeap, 100, -NNS_FND_HEAP_DEFAULT_ALIGNMENT);

        // Save the memory block allocation state. The tag is 'FRST'.
        (void)NNS_FndRecordStateForFrmHeap(hFrmHeap, 'FRST');

        // Allocate from the beginning and end of the memory block. The alignment is 8.
        pMBlocks[2] = NNS_FndAllocFromFrmHeapEx(hFrmHeap, 200,  8);
        pMBlocks[3] = NNS_FndAllocFromFrmHeapEx(hFrmHeap, 200, -8);

        // Save the memory block allocation state. The tag is 'SCND'.
        (void)NNS_FndRecordStateForFrmHeap(hFrmHeap, 'SCND');

        // Allocate from the beginning and end of the memory block. The alignment is 16.
        pMBlocks[4] = NNS_FndAllocFromFrmHeapEx(hFrmHeap, 300,  16);
        pMBlocks[5] = NNS_FndAllocFromFrmHeapEx(hFrmHeap, 300, -16);

        // Save the memory block allocation state. The tag is 'THRD'
        (void)NNS_FndRecordStateForFrmHeap(hFrmHeap, 'THRD');

        // Allocate from the beginning and end of the memory block. The alignment is 32.
        pMBlocks[6] = NNS_FndAllocFromFrmHeapEx(hFrmHeap, 300,  32);
        pMBlocks[7] = NNS_FndAllocFromFrmHeapEx(hFrmHeap, 300, -32);

        NNS_FndDumpHeap(hFrmHeap);

        // Specify a tag and restore the state
        (void)NNS_FndFreeByStateToFrmHeap(hFrmHeap, 'SCND');

        NNS_FndDumpHeap(hFrmHeap);

        // Restore the state without specifying a tag (restore to the 'FRST' state)
        (void)NNS_FndFreeByStateToFrmHeap(hFrmHeap, 0);

        NNS_FndDumpHeap(hFrmHeap);

    /*
        Free the memory block allocated from the end of the heap region, then reduce the frame heap region.
     */
        // Free the memory block allocated from the end of the heap
        (void)NNS_FndFreeToFrmHeap(hFrmHeap, NNS_FND_FRMHEAP_FREE_TAIL);

        // Reduce the frame heap
        frmHeapSize = NNS_FndAdjustFrmHeap(hFrmHeap);

        // Reduce the memory block of the expanded heap along with the reduction of the frame heap
        (void)NNS_FndResizeForMBlockExpHeap(hExpHeap, pExpMBlock, frmHeapSize);

        // Get the heap that holds the memory block, and dump it.
        // (Frame heap: the contents of hFrmHeap are displayed)
        NNS_FndDumpHeap(NNS_FndFindContainHeap(pMBlocks[0]));

        // Dump the expanded heap
        NNS_FndDumpHeap(hExpHeap);

    /*
        Clean up
     */
        // Destroy the frame heap
        NNS_FndDestroyFrmHeap(hFrmHeap);

        // Destroy the expanded heap
        NNS_FndDestroyExpHeap(hExpHeap);
}


/*---------------------------------------------------------------------------*
  Name:         SampleUnitHeap

  Description:  A sample of the unit heap.

  Arguments:    heapAddress:  The starting address of the memory allocated to the heap.
                heapSize:     The size of the memory allocated to the heap.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
SampleUnitHeap(
    void*   heapAddress,
    u32     heapSize)
{
    void* pMBlocks[4];

    // Creation of the unit heap
    NNSFndHeapHandle hUnitHeap = NNS_FndCreateUnitHeapEx(
                                    heapAddress,
                                    heapSize,
                                    64,         // The size of the memory block
                                    16,         // The alignment
                                    0);         // Options

    // Display the number of allocatable memory blocks
    OS_Printf("[demo unit heap] free block num %d.\n", NNS_FndCountFreeBlockForUnitHeap(hUnitHeap));

    // Memory block allocation
    pMBlocks[0] = NNS_FndAllocFromUnitHeap(hUnitHeap);
    pMBlocks[1] = NNS_FndAllocFromUnitHeap(hUnitHeap);
    pMBlocks[2] = NNS_FndAllocFromUnitHeap(hUnitHeap);
    pMBlocks[3] = NNS_FndAllocFromUnitHeap(hUnitHeap);

    // Free the third memory block
    NNS_FndFreeToUnitHeap(hUnitHeap, pMBlocks[2]);

    // Display the number of allocatable memory blocks
    OS_Printf("[demo unit heap] free block num %d.\n", NNS_FndCountFreeBlockForUnitHeap(hUnitHeap));

    NNS_FndDumpHeap(hUnitHeap);

    // Destroy the unit heap
    NNS_FndDestroyUnitHeap(hUnitHeap);
}


/*---------------------------------------------------------------------------*
  Name:         NitroMain
 *---------------------------------------------------------------------------*/
void
NitroMain(void)
{
    OS_Init();

	SampleExpHeap(sHeapBuf, sizeof sHeapBuf);
	SampleFrameHeap(sHeapBuf, sizeof sHeapBuf);
	SampleUnitHeap(sHeapBuf, sizeof sHeapBuf);

    while(1) {}
}
