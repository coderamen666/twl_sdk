/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CP - include
  File:     sqrt.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_CP_SQRT_H_
#define NITRO_CP_SQRT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/cp/context.h>

//---- sqrt calculation mode
#define CP_SQRT_32BIT_MODE         (0UL << REG_CP_SQRTCNT_MODE_SHIFT)   // 32bit mode
#define CP_SQRT_64BIT_MODE         (1UL << REG_CP_SQRTCNT_MODE_SHIFT)   // 64bit mode


//================================================================================
//            sqrt control register setting
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CP_SetSqrtControl

  Description:  set value to sqrt control register

  Arguments:    param : value to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CP_SetSqrtControl(u16 param)
{
    reg_CP_SQRTCNT = param;
}


//================================================================================
//            sqrt parameter setting
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CP_SetSqrtImm64_NS_
                CP_SetSqrtImm32_NS_

  Description:  set value to sqrt parameter.
                not set bit-mode.
                not thread safe.

  Arguments:    param : parameter to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CP_SetSqrtImm64_NS_(u64 param)
{
    *((REGType64 *)REG_SQRT_PARAM_ADDR) = param;
}
static inline void CP_SetSqrtImm32_NS_(u32 param)
{
    *((REGType32 *)REG_SQRT_PARAM_ADDR) = param;
}

/*---------------------------------------------------------------------------*
  Name:         CP_SetSqrtImm64
                CP_SetSqrtImm32

  Description:  set value to sqrt parameter.
                not set bit-mode.

  Arguments:    param : parameter to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CP_SetSqrtImm64(u64 param)
{
    *((REGType64 *)REG_SQRT_PARAM_ADDR) = param;
}

static inline void CP_SetSqrtImm32(u32 param)
{
    *((REGType32 *)REG_SQRT_PARAM_ADDR) = param;
}

/*---------------------------------------------------------------------------*
  Name:         CP_SetSqrt64
                CP_SetSqrt32

  Description:  set value to sqrt parameter.
                set bit-mode.

  Arguments:    param : parameter to set

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CP_SetSqrt64(u64 param)
{
    reg_CP_SQRTCNT = CP_SQRT_64BIT_MODE;
    CP_SetSqrtImm64_NS_(param);
}

static inline void CP_SetSqrt32(u32 param)
{
    reg_CP_SQRTCNT = CP_SQRT_32BIT_MODE;
    CP_SetSqrtImm32_NS_(param);
}


//================================================================================
//            wait to finish
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CP_IsSqrtBusy

  Description:  check sqrt calculation finished

  Arguments:    None.

  Returns:      non-0 if busy, 0 if finish
 *---------------------------------------------------------------------------*/
static inline s32 CP_IsSqrtBusy(void)
{
    return (reg_CP_SQRTCNT & REG_CP_SQRTCNT_BUSY_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         CP_WaitSqrt

  Description:  wait till sqrt calculation finish

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void CP_WaitSqrt(void)
{
    while (CP_IsSqrtBusy())
    {
    }
}

//================================================================================
//            getting result
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         CP_GetSqrtResultImm8 / 16 / 32

  Description:  read result.
                not make sure to finish.

  Arguments:    None.

  Returns:      result
 *---------------------------------------------------------------------------*/
static inline u32 CP_GetSqrtResultImm32(void)
{
    return (u32)(*((REGType32 *)REG_SQRT_RESULT_ADDR));
}
static inline u16 CP_GetSqrtResultImm16(void)
{
    return (u16)(*((REGType16 *)REG_SQRT_RESULT_ADDR));
}
static inline u8 CP_GetSqrtResultImm8(void)
{
    return (u8)(*((REGType8 *)REG_SQRT_RESULT_ADDR));
}

/*---------------------------------------------------------------------------*
  Name:         CP_GetSqrtResult8 / 16 / 32

  Description:  wait till finish, and read result.

  Arguments:    None.

  Returns:      result
 *---------------------------------------------------------------------------*/
static inline u32 CP_GetSqrtResult32(void)
{
    CP_WaitSqrt();
    return CP_GetSqrtResultImm32();
}
static inline u16 CP_GetSqrtResult16(void)
{
    CP_WaitSqrt();
    return CP_GetSqrtResultImm16();
}
static inline u8 CP_GetSqrtResult8(void)
{
    CP_WaitSqrt();
    return CP_GetSqrtResultImm8();
}

#ifdef __cplusplus
} /* extern "C" */
#endif


/* NITRO_CP_SQRT_H_ */
#endif
