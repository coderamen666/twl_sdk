/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_task.h

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

#if	!defined(NITRO_MB_TASK_H_)
#define NITRO_MB_TASK_H_


#if	defined(__cplusplus)
extern  "C"
{
#endif


/* constant ---------------------------------------------------------------- */

/* Expanded definitions for task thread priority levels */
    /* Specify a priority level just 1 higher than the caller */
#define MB_TASK_PRIORITY_ABOVE  (OS_THREAD_PRIORITY_MAX + 1)
    /* Specify a priority level just 1 lower than the caller */
#define MB_TASK_PRIORITY_BELOW  (OS_THREAD_PRIORITY_MAX + 2)
    /* Specify the same priority level as the caller */
#define MB_TASK_PRIORITY_NORMAL (OS_THREAD_PRIORITY_MAX + 3)

#define MB_TASK_WORK_MIN	(sizeof(OSThread) + 256)


/* struct ------------------------------------------------------------------ */

    struct MBiTaskInfo_tag;

    typedef void (*MB_TASK_FUNC) (struct MBiTaskInfo_tag *);

/*
 * The task information structure to request to the task thread.
 */
    typedef struct MBiTaskInfo_tag
    {
/* Private: */
        struct MBiTaskInfo_tag *next;  /* Next element as list */
        u32     busy:1;                /* If now processing, set 1 */
        u32     priority:31;           /* Thread priority */
        MB_TASK_FUNC task;             /* Task function */
        MB_TASK_FUNC callback;         /* Callback */
/* Public: */
        u32     param[4];              /* User-defined argument and return-value */
    }
    MBiTaskInfo;



/* Function ---------------------------------------------------------------- */


/*---------------------------------------------------------------------------*
  Name:         MBi_InitTaskThread

  Description:  Starts a task thread.
                
  Arguments:    p_work:     Internal work buffer.
                           Used internally until MBi_EndTaskThread() completes.
                size:       Byte size of p_work.
                           Must be greater than MB_TASK_WORK_MIN; size - MB_TASK_WORK_MIN is used by the stack.
                           

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    MBi_InitTaskThread(void *p_work, u32 size);

/*---------------------------------------------------------------------------*
  Name:         MBi_IsTaskAvailable

  Description:  Checks if a task thread is currently available.
                
  Arguments:    None.

  Returns:      TRUE if currently available, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
    BOOL    MBi_IsTaskAvailable(void);

/*---------------------------------------------------------------------------*
  Name:         MBi_InitTaskInfo

  Description:  Initializes a task information structure.
                Must be called once before using.

  Arguments:    pt:         Uninitialized task information structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    MBi_InitTaskInfo(MBiTaskInfo * pt);

/*---------------------------------------------------------------------------*
  Name:         MBi_IsTaskBusy

  Description:  Checks if task information is currently being used.
                
  Arguments:    pt:         Task information.

  Returns:      TRUE if currently being used, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
    BOOL    MBi_IsTaskBusy(volatile const MBiTaskInfo * pt);

/*---------------------------------------------------------------------------*
  Name:         MBi_SetTask

  Description:  Adds a task to an internal thread.
                
  Arguments:    pt:         Currently unused task information
                task:       Task function
                callback:   Callback when task completes (ignored if NULL)
                priority:   Thread priority while executing a task

  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    MBi_SetTask(MBiTaskInfo * pt, MB_TASK_FUNC task, MB_TASK_FUNC callback, u32 priority);

/*---------------------------------------------------------------------------*
  Name:         MBi_EndTaskThread

  Description:  Ends the task thread.
                
  Arguments:    callback:   Callback when task thread ends (ignored if NULL)
                           This callback is called in the state just before the task thread ends, while interrupts are still disabled.
                           
  Returns:      None.
 *---------------------------------------------------------------------------*/
    void    MBi_EndTaskThread(MB_TASK_FUNC callback);


#if	defined(__cplusplus)
}
#endif


#endif                                 /* NITRO_MB_TASK_H_ */
