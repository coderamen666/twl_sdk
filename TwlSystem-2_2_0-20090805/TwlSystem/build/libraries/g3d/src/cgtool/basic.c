/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g3d - src - cgtool
  File:     basic.c

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

#include <nnsys/g3d/cgtool/basic.h>
#include <nnsys/g3d/gecom.h>
#include <nnsys/g3d/binres/res_struct.h>


/*---------------------------------------------------------------------------*
    NNSi_G3dSendJointSRTBasic

    This configures the joint matrix in the geometry engine.
    Normally, a pointer is stored in NNS_G3dSendJointSRT_FuncArray, and called when NNS_G3D_SCALINGRULE_STANDARD (when <model_info>::scaling_rule is standard) is specified.
    
    

    In addition, Position/Vector mode is required and the target matrix must be the current matrix when called.
    
 *---------------------------------------------------------------------------*/
void NNSi_G3dSendJointSRTBasic(const NNSG3dJntAnmResult* result)
{
    NNS_G3D_NULL_ASSERT(result);

    // 
    // At this point, the matrix mode has to be Position/Vector mode.
    // The matrix that is the subject of the process must be in the current matrix.
    if (!(result->flag & NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO))
    {
        if (!(result->flag & NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO))
        {
            // HACK ALERT
            // It is assumed that 'rot' and 'trans' are contiguous.
            // That is, the code depends on the order of the members of NNSG3dJntAnmResult.
            NNS_G3dGeBufferOP_N(G3OP_MTX_MULT_4x3,
                                (u32*)&result->rot._00,
                                G3OP_MTX_MULT_4x3_NPARAMS);
        }
        else
        {
            // Position/Vector mode is slow, but it's not desirable to use the CPU just to send the command to switch modes.
            // 
            // 
            NNS_G3dGeBufferOP_N(G3OP_MTX_TRANS,
                                (u32*)&result->trans.x,
                                G3OP_MTX_TRANS_NPARAMS);
        }
    }
    else
    {
        if (!(result->flag & NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO))
        {
            NNS_G3dGeBufferOP_N(G3OP_MTX_MULT_3x3,
                                (u32*)&result->rot._00,
                                G3OP_MTX_MULT_3x3_NPARAMS);
        }
    }

    if (!(result->flag & NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE))
    {
        NNS_G3dGeBufferOP_N(G3OP_MTX_SCALE,
                            (u32*)&result->scale.x,
                            G3OP_MTX_SCALE_NPARAMS);
    }
}


/*---------------------------------------------------------------------------*
    NNSi_G3dGetJointScaleBasic

    Performs the most basic of scaling, *(p + 3), *(p + 4) or *(p + 5) is not used.
 *---------------------------------------------------------------------------*/
void NNSi_G3dGetJointScaleBasic(NNSG3dJntAnmResult* pResult,
                                const fx32* p,
                                const u8*,
                                u32 srtflag)
{
    // NOTICE:
    // cmd can be NULL
    // if NNS_G3D_SRTFLAG_SCALE_ONE is ON, p can be NULL

    NNS_G3D_NULL_ASSERT(pResult);
    
    if (srtflag & NNS_G3D_SRTFLAG_SCALE_ONE)
    {
        pResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE;
    }
    else
    {
        NNS_G3D_NULL_ASSERT(p);

        pResult->scale.x = *(p + 0);
        pResult->scale.y = *(p + 1);
        pResult->scale.z = *(p + 2);
    }

    // not used, but best to set anyway (to balance with blend)
    pResult->flag |= NNS_G3D_JNTANM_RESULTFLAG_SCALEEX0_ONE |
                     NNS_G3D_JNTANM_RESULTFLAG_SCALEEX1_ONE;
}

