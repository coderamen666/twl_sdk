/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_context.c

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-19#$
  $Rev: 10786 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/
#include <nitro/os.h>
#include <nitro/memorymap.h>

#ifdef SDK_ARM9
#include <nitro/cp/context.h>
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_InitContext

  Description:  Initializes context.

  Arguments:    context:     Context to be initialized
                newpc:       Program counter
                newsp:       Stack pointer

  Returns:      None.
 *---------------------------------------------------------------------------*/
#include    <nitro/code32.h>
asm void OS_InitContext(
    register OSContext* context,
    register u32         newpc,
    register u32         newsp
    )
{
    // ---- Save the execution position (r0 = context)
    add   newpc, newpc, #4
    str   newpc, [ context, #OS_CONTEXT_PC_PLUS4 ]
    
    // ---- Save the stack
#ifdef  SDK_CONTEXT_HAS_SP_SVC
    str   newsp, [ context, #OS_CONTEXT_SP_SVC ]
    sub   newsp, newsp,     #HW_SVC_STACK_SIZE
#endif
    tst   newsp, #4
    subne newsp, newsp, #4 // For 8-byte alignment
    str   newsp, [ context, #OS_CONTEXT_SP ]
        
    // ---- Make status
    ands  r1, newpc, #1
    movne r1, #HW_PSR_SYS_MODE|HW_PSR_THUMB_STATE
    moveq r1, #HW_PSR_SYS_MODE|HW_PSR_ARM_STATE
    str   r1, [ context, #OS_CONTEXT_CPSR ]
    
    // ---- Clear other registers
    mov   r1, #0
    str   r1, [ context, #OS_CONTEXT_R0 ]
    str   r1, [ context, #OS_CONTEXT_R1 ]
    str   r1, [ context, #OS_CONTEXT_R2 ]
    str   r1, [ context, #OS_CONTEXT_R3 ]
    str   r1, [ context, #OS_CONTEXT_R4 ]
    str   r1, [ context, #OS_CONTEXT_R5 ]
    str   r1, [ context, #OS_CONTEXT_R6 ]
    str   r1, [ context, #OS_CONTEXT_R7 ]
    str   r1, [ context, #OS_CONTEXT_R8 ]
    str   r1, [ context, #OS_CONTEXT_R9 ]
    str   r1, [ context, #OS_CONTEXT_R10 ]
    str   r1, [ context, #OS_CONTEXT_R11 ]
    str   r1, [ context, #OS_CONTEXT_R12 ]
    str   r1, [ context, #OS_CONTEXT_LR  ]

    bx    lr    // Start here and switch arm/thumb mode
}

#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_begin.h>
#endif
/*---------------------------------------------------------------------------*
  Name:         OS_SaveContext

  Description:  Saves current context into specified memory.

  Arguments:    context:   Pointer to the memory to be stored the current context

  Returns:      0:    Saving a context (normal)
                1:    If context are reloaded via OS_LoadContext
 *---------------------------------------------------------------------------*/
asm BOOL OS_SaveContext( register OSContext* context )
{
#if defined(SDK_ARM9)
    stmfd   sp!, { lr, r0 }
    add     r0, r0, #OS_CONTEXT_CP_CONTEXT
    ldr     r1, =CP_SaveContext
    blx     r1
    ldmfd   sp!, { lr, r0 }
#endif

    add     r1, r0, #OS_CONTEXT_CPSR
    
    //---- Save CPSR
    mrs     r2, cpsr
    str     r2, [ r1 ], #OS_CONTEXT_R0-OS_CONTEXT_CPSR  // r1 moved to context.r0
        
#ifdef  SDK_CONTEXT_HAS_SP_SVC
    //---- Save SP_svc
    mov     r0, #HW_PSR_SVC_MODE|HW_PSR_IRQ_DISABLE|HW_PSR_FIQ_DISABLE|HW_PSR_ARM_STATE
    msr     cpsr_c, r0
    str     sp, [ r1, #OS_CONTEXT_SP_SVC - OS_CONTEXT_R0 ]
    msr     cpsr_c, r2
#endif

    //---- Save others
    mov     r0, #1              // Return value via OS_LoadContext
    stmia   r1, {r0-r14}        // Save R0-R14
    add     r0, pc, #8          // Set  PC_plus4 to do ("bx lr" + 4)
    str     r0, [r1, #OS_CONTEXT_PC_PLUS4 - OS_CONTEXT_R0 ]
    
    mov     r0, #0              // Regular return value
    bx      lr
}


#define OFFSETOF(x,y) (int)(&(((x*)0)->y))

/*---------------------------------------------------------------------------*
  Name:         OS_LoadContext

  Description:  Reloads specified context as current context.

  Arguments:    context:   Pointer to the memory to switch to the context

  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void OS_LoadContext( register OSContext* context )
{
#if OS_CONTEXT_CPSR != 0
#pragma message(has changed!!!)
    add   r0, r0, #OS_CONTEXT_CPSR
#endif
    
#if defined(SDK_ARM9)
    // Call CPi_RestoreContext
    stmfd     sp!, { lr, r0 }
    add       r0, r0, #OS_CONTEXT_CP_CONTEXT

    ldr       r1, =CPi_RestoreContext
    blx       r1
    ldmfd     sp!, { lr, r0 }
#endif
    
    //---- Mode -> svc
    mrs       r1, cpsr
    bic       r1, r1, #HW_PSR_CPU_MODE_MASK
    orr       r1, r1, #HW_PSR_SVC_MODE|HW_PSR_IRQ_DISABLE|HW_PSR_FIQ_DISABLE
    msr       cpsr_c, r1

    //---- Load cpcr to spsr
    ldr       r1, [ r0 ], #OS_CONTEXT_R0-OS_CONTEXT_CPSR
    msr       spsr_fsxc, r1

#ifdef  SDK_CONTEXT_HAS_SP_SVC
    //---- Load SP_svc
    ldr       sp, [ r0, #OS_CONTEXT_SP_SVC   - OS_CONTEXT_R0 ]
#endif

    //---- Load r0-r14
    ldr       lr, [ r0, #OS_CONTEXT_PC_PLUS4 - OS_CONTEXT_R0 ]
    ldmia     r0, { r0 - r14 }^
    nop

    //---- CP_WaitDiv
    // On NITRO mode, no need to wait divider, because still spend more than 34 cycle.
#if defined(SDK_ARM9) && defined(SDK_TWL)
#ifndef SDK_TWLLTD
    stmfd     sp!, {r0-r3,r12,r14}
#else
    stmfd     sp!, {r0-r1}
#endif

#ifndef SDK_TWLLTD
    bl        OS_IsRunOnTwl
    beq       @01
#endif

    ldr       r0, =REG_DIVCNT_ADDR
@00:
    ldr       r1, [r0]
    ands      r1, r1, #REG_CP_DIVCNT_BUSY_MASK
    bne       @00

#ifndef SDK_TWLLTD
@01:
    ldmfd  sp!, {r0-r3,r12,r14}
#else
    ldmfd  sp!, {r0-r1}
#endif
#endif //if defined(SDK_ARM9) && defined(SDK_TWL)

    //---- Switch cpsr and Jump to (context->pc_plus4 - 4)
    subs      pc, lr, #4
}
#if defined(SDK_TCM_APPLY) && defined(SDK_ARM9)
#include    <nitro/itcm_end.h>
#endif

#include    <nitro/codereset.h>


/*---------------------------------------------------------------------------*
  Name:         OS_DumpContext

  Description:  Performs exception initialization.
                - Installs the first level exception handlers
                - Sets up exception table and common exception handler

  Arguments:    installDBIntegrator if TRUE, copy OSDBIntegrator into
                                    low memory.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void OS_DumpContext(OSContext *context)
{
#ifndef SDK_FINALROM
    s32     i;

    OS_TPrintf("context=%08x\n", context);
    if (context)
    {
        OS_TPrintf("CPSR  %08x\n", context->cpsr);
        for (i = 0; i < 13; i++)
        {
            OS_TPrintf("R%02d   %08x\n", i, context->r[i]);
        }
        OS_TPrintf("SP    %08x\n", context->sp);
        OS_TPrintf("LR    %08x\n", context->lr);
        OS_TPrintf("PC+4  %08x\n", context->pc_plus4);
#ifdef  SDK_CONTEXT_HAS_SP_SVC
        OS_TPrintf("SPsvc %08x\n", context->sp_svc);
#endif
    }
#else
    (void)context;                     // Avoiding to unused warning
#endif
}
