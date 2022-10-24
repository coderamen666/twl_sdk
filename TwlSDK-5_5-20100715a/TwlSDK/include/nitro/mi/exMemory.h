/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     exMemory.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_MI_EXMEMORY_H_
#define NITRO_MI_EXMEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/ioreg.h>
#else
#include <twl/ioreg.h>
#endif

#include  <nitro/types.h>
#include  <nitro/misc.h>

//----------------------------------------------------------------
// alias for ARM7

#ifdef SDK_ARM7
#define reg_MI_EXMEMCNT              reg_MI_EXMEMCNT_L

#define REG_MI_EXMEMCNT_EP_SHIFT     REG_MI_EXMEMCNT_L_EP_SHIFT
#define REG_MI_EXMEMCNT_EP_SIZE      REG_MI_EXMEMCNT_L_EP_SIZE
#define REG_MI_EXMEMCNT_EP_MASK      REG_MI_EXMEMCNT_L_EP_MASK

#define REG_MI_EXMEMCNT_MP_SHIFT     REG_MI_EXMEMCNT_L_MP_SHIFT
#define REG_MI_EXMEMCNT_MP_SIZE      REG_MI_EXMEMCNT_L_MP_SIZE
#define REG_MI_EXMEMCNT_MP_MASK      REG_MI_EXMEMCNT_L_MP_MASK

#define REG_MI_EXMEMCNT_CP_SHIFT     REG_MI_EXMEMCNT_L_CP_SHIFT
#define REG_MI_EXMEMCNT_CP_SIZE      REG_MI_EXMEMCNT_L_CP_SIZE
#define REG_MI_EXMEMCNT_CP_MASK      REG_MI_EXMEMCNT_L_CP_MASK

#define REG_MI_EXMEMCNT_PHI_SHIFT    REG_MI_EXMEMCNT_L_PHI_SHIFT
#define REG_MI_EXMEMCNT_PHI_SIZE     REG_MI_EXMEMCNT_L_PHI_SIZE
#define REG_MI_EXMEMCNT_PHI_MASK     REG_MI_EXMEMCNT_L_PHI_MASK

#define REG_MI_EXMEMCNT_ROM2nd_SHIFT REG_MI_EXMEMCNT_L_ROM2nd_SHIFT
#define REG_MI_EXMEMCNT_ROM2nd_SIZE  REG_MI_EXMEMCNT_L_ROM2nd_SIZE
#define REG_MI_EXMEMCNT_ROM2nd_MASK  REG_MI_EXMEMCNT_L_ROM2nd_MASK

#define REG_MI_EXMEMCNT_ROM1st_SHIFT REG_MI_EXMEMCNT_L_ROM1st_SHIFT
#define REG_MI_EXMEMCNT_ROM1st_SIZE  REG_MI_EXMEMCNT_L_ROM1st_SIZE
#define REG_MI_EXMEMCNT_ROM1st_MASK  REG_MI_EXMEMCNT_L_ROM1st_MASK

#define REG_MI_EXMEMCNT_RAM_SHIFT    REG_MI_EXMEMCNT_L_RAM_SHIFT
#define REG_MI_EXMEMCNT_RAM_SIZE     REG_MI_EXMEMCNT_L_RAM_SIZE
#define REG_MI_EXMEMCNT_RAM_MASK     REG_MI_EXMEMCNT_L_RAM_MASK
#endif // SDK_ARM7

//----------------------------------------------------------------
//---- Access priority 
typedef enum
{
    MI_PROCESSOR_ARM9 = 0,             // Main processor
    MI_PROCESSOR_ARM7 = 1              // Sub processor
}
MIProcessor;

//---- PHI output control
typedef enum
{
    MIi_PHI_CLOCK_LOW = 0,             // Low level
    MIi_PHI_CLOCK_4MHZ = 1,            //  4.19 MHz
    MIi_PHI_CLOCK_8MHZ = 2,            //  8.38 MHz
    MIi_PHI_CLOCK_16MHZ = 3            // 16.76 MHz
}
MIiPhiClock;

//---- cartridge ROM 1st access cycle
typedef enum
{
    MI_CTRDG_ROMCYCLE1_10 = 0,         // 10 cycle
    MI_CTRDG_ROMCYCLE1_8 = 1,          //  8 cycle
    MI_CTRDG_ROMCYCLE1_6 = 2,          //  6 cycle
    MI_CTRDG_ROMCYCLE1_18 = 3          // 18 cycle
}
MICartridgeRomCycle1st;

//---- cartridge ROM 2nd access cycle
typedef enum
{
    MI_CTRDG_ROMCYCLE2_6 = 0,          // 6 cycle
    MI_CTRDG_ROMCYCLE2_4 = 1           // 4 cycle
}
MICartridgeRomCycle2nd;

//---- cartridge RAM access cycle
typedef enum
{
    MI_CTRDG_RAMCYCLE_10 = 0,          // 10 cycle
    MI_CTRDG_RAMCYCLE_8 = 1,           //  8 cycle
    MI_CTRDG_RAMCYCLE_6 = 2,           //  6 cycle
    MI_CTRDG_RAMCYCLE_18 = 3           // 18 cycle
}
MICartridgeRamCycle;

//---- Wireless System (Wait State 0) control 1st wait (for ARM7)
typedef enum
{
    MI_WMWAIT0_1_9 = 0,                //  9 wait
    MI_WMWAIT0_1_7 = 1,                //  7 wait
    MI_WMWAIT0_1_5 = 2,                //  5 wait
    MI_WMWAIT0_1_17 = 3                // 17 wait
}
MIWMWait0_1;

//---- Wireless System (Wait State 0) control 2nd wait (for ARM7)
typedef enum
{
    MI_WMWAIT0_2_5 = 0,                //  5 wait
    MI_WMWAIT0_2_3 = 1                 //  3 wait
}
MIWMWait0_2;

//---- Wireless System (Wait State 1) control 1st wait (for ARM7)
typedef enum
{
    MI_WMWAIT1_1_9 = 0,                //  9 wait
    MI_WMWAIT1_1_7 = 1,                //  7 wait
    MI_WMWAIT1_1_5 = 2,                //  5 wait
    MI_WMWAIT1_1_17 = 3                // 17 wait
}
MIWMWait1_1;

//---- Wireless System (Wait State 1) control 2nd wait (for ARM7)
typedef enum
{
    MI_WMWAIT1_2_9 = 0,                //  9 wait
    MI_WMWAIT1_2_3 = 1                 //  3 wait
}
MIWMWait1_2;

//---- PHI Control (for ARM7)
typedef enum
{
    MI_PHI_CLOCK_CARTRIDGE = 0,        //  conform to cartridge setting
    MI_PHI_CLOCK_33MHZ = 1             //  33.51 MHz output from PHI (default)
}
MIPhiControl;


//----------------------------------------------------------------
//      assert definition
//
#define MI_PROCESSOR_ASSERT( x )   SDK_ASSERT( (u32)x <= MI_PROCESSOR_ARM7 )
#define MI_PHICLOCK_ASSERT( x )    SDK_ASSERT( (u32)x <= MIi_PHI_CLOCK_16MHZ )


#define MI_ROMCYCLE1_ASSERT( x )   SDK_ASSERT( (u32)x <= MI_CTRDG_ROMCYCLE1_18 )
#define MI_ROMCYCLE2_ASSERT( x )   SDK_ASSERT( (u32)x <= MI_CTRDG_ROMCYCLE2_4 )
#define MI_RAMCYCLE_ASSERT( x )    SDK_ASSERT( (u32)x <= MI_CTRDG_RAMCYCLE_18 )


#define MI_WMWAIT0_1_ASSERT( x )   SDK_ASSERT( (u32)x <= MI_WMWAIT0_1_17 )
#define MI_WMWAIT0_2_ASSERT( x )   SDK_ASSERT( (u32)x <= MI_WMWAIT0_2_3 )
#define MI_WMWAIT1_1_ASSERT( x )   SDK_ASSERT( (u32)x <= MI_WMWAIT1_1_17 )
#define MI_WMWAIT1_2_ASSERT( x )   SDK_ASSERT( (u32)x <= MI_WMWAIT1_2_3 )

#define MI_PHICONTROL_ASSERT( x )  SDK_ASSERT( (u32)x <= MI_PHI_CLOCK_33MHZ )

//================================================================
//              priority setting
//================================================================
//----------------
//      set/get of access priority : main memory
#ifdef SDK_ARM9
static inline void MI_SetMainMemoryPriority(MIProcessor proc)
{
    MI_PROCESSOR_ASSERT(proc);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_EP_MASK) | (proc << REG_MI_EXMEMCNT_EP_SHIFT));
}
#endif

static inline MIProcessor MI_GetMainMemoryPriority(void)
{
    return (MIProcessor)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_EP_MASK) >> REG_MI_EXMEMCNT_EP_SHIFT);
}

//----------------
//      set/get of access priority : NITRO card
#ifdef SDK_ARM9
static inline void MIi_SetCardProcessor(MIProcessor proc)
{
    MI_PROCESSOR_ASSERT(proc);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_MP_MASK) | (proc << REG_MI_EXMEMCNT_MP_SHIFT));
}
#endif

static inline MIProcessor MI_GetCardProcessor(void)
{
    return (MIProcessor)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_MP_MASK) >> REG_MI_EXMEMCNT_MP_SHIFT);
}

//----------------
//      set/get of access priority : Cartridge
#ifdef SDK_ARM9
static inline void MIi_SetCartridgeProcessor(MIProcessor proc)
{
    MI_PROCESSOR_ASSERT(proc);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_CP_MASK) | (proc << REG_MI_EXMEMCNT_CP_SHIFT));
}
#endif

static inline MIProcessor MI_GetCartridgeProcessor(void)
{
    return (MIProcessor)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_CP_MASK) >> REG_MI_EXMEMCNT_CP_SHIFT);
}

//================================================================
//              speed and control setting
//================================================================
/*---------------------------------------------------------------------------*
  Name:         MIi_SetPhiClock

  Description:  set PHI output clock

  Arguments:    clock :    Phi clock to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MIi_SetPhiClock(MIiPhiClock clock)
{
    MI_PHICLOCK_ASSERT(clock);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_PHI_MASK) | (clock << REG_MI_EXMEMCNT_PHI_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MIi_GetPhiClock

  Description:  get PHI output clock

  Arguments:    None

  Returns:      Phi clock
 *---------------------------------------------------------------------------*/
static inline MIiPhiClock MIi_GetPhiClock(void)
{
    return (MIiPhiClock) ((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_PHI_MASK) >>
                          REG_MI_EXMEMCNT_PHI_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SetCartridgeRomCycle1st

  Description:  set Cartridge ROM 1st access cycle

  Arguments:    c1 :       access cycle to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetCartridgeRomCycle1st(MICartridgeRomCycle1st c1)
{
    MI_ROMCYCLE1_ASSERT(c1);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_ROM1st_MASK) |
              (c1 << REG_MI_EXMEMCNT_ROM1st_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetCartridgeRomCycle1st

  Description:  get Cartridge ROM 1st access cycle

  Arguments:    None

  Returns:      cartridge ROM 1st access cycle
 *---------------------------------------------------------------------------*/
static inline MICartridgeRomCycle1st MI_GetCartridgeRomCycle1st(void)
{
    return (MICartridgeRomCycle1st)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_ROM1st_MASK) >>
                                    REG_MI_EXMEMCNT_ROM1st_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SetCartridgeRomCycle2nd

  Description:  set Cartridge ROM 2nd access cycle

  Arguments:    c2 :       access cycle to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetCartridgeRomCycle2nd(MICartridgeRomCycle2nd c2)
{
    MI_ROMCYCLE2_ASSERT(c2);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_ROM2nd_MASK) |
              (c2 << REG_MI_EXMEMCNT_ROM2nd_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetCartridgeRomCycle2nd

  Description:  get Cartridge ROM 2nd access cycle

  Arguments:    None

  Returns:      cartridge ROM 2nd access cycle
 *---------------------------------------------------------------------------*/
static inline MICartridgeRomCycle2nd MI_GetCartridgeRomCycle2nd(void)
{
    return (MICartridgeRomCycle2nd)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_ROM2nd_MASK) >>
                                    REG_MI_EXMEMCNT_ROM2nd_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SetAgbCartridgeFastestRomCycle

  Description:  set Cartridge ROM access cycles for AGB. (8-4 cycle)

  Arguments:    prev1st : pointer to be store the previous 1st ROM access cycle.
                          if NULL, no effect.
                prev2nd : pointer to be store the previous 2nd ROM access cycle.
                          if NULL, no effect.

  Returns:      None
 *---------------------------------------------------------------------------*/
void    MI_SetAgbCartridgeFastestRomCycle(MICartridgeRomCycle1st *prev1st,
                                          MICartridgeRomCycle2nd *prev2nd);

/*---------------------------------------------------------------------------*
  Name:         MI_SetCartridgeRamCycle

  Description:  set Cartridge RAM access cycle

  Arguments:    c :       access cycle to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetCartridgeRamCycle(MICartridgeRamCycle c)
{
    MI_RAMCYCLE_ASSERT(c);
    reg_MI_EXMEMCNT =
        (u16)((reg_MI_EXMEMCNT & ~REG_MI_EXMEMCNT_RAM_MASK) | (c << REG_MI_EXMEMCNT_RAM_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetCartridgeRamCycle

  Description:  get Cartridge RAM access cycle

  Arguments:    None

  Returns:      cartridge RAM access cycle
 *---------------------------------------------------------------------------*/
static inline MICartridgeRamCycle MI_GetCartridgeRamCycle(void)
{
    return (MICartridgeRamCycle)((reg_MI_EXMEMCNT & REG_MI_EXMEMCNT_RAM_MASK) >>
                                 REG_MI_EXMEMCNT_RAM_SHIFT);
}


//================================================================
//              speed and control setting (for ARM7)
//================================================================
#ifdef SDK_ARM7
/*---------------------------------------------------------------------------*
  Name:         MI_SetWM0Wait1st

  Description:  set 1st wait to Wireless system area at wait state 0 
                (this function is for ARM7)

  Arguments:    wait :       1st wait to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetWM0Wait1st(MIWMWait0_1 wait)
{
    MI_WMWAIT0_1_ASSERT(wait);
    reg_MI_EXMEMCNT_H =
        (u16)((reg_MI_EXMEMCNT_H & ~REG_MI_EXMEMCNT_H_WW0_1st_MASK) |
              (wait << REG_MI_EXMEMCNT_H_WW0_1st_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetWM0Wait1st

  Description:  get 1st wait to Wireless system area at wait state 0 
                (this function is for ARM7)

  Arguments:    None

  Returns:      1st wait to wireless system area at wait state 0
 *---------------------------------------------------------------------------*/
static inline MIWMWait0_1 MI_GetWM0Wait1st(void)
{
    return (MIWMWait0_1)((reg_MI_EXMEMCNT_H & REG_MI_EXMEMCNT_H_WW0_1st_MASK) >>
                         REG_MI_EXMEMCNT_H_WW0_1st_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SetWM0Wait2nd

  Description:  set 2nd wait to Wireless system area at wait state 0
                (this function is for ARM7)

  Arguments:    wait :       2nd wait to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetWM0Wait2nd(MIWMWait0_2 wait)
{
    MI_WMWAIT0_2_ASSERT(wait);
    reg_MI_EXMEMCNT_H =
        (u16)((reg_MI_EXMEMCNT_H & ~REG_MI_EXMEMCNT_H_WW0_2nd_MASK) |
              (wait << REG_MI_EXMEMCNT_H_WW0_2nd_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetWM0Wait2nd

  Description:  get 2nd wait to Wireless system area at wait state 0 
                (this function is for ARM7)

  Arguments:    None

  Returns:      2nd wait to wireless system area at wait state 0
 *---------------------------------------------------------------------------*/
static inline MIWMWait0_2 MI_GetWM0Wait2nd(void)
{
    return (MIWMWait0_2)((reg_MI_EXMEMCNT_H & REG_MI_EXMEMCNT_H_WW0_2nd_MASK) >>
                         REG_MI_EXMEMCNT_H_WW0_2nd_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SetWM1Wait1st

  Description:  set 1st wait to Wireless system area at wait state 1
                (this function is for ARM7)

  Arguments:    wait :       1st wait to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetWM1Wait1st(MIWMWait1_1 wait)
{
    MI_WMWAIT1_1_ASSERT(wait);
    reg_MI_EXMEMCNT_H =
        (u16)((reg_MI_EXMEMCNT_H & ~REG_MI_EXMEMCNT_H_WW1_1st_MASK) |
              (wait << REG_MI_EXMEMCNT_H_WW1_1st_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetWM1Wait1st

  Description:  get 1st wait to Wireless system area at wait state 1
                (this function is for ARM7)

  Arguments:    None

  Returns:      1st wait to wireless system area at wait state 1
 *---------------------------------------------------------------------------*/
static inline MIWMWait1_1 MI_GetWM1Wait1st(void)
{
    return (MIWMWait1_1)((reg_MI_EXMEMCNT_H & REG_MI_EXMEMCNT_H_WW1_1st_MASK) >>
                         REG_MI_EXMEMCNT_H_WW1_1st_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SetWM1Wait2nd

  Description:  set 2nd wait to Wireless system area at wait state 1
                (this function is for ARM7)

  Arguments:    wait :       2nd wait to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetWM1Wait2nd(MIWMWait1_2 wait)
{
    MI_WMWAIT1_2_ASSERT(wait);
    reg_MI_EXMEMCNT_H =
        (u16)((reg_MI_EXMEMCNT_H & ~REG_MI_EXMEMCNT_H_WW1_2nd_MASK) |
              (wait << REG_MI_EXMEMCNT_H_WW0_2nd_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetWM1Wait2nd

  Description:  get 2nd wait to Wireless system area at wait state 1
                (this function is for ARM7)

  Arguments:    None

  Returns:      2nd wait to wireless system area at wait state 1
 *---------------------------------------------------------------------------*/
static inline MIWMWait1_2 MI_GetWM1Wait2nd(void)
{
    return (MIWMWait1_2)((reg_MI_EXMEMCNT_H & REG_MI_EXMEMCNT_H_WW1_2nd_MASK) >>
                         REG_MI_EXMEMCNT_H_WW1_2nd_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         MI_SetPhiControl

  Description:  set PHI control
                (this function is for ARM7)

  Arguments:    control :   PHI setting

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void MI_SetPhiControl(MIPhiControl control)
{
    MI_PHICONTROL_ASSERT(control);
    reg_MI_EXMEMCNT_H =
        (u16)((reg_MI_EXMEMCNT_H & ~REG_MI_EXMEMCNT_H_PHI33M_MASK) |
              (control << REG_MI_EXMEMCNT_H_PHI33M_SHIFT));
}

/*---------------------------------------------------------------------------*
  Name:         MI_GetPhiControl

  Description:  get PHI control
                (this function is for ARM7)

  Arguments:    None

  Returns:      PHI control setting
 *---------------------------------------------------------------------------*/
static inline MIPhiControl MI_GetPhiControl(void)
{
    return (MIPhiControl)((reg_MI_EXMEMCNT_H & REG_MI_EXMEMCNT_H_PHI33M_MASK) >>
                          REG_MI_EXMEMCNT_H_PHI33M_SHIFT);
}

#endif // ifdef SDK_ARM7



#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_EXMEMORY_H_ */
#endif
