/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     kernel.h

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

#ifndef NNSG3D_KERNEL_H_
#define NNSG3D_KERNEL_H_

#include <nnsys/g3d/config.h>
#include <nnsys/g3d/binres/res_struct.h>
#include <nnsys/g3d/binres/res_struct_accessor_anm.h>

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Definitions of structures and typedefs
//

#define NNS_G3D_SIZE_MAT_MAX_MAX 256
#define NNS_G3D_SIZE_JNT_MAX_MAX 256
#define NNS_G3D_SIZE_SHP_MAX_MAX 256

#if (NNS_G3D_SIZE_MAT_MAX <= 0 || NNS_G3D_SIZE_MAT_MAX > NNS_G3D_SIZE_MAT_MAX_MAX)
#error NNS_G3D_SIZE_MAT_MAX range error.
#endif

#if (NNS_G3D_SIZE_JNT_MAX <= 0 || NNS_G3D_SIZE_JNT_MAX > NNS_G3D_SIZE_JNT_MAX_MAX)
#error NNS_G3D_SIZE_JNT_MAX range error.
#endif

#if (NNS_G3D_SIZE_SHP_MAX <= 0 || NNS_G3D_SIZE_SHP_MAX > NNS_G3D_SIZE_SHP_MAX_MAX)
#error NNS_G3D_SIZE_SHP_MAX range error.
#endif

#if (NNS_G3D_SIZE_MAT_MAX % 32 != 0)
#error NNS_G3D_SIZE_MAT_MAX must be a multiple of 32.
#endif

#if (NNS_G3D_SIZE_JNT_MAX % 32 != 0)
#error NNS_G3D_SIZE_JNT_MAX must be a multiple of 32.
#endif

#if (NNS_G3D_SIZE_SHP_MAX % 32 != 0)
#error NNS_G3D_SIZE_SHP_MAX must be a multiple of 32.
#endif

typedef u32 NNSG3dTexKey;    // compatible with NNSGfdTexKey
typedef u32 NNSG3dPlttKey;   // compatible with NNSGfdPlttKey


/*---------------------------------------------------------------------------*
    NNSG3dAnmObj

    The structure that is referenced by NNSG3dRenderObj. The user must allocate and deallocate memory on his or her own.
     The NNS_G3dAnmObjInit function is used to initialize.
    The purpose of this structure is to:
    - Specify pairing for animation resources and the functions that process them,
    - Associate animation resources and model resources, and
    - Maintain the current animation frame.

    frame:      Designates which frame to play
    ratio:      Used by the animation blend function
    resAnm:     The pointer to the individual animation resources
    funcAnm:    The pointer to the function that plays the frame in the resAnm frame-numbered position
    next:       Configured with the NNS_G3dRenderObjBindAnmObj and NNS_G3dRenderObjReleaseAnmObj functions.
                
    resTex:     The pointer to the texture resource.
                Configured only when animations are needed.
    priority:   The priority when NNSG3dRenderObj is registered
    numMapData: The number of table entries that take corresponding indices for model and animation resources.
                
 *---------------------------------------------------------------------------*/
typedef struct NNSG3dAnmObj_
{
    fx32                  frame;
    fx32                  ratio;
    void*                 resAnm;     // The pointer to the animation data block inside the resource  file
    void*                 funcAnm;    // Cast in the function pointer of each animation. Takes the default, but can be changed.
    struct NNSG3dAnmObj_* next;
    const NNSG3dResTex*   resTex;     // When the information about the texture  block is needed (the texture pattern animation only)
    u8                    priority;
    u8                    numMapData;
    u16                   mapData[1]; // Becomes numMapData array (NNSG3dAnmObjMapData)
}
NNSG3dAnmObj;

//
// The size of the memory needed by NNSG3dAnmObj is determined from the categories of the model resource and the animation.
// For material animation, use the macro below.
// In actuality, it is sizeof(NNSG3dAnmObj) + sizeof(u16) * (pMdl->info.numMat - 1)
// But it uses a 4 byte boundary.
//
#define NNS_G3D_ANMOBJ_SIZE_MATANM(pMdl) ((sizeof(NNSG3dAnmObj) + sizeof(u16) * pMdl->info.numMat) & ~3)

//
// The size of the memory needed by NNSG3dAnmObj is determined from the categories of the model resource and the animation.
// Use the macro below for joint animation and visibility animation.
// In actuality, it is sizeof(NNSG3dAnmObj) + sizeof(u16) * (pMdl->info.numNode - 1)
// But it uses a 4 byte boundary.
//
#define NNS_G3D_ANMOBJ_SIZE_JNTANM(pMdl) ((sizeof(NNSG3dAnmObj) + sizeof(u16) * pMdl->info.numNode) & ~3)
#define NNS_G3D_ANMOBJ_SIZE_VISANM(pMdl) ((sizeof(NNSG3dAnmObj) + sizeof(u16) * pMdl->info.numNode) & ~3)

/*---------------------------------------------------------------------------*
    NNSG3dAnmObjMapData

    Useful enumerated type for data to be stored in the mapData array NSSG3dAnmObj.
    
 *---------------------------------------------------------------------------*/
typedef enum
{
    NNS_G3D_ANMOBJ_MAPDATA_EXIST     = 0x0100,
    NNS_G3D_ANMOBJ_MAPDATA_DISABLED  = 0x0200,
    NNS_G3D_ANMOBJ_MAPDATA_DATAFIELD = 0x00ff
}
NNSG3dAnmObjMapData;


/*---------------------------------------------------------------------------*
    The animation blend function typedef
 *---------------------------------------------------------------------------*/
// Define with anm.h
struct NNSG3dMatAnmResult_;
struct NNSG3dJntAnmResult_;
struct NNSG3dVisAnmResult_;


// material animation blend function
typedef BOOL (*NNSG3dFuncAnmBlendMat)(struct NNSG3dMatAnmResult_*,
                                      const NNSG3dAnmObj*,
                                      u32);

// joint animation blend function
typedef BOOL (*NNSG3dFuncAnmBlendJnt)(struct NNSG3dJntAnmResult_*,
                                      const NNSG3dAnmObj*,
                                      u32);

// visibility animation blend function
typedef BOOL (*NNSG3dFuncAnmBlendVis)(struct NNSG3dVisAnmResult_*,
                                      const NNSG3dAnmObj*,
                                      u32);


/*---------------------------------------------------------------------------*
    NNSG3dRenderObjFlag

    These are the flags held by NNSG3dRenderObj. They can customize the operations of NNS_G3dDraw.
    
    NNS_G3D_RENDEROBJ_FLAG_RECORD
        When the NNS_G3dDraw is run, stores the calculation results for joints and materials in recJntAnm and recMatAnm. 
        After execution finishes, these flags are reset.
        When this flag has been reset and recJntAnm and recMatAnm are not null, the calculation results within recJntAnm and recMatAnm are used as is.
        
    NNS_G3D_RENDEROBJ_FLAG_NOGECMD
        This does not send geometry commands when NNS_G3dDraw is executed.
    NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_DRAW
        When NNS_G3dDraw is executed, this skips the execution of SBC rendering-related commands.
    NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_MTXCALC
        When NNS_G3dDraw is executed, this skips the execution of SBC matrix calculation-related commands.

    NNS_G3D_RENDEROBJ_FLAG_HINT_OBSOLETE
        When the flags that set and reset internally for G3D are not in a correct state for hintXXXAnmExist, they are set (set when the NNS_G3dRenderObjRemoveAnmObj function is run).
        

    NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_DRAW and NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_MTXCALC are enabled for models that have been converted with the -s option specified to g3dcvtr.
    
 *---------------------------------------------------------------------------*/
typedef enum
{
    NNS_G3D_RENDEROBJ_FLAG_RECORD             = 0x00000001,
    NNS_G3D_RENDEROBJ_FLAG_NOGECMD            = 0x00000002,
    NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_DRAW      = 0x00000004,
    NNS_G3D_RENDEROBJ_FLAG_SKIP_SBC_MTXCALC   = 0x00000008,
    NNS_G3D_RENDEROBJ_FLAG_HINT_OBSOLETE      = 0x00000010
}
NNSG3dRenderObjFlag;


/*---------------------------------------------------------------------------*
    NNSG3dSbcCallBackFunc

    The pointer to the callback function stored inside NNSG3dRS
 *---------------------------------------------------------------------------*/
struct NNSG3dRS_;
typedef void (*NNSG3dSbcCallBackFunc)(struct NNSG3dRS_*);


/*---------------------------------------------------------------------------*
    NNSG3dSbcCallBackTiming

    This can designate three kinds of timing for the callback that starts up in the SBC command.
 *---------------------------------------------------------------------------*/
typedef enum
{
    NNS_G3D_SBC_CALLBACK_TIMING_NONE = 0x00,
    NNS_G3D_SBC_CALLBACK_TIMING_A    = 0x01,
    NNS_G3D_SBC_CALLBACK_TIMING_B    = 0x02,
    NNS_G3D_SBC_CALLBACK_TIMING_C    = 0x03
}
NNSG3dSbcCallBackTiming;

#define NNS_G3D_SBC_CALLBACK_TIMING_ASSERT(x)               \
    NNS_G3D_ASSERT(x == NNS_G3D_SBC_CALLBACK_TIMING_NONE || \
                   x == NNS_G3D_SBC_CALLBACK_TIMING_A    || \
                   x == NNS_G3D_SBC_CALLBACK_TIMING_B    || \
                   x == NNS_G3D_SBC_CALLBACK_TIMING_C)

/*---------------------------------------------------------------------------*
    NNSG3dRenderObj

    The members and other features of the structure are always subject to change
 *---------------------------------------------------------------------------*/
typedef struct NNSG3dRenderObj_
{
    u32 flag; // NNSG3dRenderObjFlag

    // NOTICE:
    // The contents of NNSG3dResMdl are not rewritten inside NNS_G3dDraw.
    // (With the exception of when the callback was used, etc.)
    NNSG3dResMdl*         resMdl;
    NNSG3dAnmObj*         anmMat;
    NNSG3dFuncAnmBlendMat funcBlendMat;
    NNSG3dAnmObj*         anmJnt;
    NNSG3dFuncAnmBlendJnt funcBlendJnt;
    NNSG3dAnmObj*         anmVis;
    NNSG3dFuncAnmBlendVis funcBlendVis;

    // Information for the callback
    NNSG3dSbcCallBackFunc cbFunc;              // There is no callback if NULL
    u8                    cbCmd;               // Designate the stopping position with the command. NNS_G3D_SBC_***** (defined in res_struct.h)
    u8                    cbTiming;            // NNSG3dSbcCallBackTiming (defined in sbc.h)
    u16                   dummy_;

    // Called immediately before starting rendering.
    // Generally used to configure the NNS_G3dRS function's callback vector.
    NNSG3dSbcCallBackFunc cbInitFunc;

    // The pointer to the region that the user manages.
    // This can be used with the callback if the pointer is configured in advance.
    void*                 ptrUser;

    //
    // By default, the SBC stored within resMdl is used, but a user-defined SBC can be used by storing a pointer in ptrUserSbc.
    // 
    // 
    //
    // Example:
    // When, for example, in a particle system, you want to use a simple model (1 material, 1 shape) in various locations, it is efficient to create the following type of SBC code and then insert a pointer to that code in ptrUserSbc.
    // 
    // 
    // 
    // MAT[000] 0
    // MTX 0
    // SHP 0
    // MTX 1
    // SHP 0
    // ....
    // MTX n
    // SHP 0
    // RET
    //
    // The matrix must be set into the matrix stack in advance.
    // It is probably best to use a callback to make changes when wanting to slightly alter the material on a shape-by-shape basis.
    // 
    //
    u8*                   ptrUserSbc;

    //
    // The pointer to the buffering region of the calculation result.
    // When you want to use joint and material calculation results in multiple frames and/or models, set a buffer in recJntAnm and recMatAnm.
    // 
    //
    // When the NNS_G3D_RENDEROBJ_FLAG_RECORD flag is ON, and when recJntAnm and recMatAnm are not NULL, their respective material and joint calculations are recorded.
    // 
    // 
    //
    // When the NNS_G3D_RENDEROBJ_FLAG_RECORD flag is OFF, and when recJntAnm and recMatAnm are not NULL, their respective material and joint calculations are used.
    // 
    // 
    //
    // The user must allocate the following regions.
    // For recJntAnm:
    // sizeof(NNSG3dJntAnmResult) * resMdl->info.numNode bytes
    // For recMatAnm:
    // sizeof(NNSG3dMatAnmResult) * resMdl->info.numMat bytes
    //  
    //
    struct NNSG3dJntAnmResult_*   recJntAnm;
    struct NNSG3dMatAnmResult_*   recMatAnm;

    //
    // When animation has been added and there are definitions for matID and nodeID, the bit becomes 1.
    //  Each ID can be a maximum of 256 units large, so they can be managed by 8 words apiece.
    // However, even when the animation is deleted, the bit remains 1.
    // The SBC interpreter checks this field and determines whether to call NNSG3dFuncBlendMatXXX.
    // 
    // You should note that when the bit is 0, all that is indicated is that there is no animation related to that matID or nodeID.
    // 
    //
    u32 hintMatAnmExist[NNS_G3D_SIZE_MAT_MAX / 32];
    u32 hintJntAnmExist[NNS_G3D_SIZE_JNT_MAX / 32];
    u32 hintVisAnmExist[NNS_G3D_SIZE_JNT_MAX / 32];
}
NNSG3dRenderObj;

// Buffer size calculation macro used when buffering the NNSG3dJntAnmResult calculation results.
// 
#define NNS_G3D_RENDEROBJ_JNTBUFFER_SIZE(numJnt) \
    ((u32)(sizeof(NNSG3dJntAnmResult) * (numJnt)))

// Buffer size calculation macro used when buffering the NNSG3dMatAnmResult calculation results.
// 
#define NNS_G3D_RENDEROBJ_MATBUFFER_SIZE(numMat) \
    ((u32)(sizeof(NNSG3dMatAnmResult) * (numMat)))





////////////////////////////////////////////////////////////////////////////////
//
// Function Declarations
//

//
// inline functions for NNSG3dAnmObj
//
NNS_G3D_INLINE void NNS_G3dAnmObjSetFrame(NNSG3dAnmObj* pAnmObj, fx32 frame);
NNS_G3D_INLINE void NNS_G3dAnmObjSetBlendRatio(NNSG3dAnmObj* pAnmObj, fx32 ratio);
NNS_G3D_INLINE fx32 NNS_G3dAnmObjGetNumFrame(const NNSG3dAnmObj* pAnmObj);


//
// non-inline functions for NNSG3dAnmObj
//
u32 NNS_G3dAnmObjCalcSizeRequired(const void* pAnm, const NNSG3dResMdl* pMdl);
void NNS_G3dAnmObjInit(NNSG3dAnmObj* pAnmObj,
                       void* pResAnm,
                       const NNSG3dResMdl* pResMdl,
                       const NNSG3dResTex* pResTex);
void NNS_G3dAnmObjEnableID(NNSG3dAnmObj* pAnmObj, int id);
void NNS_G3dAnmObjDisableID(NNSG3dAnmObj* pAnmObj, int id);


//
// inline functions for NNSG3dRenderObj
//
NNS_G3D_INLINE void NNS_G3dRenderObjSetFlag(NNSG3dRenderObj* pRenderObj, NNSG3dRenderObjFlag flag);
NNS_G3D_INLINE void NNS_G3dRenderObjResetFlag(NNSG3dRenderObj* pRenderObj, NNSG3dRenderObjFlag flag);
NNS_G3D_INLINE BOOL NNS_G3dRenderObjTestFlag(const NNSG3dRenderObj* pRenderObj, NNSG3dRenderObjFlag flag);
NNS_G3D_INLINE u8* NNS_G3dRenderObjSetUserSbc(NNSG3dRenderObj* pRenderObj, u8* sbc);
NNS_G3D_INLINE void NNS_G3dRenderObjSetJntAnmBuffer(NNSG3dRenderObj* pRenderObj, struct NNSG3dJntAnmResult_* buf);
NNS_G3D_INLINE void NNS_G3dRenderObjSetMatAnmBuffer(NNSG3dRenderObj* pRenderObj, struct NNSG3dMatAnmResult_* buf);

NNS_G3D_INLINE struct NNSG3dJntAnmResult_* NNS_G3dRenderObjReleaseJntAnmBuffer(NNSG3dRenderObj* pRenderObj);
NNS_G3D_INLINE struct NNSG3dMatAnmResult_* NNS_G3dRenderObjReleaseMatAnmBuffer(NNSG3dRenderObj* pRenderObj);
NNS_G3D_INLINE void* NNS_G3dRenderObjSetUserPtr(NNSG3dRenderObj* pRenderObj, void* ptr);

NNS_G3D_INLINE NNSG3dResMdl* NNS_G3dRenderObjGetResMdl(NNSG3dRenderObj* pRenderObj);
NNS_G3D_INLINE void NNS_G3dRenderObjSetBlendFuncMat(NNSG3dRenderObj* pRenderObj, NNSG3dFuncAnmBlendMat func);
NNS_G3D_INLINE void NNS_G3dRenderObjSetBlendFuncJnt(NNSG3dRenderObj* pRenderObj, NNSG3dFuncAnmBlendJnt func);
NNS_G3D_INLINE void NNS_G3dRenderObjSetBlendFuncVis(NNSG3dRenderObj* pRenderObj, NNSG3dFuncAnmBlendVis func);


//
// non-inline functions for NNSG3dRenderObj
//
void NNS_G3dRenderObjInit(NNSG3dRenderObj* pRenderObj, NNSG3dResMdl* pResMdl);
void NNS_G3dRenderObjAddAnmObj(NNSG3dRenderObj* pRenderObj, NNSG3dAnmObj* pAnmObj);
void NNS_G3dRenderObjRemoveAnmObj(NNSG3dRenderObj* pRenderObj, NNSG3dAnmObj* pAnmObj);
void NNS_G3dRenderObjSetCallBack(NNSG3dRenderObj* pRenderObj,
                                 NNSG3dSbcCallBackFunc func,
                                 u8*,
                                 u8 cmd,
                                 NNSG3dSbcCallBackTiming timing);
void NNS_G3dRenderObjResetCallBack(NNSG3dRenderObj* pRenderObj);
void NNS_G3dRenderObjSetInitFunc(NNSG3dRenderObj* pRenderObj, NNSG3dSbcCallBackFunc func);


//
// non-inline functions for Model/Texture binding
//

// Texture Key
u32 NNS_G3dTexGetRequiredSize(const NNSG3dResTex* pTex);
u32 NNS_G3dTex4x4GetRequiredSize(const NNSG3dResTex* pTex);
void NNS_G3dTexSetTexKey(NNSG3dResTex* pTex,
                         NNSG3dTexKey texKey,
                         NNSG3dTexKey tex4x4Key);
void NNS_G3dTexLoad(NNSG3dResTex* pTex, BOOL exec_begin_end);
void NNS_G3dTexReleaseTexKey(NNSG3dResTex* pTex,
                             NNSG3dTexKey* texKey,
                             NNSG3dTexKey* tex4x4Key);

// Palette Key
u32 NNS_G3dPlttGetRequiredSize(const NNSG3dResTex* pTex);
void NNS_G3dPlttSetPlttKey(NNSG3dResTex* pTex, NNSG3dPlttKey plttKey);
void NNS_G3dPlttLoad(NNSG3dResTex* pTex, BOOL exec_begin_end);
NNSG3dPlttKey NNS_G3dPlttReleasePlttKey(NNSG3dResTex* pTex);

// Model -> Texture binding
BOOL NNS_G3dBindMdlTex(NNSG3dResMdl* pMdl, const NNSG3dResTex* pTex);
BOOL NNS_G3dBindMdlTexEx(NNSG3dResMdl* pMdl,
                         const NNSG3dResTex* pTex,
                         const NNSG3dResName* pResName);
BOOL NNS_G3dForceBindMdlTex(NNSG3dResMdl* pMdl,
                            const NNSG3dResTex* pTex,
                            u32 texToMatListIdx,
                            u32 texIdx);

void NNS_G3dReleaseMdlTex(NNSG3dResMdl* pMdl);
BOOL NNS_G3dReleaseMdlTexEx(NNSG3dResMdl* pMdl, const NNSG3dResName* pResName);

// Model -> Palette binding
BOOL NNS_G3dBindMdlPltt(NNSG3dResMdl* pMdl, const NNSG3dResTex* pTex);
BOOL NNS_G3dBindMdlPlttEx(NNSG3dResMdl* pMdl,
                          const NNSG3dResTex* pTex,
                          const NNSG3dResName* pResName);
BOOL NNS_G3dForceBindMdlPltt(NNSG3dResMdl* pMdl,
                             const NNSG3dResTex* pTex,
                             u32 plttToMatListIdx,
                             u32 plttIdx);
void NNS_G3dReleaseMdlPltt(NNSG3dResMdl* pMdl);
BOOL NNS_G3dReleaseMdlPlttEx(NNSG3dResMdl* pMdl, const NNSG3dResName* pResName);

BOOL NNS_G3dBindMdlSet(NNSG3dResMdlSet* pMdlSet, const NNSG3dResTex* pTex);
void NNS_G3dReleaseMdlSet(NNSG3dResMdlSet* pMdlSet);


//
// Misc inline functions
//

#ifdef __cplusplus
}
#endif

#include <nnsys/g3d/kernel_inline.h>

#endif
