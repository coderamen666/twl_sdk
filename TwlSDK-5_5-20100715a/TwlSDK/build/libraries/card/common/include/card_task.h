/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_task.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_TASK_H__
#define NITRO_LIBRARIES_CARD_TASK_H__


#include <nitro/os.h>


/*---------------------------------------------------------------------------*
 * This utility manages a priority task queue.
 * This is used only within the CARD library, but it could be taken and used separately.
 *---------------------------------------------------------------------------*/


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */


// Task structure that causes a worker thread to run a process in the background.
struct CARDTask;
typedef void (*CARDTaskFunction)(struct CARDTask *);

typedef struct CARDTask
{
    struct CARDTask    *next;
    u32                 priority;
    void               *userdata;
    CARDTaskFunction    function;
    CARDTaskFunction    callback;
}
CARDTask;

// Task queue structure.
typedef struct CARDTaskQueue
{
    CARDTask * volatile list;
    OSThreadQueue       workers[1];
    u32                 quit:1;
    u32                 dummy:31;
}
CARDTaskQueue;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitTaskQueue

  Description:  Initializes a task queue structure.

  Arguments:    queue:    CARDTaskQueue structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitTaskQueue(CARDTaskQueue *queue);

/*---------------------------------------------------------------------------*
  Name:         CARDi_QuitTaskQueue

  Description:  Stops all worker threads that observe the task queue and sets their states to QUIT.
                The task queue's state is left unchanged, and tasks in progress will continue to completion.

  Arguments:    queue    CARDTaskQueue structure.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_QuitTaskQueue(CARDTaskQueue *queue);

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitTask

  Description:  Initializes a task structure.

  Arguments:    task:      CARDTask structure.
                priority:  Thread priority.
                userdata:  Arbitrary user-defined data to associate with a task.
                function:  Function pointer to the task to run.
                callback:  Callback to invoke after task completion. (NULL if this is not necessary)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitTask(CARDTask *task, u32 priority, void *userdata,
                    CARDTaskFunction function, CARDTaskFunction callback);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ProcessTask

  Description:  Runs a task or adds it to a task queue.

  Arguments:    queue:    CARDTaskQueue structure.
                task:     Task to run or add to the queue.
                blocking:        TRUE to run it now without adding it to the queue.
                                 FALSE to add it to the queue and let a worker thread process it.
                changePriority:  TRUE to run at the priority set for the task.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ProcessTask(CARDTaskQueue *queue, CARDTask *task, BOOL blocking, BOOL changePriority);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReceiveTask

  Description:  Gets the next task from a task queue.

  Arguments:    queue:    CARDTaskQueue structure.
                blocking:  TRUE to block on an empty queue.

  Returns:      The new available task.
                NULL during non-blocking mode if the queue is empty, or whenever the queue's state is QUIT.
 *---------------------------------------------------------------------------*/
CARDTask* CARDi_ReceiveTask(CARDTaskQueue *queue, BOOL blocking);

/*---------------------------------------------------------------------------*
  Name:         CARDi_TaskWorkerProcedure

  Description:  Worker thread procedure that continues to monitor a task thread.
                The thread created by specifying this procedure will continue to process the task until the CARDi_QuitTask function is issued.
                
                You may create multiple worker threads for a single task queue.

  Arguments:    arg:  The CARDTaskQueue structure that should be monitored.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_TaskWorkerProcedure(void *arg);


/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_LIBRARIES_CARD_TASK_H__
