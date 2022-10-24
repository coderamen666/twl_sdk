/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - seq
  File:     seq.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include "seq.h"
#include "smfdefine.h"

/* Status constants. */
#define SEQ_STATE_PLAY 0x01            /* Playing */
#define SEQ_STATE_END  0x02            /* End of playback (end of track) */
#define SEQ_STATE_LOOP 0x04            /* Loop flag */
#define SEQ_STATE_MUTE 0x10            /* Mute */

/* Default tempo = 120 BPM (microseconds) */
#define SEQ_DEFAULT_TEMPO 500000

/* System exclusive buffer size (ignores the message when over size) */
#define SEQ_SYSX_BUFFER_SIZE 64


static u8 SeqReadVariableData(const u8 *data, u32 *result);
static u8 SeqReadSMFHeader(SeqHandle * block, const u8 *data);
static u8 SeqExecMetaEvent(SeqHandle * block);

static u16 ReverseEndian16(u16 data);
static u32 ReverseEndian32(u32 data);

typedef struct tSeqSystemBlock
{
    void    (*callback) (const u8 *);
    u32     clock_interval;            /* Time interval that calls SeqMain. Units in microseconds. */
}
SeqSystemBlock;

SeqSystemBlock seq_sys;

/* Data byte number of each MIDI event */
static const u8 seq_midi_byte_size[8] = {
/*  8x,9x,Ax,Bx,Cx,Dx,Ex,Fx*/
    2, 2, 2, 2, 1, 1, 2, 0
};

/* System exclusive buffer */
static u8 seqSysxBuffer[SEQ_SYSX_BUFFER_SIZE];

/* Saves the error code at time of an error */
static u8 seqErrorCode;

/*---------------------------------------------------------------------------*
  Name:         SeqInit

  Description:  Initializes the sequence system.

  Arguments:    clock_interval: Time interval that calls SeqMain. Units in microseconds.
                callback: Configures the function called when a MIDI event occurs.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SeqInit(u32 clock_interval, void (*callback) (const u8 *))
{
    seq_sys.callback = callback;       // Saves the callback function
    seq_sys.clock_interval = clock_interval;    // Saves the call interval
}

/*---------------------------------------------------------------------------*
  Name:         SeqMain

  Description:  The main routine of the sequencer.

  Arguments:    handle: Song handle that executes a process

  Returns:      TRUE if an error occurs.
 *---------------------------------------------------------------------------*/
BOOL SeqMain(SeqHandle * handle)
{
    u8      read_size;
    u8      current_event;
    u8      f_exist_status;            /* Whether there are MIDI event instructions (whether currently in running status) */

    /* Returns as-is when not in a playback state */
    if (!(handle->state & SEQ_STATE_PLAY))
    {
        return FALSE;
    }

    /* Progresses 1 callback's worth of time */
    handle->time_control += handle->time_per_callback;

    while (handle->time_control > handle->tempo)
    {
        /* Process for each tick */

        /* Subtracts delta time */
        if (handle->delta_time > 0)
        {
            handle->delta_time--;
        }

        while (handle->delta_time == 0)
        {

            /* Delta time reached 0, so an event is executed */
            handle->current_ptr += handle->next_delta_bytesize;

            /* current_ptr is the start of a MIDI message */
            if (*handle->current_ptr >= 0x80)
            {
                /* Running status is configured because of starting from an instruction */
                current_event = handle->running_status = *handle->current_ptr;
                f_exist_status = 1;
            }
            else
            {
                /* Running status is used because of starting from a value */
                if (handle->running_status < 0x80)
                {
                    seqErrorCode = SEQ_ERROR_WRONG_TRACK;
                    return TRUE;       /* Error */
                }
                current_event = handle->running_status;
                f_exist_status = 0;
            }

            if (*handle->current_ptr == SMF_META)
            {

                /* The meta event is an operation within the sequencer */
                read_size = SeqExecMetaEvent(handle);

                if (handle->state & SEQ_STATE_END)
                {
                    /* END OF TRACK: Finished playing a song to the end. */
                    (void)SeqStop(handle);
                    handle->state &= ~SEQ_STATE_END;
                    return FALSE;
                }
                else if (handle->state & SEQ_STATE_LOOP)
                {
                    /* LOOP END: Return to the starting position of a loop. */
                    handle->current_ptr = handle->loop_begin;
                    handle->state &= ~SEQ_STATE_LOOP;
                }
                else
                {
                    /* Proceed to the next event */
                    handle->current_ptr += read_size;
                }

            }
            else                       /* Other than meta-event */
            {
                if (*handle->current_ptr == SMF_SYSX)
                {
                    /* System exclusive */

                    if (SEQ_SYSX_BUFFER_SIZE > handle->current_ptr[1] + 1)
                    {
                        u8      i;

                        /* Copy to buffer */
                        seqSysxBuffer[0] = handle->current_ptr[0];
                        for (i = 1; i <= handle->current_ptr[1]; i++)
                        {
                            seqSysxBuffer[i] = handle->current_ptr[i + 1];
                        }

                        /* Send the MIDI event to the callback */
                        seq_sys.callback(seqSysxBuffer);
                    }

                    /* Proceed to the next event */
                    handle->current_ptr += handle->current_ptr[1] + 2;
                }
                else
                {
                    /* Short MIDI event */

                    /* Send the MIDI event to the callback */
                    seq_sys.callback(handle->current_ptr);

                    /* Proceed to the next event */
                    handle->current_ptr +=
                        seq_midi_byte_size[(current_event >> 4) - 8] + f_exist_status;
                }
            }

            /* Read the delta time of the next event */
            handle->next_delta_bytesize =
                SeqReadVariableData(handle->current_ptr, &handle->delta_time);
            if (handle->next_delta_bytesize == 0)
            {
                return TRUE;           /* Error */
            }
        }

        /* Advance tick */
        handle->total_tick++;

        /* Advance time */
        handle->time_control -= handle->tempo;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         SeqExecMetaEvent

  Description:  Meta-event process routine.

  Arguments:    handle: Song handle that executes a process

  Returns:      Meta-event byte size.
 *---------------------------------------------------------------------------*/
static u8 SeqExecMetaEvent(SeqHandle * handle)
{
    switch (handle->current_ptr[1])
    {
    case SMF_META_MARKER:             /* Marker */
        if (handle->current_ptr[2] != 1)
        {
            break;
        }

        /* NITRO Composer compatible loop functionality */
        if (handle->current_ptr[3] == '[')
        {
            /* Loop start position */
            handle->loop_begin = handle->current_ptr + handle->current_ptr[2] + 3;
        }
        else if (handle->current_ptr[3] == ']')
        {
            /* Loop end position */
            if (handle->loop_begin != NULL)
            {
                handle->state |= SEQ_STATE_LOOP;
            }
        }
        break;
    case SMF_META_ENDOFTRACK:         /* End of track */
        handle->state |= SEQ_STATE_END;
        break;
    case SMF_META_TEMPO:              /* Set tempo */
        handle->tempo = handle->current_ptr[3];
        handle->tempo <<= 8;
        handle->tempo += handle->current_ptr[4];
        handle->tempo <<= 8;
        handle->tempo += handle->current_ptr[5];
        break;
    default:
        break;
    }

    return (u8)(handle->current_ptr[2] + 3);
}

/*---------------------------------------------------------------------------*
  Name:         SeqPlay

  Description:  Starts SMF data playback.

  Arguments:    handle: Song handle that executes a process
                smfdata: SMF data

  Returns:      Meta-event byte size.
 *---------------------------------------------------------------------------*/
BOOL SeqPlay(SeqHandle * handle, const u8 *smfdata)
{
    u8      read_size;
    static const u8 mtrk[] = "MTrk";
    u8      i;

    /* Reads the SMF header chunk */
    read_size = SeqReadSMFHeader(handle, smfdata);
    if (read_size == 0)
        return TRUE;                   /* Error */

    handle->current_ptr = smfdata + read_size;

    /* Reads track chunk header */
    /* 'MTrk' check */
    for (i = 0; i < 4; i++)
    {
        if (handle->current_ptr[i] != mtrk[i])
        {
            seqErrorCode = SEQ_ERROR_WRONG_TRACK;       /* Error */
            return TRUE;
        }
    }
    handle->current_ptr += sizeof(u8) * 4;
    /* Chunk size */
    handle->chunk_size = ReverseEndian32(*(u32 *)handle->current_ptr);
    handle->current_ptr += sizeof(u32);

    /* Track head */
    handle->track_begin = handle->current_ptr;

    /* Initial settings */
    handle->tempo = SEQ_DEFAULT_TEMPO;
    handle->time_control = 0;
    handle->time_per_callback = handle->division * seq_sys.clock_interval;
    handle->total_tick = 0;
    handle->running_status = 0x00;
    handle->loop_begin = NULL;

    /* Status variable settings */
    handle->state = SEQ_STATE_PLAY;

    /* Reads the initial delta time */
    handle->next_delta_bytesize = SeqReadVariableData(handle->current_ptr, &handle->delta_time);
    if (handle->next_delta_bytesize == 0)
    {
        seqErrorCode = SEQ_ERROR_WRONG_TRACK;
        return TRUE;                   /* Error */
    }

    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         SeqStop

  Description:  Stops SMF data playback.

  Arguments:    handle: Song handle that executes a process

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SeqStop(SeqHandle * handle)
{
    handle->current_ptr = handle->track_begin;

    /* Status variable settings */
    handle->state &= ~SEQ_STATE_PLAY;
}

/*---------------------------------------------------------------------------*
  Name:         SeqPause

  Description:  Pauses SMF data playback.

  Arguments:    handle: Song handle that executes a process

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SeqPause(SeqHandle * handle)
{
    /* Status variable settings */
    handle->state &= ~SEQ_STATE_PLAY;
}

/*---------------------------------------------------------------------------*
  Name:         SeqReadSMFHeader

  Description:  Reads the SMF header chunk.

  Arguments:    handle: Song handle that executes a process
                data: SMF data string

  Returns:      Number of bytes read. Normally 14.
 *---------------------------------------------------------------------------*/
static u8 SeqReadSMFHeader(SeqHandle * handle, const u8 *data)
{
    static const u8 mthd[] = "MThd";
    u8      i;
    const u8 *ptr = data;

    /* All data that is fixed in a format other than 0 is considered an error. */

    /* 'MThd' check */
    for (i = 0; i < 4; i++)
    {
        if (ptr[i] != mthd[i])
        {
            seqErrorCode = SEQ_ERROR_WRONG_HEADER;      /* Error */
            return 0;
        }
    }
    ptr += sizeof(u8) * 4;

    /* The length of the header chunk data is 6 bytes */
    if (ReverseEndian32(*(u32 *)ptr) != 6)
    {
        seqErrorCode = SEQ_ERROR_WRONG_HEADER;  /* Error */
        return 0;
    }
    ptr += sizeof(u32);

    /* Only handles format 0 */
    if (*(u16 *)ptr != SMF_FORMAT0)
    {
        seqErrorCode = SEQ_ERROR_NOT_FORMAT0;   /* Error */
        return 0;
    }
    ptr += sizeof(u16);

    /* Error if there is other than 1 track */
    if (ReverseEndian16(*(u16 *)ptr) != 1)
    {
        seqErrorCode = SEQ_ERROR_WRONG_HEADER;  /* Error */
        return 0;
    }
    ptr += sizeof(u16);

    /* Saves the resolution */
    handle->division = ReverseEndian16(*(u16 *)ptr);
    if (handle->division >= 0x8000)
    {
        seqErrorCode = SEQ_ERROR_DIVISION_TIMECODE;     /* Error */
        return 0;
    }
    ptr += sizeof(u16);

    return (u8)(ptr - data);
}

/*---------------------------------------------------------------------------*
  Name:         SeqReadVariableData

  Description:  Routine for reading variable length data.

  Arguments:    data: Variable length SMFMIDI data string
                result: Area for storing the read numerical value

  Returns:      Number of bytes read. The maximum is 4 bytes in SMF, so anything above that is considered an error and returns 0.
                
 *---------------------------------------------------------------------------*/
static u8 SeqReadVariableData(const u8 *data, u32 *result)
{
    u8      count = 0;
    *result = 0;

    while (*data & 0x80)
    {
        /* Continuous bytes */
        *result = (*result << 7) + (*data & 0x7f);
        count++;
        if (count == 4)
        {
            /* The 4th byte is not the final byte, so this is an error */
            seqErrorCode = SEQ_ERROR_WRONG_DELTA_TIME;
            return 0;
        }
        data++;
    }

    /* Final byte */
    *result = (*result << 7) + *data;
    count++;

    return count;
}

/*---------------------------------------------------------------------------*
  Name:         ReverseEndian16

  Description:  Endian conversion (2 bytes).

  Arguments:    data: The data to convert

  Returns:      Endian converted data.
 *---------------------------------------------------------------------------*/
static u16 ReverseEndian16(u16 data)
{
    return (u16)(((data & 0x00ff) << 8) | ((data & 0xff00) >> 8));
}

/*---------------------------------------------------------------------------*
  Name:         ReverseEndian32

  Description:  Endian conversion (4 bytes).

  Arguments:    data: The data to convert

  Returns:      Endian converted data.
 *---------------------------------------------------------------------------*/
static u32 ReverseEndian32(u32 data)
{
    return (((data >> 24) & 0x000000ff) |
            ((data >> 8) & 0x0000ff00) | ((data << 8) & 0x00ff0000) | ((data << 24) & 0xff000000));
}

/*---------------------------------------------------------------------------*/
u8 SeqGetErrorCode(void)
{
    return seqErrorCode;
}
