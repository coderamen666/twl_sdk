/*---------------------------------------------------------------------------*
  Project:  TwlSDK - library - dsp
  File:     dsp_pipe.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-01-08#$
  $Rev: 11275 $
  $Author: kakemizu_hironori $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <twl/dsp.h>
#include <twl/dsp/common/pipe.h>


/*---------------------------------------------------------------------------*/
/* Variables */

// Base address in DSP for pipe information
static DSPAddrInARM     DSPiPipeMonitorAddress = 0;
static OSThreadQueue    DSPiBlockingQueue[1];
static DSPPipe          DSPiDefaultPipe[DSP_PIPE_PORT_MAX][DSP_PIPE_PEER_MAX];
static DSPPipeCallback  DSPiCallback[DSP_PIPE_PORT_MAX];
static void*           (DSPiCallbackArgument[DSP_PIPE_PORT_MAX]);

// File I/O processing thread structure
typedef struct DSPFileIOContext
{
    BOOL            initialized;
    OSThread        th[1];
    OSMessage       msga[1];
    OSMessageQueue  msgq[1];
    volatile int    pollbits;
    FSFile          file[DSP_PIPE_PORT_MAX][1];
    u8              stack[4096];
}
DSPFileIOContext;

static DSPFileIOContext DSPiThread[1];


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         DSPi_FileIOProc

  Description:  File I/O processing procedure

  Arguments:    arg: DSPFileIOThread structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_FileIOProc(void *arg)
{
    DSPFileIOContext   *ctx = (DSPFileIOContext *)arg;
    for (;;)
    {
        // If necessary, wait for notice and get one port that is requested.
        OSIntrMode  bak = OS_DisableInterrupts();
        int         port;
        for (;;)
        {
            port = (int)MATH_CTZ((u32)ctx->pollbits);
            if (port < DSP_PIPE_PORT_MAX)
            {
                break;
            }
            else
            {
                OSMessage   msg[1];
                (void)OS_ReceiveMessage(ctx->msgq, msg, OS_MESSAGE_BLOCK);
            }
        }
        ctx->pollbits &= ~(1 << port);
        (void)OS_RestoreInterrupts(bak);
        // Read command from corresponding pipe
        {
            FSFile     *file = ctx->file[port];
            DSPPipe     in[1], out[1];
            u16         command;
            (void)DSP_LoadPipe(in, port, DSP_PIPE_INPUT);
            (void)DSP_LoadPipe(out, port, DSP_PIPE_OUTPUT);
            while (DSP_GetPipeReadableSize(in) >= sizeof(command))
            {
                DSP_ReadPipe(in, &command, sizeof(command));
                switch (command)
                {
                case DSP_PIPE_IO_COMMAND_OPEN:
                    // fopen command
                    {
                        u16     mode;
                        u16     len;
                        char    path[FS_ENTRY_LONGNAME_MAX + 1];
                        u16     result;
                        DSP_ReadPipe(in, &mode, sizeof(mode));
                        DSP_ReadPipe(in, &len, sizeof(len));
                        len = DSP_WORD_TO_ARM(len);
                        DSP_ReadPipe(in, path, len);
                        path[len] = '\0';
                        (void)FS_OpenFileEx(file, path, mode);
                        result = (u16)(FS_IsFile(file) ? 1 : 0);
                        DSP_WritePipe(out, &result, sizeof(result));
                    }
                    break;
                case DSP_PIPE_IO_COMMAND_MEMMAP:
                    // memmap command
                    {
                        DSPAddrInARM    addr;
                        DSPAddrInARM    length;
                        u16     result;
                        DSP_ReadPipe(in, &addr, sizeof(addr));
                        DSP_ReadPipe(in, &length, sizeof(length));
                        addr = DSP_32BIT_TO_ARM(addr);
                        length = DSP_32BIT_TO_ARM(length);
                        length = DSP_ADDR_TO_ARM(length);
                        (void)FS_CreateFileFromMemory(file, (void*)addr, length);
                        result = (u16)(FS_IsFile(file) ? 1 : 0);
                        DSP_WritePipe(out, &result, sizeof(result));
                    }
                    break;
                case DSP_PIPE_IO_COMMAND_CLOSE:
                    // fclose command
                    {
                        u16     result;
                        (void)FS_CloseFile(file);
                        result = 1;
                        DSP_WritePipe(out, &result, sizeof(result));
                    }
                    break;
                case DSP_PIPE_IO_COMMAND_SEEK:
                    // fseek command
                    {
                        s32     position;
                        u16     whence;
                        s32     result;
                        DSP_ReadPipe(in, &position, sizeof(position));
                        DSP_ReadPipe(in, &whence, sizeof(whence));
                        position = (s32)DSP_32BIT_TO_ARM(position);
                        (void)FS_SeekFile(file, position, (FSSeekFileMode)whence);
                        result = (s32)FS_GetFilePosition(file);
                        result = (s32)DSP_32BIT_TO_DSP(result);
                        DSP_WritePipe(out, &result, sizeof(result));
                    }
                    break;
                case DSP_PIPE_IO_COMMAND_READ:
                    // fread command
                    {
                        DSPWord length;
                        u32     rest;
                        u16     result;
                        DSP_ReadPipe(in, &length, sizeof(length));
                        length = DSP_WORD_TO_ARM(length);
                        // Get actual size and restrict in advance
                        rest = FS_GetFileLength(file) - FS_GetFilePosition(file);
                        length = (DSPWord)MATH_MIN(length, rest);
                        result = DSP_WORD_TO_DSP(length);
                        DSP_WritePipe(out, &result, sizeof(result));
                        while (length > 0)
                        {
                            u8      tmp[256];
                            u16     n = (u16)MATH_MIN(length, 256);
                            (void)FS_ReadFile(file, tmp, (s32)n);
                            DSP_WritePipe(out, tmp, n);
                            length -= n;
                        }
                    }
                    break;
                case DSP_PIPE_IO_COMMAND_WRITE:
                    // fwrite command
                    {
                        DSPWord length;
                        DSP_ReadPipe(in, &length, sizeof(length));
                        length = DSP_WORD_TO_ARM(length);
                        while (length > 0)
                        {
                            u8      tmp[256];
                            u16     n = (u16)MATH_MIN(length, 256);
                            DSP_ReadPipe(in, tmp, (u16)n);
                            (void)FS_WriteFile(file, tmp, (s32)n);
                            length -= n;
                        }
                    }
                    break;
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_NotifyFileIOUpdation

  Description:  Notifies update of file I/O

  Arguments:    port: Updated port number

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_NotifyFileIOUpdation(int port)
{
    DSPFileIOContext   *ctx = DSPiThread;
    OSIntrMode          bak = OS_DisableInterrupts();
    // Generate thread if necessary
    if (!ctx->initialized)
    {
        int                 i;
        for (i = 0; i < DSP_PIPE_PORT_MAX; ++i)
        {
            FS_InitFile(ctx->file[i]);
        }
        OS_InitMessageQueue(ctx->msgq, ctx->msga, 1);
        ctx->pollbits = 0;
        ctx->initialized = TRUE;
        OS_CreateThread(ctx->th, DSPi_FileIOProc, ctx,
                        &ctx->stack[sizeof(ctx->stack)], sizeof(ctx->stack), 13);
        OS_WakeupThreadDirect(ctx->th);
    }
    // Throw message for notification
    // NOBLOCK because it is acceptable for notifications not to line up in the queue if the same ones have accumulated
    ctx->pollbits |= (1 << port);
    (void)OS_SendMessage(ctx->msgq, (OSMessage)port, OS_MESSAGE_NOBLOCK);
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PipeCallbackToConsole

  Description:  DSP_PIPE_CONSOLE transmission handler

  Arguments:    userdata : Optional user-defined argument
                port: Port number
                peer: Transmission direction

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PipeCallbackToConsole(void *userdata, int port, int peer)
{
    (void)userdata;
    if (peer == DSP_PIPE_INPUT)
    {
        DSPPipe pipe[1];
        (void)DSP_LoadPipe(pipe, port, peer);
        {
            u16     max = DSP_GetPipeReadableSize(pipe);
            u16     pos = 0;
            while (pos < max)
            {
                enum { tmplen = 128 };
                char    buffer[tmplen + 2];
                u16     length = (u16)((max - pos < tmplen) ? (max - pos) : tmplen);
                DSP_ReadPipe(pipe, buffer, length);
                pos += length;
                // Debug output of the console output from DSP
                // Each printf output is delivered padded in u16 units, be careful that '\0' is inserted at irregular intervals.
                // 
                {
                    const char *str = (const char *)buffer;
                    int     current = 0;
                    int     pos;
                    for (pos = 0; pos < length; ++pos)
                    {
                        if (str[pos] == '\0')
                        {
                            OS_TPrintf("%s", &str[current]);
                            current = pos + 1;
                        }
                    }
                    if (current < length)
                    {
                        OS_TPrintf("%.*s", (length - current), &str[current]);
                    }
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_PipeCallbackForDMA

  Description:  DSP_PIPE_DMA transmission handler

  Arguments:    userdata : Optional user-defined argument
                port: Port number
                peer: Transmission direction

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_PipeCallbackForDMA(void *userdata, int port, int peer)
{
    (void)userdata;
    if (peer == DSP_PIPE_INPUT)
    {
        DSPPipe pipe[1];
        (void)DSP_LoadPipe(pipe, port, peer);
        {
            u16     max = DSP_GetPipeReadableSize(pipe);
            u16     pos = 0;
            while (pos < max)
            {
                enum { tmplen = 128 };
                char    buffer[tmplen + 2];
                u16     length = (u16)((max - pos < tmplen) ? (max - pos) : tmplen);
                DSP_ReadPipe(pipe, buffer, length);
                pos += length;
                // Debug output of the console output from DSP
                // Each printf output is delivered padded in u16 units, be careful that '\0' is inserted at irregular intervals.
                // 
                {
                    const char *str = (const char *)buffer;
                    int     current = 0;
                    int     pos;
                    for (pos = 0; pos < length; ++pos)
                    {
                        if (str[pos] == '\0')
                        {
                            OS_TPrintf("%s", &str[current]);
                            current = pos + 1;
                        }
                    }
                    if (current < length)
                    {
                        OS_TPrintf("%.*s", (length - current), &str[current]);
                    }
                }
            }
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         DSPi_WaitForPipe

  Description:  Idling update specified DSP pipe information

  Arguments:    pipe: Pipe information

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DSPi_WaitForPipe(DSPPipe *pipe)
{
    OS_SleepThread(DSPiBlockingQueue);
    (void)DSP_SyncPipe(pipe);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_InitPipe

  Description:  Initializes DSP pipe communication.
                Occupy DSP command replay register 2

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_InitPipe(void)
{
    int     i;
    DSPiPipeMonitorAddress = 0;
    OS_InitThreadQueue(DSPiBlockingQueue);
    for (i = 0; i < DSP_PIPE_PORT_MAX; ++i)
    {
        DSPiCallback[i] = NULL;
        DSPiCallbackArgument[i] = NULL;
    }
    // DSP_PIPE_CONSOLE is the debug console for DSP
    DSPiCallback[DSP_PIPE_CONSOLE] = DSPi_PipeCallbackToConsole;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SetPipeCallback

  Description:  Sets the DSP pipe communcation callback

  Arguments:    port: Pipe port number
                callback: Callback for readable/writable events
                userdata : Optional user-defined argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetPipeCallback(int port, void (*callback)(void *, int, int), void *userdata)
{
    DSPiCallback[port] = callback;
    DSPiCallbackArgument[port] = userdata;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadPipe

  Description:  Loads DSP pipe information

  Arguments:    pipe: Storage destination of pipe information (Not NULL on the DSP side)
                port: Pipe port number
                peer: DSP_PIPE_INPUT or DSP_PIPE_OUTPUT

  Returns:      Pointer of the loaded pipe information
 *---------------------------------------------------------------------------*/
DSPPipe* DSP_LoadPipe(DSPPipe *pipe, int port, int peer)
{
    // Idle until notified of the monitor address
    OSIntrMode  bak = OS_DisableInterrupts();
    while (!DSPiPipeMonitorAddress)
    {
        OS_SleepThread(DSPiBlockingQueue);
    }
    (void)OS_RestoreInterrupts(bak);
    if (((port >= 0) && (port < DSP_PIPE_PORT_MAX)) &&
        ((peer >= 0) && (peer < DSP_PIPE_PEER_MAX)))
    {
        DSPPipeMonitor *monitor = (DSPPipeMonitor *)DSPiPipeMonitorAddress;
        DSPPipe        *target = &monitor->pipe[port][peer];
        if (!pipe)
        {
            pipe = &DSPiDefaultPipe[port][peer];
        }
        DSP_LoadData((DSPAddrInARM)target, pipe, sizeof(*pipe));
    }
    return pipe;
}

/*---------------------------------------------------------------------------*
  Name:         DSP_SyncPipe

  Description:  Update DSP pipe information to the latest content.

  Arguments:    pipe: Pipe information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SyncPipe(DSPPipe *pipe)
{
    (void)DSP_LoadPipe(pipe,
                 ((pipe->flags & DSP_PIPE_FLAG_PORTMASK) >> 1),
                 (pipe->flags & 1));
}

/*---------------------------------------------------------------------------*
  Name:         DSP_FlushPipe

  Description:  Flushes the DSP pipe stream

  Arguments:    pipe: Pipe information
                port: Pipe port number
                peer: DSP_PIPE_INPUT or DSP_PIPE_OUTPUT

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_FlushPipe(DSPPipe *pipe)
{
    // Write back to the DSP only the pointer updated from the ARM side
    int     port = ((pipe->flags & DSP_PIPE_FLAG_PORTMASK) >> 1);
    int     peer = (pipe->flags & 1);
    DSPPipeMonitor *monitor = (DSPPipeMonitor *)DSPiPipeMonitorAddress;
    DSPPipe        *target = &monitor->pipe[port][peer];
    if (peer == DSP_PIPE_INPUT)
    {
        DSP_StoreData((DSPAddrInARM)&target->rpos, &pipe->rpos, sizeof(target->rpos));
    }
    else
    {
        DSP_StoreData((DSPAddrInARM)&target->wpos, &pipe->wpos, sizeof(target->wpos));
    }
    // Update notification to DSP side
    OS_SpinWaitSysCycles(4);
    while ((reg_DSP_PSTS & REG_DSP_PSTS_WFEI_MASK) == 0 || (reg_DSP_PSTS & REG_DSP_PSTS_WTIP_MASK) != 0) {}
    DSP_SendData(DSP_PIPE_COMMAND_REGISTER, (u16)(pipe->flags & DSP_PIPE_FLAG_PORTMASK));
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetPipeReadableSize

  Description:  Get maximum size that currently can be read from the specified DSP pipe

  Arguments:    pipe: Pipe information

  Returns:      Maximum size that currently can be read
 *---------------------------------------------------------------------------*/
u16 DSP_GetPipeReadableSize(const DSPPipe *pipe)
{
    return DSP_BYTE_TO_UNIT(((pipe->wpos - pipe->rpos) +
           (((pipe->rpos ^ pipe->wpos) < 0x8000) ? 0 : pipe->length)) & ~0x8000);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_GetPipeWritableSize

  Description:  Get maximum size that currently can be written to the specified DSP pipe

  Arguments:    pipe: Pipe information

  Returns:      Maximum size that currently can be written
 *---------------------------------------------------------------------------*/
u16 DSP_GetPipeWritableSize(const DSPPipe *pipe)
{
    return DSP_BYTE_TO_UNIT(((pipe->rpos - pipe->wpos) +
           (((pipe->wpos ^ pipe->rpos) < 0x8000) ? 0 : pipe->length)) & ~0x8000);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_ReadPipe

  Description:  Read data from the DSP pipe communication port

  Arguments:    pipe: Pipe information
                buffer: Transfer destination buffer
                length: Transfer size (However, the units are the word size of that environment)
                         Be careful that the ARM side is 1-byte units, and the DSP side is 2-byte units.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ReadPipe(DSPPipe *pipe, void *buffer, u16 length)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    u8         *dst = (u8 *)buffer;
    BOOL        modified = FALSE;
    length = DSP_UNIT_TO_BYTE(length);
    DSP_SyncPipe(pipe);
    while (length > 0)
    {
        u16     rpos = pipe->rpos;
        u16     wpos = pipe->wpos;
        u16     phase = (u16)(rpos ^ wpos);
        // Wait for completion if read-empty
        if (phase == 0x0000)
        {
            if (modified)
            {
                DSP_FlushPipe(pipe);
                modified = FALSE;
            }
            DSPi_WaitForPipe(pipe);
        }
        else
        {
            // If not, read from a safe range
            u16     pos = (u16)(rpos & ~0x8000);
            u16     end = (u16)((phase < 0x8000) ? (wpos & ~0x8000) : pipe->length);
            u16     len = (u16)((length < end - pos) ? length : (end - pos));
            len = (u16)(len & ~(DSP_WORD_UNIT - 1));
            DSP_LoadData(DSP_ADDR_TO_ARM(pipe->address) + pos, dst, len);
            length -= len;
            dst += DSP_BYTE_TO_UNIT(len);
            pipe->rpos = (u16)((pos + len < pipe->length) ? (rpos + len) : (~rpos & 0x8000));
            modified = TRUE;
        }
    }
    // If there is an addition, notify here
    if (modified)
    {
        DSP_FlushPipe(pipe);
        modified = FALSE;
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_WritePipe

  Description:  Writes data to the DSP pipe communication port 

  Arguments:    pipe: Pipe information
                buffer: Transfer source buffer
                length: Transfer size (However, the units are the word size of that environment)
                         Note that the ARM side is 1-byte units, and the DSP side is 2-byte units.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_WritePipe(DSPPipe *pipe, const void *buffer, u16 length)
{
    OSIntrMode  bak = OS_DisableInterrupts();
    const u8   *src = (const u8 *)buffer;
    BOOL        modified = FALSE;
    length = DSP_UNIT_TO_BYTE(length);
    DSP_SyncPipe(pipe);
    while (length > 0)
    {
        u16     rpos = pipe->rpos;
        u16     wpos = pipe->wpos;
        u16     phase = (u16)(rpos ^ wpos);
        // If write-full, wait for completion
        if (phase == 0x8000)
        {
            if (modified)
            {
                DSP_FlushPipe(pipe);
                modified = FALSE;
            }
            DSPi_WaitForPipe(pipe);
        }
        else
        {
            // If not, write to a safe range
            u16     pos = (u16)(wpos & ~0x8000);
            u16     end = (u16)((phase < 0x8000) ? pipe->length : (rpos & ~0x8000));
            u16     len = (u16)((length < end - pos) ? length : (end - pos));
            len = (u16)(len & ~(DSP_WORD_UNIT - 1));
            DSP_StoreData(DSP_ADDR_TO_ARM(pipe->address) + pos, src, len);
            length -= len;
            src += DSP_BYTE_TO_UNIT(len);
            pipe->wpos = (u16)((pos + len < pipe->length) ? (wpos + len) : (~wpos & 0x8000));
            modified = TRUE;
        }
    }
    // If there is an addition, notify here
    if (modified)
    {
        DSP_FlushPipe(pipe);
        modified = FALSE;
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         DSP_HookPipeNotification

  Description:  Pipe notification hook that should be called in a DSP interrupt

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_HookPipeNotification(void)
{
    // CR2 dedicated for pipe communication
    // However, for some reason a CR interrupt was not generated, so currently, substitute semaphore with only one bit
    // 
    while (((DSP_GetSemaphore() & 0x8000) != 0) || DSP_RecvDataIsReady(DSP_PIPE_COMMAND_REGISTER))
    {
        DSP_ClearSemaphore(0x8000);
        while (DSP_RecvDataIsReady(DSP_PIPE_COMMAND_REGISTER))
        {
            // Initially, the shared structure address is notified from the DSP side
            if (DSPiPipeMonitorAddress == 0)
            {
                DSPiPipeMonitorAddress = DSP_ADDR_TO_ARM(DSP_RecvData(DSP_PIPE_COMMAND_REGISTER));
            }
            // Thereafter, only updated pipes are notified
            else
            {
                u16     recvdata = DSP_RecvData(DSP_PIPE_COMMAND_REGISTER);
                int     port = (recvdata >> 1);
                int     peer = (recvdata & 1);
                if ((port >= 0) && (port < DSP_PIPE_PORT_MAX))
                {
                    // If a monitoring source exists on the ARM side, verify the pipe information
                    if (DSPiCallback[port])
                    {
                        (*DSPiCallback[port])(DSPiCallbackArgument[port], port, peer);
                    }
                    else
                    {
                        DSPPipe pipe[1];
                        (void)DSP_LoadPipe(pipe, port, peer);
                        // If updating the file opened on the DSP side, notify the thread
                        if ((peer == DSP_PIPE_INPUT) && ((pipe->flags & DSP_PIPE_FLAG_BOUND) != 0))
                        {
                            DSPi_NotifyFileIOUpdation(port);
                        }
                    }
                }
            }
            // Notify update to thread during blocking
            OS_WakeupThread(DSPiBlockingQueue);
        }
    }
}
