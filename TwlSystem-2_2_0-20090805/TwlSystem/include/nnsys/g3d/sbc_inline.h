/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     sbc_inline.h

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

#ifndef NNSG3D_SBC_INLINE_H_
#define NNSG3D_SBC_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

//
// Callback check
//
#ifndef NNS_G3D_SBC_CALLBACK_TIMING_A_DISABLE
NNS_G3D_INLINE BOOL
NNSi_G3dCallBackCheck_A(NNSG3dRS* rs, u8 cmd, NNSG3dSbcCallBackTiming* pTiming)
{
    // Because this is the first callback to be called, set *pTiming.
    *pTiming = NNSi_CheckPossibilityOfCallBack(rs, cmd);
    if (*pTiming == NNS_G3D_SBC_CALLBACK_TIMING_A)
    {
        rs->flag &= ~NNS_G3D_RSFLAG_SKIP;
        (*rs->cbVecFunc[cmd])(rs);
        // Recheck because the settings in the callback may have been changed
        *pTiming = NNSi_CheckPossibilityOfCallBack(rs, cmd);
        return (BOOL)(rs->flag & NNS_G3D_RSFLAG_SKIP);
    }
    else
    {
        return FALSE;
    }
}
#else
NNS_G3D_INLINE BOOL
NNSi_G3dCallBackCheck_A(NNSG3dRS* rs, u8 cmd, NNSG3dSbcCallBackTiming* pTiming)
{
    // Because this is the first callback to be called, set *pTiming.
    *pTiming = NNSi_CheckPossibilityOfCallBack(rs, cmd);
    return FALSE;
}
#endif


#ifndef NNS_G3D_SBC_CALLBACK_TIMING_B_DISABLE
NNS_G3D_INLINE BOOL
NNSi_G3dCallBackCheck_B(NNSG3dRS* rs, u8 cmd, NNSG3dSbcCallBackTiming* pTiming)
{
    if (*pTiming == NNS_G3D_SBC_CALLBACK_TIMING_B)
    {
        rs->flag &= ~NNS_G3D_RSFLAG_SKIP;
        (*rs->cbVecFunc[cmd])(rs);
        // Recheck because the settings in the callback may have been changed
        *pTiming = NNSi_CheckPossibilityOfCallBack(rs, cmd);
        return (BOOL)(rs->flag & NNS_G3D_RSFLAG_SKIP);
    }
    else
    {
        return FALSE;
    }
}
#else
NNS_G3D_INLINE BOOL
NNSi_G3dCallBackCheck_B(NNSG3dRS*, u8, NNSG3dSbcCallBackTiming*) { return FALSE; }
#endif


#ifndef NNS_G3D_SBC_CALLBACK_TIMING_C_DISABLE
NNS_G3D_INLINE BOOL
NNSi_G3dCallBackCheck_C(NNSG3dRS* rs, u8 cmd, NNSG3dSbcCallBackTiming timing)
{
    if (timing == NNS_G3D_SBC_CALLBACK_TIMING_C)
    {
        rs->flag &= ~NNS_G3D_RSFLAG_SKIP;
        (*rs->cbVecFunc[cmd])(rs);
        // Another check is unnecessary because this is the last callback
        return (BOOL)(rs->flag & NNS_G3D_RSFLAG_SKIP);
    }
    else
    {
        return FALSE;
    }
}
#else
NNS_G3D_INLINE BOOL
NNSi_G3dCallBackCheck_C(NNSG3dRS*, u8, NNSG3dSbcCallBackTiming) { return FALSE; }
#endif

//
//
//
NNS_G3D_INLINE NNSG3dSbcCallBackTiming
NNSi_CheckPossibilityOfCallBack(NNSG3dRS* rs, u8 cmd)
{
    if (rs->cbVecFunc[cmd])
    {
        return (NNSG3dSbcCallBackTiming)rs->cbVecTiming[cmd];        
    }
    else
    {
        return NNS_G3D_SBC_CALLBACK_TIMING_NONE;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSSetCallBack

    Sets the callback function that can be set for the SBC command cmd with the NNSG3dRS structure.
    Normally, it is used in the initializing function that can be set with NNS_G3dRenderObjSetInitFunc.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRSSetCallBack(NNSG3dRS* rs,
                     NNSG3dSbcCallBackFunc func,
                     u8 cmd,
                     NNSG3dSbcCallBackTiming timing)
{
    NNS_G3D_NULL_ASSERT(rs);
    NNS_G3D_SBC_CALLBACK_TIMING_ASSERT(timing);
    NNS_G3D_ASSERT(cmd < NNS_G3D_SBC_COMMAND_NUM);

    rs->cbVecFunc[cmd] = func;
    rs->cbVecTiming[cmd] = timing;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSResetCallBack

    Releases the callback function for the NNSG3dRS structure's cmd.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRSResetCallBack(NNSG3dRS* rs, u8 cmd)
{
    NNS_G3D_NULL_ASSERT(rs);
    NNS_G3D_ASSERT(cmd < NNS_G3D_SBC_COMMAND_NUM);

    rs->cbVecFunc[cmd] = NULL;
    rs->cbVecTiming[cmd] = (u8)NNS_G3D_SBC_CALLBACK_TIMING_NONE;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetRenderObj

    Returns a pointer to the NNSG3dRenderObj structure which the NNSG3dRS structure retains.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE NNSG3dRenderObj*
NNS_G3dRSGetRenderObj(NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    return rs->pRenderObj;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetMatAnmResult

    Returns a pointer to the NNSG3dMatAnmResult structure which the NNSG3dRS structure holds.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE NNSG3dMatAnmResult*
NNS_G3dRSGetMatAnmResult(NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    return rs->pMatAnmResult;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetJntAnmResult

    Returns a pointer to the NNSG3dJntAnmResult structure which the NNSG3dRS structure holds.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE NNSG3dJntAnmResult*
NNS_G3dRSGetJntAnmResult(NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    return rs->pJntAnmResult;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetJntAnmResult

    Returns a pointer to the NNSG3dJntAnmResult structure which the NNSG3dRS structure holds.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE NNSG3dVisAnmResult*
NNS_G3dRSGetVisAnmResult(NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    return rs->pVisAnmResult;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetSbcPtr

    Returns a pointer to the currently executed SBC instruction which the NNSG3dRS structure holds.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE u8*
NNS_G3dRSGetSbcPtr(NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    return rs->c;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSSetFlag

    Sets flags which the NNSG3dRS structure holds.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRSSetFlag(NNSG3dRS* rs, NNSG3dRSFlag flag)
{
    NNS_G3D_NULL_ASSERT(rs);
    rs->flag |= flag;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSResetFlag

    Resets a flag which the NNSG3dRS structure retains.
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE void
NNS_G3dRSResetFlag(NNSG3dRS* rs, NNSG3dRSFlag flag)
{
    NNS_G3D_NULL_ASSERT(rs);
    rs->flag &= ~flag;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetCurrentMatID

    Gets the material ID currently set in the NNSG3dRS structure.
    Returns -1 if no material ID is set.

    The material ID is set by the SBC MAT command. That value is maintained until it is either overridden by the next MAT command or changed by a callback function.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE int
NNS_G3dRSGetCurrentMatID(const NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    if (rs->flag & NNS_G3D_RSFLAG_CURRENT_MAT_VALID)
    {
        return rs->currentMat;
    }
    else
    {
        return -1;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetCurrentNodeID

    Gets the node ID currently set in the NNSG3dRS structure.
    Returns -1 if no node ID is set.

    The node ID is set by the SBC NODE command. That value is maintained until it is either overridden by the next NODE command or changed by a callback function.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE int
NNS_G3dRSGetCurrentNodeID(const NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    if (rs->flag & NNS_G3D_RSFLAG_CURRENT_NODE_VALID)
    {
        return rs->currentNode;
    }
    else
    {
        return -1;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetCurrentNodeDescID

    Gets the NodeDescID currently set in the NNSG3dRS structure.
    Returns -1 if no NodeDescID is set.

    The NodeDescID is set by the SBC NODEDESC command. That value is maintained until it is either overridden by the next NODEDESC command or changed by a callback function.
    
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE int
NNS_G3dRSGetCurrentNodeDescID(const NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    if (rs->flag & NNS_G3D_RSFLAG_CURRENT_NODEDESC_VALID)
    {
        return rs->currentNodeDesc;
    }
    else
    {
        return -1;
    }
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetPosScale

    Gets the scale value cached in the NNSG3dRS structure to multiply for the vertex coordinates.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE fx32
NNS_G3dRSGetPosScale(const NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    return rs->posScale;
}


/*---------------------------------------------------------------------------*
    NNS_G3dRSGetInvPosScale

    Gets the inverse scale value cached in the NNSG3dRS structure to multiply for the vertex coordinates.
    
 *---------------------------------------------------------------------------*/
NNS_G3D_INLINE fx32
NNS_G3dRSGetInvPosScale(const NNSG3dRS* rs)
{
    NNS_G3D_NULL_ASSERT(rs);
    return rs->invPosScale;
}


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif
