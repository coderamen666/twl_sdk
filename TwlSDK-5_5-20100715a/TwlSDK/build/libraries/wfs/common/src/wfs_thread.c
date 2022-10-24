/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WFS - libraries
  File:     wfs_thread.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

 *---------------------------------------------------------------------------*/


#include <nitro.h>


/*---------------------------------------------------------------------------*/
/* Constants */

/*
 * Various ROM cache setting values.
 * [Page size]
 * - An integer multiple of the ROM page size (512 bytes) is desirable.
 * - A value that is too small will increase the ROM access overhead.
 * - A value that is too large will cause server ROM accesses (FS/SND/...) to become blocked at irregular intervals.
 * [Page count]
 * - If large, this will contribute to stabilizing transfer efficiency (the responsiveness of resend processing in restricted communication environments).
 * - A value larger than the total number of clients is desirable if access locality is high.
 */
#define	WFS_FILE_CACHE_SIZE	    1024UL
#define WFS_CACHE_PAGE_TOTAL    8


/*---------------------------------------------------------------------------*/
/* Declarations */

typedef struct WFSServerThread
{
    WFSEventCallback    hook;
    MIDevice            device[1];
    FSFile              file[1];
    MICache             cache[1];
    u8                  cache_buf[MI_CACHE_BUFFER_WORKSIZE(
                                  WFS_FILE_CACHE_SIZE, WFS_CACHE_PAGE_TOTAL)];
    OSMessageQueue      msg_queue[1];
    OSMessage           msg_array[1];
    OSThread            thread[1];
    u8                  thread_stack[0x1000];
}
WFSServerThread;


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         WFSi_ReadRomCallback

  Description:  ROM device read callback

  Arguments:    userdata         WFSServerThread structure
                buffer           The transfer target memory.
                offset           The transfer source offset.
                length           The transfer size.

  Returns:      Returns a value of 0 or higher if successful; a negative number otherwise.
 *---------------------------------------------------------------------------*/
static int WFSi_ReadRomCallback(void *userdata, void *buffer, u32 offset, u32 length)
{
    WFSServerThread * const thread = (WFSServerThread*)userdata;

    /* NITRO-SDK specific: Read from memory because the first 32 KB cannot be accessed. */
    if(offset < sizeof(CARDRomRegion))
    {
        const u8 *header = (const u8 *)CARD_GetOwnRomHeader();
        if (length > sizeof(CARDRomHeader) - offset)
        {
            length = sizeof(CARDRomHeader) - offset;
        }
        MI_CpuCopy8(header + offset, buffer, length);
    }
#if defined(SDK_TWL)
    // Unique to TWL-SDK:  Do not send data from the TWL-exclusive region to the network carelessly.
    else if (OS_IsRunOnTwl() && ((((const u8 *)HW_TWL_ROM_HEADER_BUF)[0x1C] & 0x01) != 0) &&
             ((offset >= CARD_GetOwnRomHeader()->rom_size) ||
             (offset + length > CARD_GetOwnRomHeader()->rom_size)))
    {
        MI_CpuFill8(buffer, 0x00, length);
    }
#endif
    else
    {
        if (offset < 0x8000)
        {
            u32     memlen = length;
            if (memlen > 0x8000 - offset)
            {
                memlen = 0x8000 - offset;
            }
            MI_CpuFill8(buffer, 0x00, length);
            offset += memlen;
            length -= memlen;
        }
        if (length > 0)
        {
            (void)FS_SeekFile(thread->file, (int)offset, FS_SEEK_SET);
            (void)FS_ReadFile(thread->file, buffer, (int)length);
        }
    }
    return (int)length;
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_WriteRomCallback

  Description:  ROM device write callback (dummy)

  Arguments:    userdata         WFSServerThread structure
                buffer           The transfer target memory.
                offset           The transfer source offset.
                length           The transfer size.

  Returns:      Returns a value of 0 or higher if successful; a negative number otherwise.
 *---------------------------------------------------------------------------*/
static int WFSi_WriteRomCallback(void *userdata, const void *buffer, u32 offset, u32 length)
{
    (void)userdata;
    (void)buffer;
    (void)offset;
    (void)length;
    return -1;
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_ServerThreadProc

  Description:  Thread procedure for the WFS server.

  Arguments:    arg             WFSServerThread structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_ServerThreadProc(void *arg)
{
    WFSServerThread *const thread = (WFSServerThread*)arg;
    for (;;)
    {
        BOOL    busy = FALSE;
        (void)OS_ReceiveMessage(thread->msg_queue, (OSMessage*)&busy, OS_MESSAGE_BLOCK);
        if (!busy)
        {
            break;
        }
        MI_LoadCache(thread->cache, thread->device);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFSi_ThreadHook

  Description:  A hook function set in the WFS server segment callback.

  Arguments:    work            WFSServerThread structure
                argument        WFSSegmentBuffer structure
                                Completion notification if NULL.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void WFSi_ThreadHook(void *work, void *argument)
{
    WFSServerThread * const thread = (WFSServerThread *)work;
    WFSSegmentBuffer * const segment = (WFSSegmentBuffer*)argument;
    /* Completion notification if NULL */
    if (!segment)
    {
        (void)OS_SendMessage(thread->msg_queue, (OSMessage)FALSE, OS_MESSAGE_BLOCK);
        OS_JoinThread(thread->thread);
        (void)FS_CloseFile(thread->file);
    }
    /* Segment request (or notification of preparation) if not NULL */
    else if (!MI_ReadCache(thread->cache, segment->buffer, segment->offset, segment->length))
    {
        /* Instruct thread to read in case of a cache miss */
        segment->buffer = NULL; /* == "could not prepare immediately" */
        // NOBLOCK, because it is acceptable for notifications not to line up in the queue if the same ones have accumulated.
        (void)OS_SendMessage(thread->msg_queue, (OSMessage)TRUE, OS_MESSAGE_NOBLOCK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         WFS_ExecuteRomServerThread

  Description:  Registers the specified ROM file to the WFS library so that it will be distributed, and internally automatically starts up the ROM access thread.
                
                These threads are automatically destroyed in the WFS_EndServer function.

  Arguments:    context:          The WFSServerContext structure.
                file:             The SRL file that contains the file system to be distributed.
                                 Specify NULL for clone boot.
                sharedFS:         TRUE if the file system is to be shared by the parent and child.
                                 In such a case, a combined file system is distributed. For this combined file system, only the overlay held by 'file' is extracted and appended to the file system of the parent device itself.
                                 
                                 
                                 If 'file' is set to NULL, it specifies clone boot, so this argument is ignored. (It is interpreted as always TRUE.) 
                                 


  Returns:      TRUE if ROM file registration and thread creation succeed.
 *---------------------------------------------------------------------------*/
BOOL WFS_ExecuteRomServerThread(WFSServerContext *context, FSFile *file, BOOL sharedFS)
{
    BOOL    retval = FALSE;

    WFSServerThread *thread = MI_CallAlloc(context->allocator, sizeof(WFSServerThread), 32);
    if (!thread)
    {
        OS_TWarning("failed to allocate memory enough to create internal thread.");
    }
    else
    {
        /* Determine whether this is single ROM mode, clone boot mode, or shared FS mode */
        u32     pos = file ? (FS_GetFileImageTop(file) + FS_GetFilePosition(file)) : 0;
        u32     fatbase = (u32)((file && !sharedFS) ? pos : 0);
        u32     overlay = (u32)(file ? pos : 0);
        /* Initialize the ROM access device and cache */
        FS_InitFile(thread->file);
        if (!FS_CreateFileFromRom(thread->file, 0, 0x7FFFFFFF))
        {
            OS_TPanic("failed to create ROM-file!");
        }
        MI_InitDevice(thread->device, thread,
                      WFSi_ReadRomCallback, WFSi_WriteRomCallback);
        MI_InitCache(thread->cache, WFS_FILE_CACHE_SIZE,
                     thread->cache_buf, sizeof(thread->cache_buf));
        /* Register file table */
        if (WFS_RegisterServerTable(context, thread->device, fatbase, overlay))
        {
            /* Specify a hook and start a thread if the process was successful */
            context->thread_work = thread;
            context->thread_hook = WFSi_ThreadHook;
            OS_InitMessageQueue(thread->msg_queue, thread->msg_array, 1);
            OS_CreateThread(thread->thread, WFSi_ServerThreadProc, thread,
                            thread->thread_stack + sizeof(thread->thread_stack),
                            sizeof(thread->thread_stack), 15);
            OS_WakeupThreadDirect(thread->thread);
            retval = TRUE;
        }
        else
        {
            MI_CallFree(context->allocator, thread);
        }
    }
    return retval;
}


/*---------------------------------------------------------------------------*
  $Log: wfs_thread.c,v $
  Revision 1.1  2007/06/14 13:14:46 PM  yosizaki
  Initial upload.

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
