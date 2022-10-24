 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_cache.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#if	!defined(NITRO_MB_CACHE_H_)
#define NITRO_MB_CACHE_H_


#if	defined(__cplusplus)
extern  "C"
{
#endif


/* constant ---------------------------------------------------------------- */


#define	MB_CACHE_INFO_MAX	4

#define MB_CACHE_STATE_EMPTY   0
#define MB_CACHE_STATE_BUSY    1
#define MB_CACHE_STATE_READY   2
#define MB_CACHE_STATE_LOCKED  3


/* struct ------------------------------------------------------------------ */


/*
 * Structure for virtual memory that uses paging.
 * Manage a cache for a large capacity address space (for CARD and so on) using an array of these structures.
 */
    typedef struct
    {
        u32     src;                   /* Logical source address */
        u32     len;                   /* Cache length */
        u8     *ptr;                   /* Pointer to cache buffer */
        u32     state;                 /* If 1, ready to use */
    }
    MBiCacheInfo;

    typedef struct
    {
        u32     lifetime;              /* If hit or timeout, set 0. */
        u32     recent;                /* Unused */
        MBiCacheInfo *p_list;          /* Unused */
        u32     size;                  /* Unused */
        char    arc_name[FS_ARCHIVE_NAME_LEN_MAX + 1];  /* Target archive */
        u32     arc_name_len;          /* Archive name length */
        FSArchive *arc_pointer;
        u8      reserved[32 - FS_ARCHIVE_NAME_LEN_MAX - 1 - sizeof(u32) - sizeof(FSArchive*)];
        MBiCacheInfo list[MB_CACHE_INFO_MAX];
    }
    MBiCacheList;


/* Function ---------------------------------------------------------------- */



/*---------------------------------------------------------------------------*
  Name:         MBi_InitCache

  Description:  Initializes a cache list.
                

  Arguments:    pl:         Cache list.

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    MBi_InitCache(MBiCacheList * pl);

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
    void    MBi_AttachCacheBuffer(MBiCacheList * pl, u32 src, u32 len, void *ptr, u32 state);

/*---------------------------------------------------------------------------*
  Name:         MBi_ReadFromCache

  Description:  Reads the content of the specified address from the cache.

  Arguments:    pl:         Cache list.
                src:        Source address of the read.
                len:        Read size (BYTE).
                dst:        Destination address of the read.

  Returns:      TRUE if the cache hits and a read is performed; FALSE otherwise.
 *---------------------------------------------------------------------------*/
    BOOL    MBi_ReadFromCache(MBiCacheList * pl, u32 src, void *dst, u32 len);

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
    BOOL    MBi_TryLoadCache(MBiCacheList * pl, u32 src, u32 len);



#if	defined(__cplusplus)
}
#endif


#endif                                 /* NITRO_MB_CACHE_H_ */
