/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     fs_proc_mem.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-12-04#$
  $Rev: 9513 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro/fs/romfat.h>
#include <nitro/fs/api.h>

#include "../include/util.h"
#include "../include/command.h"


#if defined(FS_IMPLEMENT)

/*---------------------------------------------------------------------------*/
/* Variables */

// Memory mapped file archive
static FSArchive fsi_arc_mem;
static BOOL fsi_mem_init;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FSi_MemArchiveProc

  Description:  Memory mapped file archive procedure

  Arguments:    p_file:           FSFile structure that stores command information.
                cmd:              Command type.

  Returns:      Command processing result.
 *---------------------------------------------------------------------------*/
static FSResult FSi_MemArchiveProc(FSFile *p_file, FSCommandType cmd)
{
#pragma unused(p_file)

    switch (cmd)
    {
    case FS_COMMAND_READDIR:
        {
            FSResult    result;
            FSDirEntry *entry = p_file->arg.readdir.p_entry;
            FSArchive  *current = (FSArchive *)p_file->prop.dir.pos.pos;
            if (current == NULL)
            {
                entry->name_len = 0;
                entry->is_directory = 0;
                result = FS_RESULT_FAILURE;
            }
            else
            {
                const char *arcname = FS_GetArchiveName(current);
                entry->name_len = (u32)(STD_GetStringLength(arcname) + 1);
                entry->is_directory = TRUE;
                if (!p_file->arg.readdir.skip_string)
                {
                    (void)STD_TSPrintf(entry->name, "%s:", arcname);
                    entry->dir_id.arc = p_file->arc;
                    entry->dir_id.own_id = 0;
                    entry->dir_id.index = 0;
                    entry->dir_id.pos = 0;
                }
                result = FS_RESULT_SUCCESS;
                p_file->prop.dir.pos.pos = (u32)current->next;
            }
            return result;
        }
    case FS_COMMAND_SEEKDIR:
    case FS_COMMAND_FINDPATH:
    case FS_COMMAND_GETPATH:
    case FS_COMMAND_OPENFILEFAST:
        return FS_RESULT_UNSUPPORTED;
    default:
        return FS_RESULT_PROC_UNKNOWN;
    }
}

/*---------------------------------------------------------------------------*
  Name:         FSi_InitMemArchive

  Description:  Initializes a memory mapped file archive.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FSi_InitMemArchive(void)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();

    if (!fsi_mem_init || !FS_IsArchiveLoaded(&fsi_arc_mem))
    {
        FS_InitArchive(&fsi_arc_mem);
        FS_SetArchiveProc(&fsi_arc_mem, FSi_MemArchiveProc, (u32)FS_ARCHIVE_PROC_ALL);
        if (!FS_LoadArchive(&fsi_arc_mem, 0, 0, 0, 0, 0, NULL, NULL))
        {
            OS_TPanic("failed to load memory-mapping archive!");
        }
        fsi_mem_init = TRUE;
    }

    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         FS_CreateFileFromMemory

  Description:  Temporarily generates file that maps memory region.

  Arguments:    p_file:           FSFile structure that stores file handle
                buf:              Memory that is target of READ and WRITE
                size:            Byte size of buf

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL FS_CreateFileFromMemory(FSFile *p_file, void *buf, u32 size)
{
    FSi_InitMemArchive();
    return FS_OpenFileDirect(p_file, &fsi_arc_mem, (u32)buf, (u32)buf + size, 0);
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenTopLevelDirectory

  Description:  Opens the special high-level directory that can enumerate the archive name

  Arguments:    dir:           FSFile structure that stores directory handle

  Returns:      If successful, TRUE.
 *---------------------------------------------------------------------------*/
BOOL FS_OpenTopLevelDirectory(FSFile *dir)
{
    FSi_InitMemArchive();
    FS_InitFile(dir);
    dir->prop.dir.pos.pos = (u32)FSi_GetArchiveChain();
    FS_SetDirectoryHandle(dir, &fsi_arc_mem, &dir->prop);
    return TRUE;
}


#endif /* FS_IMPLEMENT */
