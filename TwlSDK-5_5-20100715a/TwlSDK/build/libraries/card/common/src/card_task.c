/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_task.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-21#$
  $Rev: 9385 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "../include/card_task.h"


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitTaskQueue

  Description:  Initialize task queue structure

  Arguments:    queue: CARDTaskQueue structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitTaskQueue(CARDTaskQueue *queue)
{
    queue->list = NULL;
    queue->quit = FALSE;
    OS_InitThreadQueue(queue->workers);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_QuitTaskQueue

  Description:  Ends all threads that monitor the task queue

  Arguments:    queue: CARDTaskQueue structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void CARDi_QuitTaskQueue(CARDTaskQueue *queue)
{
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    queue->quit = TRUE;
    OS_WakeupThread(queue->workers);
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitTask

  Description:  Initialize task structure

  Arguments:    task: CARDTask structure
                priority: Thread priority
                userdata: Arbitrary user-defined data associated with task
                function: Function pointer for the task to execute
                callback: Callback that is called after task completion. NULL if not necessary.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitTask(CARDTask *task, u32 priority, void *userdata,
                    CARDTaskFunction function, CARDTaskFunction callback)
{
    task->next = NULL;
    task->priority = priority;
    task->userdata = userdata;
    task->function = function;
    task->callback = callback;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProcessTask

  Description:  Whether to run or add a task structure to the end of the task queue.

  Arguments:    queue: CARDTaskQueue structure
                task: Task that is added or run
                block: TRUE when there is a blocking operation.
                                 In such cases, run here without adding to the queue.
                changePriority: TRUE when following priority set in the task

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ProcessTask(CARDTaskQueue *queue, CARDTask *task, BOOL blocking, BOOL changePriority)
{
    // If there is a non-blocking operation, add to the end of the queue
    if (!blocking)
    {
        OSIntrMode  bak_cpsr = OS_DisableInterrupts();
        CARDTask  **pp = (CARDTask **)&queue->list;
        for(; *pp; pp = &(*pp)->next)
        {
        }
        *pp = task;
        // If it is the first task for an empty queue, notify the work thread
        if (pp == &queue->list)
        {
            OS_WakeupThread(queue->workers);
        }
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
    // If there is a blocking operation, run the task and callback here
    else
    {
        // If necessary, run under the priority set in the task
        OSThread   *curth = OS_GetCurrentThread();
        u32         prio = 0;
        if (changePriority)
        {
            prio = OS_GetThreadPriority(curth);
            (void)OS_SetThreadPriority(curth, task->priority);
        }
        if (task->function)
        {
            (*task->function)(task);
        }
        // Make so that it is acceptable to set the next task in the callback
        if (task->callback)
        {
            (*task->callback)(task);
        }
        // Restore priority that was temporarily changed
        if (changePriority)
        {
            (void)OS_SetThreadPriority(curth, prio);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReceiveTask

  Description:  Gets next task from the task list.

  Arguments:    queue : CARDTaskQueue structure
                block: TRUE when blocking when this does not exist

  Returns:      Returns obtained new task or NULL.
 *---------------------------------------------------------------------------*/
CARDTask* CARDi_ReceiveTask(CARDTaskQueue *queue, BOOL blocking)
{
    CARDTask   *retval = NULL;
    OSIntrMode  bak_cpsr = OS_DisableInterrupts();
    while (!queue->quit)
    {
        retval = queue->list;
        if ((retval != NULL) || !blocking)
        {
            break;
        }
        OS_SleepThread(queue->workers);
    }
    if (retval)
    {
        queue->list = retval->next;
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_TaskWorkerProcedure

  Description:  Procedure of the work thread that follows processing of the task list.

  Arguments:    arg: CARDTaskQueue structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_TaskWorkerProcedure(void *arg)
{
    CARDTaskQueue  *queue = (CARDTaskQueue*)arg;
    // To accept any priority level of tasks, remains at a high level while idle
    (void)OS_SetThreadPriority(OS_GetCurrentThread(), 0);
    for (;;)
    {
        // Monitors queue and idles until the next task arrives
        CARDTask   *task = CARDi_ReceiveTask(queue, TRUE);
        // If in a QUIT state, NULL is returned even in blocking mode
        if (!task)
        {
            break;
        }
        // Run the task and callback based on the specified priority
        CARDi_ProcessTask(queue, task, TRUE, TRUE);
    }
}





#if defined(SDK_ARM9)
/*---------------------------------------------------------------------------*
 * Old task thread processing
 *---------------------------------------------------------------------------*/

#include "../include/card_common.h"

/*---------------------------------------------------------------------------*
  Name:         CARDi_ExecuteOldTypeTask

  Description:  If asynchronous, throws to the work thread, and if synchronous, runs here
                (The caller of this function guarantees that the task thread is already exclusively controlled by CARDi_WaitTask().)
                 

  Arguments:    task: Task function to set
                async: TRUE if wanting an asynchronous process

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL CARDi_ExecuteOldTypeTask(void (*task) (CARDiCommon *), BOOL async)
{
    CARDiCommon *p = &cardi_common;
    if (async)
    {
        // First, change priority of task threads that are sleeping
        (void)OS_SetThreadPriority(p->thread.context, p->priority);
        // Set the task to process and wake the thread
        p->task_func = task;
        p->flag |= CARD_STAT_TASK;
        OS_WakeupThreadDirect(p->thread.context);
    }
    else
    {
        (*task)(p);
        CARDi_EndTask(p);
    }
    return async ? TRUE : (p->cmd->result == CARD_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_OldTypeTaskThread

  Description:  Main function for task thread

  Arguments:    arg: Not used

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_OldTypeTaskThread(void *arg)
{
    CARDiCommon *p = &cardi_common;
    (void)arg;
    for (;;)
    {
        // Wait for the next process
        OSIntrMode bak_psr = OS_DisableInterrupts();
        for (;;)
        {
            if ((p->flag & CARD_STAT_TASK) != 0)
            {
                break;
            }
            OS_SleepThread(NULL);
        }
        (void)OS_RestoreInterrupts(bak_psr);
        // Request processing
        (void)CARDi_ExecuteOldTypeTask(p->task_func, FALSE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitForTask

  Description:  Waits until it can obtain usage rights for the task thread.
                (Locking of the specified bus is guaranteed by the caller of this function.)

  Arguments:    p: Library work buffer
                restart: If starting even the next task, TRUE
                callback: Callback function after completion of access
                callback_arg:  Argument of callback function

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL CARDi_WaitForTask(CARDiCommon *p, BOOL restart, MIDmaCallback callback, void *callback_arg)
{
    OSIntrMode bak_psr = OS_DisableInterrupts();
    // Idle until current process is complete
    while ((p->flag & CARD_STAT_BUSY) != 0)
    {
        OS_SleepThread(p->busy_q);
    }
    // If executing the next command immediately, go to reprocessing here
    if (restart)
    {
        p->flag |= CARD_STAT_BUSY;
        p->callback = callback;
        p->callback_arg = callback_arg;
    }
    (void)OS_RestoreInterrupts(bak_psr);
    return (p->cmd->result == CARD_RESULT_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_EndTask

  Description:  Notifies that the task is complete and releases the usage rights of the task thread.

  Arguments:    p: Library's work buffer (passed by argument for efficiency)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EndTask(CARDiCommon *p)
{
    MIDmaCallback   callback = p->callback;
    void           *userdata = p->callback_arg;

    OSIntrMode bak_psr = OS_DisableInterrupts();
    // Set to completed state and notify idling thread
    p->flag &= ~(CARD_STAT_BUSY | CARD_STAT_TASK | CARD_STAT_CANCEL);
    OS_WakeupThread(p->busy_q);
    (void)OS_RestoreInterrupts(bak_psr);
    // Call the necessary callback
    if (callback)
    {
        (*callback) (userdata);
    }
}

#endif // defined(SDK_ARM9)
