/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - fnd
  File:     archive.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1508 $
 *---------------------------------------------------------------------------*/

#include <nnsys/misc.h>
#include <nnsys/fnd/archive.h>
#include <string.h>

#include "archive_block.h"


/*---------------------------------------------------------------------------*
  Name:         IsValidArchiveImage

  Description:  Investigates the content of the specified archive header to determine whether the archive binary is valid.
  				

  Arguments:    arcBinary:	Pointer to the archive header

  Returns:      Returns TRUE if the archive header is valid
 *---------------------------------------------------------------------------*/
static BOOL
IsValidArchiveBinary(NNSiFndArchiveHeader* arcBinary)
{
	NNS_NULL_ASSERT(arcBinary);

	if (arcBinary->signature != 'CRAN')  return FALSE;
	if (arcBinary->byteOrder != 0xfffe)  return FALSE;
	if (arcBinary->version   != 0x0100)  return FALSE;

	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndMountArchive

  Description:  Mounts the designated archive binary to the file system.
  				When mounting the archive, specify an identifier up to 3 characters in length.
  				This identifier must be denoted before a path name when designating an archive within the file system.
  				

  Arguments:    archive:	Pointer to the NNS archive structure
  				arc Name:	Name used to identify the archive on the file system
  				arcBinary:	Pointer to the archive binary read out to memory
  							
  				
  Returns:      Returns TRUE if the mounting was successful.
 *---------------------------------------------------------------------------*/
BOOL
NNS_FndMountArchive(NNSFndArchive* archive, const char* arcName, void* arcBinary)
{
	NNSiFndArchiveHeader*		  arc = (NNSiFndArchiveHeader*)arcBinary;
	NNSiFndArchiveFatBlockHeader* fat = NULL;
	NNSiFndArchiveDirBlockHeader* fnt = NULL;
	NNSiFndArchiveImgBlockHeader* img = NULL;

	NNS_NULL_ASSERT(arcName);
	NNS_NULL_ASSERT(arcBinary);

	if (!IsValidArchiveBinary(arc))
	{
		OS_Printf("This archive binary is invalid.\n");
		return FALSE;
	}
	{
		int  count;
		NNSiFndArchiveBlockHeader* block = (NNSiFndArchiveBlockHeader*)((u32)arc + arc->headerSize);

		for (count=0; count<arc->dataBlocks; count++)
		{
			switch (block->blockType)
			{
				case 'FATB': fat = (NNSiFndArchiveFatBlockHeader*)block;	break;
				case 'FNTB': fnt = (NNSiFndArchiveDirBlockHeader*)block;	break;
				case 'FIMG': img = (NNSiFndArchiveImgBlockHeader*)block;	break;
			}
			block = (NNSiFndArchiveBlockHeader*)((u32)block + block->blockSize);
		}

		FS_InitArchive(&archive->fsArchive);
		archive->arcBinary = arc;
		archive->fatData   = (NNSiFndArchiveFatData*)fat;
		archive->fileImage = (u32)(img+1);

		if (!FS_RegisterArchiveName(&archive->fsArchive, arcName, strlen(arcName)))
		{
			OS_Printf("archive cannot register to file system.\n");
			return FALSE;
		}
		if (!FS_LoadArchive(&archive->fsArchive,
					(u32)(img+1),
					(u32)(fat+1) - (u32)(img+1), fat->blockSize-sizeof(NNSiFndArchiveFatBlockHeader),
					(u32)(fnt+1) - (u32)(img+1), fnt->blockSize-sizeof(NNSiFndArchiveDirBlockHeader),
					NULL, NULL)
		)
		{
			OS_Printf("archive cannot load to file system.\n");
			FS_ReleaseArchiveName(&archive->fsArchive);
			return FALSE;
		}
	}
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndUnmountArchive

  Description:  Unmounts the archive mounted to the file system.
  				

  Arguments:    archive:	Pointer to the NNS archive structure

  Returns:      Returns TRUE if the unmounting was successful.
 *---------------------------------------------------------------------------*/
BOOL
NNS_FndUnmountArchive(NNSFndArchive* archive)
{
	NNS_NULL_ASSERT(archive);

	if (!FS_UnloadArchive(&archive->fsArchive))
	{
		return FALSE;
	}
	FS_ReleaseArchiveName(&archive->fsArchive);
	return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetArchiveFileByName

  Description:  Gets the RAM address stored by the archive file corresponding to the specified path, using the file system.
  				

  Arguments:    path:	Path name of the archive file

  Returns:      Address where the archive file is stored
 *---------------------------------------------------------------------------*/
void*
NNS_FndGetArchiveFileByName(const char* path)
{
	void* addr = NULL;

#if 1
	FSFile file;

	NNS_NULL_ASSERT(path);
	FS_InitFile(&file);
	if (FS_OpenFile(&file, path))
	{
		NNSFndArchive* archive = (NNSFndArchive*)FS_GetAttachedArchive(&file);

		addr = (void*)(archive->fileImage + FS_GetFileImageTop(&file));
		(void)FS_CloseFile(&file);
	}
#else
	FSFileID fileID;

	NNS_NULL_ASSERT(path);
	if (FS_ConvertPathToFileID(&fileID, path))
	{
		NNSFndArchive* archive = (NNSFndArchive*)fileID.arc;

		addr = (void*)(archive->fileImage + archive->fatData->fatEntries[fileID.file_id].fileTop);
	}
#endif

	return addr;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndGetArchiveFileByIndex

  Description:  Gets the RAM address stored by the archive file specified by the index.
  				

  Arguments:    archive:	Pointer to the NNS archive structure
  				index:		Index value of the file inside the archive

  Returns:      Returns the address where the archive file is stored.
  				NULL is returned if there was no file corresponding to the specified index.
  				
 *---------------------------------------------------------------------------*/
void*
NNS_FndGetArchiveFileByIndex(NNSFndArchive* archive, u32 index)
{
	void* addr = NULL;

	NNS_NULL_ASSERT(archive);

	if (index < archive->fatData->numFiles)
	{
		addr = (void*)(archive->fileImage + archive->fatData->fatEntries[index].fileTop);
	}
	return addr;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_FndOpenArchiveFileByIndex

  Description:  Opens the archive file designated by the index.
  				

  Arguments:	file:		Pointer to the file structure
  			    archive:	Pointer to the NNS archive structure
  				index:		Index value of the file inside the archive

  Returns:      Returns TRUE if the file successfully opens.
 *---------------------------------------------------------------------------*/
BOOL
NNS_FndOpenArchiveFileByIndex(FSFile* file, NNSFndArchive* archive, u32 index)
{
	BOOL result = FALSE;

	NNS_NULL_ASSERT(file);
	NNS_NULL_ASSERT(archive);

	if (index < archive->fatData->numFiles)
	{
		FSFileID fileID;

		fileID.arc	   = &archive->fsArchive;
		fileID.file_id = index;

		result = FS_OpenFileFast(file, fileID);
	}
	return result;
}
