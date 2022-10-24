/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOWave.c

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include "DEMOWave.h"

#define MAKE_FOURCC(cc1, cc2, cc3, cc4) (u32)((cc1) | (cc2 << 8) | (cc3 << 16) | (cc4 << 24))

#define FOURCC_RIFF MAKE_FOURCC('R', 'I', 'F', 'F')
#define FOURCC_WAVE MAKE_FOURCC('W', 'A', 'V', 'E')
#define FOURCC_fmt  MAKE_FOURCC('f', 'm', 't', ' ')
#define FOURCC_data MAKE_FOURCC('d', 'a', 't', 'a')

#define L_CHANNEL 4
#define R_CHANNEL 5
#define ALARM_NUM 0
#define STREAM_THREAD_PRIO 12
#define THREAD_STACK_SIZE 1024
#define STRM_BUF_PAGESIZE 1024*32
#define STRM_BUF_SIZE STRM_BUF_PAGESIZE*2

// WAV format header
typedef struct WaveFormat
{
    u16     format;
    u16     channels;
    s32     sampleRate;
    u32     dataRate;
    u16     blockAlign;
    u16     bitPerSample;
}
WaveFormat;

// Stream object
typedef struct StreamInfo
{
    FSFile  file;
    WaveFormat format;
    u32     beginPos;
    s32     dataSize;
    BOOL    isPlay;
}
StreamInfo;

static BOOL ReadWaveFormat(StreamInfo * strm);
static void CloseWave(void);

static StreamInfo StrmInfo;

/*---------------------------------------------------------------------------*
  Name:         DEMOReadWave

  Description:  Initializes wave data.

  Arguments:    dst: Buffer in which to get waveform data
                filename:   Name of the streaming playback file

  Returns:      Returns the waveform data size
 *---------------------------------------------------------------------------*/
int DEMOReadWave(char* dst, const char *filename)
{
    int readSize;
    
    StreamInfo * strm = &StrmInfo;

    // File scanning
    if (FS_IsFile(&strm->file))
    {
        (void)FS_CloseFile(&strm->file);
    }
    if (!FS_OpenFileEx(&strm->file, filename, FS_FILEMODE_R))
    {
        OS_Panic("Error: failed to open file %s\n", filename);
    }
    if (!ReadWaveFormat(strm))
    {
        OS_Panic("Error: failed to read wavefile\n");
    }

    // seek to the sound data
    (void)FS_SeekFile(&strm->file, (s32)strm->beginPos, FS_SEEK_SET);

    readSize = FS_ReadFile(&strm->file,
                           dst, strm->dataSize);
    if (readSize == -1)
    {
        OS_Panic("read file end\n");
    }
    
    CloseWave();

    return readSize;
}

/*---------------------------------------------------------------------------*
  Name:         CloseWave

  Description:  Closes wave data

  Arguments:    strm:   Stream object

  Returns:      None
 *---------------------------------------------------------------------------*/
static void CloseWave(void)
{
    StreamInfo * strm = &StrmInfo;
    if (FS_IsFile(&strm->file))
    {
        (void)FS_CloseFile(&strm->file);
    }
}

/*---------------------------------------------------------------------------*
  Name:         ReadWaveFormat

  Description:  Gets the data size and first position of a data string, and the WAVE format data header

  Arguments:    strm:   Stream object

  Returns:      If readout is successful: TRUE; if failed: FALSE
 *---------------------------------------------------------------------------*/
static BOOL ReadWaveFormat(StreamInfo * strm)
{
    u32     tag;
    s32     size;
    BOOL    fFmt = FALSE, fData = FALSE;

    (void)FS_SeekFileToBegin(&strm->file);

    (void)FS_ReadFile(&strm->file, &tag, 4);
    if (tag != FOURCC_RIFF)
        return FALSE;

    (void)FS_ReadFile(&strm->file, &size, 4);

    (void)FS_ReadFile(&strm->file, &tag, 4);
    if (tag != FOURCC_WAVE)
        return FALSE;

    while (size > 0)
    {
        s32     chunkSize;
        if (FS_ReadFile(&strm->file, &tag, 4) == -1)
        {
            return FALSE;
        }
        if (FS_ReadFile(&strm->file, &chunkSize, 4) == -1)
        {
            return FALSE;
        }

        switch (tag)
        {
        case FOURCC_fmt:
            if (chunkSize != 0x10)
            {
                return FALSE;
            }
            if (FS_ReadFile(&strm->file, (u8 *)&strm->format, chunkSize) == -1)
            {
                return FALSE;
            }
            fFmt = TRUE;
            break;
        case FOURCC_data:
            strm->beginPos = FS_GetFilePosition(&strm->file);
            strm->dataSize = chunkSize;
            (void)FS_SeekFile(&strm->file, chunkSize, FS_SEEK_CUR);
            fData = TRUE;
            break;
        default:
            (void)FS_SeekFile(&strm->file, chunkSize, FS_SEEK_CUR);
            break;
        }
        if (fFmt && fData)
        {
            return TRUE;               // Forces end if fmt and data have finished being read
        }

        size -= chunkSize;
    }

    if (size != 0)
        return FALSE;
    return TRUE;
}
