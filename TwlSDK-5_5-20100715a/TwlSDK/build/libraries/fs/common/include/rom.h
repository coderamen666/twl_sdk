/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     rom.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#ifndef NITRO_FS_ROM_H_
#define NITRO_FS_ROM_H_

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/card/hash.h>
#include <nitro/fs/file.h>
#include <nitro/fs/archive.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* functions */
#if defined(FS_IMPLEMENT)

/*---------------------------------------------------------------------------*
  Name:         FSi_InitRomArchive

  Description:  Initializes the "rom" archive.

  Arguments:    default_dma_no: The DMA channel to use for ROM access.
                                 If this is not between 0 and 3, access will be handled by the CPU.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_InitRomArchive(u32 default_dma_no);

/*---------------------------------------------------------------------------*
  Name:         FSi_EndRomArchive

  Description:  Deallocates the "rom" archive.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_EndRomArchive(void);

/*---------------------------------------------------------------------------*
  Name:         FSi_MountSRLFile

  Description:  Mounts the ROM file system included in an SRL file.

  Arguments:    arc: The FSArchive structure to use when mounting.
                                 The name must already be registered.
                file: An open file to mount.
                                 This structure cannot be destroyed while it is mounted.
                hash: Hash context structure.

  Returns:      TRUE if processing is successful.
 *---------------------------------------------------------------------------*/
BOOL FSi_MountSRLFile(FSArchive *arc, FSFile *file, CARDRomHashContext *hash);

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertPathToFATFS

  Description:  Converts the specified path name into the FATFS format.

  Arguments:    dst: Buffer to save to
                src: Path to convert
                ignorePermission: TRUE when access permissions may be ignored

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_ConvertPathToFATFS(char *dst, const char *src, BOOL ignorePermission);

FSResult FSi_ConvertError(u32 error);

/*---------------------------------------------------------------------------*
  Name:         FSi_MountFATFS

  Description:  Mounts a FATFS interface in a file system.

  Arguments:    index: Array element to use
                arcname: Archive name
                drivename: Drive name

  Returns:      TRUE if the archive is mounted correctly.
 *---------------------------------------------------------------------------*/
BOOL FSi_MountFATFS(u32 index, const char *arcname, const char *drivename);

/*---------------------------------------------------------------------------*
  Name:         FSi_MountDefaultArchives

  Description:  Accesses the startup arguments given to the IPL and mounts the default archive.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_MountDefaultArchives(void);

#else /* FS_IMPLEMENT */

/*---------------------------------------------------------------------------*
  Name:         FSi_ReadRomDirect

  Description:  Directly reads the specified ROM address.

  Arguments:    src:        Transfer source offset
                dst: Address to transfer to
                len:        Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_ReadRomDirect(const void *src, void *dst, u32 len);

#endif


#ifdef __cplusplus
} /* extern "C" */
#endif


/*---------------------------------------------------------------------------*/


#endif /* NITRO_FS_ROM_H_ */
