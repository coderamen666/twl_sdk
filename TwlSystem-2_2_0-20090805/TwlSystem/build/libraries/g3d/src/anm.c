/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d
  File:     anm.c

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1335 $
 *---------------------------------------------------------------------------*/

//
// AUTHOR: Kenji Nishida
//

#include <nnsys/g3d/anm.h>
#include <nnsys/g3d/kernel.h>

#include <nnsys/g3d/anm/nsbca.h>
#include <nnsys/g3d/anm/nsbma.h>
#include <nnsys/g3d/anm/nsbta.h>
#include <nnsys/g3d/anm/nsbtp.h>
#include <nnsys/g3d/anm/nsbva.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>

// Blending scales when the Classic Scale of Maya SSC, Si3d is off 
//#define FIX_SCALEEX_BLEND_BUG

#ifdef FIX_SCALEEX_BLEND_BUG
#include <nnsys/g3d/sbc.h>
#include <nnsys/g3d/util.h>
#endif // FIX_SCALEEX_BLEND_BUG

//
// Pointer to default animation blend function (global variable)
// Function pointer that is set when initializing AnmObj and RenderObj
//
NNSG3dFuncAnmBlendMat NNS_G3dFuncBlendMatDefault = &NNSi_G3dAnmBlendMat;
NNSG3dFuncAnmBlendJnt NNS_G3dFuncBlendJntDefault = &NNSi_G3dAnmBlendJnt;
NNSG3dFuncAnmBlendVis NNS_G3dFuncBlendVisDefault = &NNSi_G3dAnmBlendVis;

//
// Default handler of .nsbma format (material color animation)
//
NNSG3dFuncAnmMat NNS_G3dFuncAnmMatNsBmaDefault =
#ifndef NNS_G3D_NSBMA_DISABLE
    &NNSi_G3dAnmCalcNsBma;
#else
    NULL;
#endif


//
// Default handler of .nsbtp format (texture pattern animation)
//
NNSG3dFuncAnmMat NNS_G3dFuncAnmMatNsBtpDefault =
#ifndef NNS_G3D_NSBTP_DISABLE
    &NNSi_G3dAnmCalcNsBtp;
#else
    NULL;
#endif


//
// Default handler of .nsbta format (texture SRT animation)
//
NNSG3dFuncAnmMat NNS_G3dFuncAnmMatNsBtaDefault =
#ifndef NNS_G3D_NSBTA_DISABLE
    &NNSi_G3dAnmCalcNsBta;
#else
    NULL;
#endif


//
// Default handler of .nsbca format (joint animation)
//
NNSG3dFuncAnmJnt NNS_G3dFuncAnmJntNsBcaDefault =
#ifndef NNS_G3D_NSBCA_DISABLE
    &NNSi_G3dAnmCalcNsBca;
#else
    NULL;
#endif


//
// Default handler of .nsbva format (visibility animation)
//
NNSG3dFuncAnmVis NNS_G3dFuncAnmVisNsBvaDefault =
#ifndef NNS_G3D_NSBVA_DISABLE
    &NNSi_G3dAnmCalcNsBva;
#else
    NULL;
#endif


/*---------------------------------------------------------------------------*
    NNSi_G3dAnmBlendMat

    Default material animation blend function
    Each animation will write data to pResult in order.
    The return value is whether pResult has a valid result (whether the animation has been calculated).
    
 *---------------------------------------------------------------------------*/
BOOL NNSi_G3dAnmBlendMat(NNSG3dMatAnmResult* pResult,
                         const NNSG3dAnmObj* pAnmObj,
                         u32 matID)
{
    BOOL rval = FALSE;
    NNS_G3D_NULL_ASSERT(pResult);
    NNS_G3D_ASSERT(!pAnmObj || matID < pAnmObj->numMapData);

    if (pAnmObj)
    {
        const NNSG3dAnmObj* p = pAnmObj;
        do
        {
            if (matID < p->numMapData)
            {
                // Convert from matID to resource ID
                u32 dataIdx = p->mapData[matID];
                if ((dataIdx & (NNS_G3D_ANMOBJ_MAPDATA_EXIST |
                                NNS_G3D_ANMOBJ_MAPDATA_DISABLED)) ==
                        NNS_G3D_ANMOBJ_MAPDATA_EXIST)
                {
                    NNSG3dFuncAnmMat func = (NNSG3dFuncAnmMat)p->funcAnm;
                    
                    NNS_G3D_NULL_ASSERT(func);
                    if (func)
                    {
                        (*func)(pResult,
                                p,
                                dataIdx & NNS_G3D_ANMOBJ_MAPDATA_DATAFIELD);
                        rval = TRUE;
                    }
                }
            }
            p = p->next;
        }
        while(p);
    }

    return rval;
}

#include <nitro/code32.h>
static void
blendScaleVec_(VecFx32* v0, const VecFx32* v1, fx32 ratio, BOOL isV1One)
{
    if (isV1One)
    {
        // When *v1 is (1,1,1)
        v0->x += ratio;
        v0->y += ratio;
        v0->z += ratio;
    }
    else
    {
        v0->x += ratio * v1->x >> FX32_SHIFT;
        v0->y += ratio * v1->y >> FX32_SHIFT;
        v0->z += ratio * v1->z >> FX32_SHIFT;
    }
}
#include <nitro/codereset.h>


/*---------------------------------------------------------------------------*
    NNSi_G3dAnmBlendJnt

    Default joint animation blend function
    The return value is whether pResult has a valid result (whether the animation has been calculated).
    
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
BOOL NNSi_G3dAnmBlendJnt(NNSG3dJntAnmResult* pResult,
                         const NNSG3dAnmObj* pAnmObj,
                         u32 nodeID)
{
    NNS_G3D_NULL_ASSERT(pResult);

    if (!pAnmObj)
    {
        return FALSE;
    }

    if (!pAnmObj->next)
    {
        //
        // When only one joint animation exists
        //
        u32 dataIdx;
        if (nodeID < pAnmObj->numMapData)
        {
            dataIdx = pAnmObj->mapData[nodeID];
            if ((dataIdx & (NNS_G3D_ANMOBJ_MAPDATA_EXIST |
                            NNS_G3D_ANMOBJ_MAPDATA_DISABLED)) ==
                    NNS_G3D_ANMOBJ_MAPDATA_EXIST)
            {
                NNSG3dFuncAnmJnt func = (NNSG3dFuncAnmJnt)pAnmObj->funcAnm;

                NNS_G3D_NULL_ASSERT(func);
                if (func)
                {
                    (*func)(pResult,
                            pAnmObj,
                            dataIdx & NNS_G3D_ANMOBJ_MAPDATA_DATAFIELD);
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                //
                // Because there is no animation for the nodeID, use the model data's JointSRT
                //
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        //
        // When multiple joint animations exist
        //
        const NNSG3dAnmObj* p;
        NNSG3dJntAnmResult r;
        fx32 sumOfRatio = 0;
        const NNSG3dAnmObj* pLastAnmObj;
        int numBlend = 0;
#if 1  // FIX
        int i1 = 0;
        VecFx32 keepAxisX;
        VecFx32 keepAxisZ;
#endif
#ifdef FIX_SCALEEX_BLEND_BUG
        u8 scalingRule = NNS_G3D_SCALINGRULE_STANDARD;
        if (NNS_G3dRS->pRenderObj && NNS_G3dRS->pRenderObj->resMdl)
        {
            scalingRule = NNS_G3dRS->pRenderObj->resMdl->info.scalingRule;
        }
#endif // FIX_SCALEEX_BLEND_BUG

        p = pAnmObj;
        do
        {
            //
            // Calculate the total of the ratios of each animation:
            // Each animation's ratio must be equal or larger than 0 and equal to or less than FX32_ONE.
            //
            if (nodeID < p->numMapData)
            {
                u32 dataIdx = p->mapData[nodeID];
                if ((dataIdx & (NNS_G3D_ANMOBJ_MAPDATA_EXIST |
                                NNS_G3D_ANMOBJ_MAPDATA_DISABLED)) ==
                        NNS_G3D_ANMOBJ_MAPDATA_EXIST)
                {
                    NNS_G3D_ASSERT(p->ratio >= 0 && p->ratio <= FX32_ONE);
        
                    if (p->ratio > FX32_ONE)
                        sumOfRatio += FX32_ONE;
                    else if (p->ratio > 0)
                        sumOfRatio += p->ratio;

                    pLastAnmObj = p;
                    ++numBlend;
                }
            }
            p = p->next;
        }
        while(p);

        if (sumOfRatio == 0)
        {
            //
            // The model data's JointSRT is used because either there are no animations for nodeID or all of the ratio values are 0.
            // 
            //
            return FALSE;
        }

        if (numBlend == 1)
        {
            // Only one animation is actually ON
            NNSG3dFuncAnmJnt func = (NNSG3dFuncAnmJnt)pLastAnmObj->funcAnm;
            u32 dataIdx = pLastAnmObj->mapData[nodeID];

            NNS_G3D_NULL_ASSERT(func);
            if (func)
            {   
                (*func)(pResult,
                        pLastAnmObj,
                        dataIdx & NNS_G3D_ANMOBJ_MAPDATA_DATAFIELD);
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }

        // Results must be cleared to zero before blending
        MI_CpuClearFast(pResult, sizeof(*pResult));
        pResult->flag = (NNSG3dJntAnmResultFlag)-1; // Only the flags must all be set

        p = pAnmObj;
        do
        {
            if (nodeID < p->numMapData)
            {
                // Convert from nodeID to index within the resource
                u32 dataIdx = p->mapData[nodeID];
                if ((dataIdx & (NNS_G3D_ANMOBJ_MAPDATA_EXIST |
                                NNS_G3D_ANMOBJ_MAPDATA_DISABLED)) ==
                        NNS_G3D_ANMOBJ_MAPDATA_EXIST &&
                    (p->ratio > 0))
                {
                    fx32 ratio;
                    NNSG3dFuncAnmJnt func = (NNSG3dFuncAnmJnt)p->funcAnm;
  
                    NNS_G3D_NULL_ASSERT(func);
                    if (func)
                    {
                        (*func)(&r, p, dataIdx & NNS_G3D_ANMOBJ_MAPDATA_DATAFIELD);
#if 1  // FIX
                        if (i1 == 0)
                        {
                            MI_CpuCopy32(&r.rot._00, &keepAxisX, sizeof(VecFx32));
                            MI_CpuCopy32(&r.rot._20, &keepAxisZ, sizeof(VecFx32));
                        }
#endif

                        // Gets the ratio of this animation to the whole
                        if (sumOfRatio != FX32_ONE)
                        {
                            ratio = FX_Div(p->ratio, sumOfRatio);
                        }
                        else
                        {
                            ratio = p->ratio;
                        }

                        // scale, scaleEx0, scaleEx1 blend
                        blendScaleVec_(&pResult->scale,
                                       &r.scale,
                                       ratio,
                                       r.flag & NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE);

#ifdef FIX_SCALEEX_BLEND_BUG

#ifndef NNS_G3D_SI3D_DISABLE
                        if (scalingRule == NNS_G3D_SCALINGRULE_SI3D)
                        {
                            blendScaleVec_(&pResult->scaleEx0,
                                           &r.scaleEx0,
                                           ratio,
                                           r.flag & NNS_G3D_JNTANM_RESULTFLAG_SCALEEX0_ONE);
                            if (nodeID == 0)
                            {
                                // It is necessary to restore when nodeID = 0, because parentID = 0
                                NNS_G3dRS->isScaleCacheOne[0] = 1; // Endian-dependent
                                //NNSi_G3dBitVecSet(&NNS_G3dRS->isScaleCacheOne[0], nodeID);
                            }
                        }
#endif // NNS_G3D_SI3D_DISABLE

#else // FIX_SCALEEX_BLEND_BUG
                        blendScaleVec_(&pResult->scaleEx0,
                                       &r.scaleEx0,
                                       ratio,
                                       r.flag & NNS_G3D_JNTANM_RESULTFLAG_SCALEEX0_ONE);

                        blendScaleVec_(&pResult->scaleEx1,
                                       &r.scaleEx1,
                                       ratio,
                                       r.flag & NNS_G3D_JNTANM_RESULTFLAG_SCALEEX1_ONE);
#endif // FIX_SCALEEX_BLEND_BUG

                        // Blend of the parallel translation components
                        if (!(r.flag & NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO))
                        {
                            pResult->trans.x += (fx32)((fx64)ratio * r.trans.x >> FX32_SHIFT);
                            pResult->trans.y += (fx32)((fx64)ratio * r.trans.y >> FX32_SHIFT);
                            pResult->trans.z += (fx32)((fx64)ratio * r.trans.z >> FX32_SHIFT);
                        }

                        // Blend of the rotation matrices
                        if (!(r.flag & NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO))
                        {
                            pResult->rot._00 += ratio * r.rot._00 >> FX32_SHIFT;
                            pResult->rot._01 += ratio * r.rot._01 >> FX32_SHIFT;
                            pResult->rot._02 += ratio * r.rot._02 >> FX32_SHIFT;
                            pResult->rot._10 += ratio * r.rot._10 >> FX32_SHIFT;
                            pResult->rot._11 += ratio * r.rot._11 >> FX32_SHIFT;
                            pResult->rot._12 += ratio * r.rot._12 >> FX32_SHIFT;
                        }
                        else
                        {
                            pResult->rot._00 += ratio;
                            pResult->rot._11 += ratio;
                        }

                        pResult->flag &= r.flag;
                    }
                }
            }

            p = p->next;
#if 1
            ++i1;
#endif
        }
        while(p);

#ifdef FIX_SCALEEX_BLEND_BUG

#ifndef NNS_G3D_MAYA_DISABLE
        if (scalingRule == NNS_G3D_SCALINGRULE_MAYA)
        {
            // Cannot determine whether this is the parent of SSC from the pResult, so must check NNS_G3dRS
            const u8* cmd = NNS_G3dRS->c;
            u8 opFlag = *(cmd + 3);
            // It is necessary to cache the inverse of the blended scale when this is the parent node of SSC
            if (opFlag & NNS_G3D_SBC_NODEDESC_FLAG_MAYASSC_PARENT)
            {
                if (pResult->flag & NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE)
                {
                    // When all scales prior to blending are 1 
                    NNSi_G3dBitVecSet(&NNS_G3dRS->isScaleCacheOne[0], nodeID);
                }
                else
                {
                    // When some scales prior to blending are not 1
                    NNSi_G3dBitVecReset(&NNS_G3dRS->isScaleCacheOne[0], nodeID);
                    // Cache the inverse of the scale 
                    NNS_G3dRSOnGlb.scaleCache[nodeID].inv.x = FX_Inv(pResult->scale.x);
                    NNS_G3dRSOnGlb.scaleCache[nodeID].inv.y = FX_Inv(pResult->scale.y);
                    NNS_G3dRSOnGlb.scaleCache[nodeID].inv.z = FX_Inv(pResult->scale.z);
                }
            }

            // Copy the blended inverse scale from the cache for the targeted node of SSC
            if (opFlag & NNS_G3D_SBC_NODEDESC_FLAG_MAYASSC_APPLY)
            {
                u32 parentID = *(cmd + 2);
                if (!NNSi_G3dBitVecCheck(&NNS_G3dRS->isScaleCacheOne[0], parentID))
                {
                    // Do nothing because this is only a flag if the parent scale is 1. 
                    // Otherwise, copy the inverse from the cache.
                    pResult->scaleEx0 = NNS_G3dRSOnGlb.scaleCache[parentID].inv;
                }
            }
        }
#endif // NNS_G3D_MAYA_DISABLE

#ifndef NNS_G3D_SI3D_DISABLE
        if (scalingRule == NNS_G3D_SCALINGRULE_SI3D)
        {
            // It is necessary to store the inverse after the cumulative scale is blended in scaleEx1.
            // If the referenced parent node is correctly blended, it is reasonable to store the blended value in scaleEx0 as is.
            // 
            if (pResult->flag & NNS_G3D_JNTANM_RESULTFLAG_SCALEEX0_ONE)
            {
                // This node does not use scaleEx
                if (pResult->flag & NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE)
                {
                    // Update flags for child nodes. 
                    // The cumulative scale up to this node is 1. 
                    NNSi_G3dBitVecSet(&NNS_G3dRS->isScaleCacheOne[0], nodeID);
                }
                else
                {
                    // Update flags and the cache for child nodes. 
                    // The cumulative scale up to this node is not 1. 
                    NNSi_G3dBitVecReset(&NNS_G3dRS->isScaleCacheOne[0], nodeID);
                    // Cache the cumulative scale
                    NNS_G3dRSOnGlb.scaleCache[nodeID].s = pResult->scaleEx0;
                    // Cache the inverse of the cumulative scale
                    NNS_G3dRSOnGlb.scaleCache[nodeID].inv.x = FX_Inv(pResult->scaleEx0.x);
                    NNS_G3dRSOnGlb.scaleCache[nodeID].inv.y = FX_Inv(pResult->scaleEx0.y);
                    NNS_G3dRSOnGlb.scaleCache[nodeID].inv.z = FX_Inv(pResult->scaleEx0.z);
                }
            }
            else
            {
                // This node uses scaleEx. 
                // Update flags and the cache for child nodes. 
                // The cumulative scale up to this node is not 1. 
                NNSi_G3dBitVecReset(&NNS_G3dRS->isScaleCacheOne[0], nodeID);
                // Cache the cumulative scale and its inverse 
                pResult->scaleEx1.x = FX_Inv(pResult->scaleEx0.x);
                pResult->scaleEx1.y = FX_Inv(pResult->scaleEx0.y);
                pResult->scaleEx1.z = FX_Inv(pResult->scaleEx0.z);
                MI_CpuCopy32(&pResult->scaleEx0,
                             &NNS_G3dRSOnGlb.scaleCache[nodeID],
                             sizeof(VecFx32) + sizeof(VecFx32));
            }
        }
#endif // NNS_G3D_SI3D_DISABLE

#endif // FIX_SCALEEX_BLEND_BUG

        // Use cross product to determine line 3
        VEC_CrossProduct((VecFx32*)&pResult->rot._00,
                         (VecFx32*)&pResult->rot._10,
                         (VecFx32*)&pResult->rot._20);
        
#if 0
        VEC_Normalize((VecFx32*)&pResult->rot._00, (VecFx32*)&pResult->rot._00);
        VEC_Normalize((VecFx32*)&pResult->rot._20, (VecFx32*)&pResult->rot._20);
#else  // FIX
        if ((pResult->rot._00 == 0) &&
            (pResult->rot._01 == 0) &&
            (pResult->rot._02 == 0))
        {
            // The x-axis of the blend result was a zero vector. Restore to an unblended state.
            MI_CpuCopy32(&keepAxisX, &pResult->rot._00, sizeof(VecFx32));
        }
        else
        {
            VEC_Normalize((VecFx32*)&pResult->rot._00, (VecFx32*)&pResult->rot._00);
        }

        if ((pResult->rot._20 == 0) &&
            (pResult->rot._21 == 0) &&
            (pResult->rot._22 == 0))
        {
            // The Z-axis of the blend result was a zero vector. Restore to an unblended state.
            MI_CpuCopy32(&keepAxisZ, &pResult->rot._20, sizeof(VecFx32));
        }
        else
        {
            VEC_Normalize((VecFx32*)&pResult->rot._20, (VecFx32*)&pResult->rot._20);
        }
#endif

        // Orthagonalize all three axes
        VEC_CrossProduct((VecFx32*)&pResult->rot._20,
                         (VecFx32*)&pResult->rot._00,
                         (VecFx32*)&pResult->rot._10);

        return TRUE;
    }
}
#include <nitro/codereset.h>


/*---------------------------------------------------------------------------*
    NNSi_G3dAnmBlendVis

    Default visibility animation blend function.
    Get the logical sum of the result
 *---------------------------------------------------------------------------*/
BOOL NNSi_G3dAnmBlendVis(NNSG3dVisAnmResult* pResult,
                         const NNSG3dAnmObj* pAnmObj,
                         u32 nodeID)
{
    BOOL rval = FALSE;
    const NNSG3dAnmObj* p;
    NNSG3dVisAnmResult tmp;
    NNS_G3D_NULL_ASSERT(pResult);

    p = pAnmObj;
    pResult->isVisible = FALSE;
    do
    {
        if (nodeID < p->numMapData)
        {
            // Convert from nodeID to resource-internal ID
            u32 dataIdx = p->mapData[nodeID];

            if ((dataIdx & (NNS_G3D_ANMOBJ_MAPDATA_EXIST |
                            NNS_G3D_ANMOBJ_MAPDATA_DISABLED)) ==
                    NNS_G3D_ANMOBJ_MAPDATA_EXIST)
            {
                NNSG3dFuncAnmVis func = (NNSG3dFuncAnmVis)p->funcAnm;
                NNS_G3D_NULL_ASSERT(func);
                if (func)
                {
                    (*func)(&tmp,
                            p,
                            dataIdx & NNS_G3D_ANMOBJ_MAPDATA_DATAFIELD);
                    pResult->isVisible |= tmp.isVisible;
                    rval = TRUE;
                }
            }
        }
        p = p->next;
    }
    while(p);

    return rval;
}


////////////////////////////////////////////////////////////////////////////////
//
// Global variables
//

/*---------------------------------------------------------------------------*
    NNS_G3dAnmFmtNum

    Number of animation formats registered in NNS_G3dAnmObjInitFuncArray.
    When adding the animation format, increase this value and add an entry to NNS_G3dAnmObjInitFuncArray (must be NNS_G3D_ANMFMT_MAX or less.)
    
    
 *---------------------------------------------------------------------------*/
u32 NNS_G3dAnmFmtNum = 5;


/*---------------------------------------------------------------------------*
    NNS_G3dAnmObjInitFuncArray

    {anmCategory0, dummy, anmCategory1, initFunc}
 *---------------------------------------------------------------------------*/
NNSG3dAnmObjInitFunc NNS_G3dAnmObjInitFuncArray[NNS_G3D_ANMFMT_MAX] =
{
#ifndef NNS_G3D_NSBMA_DISABLE
    {'M', 0, 'MA', &NNSi_G3dAnmObjInitNsBma},
#else
    {'M', 0, 'MA', NULL},
#endif
#ifndef NNS_G3D_NSBTP_DISABLE
    {'M', 0, 'TP', &NNSi_G3dAnmObjInitNsBtp},
#else
    {'M', 0, 'TP', NULL},
#endif
#ifndef NNS_G3D_NSBTA_DISABLE
    {'M', 0, 'TA', &NNSi_G3dAnmObjInitNsBta},
#else
    {'M', 0, 'TA', NULL},
#endif
#ifndef NNS_G3D_NSBVA_DISABLE
    {'V', 0, 'VA', &NNSi_G3dAnmObjInitNsBva},
#else
    {'V', 0, 'VA', NULL},
#endif
#ifndef NNS_G3D_NSBCA_DISABLE
    {'J', 0, 'CA', &NNSi_G3dAnmObjInitNsBca}
#else
    {'J', 0, 'CA', NULL}
#endif
};

