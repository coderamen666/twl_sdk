/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - include
  File:     math/math.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MATH_MATH_H_
#define NITRO_MATH_MATH_H_

#include <nitro/misc.h>
#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Type Definitions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Function Declarations
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Inline Function Implementation
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         MATH_ABS

  Description:  Macro that returns absolute value.
                Be careful about side effects, since each argument is subjected to multiple valuations.
  
  Arguments:    a

  Returns:      If a < 0 -a. Otherwise, a
 *---------------------------------------------------------------------------*/
#define MATH_ABS(a) ( ( (a) < 0 ) ? (-(a)) : (a) )

/*---------------------------------------------------------------------------*
  Name:         MATH_IAbs

  Description:  Inline function that returns absolute value.
                Since this is implemented as an inline function, there are no side effects.
  
  Arguments:    a

  Returns:      If a < 0 -a. Otherwise, a
 *---------------------------------------------------------------------------*/
SDK_INLINE int MATH_IAbs(int a)
{
    return (a < 0) ? -a : a;
}


/*---------------------------------------------------------------------------*
  Name:         MATH_CLAMP

  Description:  Macro that gets values in the range from "low" to "high."
                Be careful about side effects, since each argument is subjected to multiple valuations.
  
  Arguments:    x     
                low:   Maximum value
                hight: Minimum value
  
  Returns:      If x < low, low. If x > high, high. Otherwise x.
 *---------------------------------------------------------------------------*/
#define MATH_CLAMP(x, low, high) ( ( (x) > (high) ) ? (high) : ( ( (x) < (low) ) ? (low) : (x) ) )


/*---------------------------------------------------------------------------*
  Name:         MATH_MIN

  Description:  Compares two arguments and returns the smaller one.
                This is implemented as a macro, so it can be used in the type that defines the inequality operator.
                 Be careful about side effects, since each argument is subjected to multiple valuations.

  Arguments:    a, b

  Returns:      If a < b, a. Otherwise, b.
 *---------------------------------------------------------------------------*/
#define MATH_MIN(a,b) (((a) <= (b)) ? (a) : (b))


/*---------------------------------------------------------------------------*
  Name:         MATH_IMin

  Description:  Compares two int-type integer arguments and returns the smaller one.
                Since this is implemented as an inline function, there are no side effects.

  Arguments:    a, b:  int type integers

  Returns:      If a <= b, a. Otherwise, b.
 *---------------------------------------------------------------------------*/
SDK_INLINE int MATH_IMin(int a, int b)
{
    return (a <= b) ? a : b;
}

/*---------------------------------------------------------------------------*
  Name:         MATH_MAX

  Description:  Compares two arguments and returns the larger one.
                This is implemented as a macro, so it can be used in the type that defines the inequality operator.
                 Be careful about side effects, since each argument is subjected to multiple valuations.

  Arguments:    a, b

  Returns:      If a >= b, a. Otherwise, b.
 *---------------------------------------------------------------------------*/
#define MATH_MAX(a,b) (((a) >= (b)) ? (a) : (b))

/*---------------------------------------------------------------------------*
  Name:         MATH_IMax

  Description:  Compares two int-type integer arguments and returns the larger one.
                Since this is implemented as an inline function, there are no side effects.

  Arguments:    a, b:  int type integers

  Returns:      If a >= b, a. Otherwise, b.
 *---------------------------------------------------------------------------*/
SDK_INLINE int MATH_IMax(int a, int b)
{
    return (a >= b) ? a : b;
}

/*---------------------------------------------------------------------------*
  Name:         MATH_DIVUP

  Description:  This macro divides by base and takes the ceiling of the result.

  Arguments:    x:      Numeric value
                base:   Base that is a power of 2

  Returns:      A number that is the result of dividing x by base and taking the ceiling of the quotient.
 *---------------------------------------------------------------------------*/
#define MATH_DIVUP(x, base) (((x) + ((base)-1)) / (base))

/*---------------------------------------------------------------------------*
  Name:         MATH_ROUNDUP

  Description:  Macro that returns the rounded-up value (ceiling).
  
  Arguments:    x
                base:  Base of the power of 2

  Returns:      Smallest multiple of "base" that is greater than or equal to x
 *---------------------------------------------------------------------------*/
#define MATH_ROUNDUP(x, base) (((x) + ((base)-1)) & ~((base)-1))

/*---------------------------------------------------------------------------*
  Name:         MATH_ROUNDDOWN

  Description:  Macro that returns the rounded-down value (floor).
  
  Arguments:    x
                base:  Base of the power of 2

  Returns:      Largest multiple of "base" that is less than or equal to x
 *---------------------------------------------------------------------------*/
#define MATH_ROUNDDOWN(x, base) ((x) & ~((base)-1))

/*---------------------------------------------------------------------------*
  Name:         MATH_ROUNDUP32

  Description:  Macro that returns the value rounded-up to a multiple of 32.
  
  Arguments:    x

  Returns:      Smallest multiple of 32 that is greater than or equal to x
 *---------------------------------------------------------------------------*/
#define MATH_ROUNDUP32(x) MATH_ROUNDUP(x, 32)

/*---------------------------------------------------------------------------*
  Name:         MATH_ROUNDDOWN32

  Description:  Macro that returns the value rounded-down to a multiple of 32.
  
  Arguments:    x

  Returns:      Largest multiple of 32 that is less than or equal to x
 *---------------------------------------------------------------------------*/
#define MATH_ROUNDDOWN32(x) MATH_ROUNDDOWN(x, 32)

/*---------------------------------------------------------------------------*
  Name:         MATH_CountLeadingZeros

  Description:  Determines how many bits from the top have the value 0 in a binary 32-bit expression.
                In the ARM9 ARM code, this is a single command.

  Arguments:    x

  Returns:      The number of contiguous 0 bits from the top
 *---------------------------------------------------------------------------*/
u32     MATH_CountLeadingZerosFunc(u32 x);

#if !defined(PLATFORM_INTRINSIC_FUNCTION_BIT_CLZ32)

/* clz is available in ARM mode only */
#ifdef  SDK_ARM9
#if     defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__)
#pragma thumb off
SDK_INLINE u32 MATH_CountLeadingZerosInline(u32 x)
{
    asm
    {
    clz     x, x}
    return  x;
}

#pragma thumb reset
#elif   defined(SDK_ADS)
TO BE   DEFINED
#elif   defined(SDK_GCC)
TO BE   DEFINED
#endif
#endif
#endif /* PLATFORM_INTRINSIC_FUNCTION_BIT_CLZ32 */

#ifndef MATH_CountLeadingZeros
#if       defined(PLATFORM_INTRINSIC_FUNCTION_BIT_CLZ32)
#define MATH_CountLeadingZeros(x) PLATFORM_INTRINSIC_FUNCTION_BIT_CLZ32(x)
#elif     defined(SDK_ARM9) && defined(SDK_CODE_ARM)
#define MATH_CountLeadingZeros(x) MATH_CountLeadingZerosInline(x)
#else                                  // not (ARM9 && CODE_ARM)
#define MATH_CountLeadingZeros(x) MATH_CountLeadingZerosFunc(x)
#endif                                 // ARM9 && CODE_ARM
#endif

/*---------------------------------------------------------------------------*
  Name:         MATH_CLZ

  Description:  This is MATH_CountLeadingZeros under a different name.
                Determines how many bits from the top have the value 0 in a binary 32-bit expression.
                In the ARM9 ARM code, this is a single command.

  Arguments:    x

  Returns:      The number of contiguous 0 bits from the top
 *---------------------------------------------------------------------------*/
#define MATH_CLZ(x) MATH_CountLeadingZeros(x)
/*---------------------------------------------------------------------------*
  Name:         MATH_ILog2

  Description:  Gets the integer portion of the base-2 logarithm log2(x) of the u32 x argument.
                In the special case where x == 0, the function returns -1.
                In the ARM9 ARM code, this is two commands using the CLZ command.

  Arguments:    x:  u32

  Returns:      log2(x) when x > 0, or -1 when x == 0
 *---------------------------------------------------------------------------*/
        SDK_INLINE int MATH_ILog2(u32 x)
{
    return (int)(31 - MATH_CountLeadingZeros(x));
}

/*---------------------------------------------------------------------------*
  Name:         MATH_CountPopulation

  Description:  Determines the number of '1' bits in a binary 32-bit expression.

  Arguments:    x

  Returns:      The number of '1' bits in the binary expression
 *---------------------------------------------------------------------------*/
u8      MATH_CountPopulation(u32 x);


/*---------------------------------------------------------------------------*
  Name:         MATH_CountTrailingZeros

  Description:  Determines how many bits from the bottom have the value 0 in a binary 32-bit expression.
                Uses the MATH_CountLeadingZeros function.

  Arguments:    x:             u32 value used for determination

  Returns:      The number of contiguous 0 bits from the lower bit.
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 MATH_CountTrailingZeros(u32 x)
{
    return (u32)(32 - MATH_CountLeadingZeros((u32)(~x & (x - 1))));
}

/*---------------------------------------------------------------------------*
  Name:         MATH_CTZ

  Description:  This is MATH_CountTrailingZeros under a different name.
                Determines how many bits from the bottom have the value 0 in a binary 32-bit expression.
                Uses the MATH_CountLeadingZeros function.

  Arguments:    x:             u32 value used for determination

  Returns:      The number of contiguous 0 bits from the lower bit.
 *---------------------------------------------------------------------------*/
#define MATH_CTZ(x) MATH_CountTrailingZeros(x)

/*---------------------------------------------------------------------------*
  Name:         MATH_GetLeastSignificantBit

  Description:  Determines the lowest '1' bit in a binary 32bit expression.

  Arguments:    x:             u32 value used for determination

  Returns:      A u32 value expressing the lowest '1' bit.
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 MATH_GetLeastSignificantBit(u32 x)
{
    return (u32)(x & -(s32)x);
}

/*---------------------------------------------------------------------------*
  Name:         MATH_LSB

  Description:  Another name for MATH_GetLeastSignificantBit.
                Determines the lowest '1' bit in a binary 32-bit expression.

  Arguments:    x:             u32 value used for determination

  Returns:      A u32 value expressing the lowest '1' bit.
 *---------------------------------------------------------------------------*/
#define MATH_LSB(x) MATH_GetLeastSignificantBit(x)

/*---------------------------------------------------------------------------*
  Name:         MATH_GetMostSignificantBit

  Description:  Determines the highest '1' bit in a binary 32-bit expression.

  Arguments:    x:             u32 value used for determination

  Returns:      A u32 value expressing the topmost '1' bit.
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 MATH_GetMostSignificantBit(u32 x)
{
    return (u32)(x & ((s32)0x80000000 >> MATH_CountLeadingZeros(x)));
}

/*---------------------------------------------------------------------------*
  Name:         MATH_MSB

  Description:  Another name for MATH_GetMostSignificantBit.
                Determines the highest '1' bit in a binary 32-bit expression.

  Arguments:    x:             u32 value used for determination

  Returns:      A u32 value expressing the highest '1' bit.
 *---------------------------------------------------------------------------*/
#define MATH_MSB(x) MATH_GetMostSignificantBit(x)


#ifdef __cplusplus
}/* extern "C" */
#endif

/* NITRO_MATH_MATH_H_ */
#endif
