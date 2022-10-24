/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     cgtool.h

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

#ifndef NNSG3D_CGTOOL_H_
#define NNSG3D_CGTOOL_H_

#include <nnsys/g3d/config.h>
#include <nnsys/g3d/anm.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    NNSG3dGetJointScale

    Pointer to the function that performs the scale calculation of joints.
    Keep it separated because each CG tool has differences.
*---------------------------------------------------------------------------*/
typedef void (*NNSG3dGetJointScale)(NNSG3dJntAnmResult* pResult,
                                    const fx32* p,
                                    const u8* cmd,
                                    u32 srtflag);

/*---------------------------------------------------------------------------*
    NNSG3dSendJointSRT

    This configures the joint matrix in the geometry engine.
    The matrix mode must be in Position/Vector mode at the point this function is called.
    Also, the matrix to be processed must be in the current matrix.
 *---------------------------------------------------------------------------*/
typedef void (*NNSG3dSendJointSRT)(const NNSG3dJntAnmResult*);


/*---------------------------------------------------------------------------*
    NNSG3dSendTexSRT

    This function configures the texture SRT matrix in the geometry engine.
    After execution, the matrix mode becomes Position/Vector mode.
 *---------------------------------------------------------------------------*/
typedef void (*NNSG3dSendTexSRT)(const NNSG3dMatAnmResult*);


/*---------------------------------------------------------------------------*
    NNS_G3dSendJointSRT_FuncArray

    The function pointer array corresponding to the value of NNSG3dScalingRule(<model_info>::scaling_rule).
    When sending a joint matrix, access from G3D must occur via this function pointer vector.
    
 *---------------------------------------------------------------------------*/
extern NNSG3dGetJointScale NNS_G3dGetJointScale_FuncArray[NNS_G3D_FUNC_SENDJOINTSRT_MAX];
extern NNSG3dSendJointSRT NNS_G3dSendJointSRT_FuncArray[NNS_G3D_FUNC_SENDJOINTSRT_MAX];


/*---------------------------------------------------------------------------*
    NNS_G3dSendTexSRT_FuncArray

    The function pointer array corresponding to the value of NNSG3dTexMtxMode(<model_info>::tex_matrix_mode).
    When sending a texture matrix, access from G3D must occur via this function pointer vector.
    
 *---------------------------------------------------------------------------*/
extern NNSG3dSendTexSRT   NNS_G3dSendTexSRT_FuncArray[NNS_G3D_FUNC_SENDTEXSRT_MAX];


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif
