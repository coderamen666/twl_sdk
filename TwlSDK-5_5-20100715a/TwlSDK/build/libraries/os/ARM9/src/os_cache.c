/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_cache.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/code32.h>


/* ON when temporarily avoiding the Flush command */
#define SDK_AVOID_FLUSH


//===========================================================================
//              DATA CACHE CONTROL
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_Enable

  Description:  enable data cache

  Arguments:    None.

  Returns:      previous state
 *---------------------------------------------------------------------------*/
asm BOOL DC_Enable()
{
    mrc         p15, 0, r1, c1, c0, 0
    and         r0, r1, #HW_C1_DCACHE_ENABLE
    mov         r0, r0, LSR #HW_C1_DCACHE_ENABLE_SHIFT
    orr         r1, r1, #HW_C1_DCACHE_ENABLE
    mcr         p15, 0, r1, c1, c0, 0
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_Disable

  Description:  disable data cache

  Arguments:    None.

  Returns:      previous stats
 *---------------------------------------------------------------------------*/
asm BOOL DC_Disable()
{
    mrc         p15, 0, r1, c1, c0, 0
    and         r0, r1, #HW_C1_DCACHE_ENABLE
    mov         r0, r0, LSR #HW_C1_DCACHE_ENABLE_SHIFT
    bic         r1, r1, #HW_C1_DCACHE_ENABLE
    mcr         p15, 0, r1, c1, c0, 0
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_Restore

  Description:  set state of data cache

  Arguments:    data cache state to be set

  Returns:      previous state
 *---------------------------------------------------------------------------*/
asm BOOL DC_Restore( register BOOL enable )
{
    //---- Argument process
    cmp         r0, #0
    moveq       r2, #0
    movne       r2, #HW_C1_DCACHE_ENABLE

    mrc         p15, 0, r1, c1, c0, 0
    and         r0, r1, #HW_C1_DCACHE_ENABLE
    mov         r0, r0, LSR #HW_C1_DCACHE_ENABLE_SHIFT
    bic         r1, r1, #HW_C1_DCACHE_ENABLE
    orr         r1, r1, r2
    mcr         p15, 0, r1, c1, c0, 0
    bx          lr
}

//===========================================================================
//              DATA CACHE (for all range)
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_InvalidateAll

  Description:  invalidate all data cache

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_InvalidateAll( void )
{
    mov         r0, #0
    mcr         p15, 0, r0, c7, c6, 0
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_StoreAll

  Description:  clean all data cache
                (write cache data to memory)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_StoreAll( void )
{
    mov         r1, #0
@1:
    mov         r0, #0
@2:
    orr         r2, r1, r0
    mcr         p15, 0, r2, c7, c10, 2
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, #HW_DCACHE_SIZE/4
    blt         @2

    add         r1, r1, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r1, #0
    bne         @1

    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_FlushAll

  Description:  clean and invalidate all data cache
                (write cache data to memory, and invalidate cache)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_FlushAll( void )
{
    mov         r12, #0
    mov         r1, #0          // r1: Set No counter (0-3)

@1:
    mov         r0, #0          // r0: Line counter (0-DCACHE_SIZE/4)

@2:
    orr         r2, r1, r0
    mcr         p15, 0, r12, c7, c10, 4 /* wait write buffer empty */
    mcr         p15, 0, r2, c7, c14, 2  /* flush */
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, #HW_DCACHE_SIZE/4
    blt         @2

    add         r1, r1, #1<<HW_C7_CACHE_SET_NO_SHIFT
    cmp         r1, #0
    bne         @1

    bx          lr
}

//===========================================================================
//              DATA CACHE (for specified range)
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_InvalidateRange

  Description:  invalidate data cache in specified range

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_InvalidateRange( register void* startAddr, register u32 nBytes )
{
    add         r1, r1, r0
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1

@1:
    mcr         p15, 0, r0, c7, c6, 1
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @1
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_StoreRange

  Description:  clean data cache in specified range
                (write cache data to memory)

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_StoreRange( register const void* startAddr, register u32 nBytes )
{
    add         r1, r1, r0
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1

@1:
    mcr         p15, 0, r0, c7, c10, 1
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @1
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_FlushRange

  Description:  clean and invalidate data cache in specified range
                (write cache data to memory, and invalidate cache)

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_FlushRange( register const void* startAddr, register u32 nBytes )
{
    mov         r12, #0
    add         r1, r1, r0
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1
@1:
    mcr         p15, 0, r12, c7, c10, 4 /* wait write buffer empty */
    mcr         p15, 0, r0, c7, c14, 1 /* flush */
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @1

    bx          lr
}

//===========================================================================
//              DATA CACHE (for specified range)
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_TouchRange

  Description:  include specified area to data cache in advance

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_TouchRange( register const void* startAddr, register u32 nBytes )
{
    add         r1, r1, r0
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1

@1:
    pld         [r0]
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @1
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_LockdownRange

  Description:  lock specified area to prevent not to release data cache

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_LockdownRange( register const void* startAddr, register u32 nBytes )
{
    add         r1, r1, r0                              // r1: End address
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1
    mrc         p15, 0, r3, c9, c0, 0
    and         r3, r3, #HW_C9_LOCKDOWN_SET_NO_MASK     // r3: Current set number
    cmp         r3, #3                                  // Error if 3 sets are locked down
    mvneq       r0, #0
    bxeq        lr
    
    stmfd       sp!, { lr, r0, r1 }
    ldr         r0,  =OS_DisableInterrupts
    blx         r0
    mov         r2, r0
    ldmfd       sp!, { lr, r0, r1 }
    
    orr         r3, r3, #HW_C9_LOCKDOWN_LOAD_MODE       // Cache lockdown-Load Mode
    mcr         p15, 0, r3, c9, c0, 0
    
@111:
    mcr         p15, 0, r0, c7, c14, 1                  // Clean/disable data in cache once
    ldr         r12, [r0]                               // Read data and place in cache
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @111
    add         r3, r3, #1                              // Cache normal mode & set number increment
    bic         r0, r3, #HW_C9_LOCKDOWN_LOAD_MODE
    mcr         p15, 0, r3, c9, c0, 0
    
    stmfd       sp!, { lr }
    mov         r0, r2
    ldr         r1, =OS_RestoreInterrupts
    blx         r1
    ldmfd       sp!, { lr }
    
    bx          lr
}


/*---------------------------------------------------------------------------*
  Name:         DC_UnlockdownAll

  Description:  unlock all data cache to enable to release

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_UnlockdownAll( void )
{
    mov     r3, #0
    mcr     p15, 0, r3, c9, c0, 0
    bx      lr
}

/*---------------------------------------------------------------------------*
  Name:         DC_Unlockdown

  Description:  unlock any data cache to enable to release

  Arguments:    num  - specify number of datablock to unlock.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_Unlockdown( register u32 num )
{
    mrc     p15, 0, r3, c9, c0, 0
    and     r3, r3, #HW_C9_LOCKDOWN_SET_NO_MASK
    subs    r3, r3, r0
    movmi   r3, #0
    mcr     p15, 0, r3, c9, c0, 0
    bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         DC_WaitWriteBufferEmpty

  Description:  wait till write buffer becomes to be empty

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void DC_WaitWriteBufferEmpty()
{
    mov         r0, #0
    mcr         p15, 0, r0, c7, c10, 4
    bx          lr
}


//===========================================================================
//              INSTRUCTION CACHE CONTROL
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         IC_Enable

  Description:  enable instruction cache

  Arguments:    None.

  Returns:      previous state
 *---------------------------------------------------------------------------*/
asm BOOL IC_Enable()
{
    mrc         p15, 0, r1, c1, c0, 0
    and         r0, r1, #HW_C1_ICACHE_ENABLE
    mov         r0, r0, LSR #HW_C1_ICACHE_ENABLE_SHIFT
    orr         r1, r1, #HW_C1_ICACHE_ENABLE
    mcr         p15, 0, r1, c1, c0, 0
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         IC_Disable

  Description:  disable instruction cache

  Arguments:    None.

  Returns:      previous stats
 *---------------------------------------------------------------------------*/
asm BOOL IC_Disable()
{
   mrc          p15, 0, r1, c1, c0, 0
   and          r0, r1, #HW_C1_ICACHE_ENABLE
   mov          r0, r0, LSR #HW_C1_ICACHE_ENABLE_SHIFT
   bic          r1, r1, #HW_C1_ICACHE_ENABLE
   mcr          p15, 0, r1, c1, c0, 0
   bx           lr
}

/*---------------------------------------------------------------------------*
  Name:         IC_Restore

  Description:  set state of instruction cache

  Arguments:    instruction cache state to be set

  Returns:      previous stats
 *---------------------------------------------------------------------------*/
asm BOOL IC_Restore( register BOOL enable )
{
    //---- Argument process
    cmp         r0, #0
    moveq       r2, #0
    movne       r2, #HW_C1_ICACHE_ENABLE
    
    mrc         p15, 0, r1, c1, c0, 0
    and         r0, r1, #HW_C1_ICACHE_ENABLE
    mov         r0, r0, LSR #HW_C1_ICACHE_ENABLE_SHIFT
    bic         r1, r1, #HW_C1_ICACHE_ENABLE
    orr         r1, r1, r2
    mcr         p15, 0, r1, c1, c0, 0
    bx          lr
}

//===========================================================================
//              INSTRUCTION CACHE
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         IC_InvalidateAll

  Description:  invalidate all instruction cache

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_InvalidateAll( void )
{
    mov         r0, #0
    mcr         p15, 0, r0, c7, c5, 0
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         IC_InvalidateRange

  Description:  invalidate instruction cache in specified range

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_InvalidateRange( register void* startAddr, register u32 nBytes )
{
    add         r1, r1, r0
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1

@1:
    mcr         p15, 0, r0, c7, c5, 1
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @1
    bx          lr
}

/*---------------------------------------------------------------------------*
  Name:         IC_PrefetchRange

  Description:  include specified area to instruction cache in advance

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_PrefetchRange( register const void* startAddr, register u32 nBytes )
{
    add         r1, r1, r0
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1

@1:
    mcr         p15, 0, r0, c7, c13, 1
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @1
    bx          lr
}


#include    <nitro/itcm_begin.h>

/*---------------------------------------------------------------------------*
  Name:         IC_LockdownRange

  Description:  lock specified area to prevent not to release instruction cache

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_LockdownRange( register const void* startAddr, register u32 nBytes )
{
    add         r1, r1, r0                              // r1: End address
    bic         r0, r0, #HW_CACHE_LINE_SIZE - 1
    mrc         p15, 0, r3, c9, c0, 1
    and         r3, r3, #HW_C9_LOCKDOWN_SET_NO_MASK     // r3: Current set number
    cmp         r3, #3                                  // Error if 3 sets are locked down
    mvneq       r0, #0
    bxeq        lr

    stmfd       sp!, { lr, r0, r1 }
    ldr         r0,  =OS_DisableInterrupts
    blx         r0
    mov         r2, r0
    ldmfd       sp!, { lr, r0, r1 }

    orr         r3, r3, #HW_C9_LOCKDOWN_LOAD_MODE       // Cache lockdown-Load Mode
    mcr         p15, 0, r3, c9, c0, 1

@1:
    mcr         p15, 0, r0, c7, c5, 1                   // Disable from cache once
    mcr         p15, 0, r0, c7, c13, 1                  // Prefetch
    add         r0, r0, #HW_CACHE_LINE_SIZE
    cmp         r0, r1
    blt         @1
    add         r3, r3, #1                              // Cache normal mode & set number increment
    bic         r0, r3, #HW_C9_LOCKDOWN_LOAD_MODE
    mcr         p15, 0, r3, c9, c0, 1
    
    stmfd       sp!, { lr }
    mov         r0, r2
    ldr         r1, =OS_RestoreInterrupts
    blx         r1
    ldmfd       sp!, { lr }

    bx          lr
}

#include    <nitro/itcm_end.h>


/*---------------------------------------------------------------------------*
  Name:         IC_UnlockdownAll

  Description:  unlock all instruction cache to enable to release

  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_UnlockdownAll( void )
{
    mov     r3, #0
    mcr     p15, 0, r3, c9, c0, 1
    bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         IC_Unlockdown

  Description:  unlock any instruction cache to enable to release

  Arguments:    num  - specify number of datablock to unlock.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void IC_Unlockdown( register u32 num )
{
    mrc     p15, 0, r3, c9, c0, 1
    and     r3, r3, #HW_C9_LOCKDOWN_SET_NO_MASK
    subs    r3, r3, r0
    movmi   r3, #0
    mcr     p15, 0, r3, c9, c0, 1
    bx      lr
}


#include <nitro/codereset.h>
