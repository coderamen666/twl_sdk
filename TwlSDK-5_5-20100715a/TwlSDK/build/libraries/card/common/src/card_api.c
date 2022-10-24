/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_api.c

  Copyright 2008 Nintendo. All rights reserved.

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

#include "../include/card_common.h"
#include "../include/card_event.h"
#include "../include/card_rom.h"


/*---------------------------------------------------------------------------*/
/* Variables */

static BOOL CARDi_EnableFlag = FALSE;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_LockBusCondition

  Description:   Idle until the listener structure event is complete.

  Arguments:    userdata : Lock ID

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL CARDi_LockBusCondition(void *userdata)
{
    u16     lockID = *(const u16 *)userdata;
    return (OS_TryLockCard(lockID) == OS_LOCK_SUCCESS);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_Init

  Description:  Initializes the Card library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_Init(void)
{
    CARDiCommon *p = &cardi_common;

    if (!p->flag)
    {
        p->flag = CARD_STAT_INIT;

#if !defined(SDK_SMALL_BUILD) && defined(SDK_ARM9)
        // The CARD-ROM header information that is stored at non-card boot is NULL at card boot, so save it here.
        // 
        if (OS_GetBootType() == OS_BOOTTYPE_ROM)
        {
            MI_CpuCopy8((const void *)HW_ROM_HEADER_BUF, (void *)HW_CARD_ROM_HEADER,
                        HW_CARD_ROM_HEADER_SIZE);
        }
#endif // !defined(SDK_SMALL_BUILD) && defined(SDK_ARM9)

#if defined(SDK_ARM9)
        // Initialize internal library variables 
        p->src = 0;
        p->dst = 0;
        p->len = 0;
        p->dma = MI_DMA_NOT_USE;
        p->DmaCall = NULL;
        p->flush_threshold_ic = 0x400;
        p->flush_threshold_dc = 0x2400;
#endif
        cardi_rom_base = 0;
        p->priority = CARD_THREAD_PRIORITY_DEFAULT;

        // Initialize source lock 
        CARDi_InitResourceLock();

        // Start up task thread
#if defined(SDK_ARM9)
        // The old-format wait thread is used for ARM9 for compatibility
        p->callback = NULL;
        p->callback_arg = NULL;
        OS_InitThreadQueue(p->busy_q);
        OS_CreateThread(p->thread.context,
                        CARDi_OldTypeTaskThread, NULL,
                        p->thread.stack + sizeof(p->thread.stack),
                        sizeof(p->thread.stack), p->priority);
        OS_WakeupThreadDirect(p->thread.context);
#else // defined(SDK_ARM9)
        // The stable new-format wait thread is used for ARM 7
        CARDi_InitTaskQueue(p->task_q);
        OS_CreateThread(p->thread.context,
                        CARDi_TaskWorkerProcedure, p->task_q,
                        p->thread.stack + sizeof(p->thread.stack),
                        sizeof(p->thread.stack), p->priority);
        OS_WakeupThreadDirect(p->thread.context);
#endif

        // Initialize the command communication system.
        CARDi_InitCommand();

        // Initialize ROM driver
        CARDi_InitRom();

        // Allow unconditional card access only at card boot.
        // If card access is desired in other startup modes, it is necessary to explicitly call the CARD_Enable function according to predetermined conditions described in the Guidelines 
        // 
        // 
        if (OS_GetBootType() == OS_BOOTTYPE_ROM)
        {
            CARD_Enable(TRUE);
        }

#if !defined(SDK_SMALL_BUILD)
        //---- Detect pulled out card
        CARD_InitPulledOutCallback();
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IsAvailable

  Description:  Determines whether the CARD library can be used. 

  Arguments:    None.

  Returns:      If the CARD_Init function is already called, TRUE.
 *---------------------------------------------------------------------------*/
BOOL CARD_IsAvailable(void)
{
    CARDiCommon *const p = &cardi_common;
    return (p->flag != 0);
}

/*---------------------------------------------------------------------------*
  Name:         CARD_IsEnabled

  Description:  Determines whether the card can be accessed.

  Arguments:    None.

  Returns:      If the system has access rights to the card, TRUE. 
 *---------------------------------------------------------------------------*/
BOOL CARD_IsEnabled(void)
{
    return CARDi_EnableFlag;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckEnabled

  Description:  Determines whether the system has access rights to the card. If no, stop forcibly.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_CheckEnabled(void)
{
    if (!CARD_IsEnabled())
    {
        OS_TPanic("NITRO-CARD permission denied");
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_Enable

  Description:  Sets the access rights to the card.

  Arguments:    enable: If the system has access rights, TRUE 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_Enable(BOOL enable)
{
    CARDi_EnableFlag = enable;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetThreadPriority

  Description:  Get priority of asynchronous processing thread in the library.

  Arguments:    None.

  Returns:      Internal thread priority.
 *---------------------------------------------------------------------------*/
u32 CARD_GetThreadPriority(void)
{
    CARDiCommon *const p = &cardi_common;
    SDK_ASSERT(CARD_IsAvailable());

    return p->priority;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_SetThreadPriority

  Description:  Set priority of asynchronous processing thread in the library.

  Arguments:    None.

  Returns:      Priority of internal thread before setting. 
 *---------------------------------------------------------------------------*/
u32 CARD_SetThreadPriority(u32 prior)
{
    CARDiCommon *const p = &cardi_common;
    SDK_ASSERT(CARD_IsAvailable());

    {
        OSIntrMode bak_psr = OS_DisableInterrupts();
        u32     ret = p->priority;
        SDK_ASSERT((prior >= OS_THREAD_PRIORITY_MIN) && (prior <= OS_THREAD_PRIORITY_MAX));
        p->priority = prior;
        (void)OS_SetThreadPriority(p->thread.context, p->priority);
        (void)OS_RestoreInterrupts(bak_psr);
        return ret;
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetResultCode

  Description:  Gets the result of a CARD function called last.

  Arguments:    None.

  Returns:      The result of the CARD function called last.
 *---------------------------------------------------------------------------*/
CARDResult CARD_GetResultCode(void)
{
    CARDiCommon *const p = &cardi_common;
    SDK_ASSERT(CARD_IsAvailable());

    return p->cmd->result;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetRomHeader

  Description:  Get ROM header information for currently inserted card.

  Arguments:    None.

  Returns:      Pointer to the CARDRomHeader structure.
 *---------------------------------------------------------------------------*/
const u8 *CARD_GetRomHeader(void)
{
    return (const u8 *)HW_CARD_ROM_HEADER;
}

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeader

  Description:  Gets the ROM header information of the program that is currently running.

  Arguments:    None.

  Returns:      Pointer to the CARDRomHeader structure.
 *---------------------------------------------------------------------------*/
const CARDRomHeader *CARD_GetOwnRomHeader(void)
{
    return (const CARDRomHeader *)HW_ROM_HEADER_BUF;
}

#if defined(SDK_TWL)

/*---------------------------------------------------------------------------*
  Name:         CARD_GetOwnRomHeaderTWL

  Description:  Gets the TWL-ROM header information of the program that is currently running.

  Arguments:    None.

  Returns:      Pointer to the CARDRomHeaderTWL structure.
 *---------------------------------------------------------------------------*/
const CARDRomHeaderTWL *CARD_GetOwnRomHeaderTWL(void)
{
    return (const CARDRomHeaderTWL *)HW_TWL_ROM_HEADER_BUF;
}

#endif // SDK_TWL

/*---------------------------------------------------------------------------*
  Name:         CARD_GetCacheFlushThreshold

  Description:  Gets the threshold that determines whether the entire cache will be flushed, or just a portion of it.

  Arguments:    icache: Pointer to the location at which the instruction cache flush threshold should be stored.
                        Ignore if NULL.
                dcache: Pointer to the location at which the data cache flush threshold should be stored.
                        Ignore if NULL.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_GetCacheFlushThreshold(u32 *icache, u32 *dcache)
{
#if defined(SDK_ARM9)
    SDK_ASSERT(CARD_IsAvailable());
    if (icache)
    {
        *icache = cardi_common.flush_threshold_ic;
    }
    if (dcache)
    {
        *dcache = cardi_common.flush_threshold_dc;
    }
#else
    (void)icache;
    (void)dcache;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CARD_SetCacheFlushThreshold

  Description:  Sets the threshold that determines whether the entire cache will be flushed, or just a portion of it.

  Arguments:    icache: The instruction cache flush threshold
                dcache: The data cache flush threshold

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_SetCacheFlushThreshold(u32 icache, u32 dcache)
{
#if defined(SDK_ARM9)
    SDK_ASSERT(CARD_IsAvailable());
    cardi_common.flush_threshold_ic = icache;
    cardi_common.flush_threshold_dc = dcache;
#else
    (void)icache;
    (void)dcache;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CARD_LockRom

  Description:  Locks card bus for accessing ROM.

  Arguments:    Lock id

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_LockRom(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());

    /* Lock the resource */
    CARDi_LockResource(lock_id, CARD_TARGET_ROM);
    /* Lock the card bus */
    {
        // If the ARM7 processor is locked, use the lightest V-Count alarm to sleep because the CPU will be blocked for a long time
        // 
#define CARD_USING_SLEEPY_LOCK
#ifdef CARD_USING_SLEEPY_LOCK
        CARDEventListener   el[1];
        CARDi_InitEventListener(el);
        CARDi_SetEventListener(el, CARDi_LockBusCondition, &lock_id);
        CARDi_WaitForEvent(el);
#else
        // If operations have been verified, this is unnecessary 
        (void)OS_LockCard(lock_id);
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockRom

  Description:  Frees the locked card bus. 

  Arguments:    Lock id that is used by CARD_LockRom

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_UnlockRom(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());
    SDK_ASSERT(cardi_common.lock_target == CARD_TARGET_ROM);

    /* Unlock the card bus */
    {
        (void)OS_UnlockCard(lock_id);
    }
    /* Unlock the resource */
    CARDi_UnlockResource(lock_id, CARD_TARGET_ROM);

}

/*---------------------------------------------------------------------------*
  Name:         CARD_LockBackup

  Description:  Locks the card bus for backup device access.

  Arguments:    lock_id: Exclusive control ID aassigned by the OS_GetLockID function 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_LockBackup(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());

    /* Take exclusive control of ROM/backup in processor */
    CARDi_LockResource(lock_id, CARD_TARGET_BACKUP);
    /* ARM7 always accesses */
#if defined(SDK_ARM7)
    (void)OS_LockCard(lock_id);
#endif
}

/*---------------------------------------------------------------------------*
  Name:         CARD_UnlockBackup

  Description:  Frees the locked card bus for backup device access.

  Arguments:    lock_id: Exclusive control ID used by the CARD_LockBackup function 

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARD_UnlockBackup(u16 lock_id)
{
    SDK_ASSERT(CARD_IsAvailable());
    SDK_ASSERT(cardi_common.lock_target == CARD_TARGET_BACKUP);

#if defined(SDK_ARM9)
    /* Blocking if an attempt is made to deallocate a bus while an asynchronous process is running */
    if (!CARD_TryWaitBackupAsync())
    {
        OS_TWarning("called CARD_UnlockBackup() during backup asynchronous operation. (force to wait)\n");
        (void)CARD_WaitBackupAsync();
    }
#endif // defined(SDK_ARM9)

    /* ARM7 always accesses */
#if defined(SDK_ARM7)
    (void)OS_UnlockCard(lock_id);
#endif
    /* Unlock the resource */
    CARDi_UnlockResource(lock_id, CARD_TARGET_BACKUP);
}
