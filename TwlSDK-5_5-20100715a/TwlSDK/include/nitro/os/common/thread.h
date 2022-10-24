/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     thread.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_THREAD_H_
#define NITRO_OS_THREAD_H_

#include <nitro/misc.h>
#include <nitro/os/common/context.h>
#include <nitro/os/common/callTrace.h>
#include <nitro/version.h>

#ifdef __cplusplus
extern "C" {
#endif

//---- build switch to let number of thread infinity, for version 3.0 or later
#if SDK_VERSION_MAJOR >= 3
#define SDK_THREAD_INFINITY           1
#endif

#ifdef SDK_FINALROM
// define if callTrace system not used
#define OS_NO_CALLTRACE
#endif


#ifndef SDK_THREAD_INFINITY
//---- maximum number of thread
#define OS_THREAD_MAX_NUM             16        // changed 8 to 16 (2004/5/26)
#endif

//---- priority of thread that calls OS_InitThread()
#define OS_THREAD_LAUNCHER_PRIORITY   16

//---- range of thread priority
#define OS_THREAD_PRIORITY_MIN        0
#define OS_THREAD_PRIORITY_MAX        31

//----------------------------------------------------------------------------
typedef struct _OSThread OSThread;

#ifdef SDK_THREAD_INFINITY
typedef struct _OSThreadQueue OSThreadQueue;
typedef struct _OSThreadLink OSThreadLink;
typedef struct _OSMutexQueue OSMutexQueue;
typedef struct _OSMutexLink OSMutexLink;
typedef struct OSMutex OSMutex;
#endif

typedef struct OSiAlarm OSAlarm;

#ifdef SDK_THREAD_INFINITY
struct _OSThreadQueue
{
    OSThread *head;
    OSThread *tail;
};

struct _OSThreadLink
{
    OSThread *prev;
    OSThread *next;
};

struct _OSMutexQueue
{
    OSMutex *head;
    OSMutex *tail;
};

struct _OSMutexLink
{
    OSMutex *next;
    OSMutex *prev;
};

#endif
//---------------- Thread status
typedef enum
{
    OS_THREAD_STATE_WAITING = 0,
    OS_THREAD_STATE_READY = 1,
    OS_THREAD_STATE_TERMINATED = 2
}
OSThreadState;

#ifndef SDK_THREAD_INFINITY
//---------------- thread queue
#if     ( OS_THREAD_MAX_NUM <= 16 )
typedef u16 OSThreadQueue;
#define OS_SIZEOF_OSTHREADQUEUE  16
#elif   ( OS_THREAD_MAX_NUM <= 32 )
typedef u32 OSThreadQueue;
#define OS_SIZEOF_OSTHREADQUEUE  32
#else
Error:no bit field any more
#endif
#endif
#define OS_THREAD_SPECIFIC_MAX  3
typedef void (*OSThreadDestructor) (void *);

//---------------- Thread structure
struct _OSThread
{
    OSContext context;
    OSThreadState state;
    OSThread *next;
    u32     id;
    u32     priority;

    void   *profiler;

#ifdef SDK_THREAD_INFINITY
    OSThreadQueue *queue;
    OSThreadLink link;
#endif

#ifndef SDK_THREAD_INFINITY
    void   *mutex;
    void   *mutexQueueHead;
    void   *mutexQueueTail;
#else
    OSMutex *mutex;                    // OSMutex
    OSMutexQueue mutexQueue;           // OSMutexQueue
#endif

    u32     stackTop;                  // for stack overflow
    u32     stackBottom;               // for stack underflow
    u32     stackWarningOffset;

#ifndef SDK_THREAD_INFINITY
    OSThreadQueue joinQueue;           // for wakeup threads in thread termination
#if OS_SIZEOF_OSTHREADQUEUE == 16
    u16     padding;
#endif
#else
    OSThreadQueue joinQueue;

    void   *specific[OS_THREAD_SPECIFIC_MAX];   // for internal use
    OSAlarm *alarmForSleep;            // OSAlarm for sleeping
    OSThreadDestructor destructor;     // thread destructor
    void   *userParameter;             // for user

    int     systemErrno;
#endif
};

//---------------- Thread & context packed structure
typedef struct OSThreadInfo
{
    u16     isNeedRescheduling;
#ifndef SDK_THREAD_INFINITY
    u16     max_entry;
    u16     irqDepth;
    u16     padding;
#else
    u16     irqDepth;
#endif
    OSThread *current;
    OSThread *list;
    void   *switchCallback;            // type: OSSwitchThreadCallback
#ifndef SDK_THREAD_INFINITY
    OSThread *entry[OS_THREAD_MAX_NUM];
#endif
}
OSThreadInfo;

// offset
#ifndef SDK_THREAD_INFINITY
#define OS_THREADINFO_OFFSET_ISNEEDRESCHEDULING     0
#define OS_THREADINFO_OFFSET_MAX_ENTRY              2
#define OS_THREADINFO_OFFSET_IRQDEPTH               4
#define OS_THREADINFO_OFFSET_PADDING                6
#define OS_THREADINFO_OFFSET_CURRENT                8
#define OS_THREADINFO_OFFSET_LIST                   12
#define OS_THREADINFO_OFFSET_SWITCHCALLBACK         16
#define OS_THREADINFO_OFFSET_ENTRY                  20
#else  // ifndef SDK_THREAD_INFINITY
#define OS_THREADINFO_OFFSET_ISNEEDRESCHEDULING     0
#define OS_THREADINFO_OFFSET_IRQDEPTH               2
#define OS_THREADINFO_OFFSET_CURRENT                4
#define OS_THREADINFO_OFFSET_LIST                   8
#define OS_THREADINFO_OFFSET_SWITCHCALLBACK         12
#define OS_THREADINFO_OFFSET_ENTRY                  16
#endif

//----------------  CONTEXT OFFSET
#define OS_THREAD_OFFSET_CONTEXT                    0
#define OS_THREAD_OFFSET_STATE          (sizeof(OSContext))
#define OS_THREAD_OFFSET_NEXT           (sizeof(OSContext)+sizeof(OSThreadState))
#define OS_THREAD_OFFSET_ID             (OS_THREAD_OFFSET_NEXT+sizeof(OSThread*))

//---------------- thread stack overflow status
typedef enum
{
    OS_STACK_NO_ERROR = 0,
    OS_STACK_OVERFLOW = 1,
    OS_STACK_ABOUT_TO_OVERFLOW = 2,
    OS_STACK_UNDERFLOW = 3
}
OSStackStatus;

//---------------- thread switch callback
typedef void (*OSSwitchThreadCallback) (OSThread *from, OSThread *to);

//---------------- thread resource
typedef struct OSThreadResource
{
	int num;
}
OSThreadResource;

//----------------------------------------------------------------------------
//---- private function ( don't use these OSi_* function )
void    OSi_CheckStack(const char *file, int line, const OSThread *thread);
u32     OSi_GetSystemStackPointer(void);
u32     OSi_GetCurrentStackPointer(void);
OSThread *OSi_GetIdleThread(void);

/*---------------------------------------------------------------------------*
  Name:         OS_InitThread

  Description:  Initialize Thread System

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_InitThread(void);

/*---------------------------------------------------------------------------*
  Name:         OS_IsThreadAvailable

  Description:  check if thread system is available

  Arguments:    None

  Returns:      TRUE if available, FALSE if not
 *---------------------------------------------------------------------------*/
BOOL    OS_IsThreadAvailable(void);

/*---------------------------------------------------------------------------*
  Name:         OS_CreateThread

  Description:  Create a new Thread

  Arguments:    thread      pointer of thread structure
                func        function to start thread
                arg         argument for func
                stack       stack bottom address
                stackSize   stack size (byte. must be aligned by 4)
                prio        thread priority

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_CreateThread(OSThread *thread,
                        void (*func) (void *), void *arg, void *stack, u32 stackSize, u32 prio);


/*---------------------------------------------------------------------------*
  Name:         OS_ExitThread

  Description:  Exit thread

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_ExitThread(void);


/*---------------------------------------------------------------------------*
  Name:         OS_DestroyThread

  Description:  destroy specified thread.

  Arguments:    thread: thread to be destroyed

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_DestroyThread(OSThread *thread);


/*---------------------------------------------------------------------------*
  Name:         OS_KillThread

  Description:  switch PC to thread destructor to finalize thread

  Arguments:    thread : thread to wait to finish
                flag   : argument for destructor

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_THREAD_INFINITY
void    OS_KillThread(OSThread *thread, void *arg);
void    OS_KillThreadWithPriority(OSThread *thread, void *arg, u32 prio);
#endif


/*---------------------------------------------------------------------------*
  Name:         OS_JoinThread

  Description:  wait for specified thread to terminated

  Arguments:    thread : thraead to wait to finish

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_JoinThread(OSThread *thread);


/*---------------------------------------------------------------------------*
  Name:         OS_IsThreadTeminated

  Description:  check thread status whether it's terminated

  Arguments:    thread : pointer to thread to be examined

  Returns:      TRUE if the thread is terminated. FALSE if not
 *---------------------------------------------------------------------------*/
BOOL    OS_IsThreadTerminated(const OSThread *thread);

/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadStatus

  Description:  get thread status

  Arguments:    thread : pointer to thread

  Returns:      
 *---------------------------------------------------------------------------*/
OSThreadState  OS_GetThreadStatus(const OSThread *thread);

/*---------------------------------------------------------------------------*
  Name:         OS_SelectThread

  Description:  Select thread to execute

  Arguments:    None

  Returns:      thread to execute
 *---------------------------------------------------------------------------*/
OSThread *OS_SelectThread(void);


/*---------------------------------------------------------------------------*
  Name:         OS_RescheduleThread

  Description:  do rescheduling threads

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_RescheduleThread(void);

/*---------------------------------------------------------------------------*
  Name:         OS_YieldThread

  Description:  do thread rescheduling. current thread relinquish CPU
                to give chance of running to other threads which has same
                priority.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OS_YieldThread(void);


/*---------------------------------------------------------------------------*
  Name:         OS_SleepThread

  Description:  sleep current thread

  Arguments:    thread         Thread queue

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SleepThread(OSThreadQueue *queue);


/*---------------------------------------------------------------------------*
  Name:         OS_SleepThreadDirect

  Description:  Gets the thread into sleep status directly

  Arguments:    thread            thread to sleep
                queue             waiting list queue (or NULL)

  Returns:      none
 *---------------------------------------------------------------------------*/
void    OS_SleepThreadDirect(OSThread *thread, OSThreadQueue *queue);

/*---------------------------------------------------------------------------*
  Name:         OS_WakeupThread

  Description:  wake up threads by queue

  Arguments:    queue         Thread queue

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_WakeupThread(OSThreadQueue *queue);


/*---------------------------------------------------------------------------*
  Name:         OS_WakeupThreadDirect

  Description:  wake up thread by specifying thread directly

  Arguments:    thread        Thread to wake up

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_WakeupThreadDirect(OSThread *thread);


/*---------------------------------------------------------------------------*
  Name:         OS_DumpThreadList

  Description:  Dump All Thread Infomation (for DEBUG)

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_DumpThreadList(void);


/*---------------------------------------------------------------------------*
  Name:         OS_GetNumberOfThread

  Description:  Get number of thread which exists in system

  Arguments:    None

  Returns:      number of thread which exists in system
 *---------------------------------------------------------------------------*/
int     OS_GetNumberOfThread(void);

/*==== static inlie functions ====*/
/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadInfo

  Description:  Get pointer of system thread info structure.

  Arguments:    None

  Returns:      pointer of thread info structure
 *---------------------------------------------------------------------------*/
extern OSThreadInfo OSi_ThreadInfo;

static inline OSThreadInfo *OS_GetThreadInfo(void)
{
    return &OSi_ThreadInfo;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetMaxThreadId

  Description:  Gets Max id number of available thread number

  Arguments:    None

  Returns:      Max id of available thread number
 *---------------------------------------------------------------------------*/
static inline u32 OS_GetMaxThreadId(void)
{
#ifndef SDK_THREAD_INFINITY
    return OS_GetThreadInfo()->max_entry;
#else
    return 0x7fffffff;                 // (=maximin number of int)
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetThread

  Description:  Gets pointer to thread which id is specified

  Arguments:    id : thread id to get thread

  Returns:      pointer to thread which id is specified
 *---------------------------------------------------------------------------*/
#ifndef SDK_THREAD_INFINITY
static inline OSThread *OS_GetThread(u32 id)
{
    SDK_TASSERTMSG(id < OS_THREAD_MAX_NUM, "Thread id illegal\n");
    return OS_GetThreadInfo()->entry[id];
}
#else
extern OSThread *OS_GetThread(u32 id);
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadId

  Description:  Gets id of specified thread

  Arguments:    thread      pointer to thread

  Returns:      id of specified thread
 *---------------------------------------------------------------------------*/
static inline u32 OS_GetThreadId(const OSThread *thread)
{
    SDK_TASSERTMSG(thread, "null thread pointer.");
    return thread->id;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadState

  Description:  Gets state of specified thread

  Arguments:    thread      pointer to thread

  Returns:      state of specified thead
 *---------------------------------------------------------------------------*/
static inline OSThreadState OS_GetThreadState(const OSThread *thread)
{
    SDK_TASSERTMSG(thread, "null thread pointer.");
    return thread->state;
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadContext

  Description:  Gets pointer to context of specified thread

  Arguments:    thread      pointer to thread

  Returns:      pointer to context of specified thread
 *---------------------------------------------------------------------------*/
static inline OSContext *OS_GetThreadContext(const OSThread *thread)
{
    SDK_TASSERTMSG(thread, "null thread pointer.");
    return (OSContext *)&thread->context;
}

/*---------------------------------------------------------------------------*
  Name:         OS_IsThreadRunnable

  Description:  Check if thread is runnable

  Arguments:    thread   pointer to thread

  Returns:      non zero if thread is runnable
 *---------------------------------------------------------------------------*/
static inline BOOL OS_IsThreadRunnable(const OSThread *thread)
{
    return thread->state == OS_THREAD_STATE_READY;
}

/*---------------------------------------------------------------------------*
  Name:         OS_InitThreadQueue

  Description:  Initialize thread queue

  Arguments:    queue   pointer to thread queue

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OS_InitThreadQueue(OSThreadQueue *queue)
{
#ifndef SDK_THREAD_INFINITY
    *queue = 0;
#else
    queue->head = queue->tail = NULL;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         OS_GetCurrentThread

  Description:  Gets pointer to the current thread

  Arguments:    None

  Returns:      Pointer to the current thread
 *---------------------------------------------------------------------------*/
static inline OSThread *OS_GetCurrentThread(void)
{
    return OS_GetThreadInfo()->current;
}

/*---------------------------------------------------------------------------*
  Name:         OS_SetCurrentThread

  Description:  Saves pointer to the current thread

  Arguments:    thread : thread to be current thread

  Returns:      Pointer to the current thread
 *---------------------------------------------------------------------------*/
static inline void OS_SetCurrentThread(OSThread *thread)
{
    OS_GetThreadInfo()->current = thread;
}

/*==== stack check ====*/
/*---------------------------------------------------------------------------*
  Name:         OS_SetThreadStackWarningOffset

  Description:  Set warning level for stack checker

  Arguments:    thread     thread to set
                offset     offset from stack top. must be multiple of 4

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetThreadStackWarningOffset(OSThread *thread, u32 offset);


/*---------------------------------------------------------------------------*
  Name:         OS_GetStackStatus

  Description:  check thread stack. check each CheckNUM.
                return result.

  Arguments:    thread   thread checked

  Returns:      0                            no error
                OS_STACK_OVERFLOW            overflow
                OS_STACK_ABOUT_TO_OVERFLOW   about to overflow
                OS_STACK_UNDERFLOW           underflow
 *---------------------------------------------------------------------------*/
OSStackStatus OS_GetStackStatus(const OSThread *thread);

/*---------------------------------------------------------------------------*
  Name:         OS_CheckStack

  Description:  check thread stack. check each CheckNum.
                if changed, display warning and halt.

  Arguments:    thread   thread to check stack

  Returns:      None.
                ( if error occurred, never return )
 *---------------------------------------------------------------------------*/
#if !defined(SDK_FINALROM) && !defined(SDK_NO_MESSAGE)
#define  OS_CheckStack( thread )  OSi_CheckStack( __FILE__, __LINE__, (const OSThread*)thread );
#else
#define  OS_CheckStack( thread )  ((void)0)
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_SetThreadPriority

  Description:  change priority of thread

  Arguments:    thread     thread to set priority
                prio       new priority to be set

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL    OS_SetThreadPriority(OSThread *thread, u32 prio);


/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadPriority

  Description:  get priority of thread

  Arguments:    thread     thread to get priority

  Returns:      priority
 *---------------------------------------------------------------------------*/
u32     OS_GetThreadPriority(const OSThread *thread);


/*---------------------------------------------------------------------------*
  Name:         OS_Sleep

  Description:  sleep specified period

  Arguments:    msec       sleeping period. ( milliseconds )

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_Sleep(u32 msec);


/*---------------------------------------------------------------------------*
  Name:         OS_SetSwitchThreadCallback

  Description:  set callback called at switching thread

  Arguments:    callback      callback function

  Returns:      previous callback function before set callback now
 *---------------------------------------------------------------------------*/
OSSwitchThreadCallback OS_SetSwitchThreadCallback(OSSwitchThreadCallback callback);


// notice: substans is in os_callTrace.c.
//         define here because of OSThread declaration.
/*---------------------------------------------------------------------------*
  Name:         OS_DumpThreadCallTrace

  Description:  dump callStack of thread

  Arguments:    thread :    thread

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_DumpThreadCallTrace(const OSThread *thread);



/*---------------------------------------------------------------------------*
  Name:         OS_DisableScheduler

  Description:  disable scheduler

  Arguments:    None

  Returns:      Previous scheduler suspend count.
                Suspended if value >= 0.
 *---------------------------------------------------------------------------*/
u32     OS_DisableScheduler(void);

/*---------------------------------------------------------------------------*
  Name:         OS_EnableScheduler

  Description:  enable scheduler

  Arguments:    None

  Returns:      Previous scheduler suspend count.
                Suspended if value >= 0.
 *---------------------------------------------------------------------------*/
u32     OS_EnableScheduler(void);


#ifdef SDK_THREAD_INFINITY
/*---------------------------------------------------------------------------*
  Name:         OS_SetThreadDestructor

  Description:  set thread destructor, which is called when that thread exits.

  Arguments:    thread : thread pointer
                dtor   : destructor function

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetThreadDestructor(OSThread *thread, OSThreadDestructor dtor);

/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadDestructor

  Description:  get thread destructor which is set

  Arguments:    thread : thread pointer

  Returns:      destructor function
 *---------------------------------------------------------------------------*/
OSThreadDestructor OS_GetThreadDestructor(const OSThread *thread);

/*---------------------------------------------------------------------------*
  Name:         OS_SetThreadParameter

  Description:  set user parameter which is allowed to use freely.

  Arguments:    thread    : thread pointer
                parameter : user parameter

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetThreadParameter(OSThread *thread, void *parameter);

/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadParameter

  Description:  get user parameter which is set

  Arguments:    thread : thread pointer

  Returns:      user parameter which is set
 *---------------------------------------------------------------------------*/
void   *OS_GetThreadParameter(const OSThread *thread);

/*---------------------------------------------------------------------------*
  Name:         OS_GetErrno

  Description:  get system error number.

  Arguments:    None.

  Returns:      error number
 *---------------------------------------------------------------------------*/
int     OS_GetErrno(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_IsThreadInList

  Description:  check if the specified thread is in the thread list

  Arguments:    thread : thread

  Returns:      TRUE if thread is in the thread list
 *---------------------------------------------------------------------------*/
BOOL    OS_IsThreadInList(const OSThread *thread);

/*---------------------------------------------------------------------------*
  Name:         OS_SetThreadDestructorStack

  Description:  specify stack area to call thread destructor

  Arguments:    stack       stack bottom address

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OS_SetThreadDestructorStack(void *stack);


#ifdef SDK_THREAD_INFINITY
//================================================================================
//              The following functions are for internal use. Not for user.
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_RemoveMutexLinkFromQueue

  Description:  remove mutex from mutex queue

  Arguments:    queue  : mutex queue

  Returns:      mutex pointer which is removed
 *---------------------------------------------------------------------------*/
extern OSMutex *OSi_RemoveMutexLinkFromQueue(OSMutexQueue * queue);

/*---------------------------------------------------------------------------*
  Name:         OSi_SetSystemErrno

  Description:  set system error number.

  Arguments:    thread : thread to set error number
                errno  : error number to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void    OSi_SetSystemErrno(OSThread *thread, int errno);

/*---------------------------------------------------------------------------*
  Name:         OSi_GetSystemErrno

  Description:  get system error number.

  Arguments:    thread : thread to set error number

  Returns:      error number
 *---------------------------------------------------------------------------*/
int     OSi_GetSystemErrno(const OSThread *thread);


#define OSi_SPECIFIC_CPS   0

/*---------------------------------------------------------------------------*
  Name:         OSi_SetSpecificData

  Description:  set system specific data

  Arguments:    thread : thread to set data
                index  : index of specific array
                data   : data to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void OSi_SetSpecificData(OSThread *thread, int index, void *data)
{
    SDK_ASSERT(thread && 0 <= index && index < OS_THREAD_SPECIFIC_MAX);
    thread->specific[index] = data;
}

/*---------------------------------------------------------------------------*
  Name:         OSi_GetSpecificData

  Description:  get system specific data

  Arguments:    thread : thread to get data
                index  : index of specific array

  Returns:      error number
 *---------------------------------------------------------------------------*/
static inline void *OSi_GetSpecificData(const OSThread *thread, int index)
{
    SDK_ASSERT(thread && 0 <= index && index < OS_THREAD_SPECIFIC_MAX);
    return thread->specific[index];
}
#endif


//================================================================================
//              The following functions are for operations of thread struct.
//              use carefully.
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadList

  Description:  get first thread of thread list.

  Arguments:    None

  Returns:      first thread of thread list
 *---------------------------------------------------------------------------*/
static inline OSThread *OS_GetThreadList(void)
{
    return OS_GetThreadInfo()->list;
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetNextThread

  Description:  get thread which is linked next in thread list

  Arguments:    thread : thread to get next thread

  Returns:      next thread. NULL means no next thread ( specified thread may be last )
 *---------------------------------------------------------------------------*/
static inline OSThread *OS_GetNextThread(const OSThread *thread)
{
    SDK_ASSERT(thread);
    return thread->next;
}

//================================================================================
//    for DEBUG
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetThreadResource

  Description:  store resources of thread to specified pointer

  Arguments:    resource       pointer to store thread resources

  Returns:      TRUE  ... success (always return this now)
                FALSE ... fail
 *---------------------------------------------------------------------------*/
BOOL OS_GetThreadResource(OSThreadResource *resource);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_THREAD_H_ */
#endif
