/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FX - 
  File:     fx_cp.h

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

#ifndef NITRO_FX_CP_H_
#define NITRO_FX_CP_H_

#include <nitro/fx/fx.h>
#include <nitro/fx/fx_const.h>
#include <nitro/cp/divider.h>
#include <nitro/cp/sqrt.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Type definition
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Declaration of function
//----------------------------------------------------------------------------

fx32    FX_Div(fx32 numer, fx32 denom);
fx64c   FX_DivFx64c(fx32 numer, fx32 denom);
SDK_DECL_INLINE fx32 FX_Mod(fx32 numer, fx32 denom);
fx32    FX_Sqrt(fx32 x);
fx32    FX_Inv(fx32 denom);
fx64c   FX_InvFx64c(fx32 denom);
fx32    FX_InvSqrt(fx32 x);

void    FX_DivAsync(fx32 numer, fx32 denom);
SDK_DECL_INLINE void FX_DivAsyncImm(fx32 numer, fx32 denom);
fx64c   FX_GetDivResultFx64c(void);
fx32    FX_GetDivResult(void);
fx64c   FX_GetDivRemainderFx64c(void);
fx32    FX_GetDivRemainder(void);

void    FX_InvAsync(fx32 denom);
SDK_DECL_INLINE void FX_InvAsyncImm(fx32 denom);
SDK_DECL_INLINE fx64c FX_GetInvResultFx64c(void);
SDK_DECL_INLINE fx32 FX_GetInvResult(void);

void    FX_SqrtAsync(fx32 x);
void    FX_SqrtAsyncImm(fx32 x);
fx32    FX_GetSqrtResult(void);

s32     FX_DivS32(s32 a, s32 b);
s32     FX_ModS32(s32 a, s32 b);

#define FX_DIVISION_BY_ZERO(a, b) \
    SDK_WARNING(b != 0, "Division by zero(%d / %d)", a, b)

//----------------------------------------------------------------------------
// Implementation of inline function
//----------------------------------------------------------------------------

// Inv
/*---------------------------------------------------------------------------*
  Name:         FX_InvAsyncImm

  Description:  Just the same as FX_DivAsyncImm(FX32_ONE, denom).
                This function sets numer and denom onto the divider.
                It assumes that the divider is in DIVMODE 01(64/32).

  Arguments:    denom     in fx32 format

  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_INLINE void FX_InvAsyncImm(fx32 denom)
{
    SDK_ASSERT(!CP_IsDivBusy());
    FX_DIVISION_BY_ZERO(FX32_ONE, denom);

    CP_SetDivImm64_32((u64)FX32_ONE << 32, (u32)denom);
}


/*---------------------------------------------------------------------------*
  Name:         FX_GetInvResultFx64c

  Description:  Just the same as FX_GetDivResultFx64c().

  Arguments:    none

  Returns:      a reciprocal in fx64c format
 *---------------------------------------------------------------------------*/
SDK_INLINE fx64c FX_GetInvResultFx64c(void)
{
    return FX_GetDivResultFx64c();
}

/*---------------------------------------------------------------------------*
  Name:         FX_GetInvResult

  Description:  Just the same as FX_GetDivResult().

  Arguments:    none

  Returns:      a reciprocal in fx32 format
 *---------------------------------------------------------------------------*/
SDK_INLINE fx32 FX_GetInvResult(void)
{
    return FX_GetDivResult();
}

/*---------------------------------------------------------------------------*
  Name:         FX_DivAsyncImm

  Description:  Use a divider asynchronously.
                This function sets numer and denom onto the divider.
                It assumes that the divider is in DIVMODE 01(64/32).

  Arguments:    numer     in fx32 format
                denom     in fx32 format

  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_INLINE void FX_DivAsyncImm(fx32 numer, fx32 denom)
{
    SDK_ASSERT(!CP_IsDivBusy());
    FX_DIVISION_BY_ZERO(numer, denom);

    CP_SetDivImm64_32((u64)numer << 32, (u32)denom);
}

/*---------------------------------------------------------------------------*
  Name:         FX_Mod

  Description:  Divides 'numer' by 'denom', and returns the remainder in fx32
                format. This uses the divider.

  Arguments:    numer        a value in fx32 format
                denom        a value in fx32 format

  Returns:      result in fx32 format
 *---------------------------------------------------------------------------*/
SDK_INLINE fx32 FX_Mod(fx32 numer, fx32 denom)
{
    return (fx32)FX_ModS32(numer, denom);
}

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif
