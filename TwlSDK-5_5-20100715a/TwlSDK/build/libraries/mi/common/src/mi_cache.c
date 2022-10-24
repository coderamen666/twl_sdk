/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI
  File:     mi_cache.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-07#$
  $Rev: 9260 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro.h>


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         MI_InitCache

  Description:  Initializes the memory cache.

  Arguments:    cache            The MICache structure to be initialized.
                page             The buffer size per page.
                                 Must be a power of two if four or more pages.
                buffer           The buffer used for page management information.
                length           The buffer size.
                                 Broken into page lists that have the number equal to (length / (sizeof(MICachePage) + page))
                                 
                                 It is guaranteed that the buffer leading address will be (buffer + N * page) for each page (N = 0, 1,...).
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MI_InitCache(MICache *cache, u32 page, void *buffer, u32 length)
{
    /* As a result of the current implementation, pages smaller than the word size are not supported */
    SDK_ASSERT(page >= sizeof(u32));
    /* Member initialization */
    cache->pagewidth = MATH_CTZ(page);
    cache->valid_total = 0;
    cache->invalid_total = 0;
    cache->loading_total = 0;
    cache->valid = NULL;
    cache->invalid = NULL;
    cache->loading = NULL;
    /* Page division */
    {
        u32             total = length / (sizeof(MICachePage) + page);
        u8             *buf = (u8*)buffer;
        MICachePage   *inf = (MICachePage*)&buf[total * page];
        cache->invalid_total += total;
        for (; total > 0; --total)
        {
            inf->offset = 0;
            inf->buffer = buf;
            inf->next = cache->invalid;
            cache->invalid = inf;
            inf += 1;
            buf += page;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_TouchCachePages

  Description:  Requests load preparations for the specified page range.
                If the invalid list is empty, destroys the active list from its end.

  Arguments:    cache            The MICache structure.
                head             The load target's leading page number.
                bitset           The load target's page's bit set.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_TouchCachePages(MICache *cache, u32 head, u32 bitset)
{
    {
        PLATFORM_ENTER_CRITICALSECTION();
        MICachePage  **load;
        /* Search the request list, and exempt and duplicates for the current material */
        for (load = &cache->loading; *load; load = &(*load)->next)
        {
            MICachePage *p = *load;
            u32     pos = p->offset - head;
            if ((pos < 32) && ((bitset & (1 << pos)) != 0))
            {
                bitset &= ~(1UL << pos);
            }
        }
        /* If the invalid list is insufficient, destroys the active list from its end */
        {
            int     rest = MATH_CountPopulation(bitset) - cache->invalid_total;
            if (rest > 0)
            {
                int             del = cache->valid_total;
                MICachePage  **valid;
                for (valid = &cache->valid; *valid; valid = &(*valid)->next)
                {
                    if (--del < rest)
                    {
                        MICachePage **pp;
                        for (pp = &cache->invalid; *pp; pp = &(*pp)->next)
                        {
                        }
                        *pp = *valid;
                        *valid = NULL;
                        cache->valid_total -= rest;
                        cache->invalid_total += rest;
                        break;
                    }
                }
            }
        }
        /* Move from the head of the invalid list to the end of the request list */
        while (cache->invalid && bitset)
        {
            MICachePage *p = cache->invalid;
            u32     pos = MATH_CTZ(bitset);
            cache->invalid = p->next;
            p->offset = head + pos;
            p->next = NULL;
            *load = p;
    	    load = &p->next;
            --cache->invalid_total;
            --cache->loading_total;
            bitset &= ~(1UL << pos);
        }
        PLATFORM_LEAVE_CRITICALSECTION();
    }
    /* If there are absolutely fewer pages than specified, the use method us invalid; issue only one warning */
    if (bitset)
    {
        static BOOL output_once = FALSE;
        if (!output_once)
        {
            output_once = TRUE;
            OS_TWarning("not enough cache-page! "
                        "MI_TouchCache() failed. "
                        "(total pages = %d, but need more %d)",
                        cache->invalid_total + cache->valid_total,
                        MATH_CountPopulation(bitset));
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         MI_ReadCache

  Description:  Reads data from the cache.
                Move the hit pages to the top of the active list

  Arguments:    cache            The MICache structure.
                buffer           The transfer target memory.
                                 When NULL is specified, only cache preparations are requested for the entire relevant range without loading any data.
                                 
                offset           The transfer source offset.
                length           The transfer size.

  Returns:      TRUE if the entire region is hit by the cache.
 *---------------------------------------------------------------------------*/
BOOL MI_ReadCache(MICache *cache, void *buffer, u32 offset, u32 length)
{
    BOOL    retval = TRUE;

    /* Determine from the front in 32-page units */
    const u32   unit = (1UL << cache->pagewidth);
    u32     head = (offset >> cache->pagewidth);
    u32     tail = ((offset + length + unit - 1UL) >> cache->pagewidth);
    u32     pages;
    for (; (pages = MATH_MIN(tail - head, 32UL)), (pages > 0); head += pages)
    {
        /* Search for the relevant pages from the active list */
        u32     bitset = (1UL << pages) - 1UL;
        {
            PLATFORM_ENTER_CRITICALSECTION();
            MICachePage **pp;
            for (pp = &cache->valid; *pp && bitset; pp = &(*pp)->next)
            {
                MICachePage *p = *pp;
                u32     pos = p->offset - head;
                if ((pos < pages) && ((bitset & (1UL << pos)) != 0))
                {
                    // Move the hit pages to the top of the active list
                    *pp = (*pp)->next;
                    p->next = cache->valid;
                    cache->valid = p;
                    pp = &cache->valid;
                    if (buffer)
                    {
                        u32     len = unit;
                        int     src = 0;
                        int     dst = (int)((p->offset << cache->pagewidth) - offset);
                        if (dst < 0)
                        {
                            len += dst;
                            src -= dst;
                            dst = 0;
                        }
                        if (dst + len > length)
                        {
                            len = length - dst;
                        }
                        MI_CpuCopy(&p->buffer[src], &((u8*)buffer)[dst], len);
                    }
                    bitset &= ~(1UL << pos);
                }
            }
            PLATFORM_LEAVE_CRITICALSECTION();
        }
        /* When there is a page fault */
        if (bitset)
        {
            retval = FALSE;
            WFSi_TouchCachePages(cache, head, bitset);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadCache

  Description:  Runs the load process for all pages that exist in the load request list.
                When the load request list is empty, nothing happens and a control is immediately returned; if an addition is made to the load request list during a call, that is processed too.
                

  Note:         This function must be called at a timing appropriate to a context in which device blocking is acceptable.
                
                Therefore, it must not be called from an interrupt handler, etc.

  Arguments:    cache            The MICache structure.
                device           The device to be loaded.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    MI_LoadCache(MICache *cache, MIDevice *device)
{
    for (;;)
    {
        /* Get the header of the load request list */
        MICachePage *p = cache->loading;
        if (!p)
        {
            break;
        }
        /* Read pages from the device */
        (void)MI_ReadDevice(device, p->buffer,
                            (p->offset << cache->pagewidth),
                            (1UL << cache->pagewidth));
        /* Insert read pages onto the head of the active list */
        {
            PLATFORM_ENTER_CRITICALSECTION();
            cache->loading = p->next;
            p->next = cache->valid;
            cache->valid = p;
            ++cache->valid_total;
            ++cache->loading_total;
            PLATFORM_LEAVE_CRITICALSECTION();
        }
    }
}


/*---------------------------------------------------------------------------*
  $Log: mi_cache.c,v $
  Revision 1.1  2007/04/11 08:06:25  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
