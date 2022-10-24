/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_hash.c

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_rom.h"


// Use MI_CpuCopy, which uses a larger size, only in LIMITED mode to prevent it from being linked in a NITRO environment.
#if defined(SDK_TWLLTD)
#define CARDi_CopyMemory MI_CpuCopy
#else
#define CARDi_CopyMemory MI_CpuCopy8
#endif


// Enable this flag when making HMAC-SHA1 calculation high-speed
#if defined(SDK_TWL)
#define CARD_SUPPORT_SHA1OPTIMIZE
#endif

#include <twl/ltdmain_begin.h>

/*---------------------------------------------------------------------------*/
/* Constants */

// Constants determined to be appropriate by the Card library at present.
// Proper changes are acceptable depending on the result of the performance calculation.
static const u32    CARD_ROM_HASH_BLOCK_MAX = 4;
static const u32    CARD_ROM_HASH_SECTOR_MAX = 32;

#if defined(CARD_SUPPORT_SHA1OPTIMIZE)
static SVCSHA1Context   sha1_ipad_def;
static SVCSHA1Context   sha1_opad_def;
#endif

static u8 CARDiHmacKey[] ATTRIBUTE_ALIGN(4) =
{
    0x21, 0x06, 0xc0, 0xde, 0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d, 0x46, 0xf2, 0xed, 0x01,
    0x76, 0xe3, 0xcc, 0x08, 0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf, 0x9a, 0x62, 0x78, 0x34,
    0x8f, 0x6d, 0x63, 0x3c, 0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23, 0xd2, 0xcf, 0xae, 0xc2,
    0x32, 0x67, 0x8d, 0xfe, 0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37, 0x87, 0x46, 0x58, 0x24,
};

/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_CalcRomHashBufferLength

  Description:  Gets the buffer size required in ROM hashing context.

  Arguments:    header: ROM header buffer that held the ROM information necessary for management

  Returns:      The buffer size required for the ROM hash context.
 *---------------------------------------------------------------------------*/
u32 CARD_CalcRomHashBufferLength(const CARDRomHeaderTWL *header)
{
    u32     length = 0;
    length += MATH_ROUNDUP(header->digest_tabel2.length, 32);
    length += MATH_ROUNDUP(header->digest_table1_size * CARD_ROM_HASH_SECTOR_MAX, 32);
    length += MATH_ROUNDUP(MATH_ROUNDUP(CARD_ROM_HASH_SIZE * header->digest_table2_sectors, CARD_ROM_PAGE_SIZE * 3) * CARD_ROM_HASH_BLOCK_MAX, 32);
    length += MATH_ROUNDUP(sizeof(CARDRomHashSector) * CARD_ROM_HASH_SECTOR_MAX, 32);
    length += MATH_ROUNDUP(sizeof(CARDRomHashBlock) * CARD_ROM_HASH_BLOCK_MAX, 32);
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_CompareHash

  Description:  Validity check using hashing (HMAC-SHA1)

  Arguments:    hash: Hash value to be used as the standard for comparison
                src: Image to be checked
                len: Size of the subject

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_CompareHash(const void *hash, void *buffer, u32 length)
{
#if defined(CARD_SUPPORT_SHA1OPTIMIZE)
    // Copy the entire structure of the ipad/opad that was calculated in advance
    u8      tmphash[CARD_ROM_HASH_SIZE];
    const u32  *h1 = (const u32 *)hash;
    const u32  *h2 = (const u32 *)tmphash;
    u32         dif = 0;
    SVCSHA1Context  tmpcxt;
    CARDi_CopyMemory(&sha1_ipad_def, &tmpcxt, sizeof(tmpcxt));
    SVC_SHA1Update(&tmpcxt, buffer, length);
    SVC_SHA1GetHash(&tmpcxt, tmphash);
    CARDi_CopyMemory(&sha1_opad_def, &tmpcxt, sizeof(tmpcxt));
    SVC_SHA1Update(&tmpcxt, tmphash, sizeof(tmphash));
    SVC_SHA1GetHash(&tmpcxt, tmphash);
    dif |= (h1[0] ^ h2[0]);
    dif |= (h1[1] ^ h2[1]);
    dif |= (h1[2] ^ h2[2]);
    dif |= (h1[3] ^ h2[3]);
    dif |= (h1[4] ^ h2[4]);
    if (dif != 0)
#else
    u8      tmphash[CARD_ROM_HASH_SIZE];
#ifdef SDK_TWL
    if (OS_IsRunOnTwl())
    {
        SVC_CalcHMACSHA1(tmphash, buffer, length, CARDiHmacKey, sizeof(CARDiHmacKey));
    }
    else
#endif
    {
        MATH_CalcHMACSHA1(tmphash, buffer, length, CARDiHmacKey, sizeof(CARDiHmacKey));
    }
    if (MI_CpuComp8(hash, tmphash, sizeof(tmphash)) != 0)
#endif
    {
        // If the card is removed and shutdown processing is delayed, only output a warning and clear data here without forcibly stopping program execution
        // 
        if ((OS_GetBootType() == OS_BOOTTYPE_ROM) && CARD_IsPulledOut())
        {
            OS_TWarning("ROM-hash comparation error. (CARD removal)\n");
            MI_CpuFill(buffer, 0, length);
        }
        else
        {
            OS_TPanic("ROM-hash comparation error!\n");
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetHashSectorIndex

  Description:  Gets sector number where the specified ROM offset belongs.

  Arguments:    context: CARDRomHashContext structure
                offset: ROM offset

  Returns:      Sector number where the specified ROM offset belongs.
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 CARDi_GetHashSectorIndex(const CARDRomHashContext *context, u32 offset)
{
    offset -= context->area_ntr.offset;
    if (offset >= context->area_ntr.length)
    {
        offset += (context->area_ntr.offset - context->area_ltd.offset);
        if (offset < context->area_ltd.length)
        {
            offset += context->area_ntr.length;
        }
        else
        {
            OS_TPanic("specified ROM address is outof-range.(unsafe without secure hash)\n");
        }
    }
    return offset / context->bytes_per_sector;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_StartLoading

  Description:  Starts transfers for only one sector waiting for media load.

  Arguments:    context: CARDRomHashContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_StartLoading(CARDRomHashContext *context)
{
    // Start the asynchronous process here if not already running
    void   *buffer = NULL;
    u32     offset = 0;
    u32     length = 0;
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    if (context->recent_load == NULL)
    {
        if (context->loading_block)
        {
            // A larger buffer has been allocated than actually needed so that asynchronous DMA transfer is possible.
            // Align offset of the reference target to the table position that is actually required.
            CARDRomHashBlock   *block = context->loading_block;
            u32                 pos = block->offset;
            u32                 mod = (pos & (CARD_ROM_PAGE_SIZE - 1UL));
            block->hash = &block->hash_aligned[mod];
            context->recent_load = block;
            buffer = block->hash_aligned;
            offset = pos - mod;
            length = MATH_ROUNDUP(mod + CARD_ROM_HASH_SIZE * context->sectors_per_block, CARD_ROM_PAGE_SIZE);
        }
        else if (context->loading_sector)
        {
            CARDRomHashSector  *sector = context->loading_sector;
            context->recent_load = sector;
            buffer = sector->image;
            offset = sector->offset;
            length = context->bytes_per_sector;
        }
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    // Perform synchronous transfer here if the asynchronous transfer is not supported or currently unusable
    if (buffer != NULL)
    {
        if ((*context->ReadAsync)(context->userdata, buffer, offset, length) == 0)
        {
            (void)(*context->ReadSync)(context->userdata, buffer, offset, length);
            CARD_NotifyRomHashReadAsync(context);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_NotifyRomHashReadAsync

  Description:  Notifies completion of the device asynchronous read.

  Arguments:    context: CARDRomHashContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_NotifyRomHashReadAsync(CARDRomHashContext *context)
{
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    // Move list from "waiting to load" to "waiting to verify"
    if (context->recent_load == context->loading_sector)
    {
        CARDRomHashSector  *sector = context->loading_sector;
        context->loading_sector = sector->next;
        sector->next = context->loaded_sector;
        context->loaded_sector = sector;
    }
    else
    {
        CARDRomHashBlock   *block = context->loading_block;
        context->loading_block = block->next;
        block->next = context->loaded_block;
        context->loaded_block = block;
    }
    context->recent_load = NULL;
    (void)OS_RestoreInterrupts(bak_cpsr);
    // Notify wait thread that "waiting to verify" list was added
    OS_WakeupThreadDirect(context->loader);
    // Start transfer of next "waiting to load" list, if there is one
    CARDi_StartLoading(context);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_TouchRomHashBlock

  Description:  Loads hash block data that includes the specified offset.
                Does nothing if already loaded.

  Arguments:    context: CARDRomHashContext structure
                sector: Sector to access

  Returns:      Corresponding hash block data.
 *---------------------------------------------------------------------------*/
static CARDRomHashBlock* CARDi_TouchRomHashBlock(CARDRomHashContext *context, u32 sector)
{
    // Note that the block region is handled by linking the NTR and LTD regions.
    // (Do not insert any extra padding or alignment.)
    u32     index = sector / context->sectors_per_block;
    // Determine whether the corresponding block data is already loaded
    CARDRomHashBlock  **pp = &context->valid_block;
    CARDRomHashBlock   *block = NULL;
    for (;; pp = &(*pp)->next)
    {
        block = *pp;
        if (block->index == index)
        {
            // Move the touched block data to the top of the list
            *pp = block->next;
            block->next = context->valid_block;
            context->valid_block = block;
            break;
        }
        // If not loaded, destroy the oldest block data and reload
        else if (block->next == NULL)
        {
            // Move to end of "waiting to load" list
            OSIntrMode  bak_cpsr = OS_DisableInterrupts();
            CARDRomHashBlock  **tail = pp;
            CARDRomHashBlock   *loading = block;
            // Do nothing if the same block is already waiting to load or waiting to verify
            for (pp = &context->loaded_block; *pp; pp = &(*pp)->next)
            {
                if ((*pp)->index == index)
                {
                    block = (*pp);
                    loading = NULL;
                    break;
                }
            }
            if (loading)
            {
                for (pp = &context->loading_block; *pp; pp = &(*pp)->next)
                {
                    if ((*pp)->index == index)
                    {
                        block = (*pp);
                        loading = NULL;
                        break;
                    }
                }
                if (loading)
                {
                    *tail = NULL;
                    *pp = loading;
                    loading->index = index;
                    loading->offset = context->sector_hash.offset + index * (CARD_ROM_HASH_SIZE * context->sectors_per_block);
                }
            }
            (void)OS_RestoreInterrupts(bak_cpsr);
            if (loading)
            {
                // Check the start timing of the asynchronous load
                CARDi_StartLoading(context);
            }
            break;
        }
    }
    return block;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_TouchRomHashSector

  Description:  Loads the specified sector on the block.
                Does nothing if already loaded.

  Arguments:    context: CARDRomHashContext structure
                block: CARDRomHashBlock structure
                offset: ROM offset to access

  Returns:      Corresponding sector image or NULL.
 *---------------------------------------------------------------------------*/
static void* CARDi_TouchRomHashSector(CARDRomHashContext *context, u32 offset)
{
    void               *image = NULL;
    CARDRomHashSector **pp = &context->valid_sector;
    CARDRomHashSector  *sector = NULL;
    u32                 index = CARDi_GetHashSectorIndex(context, offset);
    for (pp = &context->valid_sector; ; pp = &(*pp)->next)
    {
        // If the corresponding sector data is already loaded, returns that image
        if ((*pp)->index == index)
        {
            // Move the touched sector data to the top of the list
            sector = *pp;
            *pp = (*pp)->next;
            sector->next = context->valid_sector;
            context->valid_sector = sector;
            image = sector->image;
            break;
        }
        // If not loaded, destroy the oldest sector data and reload
        else if ((*pp)->next == NULL)
        {
            // Move to end of "waiting to load" list
            OSIntrMode  bak_cpsr = OS_DisableInterrupts();
            CARDRomHashSector  *loading = *pp;
            *pp = NULL;
            for (pp = &context->loading_sector; *pp; pp = &(*pp)->next)
            {
            }
            *pp = loading;
            loading->index = index;
            loading->offset = MATH_ROUNDDOWN(offset, context->bytes_per_sector);
            (void)OS_RestoreInterrupts(bak_cpsr);
            // Search for the block that will be required for validity test
            (void)CARDi_TouchRomHashBlock(context, index);
            // Check the start timing of the asynchronous load
            CARDi_StartLoading(context);
            break;
        }
    }
    return image;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_InitRomHashContext

  Description:  Initializes the ROM hashing context.

  Arguments:    context: CARDRomHashContext structure that must be initialized
                header: ROM header buffer that held the ROM information necessary for management
                           (No need to keep this because this is referenced only within the function)
                buffer: Buffer for management information used in the context
                length: Size of management information buffer
                           (Can be calculated using the CARD_CalcRomHashBufferLength function)
                sync: Device synchronous read callback function
                async: Device asynchronous read callback function (if not supported, NULL)
                userdata: Arbitrary user-defined function passed to the device read callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_InitRomHashContext(CARDRomHashContext *context, const CARDRomHeaderTWL *header,
                             void *buffer, u32 length,
                             MIDeviceReadFunction sync, MIDeviceReadFunction async,
                             void *userdata)
{
    // Constant differs by the build
    const u32   bytes_per_sector = header->digest_table1_size;
    const u32   sectors_per_block = header->digest_table2_sectors;
    const u32   master_hash_size = header->digest_tabel2.length;

    // Calculate whether the required variable length memory has been allocated
    u8     *lo = (u8 *)MATH_ROUNDUP((u32)buffer, 32);
    u8     *hi = (u8 *)MATH_ROUNDDOWN((u32)&lo[length], 32);
    u8     *cur = lo;
    context->master_hash = (u8 *)cur;
    cur += MATH_ROUNDUP(master_hash_size, 32);
    context->images = (u8 *)cur;
    cur += MATH_ROUNDUP(bytes_per_sector * CARD_ROM_HASH_SECTOR_MAX, 32);
    context->hashes = (u8 *)cur;
    cur += MATH_ROUNDUP(MATH_ROUNDUP(CARD_ROM_HASH_SIZE * sectors_per_block, CARD_ROM_PAGE_SIZE * 3) * CARD_ROM_HASH_BLOCK_MAX, 32);
    context->sectors = (CARDRomHashSector *)cur;
    cur += MATH_ROUNDUP(sizeof(*context->sectors) * CARD_ROM_HASH_SECTOR_MAX, 32);
    context->blocks = (CARDRomHashBlock *)cur;
    cur += MATH_ROUNDUP(sizeof(*context->blocks) * CARD_ROM_HASH_BLOCK_MAX, 32);
    if (cur > hi)
    {
        OS_TPanic("cannot allocate memory for ROM-hash from ARENA");
    }
    else
    {
        // Initialize the basic settings if the memory can be allocated
        MI_CpuClear32(lo, (u32)(cur - lo));
        context->area_ntr = header->digest_area_ntr;
        context->area_ltd = header->digest_area_ltd;
        context->sector_hash = header->digest_tabel1;
        context->block_hash = header->digest_tabel2;
        context->bytes_per_sector = bytes_per_sector;
        context->sectors_per_block = sectors_per_block;
        context->block_max = CARD_ROM_HASH_BLOCK_MAX;
        context->sector_max = CARD_ROM_HASH_SECTOR_MAX;
        // Initialize the device reader
        context->ReadSync = sync;
        context->ReadAsync = async;
        context->userdata = userdata;
#if defined(CARD_SUPPORT_SHA1OPTIMIZE)
        // Calculates the ipad/opad for hash calculation in advance
        {
            u8      ipad[MATH_HASH_BLOCK_SIZE];
            u8      opad[MATH_HASH_BLOCK_SIZE];
            int     i;
            for (i = 0; i < MATH_HASH_BLOCK_SIZE; ++i)
            {
                ipad[i] = (u8)(CARDiHmacKey[i] ^ 0x36);
                opad[i] = (u8)(CARDiHmacKey[i] ^ 0x5c);
            }
            SVC_SHA1Init(&sha1_ipad_def);
            SVC_SHA1Init(&sha1_opad_def);
            SVC_SHA1Update(&sha1_ipad_def, ipad, sizeof(ipad));
            SVC_SHA1Update(&sha1_opad_def, opad, sizeof(opad));
        }
#endif
        // Determine validity by loading the master hash
        (void)(*context->ReadSync)(context->userdata, context->master_hash, context->block_hash.offset, context->block_hash.length); 
        CARDi_CompareHash(header->digest_tabel2_digest, context->master_hash, context->block_hash.length);
        // Initialize the sector data list
        {
            CARDRomHashSector  *sectors = context->sectors;
            int     sector_index = 0;
            int     i;
            for (i = 0; i < context->sector_max; ++i)
            {
                sectors[i].next = &sectors[i - 1];
                sectors[i].image = &context->images[i * context->bytes_per_sector];
                sectors[i].index = 0xFFFFFFFF;
            }
            context->sectors[0].next = NULL;
            context->valid_sector = &context->sectors[context->sector_max - 1];
            context->loading_sector = NULL;
            context->loaded_sector = NULL;
        }
        // Initialize the block data list
        {
            CARDRomHashBlock   *blocks = context->blocks;
            const u32           unit = MATH_ROUNDUP(CARD_ROM_HASH_SIZE * sectors_per_block, CARD_ROM_PAGE_SIZE * 3);
            int     i;
            for (i = 0; i < context->block_max; ++i)
            {
                blocks[i].next = &blocks[i + 1];
                blocks[i].index = 0xFFFFFFFF;
                blocks[i].hash_aligned = &context->hashes[i * unit];
            }
            context->valid_block = &context->blocks[0];
            context->blocks[context->block_max - 1].next = NULL;
            context->loading_block = NULL;
            context->loaded_block = NULL;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomHashImageDirect

  Description:  Directly copies to the transfer destination without caching to the hash context.

  Arguments:    context: CARDRomHashContext structure
                buffer: Buffer to transfer to (must be 4-byte aligned)
                offset: ROM offset to access
                length: Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_ReadRomHashImageDirect(CARDRomHashContext *context, void *buffer, u32 offset, u32 length)
{
    const u32   sectunit = context->bytes_per_sector;
    const u32   blckunit = context->sectors_per_block;
    u32         position = offset;
    u32         end = length + offset;
    u32         sector = CARDi_GetHashSectorIndex(context, position);
    while (position < end)
    {
        // Synchronously load images of minimum units that can be obtained this time
        u32     available = (u32)(*context->ReadSync)(context->userdata, buffer, position, end - position);
        // Access blocks in advance that may be needed in verification this time
        (void)CARDi_TouchRomHashBlock(context, sector);
        // Request load preparation for the portion that will be required next
        if (context->ReadAsync && (position + available < end))
        {
            (void)(*context->ReadAsync)(context->userdata, NULL, position + available, end - (position + available));
        }
        // Test validity of obtained image
        while (available >= sectunit)
        {
            CARDRomHashBlock   *block = CARDi_TouchRomHashBlock(context, sector);
            u32                 slot = sector - block->index * blckunit;
            while ((slot < blckunit) && (available >= sectunit))
            {
                // Verify the hash table in block units here if required
                if (block != context->valid_block)
                {
                    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
                    while (context->loading_block)
                    {
                        OS_SleepThread(NULL);
                    }
                    if (block == context->loaded_block)
                    {
                        context->loaded_block = block->next;
                    }
                    (void)OS_RestoreInterrupts(bak_cpsr);
                    CARDi_CompareHash(&context->master_hash[block->index * CARD_ROM_HASH_SIZE],
                                      block->hash, CARD_ROM_HASH_SIZE * blckunit);
                    block->next = context->valid_block;
                    context->valid_block = block;
                }
                // Calculate the image hash
                CARDi_CompareHash(&block->hash[slot * CARD_ROM_HASH_SIZE], buffer, sectunit);
                position += sectunit;
                available -= sectunit;
                buffer = ((u8 *)buffer) + sectunit;
                slot += 1;
                sector += 1;
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomHashImageCaching

  Description:  Directly copies to the transfer destination while caching to the hash context.

  Arguments:    context: CARDRomHashContext structure
                buffer: Buffer to transfer to
                offset: ROM offset to access
                length: Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_ReadRomHashImageCaching(CARDRomHashContext *context, void *buffer, u32 offset, u32 length)
{
    while (length > 0)
    {
        // Determine whether the next sector has undergone validity test
        void   *image = CARDi_TouchRomHashSector(context, offset);
        if (image)
        {
            // If verified, copy memory as is and go to next
            u32     max = context->bytes_per_sector;
            u32     mod = offset - MATH_ROUNDDOWN(offset, max);
            u32     len = (u32)MATH_MIN(length, (max - mod));
            CARDi_CopyMemory((u8*)image + mod, buffer, len);
            buffer = (u8 *)buffer + len;
            length -= len;
            offset += len;
        }
        else
        {
            // If no hit was found, request to load the sector in advance as much as the capacity allows so that all unverified lists can be processed together.
            // Related block hashes are also automatically read in advance, and the asynchronous transfer has already started if this is possible.
            // 
            // 
            u32     hit = MATH_ROUNDDOWN(offset, context->bytes_per_sector);
            while (context->valid_sector && context->valid_block)
            {
                hit += context->bytes_per_sector;
                if (hit >= offset + length)
                {
                    break;
                }
                (void)CARDi_TouchRomHashSector(context, hit);
            }
            for (;;)
            {
                // Idle until a verification wait list of a load-complete state is generated
                OSIntrMode          bak_cpsr = OS_DisableInterrupts();
                CARDRomHashBlock   *block = NULL;
                CARDRomHashSector  *sector = NULL;
                while ((context->loading_sector && !context->loaded_sector) ||
                        (context->loading_block && !context->loaded_block))
                {
                    OS_SleepThread(NULL);
                }
                // Process the block before the sector considering dependency relationships
                block = context->loaded_block;
                if (block)
                {
                    context->loaded_block = block->next;
                }
                else
                {
                    sector = context->loaded_sector;
                    if (sector)
                    {
                        context->loaded_sector = sector->next;
                    }
                }
                (void)OS_RestoreInterrupts(bak_cpsr);
                // Compare with the higher table and verify by calculating block or sector hash
                if (block)
                {
                    u32     len = CARD_ROM_HASH_SIZE * context->sectors_per_block;
                    CARDi_CompareHash(&context->master_hash[block->index * CARD_ROM_HASH_SIZE], block->hash, len);
                    // Move the touched block data to the top of the list
                    block->next = context->valid_block;
                    context->valid_block = block;
                }
                else if (sector)
                {
                    CARDRomHashBlock   *block = CARDi_TouchRomHashBlock(context, sector->index);
                    u32                 slot = sector->index - block->index * context->sectors_per_block;
                    CARDi_CompareHash(&block->hash[slot * CARD_ROM_HASH_SIZE], sector->image, context->bytes_per_sector);
                    // Move the touched sector data to the top of the list
                    sector->next = context->valid_sector;
                    context->valid_sector = sector;
                }
                // If there is nothing that must be asynchronously verified, end this loop
                else
                {
                    break;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRomHashImage

  Description:  Reads the hash-verified ROM image from the specified offset

  Arguments:    context: CARDRomHashContext structure
                offset: ROM offset to access

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_ReadRomHashImage(CARDRomHashContext *context, void *buffer, u32 offset, u32 length)
{
    // Initialize notification destination from the DMA
    context->loader = OS_GetCurrentThread();
    context->recent_load = NULL;
    // When conducting a large-scale read where at least half of the sector cache could be lost at once, employ a direct transfer mode to speed up processing and protect the cache
    // 
    if ((length >= context->bytes_per_sector * (CARD_ROM_HASH_SECTOR_MAX / 2))
        // TODO:
        //   For the time being, do not use the direct transfer mode with a card boot.
        //   (Particular attention to the word boundary alignment of the transfer source and destination is not necessary. However, performance may actually be worse for synchronous ROM transfers, so conditions should be added when necessary.)
        //    
        //    
        && (OS_GetBootType() != OS_BOOTTYPE_ROM))
    {
        // Split the leading end and trailing end portions without aligning the sector boundaries
        const u32   sectmask = (u32)(context->bytes_per_sector - 1UL);
        const u32   headlen = (u32)((context->bytes_per_sector - offset) & sectmask);
        const u32   bodylen = (u32)((length - headlen) & ~sectmask);
        // Leading End Portion
        if (headlen > 0)
        {
            CARDi_ReadRomHashImageCaching(context, buffer, offset, headlen);
            offset += headlen;
            length -= headlen;
            buffer = ((u8 *)buffer) + headlen;
        }
        // Middle Portion
        if (bodylen > 0)
        {
            CARDi_ReadRomHashImageDirect(context, buffer, offset, bodylen);
            offset += bodylen;
            length -= bodylen;
            buffer = ((u8 *)buffer) + bodylen;
        }
    }
    // Process the remainder that could not be used with the direct transfer mode
    CARDi_ReadRomHashImageCaching(context, buffer, offset, length);
}
#include <twl/ltdmain_end.h>
