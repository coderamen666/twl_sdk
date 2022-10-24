/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_Animation.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/g2d/g2d_Animation.h>
#include <stdlib.h>


//------------------------------------------------------------------------------
// Private Functions


//------------------------------------------------------------------------------
static NNS_G2D_INLINE const NNSG2dAnimFrame* GetFrameBegin_( const NNSG2dAnimSequence* pSequence )
{    
    NNS_G2D_NULL_ASSERT( pSequence );
    return pSequence->pAnmFrameArray;
}
//------------------------------------------------------------------------------
static NNS_G2D_INLINE const NNSG2dAnimFrame* GetFrameEnd_( const NNSG2dAnimSequence* pSequence )
{    
    NNS_G2D_NULL_ASSERT( pSequence );
    return pSequence->pAnmFrameArray + ( pSequence->numFrames );
}
//------------------------------------------------------------------------------
static NNS_G2D_INLINE const NNSG2dAnimFrame* GetFrameLoopBegin_( const NNSG2dAnimSequence* pSequence )
{
    NNS_G2D_NULL_ASSERT( pSequence );
    
    return pSequence->pAnmFrameArray + pSequence->loopStartFrameIdx;
}
//------------------------------------------------------------------------------
// TODO: NOT EFFICIENT !
static NNS_G2D_INLINE u16 GetCurrentFrameIdx_( const NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    return (u16)(( (u32)pAnimCtrl->pCurrent - (u32)pAnimCtrl->pAnimSequence->pAnmFrameArray ) 
                    / sizeof( NNSG2dAnimFrameData ));
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE NNSG2dAnimationPlayMode GetAnimationPlayMode_
( 
    const NNSG2dAnimController* pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pAnimSequence );
    
    //
    // If pAnimCtrl->overriddenPlayMode is set, that will override the playback mode.
    // 
    // 
    if( pAnimCtrl->overriddenPlayMode != NNS_G2D_ANIMATIONPLAYMODE_INVALID )
    {
        return pAnimCtrl->overriddenPlayMode;
    }else{
        return pAnimCtrl->pAnimSequence->playMode;
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL IsLoopAnimSequence_( const NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pAnimSequence );
    
    {
    const NNSG2dAnimationPlayMode   playMode = GetAnimationPlayMode_( pAnimCtrl );
    

    return ( playMode == NNS_G2D_ANIMATIONPLAYMODE_FORWARD_LOOP || 
             playMode == NNS_G2D_ANIMATIONPLAYMODE_REVERSE_LOOP ) ? TRUE : FALSE;
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL IsReversePlayAnim_( const NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pAnimSequence );
    
    {
    const NNSG2dAnimationPlayMode   playMode = GetAnimationPlayMode_( pAnimCtrl );
    

    return ( playMode == NNS_G2D_ANIMATIONPLAYMODE_REVERSE || 
             playMode == NNS_G2D_ANIMATIONPLAYMODE_REVERSE_LOOP ) ? TRUE : FALSE;
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL IsAnimCtrlMovingForward_( const NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );    
    return ( pAnimCtrl->speed > 0 )^(pAnimCtrl->bReverse) ? TRUE : FALSE;
}




//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL ShouldAnmCtrlMoveNext_( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pCurrent );
    
    if( pAnimCtrl->bActive && 
        (pAnimCtrl->currentTime >= FX32_ONE * (int)pAnimCtrl->pCurrent->frames) )
    {
        return TRUE;
    }
    return FALSE;
}


//------------------------------------------------------------------------------
static NNS_G2D_INLINE  void CallbackFuncHandling_( const NNSG2dCallBackFunctor*  pFunctor, u16 currentFrameIdx )
{
    NNS_G2D_NULL_ASSERT( pFunctor );
    
    switch( pFunctor->type )
    {
    // Call at the specified frame
    case NNS_G2D_ANMCALLBACKTYPE_SPEC_FRM:
        if( currentFrameIdx == pFunctor->frameIdx )
        {
            (*pFunctor->pFunc)( pFunctor->param, currentFrameIdx );
        }   
        break;
    // Call for every frame
    case NNS_G2D_ANMCALLBACKTYPE_EVER_FRM:        
        (*pFunctor->pFunc)( pFunctor->param, currentFrameIdx );
        break;
    }
}

//------------------------------------------------------------------------------
// Wrapper for external publication
void NNSi_G2dCallbackFuncHandling( const NNSG2dCallBackFunctor*  pFunctor, u16 currentFrameIdx )
{
    CallbackFuncHandling_( pFunctor, currentFrameIdx );
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL IsReachStartEdge_( const NNSG2dAnimController* pAnimCtrl, const NNSG2dAnimFrame* pFrame )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pFrame );
    
    return ( pFrame <= ( GetFrameLoopBegin_( pAnimCtrl->pAnimSequence ) - 1 ) ) ? TRUE : FALSE;
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL IsReachEdge_( const NNSG2dAnimController* pAnimCtrl, const NNSG2dAnimFrame* pFrame )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pFrame );
    
    
    if( IsAnimCtrlMovingForward_( pAnimCtrl ) )
    {
        // Tail edge of the animation sequence
        return ( pFrame >= GetFrameEnd_( pAnimCtrl->pAnimSequence ) ) ? TRUE : FALSE;
    }else{
        // Head edge of the animation sequence
        return IsReachStartEdge_( pAnimCtrl, pFrame );
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void MoveNext_( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    
    if( IsAnimCtrlMovingForward_( pAnimCtrl ) )
    {
        pAnimCtrl->pCurrent++;
        
    }else{
        pAnimCtrl->pCurrent--;    
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void SequenceEdgeHandleCommon_( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    
    //
    // NNS_G2D_ANMCALLBACKTYPE_LAST_FRM-type callback call process
    // 
    // 
    if( pAnimCtrl->callbackFunctor.type == NNS_G2D_ANMCALLBACKTYPE_LAST_FRM )
    {
        NNS_G2D_NULL_ASSERT( pAnimCtrl->callbackFunctor.pFunc );
        (*pAnimCtrl->callbackFunctor.pFunc)( pAnimCtrl->callbackFunctor.param, pAnimCtrl->currentTime );
    }
    
    // 
    // Reset process
    //
    if( !IsLoopAnimSequence_( pAnimCtrl ) )
    {
        // If no looped playback, stop update
        NNS_G2dStopAnimCtrl( pAnimCtrl );
    }else{
        NNS_G2dResetAnimCtrlState( pAnimCtrl );
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void SequenceEdgeHandleReverse_( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    
    // Reverse the playback direction
    pAnimCtrl->bReverse = pAnimCtrl->bReverse^TRUE;
    
    //
    // Is this the animation start frame?
    // 
    if( IsReachStartEdge_( pAnimCtrl, pAnimCtrl->pCurrent ) )
    {
        SequenceEdgeHandleCommon_( pAnimCtrl );
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void SequenceEdgeHandleNormal_( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    SequenceEdgeHandleCommon_( pAnimCtrl );
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void ValidateAnimFrame_( NNSG2dAnimController* pAnimCtrl, const NNSG2dAnimFrame** pFrame )
{
    if( *pFrame > GetFrameEnd_( pAnimCtrl->pAnimSequence ) - 1 )
    {
        *pFrame = GetFrameEnd_( pAnimCtrl->pAnimSequence ) - 1;
    }else if( *pFrame < GetFrameBegin_( pAnimCtrl->pAnimSequence ) ){
        *pFrame = GetFrameBegin_( pAnimCtrl->pAnimSequence );
    }
}

//------------------------------------------------------------------------------
static void SequenceEdgeHandle_( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    
    if( IsReversePlayAnim_( pAnimCtrl ) )
    {
        SequenceEdgeHandleReverse_( pAnimCtrl );
    }else{
        SequenceEdgeHandleNormal_( pAnimCtrl );
    }
    
    ValidateAnimFrame_( pAnimCtrl, &pAnimCtrl->pCurrent );
}

//------------------------------------------------------------------------------
// This is the implementation of SetAnimCtrlCurrentFrame.
// Called from the NNS_G2dSetAnimCtrlCurrentFrame and NNS_G2dSetAnimCtrlCurrentFrameNoResetCurrentTime functions.
// 
// 
//
static BOOL SetAnimCtrlCurrentFrameImpl_
( 
    NNSG2dAnimController*     pAnimCtrl, 
    u16                       index
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pAnimSequence );
    
    
    
    // Valid index?
    if( index < pAnimCtrl->pAnimSequence->numFrames )
    {
        // set
        pAnimCtrl->pCurrent = &pAnimCtrl->pAnimSequence->pAnmFrameArray[index];
        if( pAnimCtrl->pCurrent->frames != 0 )
        {
           pAnimCtrl->pActiveCurrent = pAnimCtrl->pCurrent;
        }
        return TRUE;   
    }
    return FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlCurrentElement
                NNS_G2dGetAnimCtrlNextElement

  Description:  Gets the current (next) animation result.
                The user can use the results of this function after being bound to an NNSG2dAnimController instance and cast to the appropriate type, depending on the animation data type.
                
                
  Arguments:    pAnimCtrl:           [IN] NNSG2dAnimController instance
                
  Returns:      Pointer to the current animation result
  
 *---------------------------------------------------------------------------*/
void* NNS_G2dGetAnimCtrlCurrentElement( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pActiveCurrent );
    NNS_G2D_NULL_ASSERT( (void*)pAnimCtrl->pActiveCurrent->pContent );
    
    // Made it so the valid display target is returned. (There are cases when pCurrent display frames is zero.)
    return (void*)pAnimCtrl->pActiveCurrent->pContent;
}
//------------------------------------------------------------------------------
void* NNS_G2dGetAnimCtrlNextElement( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    {
        const NNSG2dAnimFrame*      pNext = NULL;
        
        if( IsAnimCtrlMovingForward_( pAnimCtrl ) )
        {
            pNext = pAnimCtrl->pCurrent + 1;
        }else{
            pNext = pAnimCtrl->pCurrent - 1;
        }
        
        ValidateAnimFrame_( pAnimCtrl, &pNext );
        NNS_G2D_NULL_ASSERT( pNext->pContent );
        
        return (void*)pNext->pContent;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlNormalizedTime

  Description:  Gets the displayed time, the start time and the end time of the current displaying animation frame as a percentage between 0.0 and 1.0, in fx32 type. 
                
                
  Arguments:    pAnimCtrl:          [IN] NNSG2dAnimController instance
                
  Returns:      Animation frame display time
  
 *---------------------------------------------------------------------------*/
fx32 NNS_G2dGetAnimCtrlNormalizedTime(  NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pCurrent );
    
    return FX_Div( pAnimCtrl->currentTime, FX32_ONE * pAnimCtrl->pCurrent->frames );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTickAnimCtrl

  Description:  Advances the time for NNSG2dAnimController.
                
  Arguments:    pAnimCtrl:          [OUT]  NNSG2dAnimController instance
                frames:             [IN] time to advance (in frames)
                
                
  Returns:      Whether the playback track has changed
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dTickAnimCtrl( NNSG2dAnimController* pAnimCtrl, fx32 frames ) 
{
    BOOL    bChangeFrame = FALSE;
    
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pCurrent );
    NNS_G2D_ASSERTMSG( frames >=  0, "frames must be Greater than zero");
    
    
    if( pAnimCtrl->bActive != TRUE )
    {
        return FALSE;
    } 
       
    //
    // Update current time
    //
    pAnimCtrl->currentTime += abs( FX_Mul( pAnimCtrl->speed, frames ) );
    
    
    //    
    // Do actual work to change current animation frame
    //
    while( ShouldAnmCtrlMoveNext_( pAnimCtrl ) )
    {
        bChangeFrame = TRUE; // frame has changed
        
        pAnimCtrl->currentTime -= FX32_ONE * (int)pAnimCtrl->pCurrent->frames;    
        MoveNext_( pAnimCtrl );
        
        // If we reach the edge of animation, we have to reset status.
        if( IsReachEdge_( pAnimCtrl, pAnimCtrl->pCurrent ) )
        {
            SequenceEdgeHandle_( pAnimCtrl );
        }

        // If the display frame is not zero, it is considered a valid display frame.
        // 
        if( pAnimCtrl->pCurrent->frames != 0 )
        {
           pAnimCtrl->pActiveCurrent = pAnimCtrl->pCurrent;
        }
        
        // Call the callback function
        if( pAnimCtrl->callbackFunctor.type != NNS_G2D_ANMCALLBACKTYPE_NONE )
        {
            CallbackFuncHandling_( &pAnimCtrl->callbackFunctor, GetCurrentFrameIdx_( pAnimCtrl ) );
        }    
    }    
    return bChangeFrame;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimCtrlCurrentFrame

  Description:  Sets the animation controller's playback animation frame.
                
  Arguments:    pAnimCtrl:        [OUT]  NNSG2dAnimController instance
                index:        [IN]  animation sequence number
                        
  Returns:      TRUE if the change has succeeded.
  
 *---------------------------------------------------------------------------*/
BOOL           NNS_G2dSetAnimCtrlCurrentFrame
( 
    NNSG2dAnimController*     pAnimCtrl, 
    u16                       index
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pAnimSequence );
    {
        const BOOL result = SetAnimCtrlCurrentFrameImpl_( pAnimCtrl, index );
        
        if( result )
        {
            // Reset the current animation frame display time
            pAnimCtrl->currentTime   = 0;
        }
        return result;
    }       
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimCtrlCurrentFrameNoResetCurrentTime

  Description:  Sets the animation controller's playback animation frame.
                When the frame is changed, the current frame display time counter within the animation controller is not reset.
                
                
                As a result, it is possible to advance an animation sequence, even when switching to an animation frame that is shorter in duration than one animation frame's display time.
                
                

                
  Arguments:    pAnimCtrl:           [OUT]  NNSG2dAnimController instance
                index:           [IN]  animation sequence number
                        
  Returns:      TRUE if the change has succeeded.
  
 *---------------------------------------------------------------------------*/
BOOL           NNS_G2dSetAnimCtrlCurrentFrameNoResetCurrentTime
( 
    NNSG2dAnimController*     pAnimCtrl, 
    u16                       index
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pAnimSequence );
    
    //
    // Do not reset the current animation frame display time
    //
    return SetAnimCtrlCurrentFrameImpl_( pAnimCtrl, index );
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlCurrentFrame

  Description:  Gets the current animation frame number in NNSG2dAnimController.
                
  Arguments:    pAnimCtrl:          [OUT]  NNSG2dAnimController instance
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
u16 NNS_G2dGetAnimCtrlCurrentFrame
(
    const NNSG2dAnimController*     pAnimCtrl
)
{
    return GetCurrentFrameIdx_( pAnimCtrl );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitAnimCtrl

  Description:  Initializes NNSG2dAnimController.
                
  Arguments:    pAnimCtrl:          [OUT]  NNSG2dAnimController instance
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitAnimCtrl( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    
    NNS_G2dInitAnimCallBackFunctor( &pAnimCtrl->callbackFunctor );
    
    pAnimCtrl->pCurrent              = NULL;
    pAnimCtrl->pActiveCurrent        = NULL;
    
    pAnimCtrl->bReverse              = FALSE;
    pAnimCtrl->bActive               = TRUE;
    
    pAnimCtrl->currentTime           = 0;
    pAnimCtrl->speed                 = FX32_ONE;
    
    pAnimCtrl->overriddenPlayMode    = NNS_G2D_ANIMATIONPLAYMODE_INVALID;
    pAnimCtrl->pAnimSequence         = NULL;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitAnimCtrlCallBackFunctor

  Description:  Initializes callbacks inside NNSG2dAnimController.
                
  Arguments:    pAnimCtrl:          [OUT]  NNSG2dAnimController instance
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitAnimCtrlCallBackFunctor( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    
    NNS_G2dInitAnimCallBackFunctor( &pAnimCtrl->callbackFunctor );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitAnimCallBackFunctor

  Description:  Initializes callback functors.
                
  Arguments:    pCallBack:          [OUT] NNSG2dCallBackFunctor instance
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitAnimCallBackFunctor( NNSG2dCallBackFunctor* pCallBack )
{
    NNS_G2D_NULL_ASSERT( pCallBack );
    
    pCallBack->type     = NNS_G2D_ANMCALLBACKTYPE_NONE;
    pCallBack->param    = 0x0;
    pCallBack->pFunc    = NULL;  
    pCallBack->frameIdx = 0;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dResetAnimCtrlState

  Description:  Resets the state of NNSG2dAnimController.
                
  Arguments:    pAnimCtrl:          [OUT]  NNSG2dAnimController instance
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dResetAnimCtrlState( NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    
    if( IsAnimCtrlMovingForward_( pAnimCtrl ) )
    {
        pAnimCtrl->pCurrent      = GetFrameLoopBegin_( pAnimCtrl->pAnimSequence );
    }else{
        pAnimCtrl->pCurrent      = GetFrameEnd_( pAnimCtrl->pAnimSequence ) - 1;
    }
    // Even when a sequence is comprised of only zero-frame display frames, it is not set to NULL.
    // 
    pAnimCtrl->pActiveCurrent = pAnimCtrl->pCurrent;
    
    //
    // Resets currentTime
    //
    pAnimCtrl->currentTime   = 0;
    
    //
    // Advance the zero frame counter
    // An animation frame with a display frame length of 0 exists at the beginning.
    // routine to support animation sequence
    //
    (void)NNS_G2dTickAnimCtrl( pAnimCtrl, 0 );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBindAnimCtrl

  Description:  Associates animation data with the NNSG2dAnimController instance.
                
  Arguments:    pAnimCtrl:           [OUT]  NNSG2dAnimController instance
                pAnimSequence:       [IN]   animation data
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dBindAnimCtrl( NNSG2dAnimController* pAnimCtrl, const NNSG2dAnimSequence* pAnimSequence )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimSequence );
    
    pAnimCtrl->pAnimSequence = pAnimSequence;
    
    NNS_G2dResetAnimCtrlState( pAnimCtrl );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimCtrlCallBackFunctor

  Description:  Configures callback settings.
                Use the NNS_G2dSetAnimCtrlCallBackFunctorAtAnimFrame function to configure callbacks for specified frame call-types.
                
                
  Arguments:    pAnimCtrl:         [OUT]  NNSG2dAnimController instance
                type:              [IN]  callback type
                param:             [IN]  user-defined information
                pFunc:             [IN]  callback function pointer
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetAnimCtrlCallBackFunctor
( 
    NNSG2dAnimController*   pAnimCtrl, 
    NNSG2dAnmCallbackType   type, 
    u32                     param, 
    NNSG2dAnmCallBackPtr    pFunc 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( (void*)pFunc );
    NNS_G2D_MINMAX_ASSERT( type, NNS_G2D_ANMCALLBACKTYPE_NONE, AnmCallbackType_MAX );
    NNS_G2D_ASSERTMSG( type != NNS_G2D_ANMCALLBACKTYPE_SPEC_FRM, 
        "Use NNS_G2dSetAnimCtrlCallBackFunctorAtAnimFrame() instead." );
        
    pAnimCtrl->callbackFunctor.pFunc     = pFunc;
    pAnimCtrl->callbackFunctor.param     = param;
    pAnimCtrl->callbackFunctor.type      = type;
    pAnimCtrl->callbackFunctor.frameIdx  = 0; // meaningless for these kinds of types
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimCtrlCallBackFunctorAtAnimFrame

  Description:  Configures callback settings.
                Use the NNS_G2dSetAnimCtrlCallBackFunctorAtAnimFrame function to configure callbacks for specified frame call-types.
                
                
  Arguments:    pAnimCtrl:         [OUT]  NNSG2dAnimController instance
                param:             [IN]  user-defined information
                pFunc:             [IN]  callback function pointer
                frameIdx:          [IN]    animation frame number that invokes callback
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetAnimCtrlCallBackFunctorAtAnimFrame
( 
    NNSG2dAnimController*   pAnimCtrl, 
    u32                     param, 
    NNSG2dAnmCallBackPtr    pFunc, 
    u16                     frameIdx 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( (void*)pFunc );
    
    pAnimCtrl->callbackFunctor.type  = NNS_G2D_ANMCALLBACKTYPE_SPEC_FRM;
    
    pAnimCtrl->callbackFunctor.pFunc     = pFunc;
    pAnimCtrl->callbackFunctor.param     = param;
    pAnimCtrl->callbackFunctor.frameIdx  = frameIdx;
}

/*---------------------------------------------------------------------------*
  Name:         NNSi_G2dIsAnimCtrlLoopAnim

  Description:  Determines whether the animation controller is playing a loop animation.
                
                
                [library internal public function]
               
  Arguments:    pAnimCtrl:          [IN] NNSG2dAnimController instance
                
  Returns:      If playing a loop animation, returns TRUE.
  
 *---------------------------------------------------------------------------*/
BOOL NNSi_G2dIsAnimCtrlLoopAnim( const NNSG2dAnimController* pAnimCtrl )
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_NULL_ASSERT( pAnimCtrl->pAnimSequence );
    
    return IsLoopAnimSequence_( pAnimCtrl );
}
