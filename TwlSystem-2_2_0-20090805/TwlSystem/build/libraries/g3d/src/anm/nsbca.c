/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d - src - anm
  File:     nsbca.c

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

#include <nnsys/g3d/anm/nsbca.h>
#include <nnsys/g3d/sbc.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>

#ifndef NNS_G3D_NSBCA_DISABLE
/*---------------------------------------------------------------------------*
    NNSi_G3dAnmObjInitNsBca

    Initializes NNSG3dAnmObj for the .nsbca resource.
    This is called from NNS_G3dInitAnmObj.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dAnmObjInitNsBca(NNSG3dAnmObj* pAnmObj,
                        void* pResAnm,
                        const NNSG3dResMdl* pResMdl)
{
    u32 i;
    u16* ofsArray;
    NNSG3dResJntAnm* jntAnm;
    const NNSG3dResNodeInfo* jnt;

    NNS_G3D_NULL_ASSERT(pAnmObj);
    NNS_G3D_NULL_ASSERT(pResAnm);
    NNS_G3D_NULL_ASSERT(pResMdl);

    pAnmObj->resAnm = pResAnm;
    jntAnm = (NNSG3dResJntAnm*)pResAnm;
    jnt = NNS_G3dGetNodeInfo(pResMdl);
    pAnmObj->funcAnm = NNS_G3dFuncAnmJntNsBcaDefault;
    pAnmObj->numMapData = pResMdl->info.numNode;

    
    // zero out the mapData first
    MI_CpuClear16(&pAnmObj->mapData[0], sizeof(u16) * pAnmObj->numMapData);

    //
    // The nodeID of the model is stored in NNSG3dJntAnmSRTTag.
    //
    ofsArray = (u16*)((u8*)jntAnm + sizeof(NNSG3dResJntAnm));

    // Check that the size of the mapData array, allocated by the animation object, is not exceeded
    NNS_G3D_ASSERT(jntAnm->numNode <= pAnmObj->numMapData);
    
    for (i = 0; i < jntAnm->numNode; ++i)
    {
        // For .nsbca, the index of the current resource and the node ID of the model are identical.
        NNSG3dResJntAnmSRTTag* pTag =
            (NNSG3dResJntAnmSRTTag*)((u8*)jntAnm + ofsArray[i]);
        pAnmObj->mapData[i] = (u16)((pTag->tag >> NNS_G3D_JNTANM_SRTINFO_NODE_SHIFT) |
                                    NNS_G3D_ANMOBJ_MAPDATA_EXIST);
    }
}


static void
getJntSRTAnmResult_(const NNSG3dResJntAnm* pJntAnm, 
                    u32                    dataIdx, 
                    fx32                   Frame,
                    NNSG3dJntAnmResult*    pResult);

static void
getTransData_(fx32* pVal,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm);

static void
getTransDataEx_(fx32* pVal,
                fx32 Frame,
                const u32* pData,
                const NNSG3dResJntAnm* pJntAnm);

static void
getScaleData_(fx32* s_invs,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm);

static void
getScaleDataEx_(fx32* s_invs,
                fx32 Frame,
                const u32* pData,
                const NNSG3dResJntAnm* pJntAnm);


static void
getRotData_(MtxFx33* pRot,
            fx32 Frame,
            const u32* pData,
            const NNSG3dResJntAnm* pJntAnm);

static void
getRotDataEx_(MtxFx33* pRot,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm);

static BOOL
getRotDataByIdx_(MtxFx33* pRot,
                 const void* pArrayRot3,
                 const void* pArrayRot5,
                 NNSG3dJntAnmRIdx info);


/*---------------------------------------------------------------------------*
    NNSi_G3dAnmCalcNsBca

    pResult: stores the result of the joint animation
    pAnmObj:
    dataIdx: Index that indicates the storage location for the data in the resource.
 *---------------------------------------------------------------------------*/
void NNSi_G3dAnmCalcNsBca(NNSG3dJntAnmResult* pResult,
                          const NNSG3dAnmObj* pAnmObj,
                          u32 dataIdx)
{
    NNS_G3D_NULL_ASSERT(pResult);
    NNS_G3D_NULL_ASSERT(pAnmObj);

    //
    // how to implement joint animation
    // 
    // This function is designed to set the following:
    // pResult->flag,
    // pResult->trans,
    // pResult->rot.
    //  
    // As for pResult->flag, be sure to set it when:
    // NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO
    // NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO
    // NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE
    // are not rotating or moving.
    //
    // pResult->scale, pResult->scaleEx0, and pResult->scaleEx1 can be calculated by preparing arguments and calling (*NNS_G3dRS->funcJntScale).
    // 
    // The remaining flags of pResult->flag are also set here.
    // These configurations are due to the fact that separate calculations must be made for each CG tool.
    // In practical terms, one of the following is called: NNS_G3dGetJointScaleBasic, NNS_G3dGetJointScaleMaya, or NNS_G3dGetJointScaleSi3d.
    // 
    // 
    // 
    // The arguments are along the lines of
    // void NNS_G3dGetJointScaleBasic(NNSG3dJntAnmResult* pResult,
    //                                const fx32* p,
    //                                const u8* cmd,
    //                                u32 srtflag)
    // and will process pResult.
    // Now we will explain the 2nd and subsequent arguments:
    // p is a six-word array that holds the scale and the scale reciprocal for the joint.
    // NNS_G3dRS->c goes into cmd.
    // A flag that will match NNSG3dSRTFlag (as defined in res_struct.h) should be built and inserted into srtflag.
    // 
    
    {
        fx32 frame;
        NNSG3dResJntAnm* anm = (NNSG3dResJntAnm*)(pAnmObj->resAnm);

        if (pAnmObj->frame >= (anm->numFrame << FX32_SHIFT))
            frame = (anm->numFrame << FX32_SHIFT) - 1;
        else if (pAnmObj->frame < 0)
            frame = 0;
        else
            frame = pAnmObj->frame;
        
        //
        // Insert the results from the frame # resource for the dataIdx # joint animation resource, pAnm, into result.
        // 
        //

        getJntSRTAnmResult_(anm,
                            dataIdx,
                            frame,
                            pResult);

    }
}


/*---------------------------------------------------------------------------*
    vecCross_

    For a rotation matrix, there is no need to cast the cross product to fx64 since the absolute value won't exceed FX32_ONE.
    
 *---------------------------------------------------------------------------*/
#include <nitro/code32.h>
static NNS_G3D_INLINE void
vecCross_(const VecFx32 * a, const VecFx32 * b, VecFx32 * axb)
{
    fx32 x, y, z;
    NNS_G3D_NULL_ASSERT(a);
    NNS_G3D_NULL_ASSERT(b);
    NNS_G3D_NULL_ASSERT(axb);

    x = (a->y * b->z - a->z * b->y) >> FX32_SHIFT;
    y = (a->z * b->x - a->x * b->z) >> FX32_SHIFT;
    z = (a->x * b->y - a->y * b->x) >> FX32_SHIFT;

    axb->x = x;
    axb->y = y;
    axb->z = z;
}
#include <nitro/codereset.h>


/*---------------------------------------------------------------------------*
    getMdlTrans_

    use the translation of the model
 *---------------------------------------------------------------------------*/
static void 
getMdlTrans_(NNSG3dJntAnmResult* pResult)
{
    u32 idxNode;
    const NNSG3dResNodeData* pNd;
    NNS_G3D_NULL_ASSERT(NNS_G3dRS);

    // HACK ALERT
    // internal to the NODEDESC command, so get the data from there
    NNS_G3D_ASSERT(NNS_G3D_GET_SBCCMD(*NNS_G3dRS->c) == NNS_G3D_SBC_NODEDESC);
    idxNode = *(NNS_G3dRS->c + 1);
    pNd = NNS_G3dGetNodeDataByIdx(NNS_G3dRS->pResNodeInfo, idxNode);
    
    // Translation
    if (pNd->flag & NNS_G3D_SRTFLAG_TRANS_ZERO)
    {
        // Control memory access and subsequent processing costs by simply raising a flag instead of writing pResult to trans.
        // 
        pResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO;
    }
    else
    {
        // According to the flag value, NNSG3dResNodeData will have data at the end.
        // Refer to the definition of NNSG3dResNodeData in res_struct.h.
        const fx32* p_fx32 = (const fx32*)((const u8*)pNd + sizeof(NNSG3dResNodeData));
        
        pResult->trans.x = *(p_fx32 + 0);
        pResult->trans.y = *(p_fx32 + 1);
        pResult->trans.z = *(p_fx32 + 2);
    }
}


/*---------------------------------------------------------------------------*
    getMdlScale_

    use the scale of the model
 *---------------------------------------------------------------------------*/
static void
getMdlScale_(NNSG3dJntAnmResult* pResult)
{
    u32 idxNode;
    const NNSG3dResNodeData* pNd;
    const u8* p;
    NNS_G3D_NULL_ASSERT(NNS_G3dRS);
    NNS_G3D_NULL_ASSERT(NNS_G3dRS->funcJntScale);

    // HACK ALERT
    // internal to the NODEDESC command, so get the data from there
    NNS_G3D_ASSERT(NNS_G3D_GET_SBCCMD(*NNS_G3dRS->c) == NNS_G3D_SBC_NODEDESC);
    idxNode = *(NNS_G3dRS->c + 1);

    // According to the flag value, NNSG3dResNodeData will have data at the end.
    // Refer to the definition of NNSG3dResNodeData in res_struct.h.
    pNd = NNS_G3dGetNodeDataByIdx(NNS_G3dRS->pResNodeInfo, idxNode);
    p = (const u8*)pNd + sizeof(*pNd);
    
    // Translation
    if (!(pNd->flag & NNS_G3D_SRTFLAG_TRANS_ZERO))
    {
        // skip Tx, Ty, and Tz
        p += 3 * sizeof(fx32);
    }

    // Rotation
    if (!(pNd->flag & NNS_G3D_SRTFLAG_ROT_ZERO))
    {
        // skip rotation data
        if (pNd->flag & NNS_G3D_SRTFLAG_PIVOT_EXIST)
        {
            // using a pivoted format, so skip A and B
            p += 2 * sizeof(fx16);
        }
        else
        {
            // skip _01, _02, and so on
            p += 8 * sizeof(fx16);
        }
    }

    // Scale
    (*NNS_G3dRS->funcJntScale)(pResult, (const fx32*)p, NNS_G3dRS->c, pNd->flag);
}


// Get the data from Rot3.
// The first index in the array storing the positions of non-zero, non-pivot elements will be the pivot position.
// 
static const u8 pivotUtil_[9][4] =
{
    {4, 5, 7, 8},
    {3, 5, 6, 8},
    {3, 4, 6, 7},

    {1, 2, 7, 8},
    {0, 2, 6, 8},
    {0, 1, 6, 7},

    {1, 2, 4, 5},
    {0, 2, 3, 5},
    {0, 1, 3, 4}
};


/*---------------------------------------------------------------------------*
    getMdlRot_

    use the rotation of the model
 *---------------------------------------------------------------------------*/
static void
getMdlRot_(NNSG3dJntAnmResult* pResult)
{
    u32 idxNode;
    const NNSG3dResNodeData* pNd;
    const u8* p;
    NNS_G3D_NULL_ASSERT(NNS_G3dRS);

    // HACK ALERT
    // internal to the NODEDESC command, so get the data from there
    NNS_G3D_ASSERT(NNS_G3D_GET_SBCCMD(*NNS_G3dRS->c) == NNS_G3D_SBC_NODEDESC);
    idxNode = *(NNS_G3dRS->c + 1);

    // According to the flag value, NNSG3dResNodeData will have data at the end.
    // Refer to the definition of NNSG3dResNodeData in res_struct.h.
    pNd = NNS_G3dGetNodeDataByIdx(NNS_G3dRS->pResNodeInfo, idxNode);
    p = (const u8*)pNd + sizeof(*pNd);
    
    // Translation
    if (!(pNd->flag & NNS_G3D_SRTFLAG_TRANS_ZERO))
    {
        // skip Tx, Ty, and Tz
        p += 3 * sizeof(fx32);
    }

    // Rotation
    if (!(pNd->flag & NNS_G3D_SRTFLAG_ROT_ZERO))
    {
        if (pNd->flag & NNS_G3D_SRTFLAG_PIVOT_EXIST)
        {
            // When compressed (mainly for single-axis rotations), the compressed matrix is restored.
            // 
            fx32 A = *(fx16*)(p + 0);
            fx32 B = *(fx16*)(p + sizeof(fx16));
            u32 idxPivot = (u32)( (pNd->flag & NNS_G3D_SRTFLAG_IDXPIVOT_MASK) >> 
                                            NNS_G3D_SRTFLAG_IDXPIVOT_SHIFT );
            // clear anmResult.rot
            MI_Zero36B(&pResult->rot);
            
            pResult->rot.a[idxPivot] =
                (pNd->flag & NNS_G3D_SRTFLAG_PIVOT_MINUS) ?
                    -FX32_ONE :
                    FX32_ONE;
            
            pResult->rot.a[pivotUtil_[idxPivot][0]] = A;
            pResult->rot.a[pivotUtil_[idxPivot][1]] = B;

            pResult->rot.a[pivotUtil_[idxPivot][2]] =
                (pNd->flag & NNS_G3D_SRTFLAG_SIGN_REVC) ? -B : B;

            pResult->rot.a[pivotUtil_[idxPivot][3]] =
                (pNd->flag & NNS_G3D_SRTFLAG_SIGN_REVD) ? -A : A;
        }
        else
        {
            // NOTICE:
            // do not replace in the memory copy API
            // this is because of the implicit casting from fx16 to fx32

            const fx16* pp = (const fx16*)p;

            // Set data in the 3x3 rotation matrix for pResult.
            pResult->rot.a[0] = pNd->_00;
            pResult->rot.a[1] = *(pp + 0);
            pResult->rot.a[2] = *(pp + 1);
            pResult->rot.a[3] = *(pp + 2);
            pResult->rot.a[4] = *(pp + 3);
            pResult->rot.a[5] = *(pp + 4);
            pResult->rot.a[6] = *(pp + 5);
            pResult->rot.a[7] = *(pp + 6);
            pResult->rot.a[8] = *(pp + 7);
        }
    }
    else
    {
        // Control memory access and subsequent processing costs by simply raising a flag instead of writing pResult to rot.
        // 
        pResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO;
    }
}


/*---------------------------------------------------------------------------*
    getJntSRTAnmResult_

    Sets the rotation, translation, and flag in pResult.
    Sets the scale and the scale reciprocal in pS_invS.
 *---------------------------------------------------------------------------*/
static void
getJntSRTAnmResult_(const NNSG3dResJntAnm* pJntAnm, 
                    u32                    dataIdx, 
                    fx32                   Frame,
                    NNSG3dJntAnmResult*    pResult)
{
    NNSG3dResJntAnmSRTTag* pAnmSRTTag;
    NNSG3dJntAnmSRTTag     tag;
    const u32*             pData;
    BOOL                   IsDecimalFrame;
    fx32                   pS_invS[6]; // changed pointer argument to local array

    // check the input
    NNS_G3D_NULL_ASSERT(pJntAnm);
    NNS_G3D_NULL_ASSERT(pResult);
    NNS_G3D_ASSERT(dataIdx < pJntAnm->numNode);
    NNS_G3D_ASSERT(Frame < (pJntAnm->numFrame << FX32_SHIFT));
    NNS_G3D_ASSERT(pJntAnm->anmHeader.category0 == 'J' &&
                   pJntAnm->anmHeader.category1 == 'CA');

    {
        const u16* ofsTag = (const u16*)((u8*) pJntAnm + sizeof(NNSG3dResJntAnm));
        // get the SRT tag of the dataIdx-th data inside the animation resource
        pAnmSRTTag = (NNSG3dResJntAnmSRTTag*)((u8*) pJntAnm + ofsTag[dataIdx]);

        tag = (NNSG3dJntAnmSRTTag)pAnmSRTTag->tag;
    }

    // check if no animation exists
    if (tag & NNS_G3D_JNTANM_SRTINFO_IDENTITY)
    {
        pResult->flag = (NNSG3dJntAnmResultFlag)(NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE |
                                                 NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO  |
                                                 NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO);
        goto GET_JOINTSCALE;
    }

    // The variable-length data in accordance with the pAnmSRTTag value is stored in the region following pAnmSRTTag.
    pData = (const u32*)((const u8*)pAnmSRTTag + sizeof(NNSG3dResJntAnmSRTTag));

    IsDecimalFrame = (BOOL)((Frame & (FX32_ONE - 1)) &&
                            (pJntAnm->flag & NNS_G3D_JNTANM_OPTION_INTERPOLATION));

    // reset first
    pResult->flag = (NNSG3dJntAnmResultFlag) 0;

    //
    // Refer to the comments about NNSG3dResJntAnmSRT in res_struct.h.
    //
    
    //
    // Get the Translation.
    //
    if (!(tag & (NNS_G3D_JNTANM_SRTINFO_IDENTITY_T | NNS_G3D_JNTANM_SRTINFO_BASE_T)))
    {
        // TX
        if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_TX))
        {
            // Call the function that extracts TX.
            // The data offset will be from pAnmSRTTag.
            if (IsDecimalFrame)
            {
                getTransDataEx_(&pResult->trans.x,
                                Frame,
                                pData,
                                pJntAnm);
            }
            else
            {
                getTransData_(&pResult->trans.x,
                              Frame,
                              pData,
                              pJntAnm);
            }

            // portion with NNSG3dJntAnmTInfo and the offset data to the array
            pData += 2;
        }
        else
        {
            pResult->trans.x = *(fx32*)pData;

            // portion with constant data
            pData += 1;
        }

        // TY
        if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_TY))
        {
            // Call the function that extracts TY.
            // The data offset will be from pAnmSRTTag.
            if (IsDecimalFrame)
            {
                getTransDataEx_(&pResult->trans.y,
                                Frame,
                                pData,
                                pJntAnm);
            }
            else
            {
                getTransData_(&pResult->trans.y,
                              Frame,
                              pData,
                              pJntAnm);
            }

            // portion with NNSG3dJntAnmTInfo and the offset data to the array
            pData += 2;
        }
        else
        {
            pResult->trans.y = *(fx32*)pData;

            // portion with constant data
            pData += 1;
        }

        // TZ
        if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_TZ))
        {
            // Call the function that extracts TZ
            // The data offset will be from pAnmSRTTag.
            if (IsDecimalFrame)
            {
                getTransDataEx_(&pResult->trans.z,
                                Frame,
                                pData,
                                pJntAnm);
            }
            else
            {
                getTransData_(&pResult->trans.z,
                              Frame,
                              pData,
                              pJntAnm);
            }

            // portion with NNSG3dJntAnmTInfo and the offset data to the array
            pData += 2;
        }
        else
        {
            pResult->trans.z = *(fx32*)pData;

            // portion with constant data
            pData += 1;
        }
    }
    else
    {
        if (tag & NNS_G3D_JNTANM_SRTINFO_IDENTITY_T)
        {
            // Trans = (0, 0, 0)
            pResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO;

            // pData does not need to be advanced because there is no succeeding data
        }
        else
        {
            // get the Trans of the model
            getMdlTrans_(pResult);
        }
    }

    //
    // get the Rotation
    //
    if (!(tag & (NNS_G3D_JNTANM_SRTINFO_IDENTITY_R | NNS_G3D_JNTANM_SRTINFO_BASE_R)))
    {
        if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_R))
        {
            // Call the function that extracts R.
            // The data offset is derived from either pJntAnm->ofsRot3 or pJntAnm->ofsRot5.
            // 
            if (IsDecimalFrame)
            {
                getRotDataEx_(&pResult->rot,
                              Frame,
                              pData,
                              pJntAnm);
            }
            else
            {
                getRotData_(&pResult->rot,
                            Frame,
                            pData,
                            pJntAnm);
            }

            // portion with NNSG3dJntAnmRInfo and the offset data to the array
            pData += 2;
        }
        else
        {
            // This code extracts the R of the const.
            if (getRotDataByIdx_(&pResult->rot,
                                 (void*)((u8*)pJntAnm + pJntAnm->ofsRot3),
                                 (void*)((u8*)pJntAnm + pJntAnm->ofsRot5),
                                 (NNSG3dJntAnmRIdx)*pData))
            {
                vecCross_((const VecFx32*)&pResult->rot._00,
                          (const VecFx32*)&pResult->rot._10,
                          (VecFx32*)&pResult->rot._20);
            }

            // portion with constant data
            pData += 1;
        }
    }
    else
    {
        if (tag & NNS_G3D_JNTANM_SRTINFO_IDENTITY_R)
        {
            // Rot = Identity
            pResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO;

            // pData does not need to be advanced because there is no subsequent data
        }
        else
        {
            // get the Rot of the model
            getMdlRot_(pResult);
        }
    }

    //
    // get the Scale
    //
    if (!(tag & (NNS_G3D_JNTANM_SRTINFO_IDENTITY_S | NNS_G3D_JNTANM_SRTINFO_BASE_S)))
    {
        if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_SX))
        {
            // Call the function that extracts SX.
            // The data offset will be from pAnmSRTTag.
            fx32 sx_invsx[2];
            if (IsDecimalFrame)
            {
                getScaleDataEx_(&sx_invsx[0],
                                Frame,
                                pData,
                                pJntAnm);
            }
            else
            {
                getScaleData_(&sx_invsx[0],
                              Frame,
                              pData,
                              pJntAnm);
            }
            *(pS_invS + 0) = sx_invsx[0];
            *(pS_invS + 3) = sx_invsx[1];
        }
        else
        {
            const fx32* p_fx32 = (const fx32*)pData;

            *(pS_invS + 0) = *(p_fx32 + 0);
            *(pS_invS + 3) = *(p_fx32 + 1);
        }

        // portion with NNSG3dJntAnmSInfo and the offset data to the array
        pData += 2;

        if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_SY))
        {
            // Call the function that extracts SY.
            // The data offset will be from pAnmSRTTag.
            fx32 sy_invsy[2];
            if (IsDecimalFrame)
            {
                getScaleDataEx_(&sy_invsy[0],
                                Frame,
                                pData,
                                pJntAnm);
            }
            else
            {
                getScaleData_(&sy_invsy[0],
                              Frame,
                              pData,
                              pJntAnm);
            }
            *(pS_invS + 1) = sy_invsy[0];
            *(pS_invS + 4) = sy_invsy[1];
        }
        else
        {
            const fx32* p_fx32 = (const fx32*)pData;

            *(pS_invS + 1) = *(p_fx32 + 0);
            *(pS_invS + 4) = *(p_fx32 + 1);

        }

        // portion with NNSG3dJntAnmSInfo and the offset data to the array
        pData += 2;

        if (!(tag & NNS_G3D_JNTANM_SRTINFO_CONST_SZ))
        {
            // Call the function that extracts SZ.
            // The data offset will be from pAnmSRTTag.
            fx32 sz_invsz[2];
            if (IsDecimalFrame)
            {
                getScaleDataEx_(&sz_invsz[0],
                                Frame,
                                pData,
                                pJntAnm);
            }
            else
            {
                getScaleData_(&sz_invsz[0],
                              Frame,
                              pData,
                              pJntAnm);
            }
            *(pS_invS + 2) = sz_invsz[0];
            *(pS_invS + 5) = sz_invsz[1];
        }
        else
        {
            const fx32* p_fx32 = (const fx32*)pData;

            *(pS_invS + 2) = *(p_fx32 + 0);
            *(pS_invS + 5) = *(p_fx32 + 1);
        }

        // portion with NNSG3dJntAnmSInfo and the offset data to the array
        pData += 2;
    }
    else
    {
        if (tag & NNS_G3D_JNTANM_SRTINFO_IDENTITY_S)
        {
            // Scale = (1, 1, 1)
            pResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE;

            // pData does not need to be advanced because there is no subsequent data
        }
        else
        {
            // get the Scale of the model
            getMdlScale_(pResult);

            // Return because the NNSG3dJntAnmResult scaling information is configured within getMdlScale_.
            // 
            return;
        }
    }
GET_JOINTSCALE:
    //
    // set the NNSG3dJntAnmResult scaling info
    //
    (*NNS_G3dRS->funcJntScale)(
        pResult, 
        pS_invS,
        NNS_G3dRS->c,
        ((pResult->flag & NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE) ?
                (u32)NNS_G3D_SRTFLAG_SCALE_ONE :
                0) // need to change the flag
    );
}


/*---------------------------------------------------------------------------*
    getTransData_

    Get the translation data and put it into *pVal.
 *---------------------------------------------------------------------------*/
static void
getTransData_(fx32* pVal,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm)
{
    u32 frame = (u32)FX_Whole(Frame);
    const void* pArray = (const void*)((const u8*)pJntAnm + *(pData + 1));
    NNSG3dJntAnmTInfo info = (NNSG3dJntAnmTInfo)*pData;
    u32 last_interp;
    u32 idx;
    u32 idx_sub;

    NNS_G3D_NULL_ASSERT(pVal);
    NNS_G3D_NULL_ASSERT(pArray);

    if (!(info & NNS_G3D_JNTANM_TINFO_STEP_MASK))
    {
        // NNS_G3D_JNTANM_TINFO_STEP1 is selected
        idx = frame;
        goto TRANS_NONINTERP;
    }

    // data is input in each frame from last_interp
    // last_interp is either a multiple of 2 or 4
    last_interp = ((u32)info & NNS_G3D_JNTANM_TINFO_LAST_INTERP_MASK) >>
                                    NNS_G3D_JNTANM_TINFO_LAST_INTERP_SHIFT;

    if (info & NNS_G3D_JNTANM_TINFO_STEP_2)
    {
        // NNS_G3D_JNTANM_TINFO_STEP_2 is selected
        if (frame & 1)
        {
            if (frame > last_interp)
            {
                // impossible if anything other than the last frame
                idx = (last_interp >> 1) + 1;
                goto TRANS_NONINTERP;
            }
            else
            {
                // Since the final frame is not on an odd number, 50:50 interpolation is needed
                idx = frame >> 1;
                goto TRANS_INTERP_2;
            }
        }
        else
        {
            // even frame, so no interpolation is needed
            idx = frame >> 1;
            goto TRANS_NONINTERP;
        }
    }
    else
    {
        // NNS_G3D_JNTANM_TINFO_STEP_4 is selected
        if (frame & 3)
        {
            if (frame > last_interp)
            {
                idx = (last_interp >> 2) + (frame & 3);
                goto TRANS_NONINTERP;
            }

            // with interpolation management
            if (frame & 1)
            {
                fx32 v, v_sub;
                if (frame & 2)
                {
                    // interpolate with 3:1 position
                    idx_sub = (frame >> 2);
                    idx = idx_sub + 1;
                }
                else
                {
                    // interpolate with 1:3 position
                    idx = (frame >> 2);
                    idx_sub = idx + 1;
                }
                
                // interpolation for when 1:3 and 3:1
                if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
                {
                    const fx16* p_fx16 = (const fx16*)pArray;

                    v = *(p_fx16 + idx);
                    v_sub = *(p_fx16 + idx_sub);
                    *pVal = (v + v + v + v_sub) >> 2; // a 3:1 blend of v and v_sub
                }
                else
                {
                    const fx32* p_fx32 = (const fx32*)pArray;

                    v = *(p_fx32 + idx);
                    v_sub = *(p_fx32 + idx_sub);
                    // A 3:1 blend of v and v_sub. Calculated using fx64 to avoid overflow errors.
                    *pVal = (fx32)(((fx64)v + v + v + v_sub) >> 2);
                }
                return;
            }
            else
            {
                // becomes 50:50 interpolation
                idx = frame >> 2;
                goto TRANS_INTERP_2;
            }
        }
        else
        {
            // The frame is set to be an exact multiple of 4.
            idx = frame >> 2;
            goto TRANS_NONINTERP;
        }
    }
    NNS_G3D_ASSERT(0);
TRANS_INTERP_2:
    if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
    {
        const fx16* p_fx16 = (const fx16*)pArray;

        *pVal = (*(p_fx16 + idx) + *(p_fx16 + idx + 1)) >> 1;
    }
    else
    {
        const fx32* p_fx32 = (const fx32*)pArray;

        fx32 v1 = *(p_fx32 + idx) >> 1;
        fx32 v2 = *(p_fx32 + idx + 1) >> 1;
        *pVal = v1 + v2;
    }
    return;
TRANS_NONINTERP:
    if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
    {
        *pVal = *((const fx16*)pArray + idx);
    }
    else
    {
        *pVal = *((const fx32*)pArray + idx);
    }
    return;
}


/*---------------------------------------------------------------------------*
    getTransDataEx_

    Get the translation data, and put it in *pVal.
    Interpolate if it has a fraction.
 *---------------------------------------------------------------------------*/
static void
getTransDataEx_(fx32* pVal,
                fx32 Frame,
                const u32* pData,
                const NNSG3dResJntAnm* pJntAnm)
{
    // Only frames that fall within the range (0 < Frame < numFrame) can arrive at this function.
    // 
    const void* pArray = (const void*)((const u8*)pJntAnm + *(pData + 1));
    NNSG3dJntAnmTInfo info = (NNSG3dJntAnmTInfo)*pData;

    u32 last_interp;
    u32 idx;
    fx32 remainder;
    int step;
    u32 step_shift;
    u32 frame;

    NNS_G3D_NULL_ASSERT(pVal);
    NNS_G3D_NULL_ASSERT(pArray);

    frame = (u32)FX_Whole(Frame);

    if (frame == pJntAnm->numFrame - 1)
    {
        //
        // when the frame is between numFrame - 1 and numFrame
        // when exactly between the first and last frame
        //

        //
        // first, get the final frame data index
        //
        if (!(info & NNS_G3D_JNTANM_TINFO_STEP_MASK))
        {
            idx = frame;
        }
        else if (info & NNS_G3D_JNTANM_TINFO_STEP_2)
        {
            idx = (frame >> 1) + (frame & 1);
        }
        else
        {
            idx = (frame >> 2) + (frame & 3);
        }

        //
        // a flag determines whether to extrapolate or to return the final data
        //
        if (pJntAnm->flag & NNS_G3D_JNTANM_OPTION_END_TO_START_INTERPOLATION)
        {
            // when extrapolating, do it with the first and last data (loop)
            fx32 v0, v1;
            remainder = Frame & (FX32_ONE - 1);

            if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
            {
                const fx16* p_fx16 = (const fx16*)pArray;

                v0 = *(p_fx16 + idx);
                v1 = *p_fx16;
            }
            else
            {
                const fx32* p_fx32 = (const fx32*)pArray;

                v0 = *(p_fx32 + idx);
                v1 = *p_fx32;
            }

            *pVal = v0 + (((v1 - v0) * remainder) >> FX32_SHIFT);
            return;
        }
        else
        {
            // when returning the final data
            if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
            {
                *pVal = *((const fx16*)pArray + idx);
            }
            else
            {
                *pVal = *((const fx32*)pArray + idx);
            }
            return;
        }
    }

    if (!(info & NNS_G3D_JNTANM_TINFO_STEP_MASK))
    {
        // NNS_G3D_JNTANM_TINFO_STEP1 is selected
        goto TRANS_EX_0;
    }

    // data is input in each frame from last_interp
    // last_interp is either a multiple of 2 or 4
    last_interp = ((u32)info & NNS_G3D_JNTANM_TINFO_LAST_INTERP_MASK) >>
                                    NNS_G3D_JNTANM_TINFO_LAST_INTERP_SHIFT;
    
    if (info & NNS_G3D_JNTANM_TINFO_STEP_2)
    {
        // NNS_G3D_JNTANM_TINFO_STEP_2 is selected
        // The inclusion of the sign is due to consideration of the discarded data.
        if (frame >= last_interp)
        {
            // impossible if anything other than the last frame
            idx = (last_interp >> 1);
            goto TRANS_EX_0_1;
        }
        else
        {
            idx = frame >> 1;
            remainder = Frame & (FX32_ONE * 2 - 1);
            step = 2;
            step_shift = 1;
            goto TRANS_EX;
        }
    }
    else
    {
        // NNS_G3D_JNTANM_TINFO_STEP_4 is selected
        if (frame >= last_interp)
        {
            // Based on the conditions with which this function is called, frame is not the final data. As such, the following code ensures it remains with the boundaries.
            // 
            idx = (frame >> 2) + (frame & 3);
            goto TRANS_EX_0_1;
        }
        else
        {
            idx = frame >> 2;
            remainder = Frame & (FX32_ONE * 4 - 1);
            step = 4;
            step_shift = 2;
            goto TRANS_EX;
        }
    }
    NNS_G3D_ASSERT(0);
TRANS_EX_0:
    idx = (u32)frame;
TRANS_EX_0_1:
    remainder = Frame & (FX32_ONE - 1);
    step = 1;
    step_shift = 0;
TRANS_EX:
    {
        fx32 v0, v1;
        if (info & NNS_G3D_JNTANM_TINFO_FX16ARRAY)
        {
            const fx16* p_fx16 = (const fx16*)pArray;

            v0 = *(p_fx16 + idx);
            v1 = *(p_fx16 + idx + 1);
        }
        else
        {
            const fx32* p_fx32 = (const fx32*)pArray;

            v0 = *(p_fx32 + idx);
            v1 = *(p_fx32 + idx + 1);
        }

        *pVal = ((v0 * step) + (((v1 - v0) * remainder) >> FX32_SHIFT)) >> step_shift;
    }
    return;
}


/*---------------------------------------------------------------------------*
    getScaleData_

    Get the scale and its inverse, then put them in s_invs[0] and s_invs[1].
 *---------------------------------------------------------------------------*/
static void
getScaleData_(fx32* s_invs,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm)
{
    u32 frame = (u32)FX_Whole(Frame);
    const void* pArray = (const void*)((u8*)pJntAnm + *(pData + 1));
    NNSG3dJntAnmSInfo info = (NNSG3dJntAnmSInfo)*pData;
    u32 last_interp;
    u32 idx;
    u32 idx_sub;

    NNS_G3D_NULL_ASSERT(s_invs);
    NNS_G3D_NULL_ASSERT(pArray);

    if (!(info & NNS_G3D_JNTANM_SINFO_STEP_MASK))
    {
        // NNS_G3D_JNTANM_SINFO_STEP_1 is selected
        idx = frame;
        goto SCALE_NONINTERP;
    }

    // data is input in each frame from last_interp
    // last_interp is either a multiple of 2 or 4
    last_interp = ((u32)info & NNS_G3D_JNTANM_SINFO_LAST_INTERP_MASK) >>
                                    NNS_G3D_JNTANM_SINFO_LAST_INTERP_SHIFT;

    if (info & NNS_G3D_JNTANM_SINFO_STEP_2)
    {
        // NNS_G3D_JNTANM_SINFO_STEP_2 is selected
        if (frame & 1)
        {
            if (frame > last_interp)
            {
                // impossible if anything other than the last frame
                idx = (last_interp >> 1) + 1;
                goto SCALE_NONINTERP;
            }
            else
            {
                // Since the final frame is not on an odd number, 50:50 interpolation is needed
                idx = frame >> 1;
                goto SCALE_INTERP_2;
            }
        }
        else
        {
            // even frame, so no interpolation is needed
            idx = frame >> 1;
            goto SCALE_NONINTERP;
        }
    }
    else
    {
        // NNS_G3D_JNTANM_TINFO_STEP_4 is selected
        if (frame & 3)
        {
            if (frame > last_interp)
            {
                // no interpolation starting from last_interp
                idx = (last_interp >> 2) + (frame & 3);
                goto SCALE_NONINTERP;
            }

            // with interpolation management
            if (frame & 1)
            {
                fx32 v, v_sub;
                if (frame & 2)
                {
                    // interpolate with 3:1 position
                    idx_sub = (frame >> 2);
                    idx = idx_sub + 1;
                }
                else
                {
                    // interpolate with 1:3 position
                    idx = (frame >> 2);
                    idx_sub = idx + 1;
                }
                
                if (info & NNS_G3D_JNTANM_SINFO_FX16ARRAY)
                {
                    const fx16* p_fx16 = (const fx16*)pArray;

                    // scale calculation
                    v = *(p_fx16 + 2 * idx);
                    v_sub = *(p_fx16 + 2 * idx_sub);
                    s_invs[0] = (v + (v << 1) + v_sub) >> 2; // a 3:1 blend of v and v_sub

                    // inverse scale calculation
                    v = *(p_fx16 + 2 * idx + 1);
                    v_sub = *(p_fx16 + 2 * idx_sub + 1);
                    s_invs[1] = (v + (v << 1) + v_sub) >> 2; // a 3:1 blend of v and v_sub
                }
                else
                {
                    const fx32* p_fx32 = (const fx32*)pArray;

                    // scale calculation
                    v = *(p_fx32 + 2 * idx);
                    v_sub = *(p_fx32 + 2 * idx_sub);
                    // A 3:1 blend of v and v_sub. Calculated using fx64 to avoid overflow errors.
                    s_invs[0] = (fx32)(((fx64)v + v + v + v_sub) >> 2);

                    // inverse scale calculation
                    v = *(p_fx32 + 2 * idx + 1);
                    v_sub = *(p_fx32 + 2 * idx_sub + 1);
                    // A 3:1 blend of v and v_sub. Calculated using fx64 to avoid overflow errors.
                    s_invs[1] = (fx32)(((fx64)v + v + v + v_sub) >> 2);
                }
                return;
            }
            else
            {
                // becomes 50:50 interpolation
                idx = frame >> 2;
                goto SCALE_INTERP_2;
            }
        }
        else
        {
            // The frame is set to be an exact multiple of 4.
            idx = frame >> 2;
            goto SCALE_NONINTERP;
        }
    }
SCALE_NONINTERP:
    if (info & NNS_G3D_JNTANM_SINFO_FX16ARRAY)
    {
        const fx16* p_fx16 = (const fx16*)pArray;

        s_invs[0] = *(p_fx16 + 2 * idx);
        s_invs[1] = *(p_fx16 + 2 * idx + 1);
    }
    else
    {
        const fx32* p_fx32 = (const fx32*)pArray;

        s_invs[0] = *(p_fx32 + 2 * idx);
        s_invs[1] = *(p_fx32 + 2 * idx + 1);
    }
    return;
SCALE_INTERP_2:
    if (info & NNS_G3D_JNTANM_SINFO_FX16ARRAY)
    {
        const fx16* p_fx16 = (const fx16*)pArray;

        s_invs[0] = (*(p_fx16 + 2 * idx) + *(p_fx16 + 2 * idx + 2)) >> 1;
        s_invs[1] = (*(p_fx16 + 2 * idx + 1) + *(p_fx16 + 2 * idx + 3)) >> 1;
    }
    else
    {
        const fx32* p_fx32 = (const fx32*)pArray;

        s_invs[0] = (*(p_fx32 + 2 * idx) + (*(p_fx32 + 2 * idx + 2))) >> 1;
        s_invs[1] = (*(p_fx32 + 2 * idx + 1) + (*(p_fx32 + 2 * idx + 3))) >> 1;
    }
    return;
}


/*---------------------------------------------------------------------------*
    getScaleDataEx_

    Get the scale and its inverse, then put them in s_invs[0] and s_invs[1].
    Interpolate if it has a fraction.
 *---------------------------------------------------------------------------*/
static void
getScaleDataEx_(fx32* s_invs,
                fx32 Frame,
                const u32* pData,
                const NNSG3dResJntAnm* pJntAnm)
{
    const void* pArray = (const void*)((const u8*)pJntAnm + *(pData + 1));
    NNSG3dJntAnmSInfo info = (NNSG3dJntAnmSInfo)*pData;
    u32 last_interp;
    u32 idx0, idx1;
    fx32 remainder;
    int step;
    u32 step_shift;
    u32 frame;

    frame = (u32)FX_Whole(Frame);

    if (frame == pJntAnm->numFrame - 1)
    {
        //
        // when the frame is between numFrame - 1 and numFrame
        // when exactly between the first and last frame
        //

        //
        // first, get the final frame data index
        //
        if (!(info & NNS_G3D_JNTANM_SINFO_STEP_MASK))
        {
            idx0 = frame;
        }
        else if (info & NNS_G3D_JNTANM_SINFO_STEP_2)
        {
            idx0 = (frame >> 1) + (frame & 1);
        }
        else
        {
            idx0 = (frame >> 2) + (frame & 3);
        }

        //
        // a flag determines whether to extrapolate or to return the final data
        //
        if (pJntAnm->flag & NNS_G3D_JNTANM_OPTION_END_TO_START_INTERPOLATION)
        {
            idx1 = 0;
            goto SCALE_EX_0_1;
        }
        else
        {
            if (info & NNS_G3D_JNTANM_SINFO_FX16ARRAY)
            {
                const fx16* p_fx16 = (const fx16*)pArray;

                s_invs[0] = *(p_fx16 + 2 * idx0);
                s_invs[1] = *(p_fx16 + 2 * idx0 + 1);
            }
            else
            {
                const fx32* p_fx32 = (const fx32*)pArray;

                s_invs[0] = *(p_fx32 + 2 * idx0);
                s_invs[1] = *(p_fx32 + 2 * idx0 + 1);
            }
            return;
        }
    }

    if (!(info & NNS_G3D_JNTANM_SINFO_STEP_MASK))
    {
        // NNS_G3D_JNTANM_SINFO_STEP_1 is selected
        goto SCALE_EX_0;
    }

    last_interp = ((u32)info & NNS_G3D_JNTANM_SINFO_LAST_INTERP_MASK) >>
                                    NNS_G3D_JNTANM_SINFO_LAST_INTERP_SHIFT;

    if (info & NNS_G3D_JNTANM_SINFO_STEP_2)
    {
        // NNS_G3D_JNTANM_SINFO_STEP_2 is selected
        // The inclusion of the sign is due to consideration of the discarded data.
        if (frame >= last_interp)
        {
            // impossible if anything other than the last frame
            idx0 = (last_interp >> 1);
            idx1 = idx0 + 1;
            goto SCALE_EX_0_1;
        }
        else
        {
            idx0 = frame >> 1;
            idx1 = idx0 + 1;
            remainder = Frame & (FX32_ONE * 2 - 1);
            step = 2;
            step_shift = 1;
            goto SCALE_EX;
        }
    }
    else
    {
        // NNS_G3D_JNTANM_SINFO_STEP_4 is selected
        if (frame >= last_interp)
        {
            // Based on the conditions with which this function is called, frame is not the final data. As such, the following code ensures it remains with the boundaries.
            // 
            idx0 = (frame >> 2) + (frame & 3);
            idx1 = idx0 + 1;
            goto SCALE_EX_0_1;
        }
        else
        {
            idx0 = frame >> 2;
            idx1 = idx0 + 1;
            remainder = Frame & (FX32_ONE * 4 - 1);
            step = 4;
            step_shift = 2;
            goto SCALE_EX;
        }
    }
    NNS_G3D_ASSERT(0);
SCALE_EX_0:
    idx0 = (u32)frame;
    idx1 = idx0 + 1;
SCALE_EX_0_1:
    remainder = Frame & (FX32_ONE - 1);
    step = 1;
    step_shift = 0;
SCALE_EX:
    {
        fx32 s0, s1;
        fx32 inv0, inv1;

        if (info & NNS_G3D_JNTANM_SINFO_FX16ARRAY)
        {
            const fx16* p_fx16 = (const fx16*)pArray;

            s0   = *(p_fx16 + 2 * idx0);
            inv0 = *(p_fx16 + 2 * idx0 + 1);

            s1   = *(p_fx16 + 2 * idx1);
            inv1 = *(p_fx16 + 2 * idx1 + 1);
        }
        else
        {
            const fx32* p_fx32 = (const fx32*)pArray;

            s0   = *(p_fx32 + 2 * idx0);
            inv0 = *(p_fx32 + 2 * idx0 + 1);

            s1   = *(p_fx32 + 2 * idx1);
            inv1 = *(p_fx32 + 2 * idx1 + 1);
        }

        s_invs[0] = ((s0 * step) + (((s1 - s0) * remainder) >> FX32_SHIFT)) >> step_shift;
        s_invs[1] = ((inv0 * step) + (((inv1 - inv0) * remainder) >> FX32_SHIFT)) >> step_shift;
    }

    return;
}



// When interpolation between frames is necessary, the model's shape may warp when the rotation angle between key frames is large. This is because only linear interpolation will occur when G3D_NORMALIZE_ROT_MTX is disabled.
// 
// 
// When G3D_NORMALIZE_ROT_MTX is enabled, model warping is controlled with normalization processing.
// 
#define G3D_NORMALIZE_ROT_MTX

#ifdef G3D_NORMALIZE_ROT_MTX
#define ROT_FILTER_SHIFT    0
#else
#define ROT_FILTER_SHIFT    1
#endif

/*---------------------------------------------------------------------------*
    getRotData_

    Gets the rotation matrix and sets to *pRot.
 *---------------------------------------------------------------------------*/
static void
getRotData_(MtxFx33* pRot,
            fx32 Frame,
            const u32* pData,
            const NNSG3dResJntAnm* pJntAnm)
{
    u32 frame = (u32)FX_Whole(Frame);
    const void* pArray = (const void*)((const u8*)pJntAnm + *(pData + 1));
    const void* pArrayRot3 = (const void*)((const u8*)pJntAnm + pJntAnm->ofsRot3);
    const void* pArrayRot5 = (const void*)((const u8*)pJntAnm + pJntAnm->ofsRot5);
    NNSG3dJntAnmRInfo info = (NNSG3dJntAnmRInfo)*pData;
    const u16* p = (const u16*)pArray;

    u32 last_interp;
    u32 idx;
    u32 idx_sub;

    if (!(info & NNS_G3D_JNTANM_RINFO_STEP_MASK))
    {
        // NNS_G3D_JNTANM_RINFO_STEP_1 is selected
        idx = frame;
        goto ROT_NONINTERP;
    }

    last_interp = ((u32)info & NNS_G3D_JNTANM_RINFO_LAST_INTERP_MASK) >>
                                    NNS_G3D_JNTANM_RINFO_LAST_INTERP_SHIFT;

    if (info & NNS_G3D_JNTANM_RINFO_STEP_2)
    {
        // NNS_G3D_JNTANM_RINFO_STEP_2 is selected
        if (frame & 1)
        {
            if (frame > last_interp)
            {
                // impossible if anything other than the last frame
                idx = (last_interp >> 1) + 1;
                goto ROT_NONINTERP;
            }
            else
            {
                // Since the final frame is not on an odd number, 50:50 interpolation is needed
                idx = frame >> 1;
                goto ROT_INTERP_2;
            }
        }
        else
        {
            // even frame, so no interpolation is needed
            idx = frame >> 1;
            goto ROT_NONINTERP;
        }
    }
    else
    {
        // NNS_G3D_JNTANM_RINFO_STEP_4 is selected
        if (frame & 3)
        {
            if (frame > last_interp)
            {
                idx = (last_interp >> 2) + (frame & 3);
                goto ROT_NONINTERP;
            }

            // with interpolation management
            if (frame & 1)
            {
                MtxFx33 tmp;
                BOOL    doCross = FALSE;
                if (frame & 2)
                {
                    // interpolate with 3:1 position
                    idx_sub = (frame >> 2);
                    idx = idx_sub + 1;
                }
                else
                {
                    // interpolate with 1:3 position
                    idx = (frame >> 2);
                    idx_sub = idx + 1;
                }
                doCross |= getRotDataByIdx_(pRot,
                                            pArrayRot3,
                                            pArrayRot5,
                                            (NNSG3dJntAnmRIdx)p[idx]);
                doCross |= getRotDataByIdx_(&tmp,
                                            pArrayRot3,
                                            pArrayRot5,
                                            (NNSG3dJntAnmRIdx)p[idx_sub]);
                pRot->_00 = (pRot->_00 * 3 + tmp._00) >> (2 * ROT_FILTER_SHIFT);
                pRot->_01 = (pRot->_01 * 3 + tmp._01) >> (2 * ROT_FILTER_SHIFT);
                pRot->_02 = (pRot->_02 * 3 + tmp._02) >> (2 * ROT_FILTER_SHIFT);
                pRot->_10 = (pRot->_10 * 3 + tmp._10) >> (2 * ROT_FILTER_SHIFT);
                pRot->_11 = (pRot->_11 * 3 + tmp._11) >> (2 * ROT_FILTER_SHIFT);
                pRot->_12 = (pRot->_12 * 3 + tmp._12) >> (2 * ROT_FILTER_SHIFT);
                
#ifdef G3D_NORMALIZE_ROT_MTX
                VEC_Normalize( (VecFx32*)(&pRot->_00), (VecFx32*)(&pRot->_00) );
                VEC_Normalize( (VecFx32*)(&pRot->_10), (VecFx32*)(&pRot->_10) );
#endif
                
                if (!doCross)
                {
                    // When both are data in Rot3 format, calculation of the cross product is avoided.
                    // 
                    pRot->_20 = (pRot->_20 * 3 + tmp._20) >> (2 * ROT_FILTER_SHIFT);
                    pRot->_21 = (pRot->_21 * 3 + tmp._21) >> (2 * ROT_FILTER_SHIFT);
                    pRot->_22 = (pRot->_22 * 3 + tmp._22) >> (2 * ROT_FILTER_SHIFT);

#ifdef G3D_NORMALIZE_ROT_MTX
                    VEC_Normalize( (VecFx32*)(&pRot->_20), (VecFx32*)(&pRot->_20) );
#endif
                }
                else
                {
                    vecCross_((const VecFx32*)&pRot->_00,
                              (const VecFx32*)&pRot->_10,
                              (VecFx32*)&pRot->_20);
                }
                return;
            }
            else
            {
                // becomes 50:50 interpolation
                idx = frame >> 2;
                goto ROT_INTERP_2;
            }
        }
        else
        {
            idx = frame >> 2;
            goto ROT_NONINTERP;
        }
    }
ROT_INTERP_2:
    {
        MtxFx33 tmp;
        BOOL    doCross = FALSE;
        doCross |= getRotDataByIdx_(pRot,
                                    pArrayRot3,
                                    pArrayRot5,
                                    (NNSG3dJntAnmRIdx)p[idx]);
        doCross |= getRotDataByIdx_(&tmp,
                                    pArrayRot3,
                                    pArrayRot5,
                                    (NNSG3dJntAnmRIdx)p[idx + 1]);
        pRot->_00 = (pRot->_00 + tmp._00) >> ROT_FILTER_SHIFT;
        pRot->_01 = (pRot->_01 + tmp._01) >> ROT_FILTER_SHIFT;
        pRot->_02 = (pRot->_02 + tmp._02) >> ROT_FILTER_SHIFT;
        pRot->_10 = (pRot->_10 + tmp._10) >> ROT_FILTER_SHIFT;
        pRot->_11 = (pRot->_11 + tmp._11) >> ROT_FILTER_SHIFT;
        pRot->_12 = (pRot->_12 + tmp._12) >> ROT_FILTER_SHIFT;
        
#ifdef G3D_NORMALIZE_ROT_MTX
        VEC_Normalize( (VecFx32*)(&pRot->_00), (VecFx32*)(&pRot->_00) );
        VEC_Normalize( (VecFx32*)(&pRot->_10), (VecFx32*)(&pRot->_10) );
#endif
        
        if (!doCross)
        {
            pRot->_20 = (pRot->_20 + tmp._20) >> ROT_FILTER_SHIFT;
            pRot->_21 = (pRot->_21 + tmp._21) >> ROT_FILTER_SHIFT;
            pRot->_22 = (pRot->_22 + tmp._22) >> ROT_FILTER_SHIFT;
            
#ifdef G3D_NORMALIZE_ROT_MTX
            VEC_Normalize( (VecFx32*)(&pRot->_20), (VecFx32*)(&pRot->_20) );
#endif
        }
        else
        {
            vecCross_((const VecFx32*)&pRot->_00,
                      (const VecFx32*)&pRot->_10,
                      (VecFx32*)&pRot->_20);
        }
        return;
    }
ROT_NONINTERP:
    if (getRotDataByIdx_(pRot,
                         pArrayRot3,
                         pArrayRot5,
                         (NNSG3dJntAnmRIdx)p[idx]))
    {
        vecCross_((const VecFx32*)&pRot->_00,
                  (const VecFx32*)&pRot->_10,
                  (VecFx32*)&pRot->_20);
    }
    else
    {
#ifdef G3D_NORMALIZE_ROT_MTX
        VEC_Normalize( (VecFx32*)(&pRot->_20), (VecFx32*)(&pRot->_20) );
#endif
    }
    return;
}


/*---------------------------------------------------------------------------*
    getRotData_

    Gets the rotation matrix, and sets to *pRot.
    Interpolate if it has a fraction.
 *---------------------------------------------------------------------------*/
static void
getRotDataEx_(MtxFx33* pRot,
              fx32 Frame,
              const u32* pData,
              const NNSG3dResJntAnm* pJntAnm)
{
    // Only frames that fall within the range, 0 < Frame < numFrame, can arrive at this function.
    // 

    const void* pArray = (const void*)((const u8*)pJntAnm + *(pData + 1));
    const void* pArrayRot3 = (const void*)((const u8*)pJntAnm + pJntAnm->ofsRot3);
    const void* pArrayRot5 = (const void*)((const u8*)pJntAnm + pJntAnm->ofsRot5);
    NNSG3dJntAnmRInfo info = (NNSG3dJntAnmRInfo)*pData;

    u32 last_interp;
    u32 idx0, idx1;
    fx32 remainder;
    int step;
    u32 step_shift;
    u32 frame;
    const u16* p = (const u16*)pArray;

    frame = (u32)FX_Whole(Frame);

    if (frame == pJntAnm->numFrame - 1)
    {
        //
        // when the frame is between numFrame - 1 and numFrame
        // when exactly between the first and last frame
        //

        //
        // first, get the final frame data index
        //
        if (!(info & NNS_G3D_JNTANM_RINFO_STEP_MASK))
        {
            idx0 = frame;
        }
        else if (info & NNS_G3D_JNTANM_RINFO_STEP_2)
        {
            idx0 = (frame >> 1) + (frame & 1);
        }
        else
        {
            idx0 = (frame >> 2) + (frame & 3);
        }

        //
        // a flag determines whether to extrapolate or to return the final data
        //
        if (pJntAnm->flag & NNS_G3D_JNTANM_OPTION_END_TO_START_INTERPOLATION)
        {
            // when extrapolating, do it with the first and last data (loop)
            idx1 = 0;
            goto ROT_EX_0_1;
        }
        else
        {
            // when returning the final data
            if (getRotDataByIdx_(pRot,
                                 pArrayRot3,
                                 pArrayRot5,
                                 (NNSG3dJntAnmRIdx)p[idx0]))
            {
                vecCross_((const VecFx32*)&pRot->_00,
                          (const VecFx32*)&pRot->_10,
                          (VecFx32*)&pRot->_20);
            }
            else
            {
#ifdef G3D_NORMALIZE_ROT_MTX
                VEC_Normalize( (VecFx32*)(&pRot->_20), (VecFx32*)(&pRot->_20) );
#endif
            }
            return;
        }
    }

    if (!(info & NNS_G3D_JNTANM_RINFO_STEP_MASK))
    {
        // NNS_G3D_JNTANM_RINFO_STEP_1 is selected
        goto ROT_EX_0;
    }

    last_interp = ((u32)info & NNS_G3D_JNTANM_RINFO_LAST_INTERP_MASK) >>
                                    NNS_G3D_JNTANM_RINFO_LAST_INTERP_SHIFT;

    if (info & NNS_G3D_JNTANM_RINFO_STEP_2)
    {
        // NNS_G3D_JNTANM_RINFO_STEP_2 is selected
        // The inclusion of the sign is due to consideration of the discarded data.
        if (frame >= last_interp)
        {
            // impossible if anything other than the last frame
            idx0 = (last_interp >> 1);
            idx1 = idx0 + 1;
            goto ROT_EX_0_1;
        }
        else
        {
            idx0 = frame >> 1;
            idx1 = idx0 + 1;
            remainder = Frame & (FX32_ONE * 2 - 1);
            step = 2;
            step_shift = 1;
            goto ROT_EX;
        }
    }
    else
    {
        // NNS_G3D_JNTANM_RINFO_STEP_4 is selected
        if (frame >= last_interp)
        {
            // Based on the conditions with which this function is called, frame is not the final data. As such, the following code ensures it remains with the boundaries.
            // 
            idx0 = (frame >> 2) + (frame & 3);
            idx1 = idx0 + 1;
            goto ROT_EX_0_1;
        }
        else
        {
            idx0 = frame >> 2;
            idx1 = idx0 + 1;
            remainder = Frame & (FX32_ONE * 4 - 1);
            step = 4;
            step_shift = 2;
            goto ROT_EX;
        }
    }
    NNS_G3D_ASSERT(0);
ROT_EX_0:
    idx0 = (u32)frame;
    idx1 = idx0 + 1;
ROT_EX_0_1:
    remainder = Frame & (FX32_ONE - 1);
    step = 1;
    step_shift = 0;
ROT_EX:
    {
        MtxFx33 r0, r1;
        BOOL doCross = FALSE;
        doCross |= getRotDataByIdx_(&r0,
                                    pArrayRot3,
                                    pArrayRot5,
                                    (NNSG3dJntAnmRIdx)p[idx0]);
        doCross |= getRotDataByIdx_(&r1,
                                    pArrayRot3,
                                    pArrayRot5,
                                    (NNSG3dJntAnmRIdx)p[idx1]);

        pRot->_00 = ((r0._00 * step) + (((r1._00 - r0._00) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);
        pRot->_01 = ((r0._01 * step) + (((r1._01 - r0._01) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);
        pRot->_02 = ((r0._02 * step) + (((r1._02 - r0._02) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);
        pRot->_10 = ((r0._10 * step) + (((r1._10 - r0._10) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);
        pRot->_11 = ((r0._11 * step) + (((r1._11 - r0._11) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);
        pRot->_12 = ((r0._12 * step) + (((r1._12 - r0._12) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);

#ifdef G3D_NORMALIZE_ROT_MTX
        VEC_Normalize( (VecFx32*)(&pRot->_00), (VecFx32*)(&pRot->_00) );
        VEC_Normalize( (VecFx32*)(&pRot->_10), (VecFx32*)(&pRot->_10) );
#endif

        if (!doCross)
        {
            // When both are data in Rot3 format, calculation of the cross product is avoided.
            // 
            pRot->_20 = ((r0._20 * step) + (((r1._20 - r0._20) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);
            pRot->_21 = ((r0._21 * step) + (((r1._21 - r0._21) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);
            pRot->_22 = ((r0._22 * step) + (((r1._22 - r0._22) * remainder) >> FX32_SHIFT)) >> (step_shift * ROT_FILTER_SHIFT);

#ifdef G3D_NORMALIZE_ROT_MTX
            VEC_Normalize( (VecFx32*)(&pRot->_20), (VecFx32*)(&pRot->_20) );
#endif
        }
        else
        {
            vecCross_((const VecFx32*)&pRot->_00,
                      (const VecFx32*)&pRot->_10,
                      (VecFx32*)&pRot->_20);
        }
        return;
    }
}


/*---------------------------------------------------------------------------*
    getRotDataByIdx_

    Takes the data of either Rot3 or Rot5 from index, and stores it in pRot.
 *---------------------------------------------------------------------------*/
static BOOL
getRotDataByIdx_(MtxFx33* pRot,
                 const void* pArrayRot3,
                 const void* pArrayRot5,
                 NNSG3dJntAnmRIdx info)
{
    if (info & NNS_G3D_JNTANM_RIDX_PIVOT)
    {
        const fx16* data;
        fx32 A, B;
        u32 idxPivot;

        pRot->_00 = pRot->_01 = pRot->_02 =
        pRot->_10 = pRot->_11 = pRot->_12 =
        pRot->_20 = pRot->_21 = pRot->_22 = 0;

        // data[0] NNSG3dJntAnmPivotInfo flag data
        // non-zero elements of data[1] and data[2] matrices
        data = (const fx16*)pArrayRot3 + ((info & NNS_G3D_JNTANM_RIDX_IDXDATA_MASK) * 3);
        A = *(data + 1);
        B = *(data + 2);
        idxPivot = (u32)( (*data & NNS_G3D_JNTANM_PIVOTINFO_IDXPIVOT_MASK) >>
                                 NNS_G3D_JNTANM_PIVOT_INFO_IDXPIVOT_SHIFT );

        pRot->a[idxPivot] =
            (*data & NNS_G3D_JNTANM_PIVOTINFO_MINUS) ? -FX32_ONE : FX32_ONE;

        pRot->a[pivotUtil_[idxPivot][0]] = A;
        pRot->a[pivotUtil_[idxPivot][1]] = B;

        pRot->a[pivotUtil_[idxPivot][2]] =
            (*data & NNS_G3D_JNTANM_PIVOTINFO_SIGN_REVC) ? -B : B;
        pRot->a[pivotUtil_[idxPivot][3]] =
            (*data & NNS_G3D_JNTANM_PIVOTINFO_SIGN_REVD) ? -A : A;

        // filled up to the third line
        return FALSE;
    }
    else
    {
        // brings the data from Rot5
        const fx16* data = (fx16*)pArrayRot5 +
                           ((info & NNS_G3D_JNTANM_RIDX_IDXDATA_MASK) * 5);
        fx16 _12;

        _12 = (fx16)(data[4] & 7);
        pRot->_11 = (data[4] >> 3);

        _12 = (fx16)((_12 << 3) | (data[0] & 7));
        pRot->_00 = (data[0] >> 3);

        _12 = (fx16)((_12 << 3) | (data[1] & 7));
        pRot->_01 = (data[1] >> 3);

        _12 = (fx16)((_12 << 3) | (data[2] & 7));
        pRot->_02 = (data[2] >> 3);

        _12 = (fx16)((_12 << 3) | (data[3] & 7));
        pRot->_10 = (data[3] >> 3);

        // After casting the shifted value to fx16 for sign extension, implicitly cast to fx32.
        // 
        pRot->_12 = ((fx16)(_12 << 3) >> 3);

        // The third line must be derived with a cross product.
        return TRUE;
    }
}

#endif // NNS_G3D_NSBCA_DISABLE
