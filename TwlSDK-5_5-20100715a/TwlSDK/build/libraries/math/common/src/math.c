/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MATH - 
  File:     math.c

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author:$
 *---------------------------------------------------------------------------*/

#include <nitro/math.h>

/*---------------------------------------------------------------------------*
  Name:         MATH_CountLeadingZerosFunc

  Description:  Determines how many high-order bits in a 32-bit binary expression have a value of 0.
                (For ARM9 Thumb or ARM7 which do not have CLZ command)

  Arguments:    x

  Returns:      The number of contiguous 0 bits from the top
 *---------------------------------------------------------------------------*/
#if defined(SDK_ARM9) && (defined(SDK_CW) || defined(SDK_RX) || defined(__MWERKS__))

#pragma thumb off
u32 MATH_CountLeadingZerosFunc(u32 x)
{
    // Even if the call source on ARM9 was Thumb, switching to ARM mode and calling a CLZ command is better for speed and size
    // 
    // 
    asm
    {
    clz     x, x}
    return  x;
}
#pragma thumb reset

#else // !ARM9 || !(CW || __MWERKS__)

u32 MATH_CountLeadingZerosFunc(u32 x)
{
    u32     y;
    u32     n = 32;

    // Use binary search to find the location where 0's end.
    y = x >> 16;
    if (y != 0)
    {
        n -= 16;
        x = y;
    }
    y = x >> 8;
    if (y != 0)
    {
        n -= 8;
        x = y;
    }
    y = x >> 4;
    if (y != 0)
    {
        n -= 4;
        x = y;
    }
    y = x >> 2;
    if (y != 0)
    {
        n -= 2;
        x = y;
    }
    y = x >> 1;
    if (y != 0)
    {
        n -= 2;
    }                                  // x == 0b10 or 0b11 -> n -= 2
    else
    {
        n -= x;
    }                                  // x == 0b00 or 0b01 -> n -= x

    return n;
}

#endif // ARM9 && (CW || __MWERKS__)

/*---------------------------------------------------------------------------*
  Name:         MATH_CountPopulation

  Description:  Determines the number of '1' bits in a binary 32-bit expression.

  Arguments:    x

  Returns:      The number of '1' bits in the binary expression
 *---------------------------------------------------------------------------*/
u8 MATH_CountPopulation(u32 x)
{
    // Note that in the ARM code below, shift and arithmetic operations can be done simultaneously.
    // With the Release Build and over, it will be 13 commands without stalls + bx lr.

    // Rather than counting 32 bits directly, first store the number of 1's for every 2 bits in the same location as those 2 bits.
    //  
    // In other words, every 2 bits are converted such that 00 -> 00, 01 -> 01, 10 -> 01, and 11 -> 10.
    // When x -> x', for a 2-bit value we have x' = x - (x >> 1).
    x -= ((x >> 1) & 0x55555555);

    // When counting in 4-bit units, add the number of 1's stored in the upper and lower 2 bits, and then store this as the number of 1's in that original 4-bit location.
    //  
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);

    // Do the same for 8-bit units.
    // However, the maximum result for each digit is 8, and this fits in 4 bits, it is not necessary to mask ahead of time.
    //  
    x += (x >> 4);

    // Mask unnecessary parts in preparation for the next operations.
    x &= 0x0f0f0f0f;

    // Get the sum of the upper 8 bits and lower 8 bits in 16-bit units.
    x += (x >> 8);

    // Do the same for 32-bit units.
    x += (x >> 16);

    // The lower 8-bit value is the result.
    return (u8)x;
}

#if 0
// Reference implementation in assembler.
// This is faster when the number of unset bits ('0') is greater than the number of set bits ('1').
/*---------------------------------------------------------------------------*
  Name:         MATH_CountPopulation_Asm

  Description:  Counts bits that are set to 1 when a value is expressed as 32 bits.

  Arguments:    value   -   Value to examine.

  Returns:      u32     -   Returns the number of bits. An integer between 0 and 32.
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
asm u32 MATH_CountPopulation_Asm(u32 value)
{
    mov     r1 ,    #0
    mov     r2 ,    #1

@loop:
    clz     r3 ,    r0
    rsbs    r3 ,    r3 ,    #31
    bcc     @end
    add     r1 ,    r1 ,    #1
    mvn     r3 ,    r2 ,    LSL r3
    and     r0 ,    r0 ,    r3
    b       @loop

@end:
    mov     r0 ,    r1
    bx      lr
}
#include <nitro/codereset.h>
#endif
