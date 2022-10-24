/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_task.c

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

#include <nitro.h>

#include "mb_task.h"


/* Struct ------------------------------------------------------------------ */

typedef struct
{
    OSThread th[1];                    /* Thread context */
    MBiTaskInfo *volatile list;        /* Waiting task list */
    MBiTaskInfo end_task;              /* Task structure for end-command */
}
MBiTaskWork;

/* Variable ---------------------------------------------------------------- */

static MBiTaskWork *mbi_task_work = NULL;


/* Function ---------------------------------------------------------------- */

static void MBi_TaskThread(void *arg)
{
    MBiTaskWork *const p = (MBiTaskWork *) arg;
    for (;;)
    {
        MBiTaskInfo *trg = NULL;
        /* Get the next task. */
        {
            OSIntrMode bak_cpsr = OS_DisableInterrupts();
            /* Sleep if in an idle state. */
            while (!p->list)
            {
                (void)OS_SetThreadPriority(p->th, OS_THREAD_PRIORITY_MIN);
                OS_SleepThread(NULL);
            }
            trg = p->list;
            p->list = p->list->next;
            (void)OS_SetThreadPriority(p->th, trg->priority);
            (void)OS_RestoreInterrupts(bak_cpsr);
        }
        /* Execute task. */
        if (trg->task)
            (*trg->task) (trg);
        /* Execute task completion callback. */
        {
            OSIntrMode bak_cpsr = OS_DisableInterrupts();
            MB_TASK_FUNC callback = trg->callback;
            /*
             * Here, we operate cautiously with regard to thread priority levels.
             * 1. If there is no next task, specify the highest (wait sleep).
             * 2. If there is a next task that is higher than the current one, change to that.
             * 3. If there is a next task that is lower than the current one, leave as is.
             * The priority level will never be lower than the current state.
             */
            const u32 cur_priority = OS_GetThreadPriority(p->th);
            u32     new_priority;
            if (!p->list)
                new_priority = OS_THREAD_PRIORITY_MIN;
            else if (cur_priority < p->list->priority)
                new_priority = p->list->priority;
            else
                new_priority = cur_priority;
            if (new_priority != cur_priority)
                (void)OS_SetThreadPriority(p->th, new_priority);
            trg->next = NULL;
            trg->busy = FALSE;
            if (callback)
                (*callback) (trg);
            /*
             * If an end request, the thread will end with interrupts prohibited.
             * (This disable setting is valid up to the moment of a context switch.)
             */
            if (trg == &p->end_task)
                break;
            (void)OS_RestoreInterrupts(bak_cpsr);
        }
    }
    OS_TPrintf("task-thread end.\n");
    OS_ExitThread();
    return;
}

/*---------------------------------------------------------------------------*
  Name:         MBi_InitTaskThread

  Description:  Starts a task thread.
                
  Arguments:    p_work:     Internal work buffer.
                           Used internally until MBi_EndTaskThread() completes.
                size:       Byte size of p_work.
                           Must be greater than MB_TASK_WORK_MIN; size - MB_TASK_WORK_MIN is used by the stack.
                           

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_InitTaskThread(void *p_work, u32 size)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    if (!mbi_task_work)
    {
        MBiTaskWork *const p = (MBiTaskWork *) p_work;

        SDK_ASSERT(size >= MB_TASK_WORK_MIN);
        SDK_ASSERT(OS_IsThreadAvailable());

        /* Prepare the work structure, stack buffer and task thread. */
        mbi_task_work = p;
        MBi_InitTaskInfo(&p->end_task);
        p->list = NULL;
        size = (u32)((size - sizeof(MBiTaskWork)) & ~3);
        OS_CreateThread(p->th, MBi_TaskThread, p,
                        (u8 *)(p + 1) + size, size, OS_THREAD_PRIORITY_MIN);
        OS_WakeupThreadDirect(p->th);
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_IsTaskAvailable

  Description:  Checks if a task thread is currently available.
                
  Arguments:    None.

  Returns:      TRUE if currently available, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL MBi_IsTaskAvailable(void)
{
    return (mbi_task_work != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         MBi_InitTaskInfo

  Description:  Initializes a task information structure.
                Must be called once before using.

  Arguments:    pt:         Uninitialized task information structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_InitTaskInfo(MBiTaskInfo * pt)
{
    SDK_ASSERT(pt != NULL);
    MI_CpuClear8(pt, sizeof(*pt));
}

/*---------------------------------------------------------------------------*
  Name:         MBi_IsTaskBusy

  Description:  Checks if task information is currently being used.
                
  Arguments:    pt:         Task information.

  Returns:      TRUE if currently being used, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL MBi_IsTaskBusy(volatile const MBiTaskInfo * pt)
{
    return pt->busy != FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         MBi_SetTask

  Description:  Adds a task to an internal thread.
                
  Arguments:    pt:         Currently-unused task information
                task:       Task function
                callback:   Callback when task completes (ignored if NULL)
                priority:   Thread priority while executing a task

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_SetTask(MBiTaskInfo * pt, MB_TASK_FUNC task, MB_TASK_FUNC callback, u32 priority)
{
    MBiTaskWork *const p_work = mbi_task_work;

    SDK_ASSERT(pt != NULL);

    /* Changed so that processing is silently ignored when the library shuts down or a card is removed. */
    if (!MBi_IsTaskAvailable())
    {
        OS_TWarning("MBi_SetTask() ignored... (task-thread is not available now)");
        return;
    }
    if (pt->busy)
    {
        OS_TWarning("MBi_SetTask() ignored... (specified structure is busy)");
        return;
    }

    /* Support for expanded definitions of priority levels. */
    if (priority > OS_THREAD_PRIORITY_MAX)
    {
        const u32 cur_priority = OS_GetThreadPriority(p_work->th);
        if (priority == MB_TASK_PRIORITY_ABOVE)
        {
            /* Only one priority level higher than the caller. */
            priority = (u32)((cur_priority > OS_THREAD_PRIORITY_MIN) ?
                             (cur_priority - 1) : OS_THREAD_PRIORITY_MIN);
        }
        else if (priority == MB_TASK_PRIORITY_BELOW)
        {
            /* Only one priority level lower than the caller. */
            priority = (u32)((cur_priority < OS_THREAD_PRIORITY_MAX) ?
                             (cur_priority + 1) : OS_THREAD_PRIORITY_MAX);
        }
        else if (priority == MB_TASK_PRIORITY_NORMAL)
        {
            /* Same priority level as the caller. */
            priority = cur_priority;
        }
        else
        {
            /* Merely an invalid specification. */
            priority = OS_THREAD_PRIORITY_MAX;
        }
    }
    /* Add task. */
    {
        OSIntrMode bak_cpsr = OS_DisableInterrupts();
        pt->busy = TRUE;
        pt->priority = priority;
        pt->task = task;
        pt->callback = callback;
        /* Activate the thread if the task is new and in an idle state. */
        if (!p_work->list)
        {

            if (pt == &p_work->end_task)
            {
                /* Prohibit task thread usage from here. */
                mbi_task_work = NULL;
            }

            p_work->list = pt;
            OS_WakeupThreadDirect(p_work->th);
        }
        else
        {
            /* Insert if the list is not empty. */
            MBiTaskInfo *pos = p_work->list;
            /* Always add to the end if this is an end command. */
            if (pt == &p_work->end_task)
            {
                while (pos->next)
                    pos = pos->next;
                pos->next = pt;
                /* Prohibit task thread usage from here. */
                mbi_task_work = NULL;
            }
            /* Insert in order of priority if this is a normal command. */
            else
            {
                if (pos->priority > priority)
                {
                    p_work->list = pt;
                    pt->next = pos;
                }
                else
                {
                    while (pos->next && (pos->next->priority <= priority))
                        pos = pos->next;
                    pt->next = pos->next;
                    pos->next = pt;
                }
            }
        }
        (void)OS_RestoreInterrupts(bak_cpsr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         MBi_EndTaskThread

  Description:  Ends the task thread.
                
  Arguments:    callback:   Callback when task thread ends (ignored if NULL)
                           This callback is called in the state just before the task thread ends, while interrupts are still disabled.
                           
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_EndTaskThread(MB_TASK_FUNC callback)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    if (MBi_IsTaskAvailable())
    {
        MBi_SetTask(&mbi_task_work->end_task, NULL, callback, OS_THREAD_PRIORITY_MIN);
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}
