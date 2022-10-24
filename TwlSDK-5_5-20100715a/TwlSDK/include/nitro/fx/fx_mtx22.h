/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FX - 
  File:     fx_mtx22.h

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

#ifndef NITRO_FX_MTX22_H_
#define NITRO_FX_MTX22_H_

#include <nitro/fx/fx.h>
#include <nitro/mi/memory.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Type definition
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Declaration of function
//----------------------------------------------------------------------------

SDK_DECL_INLINE void MTX_Identity22(MtxFx22 *pDst);
SDK_DECL_INLINE void MTX_Copy22(const MtxFx22 *pSrc, MtxFx22 *pDst);
SDK_DECL_INLINE void MTX_Transpose22(const MtxFx22 *pSrc, MtxFx22 *pDst);
SDK_DECL_INLINE void MTX_Rot22(MtxFx22 *pDst, fx32 sinVal, fx32 cosVal);
int     MTX_Inverse22(const MtxFx22 *pSrc, MtxFx22 *pDst);
void    MTX_Concat22(const MtxFx22 *a, const MtxFx22 *b, MtxFx22 *ab);
void    MTX_ScaleApply22(const MtxFx22 *pSrc, MtxFx22 *pDst, fx32 x, fx32 y);

void    MTX_Identity22_(register MtxFx22 *pDst);
void    MTX_Transpose22_(const register MtxFx22 *pSrc, register MtxFx22 *pDst);
void    MTX_Scale22_(register MtxFx22 *pDst, register fx32 x, register fx32 y);
void    MTX_Rot22_(register MtxFx22 *pDst, register fx32 sinVal, register fx32 cosVal);

//----------------------------------------------------------------------------
// Implementation of inline function
//----------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         MTX_Identity22

  Description:  Sets a matrix to identity.

  Arguments:    pDst         a pointer to a 2x2 matrix

  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_INLINE void MTX_Identity22(MtxFx22 *pDst)
{
    SDK_NULL_ASSERT(pDst);
    MTX_Identity22_(pDst);
}

/*---------------------------------------------------------------------------*
  Name:         MTX_Copy22

  Description:  Copies a 2x2 matrix to a 2x2 matrix.

  Arguments:    pSrc         a pointer to a 2x2 matrix
                pDst         a pointer to a 2x2 matrix

  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_INLINE void MTX_Copy22(const MtxFx22 *pSrc, MtxFx22 *pDst)
{
    SDK_NULL_ASSERT(pSrc);
    SDK_NULL_ASSERT(pDst);
    MI_Copy16B(pSrc, pDst);
}


/*---------------------------------------------------------------------------*
  Name:         MTX_Transpose22

  Description:  Computes the transpose of a 2x2 matrix.

  Arguments:    pSrc         a pointer to a 2x2 matrix
                pDst         a pointer to a 2x2 matrix

  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_INLINE void MTX_Transpose22(const MtxFx22 *pSrc, MtxFx22 *pDst)
{
    SDK_NULL_ASSERT(pSrc);
    SDK_NULL_ASSERT(pDst);
    MTX_Transpose22_(pSrc, pDst);
}


/*---------------------------------------------------------------------------*
  Name:         MTX_Scale22

  Description:  Sets a scaling matrix.

  Arguments:    pDst         a pointer to a 2x2 matrix
                x            x scale factor
                y            y scale factor
  
  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_INLINE void MTX_Scale22(MtxFx22 *pDst, fx32 x, fx32 y)
{
    SDK_NULL_ASSERT(pDst);
    MTX_Scale22_(pDst, x, y);
}


/*---------------------------------------------------------------------------*
  Name:         MTX_Rot22

  Description:  Sets a rotation matrix.

  Arguments:    pDst         a pointer to a 2x2 matrix
                sinVal       sine of an angle of rotation
                cosVal       cosine of an angle of rotation
  
  Returns:      none
 *---------------------------------------------------------------------------*/
SDK_INLINE void MTX_Rot22(MtxFx22 *pDst, fx32 sinVal, fx32 cosVal)
{
    SDK_NULL_ASSERT(pDst);
    MTX_Rot22_(pDst, sinVal, cosVal);
}


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif
