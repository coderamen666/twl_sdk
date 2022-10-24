/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     archive_block.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_FND_ARCHIVE_BLOCK_H_
#define NNS_FND_ARCHIVE_BLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         NNSiFndArchiveBlockHeader

  Description:  Data block header of the archive file
 *---------------------------------------------------------------------------*/
typedef struct
{
	u32			blockType;				// block type
	u32			blockSize;				// block size

} NNSiFndArchiveBlockHeader;


/*---------------------------------------------------------------------------*
  Name:         NNSiFndArchiveDirBlock
  				NNSiFndArchiveImgBlock

  Description:  The directory information block header structure of the archive file.
  				The file image block header structure of the archive file.
 *---------------------------------------------------------------------------*/

typedef	NNSiFndArchiveBlockHeader	NNSiFndArchiveDirBlockHeader;
typedef	NNSiFndArchiveBlockHeader	NNSiFndArchiveImgBlockHeader;


/*---------------------------------------------------------------------------*
  Name:         NNSFndArchiveFatBlock

  Description:  The FAT block header structure of the archive file.
 *---------------------------------------------------------------------------*/
typedef struct
{
	u32			blockType;				// block type
	u32			blockSize;				// block size
	u16			numFiles;				// number of files
	u16			reserved;				// Reserved

} NNSiFndArchiveFatBlockHeader;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_FND_ARCHIVE_BLOCK_H_ */
#endif
