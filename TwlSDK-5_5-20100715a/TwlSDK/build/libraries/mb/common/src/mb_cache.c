/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_cache.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
 
#include <nitro.h>

#include "mb_cache.h"


/*---------------------------------------------------------------------------*
  Name:         MBi_InitCache

  Description:  Initializes a cache list.
                

  Arguments:    pl:         Cache list.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_InitCache(MBiCacheList * pl)
{
    MI_CpuClear8(pl, sizeof(*pl));
}

/*---------------------------------------------------------------------------*
  Name:         MBi_AttachCacheBuffer

  Description:  Assigns a buffer to a cache list.
                
  Arguments:    pl:         Cache list.
                ptr:        Buffer to assign.
                src:        Source address of ptr.
                len:        Byte size of ptr.
                state:      Initial state to specify.
                           (MB_CACHE_STATE_READY or MB_CACHE_STATE_LOCKED)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_AttachCacheBuffer(MBiCacheList * pl, u32 src, u32 len, void *ptr, u32 state)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        /* Search for an unregistered page. */
        MBiCacheInfo *pi = pl->list;
        for (;; ++pi)
        {
            if (pi >= &pl->list[MB_CACHE_INFO_MAX])
            {
                OS_TPanic("MBi_AttachCacheBuffer() failed! (over maximum count)");
            }
            if (pi->state == MB_CACHE_STATE_EMPTY)
            {
                pi->src = src;
                pi->len = len;
                pi->ptr = (u8 *)ptr;
                pi->state = state;
                break;
            }
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_ReadFromCache

  Description:  Reads the content of the specified address from the cache.

  Arguments:    pl:         Cache list.
                src:        Source address of the read.
                len:        Read size (BYTE)
                dst:        Destination address of the read.

  Returns:      TRUE if the cache hits and a read is performed; FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL MBi_ReadFromCache(MBiCacheList * pl, u32 src, void *dst, u32 len)
{
    BOOL    ret = FALSE;
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        /* Only search for usable pages */
        const MBiCacheInfo *pi = pl->list;
        for (; pi < &pl->list[MB_CACHE_INFO_MAX]; ++pi)
        {
            if (pi->state >= MB_CACHE_STATE_READY)
            {
                /* Determine if the target address is within range. */
                const int ofs = (int)(src - pi->src);
                if ((ofs >= 0) && (ofs + len <= pi->len))
                {
                    /* Read because the cache was hit. */
                    MI_CpuCopy8(pi->ptr + ofs, dst, len);
                    pl->lifetime = 0;
                    ret = TRUE;
                    break;
                }
            }
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         MBi_TryLoadCache

  Description:  Loads the content of the specified address to the cache.
                The READY page cache with the lowest address is discarded.

  Arguments:    pl:         Cache list.
                src:        Reload source address.
                len:        Reload size (in bytes).

  Returns:      TRUE if the reload could be started and FALSE otherwise.
                (When there is a single reload processing engine in the system, this function should return FALSE if processing was not finished previously.)
                 
 *---------------------------------------------------------------------------*/
BOOL MBi_TryLoadCache(MBiCacheList * pl, u32 src, u32 len)
{
    BOOL    ret = FALSE;
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    {
        /* Only search for pages that can be reloaded. */
        MBiCacheInfo *trg = NULL;
        MBiCacheInfo *pi = pl->list;
        for (; pi < &pl->list[MB_CACHE_INFO_MAX]; ++pi)
        {
            if (pi->state == MB_CACHE_STATE_READY)
            {
                /* Leave it as candidate if it is the smallest address. */
                if (!trg || (trg->src > pi->src))
                {
                    trg = pi;
                }
            }
        }
        /* Request processing if the reload page target can be found. */
        if (trg)
        {
            /* The processing request to the task thread is described here. */
            (void)src;
            (void)len;
        }
        OS_TPanic("reload-system is not yet!");
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    return ret;
}
