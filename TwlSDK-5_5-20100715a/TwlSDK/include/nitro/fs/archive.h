/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     archive.h

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


#ifndef NITRO_FS_ARCHIVE_H_
#define NITRO_FS_ARCHIVE_H_


#include <nitro/fs/types.h>
#include <nitro/fs/romfat.h>
#include <nitro/os/common/thread.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// Archive command interface
typedef struct FSArchiveInterface
{
    // Commands that are compatible with the old specifications
    FSResult (*ReadFile)(struct FSArchive*, struct FSFile*, void *buffer, u32 *length);
    FSResult (*WriteFile)(struct FSArchive*, struct FSFile*, const void *buffer, u32 *length);
    FSResult (*SeekDirectory)(struct FSArchive*, struct FSFile*, u32 id, u32 position);
    FSResult (*ReadDirectory)(struct FSArchive*, struct FSFile*, FSDirectoryEntryInfo *info);
    FSResult (*FindPath)(struct FSArchive*, u32 base_dir_id, const char *path, u32 *target_id, BOOL target_is_directory);
    FSResult (*GetPath)(struct FSArchive*, struct FSFile*, BOOL is_directory, char *buffer, u32 *length);
    FSResult (*OpenFileFast)(struct FSArchive*, struct FSFile*, u32 id, u32 mode);
    FSResult (*OpenFileDirect)(struct FSArchive*, struct FSFile*, u32 top, u32 bottom, u32 *id);
    FSResult (*CloseFile)(struct FSArchive*, struct FSFile*);
    void (*Activate)(struct FSArchive*);
    void (*Idle)(struct FSArchive*);
    void (*Suspend)(struct FSArchive*);
    void (*Resume)(struct FSArchive*);
    // New commands that are compatible with the old specifications
    FSResult (*OpenFile)(struct FSArchive*, struct FSFile*, u32 base_dir_id, const char *path, u32 mode);
    FSResult (*SeekFile)(struct FSArchive*, struct FSFile*, int *offset, FSSeekFileMode from);
    FSResult (*GetFileLength)(struct FSArchive*, struct FSFile*, u32 *length);
    FSResult (*GetFilePosition)(struct FSArchive*, struct FSFile*, u32 *position);
    // Extended commands in the new specifications
    void (*Mount)(struct FSArchive*);
    void (*Unmount)(struct FSArchive*);
    FSResult (*GetArchiveCaps)(struct FSArchive*, u32 *caps);
    FSResult (*CreateFile)(struct FSArchive*, u32 baseid, const char *relpath, u32 permit);
    FSResult (*DeleteFile)(struct FSArchive*, u32 baseid, const char *relpath);
    FSResult (*RenameFile)(struct FSArchive*, u32 baseid_src, const char *relpath_src, u32 baseid_dst, const char *relpath_dst);
    FSResult (*GetPathInfo)(struct FSArchive*, u32 baseid, const char *relpath, FSPathInfo *info);
    FSResult (*SetPathInfo)(struct FSArchive*, u32 baseid, const char *relpath, FSPathInfo *info);
    FSResult (*CreateDirectory)(struct FSArchive*, u32 baseid, const char *relpath, u32 permit);
    FSResult (*DeleteDirectory)(struct FSArchive*, u32 baseid, const char *relpath);
    FSResult (*RenameDirectory)(struct FSArchive*, u32 baseid, const char *relpath_src, u32 baseid_dst, const char *relpath_dst);
    FSResult (*GetArchiveResource)(struct FSArchive*, FSArchiveResource *resource);
    void   *unused_29;
    FSResult (*FlushFile)(struct FSArchive*, struct FSFile*);
    FSResult (*SetFileLength)(struct FSArchive*, struct FSFile*, u32 length);
    FSResult (*OpenDirectory)(struct FSArchive*, struct FSFile*, u32 base_dir_id, const char *path, u32 mode);
    FSResult (*CloseDirectory)(struct FSArchive*, struct FSFile*);
    FSResult (*SetSeekCache)(struct FSArchive*, struct FSFile*, void* buf, u32 buf_size);
    // Reserved for future extensions
    u8      reserved[116];
}
FSArchiveInterface;

SDK_COMPILER_ASSERT(sizeof(FSArchiveInterface) == 256);


typedef struct FSArchive
{
// private:

    // A unique name to identify the archive.
    // It uses 1-3 alphanumeric characters and is case insensitive.
    union
    {
        char    ptr[FS_ARCHIVE_NAME_LEN_MAX + 1];
        u32     pack;
    }
    name;
    struct FSArchive   *next;       // Archive registration list
    struct FSFile      *list;       // Process wait command list
    OSThreadQueue       queue;      // General-purpose queue to wait for events
    u32                 flag;       // Internal status flags (FS_ARCHIVE_FLAG_*)
    FSCommandType       command;    // The most recent command
    FSResult            result;     // The most recent processing result
    void               *userdata;   // User-defined pointer
    const FSArchiveInterface *vtbl; // Command interface

    union
    {
        // Data that can be freely used in user-defined archives
        u8      reserved2[52];
        // Data to use in ROMFAT archives (maintained for compatibility)
        struct  FS_ROMFAT_CONTEXT_DEFINITION();
    };
}
FSArchive;

// NITRO-SDK compatibility is taken seriously; at the moment, it strictly forbidden to change the size.
SDK_COMPILER_ASSERT(sizeof(FSArchive) == 92);


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         FS_FindArchive

  Description:  Searches for an archive name.
                Returns NULL if there is no matching name.

  Arguments:    name: String with the archive name to search for
                name_len: Length of the 'name' string

  Returns:      Pointer to the archive that was found, or NULL.
 *---------------------------------------------------------------------------*/
FSArchive *FS_FindArchive(const char *name, int name_len);

/*---------------------------------------------------------------------------*
  Name:         FS_NormalizePath

  Description:  Normalizes the specified path name with the current directory, as follows: (archive pointer) + (base directory ID) + (path name).
                
                

  Arguments:    path:        Non-normalized path string.
                baseid: Location to store the base directory ID, or NULL
                relpath: Location to store the converted path name, or NULL

  Returns:      An archive pointer or NULL.
 *---------------------------------------------------------------------------*/
FSArchive* FS_NormalizePath(const char *path, u32 *baseid, char *relpath);

/*---------------------------------------------------------------------------*
  Name:         FS_GetCurrentDirectory

  Description:  Gets the current directory as a path name.

  Arguments:    None.

  Returns:      String that indicates the current directory.
 *---------------------------------------------------------------------------*/
const char *FS_GetCurrentDirectory(void);

/*---------------------------------------------------------------------------*
  Name:         FS_GetLastArchiveCommand

  Description:  Gets the most recent command processed by an archive.

  Arguments:    arc: FSArchive structure

  Returns:      The result of the most recent command processed by the archive.
 *---------------------------------------------------------------------------*/
SDK_INLINE FSCommandType FS_GetLastArchiveCommand(const FSArchive *arc)
{
    return arc->command;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResultCode

  Description:  Gets the most recent error code for the specified archive.

  Arguments:    path_or_archive: The FSArchive structure or path string that indicates the target archive.
                                 

  Returns:      The most recent error code for the specified archive.
                FS_RESULT_ERROR when the applicable archive does not exist.
 *---------------------------------------------------------------------------*/
FSResult FS_GetArchiveResultCode(const void *path_or_archive);

/*---------------------------------------------------------------------------*
  Name:         FSi_EndArchive

  Description:  Shuts down and releases all archives.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FSi_EndArchive(void);

/*---------------------------------------------------------------------------*
  Name:         FS_InitArchive

  Description:  Initialize archive structure

  Arguments:    arc: Archive to initialize.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_InitArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveName

  Description:  Acquire archive name

  Arguments:    arc: Archive from which to get a name

  Returns:      The archive name registered in the file system.
 *---------------------------------------------------------------------------*/
const char *FS_GetArchiveName(const FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveLoaded

  Description:  Determines whether the archive is completely loaded into the the current file system

  Arguments:    arc: Archive to check

  Returns:      TRUE if it has already been loaded into the file system.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsArchiveLoaded(volatile const FSArchive *arc)
{
    return ((arc->flag & FS_ARCHIVE_FLAG_LOADED) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveSuspended

  Description:  Determines whether the archive is currently suspended.

  Arguments:    arc: Archive to check

  Returns:      TRUE if it is currently suspended.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FS_IsArchiveSuspended(volatile const FSArchive *arc)
{
    return ((arc->flag & FS_ARCHIVE_FLAG_SUSPEND) != 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveUserData

  Description:  Gets the user-defined pointer associated with an archive.

  Arguments:    arc: FSArchive structure

  Returns:      User-defined pointer
 *---------------------------------------------------------------------------*/
SDK_INLINE void* FS_GetArchiveUserData(const FSArchive *arc)
{
    return arc->userdata;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RegisterArchiveName

  Description:  Registers and associates an archive name with the file system.
                The archive itself is still not loaded into the file system.
                The archive name "rom" is reserved by the file system.

  Arguments:    arc: Archive to associate with the name
                name: String of the name to register
                name_len: Length of the 'name' string

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL    FS_RegisterArchiveName(FSArchive *arc, const char *name, u32 name_len);

/*---------------------------------------------------------------------------*
  Name:         FS_ReleaseArchiveName

  Description:  Releases a registered archive name.

  Arguments:    arc: Archive with a name to release

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_ReleaseArchiveName(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_MountArchive

  Description:  Mounts an archive.

  Arguments:    arc: Archive to mount
                userdata: User-defined variable to associate with the archive
                vtbl: Command interface
                reserved: Reserved for future use (always specify 0)


  Returns:      TRUE if it was mounted successfully.
 *---------------------------------------------------------------------------*/
BOOL    FS_MountArchive(FSArchive *arc, void *userdata,
                        const FSArchiveInterface *vtbl, u32 reserved);

/*---------------------------------------------------------------------------*
  Name:         FS_UnmountArchive

  Description:  Unmounts an archive.

  Arguments:    arc: Archive to unmount

  Returns:      TRUE if it was unmounted successfully.
 *---------------------------------------------------------------------------*/
BOOL    FS_UnmountArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_SuspendArchive

  Description:  Stops the archive processing mechanism itself.
                This will wait for running processes to complete.

  Arguments:    arc: Archive to stop

  Returns:      TRUE if it was not suspended before the function call.
 *---------------------------------------------------------------------------*/
BOOL    FS_SuspendArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_ResumeArchive

  Description:  Resumes suspended archive processing.

  Arguments:    arc: Archive to resume

  Returns:      TRUE if it was not suspended before the function call.
 *---------------------------------------------------------------------------*/
BOOL    FS_ResumeArchive(FSArchive *arc);

/*---------------------------------------------------------------------------*
  Name:         FS_NotifyArchiveAsyncEnd

  Description:  This is called from the archive implementation to send a notification when asynchronous archive processing is complete.
                

  Arguments:    arc: Archive for which to send a notification of completion
                ret: Processing result

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    FS_NotifyArchiveAsyncEnd(FSArchive *arc, FSResult ret);


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_ARCHIVE_H_ */
