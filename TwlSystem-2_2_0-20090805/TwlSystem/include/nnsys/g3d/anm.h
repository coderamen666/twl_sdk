/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     anm.h

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

#ifndef NNSG3D_ANM_H_
#define NNSG3D_ANM_H_

#include <nnsys/g3d/config.h>
#include <nnsys/g3d/binres/res_struct.h>
#include <nnsys/g3d/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
    Summary of the animation operations:

    When animating specific to matID and nodeID values, the blend function registered to NNSG3dRenderObj is called from the SBC interpreter.
    
    Results (the content of which BlendMat is responsible for) are stored in NNSG3d[Mat|Jnt]AnmResult*. NNSG3dAnmObj* is the data member of NNSG3dRenderObj and is the list of animation objects.
    
    
 *---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
//
// Definitions of structures and typedefs
//


struct NNSG3dResMdl_;
/*---------------------------------------------------------------------------*
    NNSG3dAnimInitFunc

    The type of function that initializes the NNSG3dAnmObj. void* is the pointer to the animation resource.
    This is because depending on the kind of animation resource, the initialization method ofNNSG3dAnmObj will differ.
 *---------------------------------------------------------------------------*/
typedef void (*NNSG3dAnimInitFunc)(NNSG3dAnmObj*,
                                   void*,
                                   const NNSG3dResMdl*);






/*---------------------------------------------------------------------------*
    NNSG3dAnmObjInitFunc

    category0 and category1 are the same as those of NNSG3dResAnmHeader.
 *---------------------------------------------------------------------------*/
typedef struct
{
    u8                  category0;
    u8                  dummy_;
    u16                 category1;
    NNSG3dAnimInitFunc  func;
}
NNSG3dAnmObjInitFunc;


/*---------------------------------------------------------------------------*
    About the material animation operations

    The values for the material animation calculations are continuously overwritten in NNSG3dMatAnmResult. Ultimately the data is sent to the geometry engine.
    
 *---------------------------------------------------------------------------*/
typedef enum
{
    NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SCALEONE  = 0x00000001,
    NNS_G3D_MATANM_RESULTFLAG_TEXMTX_ROTZERO   = 0x00000002,
    NNS_G3D_MATANM_RESULTFLAG_TEXMTX_TRANSZERO = 0x00000004,

    NNS_G3D_MATANM_RESULTFLAG_TEXMTX_SET       = 0x00000008,
    NNS_G3D_MATANM_RESULTFLAG_TEXMTX_MULT      = 0x00000010,
    NNS_G3D_MATANM_RESULTFLAG_WIREFRAME        = 0x00000020
}
NNSG3dMatAnmResultFlag;


/*---------------------------------------------------------------------------*
    NNSG3dMatAnmResult

    This is a structure for storing the calculated material information.
    The calculation is done inside the MAT command of SBC.
 *---------------------------------------------------------------------------*/
typedef struct NNSG3dMatAnmResult_
{
    NNSG3dMatAnmResultFlag flag;
    u32                    prmMatColor0;
    u32                    prmMatColor1;
    u32                    prmPolygonAttr;
    u32                    prmTexImage;
    u32                    prmTexPltt;

    // When the flag is SCALEONE, ROTZERO or TRANSZERO, corresponding values are not set.
    // 
    fx32                   scaleS, scaleT;
    fx16                   sinR, cosR;
    fx32                   transS, transT;

    u16                    origWidth, origHeight;
    fx32                   magW, magH;
}
NNSG3dMatAnmResult;


/*---------------------------------------------------------------------------*
    About the joint animation operations

    Quarternions are not supported.
 *---------------------------------------------------------------------------*/
typedef enum
{
    NNS_G3D_JNTANM_RESULTFLAG_SCALE_ONE      = 0x00000001,
    NNS_G3D_JNTANM_RESULTFLAG_ROT_ZERO       = 0x00000002,
    NNS_G3D_JNTANM_RESULTFLAG_TRANS_ZERO     = 0x00000004,
    NNS_G3D_JNTANM_RESULTFLAG_SCALEEX0_ONE   = 0x00000008,
    NNS_G3D_JNTANM_RESULTFLAG_SCALEEX1_ONE   = 0x00000010,
    NNS_G3D_JNTANM_RESULTFLAG_MAYA_SSC       = 0x00000020
//    NNS_G3D_JNTANM_RESULTFLAG_ROT_QUATERNION = 0x00000040
}
NNSG3dJntAnmResultFlag;


/*---------------------------------------------------------------------------*
    NNSG3dJntAnmResult

    This is a structure for storing the calculated node information.
    The calculation is done inside the NODEDESC command of SBC.
    scaleEx0 and scaleEx1 are used to store additional scale information when SSC is set in Maya and Classic Scale off is set in Si3d.
    
 *---------------------------------------------------------------------------*/
typedef struct NNSG3dJntAnmResult_
{
    NNSG3dJntAnmResultFlag flag;
    VecFx32                scale;
    VecFx32                scaleEx0;
    VecFx32                scaleEx1;
    MtxFx33                rot;
    VecFx32                trans;
}
NNSG3dJntAnmResult;


/*---------------------------------------------------------------------------*
    About the visibility animation operations

    These operations make the joint entirely visible or invisible.
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    NNSG3dVisAnmResult

    This is a structure for storing the calculated visibility information.
    The calculation is done inside the NODE command of SBC.
 *---------------------------------------------------------------------------*/
typedef struct NNSG3dVisAnmResult_
{
    BOOL isVisible;
}
NNSG3dVisAnmResult;


// Material animation calculation function
typedef void (*NNSG3dFuncAnmMat)(NNSG3dMatAnmResult*,
                                 const NNSG3dAnmObj*,
                                 u32);

// Joint animation calculation function
typedef void (*NNSG3dFuncAnmJnt)(NNSG3dJntAnmResult*,
                                 const NNSG3dAnmObj*,
                                 u32);

// Visibility animation calculation function
typedef void (*NNSG3dFuncAnmVis)(NNSG3dVisAnmResult*,
                                 const NNSG3dAnmObj*,
                                 u32);


////////////////////////////////////////////////////////////////////////////////
//
// Function Declarations
//

//
// Material animation blend function defaults
// In short, the animation results that were calculated later have priority.
//
BOOL NNSi_G3dAnmBlendMat(NNSG3dMatAnmResult* pResult,
                         const NNSG3dAnmObj* pAnmObj,
                         u32 matID);

BOOL NNSi_G3dAnmBlendJnt(NNSG3dJntAnmResult*,
                         const NNSG3dAnmObj*,
                         u32);

BOOL NNSi_G3dAnmBlendVis(NNSG3dVisAnmResult*,
                         const NNSG3dAnmObj*,
                         u32);

////////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//

//
// The pointer to the default animation blend function
// This is used to initially configure the NNSG3dRenderObj in NNS_G3dRenderObjInit.
//
extern NNSG3dFuncAnmBlendMat NNS_G3dFuncBlendMatDefault;
extern NNSG3dFuncAnmBlendJnt NNS_G3dFuncBlendJntDefault;
extern NNSG3dFuncAnmBlendVis NNS_G3dFuncBlendVisDefault;

//
// The pointer to the default animation calculation function
// This is used to initially configure the NNSG3dAnmObj in NNS_G3dAnmObjInit.
//
extern NNSG3dFuncAnmMat NNS_G3dFuncAnmMatNsBmaDefault;
extern NNSG3dFuncAnmMat NNS_G3dFuncAnmMatNsBtpDefault;
extern NNSG3dFuncAnmMat NNS_G3dFuncAnmMatNsBtaDefault;
extern NNSG3dFuncAnmJnt NNS_G3dFuncAnmJntNsBcaDefault;
extern NNSG3dFuncAnmVis NNS_G3dFuncAnmVisNsBvaDefault;

extern u32 NNS_G3dAnmFmtNum;
extern NNSG3dAnmObjInitFunc NNS_G3dAnmObjInitFuncArray[NNS_G3D_ANMFMT_MAX];


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif
