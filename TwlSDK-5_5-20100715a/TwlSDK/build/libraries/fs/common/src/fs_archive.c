/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_archive.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-08-27#$
  $Rev: 11020 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro/fs.h>

#include <nitro/mi/memory.h>
#include <nitro/std.h>


#include "../include/util.h"
#include "../include/command.h"


#if defined(FS_IMPLEMENT)


/*---------------------------------------------------------------------------*/
/* Constants */

// Enable when internally supporting long archive names
#define FS_SUPPORT_LONG_ARCNAME


/*---------------------------------------------------------------------------*/
/* Variables */

// Archive list registered to the file system
static FSArchive   *arc_list = NULL;

// Current directory
static FSDirPos     current_dir_pos;
static char         current_dir_path[FS_ENTRY_LONGNAME_MAX];

#if defined(FS_SUPPORT_LONG_ARCNAME)
// Static buffer for internal support of long archive names
#define FS_LONG_ARCNAME_LENGTH_MAX  15
#define FS_LONG_ARCNAME_TABLE_MAX   16
static char FSiLongNameTable[FS_LONG_ARCNAME_TABLE_MAX][FS_LONG_ARCNAME_LENGTH_MAX + 1];
#endif


/*---------------------------------------------------------------------------*/
/* Functions */

FSArchive* FSi_GetArchiveChain(void)
{
    return arc_list;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IsEventCommand

  Description:  Determines whether the command is an event notification.

  Arguments:    command: Command type

  Returns:      TRUE when command is event notification.
 *---------------------------------------------------------------------------*/
static BOOL FSi_IsEventCommand(FSCommandType command)
{
    return
        ((command == FS_COMMAND_ACTIVATE) ||
        (command == FS_COMMAND_IDLE) ||
        (command == FS_COMMAND_SUSPEND) ||
        (command == FS_COMMAND_RESUME) ||
        (command == FS_COMMAND_MOUNT) ||
        (command == FS_COMMAND_UNMOUNT) ||
        (command == FS_COMMAND_INVALID));
}

/*---------------------------------------------------------------------------*
  Name:         FSi_EndCommand

  Description:  Completes the command and returns if there is an idle thread.

  Arguments:    file: Completed command
                ret: Command result value

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_EndCommand(FSFile *file, FSResult ret)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    // Delete command from the list
    FSArchive *const arc = file->arc;
    if (arc)
    {
        FSFile    **pp = &arc->list;
        for (; *pp; pp = &(*pp)->next)
        {
            if (*pp == file)
            {
                *pp = file->next;
                break;
            }
        }
        file->next = NULL;
    }
    // Store results
    {
        FSCommandType   command = FSi_GetCurrentCommand(file);
        if (!FSi_IsEventCommand(command) && (arc != NULL))
        {
            arc->command = command;
            arc->result = ret;
        }
        file->error = ret;
        file->stat &= ~(FS_FILE_STATUS_CANCEL | FS_FILE_STATUS_BUSY |
                        FS_FILE_STATUS_BLOCKING | FS_FILE_STATUS_OPERATING |
                        FS_FILE_STATUS_ASYNC_DONE | FS_FILE_STATUS_UNICODE_MODE);
    }
    // Notify if there is an idling thread
    OS_WakeupThread(file->queue);
    (void)OS_RestoreInterrupts(bak_psr);
}


// The following two temporarily publicized for procedures
// Commands recursively calling from the procedures are all synchronous, so it is acceptable for applying only the FSi_WaitForArchiveCompletion function by directly calling via the vtbl.
// 

FSResult FSi_WaitForArchiveCompletion(FSFile *file, FSResult result)
{
    if (result == FS_RESULT_PROC_ASYNC)
    {
        FSi_WaitConditionOn(&file->stat, FS_FILE_STATUS_ASYNC_DONE, file->queue);
        file->stat &= ~FS_FILE_STATUS_ASYNC_DONE;
        result = file->error;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_InvokeCommand

  Description:  Issues a command to an archive.
                Command list management is resolved before calling this function.

  Arguments:    file: FSFile structure that stores the argument
                Command: Command ID

  Returns:      The processing result for the command.
 *---------------------------------------------------------------------------*/
static FSResult FSi_InvokeCommand(FSFile *file, FSCommandType command)
{
    FSResult            result = FS_RESULT_UNSUPPORTED;
    FSArchive   * const arc = file->arc;

    {
        const void   *(*table) = (const void*)arc->vtbl;
        // Undefined command out of range
        if ((command < 0) || (command >= FS_COMMAND_MAX))
        {
            OS_TWarning("undefined command (%d)\n", command);
            result = FS_RESULT_UNSUPPORTED;
        }
        // Call by restoring the argument if it is not a command whose implementation was abandoned by the driver side
        else if (table[command] == NULL)
        {
            result = FS_RESULT_UNSUPPORTED;
        }
        else
        {
#define FS_DECLARE_ARGUMENT_(type)                              \
            type *arg = (type*) file->argument;                 \
            (void)arg

#define FS_INVOKE_METHOD_(command, ...)                         \
            do                                                  \
            {                                                   \
                FS_DECLARE_ARGUMENT_(FSArgumentFor ## command); \
                result = arc->vtbl->command(__VA_ARGS__);       \
            }                                                   \
            while(0)
#define FS_NOTIFY_EVENT_(command, ...)                          \
            do                                                  \
            {                                                   \
                FS_DECLARE_ARGUMENT_(FSArgumentFor ## command); \
                (void)arc->vtbl->command(__VA_ARGS__);          \
                return FS_RESULT_SUCCESS;                       \
            }                                                   \
            while(0)
            switch (command)
            {
            case FS_COMMAND_READFILE:
                FS_INVOKE_METHOD_(ReadFile, arc, file, arg->buffer, &arg->length);
                break;
            case FS_COMMAND_WRITEFILE:
                FS_INVOKE_METHOD_(WriteFile, arc, file, arg->buffer, &arg->length);
                break;
            case FS_COMMAND_SEEKDIR:
                FS_INVOKE_METHOD_(SeekDirectory, arc, file, arg->id, arg->position);
                break;
            case FS_COMMAND_READDIR:
                FS_INVOKE_METHOD_(ReadDirectory, arc, file, arg->info);
                break;
            case FS_COMMAND_FINDPATH:
                FS_INVOKE_METHOD_(FindPath, arc, arg->baseid, arg->relpath, &arg->target_id, arg->target_is_directory);
                break;
            case FS_COMMAND_GETPATH:
                FS_INVOKE_METHOD_(GetPath, arc, file, arg->is_directory, arg->buffer, &arg->length);
                break;
            case FS_COMMAND_OPENFILEFAST:
                FS_INVOKE_METHOD_(OpenFileFast, arc, file, arg->id, arg->mode);
                break;
            case FS_COMMAND_OPENFILEDIRECT:
                FS_INVOKE_METHOD_(OpenFileDirect, arc, file, arg->top, arg->bottom, &arg->id);
                break;
            case FS_COMMAND_CLOSEFILE:
                FS_INVOKE_METHOD_(CloseFile, arc, file);
                break;
            case FS_COMMAND_ACTIVATE:
                FS_NOTIFY_EVENT_(Activate, arc);
                break;
            case FS_COMMAND_IDLE:
                FS_NOTIFY_EVENT_(Idle, arc);
                break;
            case FS_COMMAND_SUSPEND:
                FS_NOTIFY_EVENT_(Suspend, arc);
                break;
            case FS_COMMAND_RESUME:
                FS_NOTIFY_EVENT_(Resume, arc);
                break;
            case FS_COMMAND_OPENFILE:
                FS_INVOKE_METHOD_(OpenFile, arc, file, arg->baseid, arg->relpath, arg->mode);
                break;
            case FS_COMMAND_SEEKFILE:
                FS_INVOKE_METHOD_(SeekFile, arc, file, &arg->offset, arg->from);
                break;
            case FS_COMMAND_GETFILELENGTH:
                FS_INVOKE_METHOD_(GetFileLength, arc, file, &arg->length);
                break;
            case FS_COMMAND_GETFILEPOSITION:
                FS_INVOKE_METHOD_(GetFilePosition, arc, file, &arg->position);
                break;
                // From here the extended command
            case FS_COMMAND_MOUNT:
                FS_NOTIFY_EVENT_(Mount, arc);
                break;
            case FS_COMMAND_UNMOUNT:
                FS_NOTIFY_EVENT_(Unmount, arc);
                break;
            case FS_COMMAND_GETARCHIVECAPS:
                FS_INVOKE_METHOD_(GetArchiveCaps, arc, &arg->caps);
                break;
            case FS_COMMAND_CREATEFILE:
                FS_INVOKE_METHOD_(CreateFile, arc, arg->baseid, arg->relpath, arg->permit);
                break;
            case FS_COMMAND_DELETEFILE:
                FS_INVOKE_METHOD_(DeleteFile, arc, arg->baseid, arg->relpath);
                break;
            case FS_COMMAND_RENAMEFILE:
                FS_INVOKE_METHOD_(RenameFile, arc, arg->baseid_src, arg->relpath_src, arg->baseid_dst, arg->relpath_dst);
                break;
            case FS_COMMAND_GETPATHINFO:
                FS_INVOKE_METHOD_(GetPathInfo, arc, arg->baseid, arg->relpath, arg->info);
                break;
            case FS_COMMAND_SETPATHINFO:
                FS_INVOKE_METHOD_(SetPathInfo, arc, arg->baseid, arg->relpath, arg->info);
                break;
            case FS_COMMAND_CREATEDIRECTORY:
                FS_INVOKE_METHOD_(CreateDirectory, arc, arg->baseid, arg->relpath, arg->permit);
                break;
            case FS_COMMAND_DELETEDIRECTORY:
                FS_INVOKE_METHOD_(DeleteDirectory, arc, arg->baseid, arg->relpath);
                break;
            case FS_COMMAND_RENAMEDIRECTORY:
                FS_INVOKE_METHOD_(RenameDirectory, arc, arg->baseid_src, arg->relpath_src, arg->baseid_dst, arg->relpath_dst);
                break;
            case FS_COMMAND_GETARCHIVERESOURCE:
                FS_INVOKE_METHOD_(GetArchiveResource, arc, arg->resource);
                break;
            case FS_COMMAND_FLUSHFILE:
                FS_INVOKE_METHOD_(FlushFile, arc, file);
                break;
            case FS_COMMAND_SETFILELENGTH:
                FS_INVOKE_METHOD_(SetFileLength, arc, file, arg->length);
                break;
            case FS_COMMAND_OPENDIRECTORY:
                FS_INVOKE_METHOD_(OpenDirectory, arc, file, arg->baseid, arg->relpath, arg->mode);
                break;
            case FS_COMMAND_CLOSEDIRECTORY:
                FS_INVOKE_METHOD_(CloseDirectory, arc, file);
                break;
            case FS_COMMAND_SETSEEKCACHE:
                FS_INVOKE_METHOD_(SetSeekCache, arc, file, arg->buf, arg->buf_size);
                break;
            default:
                result = FS_RESULT_UNSUPPORTED;
            }
#undef FS_DECLARE_ARGUMENT_
#undef FS_INVOKE_METHOD_
#undef FS_NOTIFY_EVENT_
        }
    }
    // If this is a normal command with no event notification, return to satisfy conditions when issued
    if (!FSi_IsEventCommand(command))
    {
        // Because there are many cases where the path name is incorrect and unsupported, if there is the possibility, a warning will be output
        // 
        if (result == FS_RESULT_UNSUPPORTED)
        {
            OS_TWarning("archive \"%s:\" cannot support command %d.\n", FS_GetArchiveName(arc), command);
        }
        // If necessary, apply blocking
        if ((file->stat & FS_FILE_STATUS_BLOCKING) != 0)
        {
            result = FSi_WaitForArchiveCompletion(file, result);
        }
        // Release here if completed while nobody is blocking
        else if (result != FS_RESULT_PROC_ASYNC)
        {
            FSi_EndCommand(file, result);
        }
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_NextCommand

  Description:  Archive selects the next command that should be processed.
                If an asynchronous command is selected, return that pointer.
                If anything other than NULL is returned, it is necessary to process at the source of the call.

  Arguments:    arc: Archive that gets the next command
                owner: TRUE if source of call already has command execution rights

  Returns:      The next command that must be processed here.
 *---------------------------------------------------------------------------*/
static FSFile *FSi_NextCommand(FSArchive *arc, BOOL owner)
{
    FSFile  *next = NULL;
    // First, check all command cancel requests
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if ((arc->flag & FS_ARCHIVE_FLAG_CANCELING) != 0)
        {
            FSFile *p = arc->list;
            arc->flag &= ~FS_ARCHIVE_FLAG_CANCELING;
            while (p != NULL)
            {
                FSFile *q = p->next;
                // Commands whose processes have already started cannot be externally canceled.
                if (FS_IsCanceling(p) && ((p->stat & FS_FILE_STATUS_OPERATING) == 0))
                {
                    FSi_EndCommand(p, FS_RESULT_CANCELED);
                    if (!q)
                    {
                        q = arc->list;
                    }
                }
                p = q;
            }
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
    // Determine whether the next command can be executed
    {
        OSIntrMode  bak_psr = OS_DisableInterrupts();
        if (((arc->flag & FS_ARCHIVE_FLAG_SUSPENDING) == 0) &&
            ((arc->flag & FS_ARCHIVE_FLAG_SUSPEND) == 0) && arc->list)
        {
            // Start up here if archive is stopped
            const BOOL  is_started = owner && ((arc->flag & FS_ARCHIVE_FLAG_RUNNING) == 0);
            if (is_started)
            {
                arc->flag |= FS_ARCHIVE_FLAG_RUNNING;
            }
            (void)OS_RestoreInterrupts(bak_psr);
            if (is_started)
            {
                (void)FSi_InvokeCommand(arc->list, FS_COMMAND_ACTIVATE);
            }
            bak_psr = OS_DisableInterrupts();
            // Command at the top of the list gets execution rights
            if (owner || is_started)
            {
                next = arc->list;
                next->stat |= FS_FILE_STATUS_OPERATING;
            }
            // If command issuing source is blocking, transfer execution rights
            if (owner && ((next->stat & FS_FILE_STATUS_BLOCKING) != 0))
            {
                OS_WakeupThread(next->queue);
                next = NULL;
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
        // Suspend or idle state
        else
        {
            // If does not have execution rights, stop and go to idle state
            if (owner)
            {
                if ((arc->flag & FS_ARCHIVE_FLAG_RUNNING) != 0)
                {
                    FSFile  tmp;
                    FS_InitFile(&tmp);
                    tmp.arc = arc;
                    arc->flag &= ~FS_ARCHIVE_FLAG_RUNNING;
                    (void)FSi_InvokeCommand(&tmp, FS_COMMAND_IDLE);
                }
                // If suspend is executing, notify the source of the suspend function call
                if ((arc->flag & FS_ARCHIVE_FLAG_SUSPENDING) != 0)
                {
                    arc->flag &= ~FS_ARCHIVE_FLAG_SUSPENDING;
                    arc->flag |= FS_ARCHIVE_FLAG_SUSPEND;
                    OS_WakeupThread(&arc->queue);
                }
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
    }
    return next;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ExecuteAsyncCommand

  Description:  Executes an asynchronous command.
                The first call is made from the user thread with interrupts enabled.
                As long as the archive is operating synchronously, command processing will repeat here. If even a single asynchronous process occurs, the rest is performed with NotifyAsyncEnd().
                

                Therefore, it is necessary to be aware of the NotifyAsyncEnd() calling environment when archive processing switches between synchronous and asynchronous.
                

  Arguments:    file: FSFile structure that stores the asynchronous command to execute

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ExecuteAsyncCommand(FSFile *file)
{
    FSArchive   * const arc = file->arc;
    while (file)
    {
        // If call source is waiting on completion, wake that up and leave it to the process
        {
            OSIntrMode bak_psr = OS_DisableInterrupts();
            file->stat |= FS_FILE_STATUS_OPERATING;
            if ((file->stat & FS_FILE_STATUS_BLOCKING) != 0)
            {
                OS_WakeupThread(file->queue);
                file = NULL;
            }
            (void)OS_RestoreInterrupts(bak_psr);
        }
        if (!file)
        {
            break;
        }
        // If processing is asynchronous, quit for the time being
        else if (FSi_InvokeCommand(file, FSi_GetCurrentCommand(file)) == FS_RESULT_PROC_ASYNC)
        {
            break;
        }
        // If the result is a synchronous completion, select the continuation here
        else
        {
            file = FSi_NextCommand(arc, TRUE);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_ExecuteSyncCommand

  Description:  Executes a command that was blocked.

  Arguments:    file: FSFile structure that stores the command to execute

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_ExecuteSyncCommand(FSFile *file)
{
    // If necessary, wait for turn, then process the command
    FSi_WaitConditionChange(&file->stat, FS_FILE_STATUS_OPERATING, FS_FILE_STATUS_BUSY, file->queue);
    // If you do not have the execution rights for some reason (for example, the command has already been cancelled), do nothing
    if ((file->stat & FS_FILE_STATUS_OPERATING) != 0)
    {
        FSArchive   * const arc = file->arc;
        FSResult            result;
        result = FSi_InvokeCommand(file, FSi_GetCurrentCommand(file));
        FSi_EndCommand(file, result);
        // If there is an asynchronous type command to be processed here, execute instead
        file = FSi_NextCommand(arc, TRUE);
        if (file)
        {
            FSi_ExecuteAsyncCommand(file);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_SendCommand

  Description:  Issues a command to an archive.
                Adjust start timing and block here if synchronous.

  Arguments:    file: FSFile structure in which the command argument has been specified
                Command: Command ID
                blocking: TRUE if blocking

  Returns:      TRUE if the command is successful.
 *---------------------------------------------------------------------------*/
BOOL FSi_SendCommand(FSFile *file, FSCommandType command, BOOL blocking)
{
    BOOL                retval = FALSE;
    FSArchive   * const arc = file->arc;
    BOOL                owner = FALSE;

    if (FS_IsBusy(file))
    {
        OS_TPanic("specified file is now still proceccing previous command!");
    }
    if (!arc)
    {
        OS_TWarning("specified handle is not related by any archive\n");
        file->error = FS_RESULT_INVALID_PARAMETER;
        return FALSE;
    }

    // Initialize command
    file->error = FS_RESULT_BUSY;
    file->stat &= ~(FS_FILE_STATUS_CMD_MASK << FS_FILE_STATUS_CMD_SHIFT);
    file->stat |= (command << FS_FILE_STATUS_CMD_SHIFT);
    file->stat |= FS_FILE_STATUS_BUSY;
    file->next = NULL;
    if (blocking)
    {
        file->stat |= FS_FILE_STATUS_BLOCKING;
    }
    // If unloading, cancel the process and if not, add to the end of the list
    {
        OSIntrMode          bak_psr = OS_DisableInterrupts();
        if ((arc->flag & FS_ARCHIVE_FLAG_UNLOADING) != 0)
        {
            FSi_EndCommand(file, FS_RESULT_CANCELED);
        }
        else
        {
            FSFile    **pp;
            for (pp = &arc->list; *pp; pp = &(*pp)->next)
            {
            }
            *pp = file;
        }
        owner = (arc->list == file) && ((arc->flag & FS_ARCHIVE_FLAG_RUNNING) == 0);
        (void)OS_RestoreInterrupts(bak_psr);
    }
    // If added to the list, check the command execution rights
    if (file->error != FS_RESULT_CANCELED)
    {
        // If there is a command that should be processed here, execute that instead
        FSFile *next = FSi_NextCommand(arc, owner);
        // If necessary, block here
        if (blocking)
        {
            FSi_ExecuteSyncCommand(file);
            retval = FS_IsSucceeded(file);
        }
        // Added to the list, so will be processed by someone
        else
        {
            if (next != NULL)
            {
                FSi_ExecuteAsyncCommand(next);
            }
            retval = TRUE;
        }
    }

    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_EndArchive

  Description:  End all archives and release.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FSi_EndArchive(void)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    while (arc_list)
    {
        FSArchive *p_arc = arc_list;
        arc_list = arc_list->next;
        (void)FS_UnloadArchive(p_arc);
        FS_ReleaseArchiveName(p_arc);
    }
    (void)OS_RestoreInterrupts(bak_psr);
}

/*---------------------------------------------------------------------------*
  Name:         FS_FindArchive

  Description:  Searches for archive name.
                If there is no matching name, returns NULL.

  Arguments:    name: String of the archive name to search for
                name_len: Length of name string

  Returns:      Pointer to the archive that was found, or NULL.
 *---------------------------------------------------------------------------*/
FSArchive *FS_FindArchive(const char *name, int name_len)
{
    OSIntrMode  bak_psr = OS_DisableInterrupts();
    FSArchive  *arc = arc_list;
    for (; arc; arc = arc->next)
    {
        if (FS_IsArchiveLoaded(arc))
        {
            const char *arcname = FS_GetArchiveName(arc);
            if ((STD_CompareNString(arcname, name, name_len) == 0) && (arcname[name_len] == '\0'))
            {
                break;
            }
        }
    }
    (void)OS_RestoreInterrupts(bak_psr);
    return arc;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveResultCode

  Description:  Gets latest error code for the specified archive.

  Arguments:    path_or_archive  Specify targeted archive
                                 FSArchive structure or path string

  Returns:      Latest error code for the specified archive.
                If the corresponding archive does not exist, FS_RESULT_ERROR.
 *---------------------------------------------------------------------------*/
FSResult FS_GetArchiveResultCode(const void *path_or_archive)
{
    OSIntrMode  bak_psr = OS_DisableInterrupts();
    FSArchive  *arc = arc_list;
    while (arc && (arc != (const FSArchive *)path_or_archive))
    {
        arc = arc->next;
    }
    if (!arc)
    {
        arc = FS_NormalizePath((const char *)path_or_archive, NULL, NULL);
    }
    (void)OS_RestoreInterrupts(bak_psr);
    return arc ? arc->result : FS_RESULT_ERROR;
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetCurrentDirectoryPath

  Description:  Gets current directory with path name.

  Arguments:    None.

  Returns:      String that represents current directory.
 *---------------------------------------------------------------------------*/
const char *FS_GetCurrentDirectory(void)
{
    return current_dir_path;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetCurrentDirectory

  Description:  Changes the current directory.

  Arguments:    path: Path name

  Returns:      TRUE if successful.
 *---------------------------------------------------------------------------*/
BOOL FS_SetCurrentDirectory(const char *path)
{
    BOOL        retval = FALSE;
    FSArchive  *arc = NULL;
    u32         baseid = 0;
    char        relpath[FS_ENTRY_LONGNAME_MAX];

    SDK_NULL_ASSERT(path);
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    // Get normalized path name
    arc = FS_NormalizePath(path, &baseid, relpath);
    if (arc)
    {
        // In any case, the setting was successful
        current_dir_pos.arc = arc;
        current_dir_pos.own_id = 0;
        current_dir_pos.index = 0;
        current_dir_pos.pos = 0;
        (void)STD_CopyLString(current_dir_path, relpath, sizeof(current_dir_path));
        // If possible, try to get directory ID
        if (arc->vtbl->FindPath != NULL)
        {
            FSFile                  dir[1];
            FSArgumentForFindPath   arg[1];
            FS_InitFile(dir);
            dir->arc = arc;
            dir->argument = arg;
            arg->baseid = baseid;
            arg->relpath = relpath;
            arg->target_is_directory = TRUE;
            if (FSi_SendCommand(dir, FS_COMMAND_FINDPATH, TRUE))
            {
                current_dir_pos.own_id = (u16)arg->target_id;
                (void)STD_CopyLString(current_dir_path, relpath, sizeof(current_dir_path));
            }
        }
        retval = TRUE;
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_CopySafeString

  Description:  Checks buffer size and copies string.

  Arguments:    dst: Transfer destination buffer
                dstlen: Transfer destination size
                src: Transfer source buffer
                srclen: Transfer destination size
                stickyFailure: FALSE, if truncated at transfer origin
                
  Returns:      Archive pointer or NULL.
 *---------------------------------------------------------------------------*/
static int FSi_CopySafeString(char *dst, int dstlen, const char *src, int srclen, BOOL *stickyFailure)
{
    int     i;
    int     n = (dstlen - 1 < srclen) ? (dstlen - 1) : srclen;
    for (i = 0; (i < n) && src[i]; ++i)
    {
        dst[i] = src[i];
    }
    if ((i < srclen) && src[i])
    {
        *stickyFailure = TRUE;
    }
    dst[i] = '\0';
    return i;
}

/*---------------------------------------------------------------------------*
  Name:         FS_NormalizePath

  Description:  Normalizes to format of (Standard Directory ID) + (Path Name) considering the current directory in the specified path.
                
                

  Arguments:    path: Non-normalized path string
                baseid: Standard directory ID storage destination or NULL
                relpath: Path name storage destination after conversion or NULL

  Returns:      Archive pointer or NULL.
 *---------------------------------------------------------------------------*/
FSArchive* FS_NormalizePath(const char *path, u32 *baseid, char *relpath)
{
    FSArchive  *arc = NULL;
    int         pathlen = 0;
    int         pathmax = FS_ENTRY_LONGNAME_MAX;
    BOOL        stickyFailure = FALSE;
    // Reevaluate when current directory is not specified
    if (current_dir_pos.arc == NULL)
    {
        current_dir_pos.arc = arc_list;
        current_dir_pos.own_id = 0;
        current_dir_pos.pos = 0;
        current_dir_pos.index = 0;
        current_dir_path[0] = '\0';
    }
    // With "/path" make the current archive route the base
    if (FSi_IsSlash((u8)*path))
    {
        arc = current_dir_pos.arc;
        ++path;
        if (baseid)
        {
            *baseid = 0;
        }
    }
    else
    {
        int     i;
        for (i = 0; ; i = FSi_IncrementSjisPosition(path, i))
        {
            u32     c = (u8)path[i];
            // With "(path/)*path" the relevant path from the current position
            if (!c || FSi_IsSlash(c))
            {
                arc = current_dir_pos.arc;
                if (baseid)
                {
                    *baseid = current_dir_pos.own_id;
                }
                if(relpath)
                {
                    // If directory is retained only by the path name, link it
                    if ((current_dir_pos.own_id == 0) && (current_dir_path[0] != '\0'))
                    {
                        pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                      current_dir_path, FS_ENTRY_LONGNAME_MAX, &stickyFailure);
                        pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                      "/", 1, &stickyFailure);
                    }
                }
                break;
            }
            // With "arc:/path," the full path
            else if (c == ':')
            {
                arc = FS_FindArchive(path, i);
                if (!arc)
                {
                    OS_TWarning("archive \"%*s\" is not found.", i, path);
                }
                path += i + 1;
                if (FSi_IsSlash((u8)*path))
                {
                    ++path;
                }
                if (baseid)
                {
                    *baseid = 0;
                }
                break;
            }
        }
    }
    if(relpath)
    {
        // Normalize relative paths, being careful of specialized entry names.
        int     curlen = 0;
        while (!stickyFailure)
        {
            char    c = path[curlen];
            if ((c != '\0') && !FSi_IsSlash((u8)c))
            {
                curlen += STD_IsSjisCharacter(&path[curlen]) ? 2 : 1;
            }
            else
            {
                // Ignore empty directory
                if (curlen == 0)
                {
                }
                // Ignore "." (current directory)
                else if ((curlen == 1) && (path[0] == '.'))
                {
                }
                // ".." (parent directory) raises the root one level as the upper limit
                else if ((curlen == 2) && (path[0] == '.') && (path[1] == '.'))
                {
                    if (pathlen > 0)
                    {
                        --pathlen;
                    }
                    pathlen = FSi_DecrementSjisPositionToSlash(relpath, pathlen) + 1;
                }
                // Add entry for anything else
                else
                {
                    pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                  path, curlen, &stickyFailure);
                    if (c != '\0')
                    {
                        pathlen += FSi_CopySafeString(&relpath[pathlen], pathmax - pathlen,
                                                      "/", 1, &stickyFailure);
                    }
                }
                if (c == '\0')
                {
                    break;
                }
                path += curlen + 1;
                curlen = 0;
            }
        }
        relpath[pathlen] = '\0';
        pathlen = FSi_TrimSjisTrailingSlash(relpath);
    }
    return stickyFailure ? NULL : arc;
}

/*---------------------------------------------------------------------------*
  Name:         FS_InitArchive

  Description:  Initializes archive structure.

  Arguments:    p_arc: Archive to initialize

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_InitArchive(FSArchive *p_arc)
{
    SDK_NULL_ASSERT(p_arc);
    MI_CpuClear8(p_arc, sizeof(FSArchive));
    OS_InitThreadQueue(&p_arc->queue);
}

/*---------------------------------------------------------------------------*
  Name:         FS_RegisterArchiveName

  Description:  Registers the archive name in the file system and associates it.
                The archive itself is not yet loaded into the file system.
                The archive name "rom" is reserved by the file system.

  Arguments:    p_arc: Archive to associate with the name
                name: String of the name to register
                name_len: Length of name string

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL FS_RegisterArchiveName(FSArchive *p_arc, const char *name, u32 name_len)
{
    BOOL    retval = FALSE;

    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(p_arc);
    SDK_NULL_ASSERT(name);

    {
        OSIntrMode bak_intr = OS_DisableInterrupts();
        if (!FS_FindArchive(name, (s32)name_len))
        {
            // Adds to the end of the list
            FSArchive **pp;
            for (pp = &arc_list; *pp; pp = &(*pp)->next)
            {
            }
            *pp = p_arc;
            // With the current specifications, the archive has a maximum of 3 characters
            if (name_len <= FS_ARCHIVE_NAME_LEN_MAX)
            {
                p_arc->name.pack = 0;
                (void)STD_CopyLString(p_arc->name.ptr, name, (int)(name_len + 1));
            }
            else
            {
#if defined(FS_SUPPORT_LONG_ARCNAME)
            // Feature for expanding the limit of the number of characters in an archive name
            // It is necessary to allocate external memory to support without changing the FSArchive structure size
            // 
                if (name_len <= FS_LONG_ARCNAME_LENGTH_MAX)
                {
                    int i;
                    for (i = 0; ; ++i)
                    {
                        if (i >= FS_LONG_ARCNAME_TABLE_MAX)
                        {
                            OS_TPanic("failed to allocate memory for long archive-name(%.*s)!", name_len, name);
                        }
                        else if (FSiLongNameTable[i][0] == '\0')
                        {
                            (void)STD_CopyLString(FSiLongNameTable[i], name, (int)(name_len + 1));
                            p_arc->name.pack = (u32)FSiLongNameTable[i];
                            break;
                        }
                    }
                }
#endif
                // Cannot register archive names that are too long
                else
                {
                    OS_TPanic("too long archive-name(%.*s)!", name_len, name);
                }
            }
            p_arc->flag |= FS_ARCHIVE_FLAG_REGISTER;
            retval = TRUE;
        }
        (void)OS_RestoreInterrupts(bak_intr);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ReleaseArchiveName

  Description:  Release registered archive names.
                They must be unloaded from the file system.

  Arguments:    p_arc: Archive with name to release

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_ReleaseArchiveName(FSArchive *p_arc)
{
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(p_arc);

	if(p_arc == arc_list)
    {
        OS_TPanic("[file-system] cannot modify \"rom\" archive.\n");
    }

    if (p_arc->name.pack)
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        // Cut from the list
        FSArchive **pp;
        for (pp = &arc_list; *pp; pp = &(*pp)->next)
        {
            if(*pp == p_arc)
            {
                *pp = (*pp)->next;
                break;
            }
        }
#if defined(FS_SUPPORT_LONG_ARCNAME)
        // Deallocate buffer to the system if there is a long archive name
        if (p_arc->name.ptr[3] != '\0')
        {
            ((char *)p_arc->name.pack)[0] = '\0';
        }
#endif
        p_arc->name.pack = 0;
        p_arc->next = NULL;
        p_arc->flag &= ~FS_ARCHIVE_FLAG_REGISTER;
        // Deallocate if it is the current archive
        if (current_dir_pos.arc == p_arc)
        {
            current_dir_pos.arc = NULL;
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetArchiveName

  Description:  Gets archive name.

  Arguments:    p_arc: Archive whose name to get

  Returns:      Archive name registered in the file system.
 *---------------------------------------------------------------------------*/
const char *FS_GetArchiveName(const FSArchive *arc)
{
#if defined(FS_SUPPORT_LONG_ARCNAME)
    return (arc->name.ptr[3] != '\0') ? (const char *)arc->name.pack : arc->name.ptr;
#else
    return arc->name.ptr;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         FS_MountArchive

  Description:  Mounts the archive.

  Arguments:    arc: Archive to mount
                userdata: User-defined variable that associates with an archive
                vtbl: Command interface
                reserved: Reserved for future use (always specify 0)


  Returns:      TRUE if mounting is successful.
 *---------------------------------------------------------------------------*/
BOOL FS_MountArchive(FSArchive *arc, void *userdata,
                     const FSArchiveInterface *vtbl, u32 reserved)
{
    (void)reserved;
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);
    SDK_ASSERT(!FS_IsArchiveLoaded(arc));
    // Initialize archive with new specifications
    arc->userdata = userdata;
    arc->vtbl = vtbl;
    // Issue mounting notification event
    {
        FSFile  tmp[1];
        FS_InitFile(tmp);
        tmp->arc = arc;
        (void)FSi_InvokeCommand(tmp, FS_COMMAND_MOUNT);
    }
    arc->flag |= FS_ARCHIVE_FLAG_LOADED;
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_UnmountArchive

  Description:  Unmounts the archive.

  Arguments:    arc: Archive to unmount

  Returns:      TRUE if unmounting is successful.
 *---------------------------------------------------------------------------*/
BOOL    FS_UnmountArchive(FSArchive *arc)
{
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);

    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        // Ignore if it is not loaded
        if (FS_IsArchiveLoaded(arc))
        {
            // Cancel all idling commands
            {
                BOOL    bak_state = FS_SuspendArchive(arc);
                FSFile *file = arc->list;
                arc->flag |= FS_ARCHIVE_FLAG_UNLOADING;
                while (file)
                {
                    FSFile *next = file->next;
                    FSi_EndCommand(file, FS_RESULT_CANCELED);
                    file = next;
                }
                arc->list = NULL;
                if (bak_state)
                {
                    (void)FS_ResumeArchive(arc);
                }
            }
            // Issue unmounting notification event
            {
                FSFile  tmp[1];
                FS_InitFile(tmp);
                tmp->arc = arc;
                (void)FSi_InvokeCommand(tmp, FS_COMMAND_UNMOUNT);
            }
            arc->flag &= ~(FS_ARCHIVE_FLAG_CANCELING |
                           FS_ARCHIVE_FLAG_LOADED | FS_ARCHIVE_FLAG_UNLOADING);
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
    return TRUE;

}

/*---------------------------------------------------------------------------*
  Name:         FS_SuspendArchive

  Description:  Stops archive processing mechanism itself.
                If a process is currently executing, waits for it to complete.

  Arguments:    p_arc: Archive to suspend

  Returns:      TRUE if it was not in a suspended state before the call.
 *---------------------------------------------------------------------------*/
BOOL FS_SuspendArchive(FSArchive *p_arc)
{
    BOOL    retval = FALSE;

    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(p_arc);

    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        retval = !FS_IsArchiveSuspended(p_arc);
        if (retval)
        {
            if ((p_arc->flag & FS_ARCHIVE_FLAG_RUNNING) == 0)
            {
                p_arc->flag |= FS_ARCHIVE_FLAG_SUSPEND;
            }
            else
            {
                p_arc->flag |= FS_ARCHIVE_FLAG_SUSPENDING;
                FSi_WaitConditionOff(&p_arc->flag, FS_ARCHIVE_FLAG_SUSPENDING, &p_arc->queue);
            }
        }
        (void)OS_RestoreInterrupts(bak_psr);
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FS_ResumeArchive

  Description:  Resumes the suspended archive processing.

  Arguments:    arc: Archive to reopen

  Returns:      TRUE if it was not in a suspended state before the call.
 *---------------------------------------------------------------------------*/
BOOL FS_ResumeArchive(FSArchive *arc)
{
    BOOL    retval;
    SDK_ASSERT(FS_IsAvailable());
    SDK_NULL_ASSERT(arc);
    {
        OSIntrMode bak_irq = OS_DisableInterrupts();
        retval = !FS_IsArchiveSuspended(arc);
        if (!retval)
        {
            arc->flag &= ~FS_ARCHIVE_FLAG_SUSPEND;
        }
        (void)OS_RestoreInterrupts(bak_irq);
    }
    {
        FSFile *file = NULL;
        file = FSi_NextCommand(arc, TRUE);
        if (file)
        {
            FSi_ExecuteAsyncCommand(file);
        }
    }
    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         FS_NotifyArchiveAsyncEnd

  Description:  This is called from the archive implementation to send a notification when asynchronous archive processing is complete.
                

  Arguments:    arc: Archive for which to notify completion
                ret: Processing result

  Returns:      None.
 *---------------------------------------------------------------------------*/
void FS_NotifyArchiveAsyncEnd(FSArchive *arc, FSResult ret)
{
    FSFile     *file = arc->list;
    if ((file->stat & FS_FILE_STATUS_BLOCKING) != 0)
    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        file->stat |= FS_FILE_STATUS_ASYNC_DONE;
        file->error = ret;
        OS_WakeupThread(file->queue);
        (void)OS_RestoreInterrupts(bak_psr);
    }
    else
    {
        FSi_EndCommand(file, ret);
        file = FSi_NextCommand(arc, TRUE);
        if (file)
        {
            FSi_ExecuteAsyncCommand(file);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         FS_WaitAsync

  Description:  Waits for end of asynchronous function and returns result.

  Arguments:    File to wait

  Returns:      If succeeded, TRUE.
 *---------------------------------------------------------------------------*/
BOOL FS_WaitAsync(FSFile *file)
{
    SDK_NULL_ASSERT(file);
    SDK_ASSERT(FS_IsAvailable());
    SDK_ASSERT(OS_GetProcMode() != OS_PROCMODE_IRQ);

    {
        BOOL    is_owner = FALSE;
        OSIntrMode bak_psr = OS_DisableInterrupts();
        if (FS_IsBusy(file))
        {
            // Take on processing here if an unprocessed asynchronous mode
            is_owner = !(file->stat & (FS_FILE_STATUS_BLOCKING | FS_FILE_STATUS_OPERATING));
            if (is_owner)
            {
                file->stat |= FS_FILE_STATUS_BLOCKING;
            }
        }
        (void)OS_RestoreInterrupts(bak_psr);
        if (is_owner)
        {
            FSi_ExecuteSyncCommand(file);
        }
        else
        {
            FSi_WaitConditionOff(&file->stat, FS_FILE_STATUS_BUSY, file->queue);
        }
    }

    return FS_IsSucceeded(file);
}


#endif /* FS_IMPLEMENT */
