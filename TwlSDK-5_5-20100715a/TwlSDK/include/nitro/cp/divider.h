/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CP - include
  File:     divider.h

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

#ifndef NITRO_CP_DIVIDER_H_
#define NITRO_CP_DIVIDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDK_TWL
#include <nitro/ioreg.h>
#else
#include <twl/ioreg.h>
#endif

//----------------------------------------------------------------
#define CP_DIV_32_32BIT_MODE   (0UL << REG_CP_DIVCNT_MODE_SHIFT)        // 32bit/32bit mode
#define CP_DIV_64_32BIT_MODE   (1UL << REG_CP_DIVCNT_MODE_SHIFT)        // 64bit/32bit mode
#define CP_DIV_64_64BIT_MODE   (2UL << REG_CP_DIVCNT_MODE_SHIFT)        // 64bit/64bit mode


/*---------------------------------------------------------------------------*
  Name:         CP_SetDivControl

  Description:  set parameter to divider control register

  Arguments:    parameter   parameter to set control register

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CP_SetDivControl(u16 parameter)
{
    reg_CP_DIVCNT = parameter;
}

/*---------------------------------------------------------------------------*
  Name:         CP_SetDivImm32_32_NS_
                CP_SetDivImm64_32_NS_
                CP_SetDivImm64_64_NS_

  Description:  set numerator(dividend) and denominator(divisor).
                ignore setting mode.
                non thread safe.

  Arguments:    numer  : numerator (dividend)
                denom  : denominator(divisor)

  Returns:      None
 *---------------------------------------------------------------------------*/
//   32bit / 32bit
static inline void CP_SetDivImm32_32_NS_(u32 numer, u32 denom)
{
    *(REGType32 *)REG_DIV_NUMER_ADDR = numer;
    *(REGType64 *)REG_DIV_DENOM_ADDR = denom;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   64bit / 32bit
static inline void CP_SetDivImm64_32_NS_(u64 numer, u32 denom)
{
    *(REGType64 *)REG_DIV_NUMER_ADDR = numer;
    *(REGType64 *)REG_DIV_DENOM_ADDR = denom;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   64bit / 64bit
static inline void CP_SetDivImm64_64_NS_(u64 numer, u64 denom)
{
    *(REGType64 *)REG_DIV_NUMER_ADDR = numer;
    *(REGType64 *)REG_DIV_DENOM_ADDR = denom;
}

/*---------------------------------------------------------------------------*
  Name:         CP_SetDivImm32_32
                CP_SetDivImm64_32
                CP_SetDivImm64_64

  Description:  set numerator(dividend) and denominator(divisor).
                ignore setting mode.
                thread safe.

  Arguments:    numer  : numerator (dividend)
                denom  : denominator(divisor)

  Returns:      None
 *---------------------------------------------------------------------------*/
//   32bit / 32bit
static inline void CP_SetDivImm32_32(u32 numer, u32 denom)
{
    CP_SetDivImm32_32_NS_(numer, denom);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   64bit / 32bit
static inline void CP_SetDivImm64_32(u64 numer, u32 denom)
{
    CP_SetDivImm64_32_NS_(numer, denom);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   64bit / 64bit
static inline void CP_SetDivImm64_64(u64 numer, u64 denom)
{
    CP_SetDivImm64_64_NS_(numer, denom);
}

/*---------------------------------------------------------------------------*
  Name:         CP_SetDiv32_32
                CP_SetDiv64_32
                CP_SetDiv64_64

  Description:  set numerator(dividend) and denominator(divisor).
                set mode.
                thread safe.

  Arguments:    numer  : numerator (dividend)
                denom  : denominator(divisor)

  Returns:      None
 *---------------------------------------------------------------------------*/
//   32bit / 32bit
static inline void CP_SetDiv32_32(u32 numer, u32 denom)
{
    reg_CP_DIVCNT = CP_DIV_32_32BIT_MODE;
    CP_SetDivImm32_32_NS_(numer, denom);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   64bit / 32bit
static inline void CP_SetDiv64_32(u64 numer, u32 denom)
{
    reg_CP_DIVCNT = CP_DIV_64_32BIT_MODE;
    CP_SetDivImm64_32_NS_(numer, denom);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   64bit / 64bit
static inline void CP_SetDiv64_64(u64 numer, u64 denom)
{
    reg_CP_DIVCNT = CP_DIV_64_64BIT_MODE;
    CP_SetDivImm64_64_NS_(numer, denom);
}

/*---------------------------------------------------------------------------*
  Name:         CP_IsDivBusy

  Description:  check whether operation finished

  Arguments:    None

  Returns:      non-0 if finish, 0 if not
 *---------------------------------------------------------------------------*/
static inline s32 CP_IsDivBusy()
{
    return (reg_CP_DIVCNT & REG_CP_DIVCNT_BUSY_MASK);
}

/*---------------------------------------------------------------------------*
  Name:         CP_WaitDiv

  Description:  wait to finish operation

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static inline void CP_WaitDiv()
{
    while (CP_IsDivBusy())
    {
    }
}

/*---------------------------------------------------------------------------*
  Name:         CP_GetDivResultImm64
                CP_GetDivResultImm32
                CP_GetDivResultImm16
                CP_GetDivResultImm8

  Description:  get result(quotient) of division.
                don't check whether operation finish

  Arguments:    None

  Returns:      result(quotient) of division
 *---------------------------------------------------------------------------*/
//   as 64bit data
static inline s64 CP_GetDivResultImm64()
{
    return (s64)(*(REGType64 *)REG_DIV_RESULT_ADDR);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 32bit data
static inline s32 CP_GetDivResultImm32()
{
    return (s32)(*(REGType32 *)REG_DIV_RESULT_ADDR);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 16bit data
static inline s16 CP_GetDivResultImm16()
{
    return (s16)(*(REGType16 *)REG_DIV_RESULT_ADDR);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 8bit data
static inline s8 CP_GetDivResultImm8()
{
    return (s8)(*(REGType8 *)REG_DIV_RESULT_ADDR);
}

/*---------------------------------------------------------------------------*
  Name:         CP_GetDivResult64
                CP_GetDivResult32
                CP_GetDivResult16
                CP_GetDivResult8

  Description:  get result(quotient) of division.
                wait till operation finish

  Arguments:    None

  Returns:      result(quotient) of division
 *---------------------------------------------------------------------------*/
//   as 64bit data
static inline s64 CP_GetDivResult64()
{
    CP_WaitDiv();
    return CP_GetDivResultImm64();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 32bit data
static inline s32 CP_GetDivResult32()
{
    CP_WaitDiv();
    return CP_GetDivResultImm32();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 16bit data
static inline s16 CP_GetDivResult16()
{
    CP_WaitDiv();
    return CP_GetDivResultImm16();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 8bit data
static inline s8 CP_GetDivResult8()
{
    CP_WaitDiv();
    return CP_GetDivResultImm8();
}

/*---------------------------------------------------------------------------*
  Name:         CP_GetDivRemainderImm64
                CP_GetDivRemainderImm32
                CP_GetDivRemainderImm16
                CP_GetDivRemainderImm8

  Description:  get remainder of division.
                don't check whether operation finish

  Arguments:    None

  Returns:      remainder of division
 *---------------------------------------------------------------------------*/
//   as 64bit data
static inline s64 CP_GetDivRemainderImm64()
{
    return (s64)(*(REGType64 *)REG_DIVREM_RESULT_ADDR);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 32bit data
static inline s32 CP_GetDivRemainderImm32()
{
    return (s32)(*(REGType32 *)REG_DIVREM_RESULT_ADDR);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 16bit data
static inline s16 CP_GetDivRemainderImm16()
{
    return (s16)(*(REGType16 *)REG_DIVREM_RESULT_ADDR);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 8bit data
static inline s8 CP_GetDivRemainderImm8()
{
    return (s8)(*(REGType8 *)REG_DIVREM_RESULT_ADDR);
}

/*---------------------------------------------------------------------------*
  Name:         CP_GetDivRemainder64
                CP_GetDivRemainder32
                CP_GetDivRemainder16
                CP_GetDivRemainder8

  Description:  get remainder of division.
                wait till operation finish

  Arguments:    None

  Returns:      remainder of division
 *---------------------------------------------------------------------------*/
//   as 64bit data
static inline s64 CP_GetDivRemainder64()
{
    CP_WaitDiv();
    return CP_GetDivRemainderImm64();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 32bit data
static inline s32 CP_GetDivRemainder32()
{
    CP_WaitDiv();
    return CP_GetDivRemainderImm32();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 16bit data
static inline s16 CP_GetDivRemainder16()
{
    CP_WaitDiv();
    return CP_GetDivRemainderImm16();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
//   as 8bit data
static inline s8 CP_GetDivRemainder8()
{
    CP_WaitDiv();
    return CP_GetDivRemainderImm8();
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_CP_DIVIDER_H_ */
#endif
