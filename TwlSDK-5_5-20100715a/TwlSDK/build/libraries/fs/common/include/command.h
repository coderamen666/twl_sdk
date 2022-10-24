/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     command.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-08#$
  $Rev: 9544 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#if	!defined(NITRO_FS_COMMAND_H_)
#define NITRO_FS_COMMAND_H_

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/fs/archive.h>
#include <nitro/fs/file.h>
#include <nitro/fs/romfat.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// Structure that packs arguments to temporarily save them to the task list.

typedef struct FSArgumentForReadFile
{
    void       *buffer;
    u32         length;
}
FSArgumentForReadFile;

typedef struct FSArgumentForWriteFile
{
    const void *buffer;
    u32         length;
}
FSArgumentForWriteFile;

typedef struct FSArgumentForSeekDirectory
{
    u32         id;
    u32         position;
}
FSArgumentForSeekDirectory;

typedef struct FSArgumentForReadDirectory
{
    FSDirectoryEntryInfo   *info;
}
FSArgumentForReadDirectory;

typedef struct FSArgumentForFindPath
{
    u32                 baseid;
    const char         *relpath;
    u32                 target_id;
    BOOL                target_is_directory;
}
FSArgumentForFindPath;

typedef struct FSArgumentForGetPath
{
    BOOL                is_directory;
    char               *buffer;
    u32                 length;
}
FSArgumentForGetPath;

typedef struct FSArgumentForOpenFileFast
{
    u32                 id;
    u32                 mode;
}
FSArgumentForOpenFileFast;

typedef struct FSArgumentForOpenFileDirect
{
    u32                 id; // in : requested-id
    u32                 top;
    u32                 bottom;
    u32                 mode;
}
FSArgumentForOpenFileDirect;

typedef void FSArgumentForCloseFile;
typedef void FSArgumentForActivate;
typedef void FSArgumentForIdle;
typedef void FSArgumentForSuspend;
typedef void FSArgumentForResume;

typedef struct FSArgumentForOpenFile
{
    u32             baseid;
    const char     *relpath;
    u32             mode;
}
FSArgumentForOpenFile;

typedef struct FSArgumentForSetSeekCache
{
    void           *buf;
    u32             buf_size;
}
FSArgumentForSetSeekCache;

typedef struct FSArgumentForSeekFile
{
    int             offset;
    FSSeekFileMode  from;
}
FSArgumentForSeekFile;

typedef struct FSArgumentForGetFileLength
{
    u32             length;
}
FSArgumentForGetFileLength;

typedef struct FSArgumentForGetFilePosition
{
    u32             position;
}
FSArgumentForGetFilePosition;

typedef struct FSArgumentForGetArchiveCaps
{
    u32             caps;
}
FSArgumentForGetArchiveCaps;

typedef void FSArgumentForMount;
typedef void FSArgumentForUnmount;

typedef struct FSArgumentForCreateFile
{
    u32         baseid;
    const char *relpath;
    u32         permit;
}
FSArgumentForCreateFile;

typedef struct FSArgumentForDeleteFile
{
    u32         baseid;
    const char *relpath;
}
FSArgumentForDeleteFile;

typedef struct FSArgumentForRenameFile
{
    u32         baseid_src;
    const char *relpath_src;
    u32         baseid_dst;
    const char *relpath_dst;
}
FSArgumentForRenameFile;

typedef struct FSArgumentForGetPathInfo
{
    u32         baseid;
    const char *relpath;
    FSPathInfo *info;
}
FSArgumentForGetPathInfo;

typedef struct FSArgumentForSetPathInfo
{
    u32         baseid;
    const char *relpath;
    FSPathInfo *info;
}
FSArgumentForSetPathInfo;

typedef FSArgumentForCreateFile     FSArgumentForCreateDirectory;
typedef FSArgumentForDeleteFile     FSArgumentForDeleteDirectory;
typedef FSArgumentForRenameFile     FSArgumentForRenameDirectory;

typedef struct FSArgumentForGetArchiveResource
{
    FSArchiveResource  *resource;
}
FSArgumentForGetArchiveResource;

typedef void FSArgumentForFlushFile;

typedef struct FSArgumentForSetFileLength
{
    u32             length;
}
FSArgumentForSetFileLength;

typedef FSArgumentForOpenFile   FSArgumentForOpenDirectory;
typedef FSArgumentForCloseFile  FSArgumentForCloseDirectory;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FSi_SendCommand

  Description:  Issues a command to an archive.
                Adjusts the start time and blocks here if it is synchronous.

  Arguments:    p_file: FSFile structure that stores the command arguments.
                command: Command ID
                blocking: TRUE to block

  Returns:      TRUE if the command is successful.
 *---------------------------------------------------------------------------*/
BOOL    FSi_SendCommand(FSFile *p_file, FSCommandType command, BOOL blocking);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetCurrentCommand

  Description:  Gets the command that is currently being processed.

  Arguments:    file: FSFile structure that stores the command arguments

  Returns:      The command that is currently being processed.
 *---------------------------------------------------------------------------*/
SDK_INLINE FSCommandType FSi_GetCurrentCommand(const FSFile *file)
{
    return (FSCommandType)((file->stat >> FS_FILE_STATUS_CMD_SHIFT) & FS_FILE_STATUS_CMD_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_WaitForArchiveCompletion

  Description:  Waits for asynchronous archive processes to complete.
                If a command depends on another low-level command, this will be called internally by the procedure.
                

  Arguments:    file: FSFile structure that stores the command arguments
                result: The return value from the archive
                                 Processing will actually block only when this is FS_RESULT_PROC_ASYNC.
                                 

  Returns:      The actual processing result
 *---------------------------------------------------------------------------*/
FSResult FSi_WaitForArchiveCompletion(FSFile *file, FSResult result);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetArchiveChain

  Description:  Gets the first registered archive.

  Arguments:    None.

  Returns:      The first registered archive.
 *---------------------------------------------------------------------------*/
FSArchive* FSi_GetArchiveChain(void);

/*---------------------------------------------------------------------------*
  Name:         FSi_IsUnreadableRomOffset

  Description:  Determines whether the specified ROM offset is unreadable in the current environment.

  Arguments:    arc: The calling archive.
                offset: The ROM offset to check.

  Returns:      TRUE if the specified ROM offset is unreadable in the current environment.
 *---------------------------------------------------------------------------*/
BOOL FSi_IsUnreadableRomOffset(FSArchive *arc, u32 offset);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_COMMAND_H_ */
