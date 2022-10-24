/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2di_RendererMtxState.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
 
#ifndef NNS_G2DI_RENDERERMTXSTATE_H_
#define NNS_G2DI_RENDERERMTXSTATE_H_

#include <nnsys/g2d/g2d_Renderer.h>

#include "g2d_Internal.h"
#include "g2di_RendererMtxStack.hpp"// for MatrixStack 
#include "g2di_RendererMtxCache.h"

//
// Matrix Cache State Manager
//
// An internal module that manages the matrix stack's state, and loads the matrix stack to the matrix cache. 
// 
//
// This manages matrix stack status by wrapping operations on the matrix stack.
// 
// It is used by the rendering module.
// The rendering module does not directly manipulate the matrix cache.
// It carries out all operations via this module's methods.
//
// This is a list of operations on the matrix stack:
//      A: SR value for the current matrix has changed.
//      B: SR value for the current matrix was loaded into the matrix cache.
//      C: Matrix stack was pushed.
//
// The corresponding function calls are as follows.
//
//      A: NNSi_G2dMCMSetCurrentMtxSRChanged()
//      B: NNSi_G2dMCMStoreCurrentMtxToMtxCache( )
//      C: NNSi_G2dMCMSetMtxStackPushed( u16 newPos, u16 lastPos )
//
//  
//
// In contrast, the renderer core module never references this module.
// The renderer core module only uses the matrix cache module.
//
// Function naming NNSi_G2dMCM...




//------------------------------------------------------------------------------
// Type Definitions for Use Inside the Module
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// These express the state of a matrix in the matrix stack.
typedef enum MCMRndMtxStateType
{
    MCM_MTX_NOT_SR = 0,                        // not SR transformed
    MCM_MTX_SR_NOT_CACHELOADED,                // SR transformed, and loading to matrix cache not completed
    MCM_MTX_SR_NOT_CACHELOADED_STACKCHANGED,   // SR transformed, and loading to matrix cache not completed
                                               // stack being manipulated
    MCM_MTX_SR_CACHELOADED                     // SR transformed, and loading to matrix cache completed

}MCMRndMtxStateType;

//------------------------------------------------------------------------------
// Expresses the state of the matrix stack.
// Takes a stack structure that is the same size as the matrices in the matrix stack.
//
// It uses this data to control loading to matrix cache.
//
// Stores information other than the matrix itself.
typedef struct MCMMtxState
{
    u16                      mtxCacheIdx; // matrix cache number
    u16                      groupID;     // group IDs that reference the same matrix cache
    u16                      stateType;   // MCMRndMtxStateType
    u16                      pad16;
    
}MCMMtxState;

//------------------------------------------------------------------------------
NNS_G2D_INLINE u16 GetMtxStateMtxCacheIdx_( const MCMMtxState* pMtxState )
{
    return pMtxState->mtxCacheIdx;
}

NNS_G2D_INLINE void SetMtxStateMtxCacheIdx_( MCMMtxState* pMtxState, u16 cacheIdx )
{
    pMtxState->mtxCacheIdx = cacheIdx;
}

//------------------------------------------------------------------------------
NNS_G2D_INLINE u16 GetMtxStateGroupID_( const MCMMtxState* pMtxState )
{
    return pMtxState->groupID;
}

NNS_G2D_INLINE void SetMtxStateGroupID_( MCMMtxState* pMtxState, u16 groupID )
{
    pMtxState->groupID = groupID;
}



//------------------------------------------------------------------------------
// data that expresses the state of the matrix stack
// This stack is the same size as the matrix stack.
// The current position is aligned with the matrix stack.
static MCMMtxState           mtxStateStack_[G2Di_NUM_MTX_CACHE];
static u16                   groupID_ = 0;



//------------------------------------------------------------------------------
// functions only available internally
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Gets the state structure of the current matrix.
NNS_G2D_INLINE MCMMtxState* GetCuurentMtxState_()
{
    return &mtxStateStack_[NNSi_G2dGetMtxStackPos()];
}

//------------------------------------------------------------------------------
// Visits the stack's parent level and if the parent level's groupID is the same, sets it to be already loaded to the cache. (When the groupID is the same, the same matrix cache is referenced.)
// 
// 
// 
NNS_G2D_INLINE void SetParentMtxStateLoaded_( u16 mtxCacheIdx, u16 groupID )
{
    int i;
    const u16 currntStackPos    = NNSi_G2dGetMtxStackPos();
    
    for( i = currntStackPos; i >= 0; i-- )
    {
        
        if( GetMtxStateGroupID_( &mtxStateStack_[i] ) != groupID )
        {
            break;
        }else{
            // Sets as cache load completed.
            mtxStateStack_[i].stateType   = MCM_MTX_SR_CACHELOADED;
            SetMtxStateMtxCacheIdx_( &mtxStateStack_[i], mtxCacheIdx );
        }
        
        NNSI_G2D_DEBUGMSG1( "Set Loaded => mtxStateStack_[%d].mtxCacheIdx = %d\n", 
                             i, 
                             GetMtxStateMtxCacheIdx_( &mtxStateStack_[i] ) );
    }
}

//------------------------------------------------------------------------------
// Gets a new groupID.
NNS_G2D_INLINE u16 GetNewGroupID_()
{
    return groupID_++;
}

//------------------------------------------------------------------------------
// Gets a new groupID.
NNS_G2D_INLINE void InitGroupID_()
{
    groupID_ = 0;
}

//------------------------------------------------------------------------------
// Functions Available Externally
//------------------------------------------------------------------------------
NNS_G2D_INLINE void NNSi_G2dMCMInitMtxCache()
{
    NNSi_G2dInitRndMtxStack();
    NNSi_RMCInitMtxCache();
    
    InitGroupID_();
    
    //
    // initialize matrix state
    //
    MI_CpuClearFast( mtxStateStack_, sizeof( mtxStateStack_ ) );
}

//------------------------------------------------------------------------------
// Gets a pointer to the current NNSG2dRndCore2DMtxCache.
//
NNS_G2D_INLINE NNSG2dRndCore2DMtxCache*   NNSi_G2dMCMGetCurrentMtxCache()
{
    return NNSi_RMCGetMtxCacheByIdx( GetMtxStateMtxCacheIdx_( GetCuurentMtxState_() ) );
}

//------------------------------------------------------------------------------
// Deletes the content of MtxCache.
//
// This is a process exclusive to 2D graphics engine rendering.
//
NNS_G2D_INLINE void NNSi_G2dMCMCleanupMtxCache()
{
    //
    // matrix stack reset
    //
    NNSi_G2dInitRndMtxStack();
    //
    // reset the matrix cache
    //
    NNSi_RMCResetMtxCache();

    InitGroupID_();
    
    //
    // initialize matrix state
    //
    MI_CpuClearFast( mtxStateStack_, sizeof( mtxStateStack_ ) );
}

//------------------------------------------------------------------------------
// Get whether it is necessary to load the current matrix to cache.
NNS_G2D_INLINE BOOL NNSi_G2dMCMShouldCurrentMtxBeLoadedToMtxCache( )
{
    MCMMtxState*     pCurrMtxState = GetCuurentMtxState_();
    //
    // If SR is converted but not loaded into the cache...
    // 
    //       
    return (BOOL)( pCurrMtxState->stateType == MCM_MTX_SR_NOT_CACHELOADED ||
                   pCurrMtxState->stateType == MCM_MTX_SR_NOT_CACHELOADED_STACKCHANGED );
}

//------------------------------------------------------------------------------
// The current matrix's state is set to SR-transformed.
NNS_G2D_INLINE void NNSi_G2dMCMSetCurrentMtxSRChanged()
{
    MCMMtxState*     pCurrMtxState = GetCuurentMtxState_();
    
    //
    // The process will change according to the state of the current matrix.
    //
    switch( pCurrMtxState->stateType )
    {
    case MCM_MTX_SR_NOT_CACHELOADED:
        //
        // If the current matrix is in the state before loading, there is no need for processes such as updating the state of the current matrix.
        // (I.e., if before loading (load is performed right before rendering), the matrix cache is not used up, no matter how many times SR conversion occurs.)
        // 
        //
        return;
    case MCM_MTX_NOT_SR:
    case MCM_MTX_SR_NOT_CACHELOADED_STACKCHANGED:
    case MCM_MTX_SR_CACHELOADED:
        // matrix stack: updates the state of the current matrix
        {
            // When it is necessary to use a new matrix cache, it is configured with a group ID as a separate group.
            // 
            SetMtxStateGroupID_( pCurrMtxState, GetNewGroupID_() );
            
            //
            // SR has changed, but load to cache has not finished
            //
            pCurrMtxState->stateType = MCM_MTX_SR_NOT_CACHELOADED;
            
            NNSI_G2D_DEBUGMSG1( "currentMtxCachePos change to %d at %d\n", 
                                 pCurrMtxState->mtxCacheIdx,
                                 NNSi_G2dGetMtxStackPos() );
        }
        break;
    }
}


//------------------------------------------------------------------------------
// Processes corresponding to the current matrix push operations.
NNS_G2D_INLINE void NNSi_G2dMCMSetMtxStackPushed( u16 newPos, u16 lastPos )
{
    
    mtxStateStack_[newPos] = mtxStateStack_[lastPos];
    //
    // If the stateType was MCM_MTX_SR_NOT_CACHELOADED, that is changed to MCM_MTX_SR_NOT_CACHELOADED_STACKCHANGED.
    // 
    // 
    // (As a result, a new SR conversion occurs and the behavior when the NNSi_G2dMCMSetCurrentMtxSRChanged function is run changes.)
    //    
    //    
    if( mtxStateStack_[lastPos].stateType == MCM_MTX_SR_NOT_CACHELOADED )
    {
        mtxStateStack_[newPos].stateType = MCM_MTX_SR_NOT_CACHELOADED_STACKCHANGED;
    }else{
        mtxStateStack_[newPos].stateType   = mtxStateStack_[lastPos].stateType;
    }
}

//------------------------------------------------------------------------------
// Stores the current matrix in the specified MtxCache.
// 
// When affine conversion is necessary for objects that used the flip feature, a dedicated matrix is generated and stored. 
// 
//
// This is a process exclusive to 2D graphics engine rendering.
//
static void NNSi_G2dMCMStoreCurrentMtxToMtxCache( )
{   
    // do not do anything if not necessary  
    if( NNSi_G2dMCMShouldCurrentMtxBeLoadedToMtxCache() )
    {
        MCMMtxState* pCurrentState = GetCuurentMtxState_();            
        const u16 mtxCacheIdx = NNSi_RMCUseNewMtxCache();
        const u16 groupID     = GetMtxStateGroupID_( pCurrentState );
           
        NNS_G2D_MINMAX_ASSERT( mtxCacheIdx, 0, G2Di_NUM_MTX_CACHE - 1 );
           
        //
        // determine the cache index
        //
        SetMtxStateMtxCacheIdx_( pCurrentState, mtxCacheIdx );
           
        ////
        //// if there is a need...
        ////
        //if( mtxCacheIdx != NNS_G2D_OAM_AFFINE_IDX_NONE )
        {
           //
           // initialize the cache
           //
           NNS_G2dInitRndCore2DMtxCache( NNSi_RMCGetMtxCacheByIdx( mtxCacheIdx ) );
                  
           //
           // copy the matrix to the cache
           //    
           NNSi_G2dGetMtxRS( NNSi_G2dGetCurrentMtxFor2DHW(), 
                                &NNSi_G2dMCMGetCurrentMtxCache()->m22 );
               
           NNSI_G2D_DEBUGMSG1( "Mtx No %d is cache loaded To %d = %d\n", 
                                        mtxStateStackPos_, 
                                        pCurrMtxState->mtxCacheIdx );
        }
        //
        // Update the status of the parent level, which shares SR transformation with the matrix, to be loaded
        //
        SetParentMtxStateLoaded_( mtxCacheIdx, groupID );
    }
}


#endif // NNS_G2DI_RENDERERMTXSTATE_H_
