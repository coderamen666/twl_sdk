/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SND - demos - stream
  File:     main.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
// USAGE:
//  A: start Bgm1
//  X : start Bgm2
//  B: stop Bgm
//---------------------------------------------------------------------------

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

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
    u32     bufPage;
    BOOL    isPlay;
}
StreamInfo;

static BOOL ReadWaveFormat(StreamInfo * strm);
static void ReadStrmData(StreamInfo * strm);
static void SoundAlarmHandler(void *arg);
static void StrmThread(void *arg);
static void VBlankIntr(void);
static void PlayStream(StreamInfo * strm, const char *filename);
static void StopStream(StreamInfo * strm);

static u16 Cont;
static u16 Trg;
static u64 strmThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread strmThread;
static OSMessageQueue msgQ;
static OSMessage msgBuf[1];

static u8 strm_lbuf[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u8 strm_rbuf[STRM_BUF_SIZE] ATTRIBUTE_ALIGN(32);
static u8 strm_tmp[STRM_BUF_PAGESIZE * 2] ATTRIBUTE_ALIGN(32);

// Filename
const char filename1[] = "kart_title.32.wav";
const char filename2[] = "fanfare.32.wav";

static StreamInfo strm;

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  Main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void NitroMain()
{
    // Initialization
    OS_Init();
    GX_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();
    FS_Init(FS_DMA_NOT_USE);
    SND_Init();

    FS_InitFile(&strm.file);
    strm.isPlay = FALSE;

    // V-Blank interrupt settings
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // Print usage
    OS_Printf("=================================\n");
    OS_Printf("USAGE:\n");
    OS_Printf(" A : start bgm1\n");
    OS_Printf(" X : start bgm2\n");
    OS_Printf(" B : stop bgm\n");
    OS_Printf("=================================\n");

    // Lock the channel
    SND_LockChannel((1 << L_CHANNEL) | (1 << R_CHANNEL), 0);

    /* Startup stream thread */
    OS_CreateThread(&strmThread,
                    StrmThread,
                    NULL,
                    strmThreadStack + THREAD_STACK_SIZE / sizeof(u64),
                    THREAD_STACK_SIZE, STREAM_THREAD_PRIO);
    OS_WakeupThreadDirect(&strmThread);

    //================ Main loop
    while (1)
    {
        u16     ReadData;

        OS_WaitVBlankIntr();

        // Receive ARM7 command reply
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

        ReadData = PAD_Read();
        Trg = (u16)(ReadData & (ReadData ^ Cont));
        Cont = ReadData;

        // Plays the PSG square wave
        if (Trg & PAD_BUTTON_A)
        {
            PlayStream(&strm, filename1);
        }

        if (Trg & PAD_BUTTON_X)
        {
            PlayStream(&strm, filename2);
        }

        if (Trg & PAD_BUTTON_B)
        {
            StopStream(&strm);
        }

        // Command flush
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         PlayStream

  Description:  Plays streaming playback.

  Arguments:    strm: Stream object
                filename: Name of the streaming playback file

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PlayStream(StreamInfo * strm, const char *filename)
{
    int     timerValue;
    u32     alarmPeriod;

    // Stops if during playback
    if (strm->isPlay)
    {
        u32     tag;
        StopStream(strm);
        tag = SND_GetCurrentCommandTag();
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK | SND_COMMAND_IMMEDIATE);
        SND_WaitForCommandProc(tag);   // Waits for a stop
    }

    // File scanning
    if (FS_IsFile(&strm->file))
        (void)FS_CloseFile(&strm->file);
    if (!FS_OpenFileEx(&strm->file, filename, FS_FILEMODE_R))
    {
        OS_Panic("Error: failed to open file %s\n", filename);
    }
    if (!ReadWaveFormat(strm))
    {
        OS_Panic("Error: failed to read wavefile\n");
    }

    strm->isPlay = TRUE;

    /* Set parameters */
    timerValue = SND_TIMER_CLOCK / strm->format.sampleRate;
    alarmPeriod = timerValue * STRM_BUF_PAGESIZE / 32U;
    alarmPeriod /= (strm->format.bitPerSample == 16) ? sizeof(s16) : sizeof(s8);

    // Read initial stream data
    (void)FS_SeekFile(&strm->file, (s32)strm->beginPos, FS_SEEK_SET);
    strm->bufPage = 0;
    ReadStrmData(strm);
    ReadStrmData(strm);

    // Set up the channel and alarm
    SND_SetupChannelPcm(L_CHANNEL,
                        (strm->format.bitPerSample ==
                         16) ? SND_WAVE_FORMAT_PCM16 : SND_WAVE_FORMAT_PCM8, strm_lbuf,
                        SND_CHANNEL_LOOP_REPEAT, 0, STRM_BUF_SIZE / sizeof(u32), 127,
                        SND_CHANNEL_DATASHIFT_NONE, timerValue, 0);
    SND_SetupChannelPcm(R_CHANNEL,
                        (strm->format.bitPerSample ==
                         16) ? SND_WAVE_FORMAT_PCM16 : SND_WAVE_FORMAT_PCM8,
                        (strm->format.channels == 1) ? strm_lbuf : strm_rbuf,
                        SND_CHANNEL_LOOP_REPEAT, 0, STRM_BUF_SIZE / sizeof(u32), 127,
                        SND_CHANNEL_DATASHIFT_NONE, timerValue, 127);
    SND_SetupAlarm(ALARM_NUM, alarmPeriod, alarmPeriod, SoundAlarmHandler, strm);
    SND_StartTimer((1 << L_CHANNEL) | (1 << R_CHANNEL), 0, 1 << ALARM_NUM, 0);
}

/*---------------------------------------------------------------------------*
  Name:         StopStream

  Description:  Stops streaming playback.

  Arguments:    strm: Stream object

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void StopStream(StreamInfo * strm)
{
    SND_StopTimer((1 << L_CHANNEL) | (1 << R_CHANNEL), 0, 1 << ALARM_NUM, 0);
    if (FS_IsFile(&strm->file))
        (void)FS_CloseFile(&strm->file);
    strm->isPlay = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         StrmThread

  Description:  The stream thread.

  Arguments:    arg: User data (unused)

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void StrmThread(void * /*arg */ )
{
    OSMessage message;

    OS_InitMessageQueue(&msgQ, msgBuf, 1);

    while (1)
    {
        (void)OS_ReceiveMessage(&msgQ, &message, OS_MESSAGE_BLOCK);
        (void)ReadStrmData((StreamInfo *) message);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SoundAlarmHandler

  Description:  Alarm callback function

  Arguments:    arg: Stream object

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void SoundAlarmHandler(void *arg)
{
    (void)OS_SendMessage(&msgQ, (OSMessage)arg, OS_MESSAGE_NOBLOCK);
}

/*---------------------------------------------------------------------------*
  Name:         ReadStrmData

  Description:  Function to read streaming data.
                Reads one page's worth of streaming data in the buffer from the file.

  Arguments:    strm: Stream object

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadStrmData(StreamInfo * strm)
{
    int     i;
    s32     readSize;
    u8     *lbuf, *rbuf;

    // Stream has reached its terminus
    if (strm->dataSize <= 0)
    {
        StopStream(strm);
        return;
    }

    // Buffer page settings
    if (strm->bufPage == 0)
    {
        lbuf = strm_lbuf;
        rbuf = strm_rbuf;
        strm->bufPage = 1;
    }
    else
    {
        lbuf = strm_lbuf + STRM_BUF_PAGESIZE;
        rbuf = strm_rbuf + STRM_BUF_PAGESIZE;
        strm->bufPage = 0;
    }

    // Read data
    if (strm->format.channels == 1)
    {
        // Monaural
        readSize = FS_ReadFile(&strm->file,
                               strm_tmp,
                               (strm->dataSize <
                                STRM_BUF_PAGESIZE) ? strm->dataSize : STRM_BUF_PAGESIZE);
        if (readSize == -1)
            OS_Panic("read file end\n");

        if (strm->format.bitPerSample == 16)
        {
            // 16-bit data
            for (i = 0; i < readSize / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = ((s16 *)strm_tmp)[i];
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = 0;  // If the stream has reached its terminus, the remainder is filled in with zeroes
            }
        }
        else
        {
            // 8-bit data
            for (i = 0; i < readSize / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = (s8)((s16)strm_tmp[i] - 128);
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = 0;
            }
        }
    }
    else
    {
        // Stereo
        readSize = FS_ReadFile(&strm->file,
                               strm_tmp,
                               (strm->dataSize <
                                STRM_BUF_PAGESIZE * 2) ? strm->dataSize : STRM_BUF_PAGESIZE * 2);
        if (readSize == -1)
            OS_Panic("read file end\n");

        if (strm->format.bitPerSample == 16)
        {
            // 16-bit data
            for (i = 0; i < (readSize / 2) / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = ((s16 *)strm_tmp)[2 * i];
                ((s16 *)rbuf)[i] = ((s16 *)strm_tmp)[2 * i + 1];
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s16); i++)
            {
                ((s16 *)lbuf)[i] = 0;
                ((s16 *)rbuf)[i] = 0;
            }
        }
        else
        {
            // 8-bit data
            for (i = 0; i < (readSize / 2) / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = (s8)((s16)strm_tmp[2 * i] - 128);
                ((s8 *)rbuf)[i] = (s8)((s16)strm_tmp[2 * i + 1] - 128);
            }
            for (; i < STRM_BUF_PAGESIZE / sizeof(s8); i++)
            {
                ((s8 *)lbuf)[i] = 0;
                ((s8 *)rbuf)[i] = 0;
            }
        }
    }

    strm->dataSize -= readSize;

    return;
}


/*---------------------------------------------------------------------------*
  Name:         ReadWaveFormat

  Description:  Gets the data size and first position of a data string, and the WAVE format data header.

  Arguments:    strm: Stream object

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
            // The chunk size for fmt is 16 bytes
            if (chunkSize != 16)
            {
                OS_Panic("wrong chunk size for fmt chunk(correct 16)");
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

//--------------------------------------------------------------------------------
//    V-Blank interrupt process
//
static void VBlankIntr(void)
{
    // Interrupt check flag
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
