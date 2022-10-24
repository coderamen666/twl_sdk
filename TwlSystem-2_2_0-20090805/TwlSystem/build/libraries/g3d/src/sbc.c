/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d
  File:     sbc.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1334 $
 *---------------------------------------------------------------------------*/

//
// AUTHOR: Kenji Nishida
//

#include <nnsys/g3d/sbc.h>

#include <nnsys/g3d/kernel.h>
#include <nnsys/g3d/binres/res_struct.h>
#include <nnsys/g3d/glbstate.h>
#include <nnsys/g3d/gecom.h>
#include <nnsys/g3d/binres/res_struct_accessor.h>
#include <nnsys/g3d/util.h>


static void
G3dDrawInternal_Loop_(NNSG3dRS* pRS)
{
    //
    // Loop for SBC execution
    //
    do
    {
        pRS->flag &= ~NNS_G3D_RSFLAG_SKIP;
        NNS_G3D_ASSERTMSG(NNS_G3dFuncSbcTable[NNS_G3D_GET_SBCCMD(*pRS->c)],
                          "SBC command not found / disabled.");

        (*NNS_G3dFuncSbcTable[NNS_G3D_GET_SBCCMD(*pRS->c)])
                                (pRS, NNS_G3D_GET_SBCFLG(*pRS->c));
    }
    while(!(pRS->flag & NNS_G3D_RSFLAG_RETURN));
}


static void
G3dDrawInternal_(NNSG3dRS* pRS, NNSG3dRenderObj* pRenderObj)
{
    NNS_G3D_NULL_ASSERT(pRS);
    NNS_G3D_NULL_ASSERT(pRenderObj);
  
    //
    // Init
    //
    // NOTICE:
    // First bit of isScaleCacheOne must be set.
    // If not, there will be problems when Si3d is "classic scale off."
    // (The bit is set in order for the scale of the parent of the root node to be computed as 1.)
    //
    MI_CpuClearFast(pRS, sizeof(*pRS));
    pRS->isScaleCacheOne[0] = 1; // Endian-dependent

    //
    // It's safer to set NNS_G3D_RSFLAG_NODE_VISIBLE.
    // This is just in case a MAT command is suddenly run with a user-defined SBC.
    // 
    //
    pRS->flag = NNS_G3D_RSFLAG_NODE_VISIBLE;

    //
    // select SBC to parse
    // Configuring a pointer to pRenderObj->ptrUserSbc enables the changing of the SBC to be parsed.
    // 
    //
    if (pRenderObj->ptrUserSbc)
    {
        pRS->c = pRenderObj->ptrUserSbc;
    }
    else
    {
        pRS->c = (u8*)pRenderObj->resMdl + pRenderObj->resMdl->ofsSbc;
    }

    //
    // configure pRenderObj
    //
    pRS->pRenderObj   = pRenderObj;

    //
    // Place information in cache that will be repeatedly used in pRenderObj->resMdl.
    //
    pRS->pResNodeInfo = NNS_G3dGetNodeInfo(pRenderObj->resMdl);
    pRS->pResMat      = NNS_G3dGetMat(pRenderObj->resMdl);
    pRS->pResShp      = NNS_G3dGetShp(pRenderObj->resMdl);
    pRS->funcJntScale = NNS_G3dGetJointScale_FuncArray[pRenderObj->resMdl->info.scalingRule];
    pRS->funcJntMtx   = NNS_G3dSendJointSRT_FuncArray[pRenderObj->resMdl->info.scalingRule];
    pRS->funcTexMtx   = NNS_G3dSendTexSRT_FuncArray[pRenderObj->resMdl->info.texMtxMode];
    pRS->posScale     = pRenderObj->resMdl->info.posScale;
    pRS->invPosScale  = pRenderObj->resMdl->info.invPosScale;

    //
    // callback setup
    //
    if (pRenderObj->cbFunc && pRenderObj->cbCmd < NNS_G3D_SBC_COMMAND_NUM)
    {
        NNS_G3D_SBC_CALLBACK_TIMING_ASSERT(pRenderObj->cbTiming);
        pRS->cbVecFunc[pRenderObj->cbCmd] = pRenderObj->cbFunc;
        pRS->cbVecTiming[pRenderObj->cbCmd] = pRenderObj->cbTiming;
    }

    if (pRenderObj->flag & NNS_G3D_RENDEROBJ_FLAG_RECORD)
    {
        pRS->flag |= NNS_G3D_RSFLAG_OPT_RECORD;
    }

    if (pRenderObj->flag & NNS_G3D_RENDEROBJ_FLAG_NOGECMD)
    {
        pRS->flag |= NNS_G3D_RSFLAG_OPT_NOGECMD;
    }

    if (pRenderObj->flag & NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_DRAW)
    {
        pRS->flag |= NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW;
    }

    if (pRenderObj->flag & NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_MTXCALC)
    {
        pRS->flag |= NNS_G3D_RSFLAG_OPT_SKIP_SBCMTXCALC;
    }

    if (pRenderObj->cbInitFunc)
    {
        (*pRenderObj->cbInitFunc)(pRS);
    }

    G3dDrawInternal_Loop_(pRS);

    //
    // clean up
    //
    // Asserts for giving a "DO NOT MODIFY" notification
    NNS_G3D_ASSERT(pRS->pRenderObj == pRenderObj);
    NNS_G3D_ASSERT(pRS == NNS_G3dRS);

    // To make use easier, record flag is reset automatically after recording.
    // The mode becomes Playback mode the next time called.
    pRenderObj->flag &= ~NNS_G3D_RENDEROBJ_FLAG_RECORD; 
}


/*---------------------------------------------------------------------------*
    updateHintVec_

    The bit corresponding to resource ID registered in pAnmObj is set to ON.
 *---------------------------------------------------------------------------*/
static void
updateHintVec_(u32* pVec, const NNSG3dAnmObj* pAnmObj)
{
    const NNSG3dAnmObj* p = pAnmObj;
    while(p)
    {
        int i;
        for (i = 0; i < p->numMapData; ++i)
        {
            if (p->mapData[i] & NNS_G3D_ANMOBJ_MAPDATA_EXIST)
            {
                pVec[i >> 5] |= 1 << (i & 31);
            }
        }
        p = p->next;
    }
}



/*---------------------------------------------------------------------------*
    NNS_G3dDraw

    Renders the model.
    Settings for animation, etc., are configured ahead of time by pRenderObj operations.
 *---------------------------------------------------------------------------*/
void
NNS_G3dDraw(NNSG3dRenderObj* pRenderObj)
{
    // NOTICE
    // NOT REENTRANT
    NNS_G3D_NULL_ASSERT(pRenderObj);
    NNS_G3D_NULL_ASSERT(pRenderObj->resMdl);
    NNS_G3D_ASSERTMSG(pRenderObj->resMdl->info.numMat <= NNS_G3D_SIZE_MAT_MAX,
                      "numMaterial exceeds NNS_G3D_SIZE_MAT_MAX");
    NNS_G3D_ASSERTMSG(pRenderObj->resMdl->info.numNode <= NNS_G3D_SIZE_JNT_MAX,
                      "numNode exceeds NNS_G3D_SIZE_JNT_MAX");
    NNS_G3D_ASSERTMSG(pRenderObj->resMdl->info.numShp <= NNS_G3D_SIZE_SHP_MAX,
                      "numShape exceeds NNS_G3D_SIZE_SHP_MAX");

    if (NNS_G3dRenderObjTestFlag(pRenderObj, NNS_G3D_RENDEROBJ_FLAG_HINT_OBSOLETE))
    {
        // Update because the bit vector for "hint" is not correct.
        MI_CpuClear32(&pRenderObj->hintMatAnmExist[0], sizeof(u32) * (NNS_G3D_SIZE_MAT_MAX / 32));
        MI_CpuClear32(&pRenderObj->hintJntAnmExist[0], sizeof(u32) * (NNS_G3D_SIZE_JNT_MAX / 32));
        MI_CpuClear32(&pRenderObj->hintVisAnmExist[0], sizeof(u32) * (NNS_G3D_SIZE_JNT_MAX / 32));

        if (pRenderObj->anmMat)
            updateHintVec_(&pRenderObj->hintMatAnmExist[0], pRenderObj->anmMat);
        if (pRenderObj->anmJnt)
            updateHintVec_(&pRenderObj->hintJntAnmExist[0], pRenderObj->anmJnt);
        if (pRenderObj->anmVis)
            updateHintVec_(&pRenderObj->hintVisAnmExist[0], pRenderObj->anmVis);

        NNS_G3dRenderObjResetFlag(pRenderObj, NNS_G3D_RENDEROBJ_FLAG_HINT_OBSOLETE);
    }

    if (NNS_G3dRS)
    {
        G3dDrawInternal_(NNS_G3dRS, pRenderObj);
    }
    else
    {
        // Take onto local stack if SBC runtime structure region has not been secured.
        NNSG3dRS rs;
        NNS_G3dRS = &rs;
        G3dDrawInternal_(&rs, pRenderObj);
        NNS_G3dRS = NULL;
    }
}



//
// Evokes warning regarding the coding of the callback-determination code.
// NOTICE:
// Be aware that the callback conditions inside the callback function can be overwritten by the user.
// That is, the determination as to whether there is a callback is stored in the auto variable, and that cannot be used for the next callback determination.
// 
//

/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_NOP

    mnemonic:   NNS_G3D_SBC_NOP
    operands:   None.
    callbacks:  A/B/C

    Explanation of operations:
    Does nothing.

    Remarks:
    Callback is valid, but the timing specification is meaningless.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_NOP(NNSG3dRS* rs, u32)
{
    NNS_G3D_NULL_ASSERT(rs);

#if !defined(NNS_G3D_SBC_CALLBACK_TIMING_A_DISABLE) || \
    !defined(NNS_G3D_SBC_CALLBACK_TIMING_B_DISABLE) || \
    !defined(NNS_G3D_SBC_CALLBACK_TIMING_C_DISABLE)
    
    if (rs->cbVecFunc[NNS_G3D_SBC_NOP])
    {
        (*rs->cbVecFunc[NNS_G3D_SBC_NOP])(rs);
    }
#endif

    rs->c++;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_RET

    mnemonic:   NNS_G3D_SBC_RET
    operands:   None.
    callbacks:  A/B/C

    Explanation of operations:
    Returns NNS_G3D_FUNCSBC_STATUS_RETURN and ends execution of Sbc.

    Remarks:
    This is the command at the end of the SBC.
    Callback is valid, but the timing specification is meaningless.
    This command does not increment NNS_G3dRS->c.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_RET(NNSG3dRS* rs, u32)
{
    NNS_G3D_NULL_ASSERT(rs);

#if !defined(NNS_G3D_SBC_CALLBACK_TIMING_A_DISABLE) || \
    !defined(NNS_G3D_SBC_CALLBACK_TIMING_B_DISABLE) || \
    !defined(NNS_G3D_SBC_CALLBACK_TIMING_C_DISABLE)

    if (rs->cbVecFunc[NNS_G3D_SBC_RET])
    {
        (*rs->cbVecFunc[NNS_G3D_SBC_RET])(rs);
    }
#endif

    rs->flag |= NNS_G3D_RSFLAG_RETURN;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_NODE

    mnemonic:   NNS_G3D_SBC_NODE
    operands:   nodeID
                flags (1 byte)
                    visible if 0x01 is on
    callbacks:  A: before
                B: None.
                C: after
    
    Explanation of operations:
    - calls the TIMING_A callback
    - performs check before execution of visibility animation
       if visibility animation exists:
           The NNS_G3D_RSFLAG_NODE_VISIBLE bit is set in conjuction with the results (and not the return value) of the visibility animation blending function.
           
       If it doesn't exist:
           References "flags" to set the NNS_G3D_RSFLAG_NODE_VISIBLE bit.
    - calls the TIMING_B callback
    - adds rs->c

    Remarks:
    The NNS_G3D_RSFLAG_NODE_VISIBLE bit is only used inside this function.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_NODE(NNSG3dRS* rs, u32)
{
    NNS_G3D_NULL_ASSERT(rs);

    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW))
    {
        BOOL                    cbFlag;
        NNSG3dSbcCallBackTiming cbTiming;
        u32                     curNode;

        curNode = rs->currentNode = *(rs->c + 1);
        rs->flag |= NNS_G3D_RSFLAG_CURRENT_NODE_VALID;
        rs->pVisAnmResult = &rs->tmpVisAnmResult;

        // callback A
        // When NNS_G3D_RSFLAG_SKIP is set internally, the visibility calculation can be controlled by manipulating rs->pVisAnmResult.
        // 
        // 
        cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_NODE, &cbTiming);

        if (!cbFlag)
        {
            NNS_G3D_NULL_ASSERT(rs->pRenderObj->funcBlendVis);

            if (rs->pRenderObj->anmVis                                            &&
                NNSi_G3dBitVecCheck(&rs->pRenderObj->hintVisAnmExist[0], curNode) &&
                (*rs->pRenderObj->funcBlendVis)(rs->pVisAnmResult, rs->pRenderObj->anmVis, curNode))
            {
                // When the AnmObj list has been configured, the hint vector bit specific to visibility animation is ON and there is an actual corresponding visibility animation within the visibility animation blender.
                // 
                // 
                ;
            }
            else
            {
                // When there is no visibility animation, the decision is based on the operand data.
                // 
                rs->pVisAnmResult->isVisible = *(rs->c + 2) & 1;
            }
        }
        
        // callback B
        // The calculated visibility can be modified by manipulating rs->pVisAnmResult.
        // 
        cbFlag = NNSi_G3dCallBackCheck_B(rs, NNS_G3D_SBC_NODE, &cbTiming);

        if (!cbFlag)
        {
            if (rs->pVisAnmResult->isVisible)
            {
                rs->flag |= NNS_G3D_RSFLAG_NODE_VISIBLE;
            }
            else
            {
                rs->flag &= ~NNS_G3D_RSFLAG_NODE_VISIBLE;
            }
        }

        // callback C
        // You can insert any process before the next command.
        (void)NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_NODE, cbTiming);
    }

    rs->c += 3;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_MTX

    mnemonic:   NNS_G3D_SBC_MTX
    operands:   idxMtx
    callbacks:  A: before
                B: None.
                C: after

    Explanation of operations:
    - skip this command entirely if NNS_G3D_RSFLAG_NODE_VISIBLE is OFF.
    - calls the TIMING_A callback
    - loads the current matrix from the matrix stack
    - calls the TIMING_B callback
    - adds rs->c

    Remarks:
    Everything including the callback is skipped if NNS_G3D_RSFLAG_NODE_VISIBLE is OFF.
    Has to be in POSITION_VECTOR mode before beginning.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_MTX(NNSG3dRS* rs, u32)
{
    NNS_G3D_NULL_ASSERT(rs);

    // The MTX command is put in the Draw category.
    // This is because the command is created before MAT and SHP.
    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW) &&
        (rs->flag & NNS_G3D_RSFLAG_NODE_VISIBLE))
    {
        BOOL                    cbFlag;
        NNSG3dSbcCallBackTiming cbTiming;

        // callback A
        // When NNS_G3D_RSFLAG_SKIP is set internally, the matrix restore action can be replaced.
        // 
        cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_MTX, &cbTiming);

        if (!cbFlag)
        {
            u32 arg = *(rs->c + 1);
            NNS_G3D_ASSERT(arg < 31);

            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                NNS_G3dGeBufferOP_N(G3OP_MTX_RESTORE,
                                    &arg,
                                    G3OP_MTX_RESTORE_NPARAMS);
            }
        }

        // callback C
        // You can insert some sort of processing before the next command.
        (void)NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_MTX, cbTiming);
    }

    rs->c += 2;
}


void
NNSi_G3dFuncSbc_MAT_InternalDefault(NNSG3dRS* rs,
                                    u32 opt,
                                    const NNSG3dResMatData* mat,
                                    u32 idxMat)
{
    static const u32 matColorMask_[8] =
    {
        0x00000000,
        0x00007fff,
        0x7fff0000,
        0x7fff7fff,
        0x00008000,
        0x0000ffff,
        0x7fff8000,
        0x7fffffff
    };
    BOOL cbFlag;
    NNSG3dSbcCallBackTiming cbTiming;

    NNS_G3D_NULL_ASSERT(rs);
    NNS_G3D_NULL_ASSERT(mat);

    rs->currentMat = (u8)idxMat;
    rs->flag |= NNS_G3D_RSFLAG_CURRENT_MAT_VALID;
    // Must specify region used when callback is called 
    rs->pMatAnmResult = &rs->tmpMatAnmResult;

    // callback A
    // When NNS_G3D_RSFLAG_SKIP is set internally, the material settings code can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_MAT, &cbTiming);

    if (!cbFlag)
    {
        NNSG3dMatAnmResult* pResult;

        //
        // Determine whether to use the calculation-result buffer.
        //
        if (rs->pRenderObj->recMatAnm &&
            !(rs->flag & NNS_G3D_RSFLAG_OPT_RECORD))
        {
            // When a buffer can be used
            pResult = (rs->pRenderObj->recMatAnm + idxMat);
        }
        else
        {
            //
            // Material data is gotten and calculations are performed within this if block, and the appropriate data is set for pResult.
            // 
            //
            if ((opt == NNS_G3D_SBCFLG_001 || opt == NNS_G3D_SBCFLG_010) &&
                NNSi_G3dBitVecCheck(&rs->isMatCached[0], idxMat))
            {
                // If the MAT cached flag is set:
                if (rs->pRenderObj->recMatAnm)
                {
                    // The previous calculation result is recorded in the buffer, so fetch it from there.
                    pResult = (rs->pRenderObj->recMatAnm + idxMat);
                }
                else
                {
                    // The previous calculation result is recorded in cache, so fetch it from there.
                    pResult = &NNS_G3dRSOnGlb.matCache[idxMat];
                }
            }
            else
            {
                if (rs->pRenderObj->recMatAnm)
                {
                    // The MAT cached flag is set.
                    NNSi_G3dBitVecSet(&rs->isMatCached[0], idxMat);

                    // If there is a buffer, record in the buffer regardless of the command option.
                    pResult = (rs->pRenderObj->recMatAnm + idxMat);
                }
                else if (opt == NNS_G3D_SBCFLG_010)
                {
                    // The MAT cached flag is set.
                    NNSi_G3dBitVecSet(&rs->isMatCached[0], idxMat);

                    // Set the write destination in the MAT cache.
                    pResult = &NNS_G3dRSOnGlb.matCache[idxMat];
                }
                else
                {
                    // Use the region in the NNSG3dRS.
                    pResult = &rs->tmpMatAnmResult;
                }

                //
                // Set the material information for the model data.
                // Combine with the G3dGlb default.
                //
                {
                    pResult->flag           = (NNSG3dMatAnmResultFlag) 0;
                    if (NNS_G3dGetMatDataByIdx(rs->pResMat, idxMat)->flag & NNS_G3D_MATFLAG_WIREFRAME)
                    {
                        pResult->flag |= NNS_G3D_MATANM_RESULTFLAG_WIREFRAME;
                    }
                }
                {
                    // Diffuse & Ambient & VtxColor
                    u32 mask = matColorMask_[(mat->flag >> 6) & 7];
                    pResult->prmMatColor0 = (NNS_G3dGlb.prmMatColor0 & ~mask) |
                                            (mat->diffAmb & mask);
                }

                {
                    // Specular & Emission & Shininess
                    u32 mask = matColorMask_[(mat->flag >> 9) & 7];
                    pResult->prmMatColor1 = (NNS_G3dGlb.prmMatColor1 & ~mask) |
                                            (mat->specEmi & mask);
                }

                pResult->prmPolygonAttr = (NNS_G3dGlb.prmPolygonAttr & ~mat->polyAttrMask) |
                                          (mat->polyAttr & mat->polyAttrMask);

                pResult->prmTexImage = mat->texImageParam;
                pResult->prmTexPltt  = mat->texPlttBase;

                // When there is a texture matrix for a model, the texture matrix is set from the model.
                // 
                if (mat->flag & NNS_G3D_MATFLAG_TEXMTX_USE)
                {
                    const u8* p = (const u8*)mat + sizeof(NNSG3dResMatData);

                    if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_SCALEONE))
                    {
                        const fx32* p_fx32 = (const fx32*)p;

                        pResult->scaleS = *(p_fx32 + 0);
                        pResult->scaleT = *(p_fx32 + 1);
                        p += 2 * sizeof(fx32);
                    }
                    else
                    {
                        pResult->flag |= NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SCALEONE;
                    }

                    if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_ROTZERO))
                    {
                        const fx16* p_fx16 = (const fx16*)p;

                        pResult->sinR = *(p_fx16 + 0);
                        pResult->cosR = *(p_fx16 + 1);
                        p += 2 * sizeof(fx16);
                    }
                    else
                    {
                        pResult->flag |= NNS_G3D_MATANM_RESULTFLAG_TEXMTX_ROTZERO;
                    }

                    if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_TRANSZERO))
                    {
                        const fx32* p_fx32 = (const fx32*)p;

                        pResult->transS = *(p_fx32 + 0);
                        pResult->transT = *(p_fx32 + 1);
                    }
                    else
                    {
                        pResult->flag |= NNS_G3D_MATANM_RESULTFLAG_TEXMTX_TRANSZERO;
                    }

                    pResult->flag |= NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SET;
                }

                NNS_G3D_NULL_ASSERT(rs->pRenderObj->funcBlendMat);

                // Check the material animation.
                if (rs->pRenderObj->anmMat &&
                    NNSi_G3dBitVecCheck(&rs->pRenderObj->hintMatAnmExist[0], idxMat))
                {
                    // Based on the material animation calculation, pResult may change.
                    (void)(*rs->pRenderObj->funcBlendMat)(pResult, rs->pRenderObj->anmMat, idxMat);
                }

                //
                // When there is originally a texture matrix and when an animation has been appended, the texture's width, height and other information is added.
                // 
                //
                if ( pResult->flag & (NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SET |
                                      NNS_G3D_MATANM_RESULTFLAG_TEXMTX_MULT))
                {
                    pResult->origWidth  = mat->origWidth;
                    pResult->origHeight = mat->origHeight;
                    pResult->magW = mat->magW;
                    pResult->magH = mat->magH;
                }
            }
        }
        rs->pMatAnmResult = pResult;
    }

    // callback B
    // When NNS_G3D_RSFLAG_SKIP is set internally, the material send action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_B(rs, NNS_G3D_SBC_MAT, &cbTiming);

    if (!cbFlag)
    {
        NNSG3dMatAnmResult* pResult;

        NNS_G3D_NULL_ASSERT(rs->pMatAnmResult);
        pResult = rs->pMatAnmResult;

        // Check whether there is transparent material.
        if (pResult->prmPolygonAttr & REG_G3_POLYGON_ATTR_ALPHA_MASK)
        {
            // When it is non-transparent wireframe display, send a geometry command
            // 
            
            if (pResult->flag & NNS_G3D_MATANM_RESULTFLAG_WIREFRAME)
            {
                // If wireframe, the alpha sent to the geometry engine becomes 0.
                pResult->prmPolygonAttr &= ~REG_G3_POLYGON_ATTR_ALPHA_MASK;
            }

            // transparent flag OFF
            rs->flag &= ~NNS_G3D_RSFLAG_MAT_TRANSPARENT;

            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                // Push to stack and transfer.
                u32 cmd[7];

                cmd[0] = GX_PACK_OP(G3OP_DIF_AMB, G3OP_SPE_EMI, G3OP_POLYGON_ATTR, G3OP_NOP);
                cmd[1] = pResult->prmMatColor0;
                cmd[2] = pResult->prmMatColor1;
                cmd[3] = pResult->prmPolygonAttr;
                cmd[4] = GX_PACK_OP(G3OP_TEXIMAGE_PARAM, G3OP_TEXPLTT_BASE, G3OP_NOP, G3OP_NOP);
                cmd[5] = pResult->prmTexImage;
                cmd[6] = pResult->prmTexPltt;

                NNS_G3dGeBufferData_N(&cmd[0], 7);

                if (pResult->flag & (NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SET |
                                     NNS_G3D_MATANM_RESULTFLAG_TEXMTX_MULT))
                {

                    // Send texture matrix.
                    NNS_G3D_NULL_ASSERT(rs->funcTexMtx);
                    (*rs->funcTexMtx)(pResult);
                }
            }
        }
        else
        {
            // transparent flag ON
            rs->flag |= NNS_G3D_RSFLAG_MAT_TRANSPARENT;
        }
    }

    // callback C
    // You can insert some sort of processing before the next command.
    (void) NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_MAT, cbTiming);
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_MAT

    mnemonic:   NNS_G3D_SBC_MAT([000], [001], [010])
    operands:   idxMat
    callbacks:  A: before
                B: during
                C: after

    Explanation of operations:
    - skip this command entirely if NNS_G3D_RSFLAG_NODE_VISIBLE is OFF
    - calls the TIMING_A callback
    - get/calculate the material information
       Operations relating to the MAT cache will differ, depending on the "opt" settings:
       if the idxMat-numbered bit for rs->isMatCached is SET, the material information has been cached to the MAT cache and can be used.
       

       [000] do not reference MAT cache
       [001] read/write to MAT cache
       [010] read MAT cache

       set to NNS_G3D_RSFLAG_MAT_TRANSPARENT
       When set, the processing up to adding rs->c is skipped.
    - calls the TIMING_B callback
    - send the material information to the geometry engine
    - call the TIMING_C callback
    - adds rs->c

    Remarks:
    Deciding how best to use [000], [001] and [010] is done at time of conversion.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_MAT(NNSG3dRS* rs, u32 opt)
{
    NNS_G3D_NULL_ASSERT(rs);

    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW))
    {
        u32 idxMat;
        idxMat = *(rs->c + 1);

        // Skip command if not VISIBLE or if currently already in same MatID state.
        if ((rs->flag & NNS_G3D_RSFLAG_NODE_VISIBLE) ||
            !((rs->flag & NNS_G3D_RSFLAG_CURRENT_MAT_VALID) &&
            (idxMat == rs->currentMat)))
        {
            const NNSG3dResMatData* mat =
                NNS_G3dGetMatDataByIdx(rs->pResMat, idxMat);

            NNS_G3D_NULL_ASSERT(NNS_G3dFuncSbcMatTable[mat->itemTag]);
            (*NNS_G3dFuncSbcMatTable[mat->itemTag])(rs, opt, mat, idxMat);
        }
    }
    rs->c += 2;
}


void
NNSi_G3dFuncSbc_SHP_InternalDefault(NNSG3dRS* rs,
                                    u32,
                                    const NNSG3dResShpData* shp,
                                    u32)
{
    BOOL cbFlag;
    NNSG3dSbcCallBackTiming cbTiming;
    NNS_G3D_NULL_ASSERT(rs);
    NNS_G3D_NULL_ASSERT(shp);

    // callback A
    // When NNS_G3D_RSFLAG_SKIP is set internally, the display list send action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_SHP, &cbTiming);

    if (!cbFlag)
    {
        if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
        {
            NNS_G3dGeSendDL(NNS_G3dGetShpDLPtr(shp), NNS_G3dGetShpDLSize(shp));
        }
    }

    // callback B
    (void) NNSi_G3dCallBackCheck_B(rs, NNS_G3D_SBC_SHP, &cbTiming);

    // restore state
    // In the future, state processing, etc. may be added.

    // callback C
    // You can insert some sort of processing before the next command.
    (void) NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_SHP, cbTiming);
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_SHP

    mnemonic:   NNS_G3D_SBC_SHP
    operands:   idxShp
    callbacks:  A: before
                B: during
                C: after

    Explanation of operations:
    - When NNS_G3D_RSFLAG_NODE_VISIBLE is OFF or when NNS_G3D_RSFLAG_MAT_TRANSPARENT is ON, this entire command is skipped.
       
    - calls the TIMING_A callback
    - call display list
    - calls the TIMING_B callback
    - return the state changed by the display list
    - call the TIMING_C callback
    - add rs->c

    Remarks:
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_SHP(NNSG3dRS* rs, u32 opt)
{
    NNS_G3D_NULL_ASSERT(rs);

    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW))
    {
        if ((rs->flag & NNS_G3D_RSFLAG_NODE_VISIBLE) &&
            !(rs->flag & NNS_G3D_RSFLAG_MAT_TRANSPARENT))
        {
            u32 idxShp = *(rs->c + 1);
            const NNSG3dResShpData* shp =
                NNS_G3dGetShpDataByIdx(rs->pResShp, idxShp);

            NNS_G3D_NULL_ASSERT(NNS_G3dFuncSbcShpTable[shp->itemTag]);
            (*NNS_G3dFuncSbcShpTable[shp->itemTag])(rs, opt, shp, idxShp);
        }
    }
    rs->c += 2;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_NODEDESC([000], [001], [010], [011])

    mnemonic:   NNS_G3D_SBC_NODEDESC
    operands:   [000]: idxNode, idxNodeParent, flags
                [001]: idxNode, idxNodeParent, flags, idxMtxDest
                [010]: idxNode, idxNodeParent, flags, idxMtxSrc
                [011]: idxNode, idxNodeParent, flags, idxMtxDest, idxMtxSrc
                0 <= idxMtxSrc, idxMtxDest <= 30
                x = 0 --> checks visibility anm and change currentNode
                x = 1 --> no
    callbacks:  A: before
                B: during
                C: after

    Explanation of operations:
    - for [010], [011] restore idxMtxSrc to current matrix
    - calls the TIMING_A callback
    - multiply the current matrix by the model/joint animation matrix
    - calls the TIMING_B callback
    - for [001], [011] store current matrix in idxMtxDest
    - call the TIMING_C callback
    - add rs->c

    Remarks:
    Unlike with NNS_G3dFuncSbc_NODE, the NODE state is not changed.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_NODEDESC(NNSG3dRS* rs, u32 opt)
{
    u32 cmdLen = 4;
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
    BOOL cbFlag;
    NNSG3dSbcCallBackTiming cbTiming;
    u32 idxNode = *(rs->c + 1);
    rs->currentNodeDesc = (u8)idxNode;
    rs->flag |= NNS_G3D_RSFLAG_CURRENT_NODEDESC_VALID;

    NNS_G3D_NULL_ASSERT(rs);

    if (rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCMTXCALC)
    {
        if (opt == NNS_G3D_SBCFLG_010 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;
        }


        if (opt == NNS_G3D_SBCFLG_001 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;
#if 1
            // Because the matrix can later be used as is, the store action must be replaced by restore.
            // 
            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                u32 idxMtxDest = *(rs->c + 4);
                NNS_G3D_ASSERT(idxMtxDest < 31);

                NNS_G3dGeBufferOP_N(G3OP_MTX_RESTORE,
                                    &idxMtxDest,
                                    G3OP_MTX_RESTORE_NPARAMS);
            }
#endif
        }
        rs->c += cmdLen;
        return;
    }

    {
        if (opt == NNS_G3D_SBCFLG_010 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            u32 idxMtxSrc;

            ++cmdLen;
            if (opt == NNS_G3D_SBCFLG_010)
            {
                idxMtxSrc = *(rs->c + 4);
            }
            else
            {
                idxMtxSrc = *(rs->c + 5);
            }
            NNS_G3D_ASSERT(idxMtxSrc < 31);

            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                NNS_G3dGeBufferOP_N(G3OP_MTX_RESTORE,
                                    &idxMtxSrc,
                                    G3OP_MTX_RESTORE_NPARAMS);
            }
        }
    }

    // Must specify region used when callback is called 
    rs->pJntAnmResult = &rs->tmpJntAnmResult;

    // callback A
    // When NNS_G3D_RSFLAG_SKIP is set internally, the joint SRT data get action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_NODEDESC, &cbTiming);

    if (!cbFlag)
    {
        NNSG3dJntAnmResult* pAnmResult;
        BOOL                isUseRecordData;
//        u32 idxNodeParent = *(rs->c + 2);
//        u32 flags         = *(rs->c + 3);

        //
        // Determine whether to use the calculation-result buffer.
        //
        if (rs->pRenderObj->recJntAnm)
        {
            // point to the external buffer
            pAnmResult = (rs->pRenderObj->recJntAnm + idxNode);
            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_RECORD))
            {
                // Play the external buffer data.
                isUseRecordData = TRUE;
            }
            else
            {
                // record the data in the external buffer
                isUseRecordData = FALSE;
            }
        }
        else
        {
            // temporarily use the buffer internal to the function
            isUseRecordData = FALSE;
            pAnmResult = &rs->tmpJntAnmResult;
        }

        if (!isUseRecordData)
        {
            // Only NNSG3dJntAnmResult::flag must be cleared in advance before the calculation.
            pAnmResult->flag = (NNSG3dJntAnmResultFlag) 0;

            NNS_G3D_NULL_ASSERT(rs->pRenderObj->funcBlendJnt);
            if (rs->pRenderObj->anmJnt &&
                (*rs->pRenderObj->funcBlendJnt)(pAnmResult, rs->pRenderObj->anmJnt, idxNode))
            {
                //
                // anmResult is being obtained using the joint animation resource
                //
                ;
            }
            else
            {
                //
                // Get anmResult using the static model resource
                //
                const NNSG3dResNodeData* pNd =
                    NNS_G3dGetNodeDataByIdx(rs->pResNodeInfo, idxNode);
                const u8* p = (const u8*)pNd + sizeof(NNSG3dResNodeData);

                //
                // Translation
                //
                if (pNd->flag & NNS_G3D_SRTFLAG_TRANS_ZERO)
                {
                    pAnmResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO;
                }
                else
                {
                    const fx32* p_fx32 = (const fx32*)p;

                    pAnmResult->trans.x = *(p_fx32 + 0);
                    pAnmResult->trans.y = *(p_fx32 + 1);
                    pAnmResult->trans.z = *(p_fx32 + 2);
                    p += 3 * sizeof(fx32);
                }

                //
                // Rotation
                //
                if (pNd->flag & NNS_G3D_SRTFLAG_ROT_ZERO)
                {
                    pAnmResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO;
                }
                else
                {
                    if (pNd->flag & NNS_G3D_SRTFLAG_PIVOT_EXIST)
                    {
                        //
                        // When compressed (mainly for single-axis rotation), the compressed matrix is restored to how it was.
                        // 
                        //
                        fx32 A = *(fx16*)(p + 0);
                        fx32 B = *(fx16*)(p + 2);
                        u32 idxPivot = (u32)( (pNd->flag & NNS_G3D_SRTFLAG_IDXPIVOT_MASK) >> 
                                                        NNS_G3D_SRTFLAG_IDXPIVOT_SHIFT );
                        // clear anmResult.rot
                        MI_Zero36B(&pAnmResult->rot);
                        
                        pAnmResult->rot.a[idxPivot] =
                            (pNd->flag & NNS_G3D_SRTFLAG_PIVOT_MINUS) ?
                                -FX32_ONE :
                                FX32_ONE;
                        
                        pAnmResult->rot.a[pivotUtil_[idxPivot][0]] = A;
                        pAnmResult->rot.a[pivotUtil_[idxPivot][1]] = B;

                        pAnmResult->rot.a[pivotUtil_[idxPivot][2]] =
                            (pNd->flag & NNS_G3D_SRTFLAG_SIGN_REVC) ? -B : B;

                        pAnmResult->rot.a[pivotUtil_[idxPivot][3]] =
                            (pNd->flag & NNS_G3D_SRTFLAG_SIGN_REVD) ? -A : A;

                        p += 2 * sizeof(fx16);
                    }
                    else
                    {
                        // NOTICE:
                        // do not replace in the memory copy API
                        // this is because of the implicit casting from fx16 to fx32

                        const fx16* pp = (const fx16*)p;
                        pAnmResult->rot.a[0] = pNd->_00;
                        pAnmResult->rot.a[1] = *(pp + 0);
                        pAnmResult->rot.a[2] = *(pp + 1);
                        pAnmResult->rot.a[3] = *(pp + 2);
                        pAnmResult->rot.a[4] = *(pp + 3);
                        pAnmResult->rot.a[5] = *(pp + 4);
                        pAnmResult->rot.a[6] = *(pp + 5);
                        pAnmResult->rot.a[7] = *(pp + 6);
                        pAnmResult->rot.a[8] = *(pp + 7);

                        p += 8 * sizeof(fx16);
                    }
                }

                //
                // Scale
                //

                //
                // NOTICE:
                // Calculations can occur when the flag has not been set when classic scaling is off in MayaSSC or Si3d, or when NNS_G3D_SRTFLAG_SCALE_ONE is ON.
                // 
                // 
                //
                NNS_G3D_NULL_ASSERT(rs->funcJntScale);
                (*rs->funcJntScale)(pAnmResult, (fx32*)p, rs->c, pNd->flag);
            }
        }
        rs->pJntAnmResult = pAnmResult;
    }

    // callback B
    // When NNS_G3D_RSFLAG_SKIP is set internally, the geometry engine send action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_B(rs, NNS_G3D_SBC_NODEDESC, &cbTiming);

    if (!cbFlag)
    {
        //
        // send to the geometry engine
        //
        if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
        {
            NNS_G3D_NULL_ASSERT(rs->pJntAnmResult);
            NNS_G3D_NULL_ASSERT(rs->funcJntMtx);

            (*rs->funcJntMtx)(rs->pJntAnmResult);
        }
    }

    rs->pJntAnmResult = NULL;

    // callback C
    // You can insert some sort of processing before the next command.
    // In addition, when NNS_G3D_RSFLAG_SKIP is turned on, the current matrix's store action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_NODEDESC, cbTiming);

    if (opt == NNS_G3D_SBCFLG_001 ||
        opt == NNS_G3D_SBCFLG_011)
    {
        ++cmdLen;

        if (!cbFlag)
        {
            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                u32 idxMtxDest = *(rs->c + 4);
                NNS_G3D_ASSERT(idxMtxDest < 31);

                NNS_G3dGeBufferOP_N(G3OP_MTX_STORE,
                                    &idxMtxDest,
                                    G3OP_MTX_STORE_NPARAMS);
            }
        }
    }

    rs->c += cmdLen;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_BB([000], [001], [010], [011])

    mnemonic:   NNS_G3D_SBC_BB
    operands:   [000]: idxNode
                [001]: idxNode, idxMtxDest
                [010]: idxNode, idxMtxSrc
                [011]: idxNode, idxMtxDest, idxMtxSrc
                0 <= idxMtxSrc, idxMtxDest <= 30
    callbacks:  A: before
                B: during
                C: after

    Explanation of operations:
    - for [010], [011] restore idxMtxSrc to current matrix
    - calls the TIMING_A callback
    - push the projection matrix and set the unit matrix
    - wait for the geometry engine to stop, and take out the current matrix
    - use the CPU to process so that this becomes the billboard matrix, then store in the current matrix
    - calls the TIMING_B callback
    - for [001], [011] store current matrix in idxMtxDest
    - call the TIMING_C callback
    - add rs->c

    Remarks:
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_BB(NNSG3dRS* rs, u32 opt)
{
    //
    // Display billboards parallel to the camera projection plane
    //
    u32 cmdLen = 2;

    static u32 bbcmd1[] =
    {
        GX_PACK_OP(G3OP_MTX_POP, G3OP_MTX_MODE, G3OP_MTX_LOAD_4x3, G3OP_MTX_SCALE),
        1,
        GX_MTXMODE_POSITION_VECTOR,
        FX32_ONE, 0, 0, 
        0, FX32_ONE, 0,
        0, 0, FX32_ONE,
        0, 0, 0,   // This area is subject to change (Trans)
        0, 0, 0    // subject to change (Scale)
    };
    
    VecFx32* trans = (VecFx32*)&bbcmd1[12];
    VecFx32* scale = (VecFx32*)&bbcmd1[15];
    MtxFx44 m;
    BOOL cbFlag;
    NNSG3dSbcCallBackTiming cbTiming;

    NNS_G3D_NULL_ASSERT(rs);

    if (rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW)
    {
        // billboard goes into the Draw category
        if (opt == NNS_G3D_SBCFLG_010 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;
        }
        if (opt == NNS_G3D_SBCFLG_001 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;
        }
        rs->c += cmdLen;
        return;
    }

    {
        if (opt == NNS_G3D_SBCFLG_010 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;

            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                u32 idxMtxSrc;
                if (opt == NNS_G3D_SBCFLG_010)
                {
                    idxMtxSrc = *(rs->c + 2);
                }
                else
                {
                    idxMtxSrc = *(rs->c + 3);
                }
                NNS_G3D_ASSERT(idxMtxSrc < 31);

                NNS_G3dGeBufferOP_N(G3OP_MTX_RESTORE,
                                    &idxMtxSrc,
                                    1);
            }
        }
    }

    // callback A
    // When NNS_G3D_RSFLAG_SKIP is set internally, the billboard matrix's settings action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_BB, &cbTiming);

    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD) &&
        (!cbFlag))
    {
        // Flush the buffer
        NNS_G3dGeFlushBuffer();

        //
        // Immediate sending is possible up to the next callback point.
        //

        // Command transfer:
        // Change to PROJ mode
        // Back up the projection matrix
        // Set the unit matrix
        reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_MODE, G3OP_MTX_PUSH, G3OP_MTX_IDENTITY, G3OP_NOP);
        reg_G3X_GXFIFO = (u32)GX_MTXMODE_PROJECTION;
        reg_G3X_GXFIFO = 0; // 2004/08/26 geometry fifo glitch

        // Also, wait for the geometry engine to stop
        // Get the current matrix
        while (G3X_GetClipMtx(&m))
            ;

        if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_WVP)
        {
            // When a camera matrix affects a projection matrix, an SRT-added camera matrix has to be multiplied from the back in advance.
            // 
            const MtxFx43* cam = NNS_G3dGlbGetSrtCameraMtx();
            MtxFx44 tmp;

            MTX_Copy43To44(cam, &tmp);
            MTX_Concat44(&m, &tmp, &m);
        }
        else if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_VP)
        {
            const MtxFx43* cam = NNS_G3dGlbGetCameraMtx();
            MtxFx44 tmp;

            MTX_Copy43To44(cam, &tmp);
            MTX_Concat44(&m, &tmp, &m);
        }

        // Billboard matrix calculation
        trans->x = m._30;
        trans->y = m._31;
        trans->z = m._32;

        scale->x = VEC_Mag((VecFx32*)&m._00);
        scale->y = VEC_Mag((VecFx32*)&m._10);
        scale->z = VEC_Mag((VecFx32*)&m._20);

        if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_WVP)
        {
            // Projection matrix POP
            // Restore to POS_VEC
            // set the inverse matrix of the camera with SRT
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_POP, G3OP_MTX_MODE, G3OP_MTX_LOAD_4x3, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[1],
                         &reg_G3X_GXFIFO,
                         2 * sizeof(u32));
            MI_CpuSend32(NNS_G3dGlbGetInvSrtCameraMtx(),
                         &reg_G3X_GXFIFO,
                         G3OP_MTX_LOAD_4x3_NPARAMS * sizeof(u32));

            // multiply by current matrix
            // multiply the calculated scale
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_MULT_4x3, G3OP_MTX_SCALE, G3OP_NOP, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[3],
                         &reg_G3X_GXFIFO,
                         sizeof(MtxFx43) + sizeof(VecFx32));
        }
        else if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_VP)
        {
            // Projection matrix POP
            // Restore to POS_VEC
            // Set the camera's inverse matrix
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_POP, G3OP_MTX_MODE, G3OP_MTX_LOAD_4x3, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[1],
                         &reg_G3X_GXFIFO,
                         2 * sizeof(u32));
            MI_CpuSend32(NNS_G3dGlbGetInvCameraMtx(),
                         &reg_G3X_GXFIFO,
                         G3OP_MTX_LOAD_4x3_NPARAMS * sizeof(u32));

            // multiply by current matrix
            // multiply the calculated scale
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_MULT_4x3, G3OP_MTX_SCALE, G3OP_NOP, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[3],
                         &reg_G3X_GXFIFO,
                         sizeof(MtxFx43) + sizeof(VecFx32));
        }
        else
        {
            // Projection matrix POP
            // Restore to POS_VEC
            // Store in the current matrix
            // multiply the calculated scale
            MI_CpuSend32(&bbcmd1[0],
                         &reg_G3X_GXFIFO,
                         18 * sizeof(u32));
        }
    }

    // callback C
    // You can insert some sort of processing before the next command.
    // In addition, when NNS_G3D_RSFLAG_SKIP is turned on, the current matrix's store action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_BB, cbTiming);

    if (opt == NNS_G3D_SBCFLG_001 ||
        opt == NNS_G3D_SBCFLG_011)
    {
        ++cmdLen;

        if (!cbFlag)
        {
            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                u32 idxMtxDest;
                idxMtxDest = *(rs->c + 2);

                NNS_G3D_ASSERT(idxMtxDest < 31);

                // Here, it may not be possible to immediately send.
                NNS_G3dGeBufferOP_N(G3OP_MTX_STORE,
                                    &idxMtxDest,
                                    G3OP_MTX_STORE_NPARAMS);
            }
        }
    }

    rs->c += cmdLen;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_BBY([000], [001], [010], [011])

    mnemonic:   NNS_G3D_SBC_BBY
    operands:   [000]: idxNode
                [001]: idxNode, idxMtxDest
                [010]: idxNode, idxMtxSrc
                [011]: idxNode, idxMtxDest, idxMtxSrc
                0 <= idxMtxSrc, idxMtxDest <= 30
    callbacks:  A: before
                B: during
                C: after

    Explanation of operations:
    - for [010], [011] restore idxMtxSrc to current matrix
    - calls the TIMING_A callback
    - push the projection matrix and set the unit matrix
    - wait for the geometry engine to stop, and take out the current matrix
    - use the CPU to process so that it becomes the Y-axis billboard matrix, then store it in the current matrix
    - calls the TIMING_B callback
    - for [001], [011] store current matrix in idxMtxDest
    - call the TIMING_C callback
    - add rs->c

    Remarks:
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_BBY(NNSG3dRS* rs, u32 opt)
{
    u32 cmdLen = 2;
    MtxFx44 m;

    static u32 bbcmd1[] =
    {
        GX_PACK_OP(G3OP_MTX_POP, G3OP_MTX_MODE, G3OP_MTX_LOAD_4x3, G3OP_MTX_SCALE),
        1,
        GX_MTXMODE_POSITION_VECTOR,
        FX32_ONE, 0, 0, // This area is subject to change (4x3Mtx)
        0, FX32_ONE, 0,
        0, 0, FX32_ONE,
        0, 0, 0,   
        0, 0, 0    // subject to change (Scale)
    };
    VecFx32* trans = (VecFx32*)&bbcmd1[12];
    VecFx32* scale = (VecFx32*)&bbcmd1[15];
    MtxFx43* mtx   = (MtxFx43*)&bbcmd1[3];
    BOOL cbFlag;
    NNSG3dSbcCallBackTiming cbTiming;

    NNS_G3D_NULL_ASSERT(rs);

    if (rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW)
    {
        // billboard goes into the Draw category
        if (opt == NNS_G3D_SBCFLG_010 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;
        }
        if (opt == NNS_G3D_SBCFLG_001 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;
        }
        rs->c += cmdLen;
        return;
    }

    {
        if (opt == NNS_G3D_SBCFLG_010 ||
            opt == NNS_G3D_SBCFLG_011)
        {
            ++cmdLen;
            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                u32 idxMtxSrc;
                if (opt == NNS_G3D_SBCFLG_010)
                {
                    idxMtxSrc = *(rs->c + 2);
                }
                else
                {
                    idxMtxSrc = *(rs->c + 3);
                }
                NNS_G3D_ASSERT(idxMtxSrc < 31);
                NNS_G3dGeBufferOP_N(G3OP_MTX_RESTORE,
                                    &idxMtxSrc,
                                    1);
            }
        }
    }

    // callback A
    // When NNS_G3D_RSFLAG_SKIP is set internally, the billboard matrix's settings action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_BBY, &cbTiming);

    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD) &&
        (!cbFlag))
    {
        // Flush the buffer
        NNS_G3dGeFlushBuffer();

        //
        // Immediate sending is possible up to the next callback point.
        //

        // Command transfer:
        // Change to PROJ mode
        // Save the projection matrix
        // Set the unit matrix
        reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_MODE, G3OP_MTX_PUSH, G3OP_MTX_IDENTITY, G3OP_NOP);
        reg_G3X_GXFIFO = (u32)GX_MTXMODE_PROJECTION;
        reg_G3X_GXFIFO = 0; // 2004/08/26 geometry fifo glitch

        // Also, wait for the geometry engine to stop
        // Get the current matrix (clip matrix)
        while (G3X_GetClipMtx(&m))
            ;

        if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_WVP)
        {
            // When a camera matrix affects a projection matrix, an SRT-added camera matrix has to be multiplied from the back in advance.
            // 
            const MtxFx43* cam = NNS_G3dGlbGetSrtCameraMtx();
            MtxFx44 tmp;

            MTX_Copy43To44(cam, &tmp);
            MTX_Concat44(&m, &tmp, &m);
        }
        else if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_VP)
        {
            // When a camera matrix affects a projection matrix, a camera matrix has to be multiplied from the back in advance.
            // 
            const MtxFx43* cam = NNS_G3dGlbGetCameraMtx();
            MtxFx44 tmp;

            MTX_Copy43To44(cam, &tmp);
            MTX_Concat44(&m, &tmp, &m);
        }

        // Billboard matrix calculation

        // 1: divert the translation
        trans->x = m._30;
        trans->y = m._31;
        trans->z = m._32;

        // 2: approximate the scale with the size of the vector on each line
        scale->x = VEC_Mag((VecFx32*)&m._00);
        scale->y = VEC_Mag((VecFx32*)&m._10);
        scale->z = VEC_Mag((VecFx32*)&m._20);

        // 3: rotation matrices are calculated using the following priorities
        //   1. the y-axis direction maintains that of the original matrix
        //   2. the z-axis direction will become parallel to the camera's field of view axis
        {
            VecFx32* vx = (VecFx32*)&mtx->_00;
            VecFx32* vy = (VecFx32*)&mtx->_10;
            VecFx32* vz = (VecFx32*)&mtx->_20;

            VEC_Set(vy, m._10, m._11, m._12);
            NNS_G3D_WARNING((vy->x != 0.0f || vy->y != 0.0f),
                            "Y axis of a 'y' billboard is parallel to eye axis.");
                            // The y-axis of the y-billboard is parallel to the line of sight

            // When vz = {0, 0, FX32_ONE},
            // vx = Cross(vy, vz) = {vy.y, -vy.x, 0}
            VEC_Set(vx, vy->y, -vy->x, 0);

            VEC_Normalize(vx, vx);
            VEC_Normalize(vy, vy);
            VEC_CrossProduct(vx, vy, vz);
        }

        if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_WVP)
        {
            // Projection matrix POP
            // Restore to POS_VEC
            // set the inverse matrix of the camera with SRT
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_POP, G3OP_MTX_MODE, G3OP_MTX_LOAD_4x3, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[1],
                         &reg_G3X_GXFIFO,
                         2 * sizeof(u32));
            MI_CpuSend32(NNS_G3dGlbGetInvSrtCameraMtx(),
                         &reg_G3X_GXFIFO,
                         G3OP_MTX_LOAD_4x3_NPARAMS * sizeof(u32));
            
            // multiply by current matrix
            // multiply the calculated scale        
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_MULT_4x3, G3OP_MTX_SCALE, G3OP_NOP, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[3],
                         &reg_G3X_GXFIFO,
                         sizeof(MtxFx43) + sizeof(VecFx32));
        }
        else if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_VP)
        {
            // Projection matrix POP
            // Restore to POS_VEC
            // Set the camera's inverse matrix
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_POP, G3OP_MTX_MODE, G3OP_MTX_LOAD_4x3, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[1],
                         &reg_G3X_GXFIFO,
                         2 * sizeof(u32));
            MI_CpuSend32(NNS_G3dGlbGetInvCameraMtx(),
                         &reg_G3X_GXFIFO,
                         G3OP_MTX_LOAD_4x3_NPARAMS * sizeof(u32));
            
            // multiply by current matrix
            // multiply the calculated scale        
            reg_G3X_GXFIFO = GX_PACK_OP(G3OP_MTX_MULT_4x3, G3OP_MTX_SCALE, G3OP_NOP, G3OP_NOP);
            MI_CpuSend32(&bbcmd1[3],
                         &reg_G3X_GXFIFO,
                         sizeof(MtxFx43) + sizeof(VecFx32));
        }
        else
        {
            // Projection matrix POP
            // Restore to POS_VEC
            // Store in the current matrix
            // multiply the calculated scale
            MI_CpuSend32(&bbcmd1[0], &reg_G3X_GXFIFO, 18 * sizeof(u32));
        }
    }

    // callback C
    // You can insert some sort of processing before the next command.
    // In addition, when NNS_G3D_RSFLAG_SKIP is turned on, the current matrix's store action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_BBY, cbTiming);

    if (opt == NNS_G3D_SBCFLG_001 ||
        opt == NNS_G3D_SBCFLG_011)
    {
        ++cmdLen;

        if (!cbFlag)
        {
            if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD))
            {
                u32 idxMtxDest;
                idxMtxDest = *(rs->c + 2);

                NNS_G3D_ASSERT(idxMtxDest < 31);
                // Here, it may not be possible to immediately send.
                NNS_G3dGeBufferOP_N(G3OP_MTX_STORE,
                                    &idxMtxDest,
                                    G3OP_MTX_STORE_NPARAMS);
            }
        }
    }

    rs->c += cmdLen;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_NODEMIX

    mnemonic:   NNS_G3D_SBC_NODEMIX
    operands:   idxMtxDest, numMtx, idxStack1, idxMtx1, ratio1, ... ,
                                    idxStackN, idxMtxN, ratioN
    callbacks:  None.

    Explanation of operations:
    - calculates the sum (ratio * InvM(InvN) * AnmM), then stores it in the matrix stack idxMtxDest
      That is, vertices are restored to each joint's local coordinates according to the inverse matrices (InvM, InvN) and blended with the matrix that multiplies the animation matrix.
      
    - The inverse matrix of the modeling conversion matrix (InvM, InvN) is stored in the model resource.
      It is calculated when converting using g3dcvtr.
    - matrix multiplication is performed using the geometry engine
    - vertices with applied weighted envelopes are stored with global coordinates

    Remarks:
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_NODEMIX(NNSG3dRS* rs, u32)
{
    fx64 w = 0;
    const NNSG3dResEvpMtx* evpMtx =
        (const NNSG3dResEvpMtx*)((u8*)rs->pRenderObj->resMdl +
                                    rs->pRenderObj->resMdl->ofsEvpMtx);
    u32 numMtx = *(rs->c + 2);
    u8* p = rs->c + 3;

    NNS_G3D_ASSERT(numMtx >= 2);

#if (NNS_G3D_USE_EVPCACHE)
    {
        u32 i;
        struct
        {
            MtxFx43 M;
            MtxFx33 N;
        } sum;
        MtxFx44* pX;
        MtxFx33* pY;

        MI_CpuClearFast(&sum, sizeof(sum));
        NNS_G3dGeFlushBuffer();

        G3_MtxMode(GX_MTXMODE_PROJECTION);
        G3_StoreMtx(1);
        G3_Identity();
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        for (i = 0; i < numMtx; ++i)
        {
            u32 idxJnt = *(p + 1);
            BOOL evpCached = NNSi_G3dBitVecCheck(&rs->isEvpCached[0], idxJnt);

            pX = &NNS_G3dRSOnGlb.evpCache[idxJnt].M;
            if (!evpCached)
            {
                NNSi_G3dBitVecSet(&rs->isEvpCached[0], idxJnt);

                G3_RestoreMtx(*p);
                G3_MtxMode(GX_MTXMODE_POSITION);
                G3_MultMtx43(&evpMtx[idxJnt].invM);
            }

            if (i != 0)
            {
                sum.N.m[0][0] += (w * pY->m[0][0]) >> FX32_SHIFT;
                sum.N.m[0][1] += (w * pY->m[0][1]) >> FX32_SHIFT;
                sum.N.m[0][2] += (w * pY->m[0][2]) >> FX32_SHIFT;

                sum.N.m[1][0] += (w * pY->m[1][0]) >> FX32_SHIFT;
                sum.N.m[1][1] += (w * pY->m[1][1]) >> FX32_SHIFT;
                sum.N.m[1][2] += (w * pY->m[1][2]) >> FX32_SHIFT;

                sum.N.m[2][0] += (w * pY->m[2][0]) >> FX32_SHIFT;
                sum.N.m[2][1] += (w * pY->m[2][1]) >> FX32_SHIFT;
                sum.N.m[2][2] += (w * pY->m[2][2]) >> FX32_SHIFT;
            }

            if (!evpCached)
            {
                while (G3X_GetClipMtx(pX))
                    ;
                G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
                G3_MultMtx33(&evpMtx[idxJnt].invN);
            }

            w = *(p + 2) << 4;

            sum.M.m[0][0] += (w * pX->m[0][0]) >> FX32_SHIFT;
            sum.M.m[0][1] += (w * pX->m[0][1]) >> FX32_SHIFT;
            sum.M.m[0][2] += (w * pX->m[0][2]) >> FX32_SHIFT;

            sum.M.m[1][0] += (w * pX->m[1][0]) >> FX32_SHIFT;
            sum.M.m[1][1] += (w * pX->m[1][1]) >> FX32_SHIFT;
            sum.M.m[1][2] += (w * pX->m[1][2]) >> FX32_SHIFT;

            sum.M.m[2][0] += (w * pX->m[2][0]) >> FX32_SHIFT;
            sum.M.m[2][1] += (w * pX->m[2][1]) >> FX32_SHIFT;
            sum.M.m[2][2] += (w * pX->m[2][2]) >> FX32_SHIFT;

            sum.M.m[3][0] += (w * pX->m[3][0]) >> FX32_SHIFT;
            sum.M.m[3][1] += (w * pX->m[3][1]) >> FX32_SHIFT;
            sum.M.m[3][2] += (w * pX->m[3][2]) >> FX32_SHIFT;

            p += 3;
            pY = &NNS_G3dRSOnGlb.evpCache[idxJnt].N;

            if (!evpCached)
            {
                while (G3X_GetVectorMtx(pY))
                    ;
            }
        }
        sum.N.m[0][0] += (w * pY->m[0][0]) >> FX32_SHIFT;
        sum.N.m[0][1] += (w * pY->m[0][1]) >> FX32_SHIFT;
        sum.N.m[0][2] += (w * pY->m[0][2]) >> FX32_SHIFT;

        sum.N.m[1][0] += (w * pY->m[1][0]) >> FX32_SHIFT;
        sum.N.m[1][1] += (w * pY->m[1][1]) >> FX32_SHIFT;
        sum.N.m[1][2] += (w * pY->m[1][2]) >> FX32_SHIFT;

        sum.N.m[2][0] += (w * pY->m[2][0]) >> FX32_SHIFT;
        sum.N.m[2][1] += (w * pY->m[2][1]) >> FX32_SHIFT;
        sum.N.m[2][2] += (w * pY->m[2][2]) >> FX32_SHIFT;

        G3_LoadMtx43((const MtxFx43*)&sum.N); // This gets overwritten, so it's OK if the garbage data gets mixed up.
        G3_MtxMode(GX_MTXMODE_POSITION);
        G3_LoadMtx43(&sum.M);
        G3_MtxMode(GX_MTXMODE_PROJECTION);
        G3_RestoreMtx(1);
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
    }
#else
    {
        u32 i;
        struct
        {
            MtxFx43 M;
            MtxFx33 N;
        } sum;
        MtxFx44 X;
        MtxFx33 Y;

        MI_CpuClearFast(&sum, sizeof(sum));
        NNS_G3dGeFlushBuffer();

        G3_MtxMode(GX_MTXMODE_PROJECTION);
        G3_StoreMtx(1);
        G3_Identity();
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

        for (i = 0; i < numMtx; ++i)
        {
            u32 idxJnt = *(p + 1);
            
            G3_RestoreMtx(*p);
            G3_MtxMode(GX_MTXMODE_POSITION);
            G3_MultMtx43(&evpMtx[idxJnt].invM);

            if (i != 0)
            {
                sum.N.m[0][0] += (w * Y.m[0][0]) >> FX32_SHIFT;
                sum.N.m[0][1] += (w * Y.m[0][1]) >> FX32_SHIFT;
                sum.N.m[0][2] += (w * Y.m[0][2]) >> FX32_SHIFT;

                sum.N.m[1][0] += (w * Y.m[1][0]) >> FX32_SHIFT;
                sum.N.m[1][1] += (w * Y.m[1][1]) >> FX32_SHIFT;
                sum.N.m[1][2] += (w * Y.m[1][2]) >> FX32_SHIFT;

                sum.N.m[2][0] += (w * Y.m[2][0]) >> FX32_SHIFT;
                sum.N.m[2][1] += (w * Y.m[2][1]) >> FX32_SHIFT;
                sum.N.m[2][2] += (w * Y.m[2][2]) >> FX32_SHIFT;
            }

            while (G3X_GetClipMtx(&X))
                ;
            G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
            G3_MultMtx33(&evpMtx[idxJnt].invN);

            w = *(p + 2) << 4;
            sum.M.m[0][0] += (w * X.m[0][0]) >> FX32_SHIFT;
            sum.M.m[0][1] += (w * X.m[0][1]) >> FX32_SHIFT;
            sum.M.m[0][2] += (w * X.m[0][2]) >> FX32_SHIFT;

            sum.M.m[1][0] += (w * X.m[1][0]) >> FX32_SHIFT;
            sum.M.m[1][1] += (w * X.m[1][1]) >> FX32_SHIFT;
            sum.M.m[1][2] += (w * X.m[1][2]) >> FX32_SHIFT;

            sum.M.m[2][0] += (w * X.m[2][0]) >> FX32_SHIFT;
            sum.M.m[2][1] += (w * X.m[2][1]) >> FX32_SHIFT;
            sum.M.m[2][2] += (w * X.m[2][2]) >> FX32_SHIFT;

            sum.M.m[3][0] += (w * X.m[3][0]) >> FX32_SHIFT;
            sum.M.m[3][1] += (w * X.m[3][1]) >> FX32_SHIFT;
            sum.M.m[3][2] += (w * X.m[3][2]) >> FX32_SHIFT;
            p += 3;

            while (G3X_GetVectorMtx(&Y))
                ;
        }
        sum.N.m[0][0] += (w * Y.m[0][0]) >> FX32_SHIFT;
        sum.N.m[0][1] += (w * Y.m[0][1]) >> FX32_SHIFT;
        sum.N.m[0][2] += (w * Y.m[0][2]) >> FX32_SHIFT;

        sum.N.m[1][0] += (w * Y.m[1][0]) >> FX32_SHIFT;
        sum.N.m[1][1] += (w * Y.m[1][1]) >> FX32_SHIFT;
        sum.N.m[1][2] += (w * Y.m[1][2]) >> FX32_SHIFT;

        sum.N.m[2][0] += (w * Y.m[2][0]) >> FX32_SHIFT;
        sum.N.m[2][1] += (w * Y.m[2][1]) >> FX32_SHIFT;
        sum.N.m[2][2] += (w * Y.m[2][2]) >> FX32_SHIFT;

        G3_LoadMtx43((const MtxFx43*)&sum.N); // This gets overwritten, so it's OK if the garbage data gets mixed up.
        G3_MtxMode(GX_MTXMODE_POSITION);
        G3_LoadMtx43(&sum.M);
        G3_MtxMode(GX_MTXMODE_PROJECTION);
        G3_RestoreMtx(1);
        G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);
    }
#endif
    G3_StoreMtx(*(rs->c + 1));
    rs->c += 3 + *(rs->c + 2) * 3;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_CALLDL

    mnemonic:   NNS_G3D_SBC_CALLDL
    operands:   rel_addr(1 word), size(1 word)
    callbacks:  A: before
                B: None.
                C: after
    Explanation of operations:
    - calls the TIMING_A callback
    -  performs the display list transmission
    - call the TIMING_C callback
    - add rs->c

    Remarks:
    When the matrix has been set for the first time with NODEDESC and a convert method that rapidly renders simple models by calling CALLDL is implemented, this command will most likely be necessary.
    
    This is also one method for user hacking.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_CALLDL(NNSG3dRS* rs, u32)
{
    BOOL cbFlag;
    NNSG3dSbcCallBackTiming cbTiming;

    NNS_G3D_NULL_ASSERT(rs);

    // callback A
    // When NNS_G3D_RSFLAG_SKIP is set internally, the display list send action can be replaced.
    // 
    cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_CALLDL, &cbTiming);

    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD) &&
        (!cbFlag))
    {
        u32 rel_addr;
        u32 size;
        rel_addr = (u32)( (*(rs->c + 1) << 0 ) |
                          (*(rs->c + 2) << 8 ) |
                          (*(rs->c + 3) << 16) |
                          (*(rs->c + 4) << 24) );

        size = (u32)( (*(rs->c + 5) << 0 ) |
                      (*(rs->c + 6) << 8 ) |
                      (*(rs->c + 7) << 16) |
                      (*(rs->c + 8) << 24) );

        NNS_G3dGeSendDL(rs->c + rel_addr, size);
    }

    // callback C
    // You can insert some sort of processing before the next command.
    (void) NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_CALLDL, cbTiming);

    rs->c += 1 + sizeof(u32) + sizeof(u32);
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_POSSCALE([000], [001])

    mnemonic:   NNS_G3D_SBC_POSSCALE
    operands:   None.
    callbacks:  A: None.
                B: None.
                C: None.

    Explanation of operations:
    - multiply the current matrix by the scaling matrix
       [000] multiply by the scaling matrix that has posScale within the model data set as an element
       [001] multiply by the scaling matrix that has invPosScale within the model data set as an element
    - add rs->c

    Remarks:
    When the pos_scale of the imd file is non-zero, this command is output.
    It must be placed immediately before the render.
    It does not place a callback, because there is no need for user attention.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_POSSCALE(NNSG3dRS* rs, u32 opt)
{
    VecFx32 s;
    NNS_G3D_NULL_ASSERT(rs);
    NNS_G3D_ASSERT(opt == NNS_G3D_SBCFLG_000 ||
                   opt == NNS_G3D_SBCFLG_001);

    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_NOGECMD) &&
        !(rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW))
    {
        if (opt == NNS_G3D_SBCFLG_000)
        {
            s.x = s.y = s.z = rs->posScale;
        }
        else
        {
            // NNS_G3D_SBCFLG_001
            s.x = s.y = s.z = rs->invPosScale;
        }
        NNS_G3dGeBufferOP_N(G3OP_MTX_SCALE,
                            (u32*)&s.x,
                            G3OP_MTX_SCALE_NPARAMS);
    }

    rs->c += 1;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_ENVMAP

    mnemonic:   NNS_G3D_SBC_ENVMAP
    operands:   idxMat, flags (reserved)
    callbacks:  A: customization for normal -> texture coordinate mapping matrix
                B: customization for effect_mtx settings
                C: customization for normal transformation

    Explanation of operations:
    nrm * (C: normal transformation matrix) * (B: effect matrix set in .imd) * 
          (A: mapping matrix) * (texture matrix set in material)
    The above calculations are performed.
    The values for A, B and C can be customized by setting your own callbacks.
    In the default state, mapping occurs such that one texture each is applied to the front and rear surfaces.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_ENVMAP(NNSG3dRS* rs, u32)
{
    NNS_G3D_NULL_ASSERT(rs);

    // If not VISIBLE the instruction will be skipped.
    // Even if the MatID status is the same, if the node is different you must recalculate.
    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW) &&
        (rs->flag & NNS_G3D_RSFLAG_NODE_VISIBLE))
    {
        BOOL cbFlag;
        NNSG3dSbcCallBackTiming cbTiming;

        // When a texture SRT animation has been added, TEXGEN becomes GX_TEXGEN_TEXCOORD, so it is reset to GX_TEXGEN_NORMAL.
        // 
        // 
        if ((rs->pMatAnmResult->prmTexImage & REG_G3_TEXIMAGE_PARAM_TGEN_MASK) !=
                (GX_TEXGEN_NORMAL << REG_G3_TEXIMAGE_PARAM_TGEN_SHIFT))
        {
            static u32 cmd[] =
            {
                G3OP_TEXIMAGE_PARAM,
                0
            };
            rs->pMatAnmResult->prmTexImage &= ~REG_G3_TEXIMAGE_PARAM_TGEN_MASK;
            rs->pMatAnmResult->prmTexImage |= GX_TEXGEN_NORMAL << REG_G3_TEXIMAGE_PARAM_TGEN_SHIFT;

            cmd[1] = rs->pMatAnmResult->prmTexImage;
            NNS_G3dGeBufferData_N(&cmd[0], 2);
        }

        // Sets the texture matrix as the operation target.
        NNS_G3dGeMtxMode(GX_MTXMODE_TEXTURE);

        // callback A
        // When NNS_G3D_RSFLAG_SKIP is set internally, the qMtx settings can be replaced.
        // 
        cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_ENVMAP, &cbTiming);
        if (!cbFlag)
        {
            // processes that can be customized with callback A
            s32 width, height;
            width = (s32)rs->pMatAnmResult->origWidth;
            height =(s32)rs->pMatAnmResult->origHeight;

            // NOTICE:
            // S,T coordinates cannot be not divided by q after transformation, therefore it will not be possible to implement Paraboloid Mapping.
            {
                //     0.5   0   0   0
                // m =  0  -0.5  0   0
                //      0    0   1   0
                //     0.5  0.5  0   1

                // Map so that one texture each is mapped respectively to the front and back faces.
                NNS_G3dGeScale(width << (FX32_SHIFT + 3), -height << (FX32_SHIFT + 3), FX32_ONE << 4);
                NNS_G3dGeTexCoord(width << (FX32_SHIFT - 1), height << (FX32_SHIFT - 1));
            }
        }

        // callback B
        // When NNS_G3D_RSFLAG_SKIP is set internally, the effect_mtx settings can be replaced.
        // 
        cbFlag = NNSi_G3dCallBackCheck_B(rs, NNS_G3D_SBC_ENVMAP, &cbTiming);
        if (!cbFlag)
        {
            u32 idxMat = *(rs->c + 1);
            const NNSG3dResMatData* mat =
                NNS_G3dGetMatDataByIdx(rs->pResMat, idxMat);

            // When effect_mtx exists in a NITRO intermediate file, multiple by effect_mtx.
            // 
            if (mat->flag & NNS_G3D_MATFLAG_EFFECTMTX)
            {
                // if effect_mtx exists in a NITRO intermediate file
                const MtxFx44* effect_mtx;
                const u8* p = (const u8*)mat + sizeof(NNSG3dResMatData);

                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_SCALEONE))
                {
                    p += sizeof(fx32) + sizeof(fx32); // scaleS, scaleT
                }

                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_ROTZERO))
                {
                    p += sizeof(fx16) + sizeof(fx16); // rotSin, rotCos
                }

                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_TRANSZERO))
                {
                    p += sizeof(fx32) + sizeof(fx32); // transS, transT
                }

                effect_mtx = (const MtxFx44*)p;
                NNS_G3dGeMultMtx44(effect_mtx);
            }
        }

        // callback C
        // When NNS_G3D_RSFLAG_SKIP is set internally, the input normal vector's conversion matrix action can be replaced.
        // 
        cbFlag = NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_ENVMAP, cbTiming);
        if (!cbFlag)
        {
            MtxFx33 n;
            NNS_G3dGeMtxMode(GX_MTXMODE_POSITION_VECTOR);
            NNS_G3dGetCurrentMtx(NULL, &n);
            NNS_G3dGeMtxMode(GX_MTXMODE_TEXTURE);

            //
            // Transforms the input normal to world coordinate system directions.
            // The normal vector that is input has the same direction as the geometry command (joint coordinate system).
            //
            if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_WVP)
            {
                // For NNS_G3dGlbFlushWVP, conversions are performed from joint coordinates to local coordinates to view coordinates.
                // 
                // 
                NNS_G3dGeMultMtx33((const MtxFx33*)NNS_G3dGlbGetCameraMtx()); // world coordinate system -> view coordinate system
                NNS_G3dGeMultMtx33(NNS_G3dGlbGetBaseRot());                   // local coordinate system -> world coordinate system
                NNS_G3dGeMultMtx33(&n);                                       // joint coordinate system -> local coordinate system

            }
            else if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_VP)
            {
                // In the case of NNS_G3dGlbFlushVP, it will become the world coordinate system directional vector as is.
                // 
                NNS_G3dGeMultMtx33((const MtxFx33*)NNS_G3dGlbGetCameraMtx()); // world coordinate system -> view coordinate system
                NNS_G3dGeMultMtx33(&n);                                       // joint coordinate system -> world coordinate system
            }
            else
            {
                NNS_G3dGeMultMtx33(&n);                            // joint coordinate system -> view coordinate system
            }
        }

        // Restores the operation target matrix to how it was before.
        NNS_G3dGeMtxMode(GX_MTXMODE_POSITION_VECTOR);
    }
    rs->c += 3;
}


/*---------------------------------------------------------------------------*
    NNSi_G3dFuncSbc_PRJMAP

    mnemonic:   NNS_G3D_SBC_PRJMAP([000], [001], [010])
    operands:   idxMat
    callbacks:  A: customization for vertex coordinate -> texture coordinate mapping matrix
                B: customization for effect_mtx settings
                C: customization for vertex coordinate transformation

    Explanation of operations:
    Because it is necessary to get the value from the calculated texture matrix and configure it as TexCoord, a position coordinate matrix that can be used for that purpose is used for calculations.
    
    After that it extracts and sets to the texture matrix and TexCoord.
    The texture matrix that was set by the MAT command will be overwritten, and become invalid.
 *---------------------------------------------------------------------------*/
void
NNSi_G3dFuncSbc_PRJMAP(NNSG3dRS* rs, u32 )
{
    //
    // Because it is necessary to set part of the texture matrix as the TexCoord, get that part after using NNS_G3D_MTXSTACK_SYS to calculate.
    // 
    //

    NNS_G3D_NULL_ASSERT(rs);

    // If not VISIBLE the instruction will be skipped.
    // Even if the MatID status is the same, if the node is different you must recalculate.
    if (!(rs->flag & NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW) &&
        (rs->flag & NNS_G3D_RSFLAG_NODE_VISIBLE))
    {
        BOOL cbFlag;
        NNSG3dSbcCallBackTiming cbTiming;

        // Extracts a current position coordinate matrix that will be used later.
        MtxFx43 m;
        NNS_G3dGetCurrentMtx(&m, NULL);
        NNS_G3dGeStoreMtx(NNS_G3D_MTXSTACK_SYS);

        // When a texture SRT animation has been added, TEXGEN becomes GX_TEXGEN_TEXCOORD, so it is reset to GX_TEXGEN_VERTEX.
        // 
        // 
        if ((rs->pMatAnmResult->prmTexImage & REG_G3_TEXIMAGE_PARAM_TGEN_MASK) !=
                (GX_TEXGEN_VERTEX << REG_G3_TEXIMAGE_PARAM_TGEN_SHIFT))
        {
            static u32 cmd[] =
            {
                G3OP_TEXIMAGE_PARAM,
                0
            };
            rs->pMatAnmResult->prmTexImage &= ~REG_G3_TEXIMAGE_PARAM_TGEN_MASK;
            rs->pMatAnmResult->prmTexImage |= GX_TEXGEN_VERTEX << REG_G3_TEXIMAGE_PARAM_TGEN_SHIFT;

            cmd[1] = rs->pMatAnmResult->prmTexImage;
            NNS_G3dGeBufferData_N(&cmd[0], 2);
        }

        // callback A
        // When NNS_G3D_RSFLAG_SKIP is set internally, the qMtx settings can be replaced.
        // 
        cbFlag = NNSi_G3dCallBackCheck_A(rs, NNS_G3D_SBC_PRJMAP, &cbTiming);
        if (!cbFlag)
        {
            // processes that can be customized with callback A
            s32 width, height;
            width = (s32)rs->pMatAnmResult->origWidth;
            height = (s32)rs->pMatAnmResult->origHeight;

            {
                static MtxFx44 mtx = {
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, FX32_ONE << 4, 0,
                    0, 0, 0, FX32_ONE << 4
                };

                mtx._00 = width << (FX32_SHIFT + 3);
                mtx._11 = -height << (FX32_SHIFT + 3);
                mtx._30 = width << (FX32_SHIFT + 3);
                mtx._31 = height << (FX32_SHIFT + 3);

                //        0.5   0   0   0
                // mtx =   0  -0.5  0   0
                //         0    0   1   0
                //        0.5  0.5  0   1
                NNS_G3dGeLoadMtx44(&mtx);
            }
        }

        // callback B
        // When NNS_G3D_RSFLAG_SKIP is set internally, the effect_mtx settings can be replaced.
        // 
        cbFlag = NNSi_G3dCallBackCheck_B(rs, NNS_G3D_SBC_PRJMAP, &cbTiming);
        if (!cbFlag)
        {
            u32 idxMat = *(rs->c + 1);
            const NNSG3dResMatData* mat =
                NNS_G3dGetMatDataByIdx(rs->pResMat, idxMat);

            // When effect_mtx exists in a NITRO intermediate file, multiple by effect_mtx.
            // 
            if (mat->flag & NNS_G3D_MATFLAG_EFFECTMTX)
            {
                // if effect_mtx exists in a NITRO intermediate file
                const MtxFx44* effect_mtx;
                const u8* p = (const u8*)mat + sizeof(NNSG3dResMatData);

                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_SCALEONE))
                {
                    p += sizeof(fx32) + sizeof(fx32); // scaleS, scaleT
                }

                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_ROTZERO))
                {
                    p += sizeof(fx16) + sizeof(fx16); // rotSin, rotCos
                }

                if (!(mat->flag & NNS_G3D_MATFLAG_TEXMTX_TRANSZERO))
                {
                    p += sizeof(fx32) + sizeof(fx32); // transS, transT
                }

                effect_mtx = (const MtxFx44*)p;
                NNS_G3dGeMultMtx44(effect_mtx);
            }
        }

        // callback C
        // When NNS_G3D_RSFLAG_SKIP is set internally, the input normal vector's conversion matrix action can be replaced.
        // 
        cbFlag = NNSi_G3dCallBackCheck_C(rs, NNS_G3D_SBC_PRJMAP, cbTiming);
        if (!cbFlag)
        {
            MtxFx44 tex_mtx;

            //
            // Transforms the input coordinates to world coordinate system coordinates.
            // The normal vector that is input has the same direction as the geometry command (joint coordinate system).
            //
            if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_WVP)
            {
                // For NNS_G3dGlbFlushWVP, conversions are performed from joint coordinates to local coordinates to world coordinates.
                // 
                // 
                NNS_G3dGeTranslateVec(NNS_G3dGlbGetBaseTrans());
                NNS_G3dGeMultMtx33(NNS_G3dGlbGetBaseRot());  // local coordinate system -> world coordinate system
                NNS_G3dGeMultMtx43(&m);                      // joint coordinate system -> local coordinate system

            }
            else if (NNS_G3dGlb.flag & NNS_G3D_GLB_FLAG_FLUSH_VP)
            {
                // In the case of NNS_G3dGlbFlushVP, it will become the world coordinate system directional vector as is.
                // 
                NNS_G3dGeMultMtx43(&m);                      // joint coordinate system -> world coordinate system
            }
            else
            {
                // For NNS_G3dGlbFlushP, conversions are performed from joint coordinates to camera coordinates to world coordinates.
                // 
                // 
                NNS_G3dGeMultMtx43(NNS_G3dGlbGetInvV()); // camera coordinate system -> world coordinate system
                NNS_G3dGeMultMtx43(&m);                            // joint coordinate system -> camera coordinate system
            }

            //
            // Reads back the calculated texture matrix from the position coordinate matrix stack.
            //
            {
                NNS_G3dGeFlushBuffer();

                G3_MtxMode(GX_MTXMODE_PROJECTION);
                G3_PushMtx();
                G3_Identity();

                while(G3X_GetClipMtx(&tex_mtx))
                    ;

                G3_PopMtx(1);
                G3_MtxMode(GX_MTXMODE_TEXTURE);
            }

            NNS_G3dGeLoadMtx44(&tex_mtx);
            NNS_G3dGeTexCoord(tex_mtx._30 >> 4, tex_mtx._31 >> 4);
        }

        // Restores the operation target matrix to how it was before.
        NNS_G3dGeMtxMode(GX_MTXMODE_POSITION_VECTOR);
        NNS_G3dGeRestoreMtx(NNS_G3D_MTXSTACK_SYS);
    }
    rs->c += 3;
}




////////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//

/*---------------------------------------------------------------------------*
    NNS_G3dRS

    This is the pointer to the structure that holds the state held by the program (the state of the state machine) during rendering.
    Memory for the structure is allocated in the stack region (DTCM) in NNS_G3dDraw. 
    NNS_G3dRS is cleaned up to NULL when NNS_G3dDraw ends.
 *---------------------------------------------------------------------------*/
NNSG3dRS* NNS_G3dRS = NULL;


/*---------------------------------------------------------------------------*
    NNS_G3dRSOnGlb

    Because the size will be big when the program enters a hold state (the state of the state machine) during rendering, it is not placed in the stack region.
    
 *---------------------------------------------------------------------------*/
NNSG3dRSOnGlb NNS_G3dRSOnGlb;


/*---------------------------------------------------------------------------*
    NNS_G3dFuncSbcTable

    The SBC code handler is registered.
    This is not a const because the user can re-write it.
 *---------------------------------------------------------------------------*/
NNSG3dFuncSbc NNS_G3dFuncSbcTable[NNS_G3D_SBC_COMMAND_NUM] =
{
    &NNSi_G3dFuncSbc_NOP,
    &NNSi_G3dFuncSbc_RET,
    &NNSi_G3dFuncSbc_NODE,
    &NNSi_G3dFuncSbc_MTX,
    &NNSi_G3dFuncSbc_MAT,
    &NNSi_G3dFuncSbc_SHP,
    &NNSi_G3dFuncSbc_NODEDESC,
    &NNSi_G3dFuncSbc_BB,

    &NNSi_G3dFuncSbc_BBY,
    &NNSi_G3dFuncSbc_NODEMIX,
    &NNSi_G3dFuncSbc_CALLDL,
    &NNSi_G3dFuncSbc_POSSCALE,
    &NNSi_G3dFuncSbc_ENVMAP,
    &NNSi_G3dFuncSbc_PRJMAP
};


/*---------------------------------------------------------------------------*
    NNS_G3dFuncSbcShpTable

    Individual tags at the top of the shape data are indexed to this table.
    Register the handler according to shape data type.
 *---------------------------------------------------------------------------*/
NNSG3dFuncSbc_ShpInternal NNS_G3dFuncSbcShpTable[NNS_G3D_SIZE_SHP_VTBL_NUM] =
{
    &NNSi_G3dFuncSbc_SHP_InternalDefault
};


/*---------------------------------------------------------------------------*
    NNS_G3dFuncSbcMatTable

    Individual tags at the top of the material data are indexed to this table.
    Register the handler according to material data type.
 *---------------------------------------------------------------------------*/
NNSG3dFuncSbc_MatInternal NNS_G3dFuncSbcMatTable[NNS_G3D_SIZE_MAT_VTBL_NUM] =
{
    &NNSi_G3dFuncSbc_MAT_InternalDefault
};
