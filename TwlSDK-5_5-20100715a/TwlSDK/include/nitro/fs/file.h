/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - include
  File:     file.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-05#$
  $Rev: 9524 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#if	!defined(NITRO_FS_FILE_H_)
#define NITRO_FS_FILE_H_


#include <nitro/fs/archive.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// File structure
typedef struct FSFile
{
// private:
    struct FSFile              *next;
    void                       *userdata;
    struct FSArchive           *arc;
    u32                         stat;
    void                       *argument;
    FSResult                    error;
    OSThreadQueue               queue[1];

    union
    {
        u8                      reserved1[16];
        FSROMFATProperty        prop;
    };
    union
    {
        u8                      reserved2[24];
        FSROMFATCommandInfo     arg;
    };
}
FSFile;


SDK_COMPILER_ASSERT(sizeof(FSFile) == 72);


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FS_InitFile

  Description:  Initializes an FSFile structure.

  Arguments:    file: An FSFile structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_InitFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetAttachedArchive

  Description:  Gets the archive associated with a file or directory.

  Arguments:    file: A file or directory handle

  Returns:      The archive for the handle if it is being held, and NULL otherwise.
 *---------------------------------------------------------------------------*/
SDK_INLINE FSArchive *FS_GetAttachedArchive(const FSFile *file)
{
    return file->arc;
}
/*---------------------------------------------------------------------------*
  Name:         FS_IsBusy

  Description:  Determines if a file is processing a command.

  Arguments:    file: The FSFile structure that issued the command.

  Returns:      TRUE if the command is being processed.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsBusy(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_BUSY) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsCanceling

  Description:  Determines if a file is cancelling a command.

  Arguments:    file: The FSFile structure that issued the command.

  Returns:      TRUE if the command is being cancelled.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsCanceling(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_CANCEL) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsSucceeded

  Description:  Determines if the most recent command completed successfully.

  Arguments:    file: The FSFile structure that issued the command.

  Returns:      TRUE if the command completed successfully.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsSucceeded(volatile const FSFile *file)
{
    return (file->error == FS_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsFile

  Description:  Determines if a handle represents an open file.

  Arguments:    file: An FSFile structure

  Returns:      TRUE if the file handle is being held.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsFile(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_IS_FILE) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsDir

  Description:  Determines if a handle represents an open directory.

  Arguments:    file: An FSFile structure

  Returns:      TRUE if the directory handle is being held.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsDir(volatile const FSFile *file)
{
    return ((file->stat & FS_FILE_STATUS_IS_DIR) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetResultCode

  Description:  Gets the value of the most recent command result.

  Arguments:    file: An FSFile structure

  Returns:      The value of the most recent command result.
 *---------------------------------------------------------------------------*/
SDK_INLINE FSResult FS_GetResultCode(volatile const FSFile *file)
{
    return file->error;
}

/*---------------------------------------------------------------------------*
  Name:         FS_WaitAsync

  Description:  Blocks until asynchronous commands have completed.

  Arguments:    file: File handle

  Returns:      TRUE if the command is successful.
 *---------------------------------------------------------------------------*/
BOOL    FS_WaitAsync(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_CancelFile

  Description:  Requests that asynchronous commands be cancelled.

  Arguments:    file: File handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_CancelFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_SetFileHandle

  Description:  Initializes a file structure as a file handle.
                This function is called from the archive implementation to support the OpenFile command.
                

  Arguments:    file: An FSFile structure
                arc: The archive with which to associate
                userdata: Arbitrary archive-defined data to maintain file handle information
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FS_SetFileHandle(FSFile *file, FSArchive *arc, void *userdata)
{
    file->stat |= FS_FILE_STATUS_IS_FILE;
    file->stat &= ~FS_FILE_STATUS_IS_DIR;
    file->arc = arc;
    file->userdata = userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetDirectoryHandle

  Description:  Initializes a file structure as a directory handle.
                This function is called from the archive implementation to support the OpenDirectory command.
                

  Arguments:    file: An FSFile structure
                arc: The archive with which to associate
                userdata: Arbitrary archive-defined data to maintain directory handle information
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FS_SetDirectoryHandle(FSFile *file, FSArchive *arc, void *userdata)
{
    file->stat |= FS_FILE_STATUS_IS_DIR;
    file->stat &= ~FS_FILE_STATUS_IS_FILE;
    file->arc = arc;
    file->userdata = userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DetachHandle

  Description:  Initializes a file structure as an invalid handle.
                This function is called from the archive implementation to support the CloseFile and CloseDirectory commands.
                

  Arguments:    file: An FSFile structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FS_DetachHandle(FSFile *file)
{
    file->userdata = NULL;
    file->stat &= ~(FS_FILE_STATUS_IS_FILE | FS_FILE_STATUS_IS_DIR);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileUserData

  Description:  Gets archive-defined data associated with a file structure.
                This is called from the archive implementation.

  Arguments:    file: An FSFile structure

  Returns:      Archive-defined data associated with a file structure.
 *---------------------------------------------------------------------------*/
SDK_INLINE void* FS_GetFileUserData(const FSFile *file)
{
    return file->userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathName

  Description:  Gets the full path name to the specified file or directory.

  Arguments:    file: A file or directory handle
                buffer: Location to store the path name
                length: The buffer size

  Returns:      TRUE if it could be obtained properly, including the terminating '\0'.
 *---------------------------------------------------------------------------*/
BOOL    FS_GetPathName(FSFile *file, char *buffer, u32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathLength

  Description:  Gets the length of the full path name to the specified file or directory.

  Arguments:    file: A file or directory handle

  Returns:      The length of the path name, including the terminating '\0', on success and -1 on failure.
 *---------------------------------------------------------------------------*/
s32     FS_GetPathLength(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFile

  Description:  Creates a file.

  Arguments:    path: Path name
                mode: Access mode

  Returns:      TRUE if the file was created normally.
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateFile(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFile

  Description:  Deletes a file.

  Arguments:    path: Path name

  Returns:      TRUE if the file was deleted normally.
 *---------------------------------------------------------------------------*/
BOOL    FS_DeleteFile(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFile

  Description:  Renames a file.

  Arguments:    src: The original filename to change.
                dst: The new filename to change to.

  Returns:      TRUE if the file was renamed normally.
 *---------------------------------------------------------------------------*/
BOOL    FS_RenameFile(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathInfo

  Description:  Gets information for a file or directory.

  Arguments:    path: Path name
                info: Location to store the information

  Returns:      Processing result
 *---------------------------------------------------------------------------*/
BOOL    FS_GetPathInfo(const char *path, FSPathInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FS_SetPathInfo

  Description:  Sets information for a file or directory.

  Arguments:    path: Path name
                info: Location to store the information

  Returns:      Processing result
 *---------------------------------------------------------------------------*/
BOOL    FS_SetPathInfo(const char *path, const FSPathInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectory

  Description:  Creates a directory.

  Arguments:    path: Path name
                mode: Access mode

  Returns:      TRUE if the directory was created normally.
 *---------------------------------------------------------------------------*/
BOOL    FS_CreateDirectory(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectory

  Description:  Deletes a directory.

  Arguments:    path: Path name

  Returns:      TRUE if the directory was deleted normally.
 *---------------------------------------------------------------------------*/
BOOL    FS_DeleteDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectory

  Description:  Renames a directory.

  Arguments:    src: The original directory name to change.
                dst: The new directory name to change to.

  Returns:      TRUE if the directory was renamed normally.
 *---------------------------------------------------------------------------*/
BOOL    FS_RenameDirectory(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileAuto

  Description:  Creates a file along with required intermediate directories.

  Arguments:    path: Path name
                permit: Access mode

  Returns:      TRUE if the file was created normally.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFileAuto(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFileAuto

  Description:  Deletes a file along with required intermediate directories.

  Arguments:    path: Path name

  Returns:      TRUE if the file was deleted normally.
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteFileAuto(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFileAuto

  Description:  Changes a filename along with required intermediate directories.

  Arguments:    src: The original filename to change.
                dst: The new filename to change to.

  Returns:      TRUE if the file was renamed normally.
 *---------------------------------------------------------------------------*/
BOOL FS_RenameFileAuto(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectoryAuto

  Description:  Creates a directory along with required intermediate directories.

  Arguments:    path: The directory name to create
                permit: Access mode

  Returns:      TRUE if the directory was created normally.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateDirectoryAuto(const char *path, u32 permit);

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectoryAuto

  Description:  Recursively deletes a directory.

  Arguments:    path: Path name

  Returns:      TRUE if the directory was deleted normally.
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteDirectoryAuto(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectoryAuto

  Description:  Renames a directory, automatically creating any required intermediate directories.

  Arguments:    src: The original directory name to change.
                dst: The new directory name to change to.

  Returns:      TRUE if the directory was renamed normally.
 *---------------------------------------------------------------------------*/
BOOL FS_RenameDirectoryAuto(const char *src, const char *dst);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResource

  Description:  Gets resource information for the specified archive.

  Arguments:    path: Path name that can specify an archive
                resource: Location to store the obtained resource information

  Returns:      TRUE if the resource information was obtained normally.
 *---------------------------------------------------------------------------*/
BOOL    FS_GetArchiveResource(const char *path, FSArchiveResource *resource);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetSpaceToCreateDirectoryEntries

  Description:  Estimates the space required to store the directory entries that will be created at the same time as a file.
                (This assumes directories that already exist in the path will need to be re-created)

  Arguments:    path: The path name for the file to create.
                bytesPerCluster: Number of bytes for each cluster in the file system.

  Returns:      Space required to store the directory entries.
 *---------------------------------------------------------------------------*/
u32  FSi_GetSpaceToCreateDirectoryEntries(const char *path, const u32 bytesPerCluster);

/*---------------------------------------------------------------------------*
  Name:         FS_HasEnoughSpaceToCreateFile

  Description:  Determines if it is currently possible to create a file with the specified path name and size.

  Arguments:    resource: Archive information obtained in advance with the FS_GetArchiveResource function.
                            When the function succeeds, each of the sizes will be decreased only by the amount that the file consumes.
                path: The path name for the file to create.
                size: The size of the file to create.

  Returns:      TRUE if a file can be created with the specified path name and size.
 *---------------------------------------------------------------------------*/
BOOL FS_HasEnoughSpaceToCreateFile(FSArchiveResource *resource, const char *path, u32 size);

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveReady

  Description:  Determines if the specified archive is currently usable.

  Arguments:    path: An absolute path that starts with "archive_name:"

  Returns:      TRUE if the specified archive is currently usable.
                FALSE for an archive on an SD Card that has not been inserted in the slot, or a save data archive that has not been imported.
                
 *---------------------------------------------------------------------------*/
BOOL FS_IsArchiveReady(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileEx

  Description:  Specifies a path name and opens a file.

  Arguments:    file: An FSFile structure
                path: Path name
                mode: Access mode

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileEx(FSFile *file, const char *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_ConvertPathToFileID

  Description:  Gets the file ID from a path name.

  Arguments:    p_fileid: Location to store the FSFileID
                path: Path name

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_ConvertPathToFileID(FSFileID *p_fileid, const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileFast

  Description:  Specifies a file ID and opens a file.

  Arguments:    file: An FSFile structure
                fileid: File ID obtained by a function such as FS_ReadDir

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenFileFast(FSFile *file, FSFileID fileid);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileDirect

  Description:  Directly maps an offset into an archive and opens a file.

  Arguments:    file: An FSFile structure
                arc: Archive to map the file
                top: Offset to the start of the file image
                bottom: Offset to the end of the file image
                fileid: Arbitrary file ID to assign

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenFileDirect(FSFile *file, FSArchive *arc,
                          u32 top, u32 bottom, u32 fileid);

/*---------------------------------------------------------------------------*
  Name:         FS_CloseFile

  Description:  Closes a file handle.

  Arguments:    file: An FSFile structure

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_CloseFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileLength

  Description:  Gets the file size.

  Arguments:    file: File handle

  Returns:      File size
 *---------------------------------------------------------------------------*/
u32     FS_GetFileLength(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_SetFileLength

  Description:  Changes the file size.

  Arguments:    file: File handle
                length: The changed size

  Returns:      Processing result
 *---------------------------------------------------------------------------*/
FSResult FS_SetFileLength(FSFile *file, u32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFilePosition

  Description:  Gets the current position of a file pointer.

  Arguments:    file: File handle

  Returns:      The current position of the file pointer.
 *---------------------------------------------------------------------------*/
u32     FS_GetFilePosition(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetSeekCacheSize

  Description:  Finds the buffer size required by a full cache for a fast reverse seek.

  Arguments:    path

  Returns:      The size on success and 0 on failure.
 *---------------------------------------------------------------------------*/
u32 FS_GetSeekCacheSize( const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_SetSeekCache

  Description:  Assigns the cache buffer for a fast reverse seek.

  Arguments:    file: File handle
                buf: Cache buffer
                buf_size: Cache buffer size

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL FS_SetSeekCache(FSFile *file, void* buf, u32 buf_size);

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFile

  Description:  Moves a file pointer.

  Arguments:    file: File handle
                offset: The offset to move
                origin: The origin to move from

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_SeekFile(FSFile *file, s32 offset, FSSeekFileMode origin);

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFileToBegin

  Description:  Moves a file pointer to the beginning of the file.

  Arguments:    file: File handle

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_SeekFileToBegin(FSFile *file)
{
    return FS_SeekFile(file, 0, FS_SEEK_SET);
}

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFileToEnd

  Description:  Moves a file pointer to the end of the file.

  Arguments:    file: File handle

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_SeekFileToEnd(FSFile *file)
{
    return FS_SeekFile(file, 0, FS_SEEK_END);
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFile

  Description:  Reads data from a file.

  Arguments:    file: File handle
                buffer: Buffer to transfer to
                Length: Read size

  Returns:      The size that was actually read on success and -1 on failure.
 *---------------------------------------------------------------------------*/
s32     FS_ReadFile(FSFile *file, void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFileAsync

  Description:  Asynchronously reads data from a file.

  Arguments:    file: File handle
                buffer: Buffer to transfer to
                Length: Read size

  Returns:      The same value as length on success and -1 on failure.
 *---------------------------------------------------------------------------*/
s32     FS_ReadFileAsync(FSFile *file, void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFile

  Description:  Writes data to a file.

  Arguments:    file: File handle
                buffer: Buffer to transfer from
                length: Write size

  Returns:      The size that was actually written on success and -1 on failure.
 *---------------------------------------------------------------------------*/
s32     FS_WriteFile(FSFile *file, const void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFileAsync

  Description:  Asynchronously writes data to a file.

  Arguments:    file: File handle
                buffer: Buffer to transfer from
                length: Write size

  Returns:      The same value as length on success and -1 on failure.
 *---------------------------------------------------------------------------*/
s32     FS_WriteFileAsync(FSFile *file, const void *buffer, s32 length);

/*---------------------------------------------------------------------------*
  Name:         FS_FlushFile

  Description:  Applies file changes to the device.

  Arguments:    file: File handle

  Returns:      Processing result
 *---------------------------------------------------------------------------*/
FSResult FS_FlushFile(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectory

  Description:  Opens a directory handle.

  Arguments:    file: An FSFile structure
                path: Path name
                mode: Access mode

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenDirectory(FSFile *file, const char *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_CloseDirectory

  Description:  Closes a directory handle.

  Arguments:    file: An FSFile structure

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_CloseDirectory(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectory

  Description:  Reads through only one directory entry.

  Arguments:    file: An FSFile structure
                info: FSDirectoryEntryInfo structure

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_ReadDirectory(FSFile *file, FSDirectoryEntryInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FS_TellDir

  Description:  Gets the current directory position from a directory handle.

  Arguments:    dir: Directory handle
                pos: Location to store the directory position

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_TellDir(const FSFile *dir, FSDirPos *pos);

/*---------------------------------------------------------------------------*
  Name:         FS_SeekDir

  Description:  Opens a directory handle with the directory position specified.

  Arguments:    dir: Directory handle
                pos: Directory position

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_SeekDir(FSFile *p_dir, const FSDirPos *p_pos);

/*---------------------------------------------------------------------------*
  Name:         FS_RewindDir

  Description:  Moves the list position for a directory handle back to the beginning.

  Arguments:    dir: Directory handle

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_RewindDir(FSFile *dir);


/*---------------------------------------------------------------------------*
 * Unicode Support
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileExW

  Description:  Specifies a path name and opens a file.

  Arguments:    file: An FSFile structure
                path: Path name

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileExW(FSFile *file, const u16 *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectoryW

  Description:  Opens a directory handle.

  Arguments:    file: An FSFile structure
                path: Path name
                mode: Access mode

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenDirectoryW(FSFile *file, const u16 *path, u32 mode);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectoryW

  Description:  Reads through only one directory entry.

  Arguments:    file: An FSFile structure
                info: FSDirectoryEntryInfo structure

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDirectoryW(FSFile *file, FSDirectoryEntryInfoW *info);


/*---------------------------------------------------------------------------*
 * Deprecated Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFile

  Description:  Specifies a path name and opens a file.

  Arguments:    file: An FSFile structure
                path: Path name

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_OpenFile(FSFile *file, const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_GetLength

  Description:  Gets the file size.

  Arguments:    file: File handle

  Returns:      File size
 *---------------------------------------------------------------------------*/
u32     FS_GetLength(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_GetPosition

  Description:  Gets the current position of a file pointer.

  Arguments:    file: File handle

  Returns:      The current position of the file pointer.
 *---------------------------------------------------------------------------*/
u32     FS_GetPosition(FSFile *file);

/*---------------------------------------------------------------------------*
  Name:         FS_FindDir

  Description:  Opens a directory handle.

  Arguments:    dir: FSFile structure
                path: Path name

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_FindDir(FSFile *dir, const char *path);

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDir

  Description:  Reads through only a single instance of directory handle entry information.

  Arguments:    dir: Directory handle
                entry: Location to store the entry information

  Returns:      TRUE on success.
 *---------------------------------------------------------------------------*/
BOOL    FS_ReadDir(FSFile *dir, FSDirEntry *entry);

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileInfo

  Description:  Gets information for a file or directory.

  Arguments:    path: Path name
                info: Location to store the information

  Returns:      Processing result
 *---------------------------------------------------------------------------*/
typedef FSPathInfo  FSFileInfo;
FSResult FS_GetFileInfo(const char *path, FSFileInfo *info);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_FILE_H_ */
