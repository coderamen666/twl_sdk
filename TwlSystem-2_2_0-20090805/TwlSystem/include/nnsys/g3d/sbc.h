/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     sbc.h

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

#ifndef NNSG3D_SBC_H_
#define NNSG3D_SBC_H_

#include <nnsys/g3d/config.h>
#include <nnsys/g3d/binres/res_struct.h>
#include <nnsys/g3d/kernel.h>
#include <nnsys/g3d/anm.h>
#include <nnsys/g3d/cgtool.h>

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Definitions of structures and typedefs
//

/*---------------------------------------------------------------------------*
    NNSG3dRSFlag

    A collection of 1-bit flags inside NNSG3dRS.
    They are set and referenced inside functions such as NNS_G3dDraw and the SBC command function.

    If NNS_G3D_RSFLAG_SKIP is set with a callback, all actions other than callback determinations will be skipped until the end of the SBC command.
    
 *---------------------------------------------------------------------------*/
typedef enum
{
    NNS_G3D_RSFLAG_NODE_VISIBLE           = 0x00000001,    // Turned on and off by the NODE command
    NNS_G3D_RSFLAG_MAT_TRANSPARENT        = 0x00000002,    // Turned on and off by the MAT command
    NNS_G3D_RSFLAG_CURRENT_NODE_VALID     = 0x00000004,    // Turned on by the NODE command
    NNS_G3D_RSFLAG_CURRENT_MAT_VALID      = 0x00000008,    // Turned on by the MAT command
    NNS_G3D_RSFLAG_CURRENT_NODEDESC_VALID = 0x00000010,
    NNS_G3D_RSFLAG_RETURN                 = 0x00000020,    // Turned on by the RET command
    NNS_G3D_RSFLAG_SKIP                   = 0x00000040,    // User flag for callbacks (skips to the next callback in the command).

    // Below are the pre-execution values determined by the NNSG3dRenderObj
    NNS_G3D_RSFLAG_OPT_RECORD             = 0x00000080,    // Stores calculation results in NNSG3dRenderObj > recJntAnm, recMatAnm.
    NNS_G3D_RSFLAG_OPT_NOGECMD            = 0x00000100,    // Does not send geometry commands.
    NNS_G3D_RSFLAG_OPT_SKIP_SBCDRAW       = 0x00000200,    // Skips Draw type SBC commands. Also skips callbacks.
    NNS_G3D_RSFLAG_OPT_SKIP_SBCMTXCALC    = 0x00000400     // Skips MtxCalc type SBC commands. Also skips callbacks.

    // A list of the Draw type SBC commands
    // NODE         --- NNSi_G3dFuncSbc_NODE
    // MTX          --- NNSi_G3dFuncSbc_MTX
    // MAT          --- NNSi_G3dFuncSbc_MAT
    // SHP          --- NNSi_G3dFuncSbc_SHP
    // NODEDESC_BB  --- NNSi_G3dFuncSbc_BB
    // NODEDESC_BBY --- NNSi_G3dFuncSbc_BBY
    // POSSCALE     --- NNSi_G3dFuncSbc_POSSCALE

    // A list of the MtxCalc type SBC commands
    // NODEDESC     --- NNSi_G3dFuncSbc_NODEDESC
}
NNSG3dRSFlag;


/*---------------------------------------------------------------------------*
    NNSG3dRS

    This is used in storing the state of the rendering engine.
    Note that it will be allocated to the stack because normally DTCM is used. Stores the variables, etc. that perform the exchange during rendering.
    A pointer is stored in NNS_G3dRS.
 *---------------------------------------------------------------------------*/
typedef struct NNSG3dRS_
{
    // Base information
    u8*                               c;                   // Pointer to the currently referenced SBC command.
    NNSG3dRenderObj*                  pRenderObj;
    u32                               flag;                // NNSG3dRSFlag 

    // Callback function vector
    NNSG3dSbcCallBackFunc             cbVecFunc[NNS_G3D_SBC_COMMAND_NUM];   // Callback function vector
    u8                                cbVecTiming[NNS_G3D_SBC_COMMAND_NUM]; // NNSG3dSbcCallBackTiming vector

    // When the NNS_G3D_RSFLAG_CURRENT_NODE_VALID flag is ON, the nodeID for the immediately previously called (and current running) NODE command is stored.
    // 
    u8                                currentNode;

    // When the NNS_G3D_RSFLAG_CURRENT_MAT_VALID flag is ON, the material ID for the immediately previously called (and current running) MAT command is stored.
    // 
    // When the material ID specified by the MAT command is the same, the NNS_G3D_RSFLAG_CURRENT_MAT_VALID flag has to be turned OFF when changing material data, as with a callback, because the sending of material information is omitted.
    // 
    // 
    u8                                currentMat;

    // When the NNS_G3D_RSFLAG_CURRENT_NODEDESC_VALID flag is ON, the node ID for the immediately previously called (and current running) NODEDESC command is stored.
    // 
    u8                                currentNodeDesc;

    u8                                dummy_;
    // When NNSG3dMatAnmResult, NNSG3dJntAnmResult, and NNSG3dVisAnmResult are being calculated, a pointer is stored to these members.
    // 
    // The calculation results can be changed in callbacks.

    // NULL until the first MAT command is called
    // After that, stores the calculation result of MAT command executed immediately before.
    NNSG3dMatAnmResult*               pMatAnmResult;

    // NULL until the first NODEDESC command is called.
    // After that, stores the calculation result of NODEDESC command executed immediately before.
    NNSG3dJntAnmResult*               pJntAnmResult;

    // NULL until the first NODE command is called.
    // After that, stores the calculation result of NODE command executed immediately before
    NNSG3dVisAnmResult*               pVisAnmResult;

    // The bit will be set if material data is already cached.
    // The body of the data will be stored in matCache in NNSG3dRSOnHeap.
    u32                               isMatCached[NNS_G3D_SIZE_MAT_MAX / 32];

    // When storing scale-related data, if the data is (1.0, 1.0, 1.0), instead of being stored, you can instead simply turn the corresponding bits on.
    // 
    u32                               isScaleCacheOne[NNS_G3D_SIZE_JNT_MAX / 32];

#if (NNS_G3D_USE_EVPCACHE)
    u32                               isEvpCached[NNS_G3D_SIZE_JNT_MAX / 32];
#endif

    // Caches information that is repeatedly used in resMdl.
    // The user does not need to touch this.
    const NNSG3dResNodeInfo*          pResNodeInfo;
    const NNSG3dResMat*               pResMat;
    const NNSG3dResShp*               pResShp;
    fx32                              posScale;
    fx32                              invPosScale;
    NNSG3dGetJointScale               funcJntScale;
    NNSG3dSendJointSRT                funcJntMtx;
    NNSG3dSendTexSRT                  funcTexMtx;

    NNSG3dMatAnmResult                tmpMatAnmResult;
    NNSG3dJntAnmResult                tmpJntAnmResult;
    NNSG3dVisAnmResult                tmpVisAnmResult;
}
NNSG3dRS;


/*---------------------------------------------------------------------------*
    NNSG3dRSOnGlb

    Among the states of the rendering engine, this stores those which are not suitable to be put in the stack due to their large size.
     It uses the global variable NNS_G3dRSOnGlb.
    It remains in the state as allocated in the main memory from start to finish.
 *---------------------------------------------------------------------------*/
typedef struct NNSG3dRSOnGlb_
{
    //
    // The material information cached by the SBC MAT command.
    // The data is valid if the bit corresponding to the isMatChached of NNSG3dRS is on.
    //
    struct NNSG3dMatAnmResult_ matCache[NNS_G3D_SIZE_MAT_MAX];

    //
    // The scale-related data corresponding to joints exists for storing data when Maya uses Segment Scale Compensate or SI3D uses Classic Scale Off.
    // 
    // It is written by the NODEDESC command of the SBC.
    // For Maya SSC:
    // For the NODEDESC flag argument, when NNS_G3D_SBC_NODEDESC_FLAG_MAYASSC_PARENT is ON, the scaling inverse is used.
    // 
    //
    // For SI3D Classic Scale Off:
    // When NNS_G3D_SCALINGRULE_SI3D is specified, the scaling exponent is stored up to the parent node.
    // 
    // 
    // Regardless, values are only stored when the bit corresponding to isScaleCacheOne in NNSG3dRS is OFF (i.e., when scaling is not 1.0).
    // 
    //
    struct
    {
        VecFx32 s;
        VecFx32 inv;
    }
    scaleCache[NNS_G3D_SIZE_JNT_MAX];

#if (NNS_G3D_USE_EVPCACHE)
    struct
    {
        MtxFx44 M;
        MtxFx33 N;
    }
    evpCache[NNS_G3D_SIZE_JNT_MAX];
#endif
}
NNSG3dRSOnGlb;


/*---------------------------------------------------------------------------*
    NNSG3dFuncSbc

    The pointer format of the SBC command functions
 *---------------------------------------------------------------------------*/
typedef void (*NNSG3dFuncSbc)(NNSG3dRS*, u32);


/*---------------------------------------------------------------------------*
    NNSG3dFuncSbc_[Mat|Shp]Internal

    The pointer format of the functions inside the MAT/SHP commands of the SBC.
 *---------------------------------------------------------------------------*/
typedef void (*NNSG3dFuncSbc_MatInternal)(NNSG3dRS*, u32, const NNSG3dResMatData*, u32);
typedef void (*NNSG3dFuncSbc_ShpInternal)(NNSG3dRS*, u32, const NNSG3dResShpData*, u32);


/*---------------------------------------------------------------------------*
    Matrix stack index definition not used in the code generated by g3dcvtr.
    NNS_G3D_MTXSTACK_SYS is reserved internally by G3D; NNS_G3D_MTXSTACK_USER can be used by the user.
    
 *---------------------------------------------------------------------------*/

#define NNS_G3D_MTXSTACK_SYS  (30)
#define NNS_G3D_MTXSTACK_USER (29)


////////////////////////////////////////////////////////////////////////////////
//
// Function Declarations
//

//
// Accessor for NNS_G3dRS
//
NNS_G3D_INLINE void NNS_G3dRSSetCallBack(NNSG3dRS* rs, NNSG3dSbcCallBackFunc func, u8 cmd, NNSG3dSbcCallBackTiming timing);
NNS_G3D_INLINE void NNS_G3dRSResetCallBack(NNSG3dRS* rs, u8 cmd);
NNS_G3D_INLINE NNSG3dRenderObj* NNS_G3dRSGetRenderObj(NNSG3dRS* rs);
NNS_G3D_INLINE NNSG3dMatAnmResult* NNS_G3dRSGetMatAnmResult(NNSG3dRS* rs);
NNS_G3D_INLINE NNSG3dJntAnmResult* NNS_G3dRSGetJntAnmResult(NNSG3dRS* rs);
NNS_G3D_INLINE NNSG3dVisAnmResult* NNS_G3dRSGetVisAnmResult(NNSG3dRS* rs);
NNS_G3D_INLINE u8* NNS_G3dRSGetSbcPtr(NNSG3dRS* rs);
NNS_G3D_INLINE void NNS_G3dRSSetFlag(NNSG3dRS* rs, NNSG3dRSFlag flag);
NNS_G3D_INLINE void NNS_G3dRSResetFlag(NNSG3dRS* rs, NNSG3dRSFlag flag);
NNS_G3D_INLINE int NNS_G3dRSGetCurrentMatID(const NNSG3dRS* rs);
NNS_G3D_INLINE int NNS_G3dRSGetCurrentNodeID(const NNSG3dRS* rs);
NNS_G3D_INLINE int NNS_G3dRSGetCurrentNodeDescID(const NNSG3dRS* rs);
NNS_G3D_INLINE fx32 NNS_G3dRSGetPosScale(const NNSG3dRS* rs);
NNS_G3D_INLINE fx32 NNS_G3dRSGetInvPosScale(const NNSG3dRS* rs);



//
// Render
//
void NNS_G3dDraw(struct NNSG3dRenderObj_* pRenderObj);

//
// SBC instructions
//
void NNSi_G3dFuncSbc_NOP(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_RET(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_NODE(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_MTX(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_MAT(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_SHP(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_NODEDESC(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_BB(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_BBY(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_NODEMIX(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_CALLDL(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_POSSCALE(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_ENVMAP(NNSG3dRS*, u32);
void NNSi_G3dFuncSbc_PRJMAP(NNSG3dRS*, u32);


// Operation when MAT/SHP itemTag is set to 0.
void NNSi_G3dFuncSbc_SHP_InternalDefault(NNSG3dRS* rs,
                                        u32 opt,
                                        const NNSG3dResShpData* shp,
                                        u32 idxShp);

void NNSi_G3dFuncSbc_MAT_InternalDefault(NNSG3dRS* rs,
                                        u32 opt,
                                        const NNSG3dResMatData* mat,
                                        u32 idxMat);

//
// Checking callbacks 
//
NNS_G3D_INLINE BOOL NNSi_G3dCallBackCheck_A(NNSG3dRS* rs, u8 cmd, NNSG3dSbcCallBackTiming* pTiming);
NNS_G3D_INLINE BOOL NNSi_G3dCallBackCheck_B(NNSG3dRS* rs, u8 cmd, NNSG3dSbcCallBackTiming* pTiming);
NNS_G3D_INLINE BOOL NNSi_G3dCallBackCheck_C(NNSG3dRS* rs, u8 cmd, NNSG3dSbcCallBackTiming timing);
NNS_G3D_INLINE NNSG3dSbcCallBackTiming NNSi_CheckPossibilityOfCallBack(NNSG3dRS* rs, u8 cmd);

////////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
extern NNSG3dFuncSbc NNS_G3dFuncSbcTable[NNS_G3D_SBC_COMMAND_NUM];
extern NNSG3dFuncSbc_MatInternal NNS_G3dFuncSbcMatTable[NNS_G3D_SIZE_MAT_VTBL_NUM];
extern NNSG3dFuncSbc_ShpInternal NNS_G3dFuncSbcShpTable[NNS_G3D_SIZE_SHP_VTBL_NUM];
extern NNSG3dRS* NNS_G3dRS;
extern NNSG3dRSOnGlb NNS_G3dRSOnGlb;



#ifdef __cplusplus
}/* extern "C" */
#endif

#include <nnsys/g3d/sbc_inline.h>

#endif
