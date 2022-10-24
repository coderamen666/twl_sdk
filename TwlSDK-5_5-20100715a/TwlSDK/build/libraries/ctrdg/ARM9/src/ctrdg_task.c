/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - libraries - ARM9
  File:     ctrdg_task.c

  Copyright 2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#include <nitro.h>

/*******************************************************

    Function's description

********************************************************/
static CTRDGiTaskWork *ctrdgi_task_work = NULL;
static CTRDGTaskInfo ctrdgi_task_list;

static void CTRDGi_TaskThread(void *arg);

u64     ctrdg_task_stack[CTRDG_TASK_STACK_SIZE / sizeof(u64)];
/*---------------------------------------------------------------------------*
  Name:         CTRDGi_InitTaskThread

  Description:  Starts a task thread.
                
  Arguments:    p_work: Internal work buffer.
                           Used internally until CTRDGi_EndTaskThread() completes.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDGi_InitTaskThread(void *p_work)
{
    // IRQ interrupt prohibition
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    // Create a thread if this structure is NULL
    if (!ctrdgi_task_work)
    {
        CTRDGiTaskWork *const p = (CTRDGiTaskWork *) p_work;

        // Determines whether the thread has been initialized and can be used
        SDK_ASSERT(OS_IsThreadAvailable());

        /* Prepare the work structure, stack buffer and task thread. */
        // At this point, the structure will no longer be NULL, so task threads are not created anew 
        ctrdgi_task_work = p;
        // Initializes the end_task structure
        CTRDGi_InitTaskInfo(&p->end_task);
        // Initializes the ctrdgi_task_list structure
        CTRDGi_InitTaskInfo(&ctrdgi_task_list);
        // There should be no waiting task lists at this point, so insert NULL
        p->list = NULL;

        OS_CreateThread(p->th, CTRDGi_TaskThread, p,
                        ctrdg_task_stack + CTRDG_TASK_STACK_SIZE / sizeof(u64),
                        CTRDG_TASK_STACK_SIZE, CTRDG_TASK_PRIORITY_DEFAULT);
        OS_WakeupThreadDirect(p->th);
    }
    // Restore IRQ interrupt permission
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsTaskAvailable

  Description:  Checks if a task thread is currently available.
                
  Arguments:    None.

  Returns:      TRUE if currently available, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL CTRDGi_IsTaskAvailable(void)
{
    return (ctrdgi_task_work != NULL);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_InitTaskInfo

  Description:  Initializes a task information structure.
                Must be called once before using.

  Arguments:    pt: Uninitialized task information structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDGi_InitTaskInfo(CTRDGTaskInfo * pt)
{
    SDK_ASSERT(pt != NULL);
    MI_CpuClear8(pt, sizeof(*pt));
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsTaskBusy

  Description:  Checks if task information is currently being used.
                
  Arguments:    pt: Task information

  Returns:      TRUE if currently being used, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL CTRDGi_IsTaskBusy(volatile const CTRDGTaskInfo * pt)
{
    return pt->busy != FALSE;
}

static void CTRDGi_TaskThread(void *arg)
{
    CTRDGiTaskWork *const p = (CTRDGiTaskWork *) arg;
    // Loop until a command to end the thread comes
    for (;;)
    {
        // Initializes the structure
        CTRDGTaskInfo trg;
        MI_CpuClear8(&trg, sizeof(CTRDGTaskInfo));
        /* Get the next task */
        {
            // IRQ interrupts prohibited
            OSIntrMode bak_cpsr = OS_DisableInterrupts();
            /* Sleep if in an idle state. */
            // Loop and wait until a task comes to the waiting task list
            while (!p->list)
            {
                OS_SleepThread(NULL);
            }
            // Because a task has come to the waiting task list, copy that task data structure to trg
            trg = *p->list;
            // Restore IRQ interrupt permission
            (void)OS_RestoreInterrupts(bak_cpsr);
        }
        /* Execute task */
        if (trg.task)
            // Run the task with the function pointer, with trg as an argument
            trg.result = (u32)(*trg.task) (&trg);
        /* Execute task completion callback. */
        // if you've come here, the task is over, so use the task callback
        {
            // IRQ interrupts prohibited
            OSIntrMode bak_cpsr = OS_DisableInterrupts();
            // Set the callback function
            CTRDG_TASK_FUNC callback = trg.callback;

            // FALSE because the task shouldn't be running at this point
            ctrdgi_task_list.busy = FALSE;
            // If there is a callback function
            if (callback)
                // Call the callback function with the function pointer, and trg as an argument
                (void)(*callback) (&trg);
            /*
             * If an end request, the thread will end with interrupts disabled.
             * (This disable setting is valid up to the moment of a context switch.)
             */
            //if (p->list == &p->end_task)
            if (ctrdgi_task_work == NULL)
                break;

            // Initialize the list structure
            p->list = NULL;

            (void)OS_RestoreInterrupts(bak_cpsr);
        }
    }
    OS_TPrintf("task-thread end.\n");
    OS_ExitThread();
    return;
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_SetTask

  Description:  Adds a task to an internal thread.
                
  Arguments:    pt: Currently unused task information
                task: Task function
                callback: Callback when task completes (ignored if NULL)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDGi_SetTask(CTRDGTaskInfo * pt, CTRDG_TASK_FUNC task, CTRDG_TASK_FUNC callback)
{
    // Insert the structure that has the current thread pointer and waiting task list
    CTRDGiTaskWork *const p_work = ctrdgi_task_work;

    SDK_ASSERT(pt != NULL);
    SDK_ASSERT(CTRDGi_IsTaskAvailable());

    if (!CTRDGi_IsTaskAvailable())
    {
        OS_TPanic("CTRDGi_SetTask() failed! (task-thread is not available now)");
    }

    // Something is amiss if there is a running task in the thread
    if (ctrdgi_task_list.busy)
    {
        OS_TPanic("CTRDGi_SetTask() failed! (specified structure is busy)");
    }

    /* Add task */
    {
        // Sets the structure's parameters
        OSIntrMode bak_cpsr = OS_DisableInterrupts();
        pt->busy = TRUE;
        pt->task = task;
        pt->callback = callback;
        /* Activate the thread if the task is new and in an idle state */

        // If this is the command that ends that task
        if (pt == &p_work->end_task)
        {
            /* Prohibit task thread usage from here */
            ctrdgi_task_work = NULL;
        }
        // Insert this task's structure into the waiting task list and launch the task thread
        ctrdgi_task_list = *pt;
        // Stores the actual address
        p_work->list = &ctrdgi_task_list;
        OS_WakeupThreadDirect(p_work->th);

        (void)OS_RestoreInterrupts(bak_cpsr);
    }
}

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_EndTaskThread

  Description:  Ends the task thread.
                
  Arguments:    callback: Callback when task thread ends (ignored if NULL)
                           This callback is called in the state just before the task thread ends, while interrupts are still disabled.
                           
  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDGi_EndTaskThread(CTRDG_TASK_FUNC callback)
{
    OSIntrMode bak_cpsr = OS_DisableInterrupts();
    if (CTRDGi_IsTaskAvailable())
    {
        (void)CTRDGi_SetTask(&ctrdgi_task_work->end_task, NULL, callback);
    }
    (void)OS_RestoreInterrupts(bak_cpsr);
}

/*---------------------------------------------------------------------------*
  Name:         CTRDG_SetTaskThreadPriority

  Description:  Changes the task thread's priority.
                
  Arguments:    priority: Task thread's priority
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void CTRDG_SetTaskThreadPriority(u32 priority)
{
    if (ctrdgi_task_work)
    {
        CTRDGiTaskWork *const p = ctrdgi_task_work;
        (void)OS_SetThreadPriority(p->th, priority);
    }
}
