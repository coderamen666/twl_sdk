/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include 
  File:     cache.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_CACHE_H_
#define NITRO_OS_CACHE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

//===========================================================================
//              DATA CACHE CONTROL
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_Enable

  Description:  enable data cache

  Arguments:    None

  Returns:      previous state
 *---------------------------------------------------------------------------*/
BOOL    DC_Enable();

/*---------------------------------------------------------------------------*
  Name:         DC_Disable

  Description:  disable data cache

  Arguments:    None

  Returns:      previous stats
 *---------------------------------------------------------------------------*/
BOOL    DC_Disable();

/*---------------------------------------------------------------------------*
  Name:         DC_Restore

  Description:  set state of data cache

  Arguments:    data cache state to be set

  Returns:      previous state
 *---------------------------------------------------------------------------*/
BOOL    DC_Restore(BOOL enable);


//===========================================================================
//              DATA CACHE (for all range)
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_InvalidateAll

  Description:  invalidate all data cache

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_InvalidateAll(void);

/*---------------------------------------------------------------------------*
  Name:         DC_StoreAll

  Description:  clean all data cache
                (write cache data to memory)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_StoreAll(void);

/*---------------------------------------------------------------------------*
  Name:         DC_FlushAll

  Description:  clean and invalidate all data cache
                (write cache data to memory, and invalidate cache)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_FlushAll(void);


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
void    DC_InvalidateRange(void *startAddr, u32 nBytes);

/*---------------------------------------------------------------------------*
  Name:         DC_StoreRange

  Description:  clean data cache in specified range
                (write cache data to memory)

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_StoreRange(const void *startAddr, u32 nBytes);

/*---------------------------------------------------------------------------*
  Name:         DC_FlushRange

  Description:  clean and invalidate data cache in specified range
                (write cache data to memory, and invalidate cache)

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_FlushRange(const void *startAddr, u32 nBytes);


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
void    DC_TouchRange(const void *startAddr, u32 nBytes);

/*---------------------------------------------------------------------------*
  Name:         DC_LockdownRange

  Description:  lock specified area to prevent not to release data cache

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_LockdownRange(const void *startAddr, u32 nBytes);

/*---------------------------------------------------------------------------*
  Name:         DC_UnlockdownAll

  Description:  unlock all data cache to enable to release

  Arguments:    none.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_UnlockdownAll(void);

/*---------------------------------------------------------------------------*
  Name:         DC_Unlockdown

  Description:  unlock any data cache to enable to release

  Arguments:    num  - specify number of datablock to unlock.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_Unlockdown(u32 num);

/*---------------------------------------------------------------------------*
  Name:         DC_WaitWriteBufferEmpty

  Description:  wait till write buffer becomes to be empty

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_WaitWriteBufferEmpty(void);


//===========================================================================
//              ALIAS OF DC function
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_CleanAll

  Description:  alias for DC_StoreAll

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DC_CleanAll(void)
{
    DC_StoreAll();
}

/*---------------------------------------------------------------------------*
  Name:         DC_CleanAndInvalidateAll

  Description:  alias for DC_FlushAll

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DC_CleanAndInvalidateAll(void)
{
    DC_FlushAll();
}

/*---------------------------------------------------------------------------*
  Name:         DC_CleanRange

  Description:  alias for DC_StoreRange

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DC_CleanRange(const void *startAddr, u32 nBytes)
{
    DC_StoreRange(startAddr, nBytes);
}

/*---------------------------------------------------------------------------*
  Name:         DC_CleanAndInvalidateRange

  Description:  alias for DC_FlushRange

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void DC_CleanAndInvalidateRange(const void *startAddr, u32 nBytes)
{
    DC_FlushRange(startAddr, nBytes);
}

//===========================================================================
//              INSTRUCTION CACHE CONTROL
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         IC_Enable

  Description:  enable instruction cache

  Arguments:    None

  Returns:      previous state
 *---------------------------------------------------------------------------*/
BOOL    IC_Enable();

/*---------------------------------------------------------------------------*
  Name:         IC_Disable

  Description:  disable instruction cache

  Arguments:    None

  Returns:      previous stats
 *---------------------------------------------------------------------------*/
BOOL    IC_Disable();

/*---------------------------------------------------------------------------*
  Name:         IC_Restore

  Description:  set state of instruction cache

  Arguments:    instruction cache state to be set

  Returns:      previous stats
 *---------------------------------------------------------------------------*/
BOOL    IC_Restore(BOOL enable);


//===========================================================================
//              INSTRUCTION CACHE
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         IC_InvalidateAll

  Description:  invalidate all instruction cache

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_InvalidateAll(void);

/*---------------------------------------------------------------------------*
  Name:         IC_InvalidateRange

  Description:  invalidate instruction cache in specified range

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_InvalidateRange(void *startAddr, u32 nBytes);

/*---------------------------------------------------------------------------*
  Name:         IC_PrefetchRange

  Description:  include specified area to instruction cache in advance

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_PrefetchRange(const void *startAddr, u32 nBytes);

/*---------------------------------------------------------------------------*
  Name:         IC_LockdownRange

  Description:  lock specified area to prevent not to release instruction cache

  Arguments:    startAddr     start address
                nBytes        size (in byte)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_LockdownRange(const void *startAddr, u32 nBytes);

/*---------------------------------------------------------------------------*
  Name:         IC_Unlockdown

  Description:  unlock any instruction cache to enable to release

  Arguments:    num  - specify number of datablock to unlock.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_Unlockdown(u32 num);

/*---------------------------------------------------------------------------*
  Name:         IC_UnlockdownAll

  Description:  unlock all instruction cache to enable to release

  Arguments:    None.
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_UnlockdownAll(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_CACHE_H_ */
#endif
