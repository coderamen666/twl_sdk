/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - 
  File:     fx_cp.c

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

#include <nitro/fx/fx_cp.h>
#include <nitro/cp/divider.h>
#include <nitro/cp/sqrt.h>

#define FX_DIV_SHIFT (32 - FX32_SHIFT) // 20
#define FX_DIV_1_2   (1 << (FX_DIV_SHIFT - 1))

#define FX_SQRT_SHIFT ((32 + FX32_SHIFT) / 2 - FX32_SHIFT)      // 10
#define FX_SQRT_1_2   (1 << (FX_SQRT_SHIFT - 1))

#define FX_INVSQRT_SHIFT \
    (FX64C_SHIFT + ((32 + FX32_SHIFT) / 2 - FX32_SHIFT))        // 42
#define FX_INVSQRT_1_2   (1LL << (FX_INVSQRT_SHIFT - 1))


/*---------------------------------------------------------------------------*
  Name:         FX_Div

  Description:  Divides 'numer' by 'denom', and returns the result in fx32
                format. This uses the divider.

  Arguments:    numer        a value in fx32 format
                denom        a value in fx32 format

  Returns:      result in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_Div(fx32 numer, fx32 denom)
{
    FX_DivAsync(numer, denom);
    return FX_GetDivResult();
}


/*---------------------------------------------------------------------------*
  Name:         FX_DivFx64c

  Description:  Divides 'numer' by 'denom', and returns the result in fx64c
                format. This uses the divider.

  Arguments:    numer        a value in fx32 format
                denom        a value in fx32 format

  Returns:      result in fx64c format
 *---------------------------------------------------------------------------*/
fx64c FX_DivFx64c(fx32 numer, fx32 denom)
{
    FX_DivAsync(numer, denom);
    return (fx64c)CP_GetDivResult64();
}

/*---------------------------------------------------------------------------*
  Name:         FX_Inv

  Description:  Returns an inverse number of 'denom'. This uses the divider.

  Arguments:    denom        a value in fx32 format

  Returns:      result in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_Inv(fx32 denom)
{
    FX_InvAsync(denom);
    return FX_GetDivResult();
}


/*---------------------------------------------------------------------------*
  Name:         FX_InvFx64c

  Description:  Returns an inverse number of 'denom'. This uses the divider.

  Arguments:    denom        a value in fx32 format

  Returns:      result in fx64c format
 *---------------------------------------------------------------------------*/
fx64c FX_InvFx64c(fx32 denom)
{
    FX_InvAsync(denom);
    return (fx64c)CP_GetDivResult64();
}


/*---------------------------------------------------------------------------*
  Name:         FX_Sqrt

  Description:  Returns a square root of 'x'. This uses the sqrt unit.

  Arguments:    x            a value in fx32 format

  Returns:      result in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_Sqrt(fx32 x)
{
    SDK_ASSERT(!CP_IsSqrtBusy());
    if (x > 0)
    {
        CP_SetSqrt64((u64)x << 32);
        return FX_GetSqrtResult();
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         FX_InvSqrt

  Description:  Returns an inverse of a square root of 'x'. This uses
                the divider and the sqrt unit.

  Arguments:    x            a value in fx32 format

  Returns:      result in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_InvSqrt(fx32 x)
{
    if (x > 0)
    {
        fx64c   inv_x;
        s64     sqrt_x;
        s64     tmp;

        FX_InvAsync(x);
        FX_SqrtAsync(x);

        inv_x = FX_GetInvResultFx64c();
        sqrt_x = CP_GetSqrtResult32();
        tmp = inv_x * sqrt_x;
        return (fx32)((tmp + FX_INVSQRT_1_2) >> FX_INVSQRT_SHIFT);
    }
    else
    {
        return 0;
    }
}


/*---------------------------------------------------------------------------*
  Name:         FX_GetDivResultFx64c

  Description:  Get the result of division in fx64c format.
                This function must be called after FX_DivAsync/FX_DivAsyncImm is executed.

  Arguments:    none

  Returns:      the result of division in fx64c format
 *---------------------------------------------------------------------------*/
fx64c FX_GetDivResultFx64c(void)
{
    return (fx64c)CP_GetDivResult64();
}


/*---------------------------------------------------------------------------*
  Name:         FX_GetDivResult

  Description:  Get the result of division in fx32 format.
                This function must be called after FX_DivAsync/FX_DivAsyncImm is executed.

  Arguments:    none

  Returns:      the result of division in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_GetDivResult(void)
{
    return (fx32)((CP_GetDivResult64() + FX_DIV_1_2) >> FX_DIV_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         FX_InvAsync

  Description:  Just the same as FX_DivAsync(FX32_ONE, denom).
                This function sets numer and denom onto the divider.
                It also sets up the divider DIVMODE 01(64/32)

  Arguments:    denom     in fx32 format

  Returns:      none
 *---------------------------------------------------------------------------*/
void FX_InvAsync(fx32 denom)
{
    SDK_ASSERT(!CP_IsDivBusy());
    FX_DIVISION_BY_ZERO(FX32_ONE, denom);

    CP_SetDiv64_32((u64)FX32_ONE << 32, (u32)denom);
}


// Sqrt
/*---------------------------------------------------------------------------*
  Name:         FX_SqrtAsync

  Description:  Use a sqrt calculator asynchronously.
                This function sets numer onto the calculator.
                It also sets up the calculator SQRTMODE 01(64).

  Arguments:    numer     in fx32 format

  Returns:      none
 *---------------------------------------------------------------------------*/
void FX_SqrtAsync(fx32 x)
{
    SDK_ASSERT(!CP_IsSqrtBusy());
    if (x > 0)
    {
        CP_SetSqrt64((u64)x << 32);
    }
    else
    {
        CP_SetSqrt64(0);
    }
}


/*---------------------------------------------------------------------------*
  Name:         FX_SqrtAsyncImm

  Description:  Use a sqrt calculator asynchronously.
                This function sets numer onto the calculator.
                It assumes that the calculator is in SQRTMODE 01(64).

  Arguments:    numer     in fx32 format

  Returns:      none
 *---------------------------------------------------------------------------*/
void FX_SqrtAsyncImm(fx32 x)
{
    SDK_ASSERT(!CP_IsSqrtBusy());
    if (x > 0)
    {
        CP_SetSqrtImm64((u64)x << 32);
    }
    else
    {
        CP_SetSqrtImm64(0);
    }
}


/*---------------------------------------------------------------------------*
  Name:         FX_GetSqrtResult

  Description:  Get the result of sqrt in fx32 format.
                This function must be called after FX_SqrtAsync/FX_SqrtAsyncImm is executed.

  Arguments:    none

  Returns:      the result of sqrt in fx32 format
 *---------------------------------------------------------------------------*/
fx32 FX_GetSqrtResult(void)
{
    return (fx32)((CP_GetSqrtResult32() + FX_SQRT_1_2) >> FX_SQRT_SHIFT);
}


// Div
/*---------------------------------------------------------------------------*
  Name:         FX_DivAsync

  Description:  Use a divider asynchronously.
                This function sets numer and denom onto the divider.
                It also sets up the divider DIVMODE 01(64/32)

  Arguments:    numer     in fx32 format
                denom     in fx32 format

  Returns:      none
 *---------------------------------------------------------------------------*/
void FX_DivAsync(fx32 numer, fx32 denom)
{
    SDK_ASSERT(!CP_IsDivBusy());
    FX_DIVISION_BY_ZERO(numer, denom);

    CP_SetDiv64_32((u64)numer << 32, (u32)denom);
}




/*---------------------------------------------------------------------------*
  Name:         FX_DivS32

  Description:  Divides an integer by an integer, and returns the result
                in integer. This uses the divider. Faster than software emulation.

  Arguments:    numer        a value in s32 format
                denom        a value in s32 format

  Returns:      result in s32 format
 *---------------------------------------------------------------------------*/
s32 FX_DivS32(s32 a, s32 b)
{
    SDK_ASSERT(!CP_IsDivBusy());
    FX_DIVISION_BY_ZERO(a, b);
    CP_SetDiv32_32((u32)a, (u32)b);
    return CP_GetDivResult32();
}


/*---------------------------------------------------------------------------*
  Name:         FX_ModS32

  Description:  Divides an integer by an integer, and returns the remainder
                in integer. This uses the divider. Faster than software emulation.

  Arguments:    numer        a value in s32 format
                denom        a value in s32 format

  Returns:      result in s32 format
 *---------------------------------------------------------------------------*/
s32 FX_ModS32(s32 a, s32 b)
{
    SDK_ASSERT(!CP_IsDivBusy());
    FX_DIVISION_BY_ZERO(a, b);
    CP_SetDiv32_32((u32)a, (u32)b);
    return CP_GetDivRemainder32();
}
