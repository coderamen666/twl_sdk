/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2di_RendererMtxCache.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
 
#ifndef NNS_G2DI_RENDERERMTXCACHE_H_
#define NNS_G2DI_RENDERERMTXCACHE_H_

#include <nnsys/g2d/g2d_RendererCore.h> // For NNSG2dRndCore2DMtxCache, NNSG2dSurfaceType
#include "g2d_Internal.h"
#include "g2di_RendererMtxStack.hpp"// for MatrixStack 


//------------------------------------------------------------------------------
typedef enum OAM_FLIP
{
    OAM_FLIP_NONE   = 0,
    OAM_FLIP_H      = 0x01,
    OAM_FLIP_V      = 0x02,
    OAM_FLIP_HV     = 0x03,
    OAM_FLIP_MAX    = NNS_G2D_OAMFLIP_PATTERN_NUM

}OAM_FLIP;

#define OAM_FLIP_ASSERT( val )                                       \
        NNS_G2D_MINMAX_ASSERT( (val), OAM_FLIP_NONE, OAM_FLIP_HV )    \

// convert from flags to OAM_FLIP
#define TO_OAM_FLIP( bFlipH, bFlipV )           (OAM_FLIP)(( bFlipV << 1 ) | ( bFlipH ))


//------------------------------------------------------------------------------

#define G2Di_NUM_MTX_CACHE                      32



//------------------------------------------------------------------------------
// Renderer 2DMatrix Cache 
// Carries out the role of caching actual affine parameter registration to 2D graphics engine.
// Duplicate registrations are not executed, and the result of the past registration is returned.
// 
// As a result when the same NNSG2dRndCore2DMtxCache index is specified, the same affine parameters are referenced.
// 
//
// MtxCache_NOT_AVAILABLE means that affineIndex is the specified value but the affine parameter registration has not yet occurred.
// 
// 
// Reset to the default value is performed by NNSi_G2dMCMCleanupMtxCache(), which is called  by NNS_G2dEndRendering().
// That is, affine parameters can only be shared within the same NNS_G2dBeginRendering-NNS_G2dEndRendering function block.
// 
//
// 
// This module is not directly manipulated by the renderer module. It is manipulated via the RendererMtxState module method.
// 
//
//
// Function names NNSi_RMC.....()          
static NNSG2dRndCore2DMtxCache             mtxCacheBuffer_[G2Di_NUM_MTX_CACHE];

//------------------------------------------------------------------------------
// Added variable: Introduced to hide NNSG2dRndCore2DMtxCache from the user.
static u16                          currentMtxCachePos_ = 0;



//------------------------------------------------------------------------------
// Functions restricted to inside the module
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Module's externally public functions
//------------------------------------------------------------------------------
NNS_G2D_INLINE void NNSi_RMCInitMtxCache()
{
    int i;
    for( i = 0; i < G2Di_NUM_MTX_CACHE; i++ )
    {
        NNS_G2dInitRndCore2DMtxCache( &mtxCacheBuffer_[i] );
    }
    currentMtxCachePos_ = 0;
}

NNS_G2D_INLINE void NNSi_RMCResetMtxCache()
{
    int i;
    //
    // Initialize up to location used
    //
    for( i = 0; i < currentMtxCachePos_; i++ )
    {
        NNS_G2dInitRndCore2DMtxCache( &mtxCacheBuffer_[i] );
    }
    currentMtxCachePos_ = 0;
}

//------------------------------------------------------------------------------
// Gets matrix cache with index.
NNS_G2D_INLINE NNSG2dRndCore2DMtxCache* NNSi_RMCGetMtxCacheByIdx( u16 idx )
{
    NNS_G2D_MINMAX_ASSERT( idx, 0, G2Di_NUM_MTX_CACHE - 1);
    return &mtxCacheBuffer_[idx];
}             

//------------------------------------------------------------------------------
// Uses a new matrix cache.
NNS_G2D_INLINE u16 NNSi_RMCUseNewMtxCache()
{
    const u16 ret = currentMtxCachePos_;
    
    if( currentMtxCachePos_ < G2Di_NUM_MTX_CACHE - 1)
    {
       currentMtxCachePos_++;
    }else{
       // Matrix cache is used up.
       NNS_G2D_WARNING( FALSE, "MtxCache is running out. G2d ignores the user request"
                               ", and uses MtxCache-Idx = 31.");
    }
    
    return ret;
}

#endif // NNS_G2DI_RENDERERMTXCACHE_H_
