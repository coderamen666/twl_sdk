/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - 
  File:     os_mutex.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-12#$
  $Rev: 9293 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#include <nitro/types.h>
#include <nitro.h>

void OSi_UnlockMutexCore(OSMutex *mutex, u32 type);

void    OSi_EnqueueTail(OSThread *thread, OSMutex *mutex);
void    OSi_DequeueItem(OSThread *thread, OSMutex *mutex);
OSMutex *OSi_DequeueHead(OSThread *thread);

/*---------------------------------------------------------------------------*
  Name:         OS_InitMutex

  Description:  initialize mutex

  Arguments:    mutex       pointer to mutex structure
                            to be initialized

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_InitMutex(OSMutex *mutex)
{
    SDK_ASSERT(mutex);

    OS_InitThreadQueue(&mutex->queue);
    mutex->thread = NULL;
    OS_SetMutexCount( mutex, 0 );
    OS_SetMutexType( mutex, OS_MUTEX_TYPE_NONE );
}

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutex

  Description:  lock mutex

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutex(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    OSThread *currentThread = OS_GetCurrentThread();

    while(1)
    {
        //---- try lock mutex
        if ( OS_TryLockMutex(mutex) )
        {
            break;
        }

        currentThread->mutex = mutex;
        OS_SleepThread(&mutex->queue);
        currentThread->mutex = NULL;
    }

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutex

  Description:  unlock mutex

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_UnlockMutex(OSMutex *mutex)
{
    OSi_UnlockMutexCore(mutex, OS_MUTEX_TYPE_STD);
}

/*---------------------------------------------------------------------------*
  Name:         OSi_UnlockAllMutex

  Description:  unlocks all the mutexes locked by the thread

  Arguments:    mutex       pointer to mutex structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_UnlockAllMutex(OSThread *thread)
{
    OSMutex *mutex;

    SDK_ASSERT(thread);

#ifndef SDK_THREAD_INFINITY
    while (thread->mutexQueueHead)
    {
        mutex = OSi_DequeueHead(thread);
        SDK_ASSERT(mutex->thread == thread);

        OS_SetMutexCount( mutex, 0 );
        mutex->thread = NULL;
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_NONE );
        OS_WakeupThread(&(mutex->queue));
    }
#else
    while (thread->mutexQueue.head)
    {
        mutex = OSi_RemoveMutexLinkFromQueue(&thread->mutexQueue);
        SDK_ASSERT(mutex->thread == thread);

        OS_SetMutexCount( mutex, 0 );
        mutex->thread = NULL;
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_NONE );
        OS_WakeupThread(&mutex->queue);
    }
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutex

  Description:  try to lock mutex

  Arguments:    mutex       pointer to mutex structure

  Returns:      True if lock
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutex(OSMutex *mutex)
{
    OSIntrMode saved = OS_DisableInterrupts();
    OSThread *currentThread = OS_GetCurrentThread();
    BOOL    locked;

    SDK_ASSERT(mutex);

    // ---- able to lock mutex
    if (mutex->thread == NULL)
    {
        mutex->thread = currentThread;
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_STD );
        OS_IncreaseMutexCount(mutex);
        OSi_EnqueueTail(currentThread, mutex);
        locked = TRUE;
    }
    // ---- current thread is same with thread locking mutex
    else if (mutex->thread == currentThread)
    {
        OS_IncreaseMutexCount(mutex);
        locked = TRUE;
    }
    // ---- current thread is different from locking mutex
    else
    {
        locked = FALSE;
    }

    (void)OS_RestoreInterrupts(saved);
    return locked;
}

//================================================================================
// Read / Write Lock
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexR

  Description:  lock RW mutex as READ access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexR(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    OSThread *currentThread = OS_GetCurrentThread();

    while(1)
    {
        //---- try lock by READ
        if ( OS_TryLockMutexR(mutex) )
        {
            break;
        }

        currentThread->mutex = mutex;
        OS_SleepThread(&mutex->queue);
        currentThread->mutex = NULL;
    }

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexW

  Description:  lock RW mutex as WRITE access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexW(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    OSThread *currentThread = OS_GetCurrentThread();

    while(1)
    {
        //---- try lock by WRITE
        if ( OS_TryLockMutexW(mutex) )
        {
            break;
        }

        currentThread->mutex = mutex;
        OS_SleepThread(&mutex->queue);
        currentThread->mutex = NULL;
    }

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexR

  Description:  try to lock RW mutex as READ access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      TRUE if locked
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexR(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    BOOL locked = FALSE;
    OSThread * currentThread = OS_GetCurrentThread();

    switch( OS_GetMutexType(mutex) )
    {
    case OS_MUTEX_TYPE_NONE:
        mutex->thread = currentThread;
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_R );
        OS_SetMutexCount( mutex, 1 );
        OSi_EnqueueTail(currentThread, mutex);
        locked = TRUE;
        break;

    case OS_MUTEX_TYPE_R:
        OS_IncreaseMutexCount(mutex);
        locked = TRUE;
        break;

    case OS_MUTEX_TYPE_W:
    default:
        break;
    }

    (void)OS_RestoreInterrupts(e);
    return locked;
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexW

  Description:  try to lock RW mutex as WRITE access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      TRUE if locked
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexW(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    BOOL locked = FALSE;
    OSThread * currentThread = OS_GetCurrentThread();

    switch( OS_GetMutexType(mutex) )
    {
    case OS_MUTEX_TYPE_NONE:
        mutex->thread = currentThread;
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_W );
        OS_SetMutexCount( mutex, 1 );
        OSi_EnqueueTail(currentThread, mutex);
        locked = TRUE;
        break;

    case OS_MUTEX_TYPE_W:
        if ( mutex->thread == currentThread )
        {
            OS_IncreaseMutexCount(mutex);
            locked = TRUE;
        }
        break;

    case OS_MUTEX_TYPE_R:
    default:
        break;
    }

    (void)OS_RestoreInterrupts(e);
    return locked;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_UnlockMutexCore

  Description:  core routine to unlock mutex

  Arguments:    mutex       pointer to RW mutex structure
                type        mutex type. if different from OS_GetMutexType(), error.
                              OS_MUTEX_TYPE_STD
                              OS_MUTEX_TYPE_R
                              OS_MUTEX_TYPE_W
                              OS_MUTEX_TYPE_NONE (no check)

  Returns:      None
 *---------------------------------------------------------------------------*/
void OSi_UnlockMutexCore(OSMutex *mutex, u32 type )
{
    OSIntrMode e = OS_DisableInterrupts();
    OSThread *currentThread = OS_GetCurrentThread();
    BOOL unlocked = FALSE;

    SDK_ASSERT(mutex);

    //---- check unlock type
    if ( type != OS_MUTEX_TYPE_NONE && type != OS_GetMutexType(mutex) )
    {
        //OS_TPanic("Illegal unlock mutex\n");
        OS_TWarning("Illegal unlock mutex");
        (void)OS_RestoreInterrupts(e);
        return;
    }

    switch( OS_GetMutexType(mutex) )
    {
    case OS_MUTEX_TYPE_STD:
    case OS_MUTEX_TYPE_W:
        if ( mutex->thread == currentThread )
        {
            OS_DecreaseMutexCount(mutex);
            if ( OS_GetMutexCount(mutex) == 0 )
            {
                unlocked = TRUE;
            }
        }
        break;

    case OS_MUTEX_TYPE_R:
        OS_DecreaseMutexCount(mutex);
        if ( OS_GetMutexCount(mutex) == 0 )
        {
            unlocked = TRUE;
        }
        break;

    default:
        OS_TWarning("Illegal unlock mutex");
        (void)OS_RestoreInterrupts(e);
        return;
        //OS_TPanic("Illegal unlock mutex\n");
        //break;
    }

    //---- unlock mutex
    if ( unlocked )
    {
        OSi_DequeueItem(currentThread, mutex);
        mutex->thread = NULL;
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_NONE );
        OS_WakeupThread(&mutex->queue);
    }

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutexR

  Description:  unlock mutex locked as READ access

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_UnlockMutexR(OSMutex *mutex)
{
    OSi_UnlockMutexCore( mutex, OS_MUTEX_TYPE_R );
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutexW

  Description:  unlock mutex locked as WRITE access

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_UnlockMutexW(OSMutex *mutex)
{
    OSi_UnlockMutexCore( mutex, OS_MUTEX_TYPE_W );
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutexRW

  Description:  unlock mutex locked as READ/WRITE access

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_UnlockMutexRW(OSMutex *mutex)
{
    OSi_UnlockMutexCore( mutex, OS_MUTEX_TYPE_NONE );
}

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexFromRToW

  Description:  Promote mutexR lock to mutexW lock without unlock.
                Wait till success.

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexFromRToW(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    OSThread *currentThread = OS_GetCurrentThread();

    while(1)
    {
        if ( OS_TryLockMutexFromRToW(mutex) )
        {
            break;
        }

        currentThread->mutex = mutex;
        OS_SleepThread(&mutex->queue);
        currentThread->mutex = NULL;
    }

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexFromRToW

  Description:  Try to promote mutexR lock to mutexW lock without unlock.

  Arguments:    mutex       pointer to mutex structure

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexFromRToW(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    BOOL locked = FALSE;

    //---- change mutex type if no other thread is locked. and mutex is R
    if ( OS_GetMutexCount(mutex) == 1 && mutex->queue.head == NULL && OS_GetMutexType(mutex) == OS_MUTEX_TYPE_R )
    {
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_W );
        locked = TRUE;
    }

    (void)OS_RestoreInterrupts(e);
    return locked;
}

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexFromWToR

  Description:  Demote mutexW lock to mutexR lock without unlock.
                Wait till success.

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexFromWToR(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    OSThread *currentThread = OS_GetCurrentThread();

    while(1)
    {
        if ( OS_TryLockMutexFromWToR(mutex) )
        {
            break;
        }

        currentThread->mutex = mutex;
        OS_SleepThread(&mutex->queue);
        currentThread->mutex = NULL;
    }

    (void)OS_RestoreInterrupts(e);
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexFromWToR

  Description:  Try to demote mutexW lock to mutexR lock without unlock.

  Arguments:    mutex       pointer to mutex structure

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexFromWToR(OSMutex *mutex)
{
    OSIntrMode e = OS_DisableInterrupts();
    BOOL locked = FALSE;

    //---- change mutex type if no other thread is locked. and mutex is W
    if ( OS_GetMutexCount(mutex) == 1 && mutex->queue.head == NULL && OS_GetMutexType(mutex) == OS_MUTEX_TYPE_W )
    {
        OS_SetMutexType( mutex, OS_MUTEX_TYPE_R );
        locked = TRUE;
    }

    (void)OS_RestoreInterrupts(e);
    return locked;
}


//===========================================================================
//     MUTEX QUEUE
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_EnqueueTail

  Description:  internal function.
                add mutex to thread's mutex list

  Arguments:    thread      pointer to thread
                mutex       pointer to mutex to be add

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_EnqueueTail(OSThread *thread, OSMutex *mutex)
{
#ifndef SDK_THREAD_INFINITY
    OSMutex *prev = thread->mutexQueueTail;

    SDK_ASSERT(thread && mutex);

    if (!prev)
    {
        thread->mutexQueueHead = mutex;
    }
    else
    {
        prev->next = mutex;
    }

    mutex->prev = prev;
    mutex->next = NULL;
    thread->mutexQueueTail = mutex;
#else
    OSMutex *prev = thread->mutexQueue.tail;

    SDK_ASSERT(thread && mutex);

    if (!prev)
    {
        thread->mutexQueue.head = mutex;
    }
    else
    {
        prev->link.next = mutex;
    }

    mutex->link.prev = prev;
    mutex->link.next = NULL;
    thread->mutexQueue.tail = mutex;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OSi_DequeueItem

  Description:  internal function.
                remove specified mutex from thread's mutex list

  Arguments:    thread      pointer to thread
                mutex       pointer to mutex to be remove

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_DequeueItem(OSThread *thread, OSMutex *mutex)
{
#ifndef SDK_THREAD_INFINITY
    OSMutex *next = mutex->next;
    OSMutex *prev = mutex->prev;

    SDK_ASSERT(thread && mutex);

    if (!next)
    {
        thread->mutexQueueTail = prev;
    }
    else
    {
        next->prev = prev;
    }

    if (!prev)
    {
        thread->mutexQueueHead = next;
    }
    else
    {
        prev->next = next;
    }
#else
    OSMutex *next = mutex->link.next;
    OSMutex *prev = mutex->link.prev;

    SDK_ASSERT(thread && mutex);

    if (!next)
    {
        thread->mutexQueue.tail = prev;
    }
    else
    {
        next->link.prev = prev;
    }

    if (!prev)
    {
        thread->mutexQueue.head = next;
    }
    else
    {
        prev->link.next = next;
    }
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OSi_DequeueHead

  Description:  remove top mutex from thread's list, and return mutex

  Arguments:    thread      pointer to thread

  Returns:      mutex which listed at top of thread
 *---------------------------------------------------------------------------*/
OSMutex *OSi_DequeueHead(OSThread *thread)
{
#ifndef SDK_THREAD_INFINITY
    OSMutex *mutex = thread->mutexQueueHead;
    OSMutex *next = mutex->next;

    SDK_ASSERT(thread);

    if (!next)
    {
        thread->mutexQueueTail = NULL;
    }
    else
    {
        next->prev = NULL;
    }

    thread->mutexQueueHead = next;
#else
    OSMutex *mutex = thread->mutexQueue.head;
    OSMutex *next = mutex->link.next;

    SDK_ASSERT(thread);

    if (!next)
    {
        thread->mutexQueue.tail = NULL;
    }
    else
    {
        next->link.prev = NULL;
    }

    thread->mutexQueue.head = next;
#endif

    return mutex;
}
