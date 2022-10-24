#include <nitro.h>


#define HEAP_ID    ((OSHeapHandle)0)
#define ARENA_ID   ((OSArenaId)OS_ARENA_MAIN)
#define ROUND(n, a)     (((u32) (n) + (a) - 1) & ~((a) - 1))

//----------------------------------------------------------------
//
void* operator new ( std::size_t blocksize )
{
    return OS_AllocFromHeap( ARENA_ID, HEAP_ID, blocksize );
}

//----------------------------------------------------------------
//
void* operator new[] ( std::size_t blocksize )
{
    return OS_AllocFromHeap( ARENA_ID, HEAP_ID, blocksize );
}

//----------------------------------------------------------------
//
void operator delete ( void* block ) throw()
{
    OS_FreeToHeap( ARENA_ID, HEAP_ID, block );
}

//----------------------------------------------------------------
//
void operator delete[] ( void* block ) throw()
{
    OS_FreeToHeap( ARENA_ID, HEAP_ID, block );
}

/*---------------------------------------------------------------------------*
  Name:         NitroStartUp

  Description:  startup befor NitroMain()

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroStartUp(void)
{
    void*    arenaLo;
    void*    arenaHi;

    OS_Init();

    arenaLo = OS_GetArenaLo( ARENA_ID );
    arenaHi = OS_GetArenaHi( ARENA_ID );

    // Create a heap
    arenaLo = OS_InitAlloc( ARENA_ID, arenaLo, arenaHi, 1 );
    OS_SetArenaLo( ARENA_ID, arenaLo );

    // Ensure boundaries are 32B aligned
    arenaLo = (void*)ROUND( arenaLo, 32 );
    arenaHi = (void*)ROUND( arenaHi, 32 );

    // The boundaries given to OSCreateHeap should be 32B aligned
    (void)OS_SetCurrentHeap( ARENA_ID, OS_CreateHeap( ARENA_ID, arenaLo, arenaHi ) );

    // From here on out, OS_Alloc and OS_Free behave like malloc and free respectively
    OS_SetArenaLo( ARENA_ID, arenaLo = arenaHi );
}
