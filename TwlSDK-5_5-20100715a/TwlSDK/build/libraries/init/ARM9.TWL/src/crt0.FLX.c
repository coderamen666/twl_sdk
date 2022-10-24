/*---------------------------------------------------------------------------*
  Project:  TwlSDK - libraries - init - ARM9.TWL
  File:     crt0.FLX.c

  Copyright 2007-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-02-04#$
  $Rev: 11289 $
  $Author: yada $
 *---------------------------------------------------------------------------*/

#include    <nitro/types.h>
#include    <nitro/hw/common/armArch.h>
#include    <nitro/mi/uncompress.h>
#include    <nitro/os/common/systemWork.h>
#include    <twl/init/crt0.h>
#include    <twl/memorymap.h>
#include    <twl/misc.h>
#include    <twl/version.h>
#include    "boot_sync.h"

/*---------------------------------------------------------------------------*/
void    _start(void);
void    _start_AutoloadDoneCallback(void* argv[]);

#define     SDK_NITROCODE_LE    0x2106c0de
#define     SDK_NITROCODE_BE    0xdec00621

#define     SDK_TWLCODE_LE      0x6314c0de
#define     SDK_TWLCODE_BE      0xdec01463

#define     SDK_BUILDCODE_LE      0x3381c0de
#define     SDK_BUILDCODE_BE      0xdec08133


/* For embedding build type */
#if defined(SDK_DEBUG)
#define SDK_BUILDCODE 2
#elif defined(SDK_RELEASE)
#define SDK_BUILDCODE 1
#elif defined(SDK_FINALROM)
#define SDK_BUILDCODE 0
#else
#define SDK_BUILDCODE 255     //  Error
#endif

#if defined(SDK_ARM9)
#define SDK_TARGET 9
#elif defined(SDK_ARM7)
#define SDK_TARGET 7
#else
#define SDK_TARGET 255      //  Error
#endif

/* External function reference definitions */
extern void OS_IrqHandler(void);
extern void _fp_init(void);
extern void __call_static_initializers(void);
extern void OS_ShowAttentionOfLimitedRom(void);

/* Internal Function Prototype Definitions */
static void INITi_CpuClear32(register u32 data, register void* destp, register u32 size);
static void INITi_InitCoprocessor(void);
static void INITi_InitRegion(void);
static BOOL INITi_IsRunOnTwl( void );
static void INITi_DoAutoload(void);
#ifndef SDK_NOINIT
static void INITi_ShelterStaticInitializer(u32* ptr);
static void INITi_CallStaticInitializers(void);
#endif
static void* INITi_Copy32(void* dst, void* src, u32 size);
static void* INITi_Fill32(void* dst, u32 value, u32 size);

/* Symbol reference defined by linker script */
extern void SDK_AUTOLOAD_LIST(void);
extern void SDK_AUTOLOAD_LIST_END(void);
extern void SDK_AUTOLOAD_START(void);
extern void SDK_STATIC_BSS_START(void);
extern void SDK_STATIC_BSS_END(void);


void* const _start_ModuleParams[]   =
{
    (void*)SDK_AUTOLOAD_LIST,
    (void*)SDK_AUTOLOAD_LIST_END,
    (void*)SDK_AUTOLOAD_START,
    (void*)SDK_STATIC_BSS_START,
    (void*)SDK_STATIC_BSS_END,
    (void*)0,       // CompressedStaticEnd. This fixed number will be updated by the compstatic tool
    (void*)SDK_CURRENT_VERSION_NUMBER,
    (void*)SDK_NITROCODE_BE,
    (void*)SDK_NITROCODE_LE,
};

/* Allocate 32 bytes for storage of build information */
#pragma force_active on
void* const _start_BuildParams[]    =
{
    (void*)0,       // Reserved
    (void*)0,       // Reserved
    (void*)0,       // Reserved
    (void*)0,       // Reserved
    (void*)(SDK_BUILDCODE | (SDK_TARGET << 8)), // Build target and build type
    (void*)1,       // Version (1byte) Reserved (3byte)
    (void*)SDK_BUILDCODE_BE,
    (void*)SDK_BUILDCODE_LE,
};

extern void SDK_LTDAUTOLOAD_LIST(void);
extern void SDK_LTDAUTOLOAD_LIST_END(void);
extern void SDK_LTDAUTOLOAD_START(void);

void* const _start_LtdModuleParams[]    =
{
    (void*)SDK_LTDAUTOLOAD_LIST,
    (void*)SDK_LTDAUTOLOAD_LIST_END,
    (void*)SDK_LTDAUTOLOAD_START,
    (void*)0,       // CompressedLtdautoloadEnd. This fixed number will be updated by the compstatic tool
    (void*)SDK_TWLCODE_BE,
    (void*)SDK_TWLCODE_LE,
};



/*---------------------------------------------------------------------------*
  Name:         ShakeHand
  Description:  Synchronizes with the ARM7 ShakeHand function.
                Must be run in memory space that is not the main memory.
  Arguments:    r0: Pointer to a variable used to synchronize with the ARM9
                r1: Pointer to a variable used to synchronize with the ARM7
  Returns:      None.
 *---------------------------------------------------------------------------*/
static const u32    microcode_ShakeHand[10] =
{
    0xe1d120b0, /*      ldrh    r2, [r1]    ; Read synchronous variable 7 from shared memory */
    0xe1d030b0, /*      ldrh    r3, [r0]    ; Read synchronous variable 9 from shared memory */
    0xe2833001, /*  @1: add     r3, r3, #1  ; Synchronous variable 9++ */
    0xe1c030b0, /*      strh    r3, [r0]    ; Write synchronous variable 9 to shared memory */
    0xe1d1c0b0, /*      ldrh    r12, [r1]   ; Read the current state of synchronous variable 7 from shared memory */
    0xe152000c, /*      cmp     r2, r12     ; Determine change of synchronous variable 7 */
    0x0afffffa, /*      beq     @1          ; If not changed, loop */
    0xe2833001, /*      add     r3, r3, #1  ; Synchronous variable 9++ */
    0xe1c030b0, /*      strh    r3, [r0]    ; Write synchronous variable 9 to shared memory */
    0xe12fff1e  /*      bx      lr          ; Complete handshake */
};

/*---------------------------------------------------------------------------*
  Name:         WaitAgreement
  Description:  Waits until the ARM7 startup vector is in a specific state.
                Must be run in memory space that is not the main memory.
  Arguments:    r0: Pointer to phase management variable for synchronization
                r1: Idling phase number
  Returns:      None.
 *---------------------------------------------------------------------------*/
static const u32    microcode_WaitAgreement[7]  =
{
    0xe1d020b0, /*  @1: ldrh    r2, [r0]    ; Read the phase management variable from shared memory */
    0xe1510002, /*      cmp     r1, r2      ; Compare with idling phase number */
    0x012fff1e, /*      bxeq    lr          ; If matched, complete idling */
    0xe3a03010, /*      mov     r3, #16     ; Initialize empty loop count */
    0xe2533001, /*  @2: subs    r3, r3, #1  ; Empty loop count  -- */
    0x1afffffd, /*      bne     @2          ; 16 loops */
    0xeafffff8  /*      b       @1          ; Return to top */
};

/*---------------------------------------------------------------------------*
  Name:         SwitchCpuClock
  Description:  Changes ARM CPU core operating clock.
                Must be run in I-TCM.
  Arguments:    r0: Speed mode to switch
                        ( 0: Constant speed/ other than 0: Double speed)
                r1: Cycle count to idle after changing the clock.
  Returns:      r0: Returns the speed mode prior to the change.
                        ( 0: Constant speed/1: Double speed)
 *---------------------------------------------------------------------------*/
static const u32    microcode_SwitchCpuClock[13]    =
{
    0xe3500000, /*      cmp     r0, #0          ; Evaluate first argument */
    0xe59f3024, /*      ldr     r3, [pc, #36]   ; Read the REG_CLK_ADDR constant */
    0xe1d300b0, /*      ldrh    r0, [r3]        ; Read the REG_CLK_ADDR content */
    0x03c02001, /*      biceq   r2, r0, #1      ; Lower the REG_SCFG_CLK_CPUSPD_MASK flag when changing to constant speed */
    0x13802001, /*      orrne   r2, r0, #1      ; Raise the REG_SCFG_CLK_CPUSPD_MASK flag when changing to double speed */
    0xe1500002, /*      cmp     r0, r2          ; Evaluate the need to change the content of REG_CLK_ADDR */
    0xe2000001, /*      and     r0, r0, #1      ; Edit the function return value */
    0x012fff1e, /*      bxeq    lr              ; Quit function if change is not necessary */
    0xe1c320b0, /*      strh    r2, [r3]        ; Write the changed contents to REG_CLK_ADDR */
    0xe2511004, /*  @1: subs    r1, r1, #4      ; 1 cycle ; Empty loop count -- */
    0xaafffffd, /*      bge     @1              ; 3 cycles or 1 cycle ; (Idle cycle count/4) numbers of loops; Complete clock change */
    0xe12fff1e, /*      bx      lr              ; Check instruction from branch destination at second cycle */
    0x04004004  /*      REG_CLK_ADDR            ; REG_CLK_ADDR constant definition */
};

/*---------------------------------------------------------------------------*/
#include    <twl/code32.h>

/*---------------------------------------------------------------------------*
  Name:         _start
  Description:  Startup vector.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
#define SET_PROTECTION_A(id, adr, siz)      ldr r0, =(adr|HW_C6_PR_##siz|HW_C6_PR_ENABLE)
#define SET_PROTECTION_B(id, adr, siz)      mcr p15, 0, r0, c6, id, 0


SDK_WEAK_SYMBOL asm void
_start(void)
{
@000:
        /* Interrupts disabled */
        mov             r12, #HW_REG_BASE
        str             r12, [r12, #REG_IME_OFFSET]     // Use that LSB of HW_REG_BASE is 0b0

        /* Initialize system control coprocessor */
        bl              INITi_InitCoprocessor

        // Processes to cover the initial state of IPL

        // (1) Clear the shared region to zero (avoid because HW_CTRDG_LOCK_BUF is also using IS-NITRO-DEBUGGER)
        mov             r0, #0
        ldr             r1, =HW_PXI_SIGNAL_PARAM_ARM9
        ldr             r2, =(HW_CTRDG_LOCK_BUF-HW_PXI_SIGNAL_PARAM_ARM9)
        bl              INITi_CpuClear32
        mov             r0, #0
        ldr             r1, =HW_INIT_LOCK_BUF
        ldr             r2, =(HW_MAIN_MEM_SYSTEM_END-HW_INIT_LOCK_BUF)
        bl              INITi_CpuClear32

        // (1.5) Clear reserved area in parameter region
        mov             r0, #0
        ldr             r1, =(HW_MAIN_MEM_PARAMETER_BUF_END-HW_PARAM_TWL_RESERVED_AREA_SIZE)
        ldr             r2, =HW_PARAM_TWL_RESERVED_AREA_SIZE
        bl              INITi_CpuClear32

        // (2) Correction of boot type (if illegal, consider it to be an unsupported firmware and go to ROM)
        ldr             r1, =HW_WM_BOOT_BUF
        ldrh            r2, [r1, #OSBootInfo.boot_type]
        cmp             r2, #OS_BOOTTYPE_ILLEGAL
        moveq           r2, #OS_BOOTTYPE_ROM
        streqh          r2, [r1, #OSBootInfo.boot_type]

        /* Copy the micro-code for the handshake to ITCM */
        ldr             r1, =microcode_ShakeHand
        ldr             r2, =HW_ITCM
        add             r3, r2, #40
@001:   ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @001

        /* Handshake with ARM7 with the code on ITCM */
        ldr             r0, =HW_BOOT_SYNC_PHASE
        mov             r1, #BOOT_SYNC_PHASE_1
        strh            r1, [r0]
        ldr             r0, =HW_BOOT_SHAKEHAND_9
        ldr             r1, =HW_BOOT_SHAKEHAND_7
        ldr             r2, =HW_ITCM
        blx             r2

#ifdef SDK_TWLLTD
        /* For LTD on NTR, do not synchronize with the ARM7 (it will not respond) */
        bl              INITi_IsRunOnTwl
        bne             @020
#endif

        /* Overwrite and copy the micro-code for synchronization with ARM7 to ITCM */
        ldr             r1, =microcode_WaitAgreement
        ldr             r2, =HW_ITCM
        add             r3, r2, #28
@002:   ldr             r0, [r1], #4
        str             r0, [r2], #4
        cmp             r2, r3
        blt             @002

@003:
        /* Synchronize with ARM7 */
        ldr             r0, =HW_BOOT_SYNC_PHASE
        mov             r1, #BOOT_SYNC_PHASE_4
        ldr             r2, =HW_ITCM
        blx             r2

@010:
        /* Check whether running on TWL hardware */
        bl              INITi_IsRunOnTwl
        bne             @020

        /* Copy the micro-code for changing to double speed to ITCM */
        ldr             r1, =microcode_SwitchCpuClock
        ldr             r2, =HW_ITCM
        add             r2, r2, #28
        mov             r3, #52
@011:   subs            r3, r3, #4
        ldr             r0, [r1, r3]
        str             r0, [r2, r3]
        bgt             @011
        /* Change to double-speed mode for the CPU clock */
        mov             r0, #REG_SCFG_CLK_CPUSPD_MASK
        mov             r1, #8
        blx             r2

        /* [TODO] Run the default settings of the added I/O registers that can only be set on ARM9 */

@020:
        /* Region default settings */
        bl              INITi_InitRegion

        /* Stack pointer settings */
        mov             r0, #HW_PSR_SVC_MODE    // SuperVisor mode
        msr             cpsr_c, r0
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE
        sub             sp, r1, #HW_DTCM_SYSRV_SIZE
        sub             r1, sp, #HW_SVC_STACK_SIZE
        mov             r0, #HW_PSR_IRQ_MODE    // IRQ mode
        msr             cpsr_c, r0
        sub             sp, r1, #4              // 4 bytes for stack check code
        tst             sp, #4
        subeq           sp, sp, #4              /* Adjust so that sp is 8-byte aligned at the time of the jump to the IRQ handler */
        ldr             r0, =SDK_IRQ_STACKSIZE
        sub             r1, r1, r0
        mov             r0, #HW_PSR_SYS_MODE    // System mode
        msr             cpsr_csfx, r0
        sub             sp, r1, #4              // 4 bytes for stack check code
        tst             sp, #4
        subne           sp, sp, #4              /* Adjust so that sp is 8-byte aligned at the time of the jump to the Main function */

        /* Clear the stack region */
        mov             r0, #0
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        mov             r2, #HW_DTCM_SIZE
        bl              INITi_CpuClear32

        /* Clear the VRAM to 0 */
        mov             r1, #0              // r1 = clear value for VRAM
        ldr             r0, =HW_PLTT        // r0 = start address of VRAM for pallet
        mov             r2, #HW_PLTT_SIZE   // r2 = size of VRAM for pallet
        bl              INITi_Fill32
        ldr             r0, =HW_OAM         // r0 = start address of VRAM for OAM
        mov             r2, #HW_OAM_SIZE    // r2 = size of VRAM for OAM
        bl              INITi_Fill32

        /* Implement Autoload */
        bl              INITi_DoAutoload

        /* Clear the .bss section of the STATIC block to 0 */
        mov             r1, #0              // r1 = clear value for STATIC bss section
        ldr             r3, =_start_ModuleParams
        ldr             r0, [r3, #12]       // r0 = start address of STATIC bss section
        ldr             r2, [r3, #16]
        subs            r2, r2, r0          // r2 = size of STATIC bss section
        blgt            INITi_Fill32

        /* Configure interrupt vector */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE - HW_DTCM_SYSRV_SIZE
        add             r1, r1, #HW_DTCM_SYSRV_OFS_INTR_VECTOR
        ldr             r0, =OS_IrqHandler
        str             r0, [r1]

#ifdef SDK_TWLLTD
        /* Warning screen for LTD on NTR */
        bl              INITi_IsRunOnTwl
        blne            OS_ShowAttentionOfLimitedRom
        // (Does not return from the function above.)
#endif

#ifndef SDK_NOINIT
        /* Initialize for C++ */
        bl              _fp_init
        bl              TwlStartUp
        bl              __call_static_initializers
        bl              INITi_CallStaticInitializers
#endif

        /* V-Count adjustment */
        ldr             r1, =REG_VCOUNT_ADDR
@022:   ldrh            r0, [r1]
        cmp             r0, #0
        bne             @022

@030:
        /* Jump to Main function */
        ldr             r1, =TwlMain
        ldr             lr, =HW_RESET_VECTOR
        bx              r1
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CpuClear32
  Description:  Clears buffer in 32-bit units.
  Arguments:    r0: Value to clear
                r1: Pointer to the clear destination
                r2: Buffer length to continuously clear
  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
INITi_CpuClear32(register u32 data, register void* destp, register u32 size)
{
        add             r12, r1, r2
@001:   cmp             r1, r12
        strlt           r0, [r1], #4
        blt             @001
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_InitCoprocessor
  Description:  Initializes the system control co-processor.
                At the same time, enable the use of I-TCM and D-TCM.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
INITi_InitCoprocessor(void)
{
        /* Get the co-processor state */
        mrc             p15, 0, r0, c1, c0, 0

        tst             r0, #HW_C1_PROTECT_UNIT_ENABLE
        beq             @010
        tst             r0, #HW_C1_DCACHE_ENABLE
        beq             @003

        /* Write back the D-Cache content to memory */
        mov             r1, #0
@001:   mov             r2, #0
@002:   orr             r3, r1, r2
        mcr             p15, 0, r3, c7, c10, 2
        add             r2, r2, #HW_CACHE_LINE_SIZE
        cmp             r2, #HW_DCACHE_SIZE / 4
        blt             @002
        adds            r1, r1, #1 << HW_C7_CACHE_SET_NO_SHIFT
        bne             @001

@003:   /* Wait until the write buffer is empty */
        mov             r1, #0
        mcr             p15, 0, r1, c7, c10, 4

@010:   /* Initialize the co-processor state */
        ldr             r1, = HW_C1_ITCM_LOAD_MODE          \
                            | HW_C1_DTCM_LOAD_MODE          \
                            | HW_C1_ITCM_ENABLE             \
                            | HW_C1_DTCM_ENABLE             \
                            | HW_C1_LD_INTERWORK_DISABLE    \
                            | HW_C1_ICACHE_ENABLE           \
                            | HW_C1_DCACHE_ENABLE           \
                            | HW_C1_PROTECT_UNIT_ENABLE
        bic             r0, r0, r1
        ldr             r1, = HW_C1_SB1_BITSET              \
                            | HW_C1_EXCEPT_VEC_UPPER
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        /* Set the I-TCM size */
        mov             r1, #HW_C9_TCMR_32MB
        mcr             p15, 0, r1, c9, c1, 1
        /* Set the D-TCM size and region base address */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        orr             r1, r1, #HW_C9_TCMR_16KB
        mcr             p15, 0, r1, c9, c1, 0

        /* Set to allow use of I-TCM and D-TCM */
        mov             r1, #HW_C1_ITCM_ENABLE | HW_C1_DTCM_ENABLE
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_InitRegion
  Description:  Initializes the region settings.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
/* When hardware is TWL
; Region G:  BACK_GROUND:  Base = 0x0,        Size = 4GB,   I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 0:  IO_VRAM:      Base = 0x04000000, Size = 64MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 1:  MAINMEM_WRAM: Base = 0x02000000, Size = 32MB,  I:Cach Buf / D:Cach Buf,  I:RW / D:RW
; Region 2:  ARM7_RESERVE: Base = 0x02f80000, Size = 512KB, I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 3:  EX_MAINMEM:   Base = 0x0d000000, Size = 16MB,  I:Cach Buf / D:Cach Buf,  I:RW / D:RW
; Region 4:  DTCM:         Base = 0x02fe0000, Size = 16KB,  I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 5:  ITCM:         Base = 0x01000000, Size = 16MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 6:  BIOS:         Base = 0xffff0000, Size = 32KB,  I:Cach NB  / D:Cach NB,   I:RO / D:RO
; Region 7:  SHARED_WORK:  Base = 0x02ffc000, Size = 16KB,  I:NC NB    / D:NC NB,     I:NA / D:RW
*/
/* When hardware is NITRO
; Region G:  BACK_GROUND:  Base = 0x0,        Size = 4GB,   I:NC NB    / D:NC NB,     I:NA / D:NA
; Region 0:  IO_VRAM:      Base = 0x04000000, Size = 64MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 1:  MAIN_MEM:     Base = 0x02000000, Size = 8MB*,  I:Cach Buf / D:Cach Buf,  I:RW / D:RW
;            (* When hardware is not debugger, size will be reduced to 4MB in OS_InitArena() )
;// Region 2:  ARM7_RESERVE: Base = 0x027e0000, Size = 128KB, I:NC NB    / D:NC NB,     I:NA / D:NA
;//            (* When hardware is not debugger, base will be moved to 0x023e0000 in OS_InitArena() )
; Region 2:  SHARED_WORK:  Base = 0x027ff000, Size = 4KB,   I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 3:  CARTRIDGE:    Base = 0x08000000, Size = 128MB, I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 4:  DTCM:         Base = 0x02fe0000, Size = 16KB,  I:NC NB    / D:NC NB,     I:NA / D:RW
; Region 5:  ITCM:         Base = 0x01000000, Size = 16MB,  I:NC NB    / D:NC NB,     I:RW / D:RW
; Region 6:  BIOS:         Base = 0xffff0000, Size = 32KB,  I:Cach NB  / D:Cach NB,   I:RO / D:RO
; Region 7:  SHARED_WORK:  Base = 0x02fff000, Size = 4KB,   I:NC NB    / D:NC NB,     I:NA / D:RW
*/

static asm void
INITi_InitRegion(void)
{
        mov             r12, lr

//#define SET_PROTECTION_A(id, adr, siz)      ldr r0, =(adr|HW_C6_PR_##siz|HW_C6_PR_ENABLE)
//#define SET_PROTECTION_B(id, adr, siz)      mcr p15, 0, r0, c6, id, 0
#define REGION_BIT(a, b, c, d, e, f, g, h)  (((a) << 0) | ((b) << 1) | ((c) << 2) | ((d) << 3) | ((e) << 4) | ((f) << 5) | ((g) << 6) | ((h) << 7))
#define REGION_ACC(a, b, c, d, e, f, g, h)  (((a) << 0) | ((b) << 4) | ((c) << 8) | ((d) << 12) | ((e) << 16) | ((f) << 20) | ((g) << 24) | ((h) << 28))
#define NA      0
#define RW      1
#define RO      5

        /* (0) I/O register and VRAM, etc. */
        SET_PROTECTION_A(c0, HW_IOREG, 64MB)
        SET_PROTECTION_B(c0, HW_IOREG, 64MB)

        /* (4) D-TCM */
        ldr             r0, =SDK_AUTOLOAD_DTCM_START
        orr             r0, r0, #HW_C6_PR_16KB | HW_C6_PR_ENABLE
        SET_PROTECTION_B(c4, SDK_AUTOLOAD_DTCM_START, 16KB)

        /* (5) I-TCM */
        SET_PROTECTION_A(c5, HW_ITCM_IMAGE, 16MB)
        SET_PROTECTION_B(c5, HW_ITCM_IMAGE, 16MB)

        /* (6) System call ROM */
        SET_PROTECTION_A(c6, HW_BIOS, 32KB)
        SET_PROTECTION_B(c6, HW_BIOS, 32KB)

        /* Check whether running on TWL hardware */
        bl              INITi_IsRunOnTwl
        bne             @002

@001:   /* When using TWL hardware */
        /* (1) Main memory and WRAM */
        SET_PROTECTION_A(c1, HW_TWL_MAIN_MEM_MAIN, 32MB)
        SET_PROTECTION_B(c1, HW_TWL_MAIN_MEM_MAIN, 32MB)

        /* (2) ARM7-dedicated main memory space */
        SET_PROTECTION_A(c2, HW_TWL_MAIN_MEM_SUB, 512KB)
        SET_PROTECTION_B(c2, HW_TWL_MAIN_MEM_SUB, 512KB)

        /* (3) Extended main memory */
        SET_PROTECTION_A(c3, HW_CTRDG_ROM, 128MB)
        SET_PROTECTION_B(c3, HW_CTRDG_ROM, 128MB)
        //SET_PROTECTION_A(c3, HW_TWL_MAIN_MEM_EX, 16MB)
        //SET_PROTECTION_B(c3, HW_TWL_MAIN_MEM_EX, 16MB)

        /* (7) ARM9/ARM7-shared main memory space */
        SET_PROTECTION_A(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)
        SET_PROTECTION_B(c7, HW_TWL_MAIN_MEM_SHARED, 16KB)

        /* Command cache permission */
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 1

        /* Data cache permission */
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 0

        /* Write buffer permission */
        mov             r0, #REGION_BIT(0, 1, 0, 1, 0, 0, 0, 0)
        mcr             p15, 0, r0, c3, c0, 0

        /* Command access permission */
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, NA, RW, RO, NA)
        mcr             p15, 0, r0, c5, c0, 3

        /* Data access permission */
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, RW, RW, RO, RW)
        mcr             p15, 0, r0, c5, c0, 2

        b               @003

@002:   /* When using NITRO hardware */
        /* (1) Main memory */
		//SET_PROTECTION_A(c1, HW_MAIN_MEM_MAIN, 8MB)
		//SET_PROTECTION_B(c1, HW_MAIN_MEM_MAIN, 8MB)
        SET_PROTECTION_A(c1, HW_MAIN_MEM_MAIN, 32MB)
        SET_PROTECTION_B(c1, HW_MAIN_MEM_MAIN, 32MB)
        /* Size will be arranged in OS_InitArena() */

        /* (2) ARM7-dedicated main memory space */
		//SET_PROTECTION_A(c2, (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE - HW_MAIN_MEM_SUB_SIZE), 128KB)
		//SET_PROTECTION_B(c2, (HW_MAIN_MEM_EX_END - HW_MAIN_MEM_SHARED_SIZE - HW_MAIN_MEM_SUB_SIZE), 128KB)
        SET_PROTECTION_A(c2, (HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE), 4KB)
        SET_PROTECTION_B(c2, (HW_MAIN_MEM_IM_SHARED_END - HW_MAIN_MEM_IM_SHARED_SIZE), 4KB)
        /* Base address will be moved in OS_InitArena() */

        /* (3) Cartridge */
        SET_PROTECTION_A(c3, HW_CTRDG_ROM, 128MB)
        SET_PROTECTION_B(c3, HW_CTRDG_ROM, 128MB)
        //SET_PROTECTION_A(c3, HW_CTRDG_ROM, 32MB)
        //SET_PROTECTION_B(c3, HW_CTRDG_ROM, 32MB)

        /* (7) ARM9/ARM7-shared main memory space */
        SET_PROTECTION_A(c7, HW_MAIN_MEM_SHARED, 4KB)
        SET_PROTECTION_B(c7, HW_MAIN_MEM_SHARED, 4KB)

        /* Command cache permission */
        mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 1

        /* Data cache permission */
        mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 1, 0)
        mcr             p15, 0, r0, c2, c0, 0

        /* Write buffer permission */
        mov             r0, #REGION_BIT(0, 1, 0, 0, 0, 0, 0, 0)
        mcr             p15, 0, r0, c3, c0, 0

        /* Command access permission */
        //ldr             r0, =REGION_ACC(RW, RW, NA, NA, NA, RW, RO, NA)
        ldr             r0, =REGION_ACC(RW, RW, NA, RW, NA, RW, RO, NA)
        mcr             p15, 0, r0, c5, c0, 3

        /* Data access permission */
        ldr             r0, =REGION_ACC(RW, RW, RW, RW, RW, RW, RO, RW)
        mcr             p15, 0, r0, c5, c0, 2

@003:   /* Set to allow use of the protection unit and cache */
        mrc             p15, 0, r0, c1, c0, 0
        ldr             r1, = HW_C1_ICACHE_ENABLE       \
                            | HW_C1_DCACHE_ENABLE       \
                            | HW_C1_CACHE_ROUND_ROBIN   \
                            | HW_C1_PROTECT_UNIT_ENABLE
        orr             r0, r0, r1
        mcr             p15, 0, r0, c1, c0, 0

        /* Destroy the content of the cache */
        mov             r1, #0
        mcr             p15, 0, r1, c7, c6, 0
        mcr             p15, 0, r1, c7, c5, 0

        bx              r12
}

/*---------------------------------------------------------------------------*
  Name:         INITi_DoAutoload
  Description:  Clears to 0 the expansion of the fixed data portion and variable portion of each autoload block based on link information.
                Expands the autoload block located in the PSRAM memory space that exceeds 4 MB only when using TWL hardware.
                If a portion of the autoload source data and the autoload destination overlap, expand from the end
                
                
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
/*
 * < Two-stage autoload >
 * The static segment and first level load source binary (second half compressed if necessary) are located at 0x02000000.
 *  If compressed, initially expand while overwriting from the end at 0x02000000.
 *  Load binary data located on ITCM that can be shared with NITRO to 0x01ff8000.
 *  Load binary data located on DTCM that can be shared with NITRO to 0x02fe0000.
 * The binary data of the second-level load source (all compressed if necessary) is located at 0x02400000.
 *  If compressed, initially expand while overwriting from the end at 0x02400000.
 *  Load TWL-specific binary data for WRAM to its specified addresses.
 *  Load TWL-specific binary data for main memory by copying from the front. 
 *  Because NITRO data for main memory is not expected to exceed 0x02400000, the storage address is expected to be smaller than 0x02400000.
 *  In some cases, the autoload information list itself is destroyed in the process of clearing the .bss section of the autoload block for main memory. But this occurs in the final stage of the autoload processes, and should not cause any problems.
 *  
 *  
 */
static asm void
INITi_DoAutoload(void)
{
        stmfd           sp!, {lr}

@01_uncompress:
        /* Decompress the NITRO-shared block */
        ldr             r1, =_start_ModuleParams
        ldr             r0, [r1, #20]       // r0 = bottom of compressed data
        bl              MIi_UncompressBackward
@01_autoload:
        /* Autoload the NITRO-shared block */
        ldr             r0, =_start_ModuleParams
        ldr             r12, [r0]           // r12 = SDK_AUTOLOAD_LIST
        ldr             r3, [r0, #4]        // r3 = SDK_AUTOLOAD_LIST_END
        ldr             r1, [r0, #8]        // r1 = SDK_AUTOLOAD_START
@01_segments_loop:
        cmp             r12, r3
        bge             @02_start
        stmfd           sp!, {r3}
        /* Fixed section */
        ldr             r0, [r12], #4       // r0 = start address of destination range of fixed section
        ldr             r2, [r12], #4       // r2 = size of fixed section
        stmfd           sp!, {r0}
        bl              INITi_Copy32
        stmfd           sp!, {r0, r1}
        /* static initializer table */
        ldr             r0, [r12], #4       // r0 = start address of source range of static initializer table
#ifndef SDK_NOINIT
        stmfd           sp!, {r12}
        bl              INITi_ShelterStaticInitializer
        ldmfd           sp!, {r12}
#endif
        /* bss section */
        ldmfd           sp!, {r0}           // r0 = start address of destination range of bss section
        mov             r1, #0              // r1 = clear value for bss section
        ldr             r2, [r12], #4       // r2 = size of bss section
        bl              INITi_Fill32
        ldmfd           sp!, {r1, r2}       // r1 = end address of source range of the autoload segment
                                            // r2 = start address of destination range of fixed section
        /* Check whether there is a cache at the edit destination */
        mov             r3, #HW_ITCM_IMAGE
        cmp             r2, r3
        movge           r3, #HW_ITCM_END
        cmpge           r3, r2
        bgt             @01_next_segment    // If I-TCM autoload block, skip cache store loop
        ldr             r3, =SDK_AUTOLOAD_DTCM_START
        cmp             r2, r3
        addge           r3, r3, #HW_DTCM_SIZE
        cmpge           r3, r2
        bgt             @01_next_segment    // If D-TCM autoload block, skip cache store loop
        /* Cache store at the edit destination */
        bic             r2, r2, #(HW_CACHE_LINE_SIZE - 1)
@01_cache_store_loop:
        cmp             r2, r0
        bge             @01_next_segment
        mcr             p15, 0, r2, c7, c14, 1  // Store and invalidate D-Cache
        mcr             p15, 0, r2, c7, c5, 1   // Invalidate I-Cache
        add             r2, r2, #HW_CACHE_LINE_SIZE
        b               @01_cache_store_loop
@01_next_segment:
        ldmfd           sp!, {r3}
        b               @01_segments_loop

@02_start:
        /* Check whether running on TWL hardware */
        bl              INITi_IsRunOnTwl
        bne             @03_start

        /* Verify existence of TWL-specific autoload block */
        ldr             r1, =HW_TWL_ROM_HEADER_BUF + 0x1cc  // Extended static module ROM size for ARM9
        ldr             r0, [r1]
        cmp             r0, #0
        beq             @03_start

@02_uncompress:
        /* Decompress the TWL-specific block */
        ldr             r1, =_start_LtdModuleParams
        ldr             r0, [r1, #12]       // r0 = bottom of compressed data
        bl              MIi_UncompressBackward
@02_autoload:
        /* Autoload the TWL-specific block */
        ldr             r0, =_start_LtdModuleParams
        ldr             r12, [r0]           // r12 = SDK_LTDAUTOLOAD_LIST
        ldr             r3, [r0, #4]        // r3 = SDK_LTDAUTOLOAD_LIST_END
        ldr             r1, [r0, #8]        // r1 = SDK_LTDAUTOLOAD_START
@02_segments_loop:
        cmp             r12, r3
        bge             @03_start
        stmfd           sp!, {r3}
        /* Fixed section */
        ldr             r0, [r12], #4       // r0 = start address of destination range of fixed section
        ldr             r2, [r12], #4       // r2 = size of fixed section
        stmfd           sp!, {r0}
        bl              INITi_Copy32
        stmfd           sp!, {r0, r1}
        /* static initializer table */
        ldr             r0, [r12], #4       // r0 = start address of source range of static initializer table
#ifndef SDK_NOINIT
        stmfd           sp!, {r12}
        bl              INITi_ShelterStaticInitializer
        ldmfd           sp!, {r12}
#endif
        /* bss section */
        ldmfd           sp!, {r0}           // r0 = start address of destination range of bss section
        mov             r1, #0              // r1 = clear value for bss section
        ldr             r2, [r12], #4       // r2 = size of bss section
        bl              INITi_Fill32
        ldmfd           sp!, {r1, r2}       // r1 = end address of source range of the ltdautoload segment
                                            // r2 = start address of destination range of fixed section
        /* Check whether there is a cache at the edit destination */
        mov             r3, #HW_ITCM_IMAGE
        cmp             r2, r3
        movge           r3, #HW_ITCM_END
        cmpge           r3, r2
        bgt             @02_next_segment    // If I-TCM autoload block, skip cache store loop
        ldr             r3, =SDK_AUTOLOAD_DTCM_START
        cmp             r2, r3
        addge           r3, r3, #HW_DTCM_SIZE
        cmpge           r3, r2
        bgt             @02_next_segment    // If D-TCM autoload block, skip cache store loop
        /* Cache store at the edit destination */
        bic             r2, r2, #(HW_CACHE_LINE_SIZE - 1)
@02_cache_store_loop:
        cmp             r2, r0
        bge             @02_next_segment
        mcr             p15, 0, r2, c7, c14, 1  // Store and invalidate D-Cache
        mcr             p15, 0, r2, c7, c5, 1   // Invalidate I-Cache
        add             r2, r2, #HW_CACHE_LINE_SIZE
        b               @02_cache_store_loop
@02_next_segment
        ldmfd           sp!, {r3}
        b               @02_segments_loop

@03_start:
        /* Wait until the write buffer is empty */
        mov             r0, #0
        mcr             p15, 0, r0, c7, c10, 4

        /* Call to the autoload the completion callback function */
        ldr             r0, =_start_ModuleParams
        ldr             r1, =_start_LtdModuleParams
        ldmfd           sp!, {lr}
        b               _start_AutoloadDoneCallback
}

/*---------------------------------------------------------------------------*
  Name:         INITi_ShelterStaticInitializer
  Description:  Saves the pointer table for the static initializer in each autoload segment to the start (offset by 4 bytes) of the IRQ stack.
                
                
  Arguments:    ptr: Pointer to pointer table in the segment
                            The table must be NULL terminated.
  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef SDK_NOINIT
static asm void
INITi_ShelterStaticInitializer(u32* ptr)
{
        /* Check argument */
        cmp             r0, #0
        bxeq            lr

        /* Calculate the start address of the moved location */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE
        sub             r1, r1, #HW_DTCM_SYSRV_SIZE
        sub             r1, r1, #HW_SVC_STACK_SIZE
        ldr             r2, =SDK_IRQ_STACKSIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* Check the empty locations from the top of the moved location */
@001:   ldr             r2, [r1]
        cmp             r2, #0
        addne           r1, r1, #4
        bne             @001

        /* Copy the table to an empty location */
@002:   ldr             r2, [r0], #4
        str             r2, [r1], #4
        cmp             r2, #0
        bne             @002

        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_CallStaticInitializers
  Description:  Call the static initializer in each autoload segment.
                Call the function pointer tables moved to the top (offset by 4 bytes) of the IRQ stack by the autoload process.
                
                
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static asm void
INITi_CallStaticInitializers(void)
{
        stmdb           sp!, {lr}

        /* Calculate the start address of the new table location */
        ldr             r1, =SDK_AUTOLOAD_DTCM_START
        add             r1, r1, #HW_DTCM_SIZE
        sub             r1, r1, #HW_DTCM_SYSRV_SIZE
        sub             r1, r1, #HW_SVC_STACK_SIZE
        ldr             r2, =SDK_IRQ_STACKSIZE
        sub             r1, r1, r2
        add             r1, r1, #4

        /* Call pointers managed in tables one at a time */
@001:   ldr             r0, [r1]
        cmp             r0, #0
        beq             @002
        stmdb           sp!, {r1}
        blx             r0
        ldmia           sp!, {r1}
        /* Zero-clear pointers that were called once (because of borrowing the IRQ stack) */
        mov             r0, #0
        str             r0, [r1], #4
        b               @001

@002:
        ldmia           sp!, {lr}
        bx              lr
}
#endif

/*---------------------------------------------------------------------------*
  Name:         MIi_UncompressBackward
  Description:  Uncompresses special archive for module compression.
  Arguments:    bottom: Bottom address of packed archive + 1
                bottom[-8..-6]: Offset for top of compressed data
                                 inp_top = bottom - bottom[-8..-6]
                bottom[ -5]: Offset for bottom of compressed data
                                 inp     = bottom - bottom[-5]
                bottom[-4..-1] = offset for bottom of original data
                                 outp    = bottom + bottom[-4..-1]
                typedef struct
                {
                   u32         bufferTop:24;
                   u32         compressBottom:8;
                   u32         originalBottom;
                }   CompFooter;
  Returns:      None.
 *---------------------------------------------------------------------------*/
asm void
MIi_UncompressBackward(register void* bottom)
{
#define data            r0
#define inp_top         r1
#define outp            r2
#define inp             r3
#define outp_save       r4
#define flag            r5
#define count8          r6
#define index           r7
#define dummy           r8
#define len             r12

        cmp             bottom, #0
        beq             @exit
        stmfd           sp!,    {r4-r8}
        ldmdb           bottom, {r1-r2}
        add             outp,    bottom,  outp
        sub             inp,     bottom,  inp_top, LSR #24
        bic             inp_top, inp_top, #0xff000000
        sub             inp_top, bottom,  inp_top
        mov             outp_save, outp
@loop:
        cmp             inp, inp_top            // Exit if inp==inp_top
        ble             @end_loop
        ldrb            flag, [inp, #-1]!       // r4 = compress_flag = *--inp
        mov             count8, #8
@loop8:
        subs            count8, count8, #1
        blt             @loop
        tst             flag, #0x80
        bne             @blockcopy
@bytecopy:
        ldrb            data, [inp, #-1]!
        ldrb            dummy, [outp, #-1]      // Dummy read to use D-Cache efficiently
        strb            data, [outp, #-1]!      // Copy 1 byte
        b               @joinhere
@blockcopy:
        ldrb            len,   [inp, #-1]!
        ldrb            index, [inp, #-1]!
        orr             index, index, len, LSL #8
        bic             index, index, #0xf000
        add             index, index, #0x0002
        add             len,   len,   #0x0020
@patterncopy:
        ldrb            data,  [outp, index]
        ldrb            dummy, [outp, #-1]      // Dummy read to use D-Cache efficiently
        strb            data,  [outp, #-1]!
        subs            len,   len,   #0x0010
        bge             @patterncopy

@joinhere:
        cmp             inp, inp_top
        mov             flag, flag, LSL #1
        bgt             @loop8
@end_loop:
    
        // DC_FlushRange & IC_InvalidateRange
        mov             r0, #0
        bic             inp,  inp_top, #HW_CACHE_LINE_SIZE - 1
@cacheflush:
        mcr             p15, 0, r0, c7, c10, 4          // Wait writebuffer empty
        mcr             p15, 0, inp, c7,  c5, 1         // ICache
        mcr             p15, 0, inp, c7, c14, 1         // DCache
        add             inp, inp, #HW_CACHE_LINE_SIZE
        cmp             inp, outp_save
        blt             @cacheflush
        
        ldmfd           sp!, {r4-r8}
@exit   bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         _start_AutoloadDoneCallback
  Description:  Autoload completion callback.
  Arguments:    argv: Array retaining the autoload parameters
                    argv[0] = SDK_AUTOLOAD_LIST
                    argv[1] = SDK_AUTOLOAD_LIST_END
                    argv[2] = SDK_AUTOLOAD_START
                    argv[3] = SDK_STATIC_BSS_START
                    argv[4] = SDK_STATIC_BSS_END
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL asm void
_start_AutoloadDoneCallback(void* argv[])
{
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         NitroStartUp
  Description:  Hook for user start-up.
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL void
NitroStartUp(void)
{
}

/*---------------------------------------------------------------------------*
  Name:         OSi_ReferSymbol
  Description:  Used by SDK_REFER_SYMBOL macro to avoid deadstripping.
  Arguments:    symbol: Unused
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
OSi_ReferSymbol(void* symbol)
{
#pragma unused(symbol)
}

/*---------------------------------------------------------------------------*
  Name:         INITi_IsRunOnTwl

  Description:  Checks running platform.

                This function is used in only NITRO-TWL hybrid mode.
                (In NITRO mode and TWL limited mode, treated as constant)

  Arguments:    None.

  Returns:      TRUE: running on TWL
                FALSE: running on NITRO
 *---------------------------------------------------------------------------*/
static asm BOOL INITi_IsRunOnTwl( void )
{
		ldr             r0, =REG_A9ROM_ADDR
		ldrb            r0, [r0]
		and             r0, r0, #REG_SCFG_A9ROM_SEC_MASK | REG_SCFG_A9ROM_RSEL_MASK
		cmp             r0, #REG_SCFG_A9ROM_SEC_MASK
		moveq           r0, #TRUE
		movne           r0, #FALSE

		bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_Copy32
  Description:  Where possible, copy in 32-byte units; remainder can be copied in 4-byte units.
                The stack is consumed in 36-bytes, but registers r3 - r12 are not destroyed.
                
  Arguments:    r0: Pointer to copy destination (4-byte aligned)
                r1: Pointer to copy source (4-byte aligned)
                r2: Length to be copied in bytes (multiples of 4)
  Returns:      r0: Pointer to the copy destination after copying (r0 + r2).
                r1: Pointer to the copy source after copying (r1 + r2).
 *---------------------------------------------------------------------------*/
static asm void*
INITi_Copy32(void* dst, void* src, u32 size)
{
        stmfd           sp!, {r3-r11}
        bics            r3, r2, #0x0000001f
        beq             @next
        add             r3, r0, r3
@loop:
        ldmia           r1!, {r4-r11}
        stmia           r0!, {r4-r11}
        cmp             r3, r0
        bgt             @loop
@next:
        tst             r2, #0x00000010
        ldmneia         r1!, {r4-r7}
        stmneia         r0!, {r4-r7}
        tst             r2, #0x00000008
        ldmneia         r1!, {r4-r5}
        stmneia         r0!, {r4-r5}
        tst             r2, #0x00000004
        ldmneia         r1!, {r4}
        stmneia         r0!, {r4}
        ldmfd           sp!, {r3-r11}
        bx              lr
}

/*---------------------------------------------------------------------------*
  Name:         INITi_Fill32
  Description:  Where possible, fill the buffer with specified data in 32-byte units; the remainder can be filled in 4-byte units.
                The stack is consumed in 36-bytes, but registers r3 - r12 are not destroyed.
                
  Arguments:    r0: Pointer to the buffer (4-byte aligned)
                r1: Content to fill the buffer using 32-bit values
                r2: Length to fill the buffer in byte units (multiples of 4)
  Returns:      r0: Pointer to the buffer after processing (r0 + r2).
 *---------------------------------------------------------------------------*/
static asm void*
INITi_Fill32(void* dst, u32 value, u32 size)
{
        stmfd           sp!, {r3-r11}
        mov             r4, r1
        mov             r5, r1
        mov             r6, r1
        mov             r7, r1
        mov             r8, r1
        mov             r9, r1
        mov             r10, r1
        mov             r11, r1
        bics            r3, r2, #0x0000001f
        beq             @next
        add             r3, r0, r3
@loop:
        stmia           r0!, {r4-r11}
        cmp             r3, r0
        bgt             @loop
@next:
        tst             r2, #0x00000010
        stmneia         r0!, {r4-r7}
        tst             r2, #0x00000008
        stmneia         r0!, {r4-r5}
        tst             r2, #0x00000004
        stmneia         r0!, {r4}
        ldmfd           sp!, {r3-r11}
        bx              lr
}

#include    <twl/codereset.h>
