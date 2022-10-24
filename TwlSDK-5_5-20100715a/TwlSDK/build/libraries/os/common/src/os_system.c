/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_system.c

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
#include <nitro/os.h>
#include <nitro/code32.h>
#ifdef SDK_TWL
#include <twl/scfg/common/scfg.h>
#endif

//============================================================================
//          CONTROL INTERRUPT
//============================================================================
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_begin.h>
#endif
/*---------------------------------------------------------------------------*
  Name:         OS_EnableInterrupts

  Description:  Set CPSR to enable irq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
asm OSIntrMode OS_EnableInterrupts( void )
{
    mrs     r0, cpsr
    bic     r1, r0, #HW_PSR_IRQ_DISABLE
    msr     cpsr_c, r1
    and     r0, r0, #HW_PSR_IRQ_DISABLE

    bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         OS_DisableInterrupts

  Description:  Set CPSR to disable irq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
asm OSIntrMode OS_DisableInterrupts( void )
{
    mrs     r0, cpsr
    orr     r1, r0, #HW_PSR_IRQ_DISABLE
    msr     cpsr_c, r1
    and     r0, r0, #HW_PSR_IRQ_DISABLE

    bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         OS_RestoreInterrupts

  Description:  Restore CPSR irq interrupt

  Arguments:    state of irq interrupt bit

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
asm OSIntrMode OS_RestoreInterrupts( register OSIntrMode state )
{
    mrs     r1, cpsr
    bic     r2, r1, #HW_PSR_IRQ_DISABLE
    orr     r2, r2, r0
    msr     cpsr_c, r2
    and     r0, r1, #HW_PSR_IRQ_DISABLE

    bx      lr
}
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_end.h>
#endif


/*---------------------------------------------------------------------------*
  Name:         OS_EnableInterrupts_IrqAndFiq

  Description:  Set CPSR to enable irq and fiq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE & HW_PSR_FIQ_DISABLE
 *---------------------------------------------------------------------------*/
asm OSIntrMode OS_EnableInterrupts_IrqAndFiq( void )
{
    mrs     r0, cpsr
    bic     r1, r0, #HW_PSR_IRQ_FIQ_DISABLE
    msr     cpsr_c, r1
    and     r0, r0, #HW_PSR_IRQ_FIQ_DISABLE

    bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         OS_DisableInterrupts_IrqAndFiq

  Description:  Set CPSR to disable irq and fiq interrupts

  Arguments:    None.

  Returns:      last state of HW_PSR_IRQ_DISABLE & HW_PSR_FIQ_DISABLE
 *---------------------------------------------------------------------------*/
asm OSIntrMode OS_DisableInterrupts_IrqAndFiq( void )
{
    mrs     r0, cpsr
    orr     r1, r0, #HW_PSR_IRQ_FIQ_DISABLE
    msr     cpsr_c, r1
    and     r0, r0, #HW_PSR_IRQ_FIQ_DISABLE

    bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         OS_RestoreInterrupts_IrqAndFiq

  Description:  Restore CPSR irq and fiq interrupt

  Arguments:    state of irq and fiq interrupt bit

  Returns:      last state of HW_PSR_IRQ_DISABLE & HW_PSR_FIQ_DISABLE
 *---------------------------------------------------------------------------*/
asm OSIntrMode OS_RestoreInterrupts_IrqAndFiq( register OSIntrMode state )
{
    mrs     r1, cpsr
    bic     r2, r1, #HW_PSR_IRQ_FIQ_DISABLE
    orr     r2, r2, r0
    msr     cpsr_c, r2
    and     r0, r1, #HW_PSR_IRQ_FIQ_DISABLE

    bx      lr
}


/*---------------------------------------------------------------------------*
  Name:         OS_GetCpsrIrq

  Description:  Get current state of CPSR irq interrupt

  Arguments:    None

  Returns:      last state of HW_PSR_IRQ_DISABLE
 *---------------------------------------------------------------------------*/
asm OSIntrMode_Irq OS_GetCpsrIrq( void )
{
    mrs     r0, cpsr
    and     r0, r0, #HW_PSR_IRQ_DISABLE

    bx      lr
}


//============================================================================
//          PROCESSER MODE
//============================================================================
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_begin.h>
#endif
/*---------------------------------------------------------------------------*
  Name:         OS_GetProcMode

  Description:  Get processor mode from CPSR

  Arguments:    None.

  Returns:      CPU processor mode (field 0x10-0x1f)
 *---------------------------------------------------------------------------*/
asm OSProcMode OS_GetProcMode( void )
{
    mrs     r0, cpsr
    and     r0, r0, #HW_PSR_CPU_MODE_MASK

    bx      lr
}
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_end.h>
#endif

#include <nitro/codereset.h>


//============================================================================
//          WAIT
//============================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_SpinWaitCpuCycles

  Description:  Spin and Wait for specified CPU cycles at least

  Arguments:    cycles      waiting CPU cycle

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM9
#include <nitro/code32.h>
asm void  OS_SpinWaitCpuCycles( u32 cycle )
{
_1: subs   r0, r0, #4   // 3 cycle
    bcs    _1           // 1 cycle
    bx     lr
}
#include <nitro/codereset.h>
#else
void OS_SpinWaitCpuCycles(u32 cycle)
{
    SVC_WaitByLoop((s32)cycle / 4);
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_SpinWaitSysCycles

  Description:  Spin and Wait for specified SYSTEM cycles at least

  Arguments:    cycles      waiting SYSTEM cycle

  Returns:      None
 *---------------------------------------------------------------------------*/
#ifdef SDK_ARM9
void OS_SpinWaitSysCycles( u32 cycle )
{
#ifdef SDK_TWL
	cycle <<= (SCFG_GetCpuSpeed()==SCFG_CPU_SPEED_2X)? 2: 1;
#else
	cycle <<= 1;
#endif

    if (cycle > 16)
    {
        OS_SpinWaitCpuCycles(cycle - 16);
    }
}
#else
void OS_SpinWaitSysCycles(u32 cycle)
{
    SVC_WaitByLoop((s32)cycle / 4);
}
#endif


/*---------------------------------------------------------------------------*
  Name:         OS_WaitInterrupt

  Description:  wait specifiled interrupt.
                the difference between OS_WaitIrq and OS_WaitInterrupt is:
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
void OS_WaitInterrupt(BOOL clear, OSIrqMask irqFlags)
{
    OSIntrMode cpsrIrq = OS_DisableInterrupts();
    BOOL    ime = OS_EnableIrq();

    if (clear)
    {
        (void)OS_ClearIrqCheckFlag(irqFlags);
    }

    while (!(OS_GetIrqCheckFlag() & irqFlags))
    {
        OS_Halt();
        (void)OS_EnableInterrupts();
        (void)OS_DisableInterrupts();
    }

    (void)OS_ClearIrqCheckFlag(irqFlags);
    (void)OS_RestoreIrq(ime);
    (void)OS_RestoreInterrupts(cpsrIrq);
}

/*---------------------------------------------------------------------------*
  Name:         OS_WaitVBlankIntr

  Description:  wait till vblank interrupt occurred.
				the difference between SVC_WaitVBlankIntr and OS_WaitVBlankIntr is:
                OS_WaitVBlankIntr does switch thread,
                SVC_WaitVBlankIntr doesn't switch thread.

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_WaitVBlankIntr(void)
{
    SVC_WaitByLoop(1);
#if defined(SDK_ENABLE_ARM7_PRINT) && defined(SDK_ARM9)
    // PrintServer for ARM7 (if specified)
    OS_PrintServer();
#endif
    OS_WaitIrq(TRUE, OS_IE_V_BLANK);
}

