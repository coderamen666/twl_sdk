/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - dsp - common
  File:     pipe.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-10-21#$
  $Rev: 9018 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/
#ifndef TWL_DSP_PIPE_H_
#define TWL_DSP_PIPE_H_


#include <twl/dsp/common/byteaccess.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

// Constants for defining IN and OUT. (In principle, the flow is configured as follows. 0: DSP -> ARM, 1: ARM -> DSP)
#ifdef SDK_TWL
#define DSP_PIPE_INPUT      0
#define DSP_PIPE_OUTPUT     1
#else
#define DSP_PIPE_INPUT      1
#define DSP_PIPE_OUTPUT     0
#endif
#define DSP_PIPE_PEER_MAX   2

// System resources
#define DSP_PIPE_PORT_MAX               8   // Maximum number of usable ports for pipes
#define DSP_PIPE_DEFAULT_BUFFER_LENGTH  256 // Default ring buffer size

// Defined ports
#define DSP_PIPE_CONSOLE    0   // DSP -> ARM: Debugging console
#define DSP_PIPE_DMA        1   // DSP <-> ARM: Pseudo-DMA
#define DSP_PIPE_AUDIO      2   // DSP <-> ARM: General-purpose audio communications
#define DSP_PIPE_BINARY     3   // DSP <-> ARM: General-purpose binary
#define DSP_PIPE_EPHEMERAL  4   // Free region that can be allocated with DSP_CreatePipe()

#define DSP_PIPE_FLAG_INPUT     0x0000  // Input side
#define DSP_PIPE_FLAG_OUTPUT    0x0001  // Output side
#define DSP_PIPE_FLAG_PORTMASK  0x00FF  // Negative field for port numbers
#define DSP_PIPE_FLAG_BOUND     0x0100  // Opened
#define DSP_PIPE_FLAG_EOF       0x0200  // EOF

#define DSP_PIPE_FLAG_EXIT_OS   0x8000  // Exit processing for the DSP's AHB master

#define DSP_PIPE_COMMAND_REGISTER   2

// Command structures for DSP file I/O.
#define DSP_PIPE_IO_COMMAND_OPEN    0
#define DSP_PIPE_IO_COMMAND_CLOSE   1
#define DSP_PIPE_IO_COMMAND_SEEK    2
#define DSP_PIPE_IO_COMMAND_READ    3
#define DSP_PIPE_IO_COMMAND_WRITE   4
#define DSP_PIPE_IO_COMMAND_MEMMAP  5

#define DSP_PIPE_IO_MODE_R          0x0001
#define DSP_PIPE_IO_MODE_W          0x0002
#define DSP_PIPE_IO_MODE_RW         0x0004
#define DSP_PIPE_IO_MODE_TRUNC      0x0008
#define DSP_PIPE_IO_MODE_CREATE     0x0010

#define DSP_PIPE_IO_SEEK_SET        0
#define DSP_PIPE_IO_SEEK_CUR        1
#define DSP_PIPE_IO_SEEK_END        2


/*---------------------------------------------------------------------------*/
/* Declarations */

// Pipe structures.
// There is no procedure for direct ARM access from the DSP, so the DSP buffer is normally controlled in the APBP-FIFO from an ARM processor.
// It will be treated as a ring buffer in the absence of a particular specification.
// 
typedef struct DSPPipe
{
    DSPAddr address;    // Starting address of the buffer
    DSPByte length;     // Buffer size
    DSPByte rpos;       // First unread region
    DSPByte wpos;       // Last appended region
    u16     flags;      // Attribute flags
}
DSPPipe;

// Pipe information maintained by the DSP and accessed by an ARM processor.
typedef struct DSPPipeMonitor
{
    DSPPipe pipe[DSP_PIPE_PORT_MAX][DSP_PIPE_PEER_MAX];
}
DSPPipeMonitor;

// DSP pipe communication handler.
typedef void (*DSPPipeCallback)(void *userdata, int port, int peer);


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         DSP_InitPipe

  Description:  Initializes DSP pipe communication.
                This takes possession of DSP command register 2.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_InitPipe(void);

/*---------------------------------------------------------------------------*
  Name:         DSP_SetPipeCallback

  Description:  Sets a callback for DSP pipe communications.

  Arguments:    port: The pipe's port number.
                callback: The callback to invoke on a Readable/Writable event.
                userdata: Arbitrary user-defined argument

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SetPipeCallback(int port, void (*callback)(void *, int, int), void *userdata);

/*---------------------------------------------------------------------------*
  Name:         DSP_LoadPipe

  Description:  Loads DSP pipe information.

  Arguments:    pipe: Location to store pipe information (this may be NULL on the DSP)
                port: The pipe's port number
                peer: DSP_PIPE_INPUT or DSP_PIPE_OUTPUT

  Returns:      A pointer to the loaded pipe information.
 *---------------------------------------------------------------------------*/
DSPPipe* DSP_LoadPipe(DSPPipe *pipe, int port, int peer);

/*---------------------------------------------------------------------------*
  Name:         DSP_SyncPipe

  Description:  Updates DSP pipe information to the most recent content.

  Arguments:    pipe: Pipe information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_SyncPipe(DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_FlushPipe

  Description:  Flushes a DSP pipe stream.

  Arguments:    pipe: Pipe information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_FlushPipe(DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_GetPipeReadableSize

  Description:  Gets the maximum size that can currently be read from the specified DSP pipe.

  Arguments:    pipe: Pipe information

  Returns:      The maximum size that can currently be read.
 *---------------------------------------------------------------------------*/
u16 DSP_GetPipeReadableSize(const DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_GetPipeWritableSize

  Description:  Gets the maximum size that can currently be written to the specified DSP pipe.

  Arguments:    pipe: Pipe information

  Returns:      The maximum size that can currently be written.
 *---------------------------------------------------------------------------*/
u16 DSP_GetPipeWritableSize(const DSPPipe *pipe);

/*---------------------------------------------------------------------------*
  Name:         DSP_ReadPipe

  Description:  Reads data from a DSP pipe's communications port.

  Arguments:    pipe: Pipe information
                buffer: Buffer to transfer data to
                length: Transfer size. (however, this is in units of the environment's word size)
                         Note that 1-byte units are used on an ARM processor and 2-byte units are used on the DSP.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_ReadPipe(DSPPipe *pipe, void *buffer, u16 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_WritePipe

  Description:  Writes data to a DSP pipe's communications port.

  Arguments:    pipe: Pipe information
                buffer: Buffer to transfer data from
                length: Transfer size. (however, this is in units of the environment's word size)
                         Note that 1-byte units are used on an ARM processor and 2-byte units are used on the DSP.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_WritePipe(DSPPipe *pipe, const void *buffer, u16 length);

/*---------------------------------------------------------------------------*
  Name:         DSP_HookPipeNotification

  Description:  Pipe notification hook that should be invoked in a DSP interrupt.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_HookPipeNotification(void);

#ifdef SDK_TWL

#else

/*---------------------------------------------------------------------------*
  Name:         DSP_Printf

  Description:  Outputs a string to the DSP pipe's console port.

  Arguments:    format: Formatted string

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DSP_Printf(const char *format, ...);

int DSP_OpenFile(const char *path, int mode);
int DSP_OpenMemoryFile(DSPAddrInARM address, DSPWord32 length);
void DSP_CloseFile(int port);
s32 DSP_GetFilePosition(int port);
s32 DSP_GetFileLength(int port);
s32 DSP_SeekFile(int port, s32 offset, int whence);
s32 DSP_ReadFile(int port, void *buffer, DSPWord length);
s32 DSP_WriteFile(int port, const void *buffer, DSPWord length);

// Sample replacement functions for the C Standard Library
#if 0
typedef void FILE;
#define fopen(path, mode)           (FILE*)DSP_OpenFile(path, mode)
#define fclose(f)                   DSP_CloseFile((int)f)
#define fseek(f, ofs, whence)       DSP_SeekFile((int)f, ofs, whence)
#define fread(buf, len, unit, f)    DSP_ReadFile((int)f, buf, (len) * (unit))
#define fwrite(buf, len, unit, f)   DSP_WriteFile((int)f, buf, (len) * (unit))
#define rewind(f)                   (void)DSP_SeekFile((int)f, 0, DSP_PIPE_IO_SEEK_SET)
#define ftell(f)                    (void)DSP_SeekFile((int)f, 0, DSP_PIPE_IO_SEEK_CUR)
#define fgetpos(f, ppos)            (((*(ppos) = ftell((int)f)) != -1) ? 0 : -1)
#define fsetpos(f, ppos)            fseek((int)f, *(ppos), DSP_PIPE_IO_SEEK_SET)
#endif

#endif


/*===========================================================================*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_DSP_PIPE_H_ */
