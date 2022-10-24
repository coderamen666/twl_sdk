/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_common.h

  Copyright 2007-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-26#$
  $Rev: 10827 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_COMMON_H__
#define NITRO_LIBRARIES_CARD_COMMON_H__


#include <nitro/card/common.h>
#include <nitro/card/backup.h>


#include "../include/card_utility.h"
#include "../include/card_task.h"
#include "../include/card_command.h"



#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Constants */

// CARD internal state
enum
{
    // Initialized
    CARD_STAT_INIT = (1 << 0),

    // PXI command initialized (ARM9)
    CARD_STAT_INIT_CMD = (1 << 1),

    // A command was issued but has not completed. (ARM9)
    // This also includes the state before processing in the WaitForTask function.
    CARD_STAT_BUSY = (1 << 2),

    // There is a transferred task (ARM9/ARM7)
    CARD_STAT_TASK = (1 << 3),

    // Waiting to be notified that a request from the ARM7 has completed (ARM9)
    CARD_STAT_WAITFOR7ACK = (1 << 5),

    // Currently requesting cancellation
    CARD_STAT_CANCEL = (1 << 6)
};

#define CARD_UNSYNCHRONIZED_BUFFER  (void*)0x80000000

typedef enum
{
    CARD_TARGET_NONE,
    CARD_TARGET_ROM,
    CARD_TARGET_BACKUP,
    CARD_TARGET_RW
}
CARDTargetMode;

typedef u32 CARDAccessLevel;
#define CARD_ACCESS_LEVEL_NONE      0x0000UL
#define CARD_ACCESS_LEVEL_BACKUP_R  0x0001UL
#define CARD_ACCESS_LEVEL_BACKUP_W  0x0002UL
#define CARD_ACCESS_LEVEL_BACKUP    (u32)(CARD_ACCESS_LEVEL_BACKUP_R | CARD_ACCESS_LEVEL_BACKUP_W)
#define CARD_ACCESS_LEVEL_ROM       0x0004UL
#define CARD_ACCESS_LEVEL_FULL      (u32)(CARD_ACCESS_LEVEL_BACKUP | CARD_ACCESS_LEVEL_ROM)


/*---------------------------------------------------------------------------*/
/* Declarations */

typedef s32 CARDiOwner;

// CARD common parameters
typedef struct CARDiCommon
{
    // Common memory for request communications
    CARDiCommandArg *cmd;

    // Status flag
    volatile u32    flag;

    // Default task priority
    u32             priority;

#if	defined(SDK_ARM9)
    // Threshold value for invalidating the entire cache when reading from a ROM.
    // FlushAll is more efficient than FlushRange past a fixed size.
    u32     flush_threshold_ic;
    u32     flush_threshold_dc;
#endif

    // Card access rights management.
    // Take exclusive control of the card/backup in the processor.
    // This is necessary because multiple asynchronous functions (Rom and Backup) that use card access may be called from the same thread.
    // 
    //
    // This follows a lock-ID because OSMutex follows a thread
    volatile CARDiOwner lock_owner;    // ==s32 with Error status
    volatile int        lock_ref;
    OSThreadQueue       lock_queue[1];
    CARDTargetMode      lock_target;

    // Task thread information
    struct
    {
        OSThread    context[1];
        u8          stack[0x400];
    }
    thread;

#if defined(SDK_ARM7)
    // New task procedure format
    CARDTask        task[1];
    CARDTaskQueue   task_q[1];
    // The most recent command from the ARM9
    int             command;
    u8              padding[20];
#else
    // Old task procedure format
    void          (*task_func) (struct CARDiCommon *);
    // User callback and arguments
    MIDmaCallback   callback;
    void           *callback_arg;
    // Thread that waits for tasks to complete
    OSThreadQueue   busy_q[1];
    // API task parameters
    u32                     src;
    u32                     dst;
    u32                     len;
    u32                     dma;
    const CARDDmaInterface *DmaCall;
    // Information for managing backup commands
    CARDRequest             req_type;
    int                     req_retry;
    CARDRequestMode         req_mode;
    OSThread               *current_thread_9;
//    u8      padding[32];
#endif

}
CARDiCommon;


SDK_COMPILER_ASSERT(sizeof(CARDiCommon) % 32 == 0);


/*---------------------------------------------------------------------------*/
/* Variables */

extern CARDiCommon cardi_common;
extern u32  cardi_rom_base;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_ExecuteOldTypeTask

  Description:  Gives asynchronous processing over to a worker thread, and runs synchronous processing here.
                (It is up to this function's caller to guarantee that the task thread has already been locked for exclusive control by the CARDi_WaitForTask function)
                 

  Arguments:    task       Task function to set
                async      TRUE if asynchronous processing is desired.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL CARDi_ExecuteOldTypeTask(void (*task) (CARDiCommon *), BOOL async);

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitForTask

  Description:  Waits until the task thread's usage rights are available.
                (It is up to this function's caller to guarantee that the designated bus is locked)

  Arguments:    p            The library's work buffer
                restart      TRUE if the next task should be started
                callback     Callback function after completion of access
                callback_arg  Argument of callback function

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL CARDi_WaitForTask(CARDiCommon *p, BOOL restart, MIDmaCallback callback, void *callback_arg);

/*---------------------------------------------------------------------------*
  Name:         CARDi_EndTask

  Description:  Sends a notification that a task is complete and releases the usage rights of the task thread.

  Arguments:    p            Library's work buffer (passed by argument for efficiency)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_EndTask(CARDiCommon *p);

void CARDi_OldTypeTaskThread(void *arg);


/*---------------------------------------------------------------------------*
  Name:         CARDi_GetTargetMode

  Description:  Gets the current lock target of the CARD bus.

  Arguments:    None.

  Returns:      One of the three states indicated by CARDTargetMode.
 *---------------------------------------------------------------------------*/
SDK_INLINE CARDTargetMode CARDi_GetTargetMode(void)
{
    return cardi_common.lock_target;
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_LockResource

  Description:  Resource exclusive lock

  Arguments:    owner      lock-ID indicating the owner of the lock
                target     Resource target on the card bus to be locked

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_LockResource(CARDiOwner owner, CARDTargetMode target);

/*---------------------------------------------------------------------------*
  Name:         CARDi_UnlockResource

  Description:  Unlocks exclusive control of the resource.

  Arguments:    owner      lock-ID indicating the owner of the lock
                target     Resource target on the card bus to be unlocked

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_UnlockResource(CARDiOwner owner, CARDTargetMode target);

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetAccessLevel

  Description:  Gets the permitted ROM access level.

  Arguments:    None.

  Returns:      Permitted ROM access level.
 *---------------------------------------------------------------------------*/
CARDAccessLevel CARDi_GetAccessLevel(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_WaitAsync

  Description:  Waits for asynchronous completion.

  Arguments:    None.

  Returns:      TRUE if the latest processing result is CARD_RESULT_SUCCESS.
 *---------------------------------------------------------------------------*/
BOOL    CARDi_WaitAsync(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_TryWaitAsync

  Description:  Attempts to wait for asynchronous completion and returns control immediately whether it succeeds or fails.

  Arguments:    None.

  Returns:      TRUE if the most recent asynchronous process has completed.
 *---------------------------------------------------------------------------*/
BOOL    CARDi_TryWaitAsync(void);

void CARDi_InitResourceLock(void);



void CARDi_InitCommand(void);


#ifdef __cplusplus
} // extern "C"
#endif



#endif // NITRO_LIBRARIES_CARD_COMMON_H__
