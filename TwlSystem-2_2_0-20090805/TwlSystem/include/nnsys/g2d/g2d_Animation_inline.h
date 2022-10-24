/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_Animation_inline.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
 
#ifndef NNS_G2D_ANIMATION_INLINE_H_
#define NNS_G2D_ANIMATION_INLINE_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Inline functions 
//------------------------------------------------------------------------------



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlType

  Description:  Gets animation controller animation type.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
                
  Returns:      the controller type
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE NNSG2dAnimationType NNS_G2dGetAnimCtrlType
( 
    const NNSG2dAnimController*  pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    if( pAnimCtrl->pAnimSequence != NULL )
    {
        return NNS_G2dGetAnimSequenceAnimType( pAnimCtrl->pAnimSequence );
    }else{
        return NNS_G2D_ANIMATIONTYPE_MAX;
    }   
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimCtrlSpeed

  Description:  Sets animation speed for the animation controller.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                speed:           speed
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dSetAnimCtrlSpeed
( 
    NNSG2dAnimController*       pAnimCtrl, 
    fx32                        speed 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    pAnimCtrl->speed = speed;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlSpeed

  Description:  Gets the animation speed for the animation controller.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
                
  Returns:      animation speed
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE fx32 NNS_G2dGetAnimCtrlSpeed
( 
    const NNSG2dAnimController*       pAnimCtrl
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    return pAnimCtrl->speed;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dStartAnimCtrl

  Description:  Starts animation playback for the animation controller.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dStartAnimCtrl
( 
    NNSG2dAnimController*       pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    pAnimCtrl->bActive = TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dStopAnimCtrl

  Description:  Ends animation playback for the animation controller.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
                
  Returns:      the controller type
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dStopAnimCtrl
( 
    NNSG2dAnimController*       pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    pAnimCtrl->bActive = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsAnimCtrlActive

  Description:  Checks whether the animation controller is playing back an animation.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
                
  Returns:      TRUE if animation is playing back
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE BOOL NNS_G2dIsAnimCtrlActive
( 
    const NNSG2dAnimController*       pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    return pAnimCtrl->bActive;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimCtrlPlayModeOverridden

  Description:  Overrides the playback animation sequence playback method specified in the file information.
                
                
  Arguments:    pAnimCtrl:            the animation controller instance
                playMode:            Animation playback method
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dSetAnimCtrlPlayModeOverridden
( 
    NNSG2dAnimController*       pAnimCtrl,
    NNSG2dAnimationPlayMode     playMode 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    NNS_G2D_MINMAX_ASSERT( playMode, NNS_G2D_ANIMATIONPLAYMODE_FORWARD,
                                 NNS_G2D_ANIMATIONPLAYMODE_REVERSE_LOOP );
                                 
    pAnimCtrl->overriddenPlayMode = playMode;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dResetAnimCtrlPlayModeOverridden

  Description:  Resets overridden animation playback method.
                (Returns to playback method specified in file information.)
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dResetAnimCtrlPlayModeOverridden
( 
    NNSG2dAnimController*       pAnimCtrl
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    pAnimCtrl->overriddenPlayMode = NNS_G2D_ANIMATIONPLAYMODE_INVALID;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlCurrentTime

  Description:  Gets the current animation frame display time for the animation controller.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
                
  Returns:      The current animation frame display time
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE fx32 NNS_G2dGetAnimCtrlCurrentTime
( 
    const NNSG2dAnimController*       pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    return pAnimCtrl->currentTime;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimCtrlCurrentTime

  Description:  Sets the current animation frame display time for the animation controller.
                
  Arguments:    pAnimCtrl:            the animation controller instance
                time:           The current animation frame display time
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE void NNS_G2dSetAnimCtrlCurrentTime
( 
    NNSG2dAnimController*       pAnimCtrl,
    fx32                        time
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    pAnimCtrl->currentTime = time;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlCurrentElemIdxVal

  Description:  Gets the index value from the current animation result for the animation controller.
                
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
  Returns:      Index value from the current animation result
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE u16 NNS_G2dGetAnimCtrlCurrentElemIdxVal
( 
    const NNSG2dAnimController*       pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    //
    // Regardless of the animation format, the first two bytes must be used as the index value.
    // 
    //
    {
        const NNSG2dAnimData* pAnmRes 
           = (const NNSG2dAnimData*)pAnimCtrl->pCurrent->pContent;
    
        return (*pAnmRes);        
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetAnimCtrlCurrentAnimSequence

  Description:  Gets the current animation sequence for the animation controller.
                
                
  Arguments:    pAnimCtrl:            the animation controller instance
                
                
  Returns:      Current animation sequence
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE const NNSG2dAnimSequence* 
NNS_G2dGetAnimCtrlCurrentAnimSequence
( 
    const NNSG2dAnimController*       pAnimCtrl 
)
{
    NNS_G2D_NULL_ASSERT( pAnimCtrl );
    return pAnimCtrl->pAnimSequence;
}


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif // NNS_G2D_ANIMATION_INLINE_H_

