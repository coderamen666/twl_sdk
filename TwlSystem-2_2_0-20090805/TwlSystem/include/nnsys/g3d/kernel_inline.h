/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     kernel_inline.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

//
// AUTHOR: Kenji Nishida
//

#ifndef NNSG3D_KERNEL_INLINE_H_
#define NNSG3D_KERNEL_INLINE_H_
//
// DO NOT INCLUDE THIS FILE DIRECTLY
//

#include <nnsys/g3d/config.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// inline functions for NNSG3dAnmObj
//

/*---------------------------------------------------------------------------
    NNS_G3dAnmObjSetFrame

    This function sets the playback frame for the animation object.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dAnmObjSetFrame(NNSG3dAnmObj* pAnmObj, fx32 frame)
{
    NNS_G3D_NULL_ASSERT(pAnmObj);

    pAnmObj->frame = frame;
}


/*---------------------------------------------------------------------------
    NNS_G3dAnmObjSetBlendRatio

    Sets the blend ratio (from 0 to FX32_ONE) in the animation object.
    The animation blend function references this.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dAnmObjSetBlendRatio(NNSG3dAnmObj* pAnmObj, fx32 ratio)
{
    NNS_G3D_NULL_ASSERT(pAnmObj);

    pAnmObj->ratio = ratio;
}


/*---------------------------------------------------------------------------
    NNS_G3dAnmObjGetNumFrame

    Gets the number of animation frames.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE fx32
NNS_G3dAnmObjGetNumFrame(const NNSG3dAnmObj* pAnmObj)
{
    const NNSG3dResAnmCommon* p;
    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_ASSERT(NNSi_G3dIsValidAnmRes(pAnmObj->resAnm));

    p = (const NNSG3dResAnmCommon*) pAnmObj->resAnm;
    return p->numFrame * FX32_ONE;
}





//
// inline functions for NNSG3dRenderObj
//

/*---------------------------------------------------------------------------
    NNS_G3dRenderObjSetFlag

    Sets what was designated by the execution control flag of the rendering object.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRenderObjSetFlag(NNSG3dRenderObj* pRenderObj, NNSG3dRenderObjFlag flag)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);

    pRenderObj->flag |= flag;
}


/*---------------------------------------------------------------------------
    NNS_G3dRenderObjResetFlag

    Resets what was designated by the execution control flag of the rendering object.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRenderObjResetFlag(NNSG3dRenderObj* pRenderObj, NNSG3dRenderObjFlag flag)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);

    pRenderObj->flag &= ~flag;
}


/*---------------------------------------------------------------------------
    NNS_G3dRenderObjTestFlag

    Tests whether or not the execution control flag of the rendering object has been set.
    Returns TRUE if the designated flags are all set.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE BOOL
NNS_G3dRenderObjTestFlag(const NNSG3dRenderObj* pRenderObj, NNSG3dRenderObjFlag flag)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);

    return (pRenderObj->flag & flag) == flag;
}


/*---------------------------------------------------------------------------
    NNS_G3dRenderObjSetUserSbc

    Sets the user-defined SBC to the rendering object.
    The return value is the user-defined SBC that had been configured up to then.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE u8*
NNS_G3dRenderObjSetUserSbc(NNSG3dRenderObj* pRenderObj, u8* sbc)
{
    u8* rval;
    NNS_G3D_NULL_ASSERT(pRenderObj);
    // sbc can be NULL

    rval = pRenderObj->ptrUserSbc;
    pRenderObj->ptrUserSbc = sbc;
    return rval;
}


/*---------------------------------------------------------------------------
    NNS_G3dRenderObjSetJntAnmBuffer

    Configures the buffer that registers and records and plays the calculation result of the joint as a rendering object.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRenderObjSetJntAnmBuffer(NNSG3dRenderObj* pRenderObj,
                                struct NNSG3dJntAnmResult_* buf)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(buf);

//    pRenderObj->flag |= NNS_G3D_RENDEROBJ_FLAG_RECORD;
    pRenderObj->recJntAnm = buf;
}


/*---------------------------------------------------------------------------
    NNS_G3dRenderObjSetMatAnmBuffer

    Configures the buffer that registers and records and plays the calculation result of the material as a rendering object.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRenderObjSetMatAnmBuffer(NNSG3dRenderObj* pRenderObj,
                                struct NNSG3dMatAnmResult_* buf)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(buf);

//    pRenderObj->flag |= NNS_G3D_RENDEROBJ_FLAG_RECORD;
    pRenderObj->recMatAnm = buf;
}


/*---------------------------------------------------------------------------
    NNS_G3dRenderObjReleaseJntAnmBuffer

    Removes the buffer that records and plays the calculation result of the joint as a rendering object.
     The return value is the pointer to the released buffer.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE struct NNSG3dJntAnmResult_*
NNS_G3dRenderObjReleaseJntAnmBuffer(NNSG3dRenderObj* pRenderObj)
{
    struct NNSG3dJntAnmResult_* rval;
    NNS_G3D_NULL_ASSERT(pRenderObj);

    // Reset the RECORD flag when both buffers disappear
    if (!pRenderObj->recMatAnm)
        pRenderObj->flag &= ~NNS_G3D_RENDEROBJ_FLAG_RECORD;
    rval = pRenderObj->recJntAnm;
    pRenderObj->recJntAnm = NULL;
    return rval;
}


/*---------------------------------------------------------------------------
    NNS_G3dRenderObjReleaseMatAnmBuffer

    Removes the buffer that records and plays the calculation result of the material as a rendering object.
     The return value is the pointer to the released buffer.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE struct NNSG3dMatAnmResult_*
NNS_G3dRenderObjReleaseMatAnmBuffer(NNSG3dRenderObj* pRenderObj)
{
    struct NNSG3dMatAnmResult_* rval;
    NNS_G3D_NULL_ASSERT(pRenderObj);

    // Reset the RECORD flag when both buffers disappear
    if (!pRenderObj->recJntAnm)
        pRenderObj->flag &= ~NNS_G3D_RENDEROBJ_FLAG_RECORD;
    rval = pRenderObj->recMatAnm;
    pRenderObj->recMatAnm = NULL;
    return rval;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRenderObjSetUserPtr

    Sets the pointer to the region that the user can use with a callback.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void*
NNS_G3dRenderObjSetUserPtr(NNSG3dRenderObj* pRenderObj, void* ptr)
{
    void* rval = pRenderObj->ptrUser;
    pRenderObj->ptrUser = ptr;
    return rval;
}


/*---------------------------------------------------------------------------*
    NNSG3dRenderObjGetResMdl

    Returns a pointer to the model resource retained by the rendering object.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE NNSG3dResMdl*
NNS_G3dRenderObjGetResMdl(NNSG3dRenderObj* pRenderObj)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    return pRenderObj->resMdl;
}


/*---------------------------------------------------------------------------*
    NNSG3dRenderObjSetBlendFuncMat

    Sets the (unique) material blend function in the rendering object.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRenderObjSetBlendFuncMat(NNSG3dRenderObj* pRenderObj,
                                NNSG3dFuncAnmBlendMat func)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(func);

    pRenderObj->funcBlendMat = func;
}


/*---------------------------------------------------------------------------*
    NNSG3dRenderObjSetBlendFuncJnt

    Sets the (unique) joint blend function in the rendering object.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRenderObjSetBlendFuncJnt(NNSG3dRenderObj* pRenderObj,
                                NNSG3dFuncAnmBlendJnt func)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(func);

    pRenderObj->funcBlendJnt = func;
}


/*---------------------------------------------------------------------------*
    NNSG3dRenderObjSetBlendFuncVis

    Sets the (unique) visibility blend function in the rendering object.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRenderObjSetBlendFuncVis(NNSG3dRenderObj* pRenderObj,
                                NNSG3dFuncAnmBlendVis func)
{
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(func);

    pRenderObj->funcBlendVis = func;
}



//
// Misc inline functions
//




#ifdef __cplusplus
}
#endif

#endif
