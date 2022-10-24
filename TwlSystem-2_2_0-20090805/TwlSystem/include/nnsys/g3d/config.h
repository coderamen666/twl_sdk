/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g3d
  File:     config.h

  Copyright 2004-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

//
// AUTHOR: Kenji Nishida
//

#ifndef NNSG3D_CONFIG_H_
#define NNSG3D_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nnsys/inline.h>
#define NNS_G3D_INLINE NNS_INLINE

// The size of the pointer table to the initialization function of the animation binary
#ifndef NNS_G3D_ANMFMT_MAX
#define NNS_G3D_ANMFMT_MAX 10
#endif

// The maximum number of joint material shapes
#ifndef NNS_G3D_SIZE_JNT_MAX
#define NNS_G3D_SIZE_JNT_MAX 64
#endif

#ifndef NNS_G3D_SIZE_MAT_MAX
#define NNS_G3D_SIZE_MAT_MAX 64
#endif

#ifndef NNS_G3D_SIZE_SHP_MAX
// Set the maximum number by default because this constant is not currently used
#define NNS_G3D_SIZE_SHP_MAX 256
#endif

//
// The buffer size (in words) of the geometry command held by G3D.
// However, it is not allocated by default.
// It can be allocated with NNS_G3dGeSetBuffer.
//
#ifndef NNS_G3D_SIZE_COMBUFFER
#define NNS_G3D_SIZE_COMBUFFER 192
#endif

//
// Check for the presence of each of the callbacks if the comment is taken out.
// The code becomes invalid.
//

//#define NNS_G3D_SBC_CALLBACK_TIMING_A_DISABLE
//#define NNS_G3D_SBC_CALLBACK_TIMING_B_DISABLE
//#define NNS_G3D_SBC_CALLBACK_TIMING_C_DISABLE


//
// The codes for each of the computer graphics tools become invalid if the comments are taken out
//

//#define NNS_G3D_MAYA_DISABLE
//#define NNS_G3D_SI3D_DISABLE
//#define NNS_G3D_3DSMAX_DISABLE
//#define NNS_G3D_XSI_DISABLE

//
// The codes for each of the animations become invalid if the comment is taken out
//

//#define NNS_G3D_NSBMA_DISABLE
//#define NNS_G3D_NSBTP_DISABLE
//#define NNS_G3D_NSBTA_DISABLE
//#define NNS_G3D_NSBCA_DISABLE
//#define NNS_G3D_NSBVA_DISABLE

// The size of the dispatch table accessed by the SHP command of SBC. (This is needed because multiple kinds of MAT binary formats are supported.)
#ifndef NNS_G3D_SIZE_SHP_VTBL_NUM
#define NNS_G3D_SIZE_SHP_VTBL_NUM 4
#endif

// The size of the dispatch table accessed by the MAT command of SBC. (This is needed because multiple kinds of SHP binary formats are supported.)
#ifndef NNS_G3D_SIZE_MAT_VTBL_NUM
#define NNS_G3D_SIZE_MAT_VTBL_NUM 4
#endif

// The number of entries in the function table used to perform the joint calculations on each CG tool
#ifndef NNS_G3D_FUNC_SENDJOINTSRT_MAX
#define NNS_G3D_FUNC_SENDJOINTSRT_MAX 3
#endif

// The number of entries in the function table used to perform the texture matrix calculations on each CG tool
#ifndef NNS_G3D_FUNC_SENDTEXSRT_MAX
#define NNS_G3D_FUNC_SENDTEXSRT_MAX 4
#endif

// Removing the comment will cause MI_SendGXCommandAsyncFast to be used instead of MI_SendGXCommandAsync for display list transfers with NNS_G3dGeSendDL
// 
//#define NNS_G3D_USE_FASTGXDMA

//
// You can save memory by defining 0 if the cache is not used while using weighted envelopes
// 
//
#ifndef NNS_G3D_USE_EVPCACHE
#define NNS_G3D_USE_EVPCACHE 1
#endif


#if defined(NNS_G3D_MAYA_DISABLE) && defined(NNS_G3D_SI3D_DISABLE) && \
    defined(NNS_G3D_3DSMAX_DISABLE) && defined(NNS_G3D_XSI_DISABLE)
#error You cannot disable all of the CG tools for G3D.
#endif

#if (NNS_G3D_FUNC_SENDJOINTSRT_MAX < 3)
#error NNS_G3D_FUNC_SENDJOINTSRT_MAX must be 3 or above.
#endif

#if (NNS_G3D_FUNC_SENDTEXSRT_MAX < 2)
#error NNS_G3D_FUNC_SENDTEXSRT_MAX must be 2 or above.
#endif


#if !defined(NNS_FROM_TOOL)
#include <nitro.h>
#define NNS_G3D_ASSERTMSG     SDK_ASSERTMSG
#define NNS_G3D_ASSERT        SDK_ASSERT
#define NNS_G3D_NULL_ASSERT   SDK_NULL_ASSERT
#define NNS_G3D_WARNING       SDK_WARNING

#else // if !defined(NNS_FROM_TOOL)

#include <nitro_win32.h>
#include <assert.h>
#define NNS_G3D_ASSERTMSG(x, y) assert((x))
#define NNS_G3D_ASSERT(x) assert((x))
#define NNS_G3D_NULL_ASSERT(x) assert(NULL != (x))
//#define NNS_G3D_WARNING(x, ...)

#endif  // if !defined(NNS_FROM_TOOL)

#ifdef __cplusplus
}
#endif

#endif
