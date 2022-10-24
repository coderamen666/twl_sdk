/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     mutex.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-11#$
  $Rev: 9287 $
  $Author: yada $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_MUTEX_H_
#define NITRO_OS_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/os/common/thread.h>

//----------------------------------------------------------------
//---- mutex type
#define OSi_MUTEX_TYPE_SHIFT  24
#define OSi_MUTEX_TYPE_MASK   (0xff << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_NONE    (0x00 << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_STD     (0x10 << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_R       (0x20 << OSi_MUTEX_TYPE_SHIFT)
#define OS_MUTEX_TYPE_W       (0x30 << OSi_MUTEX_TYPE_SHIFT)

#define OSi_MUTEX_COUNT_MASK  0xffffff

#ifndef SDK_THREAD_INFINITY
typedef struct OSMutex OSMutex;
#endif

#pragma  warn_padding off
struct OSMutex
{
    OSThreadQueue queue;
    OSThread *thread;                  // current owner
    s32     count;                     // lock count (notice: use upper 1byte as mutex type)

#ifndef SDK_THREAD_INFINITY
    OSMutex *prev;                     // link for OSThread.queueMutex
    OSMutex *next;                     // link for OSThread.queueMutex
#else
    OSMutexLink link;
#endif
};
#pragma  warn_padding reset


static inline  void OS_SetMutexCount( OSMutex* mutex, s32 count )
{
    mutex->count = (s32)( (mutex->count & OSi_MUTEX_TYPE_MASK) | (count & OSi_MUTEX_COUNT_MASK) );
}
static inline  s32 OS_GetMutexCount( OSMutex* mutex )
{
    return (s32)( mutex->count & OSi_MUTEX_COUNT_MASK);
}
static inline  void OS_IncreaseMutexCount( OSMutex* mutex )
{
    u32 type = (u32)(mutex->count & OSi_MUTEX_TYPE_MASK);
    mutex->count ++;
    mutex->count = (s32)( type | (mutex->count & OSi_MUTEX_COUNT_MASK) );
}
static inline  void OS_DecreaseMutexCount( OSMutex* mutex )
{
    u32 type = (u32)(mutex->count & OSi_MUTEX_TYPE_MASK);
    mutex->count --;
    mutex->count = (s32)( type | (mutex->count & OSi_MUTEX_COUNT_MASK) );
}
static inline  void OS_SetMutexType( OSMutex* mutex, u32 type )
{
    mutex->count = (s32)( type | (mutex->count & OSi_MUTEX_COUNT_MASK) );
}
static inline  u32 OS_GetMutexType( OSMutex* mutex )
{
    return (u32)( mutex->count & OSi_MUTEX_TYPE_MASK );
}


/*---------------------------------------------------------------------------*
  Name:         OS_InitMutex

  Description:  initialize mutex

  Arguments:    mutex       pointer to mutex structure
                            to be initialized

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_InitMutex(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutex

  Description:  lock mutex

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_LockMutex(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutex

  Description:  unlock mutex

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_UnlockMutex(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutex

  Description:  try to lock mutex

  Arguments:    mutex       pointer to mutex structure

  Returns:      True if lock
 *---------------------------------------------------------------------------*/
BOOL    OS_TryLockMutex(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OSi_UnlockAllMutex

  Description:  unlocks all the mutexes locked by the thread

  Arguments:    mutex       pointer to mutex structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OSi_UnlockAllMutex(OSThread *thread);


/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexR

  Description:  lock RW mutex as READ access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexR(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexW

  Description:  lock RW mutex as WRITE access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexW(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexR

  Description:  try to lock RW mutex as READ access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      TRUE if locked
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexR(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexW

  Description:  try to lock RW mutex as WRITE access

  Arguments:    mutex       pointer to RW mutex structure

  Returns:      TRUE if locked
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexW(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutexR

  Description:  unlock mutex locked as READ access

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_UnlockMutexR(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutexW

  Description:  unlock mutex locked as WRITE access

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_UnlockMutexW(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockMutexRW

  Description:  unlock mutex locked as READ/WRITE access

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_UnlockMutexRW(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexFromRToW

  Description:  Promote mutexR lock to mutexW lock without unlock.
                Wait till success.

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexFromRToW(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexFromRToW

  Description:  Try to promote mutexR lock to mutexW lock without unlock.

  Arguments:    mutex       pointer to mutex structure

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexFromRToW(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_LockMutexFromWToR

  Description:  Demote mutexW lock to mutexR lock without unlock.
                Wait till success.

  Arguments:    mutex       pointer to mutex structure

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_LockMutexFromWToR(OSMutex *mutex);

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockMutexFromWToR

  Description:  Try to demote mutexW lock to mutexR lock without unlock.

  Arguments:    mutex       pointer to mutex structure

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL OS_TryLockMutexFromWToR(OSMutex *mutex);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_MUTEX_H_ */
#endif
