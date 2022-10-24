/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_file.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-07-12#$
  $Rev: 11362 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/


#include <nitro/types.h>
#include <nitro/misc.h>
#include <nitro/mi.h>
#include <nitro/os.h>
#include <nitro/pxi.h>
#include <nitro/std/string.h>
#include <nitro/std/unicode.h>
#include <nitro/math/math.h>

#include <nitro/fs.h>

#include "../include/rom.h"
#include "../include/util.h"
#include "../include/command.h"


#define FS_DEBUG_TRACE(...) (void)0
//#define FS_DEBUG_TRACE  OS_TPrintf


/*---------------------------------------------------------------------------*/
/* Functions */

#if defined(FS_IMPLEMENT)

/*---------------------------------------------------------------------------*
  Name:         FSi_IsValidTransferRegion

  Description:  Simple determination whether buffer used in read/write operations is in an unsafe range.

  Arguments:    buffer: Buffer targeted for transfer
                length: Transfer size

  Returns:      pos is either one returned character from the referenced position or -1.
 *---------------------------------------------------------------------------*/
static BOOL FSi_IsValidTransferRegion(const void *buffer, s32 length)
{
    BOOL    retval = FALSE;
    if (buffer == NULL)
    {
        OS_TWarning("specified transfer buffer is NULL.\n");
    }
    else if (((u32)buffer >= HW_IOREG) && ((u32)buffer < HW_IOREG_END))
    {
        OS_TWarning("specified transfer buffer is in I/O register %08X. (seems to be dangerous)\n", buffer);
    }
    else if (length < 0)
    {
        OS_TWarning("specified transfer size is minus. (%d)\n", length);
    }
    else
    {
#if !defined(SDK_TWL)
        s32     mainmem_size = HW_MAIN_MEM_EX_SIZE;
#else
        s32     mainmem_size = OS_IsRunOnTwl() ? HW_TWL_MAIN_MEM_EX_SIZE : HW_MAIN_MEM_EX_SIZE;
#endif
        if (length > mainmem_size)
        {
            OS_TWarning("specified transfer size is over mainmemory-size. (%d)\n", length);
        }
        else
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPosition

  Description:  Returns one character from the Shift_JIS string reference position.

  Arguments:    str: Pointer indicating the start of the Shift_JIS string
                pos: Current string reference position (in bytes)

  Returns:      pos is either one returned character from the referenced position or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPosition(const char *str, int pos)
{
    // First, go back by just a one-byte amount
    int     prev = --pos;
    // Shift_JIS uses single bytes or successive bytes to separate chars. The leading and subsequent bytes share part of their mapping. The char type cannot be determined while that position appears to be the leading byte, so focus goes back farther
    // 
    // 
    for (; (prev > 0) && STD_IsSjisLeadByte(str[prev - 1]); --prev)
    {
    }
    // When there is a series of double-byte characters with unclear subsequent bytes, such as "===b," this goes back too far by a multiple of 2, so ignore it.
    // (Take a remainder of 2)
    return pos - ((pos - prev) & 1);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IncrementSjisPositionToSlash

  Description:  Advances reference position of Shift_JIS string to either directory delimiter or end.
                

  Arguments:    str: Pointer indicating the start of the Shift_JIS string
                pos: Current string reference position (in bytes)

  Returns:      Either first directory delimiter that appears after pos or the end position.
 *---------------------------------------------------------------------------*/
int FSi_IncrementSjisPositionToSlash(const char *str, int pos)
{
    while (str[pos] && !FSi_IsSlash((u8)str[pos]))
    {
        pos = FSi_IncrementSjisPosition(str, pos);
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPositionToSlash

  Description:  Return reference position of Shift_JIS string to either directory separator character or beginning.
                

  Arguments:    str: Pointer indicating the start of the Shift_JIS string
                pos: Current string reference position (in bytes)

  Returns:      Either first directory delimiter that appears before pos or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPositionToSlash(const char *str, int pos)
{
    for (;;)
    {
        pos = FSi_DecrementSjisPosition(str, pos);
        if ((pos < 0) || FSi_IsSlash((u8)str[pos]))
        {
            break;
        }
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_TrimSjisTrailingSlash

  Description:  Deletes if end of Shift_JIS string is a directory delimiter.

  Arguments:    str: Shift_JIS string

  Returns:      Length of string.
 *---------------------------------------------------------------------------*/
int FSi_TrimSjisTrailingSlash(char *str)
{
    int     length = STD_GetStringLength(str);
    int     lastpos = FSi_DecrementSjisPosition(str, length);
    if ((lastpos >= 0) && FSi_IsSlash((u8)str[lastpos]))
    {
        length = lastpos;
        str[length] = '\0';
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePosition

  Description:  Decrements Unicode string reference position by one character.

  Arguments:    str: Pointer indicating the start of the Unicode string
                pos: Current string reference position (in bytes)

  Returns:      pos is either one character back from the referenced position or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePosition(const u16 *str, int pos)
{
    // First, securely return the amount of 1 character
    int     prev = --pos;
    // Return one more character if valid surrogate pair
    if ((pos > 0) &&
        ((str[pos - 1] >= 0xD800) && (str[pos - 1] <= 0xDC00)) &&
        ((str[pos - 0] >= 0xDC00) && (str[pos - 0] <= 0xE000)))
    {
        --pos;
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePositionToSlash

  Description:  Decrements the reference position of Unicode string to either directory delimiter or beginning.
                

  Arguments:    str: Pointer indicating the start of the Unicode string
                pos: Current string reference position (in bytes)

  Returns:      Either first directory delimiter that appears before pos or -1.
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePositionToSlash(const u16 *str, int pos)
{
    for (;;)
    {
        pos = FSi_DecrementUnicodePosition(str, pos);
        if ((pos < 0) || FSi_IsUnicodeSlash(str[pos]))
        {
            break;
        }
    }
    return pos;
}

/*---------------------------------------------------------------------------*
  Name:         FS_InitFile

  Description:  Initializes the FSFile structure.

  Arguments:    file: FSFile structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_InitFile(FSFile *file)
{
    SDK_NULL_ASSERT(file);
    {
        file->arc = NULL;
        file->userdata = NULL;
        file->next = NULL;
        OS_InitThreadQueue(file->queue);
        file->stat = 0;
        file->stat |= (FS_COMMAND_INVALID << FS_FILE_STATUS_CMD_SHIFT);
        file->argument = NULL;
        file->error = FS_RESULT_SUCCESS;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_CancelFile

  Description:  Requests cancellation of an asynchronous command.

  Arguments:    file: File handle

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_CancelFile(FSFile *file)
{
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if (FS_IsBusy(file))
        {
            file->stat |= FS_FILE_STATUS_CANCEL;
            file->arc->flag |= FS_ARCHIVE_FLAG_CANCELING;
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFile

  Description:  Generates files.

  Arguments:    path: Path name
                mode: Access mode

  Returns:      TRUE if file is generated normally.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFile(const char *path, u32 permit)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                  file[1];
            FSArgumentForCreateFile arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->permit = permit;
            retval = FSi_SendCommand(file, FS_COMMAND_CREATEFILE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFile

  Description:  Deletes files.

  Arguments:    path: Path name

  Returns:      TRUE if file is deleted normally.
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteFile(const char *path)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                  file[1];
            FSArgumentForDeleteFile arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            retval = FSi_SendCommand(file, FS_COMMAND_DELETEFILE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFile

  Description:  Changes the filename.

  Arguments:    src: Filename of the conversion source
                dst: Filename of the conversion target

  Returns:      TRUE if filename is changed normally.
 *---------------------------------------------------------------------------*/
BOOL FS_RenameFile(const char *src, const char *dst)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__, src, dst);
    SDK_NULL_ASSERT(src);
    SDK_NULL_ASSERT(dst);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath_src[FS_ARCHIVE_FULLPATH_MAX + 1];
        char        relpath_dst[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid_src = 0;
        u32         baseid_dst = 0;
        FSArchive  *arc_src = FS_NormalizePath(src, &baseid_src, relpath_src);
        FSArchive  *arc_dst = FS_NormalizePath(dst, &baseid_dst, relpath_dst);
        if (arc_src != arc_dst)
        {
            OS_TWarning("cannot rename between defferent archives.\n");
        }
        else
        {
            FSFile                  file[1];
            FSArgumentForRenameFile arg[1];
            FS_InitFile(file);
            file->arc = arc_src;
            file->argument = arg;
            arg->baseid_src = baseid_src;
            arg->relpath_src = relpath_src;
            arg->baseid_dst = baseid_dst;
            arg->relpath_dst = relpath_dst;
            retval = FSi_SendCommand(file, FS_COMMAND_RENAMEFILE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathInfo

  Description:  Gets file or directory information.

  Arguments:    path: Path name
                info: Location to store the information

  Returns:      Processing result.
 *---------------------------------------------------------------------------*/
BOOL FS_GetPathInfo(const char *path, FSPathInfo *info)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                      file[1];
            FSArgumentForGetPathInfo    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->info = info;
            retval = FSi_SendCommand(file, FS_COMMAND_GETPATHINFO, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetPathInfo

  Description:  Sets file information.

  Arguments:    path: Path name
                info: Location to store the information

  Returns:      Processing result.
 *---------------------------------------------------------------------------*/
BOOL FS_SetPathInfo(const char *path, const FSPathInfo *info)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                      file[1];
            FSArgumentForSetPathInfo    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->info = (FSPathInfo*)info; //To clear FATFS_PROPERTY_CTRL_MASK in info->attributes
            retval = FSi_SendCommand(file, FS_COMMAND_SETPATHINFO, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectory

  Description:  Generates a directory.

  Arguments:    path: Path name
                mode: Access mode

  Returns:      TRUE if directory is generated normally.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateDirectory(const char *path, u32 permit)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                          file[1];
            FSArgumentForCreateDirectory    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->permit = permit;
            retval = FSi_SendCommand(file, FS_COMMAND_CREATEDIRECTORY, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectory

  Description:  Deletes the directory.

  Arguments:    path: Path name

  Returns:      TRUE if directory is deleted normally.
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteDirectory(const char *path)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile                          file[1];
            FSArgumentForDeleteDirectory    arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            retval = FSi_SendCommand(file, FS_COMMAND_DELETEDIRECTORY, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectory

  Description:  Changes the directory name.

  Arguments:    src: Directory name of the conversion source
                dst: Directory name of the conversion destination

  Returns:      TRUE if directory name is changed normally.
 *---------------------------------------------------------------------------*/
BOOL FS_RenameDirectory(const char *src, const char *dst)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__, src, dst);
    SDK_NULL_ASSERT(src);
    SDK_NULL_ASSERT(dst);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath_src[FS_ARCHIVE_FULLPATH_MAX + 1];
        char        relpath_dst[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid_src = 0;
        u32         baseid_dst = 0;
        FSArchive  *arc_src = FS_NormalizePath(src, &baseid_src, relpath_src);
        FSArchive  *arc_dst = FS_NormalizePath(dst, &baseid_dst, relpath_dst);
        if (arc_src != arc_dst)
        {
            OS_TWarning("cannot rename between defferent archives.\n");
        }
        else
        {
            FSFile                          file[1];
            FSArgumentForRenameDirectory    arg[1];
            FS_InitFile(file);
            file->arc = arc_src;
            file->argument = arg;
            arg->baseid_src = baseid_src;
            arg->relpath_src = relpath_src;
            arg->baseid_dst = baseid_dst;
            arg->relpath_dst = relpath_dst;
            retval = FSi_SendCommand(file, FS_COMMAND_RENAMEDIRECTORY, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFullPath

  Description:  Converts specified path to full path.

  Arguments:    dst: Buffer that stores obtained full path
                            Must be FS_ARCHIVE_FULLPATH_MAX+1 or higher
                path: Path name of file or directory

  Returns:      TRUE if full path was obtained normally.
 *---------------------------------------------------------------------------*/
static BOOL FSi_GetFullPath(char *dst, const char *path)
{
    FSArchive  *arc = FS_NormalizePath(path, NULL, dst);
    if (arc)
    {
        const char *arcname = FS_GetArchiveName(arc);
        int     m = STD_GetStringLength(arcname);
        int     n = STD_GetStringLength(dst);
        (void)STD_MoveMemory(&dst[m + 2], &dst[0], (u32)n + 1);
        (void)STD_MoveMemory(&dst[0], arcname, (u32)m);
        dst[m + 0] = ':';
        dst[m + 1] = '/';
    }
    return (arc != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ComplementDirectory

  Description:  Checks the existence of all parent directories up to the specified path; automatically generates and fills in missing layers.
                

  Arguments:    path: Path name of file or directory
                            Fill in a directory one level up from this
                autogen: 	Buffer that records the highest directory that was generated automatically
                            Must be FS_ARCHIVE_FULLPATH_MAX+1 or higher
                            Results are returned to autogen regardless of their success or failure; if there is a blank string, it indicates that all results exist.
                            

  Returns:      TRUE if directory is generated normally.
 *---------------------------------------------------------------------------*/
static BOOL FSi_ComplementDirectory(const char *path, char *autogen)
{
    BOOL    retval = FALSE;
    int     root = 0;
    // Normalize full path name for now. (Reuse autogen to conserve the stack)
    char   *tmppath = autogen;
    if (FSi_GetFullPath(tmppath, path))
    {
        int     length = STD_GetStringLength(tmppath);
        if (length > 0)
        {
            int     pos = 0;
            FS_DEBUG_TRACE("  trying to complete \"%s\"\n", tmppath);
            // Delete the "/" at the end
            length = FSi_TrimSjisTrailingSlash(tmppath);
            // Ignore the lowest entry
            length = FSi_DecrementSjisPositionToSlash(tmppath, length);
            // Search the deepest level that exists
            for (pos = length; pos >= 0;)
            {
                FSPathInfo  info[1];
                BOOL        exists;
                tmppath[pos] = '\0';
                exists = FS_GetPathInfo(tmppath, info);
                FS_DEBUG_TRACE("    - \"%s\" is%s existent (result:%d)\n", tmppath, exists ? "" : " not",
                               FS_GetArchiveResultCode(tmppath));
                tmppath[pos] = '/';
                // If an entry does not exist, go to a higher level in hierarchy
                if (!exists)
                {
                    pos = FSi_DecrementSjisPositionToSlash(tmppath, pos);
                }
                // If it exists, stop the search for the time being
                else
                {
                    // If the same filename exists, certain failure
                    if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0)
                    {
                        pos = -1;
                    }
                    // If the same directory name exists, automatically generate below that level
                    else
                    {
                        ++pos;
                    }
                    break;
                }
            }
            // If a base point is called for, repeatedly generate one after that level
            if (pos >= 0)
            {
                for (;;)
                {
                    // If the end is reached, success
                    if (pos >= length)
                    {
                        retval = TRUE;
                        break;
                    }
                    else
                    {
                        pos = FSi_IncrementSjisPositionToSlash(tmppath, pos);
                        tmppath[pos] = '\0';
                        if (!FS_CreateDirectory(tmppath, FS_PERMIT_R | FS_PERMIT_W))
                        {
                            break;
                        }
                        else
                        {
                            // Store the highest level
                            if (root == 0)
                            {
                                FS_DEBUG_TRACE("    - we have created \"%s\" as root\n", tmppath);
                                root = pos;
                            }
                            tmppath[pos++] = '/';
                        }
                    }
                }
            }
        }
    }
    // Record the highest directory automatically generated
    autogen[root] = '\0';
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileAuto

  Description:  Generates files including the necessary intermediate directories.

  Arguments:    path: Path name
                permit: Access mode

  Returns:      TRUE if file is generated normally.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFileAuto(const char *path, u32 permit)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    if (FSi_ComplementDirectory(path, autogen))
    {
        result = FS_CreateFile(path, permit);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteFileAuto

  Description:  Deletes files including the necessary intermediate directories.

  Arguments:    path: Path name

  Returns:      TRUE if file is deleted normally.
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteFileAuto(const char *path)
{
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    // Exists for consistency in command names, but actually the fill-in process is unnecessary
    return FS_DeleteFile(path);
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameFileAuto

  Description:  Changes filenames including the necessary intermediate directories.

  Arguments:    src: Filename of the conversion source
                dst: Filename of the conversion target

  Returns:      TRUE if filename is changed normally.
 *---------------------------------------------------------------------------*/
BOOL FS_RenameFileAuto(const char *src, const char *dst)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__);
    if (FSi_ComplementDirectory(dst, autogen))
    {
        result = FS_RenameFile(src, dst);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateDirectoryAuto

  Description:  Generates directories including the necessary intermediate directories.

  Arguments:    path: Directory name to generate
                permit: Access mode

  Returns:      TRUE if directory is generated normally.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateDirectoryAuto(const char *path, u32 permit)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    if (FSi_ComplementDirectory(path, autogen))
    {
        result = FS_CreateDirectory(path, permit);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteDirectoryAuto

  Description:  Recursively deletes the directories.

  Arguments:    path: Path name

  Returns:      TRUE if directory is deleted normally.
 *---------------------------------------------------------------------------*/
BOOL FS_DeleteDirectoryAuto(const char *path)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s(%s)\n", __FUNCTION__, path);
    if (path && *path)
    {
        char    tmppath[FS_ARCHIVE_FULLPATH_MAX + 1];
        if (FSi_GetFullPath(tmppath, path))
        {
            int     pos;
            BOOL    mayBeEmpty;
            int     length = FSi_TrimSjisTrailingSlash(tmppath);
            FS_DEBUG_TRACE("  trying to force-delete \"%s\"\n", tmppath);
            mayBeEmpty = TRUE;
            for (pos = 0; pos >= 0;)
            {
                BOOL    failure = FALSE;
                // First, try to directly delete the directory and if successful, go 1 level up
                tmppath[length + pos] = '\0';
                if (mayBeEmpty && (FS_DeleteDirectory(tmppath) ||
                    (FS_GetArchiveResultCode(tmppath) == FS_RESULT_ALREADY_DONE)))
                {
                    FS_DEBUG_TRACE("  -> succeeded to delete \"%s\"\n", tmppath);
                    pos = FSi_DecrementSjisPositionToSlash(&tmppath[length], pos);
                }
                else
                {
                    // Enumerate all entries if the directory is to be opened
                    FSFile  dir[1];
                    FS_InitFile(dir);
                    if (!FS_OpenDirectory(dir, tmppath, FS_FILEMODE_R))
                    {
                        FS_DEBUG_TRACE("  -> failed to delete & open \"%s\"\n", tmppath);
                        failure = TRUE;
                    }
                    else
                    {
                        FSDirectoryEntryInfo    info[1];
                        tmppath[length + pos] = '/';
                        mayBeEmpty = TRUE;
                        while (FS_ReadDirectory(dir, info))
                        {
                            (void)STD_CopyString(&tmppath[length + pos + 1], info->longname);
                            // If the file exists, delete it
                            if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0)
                            {
                                if (!FS_DeleteFile(tmppath))
                                {
                                    FS_DEBUG_TRACE("  -> failed to delete file \"%s\"\n", tmppath);
                                    failure = TRUE;
                                    break;
                                }
                                FS_DEBUG_TRACE("  -> succeeded to delete \"%s\"\n", tmppath);
                            }
                            // If "." or ".." ignore
                            else if ((STD_CompareString(info->longname, ".") == 0) ||
                                     (STD_CompareString(info->longname, "..") == 0))
                            {
                            }
                            // If a non-empty directory exists, move to a lower level
                            else if (!FS_DeleteDirectory(tmppath))
                            {
                                pos += 1 + STD_GetStringLength(info->longname);
                                mayBeEmpty = FALSE;
                                break;
                            }
                        }
                        (void)FS_CloseDirectory(dir);
                    }
                }
                // Cancel processing when even operations that should succeed fail (such as ALREADY_DONE)
                if (failure)
                {
                    break;
                }
            }
            retval = (pos < 0);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RenameDirectoryAuto

  Description:  Changes the directory name by automatically generating the necessary intermediate directories.

  Arguments:    src: Directory name of the conversion source
                dst: Directory name of the conversion destination

  Returns:      TRUE if directory name is changed normally.
 *---------------------------------------------------------------------------*/
BOOL FS_RenameDirectoryAuto(const char *src, const char *dst)
{
    BOOL    result = FALSE;
    char    autogen[FS_ARCHIVE_FULLPATH_MAX + 1];
    FS_DEBUG_TRACE( "%s(%s->%s)\n", __FUNCTION__, src, dst);
    if (FSi_ComplementDirectory(dst, autogen))
    {
        result = FS_RenameDirectory(src, dst);
        if (!result)
        {
            (void)FS_DeleteDirectoryAuto(autogen);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResource

  Description:  Gets resource information of the specified archive.

  Arguments:    path: Path name that specifies the archive
                resource: Storage destination of retrieved resource information

  Returns:      TRUE if resource information is obtained successfully.
 *---------------------------------------------------------------------------*/
BOOL FS_GetArchiveResource(const char *path, FSArchiveResource *resource)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArchive  *arc = FS_NormalizePath(path, NULL, NULL);
        if (arc)
        {
            FSFile                          file[1];
            FSArgumentForGetArchiveResource arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->resource = resource;
            retval = FSi_SendCommand(file, FS_COMMAND_GETARCHIVERESOURCE, TRUE);
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetSpaceToCreateDirectoryEntries

  Description:  Estimates the capacity of the directory entry generated at the same time as the file.
                (Assumes that the directory that exists in the path is newly generated.)

  Arguments:    path: Path name of generated file
                bytesPerCluster: Number of bytes per cluster on file system

  Returns:      Capacity
 *---------------------------------------------------------------------------*/
u32 FSi_GetSpaceToCreateDirectoryEntries(const char *path, u32 bytesPerCluster)
{
    static const u32    bytesPerEntry = 32UL;
    static const u32    longnamePerEntry = 13UL;
    // Deletes the scheme if it is a full path and individually determines each entry
    const char         *root = STD_SearchString(path, ":");
    const char         *current = (root != NULL) ? (root + 1) : path;
    u32                 totalBytes = 0;
    u32                 restBytesInCluster = 0;
    current += (*current == '/');
    while (*current)
    {
        BOOL    isShortName = FALSE;
        u32     entries = 0;
        // Calculate entry name length
        u32     len = (u32)FSi_IncrementSjisPositionToSlash(current, 0);
        // Determine whether the entry name can be expressed in 8.3 format
#if 0
        // (FAT drivers employed for TWL are always saved with long filenames, so it was not necessary to strictly determine up to here)
        //  
        {
            static const char  *alnum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
            static const char  *special = "!#$%&'()*+-<>?@^_`{}~";
            if ((len <= 8 + 1 + 3) && STD_SearchChar(alnum, current[0]))
            {
                u32     namelen = 0;
                u32     extlen = 0;
                u32     scanned = 0;
                for (; namelen < len; ++namelen)
                {
                    char    c = current[scanned + namelen];
                    if (!STD_SearchChar(alnum, c) && !STD_SearchChar(special, c))
                    {
                        break;
                    }
                }
                scanned += namelen;
                if ((scanned < len) && (current[scanned] == '.'))
                {
                    ++scanned;
                    for (; scanned + extlen < len; ++extlen)
                    {
                        char    c = current[scanned + extlen];
                        if (!STD_SearchChar(alnum, c) && !STD_SearchChar(special, c))
                        {
                            break;
                        }
                    }
                    scanned += extlen;
                }
                if ((scanned == len) && (namelen <= 8) && (extlen <= 3))
                {
                    isShortName = TRUE;
                }
            }
        }
#endif
        // If it is not in 8.3 format, an added entry is needed for long filenames
        if (!isShortName)
        {
            entries += ((len + longnamePerEntry - 1UL) / longnamePerEntry);
        }
        // Anyway, 1 entry is always considered to be used
        entries += 1;
        current += len;
        // If the cluster margins of directories already created are inadequate, use excess entries in cluster units
        // 
        {
            int     over = (int)(entries * bytesPerEntry - restBytesInCluster);
            if (over > 0)
            {
                totalBytes += MATH_ROUNDUP(over, bytesPerCluster);
            }
        }
        // If there is still a lower level, use 1 cluster as a directory, and the size minus two entries of "." and ".." will be the margin
        // 
        if (*current != '\0')
        {
            current += 1;
            totalBytes += bytesPerCluster;
            restBytesInCluster = bytesPerCluster - (2 * bytesPerEntry);
        }
    }
    return totalBytes;
}

/*---------------------------------------------------------------------------*
  Name:         FS_HasEnoughSpaceToCreateFile

  Description:  Determines whether it is currently possible to generate files having the specified path name and size.

  Arguments:    resource: 	Archive information obtained by the FS_GetArchiveResource function in advance
                          	If the function is successful, the sizes used by the files are reduced
                path: 		Path name of generated file
                size: 		Size of generated file

  Returns:      TRUE if it is possible to generate files having the specified path name and size.
 *---------------------------------------------------------------------------*/
BOOL FS_HasEnoughSpaceToCreateFile(FSArchiveResource *resource, const char *path, u32 size)
{
    BOOL    retval = FALSE;
    u32 bytesPerCluster = resource->bytesPerSector * resource->sectorsPerCluster;
    if (bytesPerCluster != 0)
    {
        u32     needbytes = (FSi_GetSpaceToCreateDirectoryEntries(path, bytesPerCluster) +
                             MATH_ROUNDUP(size, bytesPerCluster));
        u32     needclusters = needbytes / bytesPerCluster;
        if (needclusters <= resource->availableClusters)
        {
            resource->availableClusters -= needclusters;
            resource->availableSize -= needbytes;
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_IsArchiveReady

  Description:  Determines whether it is currently possible to use the specified archive.

  Arguments:    path: Absolute path beginning with "archive name:"

  Returns:      TRUE if currently possible to use the specified archive name.
                FALSE if an SD card archive that is not inserted in the slot or a save data archive not yet imported.
                
 *---------------------------------------------------------------------------*/
BOOL FS_IsArchiveReady(const char *path)
{
    FSArchiveResource   resource[1];
    return FS_GetArchiveResource(path, resource);
}

/*---------------------------------------------------------------------------*
  Name:         FS_FlushFile

  Description:  Applies the file changes to the device.

  Arguments:    file: File handle

  Returns:      Processing result.
 *---------------------------------------------------------------------------*/
FSResult FS_FlushFile(FSFile *file)
{
    FSResult    retval = FS_RESULT_ERROR;
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        (void)FSi_SendCommand(file, FS_COMMAND_FLUSHFILE, TRUE);
        retval = FS_GetResultCode(file);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetFileLength

  Description:  Changes the file size.

  Arguments:    file: File handle
                length: Size after changes

  Returns:      Processing result.
 *---------------------------------------------------------------------------*/
FSResult FS_SetFileLength(FSFile *file, u32 length)
{
    FSResult    retval = FS_RESULT_ERROR;
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForSetFileLength  arg[1];
        file->argument = arg;
        arg->length = length;
        (void)FSi_SendCommand(file, FS_COMMAND_SETFILELENGTH, TRUE);
        retval = FS_GetResultCode(file);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathName

  Description:  Gets path name of the specified handle.

  Arguments:    file: File or directory
                buffer: Path storage destination
                length: Buffer size

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_GetPathName(FSFile *file, char *buffer, u32 length)
{
    BOOL    retval = FALSE;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) || FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForGetPath    arg[1];
        file->argument = arg;
        arg->is_directory = FS_IsDir(file);
        arg->buffer = buffer;
        arg->length = length;
        retval = FSi_SendCommand(file, FS_COMMAND_GETPATH, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPathLength

  Description:  Gets length of full path name of the specified file or directory.

  Arguments:    file: File or directory handle

  Returns:      Length of the path name that included '\0' at the end if successful, and -1 if failed.
 *---------------------------------------------------------------------------*/
s32 FS_GetPathLength(FSFile *file)
{
    s32     retval = -1;
    if (FS_GetPathName(file, NULL, 0))
    {
        retval = file->arg.getpath.total_len;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ConvertPathToFileID

  Description:  Gets file ID from the path name.

  Arguments:    p_fileid: FSFileID storage destination
                path: Path name

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_ConvertPathToFileID(FSFileID *p_fileid, const char *path)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(p_fileid);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSFile      file[1];
            FSArgumentForFindPath   arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->target_is_directory = FALSE;
            if (FSi_SendCommand(file, FS_COMMAND_FINDPATH, TRUE))
            {
                p_fileid->arc = arc;
                p_fileid->file_id = arg->target_id;
                retval = TRUE;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileDirect

  Description:  Opens the file by directly specifying the region of the archive.

  Arguments:    file: FSFile that retains handle information
                arc: Archive that is the map source
                image_top: Offset for the file image start
                image_bottom: Offset of the file image terminator
                id: Arbitrarily specified file ID

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileDirect(FSFile *file, FSArchive *arc,
                       u32 image_top, u32 image_bottom, u32 id)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(arc);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(!FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForOpenFileDirect arg[1];
        file->arc = arc;
        file->argument = arg;
        arg->id = id;
        arg->top = image_top;
        arg->bottom = image_bottom;
        arg->mode = 0;
        retval = FSi_SendCommand(file, FS_COMMAND_OPENFILEDIRECT, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileFast

  Description:  Opens the file by specifying the ID.

  Arguments:    file: FSFile that retains handle information
                id: FSFileID that indicates the file to be opened

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileFast(FSFile *file, FSFileID id)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(!FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    if (id.arc)
    {
        FSArgumentForOpenFileFast   arg[1];
        file->arc = id.arc;
        file->argument = arg;
        arg->id = id.file_id;
        arg->mode = 0;
        retval = FSi_SendCommand(file, FS_COMMAND_OPENFILEFAST, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileEx

  Description:  Opens the file by specifying the path name.

  Arguments:    file: FSFile structure
                path: Path name

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileEx(FSFile *file, const char *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    // Logic check relating to FS_FILEMODE_L
    // (No point in opening in creation mode if there is a size restriction)
    if (((mode & FS_FILEMODE_L) != 0) &&
        ((mode & FS_FILEMODE_RW) == FS_FILEMODE_W))
    {
        OS_TWarning("\"FS_FILEMODE_WL\" seems useless.\n"
                    "(this means creating empty file and prohibiting any modifications)");
    }
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSArgumentForOpenFile   arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->mode = mode;
            if (FSi_SendCommand(file, FS_COMMAND_OPENFILE, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CloseFile

  Description:  Close the file

  Arguments:    file: File handle

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_CloseFile(FSFile *file)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        retval = FSi_SendCommand(file, FS_COMMAND_CLOSEFILE, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetSeekCacheSize

  Description:  Finds the necessary buffer size for the full cache for high-speed reverse seek.

  Arguments:    path

  Returns:      Size if successful; 0 if failed.
 *---------------------------------------------------------------------------*/
u32 FS_GetSeekCacheSize(const char *path)
{
    u32         retval = 0;
    // Gets the size if the file exists
    FSPathInfo  info;
    if (FS_GetPathInfo(path, &info) &&
        ((info.attributes & FS_ATTRIBUTE_IS_DIRECTORY) == 0))
    {
        // Get FAT information for the corresponding archive
        FSArchiveResource   resource;
        if (FS_GetArchiveResource(path, &resource))
        {
            // Calculate the cache size if the actual archive of the FAT base
            u32     bytesPerCluster = resource.sectorsPerCluster * resource.bytesPerSector;
            if (bytesPerCluster != 0)
            {
                static const u32    fatBits = 32;
                retval = (u32)((info.filesize + bytesPerCluster - 1) / bytesPerCluster) * ((fatBits + 4) / 8);
				// Add margin to align the portions before and after the buffer to the cache lines
                retval += (u32)(HW_CACHE_LINE_SIZE * 2);
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetSeekCache

  Description:  Assigns cache buffer for high-speed reverse seek.

  Arguments:    file: File handle
                buf: Cache buffer
                buf_size: Cache buffer size

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_SetSeekCache(FSFile *file, void* buf, u32 buf_size)
{
    FSArgumentForSetSeekCache    arg[1];
    BOOL     retval = FALSE;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));

    file->argument = arg;
    arg->buf      = buf;
    arg->buf_size = buf_size;
    retval = FSi_SendCommand(file, FS_COMMAND_SETSEEKCACHE, TRUE);

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileLength

  Description:  Gets file size.

  Arguments:    file: File handle

  Returns:      File size.
 *---------------------------------------------------------------------------*/
u32 FS_GetFileLength(FSFile *file)
{
    u32     retval = 0;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    // Can reference directly in this case if it is an archive procedure
    if (!FSi_GetFileLengthIfProc(file, &retval))
    {
        FSArgumentForGetFileLength    arg[1];
        file->argument = arg;
        arg->length = 0;
        if (FSi_SendCommand(file, FS_COMMAND_GETFILELENGTH, TRUE))
        {
            retval = arg->length;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFilePosition

  Description:  Gets the current position of the file pointer.

  Arguments:    file: File handle

  Returns:      Current position of the file pointer.
 *---------------------------------------------------------------------------*/
u32 FS_GetFilePosition(FSFile *file)
{
    u32     retval = 0;
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    // Can reference directly in this case if it is an archive procedure
    if (!FSi_GetFilePositionIfProc(file, &retval))
    {
        FSArgumentForGetFilePosition    arg[1];
        file->argument = arg;
        arg->position = 0;
        if (FSi_SendCommand(file, FS_COMMAND_GETFILEPOSITION, TRUE))
        {
            retval = arg->position;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SeekFile

  Description:  Moves the file pointer.

  Arguments:    file: File handle
                offset: Movement amount
                origin: Movement starting point

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_SeekFile(FSFile *file, s32 offset, FSSeekFileMode origin)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file));
    // Can reference directly in this case if it is an archive procedure
    if (!(retval = FSi_SeekFileIfProc(file, offset, origin)))    
    {
        FSArgumentForSeekFile   arg[1];
        file->argument = arg;
        arg->offset = (int)offset;
        arg->from = origin;
        retval = FSi_SendCommand(file, FS_COMMAND_SEEKFILE, TRUE);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFile

  Description:  Read data from the file.

  Arguments:    file: File handle
                buffer: Transfer destination buffer
                length: Read size.

  Returns:      Actual read size if successful, -1 if failed.
 *---------------------------------------------------------------------------*/
s32 FS_ReadFile(FSFile *file, void *buffer, s32 length)
{
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForReadFile   arg[1];
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        if (FSi_SendCommand(file, FS_COMMAND_READFILE, TRUE))
        {
            length = (s32)arg->length;
        }
        else
        {
            if( ( file->error == FS_RESULT_INVALID_PARAMETER ) || ( file->error == FS_RESULT_ERROR ) ) {
                length = -1; //If not read at all
            }else{
                length = (s32)arg->length; //If reading was tried, a value higher than -1 is entered
            }
        }
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadFileAsync

  Description:  Asynchronously reads data from the file.

  Arguments:    file: File handle
                buffer: Transfer destination buffer
                length: Read size.

  Returns:      Simply the same value as the length if successful, and -1 if failed.
 *---------------------------------------------------------------------------*/
s32 FS_ReadFileAsync(FSFile *file, void *buffer, s32 length)
{
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    // Correct size in this case if it is an archive procedure
    {
        u32     end, pos;
        if (FSi_GetFilePositionIfProc(file, &pos) &&
            FSi_GetFileLengthIfProc(file, &end) &&
            (pos + length > end))
        {
            length = (s32)(end - pos);
        }
    }
    {
        FSArgumentForReadFile  *arg = (FSArgumentForReadFile*)file->reserved2;
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        (void)FSi_SendCommand(file, FS_COMMAND_READFILE, FALSE);
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFile

  Description:  Writes data to the file.

  Arguments:    file: File handle
                buffer: Transfer source buffer
                length: Write size

  Returns:      Actual write size if successful, -1 if failed.
 *---------------------------------------------------------------------------*/
s32 FS_WriteFile(FSFile *file, const void *buffer, s32 length)
{
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForWriteFile  arg[1];
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        if (FSi_SendCommand(file, FS_COMMAND_WRITEFILE, TRUE))
        {
            length = (s32)arg->length;
        }
        else
        {
            if( file->error == FS_RESULT_INVALID_PARAMETER) {
                length = -1; //If not written at all
            }else{
                length = (s32)arg->length; //If writing was tried, a value higher than -1 is entered
            }
        }
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_WriteFileAsync

  Description:  Asynchronously writes data to file.

  Arguments:    file: File handle
                buffer: Transfer source buffer
                length: Write size

  Returns:      Simply the same value as the length if successful, and -1 if failed.
 *---------------------------------------------------------------------------*/
s32 FS_WriteFileAsync(FSFile *file, const void *buffer, s32 length)
{
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FSi_IsValidTransferRegion(buffer, length));
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsFile(file) && !FS_IsBusy(file));
    // Correct the size here if it is an archive procedure
    {
        u32     end, pos;
        if (FSi_GetFilePositionIfProc(file, &pos) &&
            FSi_GetFileLengthIfProc(file, &end) &&
            (pos + length > end))
        {
            length = (s32)(end - pos);
        }
    }
    {
        FSArgumentForWriteFile *arg = (FSArgumentForWriteFile*)file->reserved2;
        file->argument = arg;
        arg->buffer = buffer;
        arg->length = (u32)length;
        (void)FSi_SendCommand(file, FS_COMMAND_WRITEFILE, FALSE);
    }
    return length;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectory

  Description:  Opens the directory handle.

  Arguments:    file: FSFile structure
                path: Path name
                mode: Access mode

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenDirectory(FSFile *file, const char *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        char        relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FS_NormalizePath(path, &baseid, relpath);
        if (arc)
        {
            FSArgumentForOpenDirectory  arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->mode = mode;
            if (FSi_SendCommand(file, FS_COMMAND_OPENDIRECTORY, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_CloseDirectory

  Description:  Closes the directory handle.

  Arguments:    file: FSFile structure

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_CloseDirectory(FSFile *file)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        if (FSi_SendCommand(file, FS_COMMAND_CLOSEDIRECTORY, TRUE))
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectory

  Description:  Reads only one entry of the directory and advances.

  Arguments:    file: FSFile structure
                info: FSDirectoryEntryInfo structure

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDirectory(FSFile *file, FSDirectoryEntryInfo *info)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForReadDirectory  arg[1];
        file->argument = arg;
        arg->info = info;
        MI_CpuFill8(info, 0x00, sizeof(info));
        info->id = FS_INVALID_FILE_ID;
        if (FSi_SendCommand(file, FS_COMMAND_READDIR, TRUE))
        {
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SeekDir

  Description:  Opens by specifying the directory position.

  Arguments:    file: FSFile structure
                pos: Directory position obtained by FS_ReadDir and FS_TellDir

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_SeekDir(FSFile *file, const FSDirPos *pos)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(pos);
    SDK_NULL_ASSERT(pos->arc);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArgumentForSeekDirectory  arg[1];
        arg->id = (u32)((pos->own_id << 0) | (pos->index << 16));
        arg->position = pos->pos;
        file->arc = pos->arc;
        file->argument = arg;
        if (FSi_SendCommand(file, FS_COMMAND_SEEKDIR, TRUE))
        {
            file->stat |= FS_FILE_STATUS_IS_DIR;
            retval = TRUE;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_TellDir

  Description:  Gets current directory position from directory handle.

  Arguments:    dir: Directory handle
                pos: Storage destination of the directory position

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_TellDir(const FSFile *dir, FSDirPos *pos)
{
    BOOL        retval = FALSE;
    SDK_NULL_ASSERT(dir);
    SDK_NULL_ASSERT(pos);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(dir));
    {
        *pos = dir->prop.dir.pos;
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_RewindDir

  Description:  Returns enumeration position of the directory handle to the top.

  Arguments:    dir: Directory handle

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_RewindDir(FSFile *dir)
{
    BOOL        retval = FALSE;
    SDK_NULL_ASSERT(dir);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(dir));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    {
        FSDirPos pos;
        pos.arc = dir->arc;
        pos.own_id = dir->prop.dir.pos.own_id;
        pos.pos = 0;
        pos.index = 0;
        retval = FS_SeekDir(dir, &pos);
    }
    return retval;
}


/*---------------------------------------------------------------------------*
 * Unicode support
 *---------------------------------------------------------------------------*/

enum
{
    FS_UNICODE_CONVSRC_ASCII,
    FS_UNICODE_CONVSRC_SHIFT_JIS,
    FS_UNICODE_CONVSRC_UNICODE
};

/*---------------------------------------------------------------------------*
  Name:         FSi_CopySafeUnicodeString

  Description:   Checks buffer size and copies string as Unicode.

  Arguments:    dst: Transfer destination buffer
                dstlen: Transfer destination size
                src: Transfer source buffer
                srclen: Transfer character size
                srctype: Character set of transfer source
                stickyFailure: FALSE, if truncated at transfer origin
                
  Returns:      Actually stored character count.
 *---------------------------------------------------------------------------*/
static int FSi_CopySafeUnicodeString(u16 *dst, int dstlen,
                                     const void *srcptr, int srclen,
                                     int srctype, BOOL *stickyFailure)
{
    int     srcpos = 0;
    int     dstpos = 0;
    if (srctype == FS_UNICODE_CONVSRC_ASCII)
    {
        const char *src = (const char *)srcptr;
        int     n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
        while ((dstpos < n) && src[srcpos])
        {
            dst[dstpos++] = (u8)src[srcpos++];
        }
        if ((srcpos < srclen) && src[srcpos])
        {
            *stickyFailure = TRUE;
        }
    }
    else if (srctype == FS_UNICODE_CONVSRC_UNICODE)
    {
        const u16 *src = (const u16 *)srcptr;
        int     n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
        while ((dstpos < n) && src[srcpos])
        {
            dst[dstpos++] = src[srcpos++];
        }
        if ((srcpos < srclen) && src[srcpos])
        {
            *stickyFailure = TRUE;
        }
    }
    else if (srctype == FS_UNICODE_CONVSRC_SHIFT_JIS)
    {
        const char *src = (const char *)srcptr;
        srcpos = srclen;
        dstpos = dstlen - 1;
        (void)FSi_ConvertStringSjisToUnicode(dst, &dstpos, src, &srcpos, NULL);
        if ((srcpos < srclen) && src[srcpos])
        {
            *stickyFailure = TRUE;
        }
    }
    dst[dstpos] = L'\0';
    return dstpos;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_NormalizePathWtoW

  Description:  Converts Unicode path to Unicode full path that includes up to the archive name.

  Arguments:    path: Non-normalized path string
                baseid: Standard directory ID storage destination or NULL
                relpath: Path name storage destination after conversion or NULL

  Returns:      Archive pointer or NULL.
 *---------------------------------------------------------------------------*/
FSArchive* FSi_NormalizePathWtoW(const u16 *path, u32*baseid, u16 *relpath);
FSArchive* FSi_NormalizePathWtoW(const u16 *path, u32*baseid, u16 *relpath)
{
    FSArchive  *arc = NULL;
    int         pathlen = 0;
    int         pathmax = FS_ARCHIVE_FULLPATH_MAX + 1;
    BOOL        stickyFailure = FALSE;
    // First, specify archive to be the command target
    // If specified Unicode path is absolute path, get archive
    BOOL        absolute = FALSE;
    int         arcnameLen;
    for (arcnameLen = 0; arcnameLen < FS_ARCHIVE_NAME_LONG_MAX + 1; ++arcnameLen)
    {
        if (path[arcnameLen] == L'\0')
        {
            break;
        }
        else if (FSi_IsUnicodeSlash(path[arcnameLen]))
        {
            break;
        }
        else if (path[arcnameLen] == L':')
        {
            char    arcname[FS_ARCHIVE_NAME_LONG_MAX + 1];
            int     j;
            for (j = 0; j < arcnameLen; ++j)
            {
                arcname[j] = (char)path[j];
            }
            arcname[arcnameLen] = '\0';
            arc = FS_FindArchive(arcname, arcnameLen);
            break;
        }
    }
    if (arc)
    {
        absolute = TRUE;
        *baseid = 0;
    }
    else
    {
        arc = FS_NormalizePath("", baseid, NULL);
    }
    if (arc)
    {
        // If archive cannot support Unicode, fails here
        u32     caps = 0;
        (void)arc->vtbl->GetArchiveCaps(arc, &caps);
        if ((caps & FS_ARCHIVE_CAPS_UNICODE) == 0)
        {
            arc = NULL;
        }
        else
        {
            // Stores archive name at top
            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                 FS_GetArchiveName(arc), FS_ARCHIVE_NAME_LONG_MAX,
                                                 FS_UNICODE_CONVSRC_ASCII, &stickyFailure);
            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                 L":", 1,
                                                 FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
            // If absolute path, concatenate the root and everything that follows
            if (absolute)
            {
                path += arcnameLen + 1 + FSi_IsUnicodeSlash(path[arcnameLen + 1]);
            }
            // If current root, directly concatenate the root and everything that follows
            else if (FSi_IsUnicodeSlash(*path))
            {
                path += 1;
            }
            // If the current directory, convert Shift_JIS to Unicode and concatenate
            else
            {
                pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                     L"/", 1,
                                                     FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
                pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                     FS_GetCurrentDirectory(), FS_ENTRY_LONGNAME_MAX,
                                                     FS_UNICODE_CONVSRC_SHIFT_JIS, &stickyFailure);
            }
            // concatenate the remaining portion
            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                 L"/", 1,
                                                 FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
            {
                // Normalize relative paths, being careful of specialized entry names.
                int     curlen = 0;
                while (!stickyFailure)
                {
                    u16     c = path[curlen];
                    if ((c != L'\0') && !FSi_IsUnicodeSlash(c))
                    {
                        curlen += 1;
                    }
                    else
                    {
                        // Ignore empty directory
                        if (curlen == 0)
                        {
                        }
                        // Ignore "." (current directory)
                        else if ((curlen == 1) && (path[0] == L'.'))
                        {
                        }
                        // ".." (parent directory) raises the root one level as the upper limit
                        else if ((curlen == 2) && (path[0] == '.') && (path[1] == '.'))
                        {
                            if ((pathlen > 2) && (relpath[pathlen - 2] != L':'))
                            {
                                --pathlen;
                                pathlen = FSi_DecrementUnicodePositionToSlash(relpath, pathlen) + 1;
                            }
                        }
                        // Add entry for anything else
                        else
                        {
                            pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                                 path, curlen,
                                                                 FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
                            if (c != L'\0')
                            {
                                pathlen += FSi_CopySafeUnicodeString(&relpath[pathlen], pathmax - pathlen,
                                                                     L"/", 1,
                                                                     FS_UNICODE_CONVSRC_UNICODE, &stickyFailure);
                            }
                        }
                        if (c == L'\0')
                        {
                            break;
                        }
                        path += curlen + 1;
                        curlen = 0;
                    }
                }
            }
            relpath[pathlen] = L'\0';
        }
    }
    return stickyFailure ? NULL : arc;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFileExW

  Description:  Opens the file by specifying the path name.

  Arguments:    file: FSFile structure
                path: Path name

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFileExW(FSFile *file, const u16 *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    // Logic check relating to FS_FILEMODE_L
    // (No point in opening in creation mode if there is a size restriction)
    if (((mode & FS_FILEMODE_L) != 0) &&
        ((mode & FS_FILEMODE_RW) == FS_FILEMODE_W))
    {
        OS_TWarning("\"FS_FILEMODE_WL\" seems useless.\n"
                    "(this means creating empty file and prohibiting any modifications)");
    }
    {
        u16         relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FSi_NormalizePathWtoW(path, &baseid, relpath);
        // Currently, to limit to the minimum changes where the Unicode can be used, return unsupported with archives that do not support Unicode, such as ROM 
        // 
        if (!arc)
        {
            file->error = FS_RESULT_UNSUPPORTED;
        }
        else
        {
            FSArgumentForOpenFile   arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = (char*)relpath;
            arg->mode = mode;
            file->stat |= FS_FILE_STATUS_UNICODE_MODE;
            if (FSi_SendCommand(file, FS_COMMAND_OPENFILE, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenDirectoryW

  Description:  Opens the directory handle.

  Arguments:    file: FSFile structure
                path: Path name
                mode: Access mode

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenDirectoryW(FSFile *file, const u16 *path, u32 mode)
{
    BOOL    retval = FALSE;
    FS_DEBUG_TRACE( "%s\n", __FUNCTION__);
    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        u16         relpath[FS_ARCHIVE_FULLPATH_MAX + 1];
        u32         baseid = 0;
        FSArchive  *arc = FSi_NormalizePathWtoW(path, &baseid, relpath);
        // Currently, to limit to the minimum changes where Unicode can be used, return unsupported with archives that do not support Unicode, such as ROM 
        // 
        if (!arc)
        {
            file->error = FS_RESULT_UNSUPPORTED;
        }
        else
        {
            FSArgumentForOpenDirectory  arg[1];
            FS_InitFile(file);
            file->arc = arc;
            file->argument = arg;
            arg->baseid = baseid;
            arg->relpath = (char*)relpath;
            arg->mode = mode;
            file->stat |= FS_FILE_STATUS_UNICODE_MODE;
            if (FSi_SendCommand(file, FS_COMMAND_OPENDIRECTORY, TRUE))
            {
                retval = TRUE;
            }
            else
            {
                file->arc = NULL;
            }
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDirectoryW

  Description:  Reads only one entry of the directory and advances.

  Arguments:    file: FSFile structure
                info: FSDirectoryEntryInfo structure

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDirectoryW(FSFile *file, FSDirectoryEntryInfoW *info)
{
    BOOL    retval = FALSE;
    SDK_NULL_ASSERT(file);
    SDK_NULL_ASSERT(info);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(FS_IsDir(file));
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);
    {
        FSArchive  *arc = file->arc;
        // Currently, to limit to the minimum changes where the Unicode can be used, return unsupported with archives that do not support Unicode, such as ROM 
        // 
        u32     caps = 0;
        (void)arc->vtbl->GetArchiveCaps(arc, &caps);
        if ((caps & FS_ARCHIVE_CAPS_UNICODE) == 0)
        {
            file->error = FS_RESULT_UNSUPPORTED;
        }
        else
        {
            FSArgumentForReadDirectory  arg[1];
            file->argument = arg;
            arg->info = (FSDirectoryEntryInfo*)info;
            MI_CpuFill8(info, 0x00, sizeof(info));
            info->id = FS_INVALID_FILE_ID;
            file->stat |= FS_FILE_STATUS_UNICODE_MODE;
            if (FSi_SendCommand(file, FS_COMMAND_READDIR, TRUE))
            {
                retval = TRUE;
            }
        }
    }
    return retval;
}


/*---------------------------------------------------------------------------*
 * Deprecated Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertToDirEntry

  Description:  Converts from FSDirectoryEntryInfo structure to FSDirEntry structure.

  Arguments:    entry: FSDirEntry structure of the conversion destination
                arc: Archive that obtained the conversion source information
                info: FSDirectoryEntryInfo structure of the conversion source

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
static void FSi_ConvertToDirEntry(FSDirEntry *entry, FSArchive *arc, const FSDirectoryEntryInfo *info)
{
    entry->name_len = info->longname_length;
    if (entry->name_len > sizeof(entry->name) - 1)
    {
        entry->name_len = sizeof(entry->name) - 1;
    }
    MI_CpuCopy8(info->longname, entry->name, entry->name_len);
    entry->name[entry->name_len] = '\0';
    if (info->id == FS_INVALID_FILE_ID)
    {
        entry->is_directory = FALSE;
        entry->file_id.file_id = FS_INVALID_FILE_ID;
        entry->file_id.arc = NULL;
    }
    else if ((info->attributes & FS_ATTRIBUTE_IS_DIRECTORY) != 0)
    {
        entry->is_directory = TRUE;
        entry->dir_id.arc = arc;
        entry->dir_id.own_id = (u16)(info->id >> 0);
        entry->dir_id.index = (u16)(info->id >> 16);
        entry->dir_id.pos = 0;
    }
    else
    {
        entry->is_directory = FALSE;
        entry->file_id.file_id = info->id;
        entry->file_id.arc = arc;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenFile

  Description:  Opens the file by specifying the path name.

  Arguments:    file: FSFile structure
                path: Path name

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenFile(FSFile *file, const char *path)
{
    return FS_OpenFileEx(file, path, FS_FILEMODE_R);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetLength

  Description:  Gets file size.

  Arguments:    file: File handle

  Returns:      File size.
 *---------------------------------------------------------------------------*/
u32 FS_GetLength(FSFile *file)
{
    return FS_GetFileLength(file);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetPosition

  Description:  Gets the current position of the file pointer.

  Arguments:    file: File handle

  Returns:      Current position of the file pointer.
 *---------------------------------------------------------------------------*/
u32 FS_GetPosition(FSFile *file)
{
    return FS_GetFilePosition(file);
}

/*---------------------------------------------------------------------------*
  Name:         FS_FindDir

  Description:  Opens the directory handle.

  Arguments:    dir: FSFile structure
                path: Path name

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_FindDir(FSFile *dir, const char *path)
{
    return FS_OpenDirectory(dir, path, FS_FILEMODE_R);
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReadDir

  Description:  Reads only one entry of the directory and advances.

  Arguments:    file: FSFile structure
                entry: FSDirEntry structure

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_ReadDir(FSFile *file, FSDirEntry *entry)
{
    BOOL                    retval = FALSE;
    FSDirectoryEntryInfo    info[1];
    if (FS_ReadDirectory(file, info))
    {
        FSi_ConvertToDirEntry(entry, FS_GetAttachedArchive(file), info);
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ChangeDir

  Description:  Changes the current directory.

  Arguments:    path: Path name

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL    FS_ChangeDir(const char *path)
{
    return FS_SetCurrentDirectory(path);
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetFileInfo

  Description:  Gets the file information.

  Arguments:    path: Path name
                info: Location to store the information

  Returns:      Processing result.
 *---------------------------------------------------------------------------*/
FSResult FS_GetFileInfo(const char *path, FSFileInfo *info)
{
    return FS_GetPathInfo(path, info) ? FS_RESULT_SUCCESS : FS_GetArchiveResultCode(path);
}


#endif /* FS_IMPLEMENT */

// The following is also used outside of the FS library, so it is not targeted by FS_IMPLEMENT
// Support for Unicode on ARM7 is necessary only for TWL operations, so place it in extended memory
#if defined(SDK_TWL) && defined(SDK_ARM7)
#include <twl/ltdmain_begin.h>
#endif

static const int        FSiUnicodeBufferQueueMax = 4;
static OSMessageQueue   FSiUnicodeBufferQueue[1];
static OSMessage        FSiUnicodeBufferQueueArray[FSiUnicodeBufferQueueMax];
static BOOL             FSiUnicodeBufferQueueInitialized = FALSE;
static u16              FSiUnicodeBufferTable[FSiUnicodeBufferQueueMax][FS_ARCHIVE_FULLPATH_MAX + 1];

/*---------------------------------------------------------------------------*
  Name:         FSi_GetUnicodeBuffer

  Description:  Gets temporary buffer for Unicode conversion.
                The FS library is used for conversion of Shift_JIS.

  Arguments:    src: Shift_JIS string needed in Unicode conversion or NULL

  Returns:      String buffer converted to UTF16-LE if necessary.
 *---------------------------------------------------------------------------*/
u16* FSi_GetUnicodeBuffer(const char *src)
{
    u16        *retval = NULL;
    // Added buffer to message queue when making the initial call
    OSIntrMode  bak = OS_DisableInterrupts();
    if (!FSiUnicodeBufferQueueInitialized)
    {
        int     i;
        FSiUnicodeBufferQueueInitialized = TRUE;
        OS_InitMessageQueue(FSiUnicodeBufferQueue, FSiUnicodeBufferQueueArray, 4);
        for (i = 0; i < FSiUnicodeBufferQueueMax; ++i)
        {
            (void)OS_SendMessage(FSiUnicodeBufferQueue, FSiUnicodeBufferTable[i], OS_MESSAGE_BLOCK);
        }
    }
    (void)OS_RestoreInterrupts(bak);
    // Allocate the buffer from the message queue (if not necessary, block here)
    (void)OS_ReceiveMessage(FSiUnicodeBufferQueue, (OSMessage*)&retval, OS_MESSAGE_BLOCK);
    if (src)
    {
        int     dstlen = FS_ARCHIVE_FULLPATH_MAX;
        (void)FSi_ConvertStringSjisToUnicode(retval, &dstlen, src, NULL, NULL);
        retval[dstlen] = L'\0';
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ReleaseUnicodeBuffer

  Description:  Deallocates the temporary buffer for Unicode conversion,

  Arguments:    buf: Buffer allocated by the FSi_GetUnicodeBuffer function

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_ReleaseUnicodeBuffer(const void *buf)
{
    if (buf)
    {
        // Return used buffer to the message queue
        (void)OS_SendMessage(FSiUnicodeBufferQueue, (OSMessage)buf, OS_MESSAGE_BLOCK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringSjisToUnicode

  Description:  Converts a Shift JIS string into a Unicode string.
                If path name is clearly ASCII-only and conversion of Unicode and ShiftJIS can be simplified, overwrite this function to prevent linking to standard process in STD library.
                
                
                

  Arguments:    dst: 		Conversion destination buffer
  					 		The storage process is ignored if NULL is specified.
                dst_len: 	Stores and passes the max number of chars for the destination buffer, then receives the number of chars that were actually stored.
                			Ignored when NULL is given.
                                  
                src: 		Conversion source buffer
                src_len: 	Pointer which stores and passes the maximum number of characters to be converted, then receives the number actually converted.
                			The end-of-string position takes priority over the specified position.
                            When either a negative value is stored and passed, or NULL is given, the character count is revised to be the number of characters to the end of the string.
                                  
                                  
                callback: 	Callback to be called if there are any characters that can't be converted.
                			When NULL is specified, the conversion process ends at the position of the character that cannot be converted.
                                  

  Returns:      Result of the conversion process.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL
STDResult FSi_ConvertStringSjisToUnicode(u16 *dst, int *dst_len,
                                         const char *src, int *src_len,
                                         STDConvertUnicodeCallback callback)
 __attribute__((never_inline))
{
    return STD_ConvertStringSjisToUnicode(dst, dst_len, src, src_len, callback);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ConvertStringUnicodeToSjis

  Description:  Converts a Unicode character string into a ShiftJIS character string.
                If path name is clearly ASCII-only and conversion of Unicode and ShiftJIS can be simplified, overwrite this function to prevent linking to standard process of STD library.
                
                
                

  Arguments:    dst: 		Conversion destination buffer
                            The storage process is ignored if NULL is specified.
                dst_len: 	Stores and passes the maximum number of characters for the destination buffer, then receives the number of characters that were actually stored.
                            Ignored when NULL is given.
                                  
                src: 		Conversion source buffer
                src_len: 	Stores and passes the maximum number of characters to be converted, then receives the number actually converted.
                            The end-of-string position takes priority over the specified position.
                            When either a negative value is stored and passed, or NULL is given, the character count is revised to be the number of characters to the end of the string.
                                  
                                  
                callback: Callback to be called if there are any characters that can't be converted.
                                  When NULL is specified, the conversion process ends at the position of the character that cannot be converted.
                                  

  Returns:      Result of the conversion process.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL
STDResult FSi_ConvertStringUnicodeToSjis(char *dst, int *dst_len,
                                         const u16 *src, int *src_len,
                                         STDConvertSjisCallback callback)
 __attribute__((never_inline))
{
    return STD_ConvertStringUnicodeToSjis(dst, dst_len, src, src_len, callback);
}

#if defined(SDK_TWL) && defined(SDK_ARM7)
#include <twl/ltdmain_end.h>
#endif
