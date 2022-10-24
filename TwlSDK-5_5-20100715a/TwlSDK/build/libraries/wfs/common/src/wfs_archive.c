/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WFS - libraries
  File:     wfs_archive.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/


#include <nitro/wfs/client.h>


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WFSi_ArchiveReadCallback

  Description:  Child WFS archive read access callback.

  Arguments:    archive: FSArchive structure
                buffer: Transfer destination
                offset: Transfer source
                length: Transfer size

  Returns:      Process result.
 *---------------------------------------------------------------------------*/
static FSResult WFSi_ArchiveReadCallback(FSArchive *archive, void *buffer, u32 offset, u32 length)
{
    FSResult    result = FS_RESULT_ERROR;
    WFSClientContext * const context = (WFSClientContext*)FS_GetArchiveBase(archive);
    const WFSTableFormat * const table = WFS_GetTableFormat(context);
    if (table)
    {
        MI_CpuCopy8(&table->buffer[offset], buffer, length);
        result = FS_RESULT_SUCCESS;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_ArchiveReadDoneCallback

  Description:  Child WFS archive read completion callback.

  Arguments:    context: WFSClientContext structure
                succeeded: TRUE if the read succeeded, FALSE if it failed
                arg: Argument specified in the callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_ArchiveReadDoneCallback(WFSClientContext *context, BOOL succeeded, void *arg)
{
    FSFile * const      file = (FSFile*)arg;
    FSResult            result = FS_RESULT_ERROR;
    if (succeeded)
    {
        file->prop.file.pos += file->arg.readfile.len;
        result = FS_RESULT_SUCCESS;
    }
    FS_NotifyArchiveAsyncEnd(FS_GetAttachedArchive(file), result);
    (void)context;
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_RomArchiveProc

  Description:  User procedure for a child ROM archive.

  Arguments:    file: FSFile structure
                command: Command

  Returns:      Process result.
 *---------------------------------------------------------------------------*/
static FSResult WFSi_RomArchiveProc(FSFile *file, FSCommandType command)
{
    FSResult    result = FS_RESULT_ERROR;
    switch (command)
    {
    case FS_COMMAND_READFILE:
        {
            void   *buffer = file->arg.readfile.dst;
            u32     offset = file->prop.file.pos;
            u32     length = file->arg.readfile.len;
            if (length == 0)
            {
                result = FS_RESULT_SUCCESS;
            }
            else
            {
                FSArchive * const archive = FS_GetAttachedArchive(file);
                WFSClientContext * const context = (WFSClientContext*)FS_GetArchiveBase(archive);
                const WFSTableFormat * const table = WFS_GetTableFormat(context);
                if (table != NULL)
                {
                    WFS_RequestClientRead(context, buffer, offset, length,
                                          WFSi_ArchiveReadDoneCallback, file);
                    result = FS_RESULT_PROC_ASYNC;
                }
            }
        }
        break;
    case FS_COMMAND_WRITEFILE:
        result = FS_RESULT_UNSUPPORTED;
        break;
    default:
        result = FS_RESULT_PROC_UNKNOWN;
        break;
    }
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_EmptyArchiveProc

  Description:  This is an empty procedure that follows a call to WFS_EndClient().

  Arguments:    file: FSFile structure
                command: Command

  Returns:      Processing result.
 *---------------------------------------------------------------------------*/
static FSResult WFSi_EmptyArchiveProc(FSFile *file, FSCommandType command)
{
    FSResult    result = FS_RESULT_PROC_UNKNOWN;
    // Output a warning message and generate an error for commands that cannot be ignored without causing a problem.
    if ((command != FS_COMMAND_CLOSEFILE) &&
        ((command < FS_COMMAND_STATUS_BEGIN) || (command >= FS_COMMAND_STATUS_END)))
    {
        static BOOL once = FALSE;
        if (!once)
        {
            once = TRUE;
            OS_TWarning("WFS-Client has been finalized. (all the commands will fail)\n");
        }
        result = FS_RESULT_ERROR;
    }
    (void)file;
    return result;
}


/*---------------------------------------------------------------------------*
  Name:         WFSi_SubThreadForEndClient

  Description:  Archive configuration thread in line with WFS_EndClient.

  Arguments:    arg: Unused

  Returns:      None.
 *---------------------------------------------------------------------------*/
#define WFSi_STACK_SIZE         512
#define WFSi_STACK_PRIORITY     0
#define WFSi_MESG_QUEUE_DEPTH   1

static OSThread       WFSi_SubThread;
static OSMessageQueue WFSi_MesgQueue;
static OSMessage      WFSi_MesgArray[ WFSi_MESG_QUEUE_DEPTH ];
static u64            WFSi_Stack[ WFSi_STACK_SIZE / sizeof(u64) ];

static void WFSi_SubThreadForEndClient(void* arg);
static void WFSi_SubThreadForEndClient(void* arg)
{
#pragma unused(arg)
    FSArchive* archive;
    OSMessage mesg;

    (void)OS_ReceiveMessage(&WFSi_MesgQueue, &mesg, OS_MESSAGE_BLOCK);
    archive = (FSArchive*)mesg;

    (void)FS_SuspendArchive(archive);
    FS_SetArchiveProc(archive, WFSi_EmptyArchiveProc, (u32)FS_ARCHIVE_PROC_ALL);
    (void)FS_ResumeArchive(archive);

    OS_ExitThread();
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_SetThreadForEndClient

  Description:  WFSi_SubThreadForEndClient thread settings.

  Arguments:    archive:          ROM archive WFSClientContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_SetThreadForEndClient(FSArchive* archive)
{
    OSMessage mesg = (OSMessage)archive;

    OS_InitMessageQueue( &WFSi_MesgQueue, &WFSi_MesgArray[0], WFSi_MESG_QUEUE_DEPTH );
    OS_CreateThread( &WFSi_SubThread,
                     WFSi_SubThreadForEndClient,
                     (void*)0,
                     WFSi_Stack+WFSi_STACK_SIZE/sizeof(u64),
                     WFSi_STACK_SIZE,
                     WFSi_STACK_PRIORITY );
    OS_WakeupThreadDirect( &WFSi_SubThread );
    (void)OS_SendMessage( &WFSi_MesgQueue, mesg, OS_MESSAGE_BLOCK );
}


/*---------------------------------------------------------------------------*
  Name:         WFSi_RestoreRomArchive

  Description:  This is an unmount callback invoked when WFS_EndClient() is called.

  Arguments:    context: WFSClientContext structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_RestoreRomArchive(WFSClientContext *context)
{
    FSArchive *archive = FS_FindArchive("rom", 3);

    //Only create thread and send message
    WFSi_SetThreadForEndClient(archive);

    (void)context;
}

/*---------------------------------------------------------------------------*
  Name:         WFS_ReplaceRomArchive

  Description:  Mounts the WFS archive.

  Arguments:    context: WFSClientContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WFS_ReplaceRomArchive(WFSClientContext *context)
{
    const WFSTableFormat * const table = WFS_GetTableFormat(context);
    if (table != NULL)
    {
        FSArchive *archive = FS_FindArchive("rom", 3);
        if (FS_IsArchiveLoaded(archive))
        {
            (void)FS_UnloadArchive(archive);
        }
        FS_SetArchiveProc(archive, WFSi_RomArchiveProc, (u32)FS_ARCHIVE_PROC_ALL);
        context->unmount_callback = WFSi_RestoreRomArchive;
        (void)FS_LoadArchive(archive, (u32)context,
                             table->region[WFS_TABLE_REGION_FAT].offset,
                             table->region[WFS_TABLE_REGION_FAT].length,
                             table->region[WFS_TABLE_REGION_FNT].offset,
                             table->region[WFS_TABLE_REGION_FNT].length,
                             WFSi_ArchiveReadCallback, NULL);
        FS_AttachOverlayTable(MI_PROCESSOR_ARM9,
                              table->buffer +
                              table->region[WFS_TABLE_REGION_OV9].offset,
                              table->region[WFS_TABLE_REGION_OV9].length);
        FS_AttachOverlayTable(MI_PROCESSOR_ARM7,
                              table->buffer +
                              table->region[WFS_TABLE_REGION_OV7].offset,
                              table->region[WFS_TABLE_REGION_OV7].length);
    }
}


/*---------------------------------------------------------------------------*
  $Log: wfs_archive.c,v $
  Revision 1.1  2007/06/11 06:38:39  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
