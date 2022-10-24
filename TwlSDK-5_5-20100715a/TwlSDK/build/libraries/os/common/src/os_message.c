/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_message.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#include  <nitro/os.h>
#include  <nitro/os/common/message.h>

/*---------------------------------------------------------------------------*
  Name:         OS_InitMessageQueue

  Description:  initialize message queue

  Arguments:    mq          message queue
                msgArray    buffer for message queue
                msgCount    max massage size for buffer

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_InitMessageQueue(OSMessageQueue *mq, OSMessage *msgArray, s32 msgCount)
{
    OS_InitThreadQueue(&mq->queueSend);
    OS_InitThreadQueue(&mq->queueReceive);
    mq->msgArray = msgArray;
    mq->msgCount = msgCount;
    mq->firstIndex = 0;
    mq->usedCount = 0;
}

/*---------------------------------------------------------------------------*
  Name:         OS_SendMessage

  Description:  send message to message queue

  Arguments:    mq          message queue
                msg         massage which is sent
                flags       whether wait or not when queue is full

  Returns:      TRUE if send
                
 *---------------------------------------------------------------------------*/
BOOL OS_SendMessage(OSMessageQueue *mq, OSMessage msg, s32 flags)
{
    OSIntrMode enabled;
    s32     lastIndex;

    enabled = OS_DisableInterrupts();

    while (mq->msgCount <= mq->usedCount)       // check for full queue
    {
        if (!(flags & OS_MESSAGE_BLOCK))
        {
            (void)OS_RestoreInterrupts(enabled);
            return FALSE;
        }
        else
        {
            // Yield
            OS_SleepThread(&mq->queueSend);
        }
    }

    // Store message
    lastIndex = (mq->firstIndex + mq->usedCount) % mq->msgCount;
    mq->msgArray[lastIndex] = msg;
    mq->usedCount++;

    // Wakeup receiving threads if any
    OS_WakeupThread(&mq->queueReceive);

    (void)OS_RestoreInterrupts(enabled);
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         OS_ReceiveMessage

  Description:  receive message from message queue

  Arguments:    mq          message queue
                msg         massage which is received
                flags       whether wait or not when queue is empty

  Returns:      TRUE if reveice
                
 *---------------------------------------------------------------------------*/
BOOL OS_ReceiveMessage(OSMessageQueue *mq, OSMessage *msg, s32 flags)
{
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    while (mq->usedCount == 0)         // check for empty queue
    {
        if (!(flags & OS_MESSAGE_BLOCK))
        {
            (void)OS_RestoreInterrupts(enabled);
            return FALSE;
        }
        else
        {
            // Yield
            OS_SleepThread(&mq->queueReceive);
        }
    }

    // Copy-out message
    if (msg != NULL)
    {
        *msg = mq->msgArray[mq->firstIndex];
    }
    mq->firstIndex = (mq->firstIndex + 1) % mq->msgCount;
    mq->usedCount--;

    // Wakeup sending threads if any
    OS_WakeupThread(&mq->queueSend);

    (void)OS_RestoreInterrupts(enabled);
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         OS_JamMessage

  Description:  send message to message queue, cutting in the top of the queue

  Arguments:    mq          message queue
                msg         massage which is sent
                flags       whether wait or not when queue is full

  Returns:      TRUE if send
                
 *---------------------------------------------------------------------------*/
BOOL OS_JamMessage(OSMessageQueue *mq, OSMessage msg, s32 flags)
{
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    while (mq->msgCount <= mq->usedCount)       // check for full queue
    {
        if (!(flags & OS_MESSAGE_BLOCK))
        {
            (void)OS_RestoreInterrupts(enabled);
            return FALSE;
        }
        else
        {
            // Yield
            OS_SleepThread(&mq->queueSend);
        }
    }

    // Store message at the front
    mq->firstIndex = (mq->firstIndex + mq->msgCount - 1) % mq->msgCount;
    mq->msgArray[mq->firstIndex] = msg;
    mq->usedCount++;

    // Wakeup receiving threads if any
    OS_WakeupThread(&mq->queueReceive);

    (void)OS_RestoreInterrupts(enabled);
    return TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         OS_ReadMessage

  Description:  read message at message queue. no change in queue
                not occur to switch threads.

  Arguments:    mq          message queue
                msg         massage which is received
                flags       whether wait or not when queue is empty

  Returns:      TRUE if read
                
 *---------------------------------------------------------------------------*/
BOOL OS_ReadMessage(OSMessageQueue *mq, OSMessage *msg, s32 flags)
{
    OSIntrMode enabled;

    enabled = OS_DisableInterrupts();

    while (mq->usedCount == 0)         // check for empty queue
    {
        if (!(flags & OS_MESSAGE_BLOCK))
        {
            (void)OS_RestoreInterrupts(enabled);
            return FALSE;
        }
        else
        {
            // Yield
            OS_SleepThread(&mq->queueReceive);
        }
    }

    // Copy-out message
    if (msg != NULL)
    {
        *msg = mq->msgArray[mq->firstIndex];
    }

    (void)OS_RestoreInterrupts(enabled);
    return TRUE;
}
