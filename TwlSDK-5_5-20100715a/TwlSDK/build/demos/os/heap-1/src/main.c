/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - heap-1
  File:     main.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-03#$
  $Rev: 8857 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#include <nitro.h>

//----------------------------------------------------------------
#define   MAIN_HEAP_SIZE1   0x10000
#define   MAIN_HEAP_SIZE2   0x20000
#define   MAIN_HEAP_SIZE3   0x8000

#define   ALLOC_SIZE1       0x10
#define   ALLOC_SIZE2       0x100
#define   ALLOC_SIZE3       0x1000
#define   ALLOC_SIZE4       0x2000
#define   ALLOC_SIZE5       0x10

#define   HEAP_NUM          2
#define   DISPLAY_HEAP1     (1<<0)
#define   DISPLAY_HEAP2     (1<<1)

OSHeapHandle handle1;
OSHeapHandle handle2;

void DisplayHeap(u32 heap);

//================================================================================
//        Comprehensive heap sample
//        Create a heap for the main memory arena, and attempt region allocation/deallocation.
//         
/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
    void   *heapArea1;
    void   *heapArea2;
    void   *heapArea3;
    void   *nstart;
    void   *area1;
    void   *area2;
    void   *area3;
    void   *area4;

    OS_Init();
    //OS_InitArena(); /*Called in OS_Init()*/


    OS_Printf("================================ after OS_Init() ================\n", OS_GetMainArenaLo());
    OS_Printf("arena lo = %x\n", OS_GetMainArenaLo());
    OS_Printf("arena hi = %x\n", OS_GetMainArenaHi());

    //---- Initialize memory allocation system for MainRAM arena
    nstart = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), HEAP_NUM);
    OS_SetMainArenaLo(nstart);


    //---- Allocate a heap from the arena and create the heap
    OS_Printf("\n================================ create heap ================\n" );
    heapArea1 = OS_AllocFromMainArenaLo(MAIN_HEAP_SIZE1, 32);
    heapArea2 = OS_AllocFromMainArenaLo(MAIN_HEAP_SIZE2, 32);
    handle1 = OS_CreateHeap(OS_ARENA_MAIN, heapArea1, (void *)((u32)heapArea1 + MAIN_HEAP_SIZE1));
    handle2 = OS_CreateHeap(OS_ARENA_MAIN, heapArea2, (void *)((u32)heapArea2 + MAIN_HEAP_SIZE2));

    OS_Printf("heap1 handle=%d area1 = %x-%x\n", handle1, heapArea1, (u32)heapArea1 + MAIN_HEAP_SIZE1);
    OS_Printf("heap2 handle=%d area2 = %x-%x\n", handle2, heapArea2, (u32)heapArea2 + MAIN_HEAP_SIZE2);

    //---- Current heap settings
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN, handle1);

    DisplayHeap( DISPLAY_HEAP1 | DISPLAY_HEAP2 );

    //---- Allocate the areas
    OS_Printf("\n================================ allocate ================\n" );
    area1 = OS_Alloc(ALLOC_SIZE1);
    area2 = OS_Alloc(ALLOC_SIZE2);
    area3 = OS_AllocFromHeap(OS_ARENA_MAIN, handle1, ALLOC_SIZE3);
    area4 = OS_AllocFromHeap(OS_ARENA_MAIN, handle2, ALLOC_SIZE4);

    OS_Printf("area1: heap1  size=%5x  addr=%x\n", ALLOC_SIZE1, area1);
    OS_Printf("area2: heap1  size=%5x  addr=%x\n", ALLOC_SIZE2, area2);
    OS_Printf("area3: heap1  size=%5x  addr=%x\n", ALLOC_SIZE3, area3);
    OS_Printf("area4: heap2  size=%5x  addr=%x\n", ALLOC_SIZE4, area4);

    DisplayHeap( DISPLAY_HEAP1 | DISPLAY_HEAP2 );


    //---- Deallocate the areas
    OS_Printf("\n================================ Free ================\n" );
    OS_Free(area2);
    OS_Printf("free area2.\n");

    OS_FreeToHeap(OS_ARENA_MAIN, handle2, area4);
    OS_Printf("free area4.\n");

    DisplayHeap( DISPLAY_HEAP1 | DISPLAY_HEAP2 );


	//---- Expand the heap
    OS_Printf("\n================================ expand heap1 ================\n" );
    heapArea3 = OS_AllocFromMainArenaLo(MAIN_HEAP_SIZE3, 32);
    OS_Printf("expand area %x-%x\n", heapArea3, (u32)heapArea3 + MAIN_HEAP_SIZE3);

    OS_AddToHeap(OS_ARENA_MAIN, handle1, heapArea3, (void *)((u32)heapArea3 + MAIN_HEAP_SIZE3));

    DisplayHeap( DISPLAY_HEAP1 | DISPLAY_HEAP2 );


	//---- Destroy heap
    OS_Printf("\n================================ destroy heap2 ================\n" );
    OS_DestroyHeap(OS_ARENA_MAIN, handle2);


    OS_Printf("\n");
    OS_Printf("==== Finish sample.\n");
    OS_Terminate();
}

//----------------------------------------------------------------
//  DisplayHeap
//
void DisplayHeap( u32 heap )
{
	if ( heap & DISPLAY_HEAP1 )
	{
		OS_Printf( "+-------------------HEAP 1-------------------+\n" );
		OS_Printf( "|                                            |\n" );
		OS_DumpHeap(OS_ARENA_MAIN, handle1);
		OS_Printf( "|                                            |\n" );
		OS_Printf( "+--------------------------------------------+\n" );
	}

	if ( heap & DISPLAY_HEAP2 )
	{
		OS_Printf( "+-------------------HEAP 2-------------------+\n" );
		OS_Printf( "|                                            |\n" );
		OS_DumpHeap(OS_ARENA_MAIN, handle2);
		OS_Printf( "|                                            |\n" );
		OS_Printf( "+--------------------------------------------+\n" );
	}
}

/*====== End of main.c ======*/
