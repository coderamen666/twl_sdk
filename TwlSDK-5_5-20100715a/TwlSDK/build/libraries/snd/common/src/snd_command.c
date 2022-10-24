/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - snd - common
  File:     snd_command.c

  Copyright 2004-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <nitro/snd/common/command.h>

#include <nitro/misc.h>
#include <nitro/os.h>
#include <nitro/pxi.h>
#include <nitro/mi.h>

#include <nitro/snd/common/seq.h>
#include <nitro/snd/common/capture.h>
#include <nitro/snd/common/work.h>
#include <nitro/snd/common/global.h>
#include <nitro/snd/common/util.h>
#include <nitro/snd/common/alarm.h>
#include <nitro/snd/common/main.h>

/******************************************************************************
    Macro definitions
 ******************************************************************************/

#define SND_COMMAND_NUM 256

#define SND_PXI_FIFO_MESSAGE_BUFSIZE   8

#define SND_MSG_REQUEST_COMMAND_PROC   0

#define UNPACK_COMMAND( arg, shift, bit ) ( ( (arg) >> (shift) ) & ( ( 1 << (bit) ) - 1 ) )

/******************************************************************************
    Static variable
 ******************************************************************************/

#ifdef SDK_ARM9

static SNDCommand sCommandArray[SND_COMMAND_NUM] ATTRIBUTE_ALIGN(32);   /* Command array */
static SNDCommand *sFreeList;          /* Free command list */
static SNDCommand *sFreeListEnd;       /* End of free command list */

static SNDCommand *sReserveList;       /* Reserved command list */
static SNDCommand *sReserveListEnd;    /* End of reserved command list */

static SNDCommand *sWaitingCommandListQueue[SND_PXI_FIFO_MESSAGE_BUFSIZE + 1];
static int sWaitingCommandListQueueRead;
static int sWaitingCommandListQueueWrite;

static int sWaitingCommandListCount;   /* Issued command list count */

static u32 sCurrentTag;                /* Current tag */
static u32 sFinishedTag;               /* Completed tag */

static SNDSharedWork sSharedWork ATTRIBUTE_ALIGN(32);

#else  /* SDK_ARM7 */

static OSMessage sCommandMesgBuffer[SND_PXI_FIFO_MESSAGE_BUFSIZE];
static OSMessageQueue sCommandMesgQueue;

#endif /* SDK_ARM9 */


/******************************************************************************
    Static function declarations
 ******************************************************************************/

static void PxiFifoCallback(PXIFifoTag tag, u32 data, BOOL err);
static void InitPXI(void);

#ifdef SDK_ARM9
static void RequestCommandProc(void);
static SNDCommand *AllocCommand(void);
static BOOL IsCommandAvailable(void);
#else
static void SetChannelTimer(u32 chBitMask, int timer);
static void SetChannelVolume(u32 chBitMask, int volume, SNDChannelDataShift shift);
static void SetChannelPan(u32 chBitMask, int pan);
static void StartTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);
static void StopTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags);
static void ReadDriverInfo(SNDDriverInfo * info);
#endif

/******************************************************************************
    Public functions
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         SND_CommandInit

  Description:  Initializes the command library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_CommandInit(void)
{
#ifdef SDK_ARM9
    SNDCommand *command;
    int     i;
#endif /* SDK_ARM9 */

#ifdef SDK_ARM7
    OS_InitMessageQueue(&sCommandMesgQueue, sCommandMesgBuffer, SND_PXI_FIFO_MESSAGE_BUFSIZE);
#endif

    InitPXI();

#ifdef SDK_ARM9
    /* Create free list */
    sFreeList = &sCommandArray[0];
    for (i = 0; i < SND_COMMAND_NUM - 1; i++)
    {
        sCommandArray[i].next = &sCommandArray[i + 1];
    }
    sCommandArray[SND_COMMAND_NUM - 1].next = NULL;
    sFreeListEnd = &sCommandArray[SND_COMMAND_NUM - 1];

    /* Initialize reserve list */
    sReserveList = NULL;
    sReserveListEnd = NULL;

    /* Initialize other variables */
    sWaitingCommandListCount = 0;

    sWaitingCommandListQueueRead = 0;
    sWaitingCommandListQueueWrite = 0;

    sCurrentTag = 1;
    sFinishedTag = 0;

    /* Initialize shared work */
    SNDi_SharedWork = &sSharedWork;
    SNDi_InitSharedWork(SNDi_SharedWork);

    command = SND_AllocCommand(SND_COMMAND_BLOCK);
    if (command != NULL)
    {
        command->id = SND_COMMAND_SHARED_WORK;
        command->arg[0] = (u32)SNDi_SharedWork;

        SND_PushCommand(command);
        (void)SND_FlushCommand(SND_COMMAND_BLOCK);
    }

#else  /* SDK_ARM7 */

    SNDi_SharedWork = NULL;

#endif /* SDK_ARM9 */
}


#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         SND_RecvCommandReply
  
  Description:  Receives reply (ThreadSafe).
  
  Arguments:    flags - BLOCK or NOBLOCK
  
  Returns:      Processed command list, or NULL.
  *---------------------------------------------------------------------------*/
const SNDCommand *SND_RecvCommandReply(u32 flags)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    SNDCommand *commandList;
    SNDCommand *commandListEnd;

    if (flags & SND_COMMAND_BLOCK)
    {
        while (sFinishedTag == SNDi_GetFinishedCommandTag())
        {
            // Retry
            (void)OS_RestoreInterrupts(bak_psr);
            OS_SpinWait(100);
            bak_psr = OS_DisableInterrupts();
        }
    }
    else
    {
        if (sFinishedTag == SNDi_GetFinishedCommandTag())
        {
            (void)OS_RestoreInterrupts(bak_psr);
            return NULL;
        }
    }

    /* POP from the waiting command list */
    commandList = sWaitingCommandListQueue[sWaitingCommandListQueueRead];
    sWaitingCommandListQueueRead++;
    if (sWaitingCommandListQueueRead > SND_PXI_FIFO_MESSAGE_BUFSIZE)
        sWaitingCommandListQueueRead = 0;

    /* Get the end of the command list */
    commandListEnd = commandList;
    while (commandListEnd->next != NULL)
    {
        commandListEnd = commandListEnd->next;
    }

    /* Join to the end of the free list  */
    if (sFreeListEnd != NULL)
    {
        sFreeListEnd->next = commandList;
    }
    else
    {
        sFreeList = commandList;
    }
    sFreeListEnd = commandListEnd;

    /* Update counter */
    sWaitingCommandListCount--;
    sFinishedTag++;

    (void)OS_RestoreInterrupts(bak_psr);
    return commandList;
}

/*---------------------------------------------------------------------------*
  Name:         SND_AllocCommand

  Description:  Allocates command (ThreadSafe).

  Arguments:    flags - BLOCK or NOBLOCK

  Returns:      Commands.
 *---------------------------------------------------------------------------*/
SNDCommand *SND_AllocCommand(u32 flags)
{
    SNDCommand *command;

    if (!IsCommandAvailable())
        return NULL;

    command = AllocCommand();
    if (command != NULL)
        return command;

    if ((flags & SND_COMMAND_BLOCK) == 0)
        return NULL;

    if (SND_CountWaitingCommand() > 0)
    {
        /* There are commands waiting to be processed */

        /* Try receiving the completed command list */
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

        /* Is there a free command? */
        command = AllocCommand();
        if (command != NULL)
            return command;
    }
    else
    {
        /* There are no commands waiting to be processed */

        /* Issue the current command list */
        (void)SND_FlushCommand(SND_COMMAND_BLOCK);
    }

    /* Request immediate processing */
    RequestCommandProc();

    /* Wait for command processing to complete */
    do
    {
        (void)SND_RecvCommandReply(SND_COMMAND_BLOCK);
        command = AllocCommand();
    } while (command == NULL);

    return command;
}

/*---------------------------------------------------------------------------*
  Name:         SND_PushCommand

  Description:  Adds a command to the command list (ThreadSafe).

  Arguments:    Command to add

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_PushCommand(struct SNDCommand *command)
{
    OSIntrMode bak_psr;

    SDK_NULL_ASSERT(command);

    bak_psr = OS_DisableInterrupts();

    // Add to end of sReserveList

    if (sReserveListEnd == NULL)
    {
        sReserveList = command;
        sReserveListEnd = command;
    }
    else
    {
        sReserveListEnd->next = command;
        sReserveListEnd = command;
    }

    command->next = NULL;

    (void)OS_RestoreInterrupts(bak_psr);
}

/*---------------------------------------------------------------------------*
  Name:         SND_FlushCommand

  Description:  Sends the command list (ThreadSafe).

  Arguments:    flags - BLOCK or NOBLOCK

  Returns:      Whether it was successful.
 *---------------------------------------------------------------------------*/
BOOL SND_FlushCommand(u32 flags)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();

    if (sReserveList == NULL)
    {
        /* There are no reserved commands, so do nothing */
        (void)OS_RestoreInterrupts(bak_psr);
        return TRUE;
    }

    if (sWaitingCommandListCount >= SND_PXI_FIFO_MESSAGE_BUFSIZE)
    {
        /* A backlog of command processing on ARM7 has built up */
        if ((flags & SND_COMMAND_BLOCK) == 0)
        {
            (void)OS_RestoreInterrupts(bak_psr);
            return FALSE;
        }

        do
        {
            (void)SND_RecvCommandReply(SND_COMMAND_BLOCK);
        } while (sWaitingCommandListCount >= SND_PXI_FIFO_MESSAGE_BUFSIZE);

        /* Verified again in SND_RecvCommandReply because of the occurrence of interrupt processing */
        if (sReserveList == NULL)
        {
            /* There are no reserved commands, so do nothing */
            (void)OS_RestoreInterrupts(bak_psr);
            return TRUE;
        }
    }

    /* Issue command list */
    DC_FlushRange(sCommandArray, sizeof(sCommandArray));
    if (PXI_SendWordByFifo(PXI_FIFO_TAG_SOUND, (u32)sReserveList, FALSE) < 0)
    {
        if ((flags & SND_COMMAND_BLOCK) == 0)
        {
            (void)OS_RestoreInterrupts(bak_psr);
            return FALSE;
        }

        while (sWaitingCommandListCount >= SND_PXI_FIFO_MESSAGE_BUFSIZE ||
               PXI_SendWordByFifo(PXI_FIFO_TAG_SOUND, (u32)sReserveList, FALSE) < 0)
        {
            /* Wait until successful */
            (void)OS_RestoreInterrupts(bak_psr);
            (void)SND_RecvCommandReply(SND_COMMAND_NOBLOCK);
            bak_psr = OS_DisableInterrupts();

            DC_FlushRange(sCommandArray, sizeof(sCommandArray));
            /* Verified again because of the occurrence of interrupt processing */
            if (sReserveList == NULL)
            {
                /* There are no reserved commands, so do nothing */
                (void)OS_RestoreInterrupts(bak_psr);
                return TRUE;
            }
        }
    }

    /* PUSH into the command wait queue */
    sWaitingCommandListQueue[sWaitingCommandListQueueWrite] = sReserveList;
    sWaitingCommandListQueueWrite++;
    if (sWaitingCommandListQueueWrite > SND_PXI_FIFO_MESSAGE_BUFSIZE)
        sWaitingCommandListQueueWrite = 0;

    /* Update variables */
    sReserveList = NULL;
    sReserveListEnd = NULL;

    sWaitingCommandListCount++;
    sCurrentTag++;

    (void)OS_RestoreInterrupts(bak_psr);

    if (flags & SND_COMMAND_IMMEDIATE)
    {
        /* Request immediate command processing */
        RequestCommandProc();
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         SND_WaitForCommandProc

  Description:  Synchronizes the completion of command processing (ThreadSafe).

  Arguments:    tag - Command tag

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_WaitForCommandProc(u32 tag)
{
    if (SND_IsFinishedCommandTag(tag))
    {
        /* Already completed */
        return;
    }

    /* Try receiving the completed command list */
    while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
    {
    }
    if (SND_IsFinishedCommandTag(tag))
    {
        /* It has completed */
        return;
    }

    /* Request immediate command processing */
    RequestCommandProc();

    /* Wait for finish */
    while (!SND_IsFinishedCommandTag(tag))
    {
        (void)SND_RecvCommandReply(SND_COMMAND_BLOCK);
    }
}

/*---------------------------------------------------------------------------*
  Name:         SND_WaitForFreeCommand

  Description:  Waits for free commands (ThreadSafe).

  Arguments:    count - Required number of free commands

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_WaitForFreeCommand(int count)
{
    int     freeCount = SND_CountFreeCommand();

    SDK_MAX_ASSERT(count, SND_COMMAND_NUM);

    if (freeCount >= count)
        return;

    if (freeCount + SND_CountWaitingCommand() >= count)
    {
        /* OK if we wait for the completion of a command waiting to be processed */

        /* Try receiving the completed command list */
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

        /* Is there a free command? */
        if (SND_CountFreeCommand() >= count)
            return;
    }
    else
    {
        /* It is necessary to process reserved commands  */

        /* Issue the current command list */
        (void)SND_FlushCommand(SND_COMMAND_BLOCK);
    }

    /* Request immediate processing */
    RequestCommandProc();

    /* Wait for command processing to complete */
    do
    {
        (void)SND_RecvCommandReply(SND_COMMAND_BLOCK);
    } while (SND_CountFreeCommand() < count);
}

/*---------------------------------------------------------------------------*
  Name:         SND_GetCurrentCommandTag

  Description:  Gets command tag (ThreadSafe).

  Arguments:    None.

  Returns:      Command tag.
 *---------------------------------------------------------------------------*/
u32 SND_GetCurrentCommandTag(void)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    u32     tag;

    if (sReserveList == NULL)
    {
        tag = sFinishedTag;
    }
    else
    {
        tag = sCurrentTag;
    }

    (void)OS_RestoreInterrupts(bak_psr);
    return tag;
}

/*---------------------------------------------------------------------------*
  Name:         SND_IsFinishedCommandTag

  Description:  Checks whether or not a command tag has already been processed (ThreadSafe).

  Arguments:    tag - Command tag

  Returns:      Whether it was processed or not.
 *---------------------------------------------------------------------------*/
BOOL SND_IsFinishedCommandTag(u32 tag)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    BOOL    result;

    if (tag > sFinishedTag)
    {
        if (tag - sFinishedTag < 0x80000000)
        {
            result = FALSE;
        }
        else
        {
            result = TRUE;
        }
    }
    else
    {
        if (sFinishedTag - tag < 0x80000000)
        {
            result = TRUE;
        }
        else
        {
            result = FALSE;
        }
    }

    (void)OS_RestoreInterrupts(bak_psr);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         SND_CountFreeCommand

  Description:  Gets the number of free commands (ThreadSafe).

  Arguments:    None.

  Returns:      Number of free commands.
 *---------------------------------------------------------------------------*/
int SND_CountFreeCommand(void)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    SNDCommand *command;
    int     count = 0;

    command = sFreeList;
    while (command != NULL)
    {
        ++count;
        command = command->next;
    }

    (void)OS_RestoreInterrupts(bak_psr);
    return count;
}

/*---------------------------------------------------------------------------*
  Name:         SND_CountReservedCommand

  Description:  Gets the number of reserved commands (ThreadSafe).

  Arguments:    None.

  Returns:      Number of reserved commands.
 *---------------------------------------------------------------------------*/
int SND_CountReservedCommand(void)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    SNDCommand *command;
    int     count = 0;

    command = sReserveList;
    while (command != NULL)
    {
        ++count;
        command = command->next;
    }

    (void)OS_RestoreInterrupts(bak_psr);
    return count;
}

/*---------------------------------------------------------------------------*
  Name:         SND_CountWaitingCommand

  Description:  Gets the number of commands waiting to be processed (ThreadSafe).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
int SND_CountWaitingCommand(void)
{
    int     freeCount = SND_CountFreeCommand();
    int     reservedCount = SND_CountReservedCommand();

    return SND_COMMAND_NUM - freeCount - reservedCount;
}

#else  /* SDK_ARM7 */

/*---------------------------------------------------------------------------*
  Name:         SND_CommandProc

  Description:  Processes the command list.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SND_CommandProc(void)
{
    OSMessage message;
    const SNDCommand *command_p;
    SNDCommand command;

    while (OS_ReceiveMessage(&sCommandMesgQueue, &message, OS_MESSAGE_NOBLOCK))
    {
        command_p = (const SNDCommand *)message;

        while (command_p != NULL)
        {
            // Note: MainMemory Access
            command = *command_p;

            switch (command.id)
            {
            case SND_COMMAND_START_SEQ:
                SND_StartSeq((int)command.arg[0],
                             (const void *)command.arg[1],
                             command.arg[2], (struct SNDBankData *)command.arg[3]);
                break;

            case SND_COMMAND_STOP_SEQ:
                SND_StopSeq((int)command.arg[0]);
                break;

            case SND_COMMAND_PREPARE_SEQ:
                SND_PrepareSeq((int)command.arg[0],
                               (const void *)command.arg[1],
                               command.arg[2], (struct SNDBankData *)command.arg[3]);
                break;

            case SND_COMMAND_START_PREPARED_SEQ:
                SND_StartPreparedSeq((int)command.arg[0]);
                break;

            case SND_COMMAND_PAUSE_SEQ:
                SND_PauseSeq((int)command.arg[0], (BOOL)command.arg[1]);
                break;

            case SND_COMMAND_SKIP_SEQ:
                SND_SkipSeq((int)command.arg[0], (u32)command.arg[1]);
                break;

            case SND_COMMAND_PLAYER_PARAM:
                SNDi_SetPlayerParam((int)command.arg[0],
                                    command.arg[1], command.arg[2], (int)command.arg[3]);
                break;

            case SND_COMMAND_TRACK_PARAM:
                SNDi_SetTrackParam((int)UNPACK_COMMAND(command.arg[0], 0, 24),
                                   command.arg[1],
                                   command.arg[2],
                                   command.arg[3], (int)UNPACK_COMMAND(command.arg[0], 24, 8));
                break;

            case SND_COMMAND_MUTE_TRACK:
                SND_SetTrackMute((int)command.arg[0], command.arg[1], (SNDSeqMute) command.arg[2]);
                break;

            case SND_COMMAND_ALLOCATABLE_CHANNEL:
                SND_SetTrackAllocatableChannel((int)command.arg[0], command.arg[1], command.arg[2]);
                break;

            case SND_COMMAND_PLAYER_LOCAL_VAR:
                SND_SetPlayerLocalVariable((int)command.arg[0],
                                           (int)command.arg[1], (s16)command.arg[2]);
                break;

            case SND_COMMAND_PLAYER_GLOBAL_VAR:
                SND_SetPlayerGlobalVariable((int)command.arg[0], (s16)command.arg[1]);
                break;

            case SND_COMMAND_START_TIMER:
                StartTimer(command.arg[0], command.arg[1], command.arg[2], command.arg[3]);
                break;

            case SND_COMMAND_STOP_TIMER:
                StopTimer(command.arg[0], command.arg[1], command.arg[2], command.arg[3]);
                break;

            case SND_COMMAND_SETUP_CAPTURE:
                SND_SetupCapture((SNDCapture)UNPACK_COMMAND(command.arg[2], 31, 1),
                                 (SNDCaptureFormat)UNPACK_COMMAND(command.arg[2], 30, 1),
                                 (void *)command.arg[0],
                                 (int)command.arg[1],
                                 (BOOL)UNPACK_COMMAND(command.arg[2], 29, 1),
                                 (SNDCaptureIn)UNPACK_COMMAND(command.arg[2], 28, 1),
                                 (SNDCaptureOut)UNPACK_COMMAND(command.arg[2], 27, 1));
                break;

            case SND_COMMAND_SETUP_ALARM:
                SND_SetupAlarm((int)command.arg[0],
                               (OSTick)command.arg[1], (OSTick)command.arg[2], (int)command.arg[3]);
                break;

            case SND_COMMAND_CHANNEL_TIMER:
                SetChannelTimer((u32)command.arg[0], (int)command.arg[1]);
                break;

            case SND_COMMAND_CHANNEL_VOLUME:
                SetChannelVolume((u32)command.arg[0],
                                 (int)command.arg[1], (SNDChannelDataShift)command.arg[2]);
                break;

            case SND_COMMAND_CHANNEL_PAN:
                SetChannelPan((u32)command.arg[0], (int)command.arg[1]);
                break;

            case SND_COMMAND_SETUP_CHANNEL_PCM:
                SND_SetupChannelPcm((int)UNPACK_COMMAND(command.arg[0], 0, 16), /* chNo */
                                    (const void *)UNPACK_COMMAND(command.arg[1], 0, 27),        /* dataaddr */
                                    (SNDWaveFormat)UNPACK_COMMAND(command.arg[3], 24, 2),       /* format */
                                    (SNDChannelLoop)UNPACK_COMMAND(command.arg[3], 26, 2),      /* loop */
                                    (int)UNPACK_COMMAND(command.arg[3], 0, 16), /* loopStart */
                                    (int)UNPACK_COMMAND(command.arg[2], 0, 22), /* loopLen */
                                    (int)UNPACK_COMMAND(command.arg[2], 24, 7), /* volume */
                                    (SNDChannelDataShift)UNPACK_COMMAND(command.arg[2], 22, 2), /* SHIFT */
                                    (int)UNPACK_COMMAND(command.arg[0], 16, 16),        /* timer */
                                    (int)UNPACK_COMMAND(command.arg[3], 16, 7)  /* pan */
                    );
                break;

            case SND_COMMAND_SETUP_CHANNEL_PSG:
                SND_SetupChannelPsg((int)command.arg[0],        /* chNo */
                                    (SNDDuty)command.arg[3],    /* duty */
                                    (int)UNPACK_COMMAND(command.arg[1], 0, 7),  /* volume */
                                    (SNDChannelDataShift)UNPACK_COMMAND(command.arg[1], 8, 2),  /* SHIFT */
                                    (int)UNPACK_COMMAND(command.arg[2], 8, 16), /* timer */
                                    (int)UNPACK_COMMAND(command.arg[2], 0, 7)   /* pan */
                    );
                break;

            case SND_COMMAND_SETUP_CHANNEL_NOISE:
                SND_SetupChannelNoise((int)command.arg[0],      /* chNo */
                                      (int)UNPACK_COMMAND(command.arg[1], 0, 7),        /* volume */
                                      (SNDChannelDataShift)UNPACK_COMMAND(command.arg[1], 8, 2),        /* SHIFT */
                                      (int)UNPACK_COMMAND(command.arg[2], 8, 16),       /* timer */
                                      (int)UNPACK_COMMAND(command.arg[2], 0, 7) /* pan */
                    );
                break;

            case SND_COMMAND_SURROUND_DECAY:
                SNDi_SetSurroundDecay((int)command.arg[0]);
                break;

            case SND_COMMAND_MASTER_VOLUME:
                SND_SetMasterVolume((int)command.arg[0]);
                break;

            case SND_COMMAND_MASTER_PAN:
                SND_SetMasterPan((int)command.arg[0]);
                break;

            case SND_COMMAND_OUTPUT_SELECTOR:
                SND_SetOutputSelector((SNDOutput)command.arg[0],
                                      (SNDOutput)command.arg[1],
                                      (SNDChannelOut)command.arg[2], (SNDChannelOut)command.arg[3]);
                break;

            case SND_COMMAND_LOCK_CHANNEL:
                SND_LockChannel(command.arg[0], command.arg[1]);
                break;

            case SND_COMMAND_UNLOCK_CHANNEL:
                SND_UnlockChannel(command.arg[0], command.arg[1]);
                break;

            case SND_COMMAND_STOP_UNLOCKED_CHANNEL:
                SND_StopUnlockedChannel(command.arg[0], command.arg[1]);
                break;

            case SND_COMMAND_INVALIDATE_SEQ:
                SND_InvalidateSeq((const void *)command.arg[0], (const void *)command.arg[1]);
                break;

            case SND_COMMAND_INVALIDATE_BANK:
                SND_InvalidateBank((const void *)command.arg[0], (const void *)command.arg[1]);
                break;

            case SND_COMMAND_INVALIDATE_WAVE:
                SND_InvalidateWave((const void *)command.arg[0], (const void *)command.arg[1]);
                break;

            case SND_COMMAND_SHARED_WORK:
                SNDi_SharedWork = (SNDSharedWork *)command.arg[0];
                break;

            case SND_COMMAND_READ_DRIVER_INFO:
                ReadDriverInfo((SNDDriverInfo *) command.arg[0]);
                break;
            }

            command_p = command.next;

        }                              // end of while ( command_p != NULL )

        /* Command list processing complete */
        SDK_NULL_ASSERT(SNDi_SharedWork);
        SNDi_SharedWork->finishCommandTag++;
    }
}

#endif /* SDK_ARM9 */

/******************************************************************************
    Static function
 ******************************************************************************/

/*---------------------------------------------------------------------------*
  Name:         PxiFifoCallback

  Description:  PXI callback.

  Arguments:    tag  - PXI tag
                data - User data (address or message number)
                err  - Error flag

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void PxiFifoCallback(PXIFifoTag tag, u32 data, BOOL /*err */ )
{
    OSIntrMode enabled;
    BOOL    result;

#ifdef SDK_DEBUG
    SDK_ASSERT(tag == PXI_FIFO_TAG_SOUND);
#else
#pragma unused( tag )
#endif

    enabled = OS_DisableInterrupts();

#ifdef SDK_ARM9

#pragma unused( result )
    SNDi_CallAlarmHandler((int)data);

#else  /* SDK_ARM7 */

    if (data >= HW_MAIN_MEM)
    {
        /* This is controlled by the sender, so it should not fail */
        result = OS_SendMessage(&sCommandMesgQueue, (OSMessage)data, OS_MESSAGE_NOBLOCK);
        SDK_TASSERTMSG(result, "Failed OS_SendMessage");
    }
    else
    {
        switch (data)
        {
        case SND_MSG_REQUEST_COMMAND_PROC:
            {
                (void)SND_SendWakeupMessage();
                break;
            }
        }
    }

#endif /* SDK_ARM9 */

    (void)OS_RestoreInterrupts(enabled);
}

/*---------------------------------------------------------------------------*
  Name:         InitPXI

  Description:  Initializes PXI.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitPXI(void)
{
    // Register the callback and attempt initial synchronization with the other party
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_SOUND, PxiFifoCallback);

#ifdef SDK_ARM9
    if (IsCommandAvailable())
    {
        while (!PXI_IsCallbackReady(PXI_FIFO_TAG_SOUND, PXI_PROC_ARM7))
        {
            OS_SpinWait(100);
        }
    }
#endif
}

#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         RequestCommandProc

  Description:  Request command processing (ThreadSafe).

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void RequestCommandProc(void)
{
    while (PXI_SendWordByFifo(PXI_FIFO_TAG_SOUND, SND_MSG_REQUEST_COMMAND_PROC, FALSE) < 0)
    {
        // retry
    }
}

/*---------------------------------------------------------------------------*
  Name:         AllocCommand

  Description:  Allocates command (ThreadSafe).

  Arguments:    None.

  Returns:      Command or NULL.
 *---------------------------------------------------------------------------*/
static SNDCommand *AllocCommand(void)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    SNDCommand *command;

    if (sFreeList == NULL)
    {
        (void)OS_RestoreInterrupts(bak_psr);
        return NULL;
    }

    command = sFreeList;
    sFreeList = sFreeList->next;

    if (sFreeList == NULL)
        sFreeListEnd = NULL;

    (void)OS_RestoreInterrupts(bak_psr);
    return command;
}

/*---------------------------------------------------------------------------*
  Name:         IsCommandAvailable

  Description:  Checks the validity of a command routine.

  Arguments:    None.

  Returns:      Whether or not the command routine is valid.
 *---------------------------------------------------------------------------*/
static BOOL IsCommandAvailable(void)
{
    OSIntrMode prev_mode;
    u32     res;

    if (!OS_IsRunOnEmulator())
        return TRUE;

    prev_mode = OS_DisableInterrupts();

    /* Confirm sound validity */
    *(vu32 *)0x4fff200 = 0x10;
    res = *(vu32 *)0x4fff200;

    (void)OS_RestoreInterrupts(prev_mode);

    return res ? TRUE : FALSE;
}


#else  /* SDK_ARM7 */


static void SetChannelTimer(u32 chBitMask, int timer)
{
    int     ch;

    for (ch = 0; ch < SND_CHANNEL_NUM && chBitMask != 0; ch++, chBitMask >>= 1)
    {
        if ((chBitMask & 0x01) == 0)
            continue;
        SND_SetChannelTimer(ch, timer);
    }
}

static void SetChannelVolume(u32 chBitMask, int volume, SNDChannelDataShift shift)
{
    int     ch;

    for (ch = 0; ch < SND_CHANNEL_NUM && chBitMask != 0; ch++, chBitMask >>= 1)
    {
        if ((chBitMask & 0x01) == 0)
            continue;
        SND_SetChannelVolume(ch, volume, shift);
    }
}

static void SetChannelPan(u32 chBitMask, int pan)
{
    int     ch;

    for (ch = 0; ch < SND_CHANNEL_NUM && chBitMask != 0; ch++, chBitMask >>= 1)
    {
        if ((chBitMask & 0x01) == 0)
            continue;
        SND_SetChannelPan(ch, pan);
    }
}


static void StartTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 /*flags */ )
{
    OSIntrMode enabled;
    int     i;

    enabled = OS_DisableInterrupts();

    for (i = 0; i < SND_CHANNEL_NUM && chBitMask != 0; i++, chBitMask >>= 1)
    {
        if ((chBitMask & 0x01) == 0)
            continue;
        SND_StartChannel(i);
    }

    if (capBitMask & (1 << SND_CAPTURE_0))
    {
        if (capBitMask & (1 << SND_CAPTURE_1))
        {
            SND_StartCaptureBoth();
        }
        else
        {
            SND_StartCapture(SND_CAPTURE_0);
        }
    }
    else if (capBitMask & (1 << SND_CAPTURE_1))
    {
        SND_StartCapture(SND_CAPTURE_1);
    }

    for (i = 0; i < SND_ALARM_NUM && alarmBitMask != 0; i++, alarmBitMask >>= 1)
    {
        if ((alarmBitMask & 0x01) == 0)
            continue;
        SND_StartAlarm(i);
    }

    (void)OS_RestoreInterrupts(enabled);

    SND_UpdateSharedWork();
}

static void StopTimer(u32 chBitMask, u32 capBitMask, u32 alarmBitMask, u32 flags)
{
    OSIntrMode enabled;
    int     i;

    enabled = OS_DisableInterrupts();

    for (i = 0; i < SND_ALARM_NUM && alarmBitMask != 0; i++, alarmBitMask >>= 1)
    {
        if ((alarmBitMask & 0x01) == 0)
            continue;
        SND_StopAlarm(i);
    }

    for (i = 0; i < SND_CHANNEL_NUM && chBitMask != 0; i++, chBitMask >>= 1)
    {
        if ((chBitMask & 0x01) == 0)
            continue;
        SND_StopChannel(i, (s32)flags);
    }

    if (capBitMask & (1 << SND_CAPTURE_0))
    {
        SND_StopCapture(SND_CAPTURE_0);
    }
    if (capBitMask & (1 << SND_CAPTURE_1))
    {
        SND_StopCapture(SND_CAPTURE_1);
    }

    (void)OS_RestoreInterrupts(enabled);

    SND_UpdateSharedWork();
}

static void ReadDriverInfo(SNDDriverInfo * info)
{
    int     ch;

    MI_CpuCopy32(&SNDi_Work, &info->work, sizeof(SNDi_Work));

    info->workAddress = &SNDi_Work;

    for (ch = 0; ch < SND_CHANNEL_NUM; ch++)
    {
        info->chCtrl[ch] = SND_GetChannelControl(ch);
    }

    info->lockedChannels = SND_GetLockedChannel(0);
}

#endif /* SDK_ARM9 */

/*====== End of snd_command.c ======*/
