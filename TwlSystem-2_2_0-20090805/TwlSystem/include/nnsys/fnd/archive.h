/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - fnd
  File:     archive.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_FND_ARCHIVE_H_
#define NNS_FND_ARCHIVE_H_

//#include <stddef.h>
#include <nitro/os.h>
#include <nitro/fs.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         NNSFndArchiveFatEntry

  Description:  FAT Entry Structure of Archive
 *---------------------------------------------------------------------------*/
typedef struct
{
	u32						fileTop;			// Offset to start of file image
	u32						fileBottom;			// offset of file image end

} NNSiFndArchiveFatEntry;


/*---------------------------------------------------------------------------*
  Name:         NNSiFndArchiveFatData

  Description:  FAT Data Structure of Archive
 *---------------------------------------------------------------------------*/
typedef struct
{
	u32						blockType;			// block type
	u32						blockSize;			// block size
	u16						numFiles;			// number of files
	u16						reserved;			// Reserved
	NNSiFndArchiveFatEntry	fatEntries[1];

} NNSiFndArchiveFatData;


/*---------------------------------------------------------------------------*
  Name:         NNSiFndArchiveHeader

  Description:  File Header Structure of Archive
 *---------------------------------------------------------------------------*/
typedef struct
{
	u32						signature;			// signature (NARC)
	u16						byteOrder;			// byte order (0xfeff)
	u16						version;			// Version number
	u32						fileSize;			// archive file size
	u16						headerSize;			// archive file header size
	u16						dataBlocks;			// number of data blocks

} NNSiFndArchiveHeader;


/*---------------------------------------------------------------------------*
  Name:         NNSFndArchive

  Description:  NNS Archive Structure. Maintains the archive information.
  				The TWL-System's archive manager specifies this NNS archive structure to identify specific archives.
  				
 *---------------------------------------------------------------------------*/
typedef struct
{
	FSArchive				fsArchive;			// Work for file system
	NNSiFndArchiveHeader*	arcBinary;			// Start of archive binary data
	NNSiFndArchiveFatData*	fatData;			// Start of archive FAT data
	u32						fileImage;			// Start of archive file image

} NNSFndArchive;


/*---------------------------------------------------------------------------*
    Function Prototypes

 *---------------------------------------------------------------------------*/

BOOL		NNS_FndMountArchive(
					NNSFndArchive*		archive,
					const char*			arcName,
					void*				arcBinary);

BOOL		NNS_FndUnmountArchive(
					NNSFndArchive*		archive);

void*		NNS_FndGetArchiveFileByName(
					const char*			path);

void*		NNS_FndGetArchiveFileByIndex(
					NNSFndArchive*		archive,
					u32					index);

BOOL		NNS_FndOpenArchiveFileByIndex(
					FSFile*				file,
					NNSFndArchive*		archive,
					u32					index);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_FND_ARCHIVE_H_ */
#endif
