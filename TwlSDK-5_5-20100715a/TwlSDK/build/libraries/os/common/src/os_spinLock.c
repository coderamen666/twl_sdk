/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS -
  File:     os_spinLock.c

  Copyright 2003-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#include <nitro.h>


void    _ISDbgLib_Initialize(void);
void    _ISDbgLib_AllocateEmualtor(void);
void    _ISDbgLib_FreeEmulator(void);
void    _ISTDbgLib_Initialize(void);
void    _ISTDbgLib_AllocateEmualtor(void);
void    _ISTDbgLib_FreeEmulator(void);

s32     OS_LockByWord_IrqAndFiq(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_UnlockByWord_IrqAndFiq(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void));
s32     OS_TryLockByWord_IrqAndFiq(u16 lockID, OSLockWord *lockp, void (*crtlFuncp) (void));

static s32 OSi_DoLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void),
                            BOOL disableFiq);
static s32 OSi_DoUnlockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void),
                              BOOL disableFIQ);
static s32 OSi_DoTryLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void),
                               BOOL disableFIQ);

static void OSi_AllocateCartridgeBus(void);
static void OSi_FreeCartridgeBus(void);

static void OSi_AllocateCardBus(void);
static void OSi_FreeCardBus(void);

static void OSi_WaitByLoop(void);


#ifdef  SDK_ARM9
#define OSi_ASSERT_ID( id )       SDK_TASSERTMSG( id >= OS_MAINP_LOCK_ID_START && id <= OS_MAINP_SYSTEM_LOCK_ID, \
                             "lock ID %d is out of bounds", id )
#else
#define OSi_ASSERT_ID( id )       SDK_TASSERTMSG( id >= OS_SUBP_LOCK_ID_START && id <= OS_SUBP_SYSTEM_LOCK_ID, \
                             "lock ID %d is out of bounds", id )
#endif


#define OSi_LOCKID_INITIAL_FLAG_0     0xffffffff
#define OSi_LOCKID_INITIAL_FLAG_1     0xffff0000


#ifdef SDK_ARM9
#define OSi_ANYP_LOCK_ID_FLAG  HW_LOCK_ID_FLAG_MAIN
#define OSi_ANYP_LOCK_ID_START OS_MAINP_LOCK_ID_START
#else
#define OSi_ANYP_LOCK_ID_FLAG  HW_LOCK_ID_FLAG_SUB
#define OSi_ANYP_LOCK_ID_START OS_SUBP_LOCK_ID_START
#endif

//======================================================================
//                      SYNC
//======================================================================
#ifdef SDK_TWL
/*---------------------------------------------------------------------------*
  Name:         OSi_SyncWithOtherProc

  Description:  Syncs with other processor.

  Arguments:    type: OSi_SYNCTYPE_SENDER/RECVER
                syncBuf: Work area. Use 4 bytes

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OSi_SyncWithOtherProc( int type, void* syncBuf )
{
    vu8* ptr1     = (vu8*)syncBuf;
    vu8* ptr2     = (vu8*)syncBuf +1;
    vu8* pfinish  = (vu8*)syncBuf +2;
    vu8* pconf    = (vu8*)syncBuf +3;

    if ( type == OSi_SYNCTYPE_SENDER )
    {
        int n=0;
        *pfinish = FALSE;
        do
        {
            *ptr1 = (u8)( 0x80 | (n&0xf) );
            while ( *ptr1 != *ptr2 && *pfinish == FALSE )
            {
                OSi_WaitByLoop();
            }
            n ++;
        } while( *pfinish == FALSE );
        *pconf = TRUE;
    }
    else
    {
        int sum = 0;
        *ptr2 = 0;
        while( sum < 0x300 )
        {
            if ( *ptr2 != *ptr1 )
            {
                *ptr2 = *ptr1;
                sum += *ptr2;
            }
            else
            {
                OSi_WaitByLoop();
            }
        }
        *pconf   = FALSE;
        *pfinish = TRUE;
        while( *pconf == FALSE )
        {
            OSi_WaitByLoop();
        }
    }
}
#endif

//======================================================================
//                      DUMMY LOOP
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_WaitByLoop

  Description:  Waits by a for() loop.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void OSi_WaitByLoop(void)
{
    SVC_WaitByLoop(0x1000 / 4);
}

//======================================================================
//                      INITIALIZE
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_InitLock

  Description:  Initializes access rights to system lock variables and shared resources.
                

                The region used for cartridge mutexes will be cleared.
                (because that region is used by the debugger)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_InitLock(void)
{
    static BOOL isInitialized = FALSE;
#ifdef SDK_NITRO
    OSLockWord *lockp;
#endif

    if (isInitialized)
    {
        return;                        // Do it only once
    }
    isInitialized = TRUE;

//---------------- NITRO
#ifdef SDK_NITRO
    lockp = (OSLockWord *)HW_INIT_LOCK_BUF;

#ifdef  SDK_ARM9
    {
        //
        // Code for MAIN PROCESSOR
        //

        lockp->lockFlag = 0;
        (void)OS_LockByWord(OS_MAINP_SYSTEM_LOCK_ID - 1, lockp, NULL);

        // Checks by the subprocessor for unused shared resources
        while (lockp->extension != 0)
        {
            OSi_WaitByLoop();
        }

        // Flag initialization for the lock ID counter
        ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[0] = OSi_LOCKID_INITIAL_FLAG_0;
        ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[1] = OSi_LOCKID_INITIAL_FLAG_1;

        // Clear the lock buffer (except for the cartridge region)
        MI_CpuClear32((void *)HW_SHARED_LOCK_BUF, HW_CTRDG_LOCK_BUF - HW_SHARED_LOCK_BUF);

        // NITRO Card access rights -> subprocessor
        MIi_SetCardProcessor(MI_PROCESSOR_ARM7);

        // Cartridge access rights -> subprocessor
        MIi_SetCartridgeProcessor(MI_PROCESSOR_ARM7);

#ifndef SDK_FINALROM
		{
			u32 type = OS_GetConsoleType();
			if ( type & OS_CONSOLE_TWLDEBUGGER )
			{
                _ISTDbgLib_Initialize();
            }
            else if ( type & (OS_CONSOLE_ISDEBUGGER | OS_CONSOLE_NITRO) )
            {
                _ISDbgLib_Initialize();
            }
        }
#endif

        (void)OS_UnlockByWord(OS_MAINP_SYSTEM_LOCK_ID - 1, lockp, NULL);
        (void)OS_LockByWord(OS_MAINP_SYSTEM_LOCK_ID, lockp, NULL);
    }

#else  // SDK_ARM7
    {
        //
        // Code for SUBPROCESSOR
        //

        lockp->extension = 0;
        while (lockp->ownerID != OS_MAINP_SYSTEM_LOCK_ID)
        {
            OSi_WaitByLoop();
        }

#ifndef SDK_FINALROM
        // [TODO]
        // If a component leaks, it is necessary to create a component that calls the _ISTDbgLib_Initialize function
        //  
		{
			u32 type = OSi_DetectDebugger();
			if ( type & OS_CONSOLE_TWLDEBUGGER )
			{
				_ISTDbgLib_Initialize();
			}
			else if ( type & OS_CONSOLE_ISDEBUGGER )
			{
				_ISDbgLib_Initialize();
			}
        }
#endif

        // Flag initialization for the lock ID counter
        ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[0] = OSi_LOCKID_INITIAL_FLAG_0;
        ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[1] = OSi_LOCKID_INITIAL_FLAG_1;

        // Synchronize the end of initialization with the main processor
        lockp->extension = OS_SUBP_SYSTEM_LOCK_ID;
    }
#endif

//---------------- TWL
#else
#ifdef SDK_ARM9
    // Flag initialization for the lock ID counter
    ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[0] = OSi_LOCKID_INITIAL_FLAG_0;
    ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[1] = OSi_LOCKID_INITIAL_FLAG_1;

    // Clear the lock buffer (except for the cartridge region)
    MI_CpuClear32((void *)HW_SHARED_LOCK_BUF, HW_CTRDG_LOCK_BUF - HW_SHARED_LOCK_BUF);

    // NITRO Card access rights -> subprocessor
    MIi_SetCardProcessor(MI_PROCESSOR_ARM7);

    // Cartridge access rights -> subprocessor
    MIi_SetCartridgeProcessor(MI_PROCESSOR_ARM7);

    //---- Synchronize with ARM7
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_SENDER, (void*)HW_INIT_LOCK_BUF );
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_RECVER, (void*)HW_INIT_LOCK_BUF );

#ifndef SDK_FINALROM
	{
		u32 type = OSi_DetectDebugger();
		if ( type & OS_CONSOLE_TWLDEBUGGER )
		{
			_ISTDbgLib_Initialize();
		}
		else if ( type & OS_CONSOLE_ISDEBUGGER )
		{
			_ISDbgLib_Initialize();
		}
	}
#endif

    //lockp->lockFlag = 0;
    //(void)OS_LockByWord(OS_MAINP_SYSTEM_LOCK_ID, lockp, NULL);

#else  // SDK_ARM7
    // Flag initialization for the lock ID counter
    ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[0] = OSi_LOCKID_INITIAL_FLAG_0;
    ((u32 *)OSi_ANYP_LOCK_ID_FLAG)[1] = OSi_LOCKID_INITIAL_FLAG_1;

    //---- Synchronize with ARM9
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_RECVER, (void*)HW_INIT_LOCK_BUF );
    OSi_SyncWithOtherProc( OSi_SYNCTYPE_SENDER, (void*)HW_INIT_LOCK_BUF );

#ifndef SDK_FINALROM
	{
		u32 type = OSi_DetectDebugger();
		if ( type & OS_CONSOLE_TWLDEBUGGER )
		{
			_ISTDbgLib_Initialize();
		}
		else if ( type & OS_CONSOLE_ISDEBUGGER )
		{
			_ISDbgLib_Initialize();
		}
	}
#endif

#endif // SDK_ARM7

#endif // SDK_TWL
}

//======================================================================
//                      LOCK
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_DoLockByWord

  Description:  Does spinlock. Keeps trying until success.

  Arguments:    lockID: Lock ID
                lockp: Pointer to lock variable
                ctrlFuncp: Function
                disableFiq: Whether to disable fiq

  Returns:      OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
static s32 OSi_DoLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void),
                            BOOL disableFiq)
{
    s32     lastLockFlag;

    while ((lastLockFlag =
            OSi_DoTryLockByWord(lockID, lockp, ctrlFuncp, disableFiq)) > OS_LOCK_SUCCESS)
    {
        OSi_WaitByLoop();
    }

    return lastLockFlag;
}

/*---------------------------------------------------------------------------*
  Name:         OS_LockByWord

  Description:  Does spinlock. Keeps trying until success.

  Arguments:    lockID: Lock ID
                lockp: Pointer to lock variable
                ctrlFuncp: Function

  Returns:      OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_LockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void))
{
    return OSi_DoLockByWord(lockID, lockp, ctrlFuncp, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         OS_LockByWord_IrqAndFiq

  Description:  Does spinlock. Keeps trying until success.
                Disable irq and fiq.

  Arguments:    lockID: Lock ID
                lockp: Pointer to lock variable
                ctrlFuncp: Function

  Returns:      OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_LockByWord_IrqAndFiq(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void))
{
    return OSi_DoLockByWord(lockID, lockp, ctrlFuncp, TRUE);
}


//======================================================================
//                      UNLOCK
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_DoUnlockByWord

  Description:  Unlocks.

  Arguments:    lockID: Lock ID
                lockp: Pointer to unlock variable
                ctrlFuncp: Function
                disableFiq: Whether to disable fiq

  Returns:      OS_UNLOCK_SUCCESS: Success.
                OS_UNLOCK_ERROR: Error while unlocking.
 *---------------------------------------------------------------------------*/
static s32 OSi_DoUnlockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void),
                              BOOL disableFIQ)
{
    OSIntrMode lastInterrupts;

    OSi_ASSERT_ID(lockID);

    if (lockID != lockp->ownerID)
    {
        return OS_UNLOCK_ERROR;
    }

    //---- Disable irq/fiq or irq
    lastInterrupts = (disableFIQ) ? OS_DisableInterrupts_IrqAndFiq() : OS_DisableInterrupts();

    lockp->ownerID = 0;
    if (ctrlFuncp)
    {
        ctrlFuncp();
    }
    lockp->lockFlag = 0;

    //---- Restore irq/fiq or irq
    if (disableFIQ)
    {
        (void)OS_RestoreInterrupts_IrqAndFiq(lastInterrupts);
    }
    else
    {
        (void)OS_RestoreInterrupts(lastInterrupts);
    }

    return OS_UNLOCK_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockByWord

  Description:  Unlocks.
                Disable irq and fiq.

  Arguments:    lockID: Lock ID
                lockp: Pointer to unlock variable
                ctrlFuncp: Function

  Returns:      OS_UNLOCK_SUCCESS: Success.
                OS_UNLOCK_ERROR: Error while unlocking.
 *---------------------------------------------------------------------------*/
s32 OS_UnlockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void))
{
    return OSi_DoUnlockByWord(lockID, lockp, ctrlFuncp, FALSE);
}

//---- For compatibility to old name ('UnLock' <-> 'Unlock')
s32 OS_UnLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void))
{
    return OSi_DoUnlockByWord(lockID, lockp, ctrlFuncp, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockByWord_IrqAndFiq

  Description:  Unlocks.

  Arguments:    lockID: Lock ID
                lockp: Pointer to unlock variable
                ctrlFuncp: Function

  Returns:      OS_UNLOCK_SUCCESS: Success.
                OS_UNLOCK_ERROR: Error while unlocking.
 *---------------------------------------------------------------------------*/
s32 OS_UnlockByWord_IrqAndFiq(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void))
{
    return OSi_DoUnlockByWord(lockID, lockp, ctrlFuncp, TRUE);
}


//======================================================================
//                      TRY LOCK
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OSi_DoTryLockByWord

  Description:  Tries to lock. Only does spinlock once.

  Arguments:    lockID: Lock ID
                lockp: Pointer to trying to lock variable
                ctrlFuncp: Function
                disableFiq: Whether to disable fiq

  Returns:      >0 value: Previous locked ID.
                OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
static s32 OSi_DoTryLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void),
                               BOOL disableFIQ)
{
    s32     lastLockFlag;
    OSIntrMode lastInterrupts;

    OSi_ASSERT_ID(lockID);

    //---- Disable irq/fiq or irq
    lastInterrupts = (disableFIQ) ? OS_DisableInterrupts_IrqAndFiq() : OS_DisableInterrupts();

    lastLockFlag = (s32)MI_SwapWord(lockID, &lockp->lockFlag);

    if (lastLockFlag == OS_LOCK_SUCCESS)
    {
        if (ctrlFuncp)
        {
            ctrlFuncp();
        }
        lockp->ownerID = lockID;
    }

    //---- Restore irq/fiq or irq
    if (disableFIQ)
    {
        (void)OS_RestoreInterrupts_IrqAndFiq(lastInterrupts);
    }
    else
    {
        (void)OS_RestoreInterrupts(lastInterrupts);
    }

    return lastLockFlag;
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockByWord

  Description:  Tries to lock. Only does spinlock once.

  Arguments:    lockID: Lock ID
                lockp: Pointer to trying to lock variable
                ctrlFuncp: Function

  Returns:      >0 value: Previous locked ID.
                OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_TryLockByWord(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void))
{
    return OSi_DoTryLockByWord(lockID, lockp, ctrlFuncp, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockByWord_IrqAndFiq

  Description:  Tries to lock. Only does spinlock once.

  Arguments:    lockID: Lock ID
                lockp: Pointer to trying to lock variable
                ctrlFuncp: Function

  Returns:      >0 value: Previous locked ID.
                OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_TryLockByWord_IrqAndFiq(u16 lockID, OSLockWord *lockp, void (*ctrlFuncp) (void))
{
    return OSi_DoTryLockByWord(lockID, lockp, ctrlFuncp, TRUE);
}


//======================================================================
//                      CARTRIDGE
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_LockCartridge

  Description:  Locks cartridge.

  Arguments:    lockID: Lock ID

  Returns:      OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_LockCartridge(u16 lockID)
{
    s32     lastLockFlag;

    OSi_ASSERT_ID(lockID);

    lastLockFlag =
        OSi_DoLockByWord(lockID, (OSLockWord *)HW_CTRDG_LOCK_BUF, OSi_AllocateCartridgeBus, TRUE);

#ifndef SDK_FINALROM
	{
		u32 type = OSi_DetectDebugger();
		if ( type & OS_CONSOLE_TWLDEBUGGER )
		{
			_ISTDbgLib_AllocateEmualtor();
		}
		else if ( type & OS_CONSOLE_ISDEBUGGER )
		{
			_ISDbgLib_AllocateEmualtor();
		}
    }
#endif

    return lastLockFlag;
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockCartridge

  Description:  Unlocks cartridge.

  Arguments:    lockID: Lock ID

  Returns:      OS_UNLOCK_SUCCESS: Success.
                OS_UNLOCK_ERROR: Error while unlocking.
 *---------------------------------------------------------------------------*/
s32 OS_UnlockCartridge(u16 lockID)
{
    s32     lastLockFlag;

    OSi_ASSERT_ID(lockID);

#ifndef SDK_FINALROM
	{
		u32 type = OSi_DetectDebugger();
		if ( type & OS_CONSOLE_TWLDEBUGGER )
		{
			_ISTDbgLib_FreeEmulator();
		}
		else if ( type & OS_CONSOLE_ISDEBUGGER )
		{		
			_ISDbgLib_FreeEmulator();
		}
	}
#endif

    lastLockFlag =
        OSi_DoUnlockByWord(lockID, (OSLockWord *)HW_CTRDG_LOCK_BUF, OSi_FreeCartridgeBus, TRUE);

    return lastLockFlag;
}

//---- For compatibility to old name ('UnLock' <-> 'Unlock')
asm s32 OS_UnLockCartridge( u16 lockID )
{
  ldr  r1, =OS_UnlockCartridge
  bx   r1
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockCartridge

  Description:  Tries to lock cartridge.

  Arguments:    lockID: Lock ID

  Returns:      >0 value: Previous locked ID.
                OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_TryLockCartridge(u16 lockID)
{
    s32     lastLockFlag;

    lastLockFlag =
        OSi_DoTryLockByWord(lockID, (OSLockWord *)HW_CTRDG_LOCK_BUF, OSi_AllocateCartridgeBus,
                            TRUE);

#ifndef SDK_FINALROM
	if ( lastLockFlag == OS_LOCK_SUCCESS )
	{
		u32 type = OSi_DetectDebugger();
		if ( type & OS_CONSOLE_TWLDEBUGGER )
		{
			_ISTDbgLib_AllocateEmualtor();
		}
		else if ( type & OS_CONSOLE_ISDEBUGGER )
		{
			_ISDbgLib_AllocateEmualtor();
		}
    }
#endif

    return lastLockFlag;
}

//----------------
static void OSi_AllocateCartridgeBus(void)
{
#ifdef  SDK_ARM9
    MIi_SetCartridgeProcessor(MI_PROCESSOR_ARM9);       // Cartridge for MAIN
#endif
}

//----------------
static void OSi_FreeCartridgeBus(void)
{
#ifdef  SDK_ARM9
    MIi_SetCartridgeProcessor(MI_PROCESSOR_ARM7);       // Cartridge for SUB
#endif
}

//======================================================================
//                      CARD
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_LockCard

  Description:  Locks card.

  Arguments:    lockID: Lock ID

  Returns:      OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_LockCard(u16 lockID)
{
    OSi_ASSERT_ID(lockID);

    return OS_LockByWord(lockID, (OSLockWord *)HW_CARD_LOCK_BUF, OSi_AllocateCardBus);
}

/*---------------------------------------------------------------------------*
  Name:         OS_UnlockCard

  Description:  Unlocks card.

  Arguments:    lockID: Lock ID

  Returns:      OS_UNLOCK_SUCCESS: Success.
                OS_UNLOCK_ERROR: Error while unlocking.
 *---------------------------------------------------------------------------*/
s32 OS_UnlockCard(u16 lockID)
{
    OSi_ASSERT_ID(lockID);

    return OS_UnlockByWord(lockID, (OSLockWord *)HW_CARD_LOCK_BUF, OSi_FreeCardBus);
}

//---- For compatibility to old name ('UnLock' <-> 'Unlock')
asm s32 OS_UnLockCard( u16 lockID )
{
  ldr  r1, =OS_UnlockCard
  bx   r1
}

/*---------------------------------------------------------------------------*
  Name:         OS_TryLockCard

  Description:  Tries to lock card.

  Arguments:    lockID: Lock ID

  Returns:      >0 value: Previous locked ID.
                OS_LOCK_SUCCESS: Lock success.
 *---------------------------------------------------------------------------*/
s32 OS_TryLockCard(u16 lockID)
{
    return OS_TryLockByWord(lockID, (OSLockWord *)HW_CARD_LOCK_BUF, OSi_AllocateCardBus);
}

//----------------
static void OSi_AllocateCardBus(void)
{
#ifdef  SDK_ARM9
#ifdef  SDK_TWL
    // Preset reset flag with status of disable interrupts in OSi_DoTryLockByWord
    if ( ((reg_MI_MC & REG_MI_MC_SL1_MODE_MASK) >> REG_MI_MC_SL1_MODE_SHIFT) == 0x2 )
    {
        reg_MI_MCCNT1 |= REG_MI_MCCNT1_RESB_MASK;
    }
#endif
    MIi_SetCardProcessor(MI_PROCESSOR_ARM9);    // Card for MAIN
#endif
}

//----------------
static void OSi_FreeCardBus(void)
{
#ifdef  SDK_ARM9
    MIi_SetCardProcessor(MI_PROCESSOR_ARM7);    // Card for SUB
#endif
}


//======================================================================
//                      READ OWNER
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_ReadOwnerOfLockWord

  Description:  Reads the owner ID of a lock.

       - You can check which processor has ownership now if the module ID is nonzero.
         
       - For a shared resource, prohibiting interrupts can maintain only the main processor's ownership rights.
         
         In other states, it might be changed by subprocessors.
       - The lock variable may not necessarily be unlocked even if owner module ID is 0.

  Arguments:    lockp: Pointer to lock

  Returns:      Owner ID.
 *---------------------------------------------------------------------------*/
u16 OS_ReadOwnerOfLockWord(OSLockWord *lockp)
{
    return lockp->ownerID;
}

//======================================================================
//                     LOCK ID
//======================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetLockID

  Description:  Gets the lock ID.

  Arguments:    None.

  Returns:      OS_LOCK_ID_ERROR, if failed to get ID.

                if ARM9
                   0x40-0x6f: lockID
                else if ARM7
                   0x80-0xaf: lockID
                endif

                Only up to 48 kinds of IDs can be assigned.
                When managing multiple lock variables in a module, use a single ID whenever possible.
                
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
asm s32 OS_GetLockID( void )
{
    //---- Is a flag set (a free ID) in the first 32 flag bits?
    ldr    r3, =OSi_ANYP_LOCK_ID_FLAG
    ldr    r1, [r3, #0]

#ifdef SDK_ARM9
    clz    r2, r1
#else
    mov    r2, #0
    mov    r0, #0x80000000
_lp1:
    tst    r1, r0
    bne    _ex1
    add    r2, r2, #1
    cmp    r2, #32
    beq    _ex1

    mov    r0, r0, lsr #1
    b      _lp1
 _ex1:
#endif
    cmp    r2, #32

    //---- When there is a free ID
    movne  r0, #OSi_ANYP_LOCK_ID_START
    bne    _1

    //---- Is there a flag that is set (a free ID) in the rear 323 bits?
    add    r3, r3, #4
    ldr    r1, [r3, #0]
#ifdef SDK_ARM9
    clz    r2, r1
#else
    mov    r2, #0
    mov    r0, #0x80000000
_lp2:
    tst    r1, r0
    bne    _ex2
    add    r2, r2, #1
    cmp    r2, #32
    beq    _ex2

    mov    r0, r0, lsr #1
    b      _lp2
 _ex2:
#endif
    cmp    r2, #32

    //---- When there are no free IDs
    ldr    r0, =OS_LOCK_ID_ERROR
    bxeq   lr

    //---- When there is a free ID
    mov    r0, #OSi_ANYP_LOCK_ID_START+32

_1:
    add    r0, r0, r2
    mov    r1, #0x80000000
    mov    r1, r1, lsr r2

    ldr    r2, [r3, #0]
    bic    r2, r2, r1
    str    r2, [r3, #0]

    bx     lr
}

/*---------------------------------------------------------------------------*
  Name:         OS_ReleaseLockID

  Description:  Releases lock ID.

  Arguments:    ID to tend to release

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_ReleaseLockID( register u16 lockID )
{
#pragma unused( lockID )

    ldr    r3, =OSi_ANYP_LOCK_ID_FLAG

    cmp    r0, #OSi_ANYP_LOCK_ID_START+32
    addpl  r3, r3, #4

    subpl  r0, r0, #OSi_ANYP_LOCK_ID_START+32
    submi  r0, r0, #OSi_ANYP_LOCK_ID_START

    mov r1, #0x80000000
    mov r1, r1, lsr r0

    ldr    r2, [r3, #0]
    orr    r2, r2, r1
    str    r2, [r3, #0]

    bx     lr
}
#include <nitro/codereset.h>
