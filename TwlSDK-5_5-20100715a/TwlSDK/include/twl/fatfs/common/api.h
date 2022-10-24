/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FATFS - include
  File:     api.h

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_FATFS_API_H_
#define NITRO_FATFS_API_H_


#include <twl/fatfs/common/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FATFS_Init

  Description:  Initializes the FATFS library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM9
void FATFS_Init(void);
#else
BOOL FATFS_Init(u32 dma1, u32 dma2, u32 priority);
#endif

/*---------------------------------------------------------------------------*
  Name:         FATFSi_IsInitialized

  Description:  Returns a value indicating whether the FATFS library has been initialized.

  Arguments:    None.

  Returns:      TRUE if it has been initialized.
 *---------------------------------------------------------------------------*/
BOOL FATFSi_IsInitialized(void);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_GetArcnameList

  Description:  Gets a list of accessible archive names.

  Arguments:    None.

  Returns:      A list of archive names delimited by the null character ('\0'), with two null characters ("\0\0") at the end.
 *---------------------------------------------------------------------------*/
const char* FATFSi_GetArcnameList(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetLastError

  Description:  Gets the result of the last issued request.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
u32 FATFS_GetLastError(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RegisterResultBuffer

  Description:  Registers a command result buffer to associate with the calling thread.

  Arguments:    buffer: FATFSResultBuffer structure
                enable: TRUE to register and FALSE to unregister it

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFS_RegisterResultBuffer(FATFSResultBuffer *buffer, BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountDrive

  Description:  Mounts the specified NAND partition on a drive.

  Arguments:    name: Drive name (characters from 'A' to 'Z')
                partition: NAND partition number

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountDrive(const char *name, FATFSMediaType media, u32 partition);

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountNAND

  Description:  Mounts the specified NAND partition on a drive.

  Arguments:    name: Drive name (characters from 'A' to 'Z')
                partition: NAND partition number

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FATFS_MountNAND(const char *name, u32 partition)
{
    return FATFS_MountDrive(name, FATFS_MEDIA_TYPE_NAND, partition);
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_UnmountDrive

  Description:  Unmounts the specified drive.

  Arguments:    name: Drive name (characters from 'A' to 'Z')

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_UnmountDrive(const char *name);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetDefaultDrive

  Description:  Selects the default drive.

  Arguments:    path: Path, including the drive name

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetDefaultDrive(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FormatDrive

  Description:  Initializes the drive indicated by the specified path.

  Arguments:    path: Path, including the drive name

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_FormatDrive(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_FormatDriveEx

  Description:  Initializes the entire media or the entire drive indicated by the specified path.

  Arguments:    path: Path, including the drive name
                formatMedia: TRUE to erase the entire media

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFSi_FormatDriveEx(const char *path, BOOL formatMedia);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_FormatMedia

  Description:  Initializes the entire media indicated by the specified path.

  Arguments:    path: Path, including the drive name

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FATFSi_FormatMedia(const char *path)
{
    return FATFSi_FormatDriveEx(path, TRUE);
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_CheckDisk

  Description:  Checks disk content and fixes it if necessary.

  Arguments:    name: Drive name (characters from 'A' to 'Z')
                info: FATFSDiskInfo structure to store the result
                verbose: TRUE to send checked content to debug output
                fixProblems: TRUE to fix FAT errors once they are detected
                writeChains: TRUE to recover files from isolated chains
                              (valid only when fixProblems is enabled)

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_CheckDisk(const char *name, FATFSDiskInfo *info, BOOL verbose, BOOL fixProblems, BOOL writeChains);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetDriveResource

  Description:  Gets the amount of free disk space.

  Arguments:    path: Path, including the drive name
                resource: The FATFSDriveResource structure to store the result

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetDriveResource(const char *path, FATFSDriveResource *resource);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetDiskSpace

  Description:  Gets the amount of free disk space.

  Arguments:    name: Drive name (characters from 'A' to 'Z')
                totalBlocks: Pointer to get the total block count, or NULL
                freeBlocks: Pointer to get the free block count, or NULL

  Returns:      The number of bytes of free space on success and -1 on failure.
 *---------------------------------------------------------------------------*/
int FATFS_GetDiskSpace(const char *name, u32 *totalBlocks, u32 *freeBlocks);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetFileInfo

  Description:  Gets information for a file or directory.

  Arguments:    path: Path name
                info: FATFSFileInfo structure to store the result

  Returns:      Gets information and returns TRUE if the specified path exists; otherwise, returns FALSE.
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetFileInfo(const char *path, FATFSFileInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetFileInfo

  Description:  Changes file or directory information.

  Arguments:    path: Path name
                info: FATFSFileInfo structure storing the information that should be changed

  Returns:      Changes information and returns TRUE if the specified path exists; otherwise, returns FALSE.
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetFileInfo(const char *path, const FATFSFileInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateFile

  Description:  Creates a file at the specified path.

  Arguments:    path: Path name
                trunc: TRUE to delete the file if it already exists
                permit: File access permissions (rwx){1,3}

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateFile(const char *path, BOOL trunc, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteFile

  Description:  Deletes the specified file.

  Arguments:    path: Path name

  Returns:      TRUE if it was deleted successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteFile(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameFile

  Description:  Renames the specified file.

  Arguments:    path: Path name
                newpath: New path name

  Returns:      TRUE if it was renamed successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameFile(const char *path, const char *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateDirectory

  Description:  Creates a directory at the specified path.

  Arguments:    path: Path name
                permit: Directory access permissions (rwx){1,3}

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateDirectory(const char *path, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteDirectory

  Description:  Deletes the specified directory.

  Arguments:    path: Path name

  Returns:      TRUE if it was deleted successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteDirectory(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameDirectory

  Description:  Renames the specified directory.

  Arguments:    path: Path name
                newpath: New path name

  Returns:      TRUE if it was renamed successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameDirectory(const char *path, const char *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenFile

  Description:  Opens the file at the specified path.

  Arguments:    path: Path name
                mode: File access mode ("r/w/r+/w+" and "b/t")

  Returns:      The opened file handle or NULL.
 *---------------------------------------------------------------------------*/
FATFSFileHandle FATFS_OpenFile(const char *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CloseFile

  Description:  Closes the file.

  Arguments:    file: File handle

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_CloseFile(FATFSFileHandle file);

/*---------------------------------------------------------------------------*
  Name:         FATFS_ReadFile

  Description:  Reads data from the file.

  Arguments:    file: File handle
                buffer: Location to store the data that is read
                length: Data size to read

  Returns:      The data size that was actually read or -1.
 *---------------------------------------------------------------------------*/
int FATFS_ReadFile(FATFSFileHandle file, void *buffer, int length);

/*---------------------------------------------------------------------------*
  Name:         FATFS_WriteFile

  Description:  Writes data to the file.

  Arguments:    file: File handle
                buffer: Location storing the data to write
                length: Data size to write

  Returns:      The data size that was actually written or -1.
 *---------------------------------------------------------------------------*/
int FATFS_WriteFile(FATFSFileHandle file, const void *buffer, int length);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetSeekCache

  Description:  Assigns the cache buffer for a fast reverse seek.

  Arguments:    file: File handle
                buf: Cache buffer
                buf_size: Cache buffer size

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetSeekCache(FATFSFileHandle file, void* buf, u32 buf_size);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SeekFile

  Description:  Moves the file pointer.

  Arguments:    file: File handle
                offset: Distance to move
                origin: Starting point to move from

  Returns:      The shifted offset or -1.
 *---------------------------------------------------------------------------*/
int FATFS_SeekFile(FATFSFileHandle file, int offset, FATFSSeekMode origin);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FlushFile

  Description:  Flushes file content to the drive.

  Arguments:    file: File handle

  Returns:      TRUE if it was flushed normally.
 *---------------------------------------------------------------------------*/
BOOL FATFS_FlushFile(FATFSFileHandle file);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetFileLength

  Description:  Gets file size.

  Arguments:    file: File handle

  Returns:      The file size or -1.
 *---------------------------------------------------------------------------*/
int FATFS_GetFileLength(FATFSFileHandle file);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetFileLength

  Description:  Sets the file size.

  Arguments:    file: File handle

  Returns:      TRUE if it was set successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetFileLength(FATFSFileHandle file, int length);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenDirectory

  Description:  Opens the directory at the specified path.

  Arguments:    path: Path name
                mode: Directory access mode (this is currently ignored)

  Returns:      The opened directory handle or NULL.
 *---------------------------------------------------------------------------*/
FATFSDirectoryHandle FATFS_OpenDirectory(const char *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CloseDirectory

  Description:  Closes a directory.

  Arguments:    dir: Directory handle

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_CloseDirectory(FATFSDirectoryHandle dir);

/*---------------------------------------------------------------------------*
  Name:         FATFS_ReadDirectory

  Description:  Reads the next entry information from a directory.

  Arguments:    dir: Directory handle
                info: Location to store the entry information

  Returns:      TRUE if the entry information was read normally.
 *---------------------------------------------------------------------------*/
BOOL FATFS_ReadDirectory(FATFSDirectoryHandle dir, FATFSFileInfo *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FlushAll

  Description:  Flushes all drive content to the device.

  Arguments:    None.

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_FlushAll(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_UnmountAll

  Description:  Unmounts all drives.

  Arguments:    None.

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_UnmountAll(void);

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountSpecial

  Description:  Mounts a special drive.

  Arguments:    param: Parameter that specifies the mounting target
                arcname: Archive name to mount
                          You can specify "otherPub", "otherPrv", and "share".
                          If NULL is specified, previously mounted drives will be unmounted
                slot: Buffer to store the assigned index.
                          This will be set when mounting and accessed when unmounting.

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountSpecial(u64 param, const char *arcname, int *slot);

/*---------------------------------------------------------------------------*
  Name:         FATFS_FormatSpecial

  Description:  Re-initializes a special drive owned by the caller of this function.

  Arguments:    path: Path including the drive name

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_FormatSpecial(const char *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetLatencyEmulation

  Description:  Indicates that the drive layer should emulate the wait times that occur when accessing a deteriorated device.
                

  Arguments:    enable: TRUE to enable this.

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetLatencyEmulation(BOOL enable);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SearchWildcard

  Description:  Runs a wildcard search within a specified SD Card directory.

  Arguments:    directory: Arbitrary directory name of no more than 8 characters.
                           NULL to write files directly under the application's root directory.
                            
                prefix: Filename prefix (1-5 characters)
                suffix: Name of the file extension (1-3 characters)
                buffer: Buffer in which to record existing files that match the conditions.
                        If (buffer[i / 8] & (1 << (i % 8))) is TRUE, it indicates that "prefix{i}.suffix" exists.
                            
                length: Length of the buffer

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_SearchWildcard(const char* directory, const char *prefix, const char *suffix,
                          void *buffer, u32 length);

/*---------------------------------------------------------------------------*
 * Versions with Unicode support
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetDriveResourceW

  Description:  Gets the amount of free disk space.

  Arguments:    path: Path, including the drive name
                resource: FATFSDriveResource structure to store the result

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetDriveResourceW(const u16 *path, FATFSDriveResource *resource);

/*---------------------------------------------------------------------------*
  Name:         FATFS_GetFileInfoW

  Description:  Gets information for a file or directory.

  Arguments:    path: Path name
                info: FATFSFileInfoW structure to store the result

  Returns:      Gets information and returns TRUE if the specified path exists; otherwise, returns FALSE.
 *---------------------------------------------------------------------------*/
BOOL FATFS_GetFileInfoW(const u16 *path, FATFSFileInfoW *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_SetFileInfoW

  Description:  Changes file or directory information.

  Arguments:    path: Path name
                info: FATFSFileInfoW structure storing the information that should be changed

  Returns:      Changes information and returns TRUE if the specified path exists; otherwise, returns FALSE.
 *---------------------------------------------------------------------------*/
BOOL FATFS_SetFileInfoW(const u16 *path, const FATFSFileInfoW *info);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateFileW

  Description:  Creates a file at the specified path.

  Arguments:    path: Path name
                trunc: TRUE to delete the file if it already exists
                permit: File access permissions (rwx){1,3}

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateFileW(const u16 *path, BOOL trunc, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteFileW

  Description:  Deletes the specified file.

  Arguments:    path: Path name

  Returns:      TRUE if it was deleted successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteFileW(const u16 *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameFileW

  Description:  Renames the specified file.

  Arguments:    path: Path name
                newpath: New path name

  Returns:      TRUE if it was renamed successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameFileW(const u16 *path, const u16 *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_CreateDirectoryW

  Description:  Creates a directory at the specified path.

  Arguments:    path: Path name
                permit: File access permissions (rwx){1,3}

  Returns:      TRUE if the process is successful.
 *---------------------------------------------------------------------------*/
BOOL FATFS_CreateDirectoryW(const u16 *path, const char *permit);

/*---------------------------------------------------------------------------*
  Name:         FATFS_DeleteDirectoryW

  Description:  Deletes the specified directory.

  Arguments:    path: Path name

  Returns:      TRUE if it was deleted successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_DeleteDirectoryW(const u16 *path);

/*---------------------------------------------------------------------------*
  Name:         FATFS_RenameDirectoryW

  Description:  Renames the specified directory.

  Arguments:    path: Path name
                newpath: New path name

  Returns:      TRUE if it was renamed successfully.
 *---------------------------------------------------------------------------*/
BOOL FATFS_RenameDirectoryW(const u16 *path, const u16 *newpath);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenFileW

  Description:  Opens the file at the specified path.

  Arguments:    path: Path name
                mode: File access mode ("r/w/r+/w+" and "b/t")

  Returns:      The opened file handle or NULL.
 *---------------------------------------------------------------------------*/
FATFSFileHandle FATFS_OpenFileW(const u16 *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenDirectoryW

  Description:  Opens the directory at the specified path.

  Arguments:    path: Path name
                mode: Directory access mode (this is currently ignored)

  Returns:      The opened directory handle or NULL.
 *---------------------------------------------------------------------------*/
FATFSDirectoryHandle FATFS_OpenDirectoryW(const u16 *path, const char *mode);

/*---------------------------------------------------------------------------*
  Name:         FATFS_ReadDirectoryW

  Description:  Reads the next entry information from a directory.

  Arguments:    dir: Directory handle
                info: Location to store the entry information

  Returns:      TRUE if the entry information was read normally.
 *---------------------------------------------------------------------------*/
BOOL FATFS_ReadDirectoryW(FATFSDirectoryHandle dir, FATFSFileInfoW *info);


/*---------------------------------------------------------------------------*
 * Internal functions
 *---------------------------------------------------------------------------*

 /*---------------------------------------------------------------------------*
  Name:         FATFSiArcnameList

  Description:  This static buffer maintains a list of archive names that are accessible from the ARM9.
                It must be memory that can be referenced by both ARM9/ARM7, and by default a static variable in the LTDMAIN segment is used.
                For applications constructed using a special memory layout, you can change these variables to set the appropriate buffer before calling the FATFS_Init function.
                
                
                

  Arguments:    None.

  Returns:      A static buffer that maintains a list of archive names that are accessible from the ARM9.
 *---------------------------------------------------------------------------*/
extern char *FATFSiArcnameList/* [MATH_ROUNDUP(OS_MOUNT_ARCHIVE_NAME_LEN * OS_MOUNT_INFO_MAX + 1, 32)] ATTRIBUTE_ALIGN(32) */;

/*---------------------------------------------------------------------------*
  Name:         FATFSiCommandBuffer

  Description:  This is a pointer to a static command buffer for issuing requests.
                It must be memory that can be referenced by both ARM9/ARM7, and by default a static variable in the LTDMAIN segment is used.
                For applications constructed using a special memory layout, you can change these variables to set the appropriate buffer before calling the FATFS_Init function.
                
                
                

  Arguments:    None.

  Returns:      Static command buffer of FATFS_COMMAND_BUFFER_MAX bytes.
 *---------------------------------------------------------------------------*/
extern u8 *FATFSiCommandBuffer/* [FATFS_COMMAND_BUFFER_MAX] ATTRIBUTE_ALIGN(32)*/;

/*---------------------------------------------------------------------------*
  Name:         FATFSi_GetUnicodeConversionTable

  Description:  Gets the Unicode conversion table for sharing with the ARM7.
                You can override and disable this function to delete the (slightly less than) 80KB Unicode conversion table for applications that will clearly handle only ASCII path names.
                
                

  Arguments:    u2s: Conversion table from Unicode to Shift_JIS
                s2u: Conversion table from Shift_JIS to Unicode

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFSi_GetUnicodeConversionTable(const u8 **u2s, const u16 **s2u);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_SetNdmaParameters

  Description:  Directly changes the NDMA settings used by the ARM7's FAT drives.

  Arguments:    ndmaNo: NDMA channel (0-3) for which to change the settings
                blockWord: Block transfer word count (MI_NDMA_BWORD_1-MI_NDMA_BWORD_32768)
                intervalTimer: Interval (0x0000-0xFFFF)
                prescaler: Prescaler (MI_NDMA_INTERVAL_PS_1-MI_NDMA_INTERVAL_PS_64)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFSi_SetNdmaParameters(u32 ndmaNo, u32 blockWord, u32 intervalTimer, u32 prescaler);

/*---------------------------------------------------------------------------*
  Name:         FATFSi_SetRequestBuffer

  Description:  Configures a request buffer for changing the next command to be issued into an asynchronous process.
                

  Arguments:    buffer: FATFSRequestBuffer structure to use for request management.
                        It must be aligned to boundaries at integer multiples of 32 bytes.

                callback: Callback to invoke when the command has completed

                userdata: Arbitrary user-defined value to associate with the request buffer

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FATFSi_SetRequestBuffer(FATFSRequestBuffer *buffer, void (*callback)(FATFSRequestBuffer *), void *userdata);


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_FATFS_API_H_ */
#endif
