/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - 
  File:     fx_vec.c

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2010-04-16#$
  $Rev: 11321 $
  $Author: yada $
 *---------------------------------------------------------------------------*/



#include <nitro/fx/fx_vec.h>
#include <nitro/fx/fx_cp.h>
#include <nitro/fx/fx_const.h>
#include <nitro/cp/divider.h>
#include <nitro/cp/sqrt.h>


/*---------------------------------------------------------------------*
Name:           VEC_Add

Description:    add two vectors.

Arguments:      a    a pointer to first vector
                b    a pointer to second vector
                ab   a pointer to resultant vector (a + b).
                     ok if ab == a or ab == b.

Return:         none.
 *---------------------------------------------------------------------*/
void VEC_Add(const VecFx32 *a, const VecFx32 *b, VecFx32 *ab)
{
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);
    SDK_NULL_ASSERT(ab);

    ab->x = a->x + b->x;
    ab->y = a->y + b->y;
    ab->z = a->z + b->z;
}


/*---------------------------------------------------------------------*
Name:           VEC_Subtract

Description:    subtract one vector from another.

Arguments:      a       a pointer to first vector
                b       a pointer to second vector.
                a_b     a pointer to resultant vector (a - b).
                        ok if a_b == a or a_b == b.

Return:         none.
 *---------------------------------------------------------------------*/
void VEC_Subtract(const VecFx32 *a, const VecFx32 *b, VecFx32 *ab)
{
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);
    SDK_NULL_ASSERT(ab);

    ab->x = a->x - b->x;
    ab->y = a->y - b->y;
    ab->z = a->z - b->z;
}


/*---------------------------------------------------------------------*
Name:           VEC_Fx16Add

Description:    add two vectors.

Arguments:      a    a pointer to first vector
                b    a pointer to second vector
                ab   a pointer to resultant vector (a + b).
                     ok if ab == a or ab == b.

Return:         none.
 *---------------------------------------------------------------------*/
void VEC_Fx16Add(const VecFx16 *a, const VecFx16 *b, VecFx16 *ab)
{
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);
    SDK_NULL_ASSERT(ab);

    ab->x = (fx16)(a->x + b->x);
    ab->y = (fx16)(a->y + b->y);
    ab->z = (fx16)(a->z + b->z);
}


/*---------------------------------------------------------------------*
Name:           VEC_Fx16Subtract

Description:    subtract one vector from another.

Arguments:      a       a pointer to first vector
                b       a pointer to second vector.
                a_b     a pointer to resultant vector (a - b).
                        ok if a_b == a or a_b == b.

Return:         none.
 *---------------------------------------------------------------------*/
void VEC_Fx16Subtract(const VecFx16 *a, const VecFx16 *b, VecFx16 *ab)
{
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);
    SDK_NULL_ASSERT(ab);

    ab->x = (fx16)(a->x - b->x);
    ab->y = (fx16)(a->y - b->y);
    ab->z = (fx16)(a->z - b->z);
}


#include <nitro/code32.h>              // Always generate ARM binary for efficiency
/*---------------------------------------------------------------------------*
  Name:         VEC_DotProduct

  Description:  Computes the dot product of two vectors.

  Arguments:    a            a pointer to a vector
                b            a pointer to a vector
  
  Returns:      dot product value in fx32 format
 *---------------------------------------------------------------------------*/
fx32 VEC_DotProduct(const VecFx32 *a, const VecFx32 *b)
{
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);
    return (fx32)(((fx64)a->x * b->x +
                   (fx64)a->y * b->y + (fx64)a->z * b->z + (1 << (FX64_SHIFT - 1))) >> FX64_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         VEC_Fx16DotProduct

  Description:  Computes the dot product of two vectors.

  Arguments:    a            a pointer to a vector
                b            a pointer to a vector
  
  Returns:      dot product value in fx32 format
 *---------------------------------------------------------------------------*/
fx32 VEC_Fx16DotProduct(const VecFx16 *a, const VecFx16 *b)
{
    fx64    tmp1, tmp2;
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);

    tmp1 = (fx64)(a->x * b->x) + (a->y * b->y);
    tmp2 = (fx64)(a->z * b->z) + (1 << (FX64_SHIFT - 1));
    return (fx32)((tmp1 + tmp2) >> FX64_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         VEC_CrossProduct

  Description:  Computes the cross product of two vectors.

  Arguments:    a            a pointer to a vector
                b            a pointer to a vector
                axb          a pointer to the result vector,
                             OK if axb == a or axb == b.
  
  Returns:      none
 *---------------------------------------------------------------------------*/
void VEC_CrossProduct(const VecFx32 *a, const VecFx32 *b, VecFx32 *axb)
{
    fx64    x, y, z;
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);
    SDK_NULL_ASSERT(axb);

    x = ((fx64)a->y * b->z - (fx64)a->z * b->y + (1 << (FX64_SHIFT - 1))) >> FX64_SHIFT;
    y = ((fx64)a->z * b->x - (fx64)a->x * b->z + (1 << (FX64_SHIFT - 1))) >> FX64_SHIFT;
    z = ((fx64)a->x * b->y - (fx64)a->y * b->x + (1 << (FX64_SHIFT - 1))) >> FX64_SHIFT;

    SDK_WARNING(x <= FX32_MAX && x >= FX32_MIN &&
                y <= FX32_MAX && y >= FX32_MIN &&
                z <= FX32_MAX && z >= FX32_MIN,
                "VEC_CrossProduct: Overflow (0x%x, 0x%x, 0x%x) x (0x%x, 0x%x, 0x%x).",
                a->x, a->y, a->z, b->x, b->y, b->z);
	
    axb->x = (fx32)x;
    axb->y = (fx32)y;
    axb->z = (fx32)z;
}


/*---------------------------------------------------------------------------*
  Name:         VEC_Fx16CrossProduct

  Description:  Computes the cross product of two vectors.

  Arguments:    a            a pointer to a vector
                b            a pointer to a vector
                axb          a pointer to the result vector,
                             OK if axb == a or axb == b.
  
  Returns:      none
 *---------------------------------------------------------------------------*/
void VEC_Fx16CrossProduct(const VecFx16 *a, const VecFx16 *b, VecFx16 *axb)
{
    fx32    x, y, z;
    SDK_NULL_ASSERT(a);
    SDK_NULL_ASSERT(b);
    SDK_NULL_ASSERT(axb);

    x = ((a->y * b->z - a->z * b->y + (FX16_ONE >> 1)) >> FX16_SHIFT);
    y = ((a->z * b->x - a->x * b->z + (FX16_ONE >> 1)) >> FX16_SHIFT);
    z = ((a->x * b->y - a->y * b->x + (FX16_ONE >> 1)) >> FX16_SHIFT);

    SDK_WARNING(x <= FX16_MAX && x >= FX16_MIN &&
                y <= FX16_MAX && y >= FX16_MIN &&
                z <= FX16_MAX && z >= FX16_MIN,
                "VEC_Fx16CrossProduct: Overflow (0x%x, 0x%x, 0x%x) x (0x%x, 0x%x, 0x%x).",
                a->x, a->y, a->z, b->x, b->y, b->z);

    axb->x = (fx16)x;
    axb->y = (fx16)y;
    axb->z = (fx16)z;
}


/*---------------------------------------------------------------------------*
  Name:         VEC_Mag

  Description:  Computes the magnitude of a vector

  Arguments:    pSrc         a pointer to a vector
  
  Returns:      magnitude of the vector in fx32 format
 *---------------------------------------------------------------------------*/
fx32 VEC_Mag(const VecFx32 *pSrc)
{
    u64     t;							// always positive number
    fx32    rval;

    SDK_NULL_ASSERT(pSrc);
    SDK_ASSERT(!CP_IsSqrtBusy());

    t = (u64)((fx64)pSrc->x * pSrc->x);
    t += (u64)((fx64)pSrc->y * pSrc->y);
    t += (u64)((fx64)pSrc->z * pSrc->z);

    SDK_WARNING(t <= FX64_MAX>>1,
                "VEC_Mag: Overflow (0x%x, 0x%x, 0x%x).",
                pSrc->x, pSrc->y, pSrc->z);

	t <<= 2;                           // for better precision

    CP_SetSqrt64(t);
    rval = ((fx32)CP_GetSqrtResult32() + 1) >> 1;
    return rval;
}


/*---------------------------------------------------------------------------*
  Name:         VEC_Fx16Mag

  Description:  Computes the magnitude of a vector

  Arguments:    pSrc         a pointer to a vector
  
  Returns:      magnitude of the vector in fx32 format
 *---------------------------------------------------------------------------*/
fx32 VEC_Fx16Mag(const VecFx16 *pSrc)
{
    fx64    t;
    fx32    rval;

    SDK_NULL_ASSERT(pSrc);
    SDK_ASSERT(!CP_IsSqrtBusy());

    t = pSrc->x * pSrc->x;
    t += pSrc->y * pSrc->y;
    t += pSrc->z * pSrc->z;

    t <<= 2;                           // for better precision

    CP_SetSqrt64((u64)t);
    rval = ((fx32)CP_GetSqrtResult32() + 1) >> 1;
    return rval;
}


/*---------------------------------------------------------------------------*
  Name:         VEC_Normalize

  Description:  Normalizes a vector

  Arguments:    pSrc         a pointer to a vector
                pDst         a pointer to the normalized vector
  
  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL
void VEC_Normalize(const VecFx32 *pSrc, VecFx32 *pDst)
{
    fx64    t;
    s32     sqrt;

    SDK_NULL_ASSERT(pSrc);
    SDK_NULL_ASSERT(pDst);
    SDK_ASSERT(!CP_IsDivBusy() && !CP_IsSqrtBusy());

    t = (fx64)pSrc->x * pSrc->x;
    t += (fx64)pSrc->y * pSrc->y;
    t += (fx64)pSrc->z * pSrc->z;      // in s39.24 format
    SDK_TASSERTMSG(t > 0, "*pSrc is (0, 0, 0)");

    SDK_WARNING(1LL << (FX32_SHIFT + FX32_SHIFT + FX64C_SHIFT) >= (u64)t, "*pSrc is too large!");

    CP_SetDiv64_64(1LL << (FX32_SHIFT + FX32_SHIFT + FX64C_SHIFT), (u64)t);
    CP_SetSqrt64((u64)(t << 2));

    sqrt = (s32)CP_GetSqrtResult32();  // in s18.13 format    
    t = CP_GetDivResult64();           // in s31.32 format(fx64c)

    t = t * sqrt;                      // in s18.45 format
    pDst->x = (fx32)((t * pSrc->x + (1LL << (32 + FX32_SHIFT))) >> (32 + FX32_SHIFT + 1));
    pDst->y = (fx32)((t * pSrc->y + (1LL << (32 + FX32_SHIFT))) >> (32 + FX32_SHIFT + 1));
    pDst->z = (fx32)((t * pSrc->z + (1LL << (32 + FX32_SHIFT))) >> (32 + FX32_SHIFT + 1));
}


/*---------------------------------------------------------------------------*
  Name:         VEC_Fx16Normalize

  Description:  Normalizes a vector

  Arguments:    pSrc         a pointer to a vector
                pDst         a pointer to the normalized vector
  
  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_WEAK_SYMBOL
void VEC_Fx16Normalize(const VecFx16 *pSrc, VecFx16 *pDst)
{
    fx64    t;
    s32     sqrt;

    SDK_NULL_ASSERT(pSrc);
    SDK_NULL_ASSERT(pDst);
    SDK_ASSERT(!CP_IsDivBusy() && !CP_IsSqrtBusy());

    t = pSrc->x * pSrc->x;
    t += pSrc->y * pSrc->y;
    t += pSrc->z * pSrc->z;            // in s39.24 format
    SDK_TASSERTMSG(t > 0, "*pSrc is (0, 0, 0)");

    CP_SetDiv64_64(1LL << (FX32_SHIFT + FX32_SHIFT + FX64C_SHIFT), (u64)t);
    CP_SetSqrt64((u64)(t << 2));

    sqrt = (s32)CP_GetSqrtResult32();  // in s18.13 format    
    t = CP_GetDivResult64();           // in s31.32 format(fx64c)

    t = t * sqrt;                      // in s18.45 format
    pDst->x = (fx16)((t * pSrc->x + (1LL << (32 + FX32_SHIFT))) >> (32 + FX32_SHIFT + 1));
    pDst->y = (fx16)((t * pSrc->y + (1LL << (32 + FX32_SHIFT))) >> (32 + FX32_SHIFT + 1));
    pDst->z = (fx16)((t * pSrc->z + (1LL << (32 + FX32_SHIFT))) >> (32 + FX32_SHIFT + 1));
}


/*---------------------------------------------------------------------------*
  Name:         VEC_MultAdd

  Description:  Computes 'a * v1 + v2'.

  Arguments:    a            fixed point number(fx32)
                v1           a pointer to a vector
                v2           a pointer to a vector
                pDest        a pointer to a resultant vector
  
  Returns:      none
 *---------------------------------------------------------------------------*/
void VEC_MultAdd(fx32 a, const VecFx32 *v1, const VecFx32 *v2, VecFx32 *pDest)
{
    SDK_NULL_ASSERT(v1);
    SDK_NULL_ASSERT(v2);
    SDK_NULL_ASSERT(pDest);

    pDest->x = v2->x + (fx32)(((fx64)a * v1->x) >> FX32_SHIFT);
    pDest->y = v2->y + (fx32)(((fx64)a * v1->y) >> FX32_SHIFT);
    pDest->z = v2->z + (fx32)(((fx64)a * v1->z) >> FX32_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         VEC_MultSubtract

  Description:  Computes 'a * v1 - v2'.

  Arguments:    a            fixed point number(fx32)
                v1           a pointer to a vector
                v2           a pointer to a vector
                pDest        a pointer to a resultant vector
  
  Returns:      none
 *---------------------------------------------------------------------------*/
void VEC_MultSubtract(fx32 a, const VecFx32 *v1, const VecFx32 *v2, VecFx32 *pDest)
{
    SDK_NULL_ASSERT(v1);
    SDK_NULL_ASSERT(v2);
    SDK_NULL_ASSERT(pDest);

    pDest->x = -v2->x + (fx32)(((fx64)a * v1->x) >> FX32_SHIFT);
    pDest->y = -v2->y + (fx32)(((fx64)a * v1->y) >> FX32_SHIFT);
    pDest->z = -v2->z + (fx32)(((fx64)a * v1->z) >> FX32_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         VEC_Distance

  Description:  Computes |v1 - v2|

  Arguments:    v1           a pointer to a vector
                v2           a pointer to a vector
  
  Returns:      distance between v1 and v2
 *---------------------------------------------------------------------------*/
fx32 VEC_Distance(const VecFx32 *v1, const VecFx32 *v2)
{
	u64     tmp;							// always positive number
	fx32    diff;

    SDK_NULL_ASSERT(v1);
    SDK_NULL_ASSERT(v2);

    diff = v1->x - v2->x;
    tmp = (u64)((fx64)diff *diff);

    diff = v1->y - v2->y;
    tmp += (u64)((fx64)diff *diff);

    diff = v1->z - v2->z;
    tmp += (u64)((fx64)diff *diff);

    SDK_WARNING(tmp <= FX64_MAX>>1,
                "VEC_Distance: Overflow (0x%x, 0x%x, 0x%x), (0x%x, 0x%x, 0x%x).",
                v1->x, v1->y, v1->z, v2->x, v2->y, v2->z);

	tmp <<= 2;							// for better precision
	CP_SetSqrt64(tmp);

    return ((fx32)CP_GetSqrtResult32() + 1) >> 1;
}

/*---------------------------------------------------------------------------*
  Name:         VEC_Fx16Distance

  Description:  Computes |v1 - v2|

  Arguments:    v1           a pointer to a vector
                v2           a pointer to a vector
  
  Returns:      distance between v1 and v2
 *---------------------------------------------------------------------------*/
fx32 VEC_Fx16Distance(const VecFx16 *v1, const VecFx16 *v2)
{
    u64     tmp;
    fx32    diff;

    SDK_NULL_ASSERT(v1);
    SDK_NULL_ASSERT(v2);

    diff = v1->x - v2->x;
    tmp = (u32)diff * diff;

    diff = v1->y - v2->y;
    tmp += (u32)diff * diff;

    diff = v1->z - v2->z;
    tmp += (u32)diff * diff;

    tmp <<= 2;
    CP_SetSqrt64((u64)tmp);

    return ((fx32)CP_GetSqrtResult32() + 1) >> 1;
}


#include <nitro/codereset.h>
