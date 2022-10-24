/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     spinLock.h

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_OS_SPINLOCK_H_
#define NITRO_OS_SPINLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

#ifndef SDK_TWL
#ifdef	SDK_ARM9
#include	<nitro/hw/ARM9/mmap_global.h>
#else  //SDK_ARM7
#include	<nitro/hw/ARM7/mmap_global.h>
#endif
#else // SDK_TWL
#include    <twl/hw/common/mmap_shared.h>
#ifdef	SDK_ARM9
#include	<twl/hw/ARM9/mmap_global.h>
#else  //SDK_ARM7
#include	<twl/hw/ARM7/mmap_global.h>
#endif
#endif //SDK_TWL

//======================================================================
//                      Lock Functions
//
//- Use these functions for mutually exclusive control of dedicated resources, such as the inter-CPU transfer FIFO, and common resources between CPUs, such as internal work RAM and VRAM-C/D shared by Game Cards, Game Paks, and CPUs
//  
//  
//======================================================================

// Lock ID

#define OS_UNLOCK_ID            0      // ID when lock variable is not locked
#define OS_MAINP_LOCKED_FLAG    0x40   // Main processor lock verification flag
#define OS_MAINP_LOCK_ID_START  0x40   // Starting number of lock ID assignments, for main processor use
#define OS_MAINP_LOCK_ID_END    0x6f   //                               Assignment completion number
#define OS_MAINP_DBG_LOCK_ID    0x70   //                               Debugger reservation number
#define OS_MAINP_SYSTEM_LOCK_ID 0x7f   //                               System reservation number
#define OS_SUBP_LOCKED_FLAG     0x80   //   Lock verification flag by sub-processor
#define OS_SUBP_LOCK_ID_START   0x80   //   Starting number of lock ID assignments, for sub-processor use
#define OS_SUBP_LOCK_ID_END     0xaf   //                               Assignment completion number
#define OS_SUBP_DBG_LOCK_ID     0xb0   //                               Debugger reservation number
#define OS_SUBP_SYSTEM_LOCK_ID  0xbf   //                               System reservation number

#define OS_LOCK_SUCCESS         0      // Lock success
#define OS_LOCK_ERROR           (-1)   // Lock error

#define OS_UNLOCK_SUCCESS       0      // Unlock success
#define OS_UNLOCK_ERROR         (-2)   // Unlock error

#define OS_LOCK_FREE            0      // Unlocking

#define OS_LOCK_ID_ERROR        (-3)   // Lock ID error


//---- Structure of Lock Variable 
typedef volatile struct OSLockWord
{
    u32     lockFlag;
    u16     ownerID;
    u16     extension;
}
OSLockWord;

/*---------------------------------------------------------------------------*
  Name:         OS_InitLock

  Description:  Initializes access rights to system lock variables and shared resources.
                

                The region used for cartridge mutexes will be cleared.
                (because that region is used by the debugger)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_InitLock(void);

/*---------------------------------------------------------------------------*
  Name:         OS_LockByWord
                OS_LockCartridge
                OS_LockCard

  Description:  Spinlock functions.

                These use spinlocks on lock variables for mutually exclusive control of resources shared between processors and between modules.
                It will keep trying until the lock succeeds.
                Lock resources shared between processors before using them.
                If you can adjust the timing for resources owned by a single processor, you do not need to lock them.
                It's also possible to lock exclusive-use resources just for debugging.
                
                

  Arguments:    lockID: Lock ID
                lockp: Lock variable's pointer

  Returns:      >0: A locked value
                OS_LOCK_SUCCESS (=0): Successful lock
 *---------------------------------------------------------------------------*/
s32     OS_LockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_LockCartridge(u16 lockID);
s32     OS_LockCard(u16 lockID);

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockByWord
                OS_UnlockCartridge
                OS_UnlockCard

  Description:  Unlocks.
                This unlocks and passes the access rights of a shared resource to the subprocessor.
                If an unlocked module is run, it is not unlocked and OS_UNLOCK_ERROR is returned.
                 

  Arguments:    lockID: Lock ID
                lockp: Lock variable's pointer

  Returns:      OS_UNLOCK_SUCCESS: Unlock success.
                OS_UNLOCK_ERROR: Unlock error.
 *---------------------------------------------------------------------------*/
s32     OS_UnlockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_UnlockCartridge(u16 lockID);
s32     OS_UnlockCard(u16 lockID);

//---- The previous names have also been left for compatibility. ('UnLock' <-> 'Unlock')
//     The ISD library calls these functions. They are not inline.
s32     OS_UnLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_UnLockCartridge(u16 lockID);
s32     OS_UnLockCard(u16 lockID);

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockByWord
                OS_TryLockCartridge
                OS_TryLockCard

  Description:  Attempts to lock.

                Attempts only a single spinlock.
                Lock resources shared between processors before using them.
                If you can adjust the timing for resources owned by a single processor, you do not need to lock them.
                It's also possible to lock exclusive-use resources just for debugging.
                

  Arguments:    lockID: Lock ID
                lockp: Lock variable's pointer
                CtrlFuncp: Resource control functions pointer

  Returns:      >0: Locked (the previously stored ID).
                OS_LOCK_SUCCESS: Lock success
 *---------------------------------------------------------------------------*/
s32     OS_TryLockByWord(u16 lockID, OSLockWord *lockp, void (*crtlFuncp) (void));
s32     OS_TryLockCartridge(u16 lockID);
s32     OS_TryLockCard(u16 lockID);

/*---------------------------------------------------------------------------*
  Name:         OS_ReadOwnerOfLockWord
                OS_ReadOwnerOfLockCartridge
                OS_ReadOwnerOfLockCard

  Description:  Reads the lock variable's owner module ID.

                Reads a lock variable's owner module ID.
                You can check which processor has ownership now if the module ID is nonzero.
                For a shared resource, prohibiting interrupts can maintain only the main processor's ownership rights.
                In other states, it might be changed by subprocessors.
                The lock variable may not necessarily be unlocked even if the owner module ID is 0.
                
                

                Note: Be aware that byte access to main memory is not possible unless it is through a cache.
                
                Thus, as a basic rule, you should use the OS_ReadOwnerOfLockWord function for main memory.

  Arguments:    lockp: Lock variable's pointer

  Returns:      Owner module ID.
 *---------------------------------------------------------------------------*/
u16     OS_ReadOwnerOfLockWord(OSLockWord *lockp);
#define OS_ReadOwnerOfLockCartridge()  OS_ReadOwnerOfLockWord( (OSLockWord *)HW_CTRDG_LOCK_BUF )
#define OS_ReadOwnerOfLockCard()       OS_ReadOwnerOfLockWord( (OSLockWord *)HW_CARD_LOCK_BUF  )

/*---------------------------------------------------------------------------*
  Name:         OS_GetLockID

  Description:  Gets the lock ID.

  Arguments:    None.

  Returns:      OS_LOCK_ID_ERROR: Failed to get the ID
                (for the ARM9)
                0x40-0x6f: lock ID
                (for the ARM7)
                0x80-0xaf: lock ID

                Only up to 48 kinds of IDs can be assigned.
                When managing multiple lock variables in a module, use a single ID whenever possible.
                
 *---------------------------------------------------------------------------*/
s32     OS_GetLockID(void);


/*---------------------------------------------------------------------------*
  Name:         OS_ReleaseLockID

  Description:  Releases a lock ID.

  Arguments:    lockID: Lock ID to attempt to release.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    OS_ReleaseLockID(u16 lockID);


#ifdef SDK_TWL
//================================================================
//   Private Functions
//   The following are internal functions. Do not use them from an application.
#define OSi_SYNCTYPE_SENDER 0
#define OSi_SYNCTYPE_RECVER 1

#define OSi_SYNCVAL_NOT_READY 0
#define OSi_SYNCVAL_READY     1

void OSi_SyncWithOtherProc( int type, void* syncBuf );

static inline void OSi_SetSyncValue( u8 n )
{
	*(vu8*)(HW_INIT_LOCK_BUF+4) = n;
}
static inline u8 OSi_GetSyncValue( void )
{
	return *(vu8*)(HW_INIT_LOCK_BUF+4);
}

#endif


//--------------------------------------------------------------------------------
#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_SPINLOCK_H_ */
#endif
