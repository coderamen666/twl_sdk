/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     system.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_OS_SYSTEM_H_
#define NITRO_OS_SYSTEM_H_

#ifndef SDK_ASM
#include  <nitro/types.h>
#include  <nitro/os/common/systemCall.h>

#include  <nitro/os/common/tick.h>
#include  <nitro/os/common/thread.h>
#include  <nitro/os/common/alarm.h>
#include  <nitro/os/common/arena.h>
#include  <nitro/os/common/valarm.h>
#endif

#include  <nitro/hw/common/armArch.h>

#ifdef SDK_TWL
#include  <twl/os/common/system.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
#ifndef SDK_ASM
#define		OS_EXIT_STRING_1		"*** Exit Nitro/Twl program (status="
#define		OS_EXIT_STRING_2		"%d).\n"
#define		OS_EXIT_STRING			OS_EXIT_STRING_1 OS_EXIT_STRING_2

typedef enum
{
    OS_PROCMODE_USER = HW_PSR_USER_MODE,
    OS_PROCMODE_FIQ = HW_PSR_FIQ_MODE,
    OS_PROCMODE_IRQ = HW_PSR_IRQ_MODE,
    OS_PROCMODE_SVC = HW_PSR_SVC_MODE,
    OS_PROCMODE_ABORT = HW_PSR_ABORT_MODE,
    OS_PROCMODE_UNDEF = HW_PSR_UNDEF_MODE,
    OS_PROCMODE_SYS = HW_PSR_SYS_MODE
}
OSProcMode;


typedef enum
{
    OS_INTRMODE_IRQ_DISABLE = HW_PSR_IRQ_DISABLE,
    OS_INTRMODE_IRQ_ENABLE = 0
}
OSIntrMode_Irq;

typedef enum
{
    OS_INTRMODE_FIQ_DISABLE = HW_PSR_FIQ_DISABLE,
    OS_INTRMODE_FIQ_ENABLE = 0
}
OSIntrMode_Fiq;

typedef u32 OSIntrMode;

#ifdef SDK_ARM9
typedef void (*OSTerminateCallback)(void*);
#endif

//============================================================================
//          CONTROL INTERRUPT
//============================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_EnableInterrupts

  Description:  Set CPSR to enable irq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
extern OSIntrMode OS_EnableInterrupts(void);

/*---------------------------------------------------------------------------*
  Name:         OS_DisableInterrupts

  Description:  Set CPSR to disable irq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
extern OSIntrMode OS_DisableInterrupts(void);

/*---------------------------------------------------------------------------*
  Name:         OS_RestoreInterrupts

  Description:  Restore CPSR irq interrupt

  Arguments:    state of irq interrupt bit

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
extern OSIntrMode OS_RestoreInterrupts(OSIntrMode state);

/*---------------------------------------------------------------------------*
  Name:         OS_EnableInterrupts_IrqAndFiq

  Description:  Set CPSR to enable irq and fiq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE & HW_PSR_FIQ_DISABLE
 *---------------------------------------------------------------------------*/
extern OSIntrMode OS_EnableInterrupts_IrqAndFiq(void);

/*---------------------------------------------------------------------------*
  Name:         OS_DisableInterrupts_IrqAndFiq

  Description:  Set CPSR to disable irq and fiq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE & HW_PSR_FIQ_DISABLE
 *---------------------------------------------------------------------------*/
extern OSIntrMode OS_DisableInterrupts_IrqAndFiq(void);

/*---------------------------------------------------------------------------*
  Name:         OS_RestoreInterrupts_IrqAndFiq

  Description:  Restore CPSR irq and fiq interrupt

  Arguments:    state of irq and fiq interrupt bit

  Returns:      last state of HW_PSR_IRQ_DISABLE & HW_PSR_FIQ_DISABLE
 *---------------------------------------------------------------------------*/
extern OSIntrMode OS_RestoreInterrupts_IrqAndFiq(OSIntrMode state);

/*---------------------------------------------------------------------------*
  Name:         OS_GetCpsrIrq

  Description:  Get current state of CPSR irq interrupt

  Arguments:    None

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
extern OSIntrMode_Irq OS_GetCpsrIrq(void);

//============================================================================
//          PROCESSER MODE
//============================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetProcMode

  Description:  Get processor mode from CPSR

  Arguments:    None.

  Returns:      CPU processor mode (field 0x10-0x1f)
 *---------------------------------------------------------------------------*/
extern OSProcMode OS_GetProcMode(void);


//============================================================================
//          TERMINATE and HALT
//============================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_Terminate

  Description:  finalize and terminate
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OS_Terminate(void);

/*---------------------------------------------------------------------------*
  Name:         OSi_TerminateCore

  Description:  Halt CPU and loop
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OSi_TerminateCore(void);

#ifdef SDK_ARM9
/*---------------------------------------------------------------------------*
  Name:         OSi_SetTerminateCallback

  Description:  set terminate callback
                (internal function. don't use)
  
  Arguments:    callback : callback
                arg      : callback argument

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OSi_SetTerminateCallback( OSTerminateCallback callback, void* arg );
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_Halt

  Description:  Halt CPU
  
  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM9
extern void OS_Halt(void);
#else
static inline void OS_Halt(void)
{
    SVC_Halt();
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_Exit / OS_FExit

  Description:  Display exit string and Terminate.
                This is useful for 'loadrun' tool command.                 
  
  Arguments:    console : console to output string
                status  : exit status

  Returns:      --  (Never return)
 *---------------------------------------------------------------------------*/
extern void OS_Exit(int status);
extern void OS_FExit(int console, int status);

//============================================================================
//          WAIT
//============================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SpinWaitCpuCycles

  Description:  Spin and Wait for specified CPU cycles at least

  Arguments:    cycles      waiting CPU cycle

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OS_SpinWaitCpuCycles(u32 cycle);

/*---------------------------------------------------------------------------*
  Name:         OS_SpinWaitSysCycles

  Description:  Spin and Wait for specified SYSTEM cycles at least

  Arguments:    cycles      waiting SYSTEM cycle

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OS_SpinWaitSysCycles(u32 cycle);

/*---------------------------------------------------------------------------*
  Name:         OS_SpinWait

  Description:  Spin and Wait for specified CPU cycles at least

  Arguments:    cycles      waiting SYSTEM/2 cycle

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
#define OS_SpinWait(cycle)		OS_SpinWaitSysCycles((cycle)>>1)
#else
#define OS_SpinWait(cycle)		OS_SpinWaitSysCycles(cycle)
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_WaitInterrupt

  Description:  wait specified interrupt
                the difference between OS_WaitIrq and OS_WaitInterrupt,
                in waiting interrupt
                OS_WaitIrq does switch thread,
                OS_WaitInterrupt doesn't switch thread.
                OS_WaitIrq wait by using OS_SleepThread() with threadQueue,
                OS_WaitInterrupt wait by using OS_Halt().
                if SDK_NO_THREAD defined, 2 functions become same.

  Arguments:    clear      TRUE if want to clear interrupt flag before wait.
                           FALSE if not.
                irqFlags   bit of interrupts to wait for

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifndef OSi_OSIRQMASK_DEFINED
typedef u32 OSIrqMask;
#define OSi_OSIRQMASK_DEFINED
#endif
extern void OS_WaitInterrupt(BOOL clear, OSIrqMask irqFlags);

/*---------------------------------------------------------------------------*
  Name:         OS_WaitVBlankIntr

  Description:  wait till vblank interrupt occurred.
				the difference between SVC_WaitVBlankIntr and OS_WaitVBlankIntr is:
                OS_WaitVBlankIntr does switch thread,
                SVC_WaitVBlankIntr doesn't switch thread.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
extern void OS_WaitVBlankIntr(void);


#endif /* SDK_ASM */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*NITRO_OS_SYSTEM_H_*/
#endif
