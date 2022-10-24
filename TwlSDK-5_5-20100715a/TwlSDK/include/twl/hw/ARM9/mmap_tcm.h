/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - twl - HW - ARM9
  File:     mmap_tcm.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_HW_ARM9_MMAP_TCM_H_
#define TWL_HW_ARM9_MMAP_TCM_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    Size definition in DTCM
 *---------------------------------------------------------------------------*/
//---- system reserved stack size
#define HW_SVC_STACK_SIZE               0x40    // 64 bytes

//---- system reserved work buffer size
#define HW_DTCM_SYSRV_SIZE              0x40    // 64 bytes

//---- IRQ stack size
#ifndef SDK_ASM
#include    <nitro/types.h>
extern  u32 SDK_IRQ_STACKSIZE[];
#define HW_DTCM_IRQ_STACK_SIZE          ((u32)SDK_IRQ_STACKSIZE)
#else
.extern SDK_IRQ_STACKSIZE
#define HW_DTCM_IRQ_STACK_SIZE          SDK_IRQ_STACKSIZE
#endif

#define HW_SYS_AND_IRQ_STACK_SIZE_MAX   (HW_DTCM_SIZE - HW_SVC_STACK_SIZE - HW_DTCM_SYSRV_SIZE)

/*---------------------------------------------------------------------------*
    Structure of DTCM
 *---------------------------------------------------------------------------*/
#ifndef SDK_ASM
#include    <nitro/types.h>

typedef volatile struct
{
    //---- stack/heap area
    u8      sys_and_irq_stack[HW_SYS_AND_IRQ_STACK_SIZE_MAX];   // 0000-3f80 system & irq stack
    u8      svc_stack[HW_SVC_STACK_SIZE];       // 3f80-3fbf svc stack

    //---- system reserved area
    u8      reserved[HW_DTCM_SYSRV_SIZE - 8];   // 3fc0-3ff7 ????
    u32     intr_check;                // 3ff8-3ffb intr_check for svc
    void   *intr_vector;               // 3ffc-3fff intr handler

}
OS_DTCM;
#endif

/*---------------------------------------------------------------------------*
    Other definition
 *---------------------------------------------------------------------------*/
//---- stack address ( for initial stack pointer )
#define HW_DTCM_SYS_STACK_DEFAULT       HW_DTCM
#define HW_DTCM_SYS_STACK_DEFAULT_END   HW_DTCM_IRQ_STACK
#define HW_DTCM_IRQ_STACK               (HW_DTCM_IRQ_STACK_END - HW_DTCM_IRQ_STACK_SIZE)
#define HW_DTCM_IRQ_STACK_END           HW_DTCM_SVC_STACK
#define HW_DTCM_SVC_STACK               (HW_DTCM_SVC_STACK_END - HW_SVC_STACK_SIZE)
#define HW_DTCM_SVC_STACK_END           HW_DTCM_SYSRV

//---- system reserved area
#define HW_DTCM_SYSRV                   (HW_DTCM + HW_DTCM_SIZE - HW_DTCM_SYSRV_SIZE)
#define HW_INTR_CHECK_BUF               (HW_DTCM_SYSRV + HW_DTCM_SYSRV_OFS_INTR_CHECK)
#define HW_INTR_VECTOR_BUF              (HW_DTCM_SYSRV + HW_DTCM_SYSRV_OFS_INTR_VECTOR)

//---- offset in system reserved area
#define HW_DTCM_SYSRV_OFS_DEBUGGER      0x00
#define HW_DTCM_SYSRV_OFS_RESERVED2     0x1c
#define HW_DTCM_SYSRV_OFS_RESERVED      0x20
#define HW_DTCM_SYSRV_OFS_INTR_CHECK    0x38
#define HW_DTCM_SYSRV_OFS_INTR_VECTOR   0x3c

/*---------------------------------------------------------------------------*
    Definition for Arena
 *---------------------------------------------------------------------------*/
//---- default address for DTCM Arena
#define HW_DTCM_ARENA_LO_DEFAULT        HW_DTCM
#define HW_DTCM_ARENA_HI_DEFAULT        HW_DTCM

//---- default address for ITCM Arena
#define HW_ITCM_ARENA_LO_DEFAULT        HW_ITCM
#define HW_ITCM_ARENA_HI_DEFAULT        HW_ITCM_END

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_HW_ARM9_MMAP_TCM_H_ */
