/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - include
  File:     ctrdg_task.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_CTRDG_TASK_H_
#define	NITRO_CTRDG_TASK_H_

#include <nitro.h>

/* Task thread priority level */
#define CTRDG_TASK_PRIORITY_DEFAULT 20 // Enables the creation of multiple threads between the task and main threads, with lower priorities than that of the main thread.

/* Task thread's stack size */
#define CTRDG_TASK_STACK_SIZE 1024

struct CTRDGTaskInfo_tag;

// Argument declares a CTRDGTaskInfo_tag function pointer.
typedef u32 (*CTRDG_TASK_FUNC) (struct CTRDGTaskInfo_tag *);

/*
 * The task information structure to request to the task thread.
 */
typedef struct CTRDGTaskInfo_tag
{
    // TRUE, in the interval between leaving the standby loop which waits for a task to come for the waiting task list in the thread, and the task's completion.
    CTRDG_TASK_FUNC task;              /* Task function */
    CTRDG_TASK_FUNC callback;          /* Callback */
    u32     result;                    /* The task function's return value. */
    u8     *data;                      /* The data to be written; only program commands can be used. */
    u8     *adr;                       /* Address of the data to be read/written. */
    u32     offset;                    /* The offset, in bytes, within the sector */
    u32     size;                      /* Size */
    u8     *dst;                       /* Address of the work region where the read data is stored */
    u16     sec_num;                   /* Sector number */
    u8      busy;                      /* If now processing */
    u8      param[1];                  /* User-defined argument and return-value */
}
CTRDGTaskInfo;

typedef struct
{
    OSThread th[1];                    /* Thread context */
    CTRDGTaskInfo *volatile list;      /* Waiting task list */
    CTRDGTaskInfo end_task;            /* Task structure for end-command */
}
CTRDGiTaskWork;

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_InitTaskThread

  Description:  Starts a task thread.
                
  Arguments:    p_work:     Internal work buffer.
                           Used internally until CTRDGi_EndTaskThread() completes.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDGi_InitTaskThread(void *p_work);

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsTaskAvailable

  Description:  Checks if a task thread is currently available.
                
  Arguments:    None.

  Returns:      TRUE if currently available, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL    CTRDGi_IsTaskAvailable(void);

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_InitTaskInfo

  Description:  Initializes a task information structure.
                Must be called once before using.

  Arguments:    pt:         Uninitialized task information structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDGi_InitTaskInfo(CTRDGTaskInfo * pt);

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_IsTaskBusy

  Description:  Checks if task information is currently being used.
                
  Arguments:    pt:         Task information.

  Returns:      TRUE if currently being used, and FALSE otherwise.
 *---------------------------------------------------------------------------*/
BOOL    CTRDGi_IsTaskBusy(volatile const CTRDGTaskInfo * pt);

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_SetTask

  Description:  Adds a task to an internal thread.
                
  Arguments:    pt:         Currently unused task information
                task:       Task function
                callback:   Callback when task completes (ignored if NULL)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDGi_SetTask(CTRDGTaskInfo * pt, CTRDG_TASK_FUNC task, CTRDG_TASK_FUNC callback);

/*---------------------------------------------------------------------------*
  Name:         CTRDGi_EndTaskThread

  Description:  Ends the task thread.
                
  Arguments:    callback:   Callback when task thread ends (ignored if NULL)
                           This callback is called in the state just before the task thread ends, while interrupts are still disabled.
                           
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDGi_EndTaskThread(CTRDG_TASK_FUNC callback);

/*---------------------------------------------------------------------------*
  Name:         CTRDG_SetTaskThreadPriority

  Description:  Changes the task thread's priority.
                
  Arguments:    priority:   The task thread's priority
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CTRDG_SetTaskThreadPriority(u32 priority);

#endif /* NITRO_CTRDG_TASK_H_ */
