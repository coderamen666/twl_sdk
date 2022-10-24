/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     hash.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_CARD_HASH_H_
#define NITRO_CARD_HASH_H_


#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/card/types.h>
#include <nitro/mi/device.h>


#ifdef __cplusplus
extern "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

#define CARD_ROM_HASH_SIZE      20


/*---------------------------------------------------------------------------*/
/* Declarations */

struct CARDRomHashSector;
struct CARDRomHashBlock;
struct CARDRomHashContext;

// This structure manages the image cache for various sectors.
typedef struct CARDRomHashSector
{
    struct CARDRomHashSector   *next;
    u32                         index;
    u32                         offset;
    void                       *image;
}
CARDRomHashSector;

// This block structure manages the hash and image cache for various sectors.
// If the final block size of the NTR region is an odd number, the start of the LTD region will follow consecutively. You must therefore note that no more than a single block may exist in both regions.
// 
typedef struct CARDRomHashBlock
{
    struct CARDRomHashBlock    *next;
    u32                         index;
    u32                         offset;
    u8                         *hash;
    u8                         *hash_aligned;
}
CARDRomHashBlock;

// This structure manages SRL file hashes.
// Each program has different size requirements that cannot be calculated statically. As a result, the plan is to dynamically allocate only the appropriate amount from the arena.
// 
// Memory requirements are as follows.
//   - The master hash table:
//       A ROM header must be compared with its hash to determine if it is valid, so we will load it all at once from ROM and make it resident during initialization.
//       
//       Calculate the number of bytes required as follows: (ROM size / sector size / number of sectors) * 20 bytes (for SHA1). If the ROM size is 1 gigabit, the sector size is 1024 bytes, and there are 32 sectors, this will be approximately 80 KB.
//       
//   - The block cache:
//       Manages the ROM image cache and its hash in blocks.
//       A list structure that can always manage multiple blocks is necessary because discrete accesses may span block boundaries.
//       
//       Memory is required only for the total list count, with 20 structures per sector + a.
//   - The sector cache:
//       Maintains the actual image cache.
//       Blocks will not necessarily access all sectors universally, so a separate list structure is required to manage sectors, as well.
//       
typedef struct CARDRomHashContext
{
    // Basic settings obtained from the ROM header
    CARDRomRegion       area_ntr;
    CARDRomRegion       area_ltd;
    CARDRomRegion       sector_hash;
    CARDRomRegion       block_hash;
    u32                 bytes_per_sector;
    u32                 sectors_per_block;
    u32                 block_max;
    u32                 sector_max;

    // Device interface to load data and its hash
    void                   *userdata;
    MIDeviceReadFunction    ReadSync;
    MIDeviceReadFunction    ReadAsync;

    // Thread that is currently loading data.
    OSThread           *loader;
    void               *recent_load;

    // Sector and block cache
    CARDRomHashSector  *loading_sector; // Sector waiting for media to load
    CARDRomHashSector  *loaded_sector;  // Sector waiting for hash verification
    CARDRomHashSector  *valid_sector;   // Sector that has already been verified
    CARDRomHashBlock   *loading_block;  // Block waiting for media to load
    CARDRomHashBlock   *loaded_block;   // Block waiting for hash verification
    CARDRomHashBlock   *valid_block;    // Block that has already been verified
    // Array allocated from an arena
    u8                 *master_hash;    // A block hash array
    u8                 *images;         // Sector image
    u8                 *hashes;         // The hash array within a block
    CARDRomHashSector  *sectors;        // Sector data
    CARDRomHashBlock   *blocks;         // Block data
}
CARDRomHashContext;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_InitRomHashContext

  Description:  Initializes a ROM hash context.

  Arguments:    context: The CARDRomHashContext structure that should be initialized
                header: A ROM header buffer with ROM data that should be managed
                           (This is only used within this function and does not need to be maintained)
                buffer: Buffer for management data to use within a context
                length: Buffer size for management data
                           (This can be calculated with the CARD_CalcRomHashBufferLength function)
                sync: Synchronous callback function for device reads
                async: Asynchronous callback function for device reads (NULL if unsupported)
                userdata: Arbitrary user-defined argument to pass to the device read callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_InitRomHashContext(CARDRomHashContext *context, const CARDRomHeaderTWL *header,
                             void *buffer, u32 length,
                             MIDeviceReadFunction sync, MIDeviceReadFunction async,
                             void *userdata);

/*---------------------------------------------------------------------------*
  Name:         CARD_CalcRomHashBufferLength

  Description:  Gets the buffer size required for the ROM hash context.

  Arguments:    header: A ROM header buffer with ROM data that should be managed

  Returns:      The buffer size required for the ROM hash context.
 *---------------------------------------------------------------------------*/
u32 CARD_CalcRomHashBufferLength(const CARDRomHeaderTWL *header);

/*---------------------------------------------------------------------------*
  Name:         CARDi_NotifyRomHashReadAsync

  Description:  Notifies a hash context that an asynchronous device read has finished.

  Arguments:    context: CARDRomHashContext structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_NotifyRomHashReadAsync(CARDRomHashContext *context);

/*---------------------------------------------------------------------------*
  Name:         CARD_ReadRomHashImage

  Description:  Loads a ROM image with a verified hash from the specified offset.

  Arguments:    context: CARDRomHashContext structure.
                offset: The ROM offset to access.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_ReadRomHashImage(CARDRomHashContext *context, void *buffer, u32 offset, u32 length);


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_HASH_H_
