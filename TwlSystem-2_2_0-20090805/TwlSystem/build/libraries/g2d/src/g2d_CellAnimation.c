/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_CellAnimation.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1451 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/g2d/g2d_CellAnimation.h>
#include <nnsys/g2d/g2d_SRTControl.h>

#include <nnsys/g2d/load/g2d_NCE_load.h>
#include <nnsys/g2d/fmt/g2d_Oam_data.h>

#include "g2d_Internal.h"

//------------------------------------------------------------------------------
// applies animation changes
//
// This is a bit sloppy, but regardless of the animation format, it will be accessed by NNSG2dAnimDataSRT.
// (If the animation format is NNS_G2D_ANIMELEMENT_INDEX, it will cause incorrect data to be put into the SRT section.)
//     
static void ApplyCurrentAnimResult_( NNSG2dCellAnimation* pCellAnim )
{
    const NNSG2dAnimDataSRT*   pAnimResult  = NULL;
    const NNSG2dCellDataBank*  pCellBank    = NULL;
    
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pCellAnim->pCellDataBank );
    
    //
    // Changed so that animation frames with 0 display time will not update.
    //
    if( pCellAnim->animCtrl.pActiveCurrent->frames == 0 )
    {
        return; 
    }
    pAnimResult = (const NNSG2dAnimDataSRT*)NNS_G2dGetAnimCtrlCurrentElement( &pCellAnim->animCtrl );
    NNS_G2D_NULL_ASSERT( pAnimResult );

    pCellBank   = pCellAnim->pCellDataBank;
    
    NNSI_G2D_DEBUGMSG0( "pAnimResult->index = %d\n", pAnimResult->index );
    
    pCellAnim->pCurrentCell 
        = NNS_G2dGetCellDataByIdx( pCellBank, pAnimResult->index );
    NNS_G2D_NULL_ASSERT( pCellAnim->pCurrentCell );
    
    //
    // use SRT 
    //
    {
       const NNSG2dAnimationElement elemType 
           = NNSi_G2dGetAnimSequenceElementType( pCellAnim->animCtrl.pAnimSequence->animType );
       NNSi_G2dSrtcInitControl  ( &pCellAnim->srtCtrl, NNS_G2D_SRTCONTROLTYPE_SRT );
       
       if( elemType != NNS_G2D_ANIMELEMENT_INDEX )
       {
           if( elemType == NNS_G2D_ANIMELEMENT_INDEX_T )
           {
              const NNSG2dAnimDataT*  pAnmResT = (const NNSG2dAnimDataT*)pAnimResult;
              NNSi_G2dSrtcSetTrans     ( &pCellAnim->srtCtrl, pAnmResT->px, pAnmResT->py );// T
           }else{
              NNSi_G2dSrtcSetSRTScale  ( &pCellAnim->srtCtrl, pAnimResult->sx, pAnimResult->sy );// S    
              NNSi_G2dSrtcSetSRTRotZ   ( &pCellAnim->srtCtrl, pAnimResult->rotZ );// R    
              NNSi_G2dSrtcSetTrans     ( &pCellAnim->srtCtrl, pAnimResult->px, pAnimResult->py );// T    
           }
       }
    }
    
    
   
    //
    // if VRAM transfer information is set...
    //
    if( NNS_G2dCellDataBankHasVramTransferData( pCellBank ) && 
        NNSi_G2dIsCellAnimVramTransferHandleValid( pCellAnim ) )
    {
        const NNSG2dCellVramTransferData*   pCellTransferData 
            = NNSi_G2dGetCellVramTransferData( pCellBank, pAnimResult->index );   
        //
        // requests a transfer
        //
        NNS_G2dSetCellTransferStateRequested( pCellAnim->cellTransferStateHandle,
                                              pCellTransferData->srcDataOffset,
                                              pCellTransferData->szByte );
    }
}

//------------------------------------------------------------------------------
// Initializes cell animation.
// Part that implements NNS_G2dInitCellAnimation.
static NNS_G2D_INLINE void InitCellAnimationImpl_
( 
    NNSG2dCellAnimation*        pCellAnim, 
    const NNSG2dAnimSequence*   pAnimSeq, 
    const NNSG2dCellDataBank*   pCellDataBank,
    u32                         cellTransferStateHandle 
)
{
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pAnimSeq );
    NNS_G2D_NULL_ASSERT( pCellDataBank );
    

    pCellAnim->pCellDataBank            = pCellDataBank;
    pCellAnim->cellTransferStateHandle   = cellTransferStateHandle;
    
    // TODO: The srtCtrl type needs to be set according to the animation format.
    //       It needs to be reset each time there is a bind.
    NNSi_G2dSrtcInitControl( &pCellAnim->srtCtrl, NNS_G2D_SRTCONTROLTYPE_SRT );
    
    NNS_G2dInitAnimCtrl( &pCellAnim->animCtrl );
    NNS_G2dSetCellAnimationSequence( pCellAnim, pAnimSeq );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitCellAnimation

  Description:  Initializes a NNSG2dCellAnimation instance.
                
  Arguments:    pCellAnim:           [OUT]  cell animation instance
                pAnimSeq:            [IN]   animation data
                pCellDataBank:       [IN]   cell databank
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitCellAnimation
( 
    NNSG2dCellAnimation*        pCellAnim, 
    const NNSG2dAnimSequence*   pAnimSeq, 
    const NNSG2dCellDataBank*   pCellDataBank 
)
{
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pAnimSeq );
    NNS_G2D_NULL_ASSERT( pCellDataBank );
    
    InitCellAnimationImpl_( pCellAnim, 
                            pAnimSeq, 
                            pCellDataBank, 
                            NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE ); 
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitCellAnimationVramTransfered

  Description:  Initializes cell animations that perform VRAM transfer animations.
                
  Arguments:    pCellAnim:           [OUT]  cell animation instance
                pAnimSeq:            [IN]   animation data
                pCellBank:           [IN]   cell databank
                vramStateHandle:     [IN] handle of the cell VRAM transfer state object
                dstAddr3D:           [IN]  transfer destination data (for 3D)
                dstAddr2DMain:       [IN]  transfer destination data (for 2D main)
                dstAddr2DSub:        [IN]  transfer destination data (for 2D sub)
                pSrcNCGR:            [IN] transfer source data (NCGR)
                pSrcNCBR:            [IN] transfer source data (NCBR)
                szSrcData:           [IN]  transfer source data size
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitCellAnimationVramTransfered
(
    NNSG2dCellAnimation*        pCellAnim, 
    const NNSG2dAnimSequence*   pAnimSeq, 
    const NNSG2dCellDataBank*   pCellBank,
    u32                         vramStateHandle,
    u32                         dstAddr3D,
    u32                         dstAddr2DMain,
    u32                         dstAddr2DSub,
    const void*                pSrcNCGR,
    const void*                pSrcNCBR,
    u32                         szSrcData
     
)
{
    //
    // has VRAM transfer data
    //
    NNS_G2D_ASSERT( NNS_G2dCellDataBankHasVramTransferData( pCellBank ) );
    
    //
    // initializes VRAM transfer related areas
    //
    {                                                
        const NNSG2dVramTransferData*    pVramData = 
            (const NNSG2dVramTransferData*)pCellBank->pVramTransferData;
        
        // initializes the setup work
        NNSi_G2dInitCellTransferState( vramStateHandle,
                                        dstAddr3D,              // sets the dst transfer destination Vram address
                                        dstAddr2DMain,          // sets the dst transfer destination Vram address
                                        dstAddr2DSub,           // sets the dst transfer destination Vram address
                                        pVramData->szByteMax,   // maximum data size for dst transfer
                                        pSrcNCGR,               // src 2D char-data
                                        pSrcNCBR,               // src 3D Tex-data
                                        szSrcData );            // src size 
           
        NNSi_G2dSetCellAnimVramTransferHandle( pCellAnim, vramStateHandle );
    }
    
    //
    // initialize cell animation
    //
    InitCellAnimationImpl_( pCellAnim, pAnimSeq, pCellBank, vramStateHandle );        
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetCellAnimationSequence

  Description:  Sets the animation sequence to the cell animation.

  Arguments:    pCellAnim:    [OUT]  cell animation instance
                pAnimSeq:     [IN]  animation sequence
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetCellAnimationSequence
( 
    NNSG2dCellAnimation*        pCellAnim, 
    const NNSG2dAnimSequence*   pAnimSeq 
)
{
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pAnimSeq );
    NNS_G2D_ASSERTMSG( NNS_G2dGetAnimSequenceAnimType( pAnimSeq ) == NNS_G2D_ANIMATIONTYPE_CELL, 
                       "A cell-Animation's Data is expected");
    
    NNS_G2dBindAnimCtrl( &pCellAnim->animCtrl, pAnimSeq );
    ApplyCurrentAnimResult_( pCellAnim );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetCellAnimationSequenceNoReset

  Description:  Sets the animation sequence to the cell animation.
                Does not reset the internal playback animation frame number or the current frame display time.

  Arguments:    pCellAnim:  [OUT]  cell animation instance
                pAnimSeq:   [IN]      animation sequence
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetCellAnimationSequenceNoReset
( 
    NNSG2dCellAnimation*        pCellAnim, 
    const NNSG2dAnimSequence*   pAnimSeq 
)
{
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pAnimSeq );
    NNS_G2D_ASSERTMSG( NNS_G2dGetAnimSequenceAnimType( pAnimSeq ) == NNS_G2D_ANIMATIONTYPE_CELL,
                       "A cell-Animation's Data is expected");
    
    {
        const u16 frameIdx = NNS_G2dGetAnimCtrlCurrentFrame( &pCellAnim->animCtrl );
        
        // counter is not reset
        pCellAnim->animCtrl.pAnimSequence = pAnimSeq;
        
        if( !NNS_G2dSetAnimCtrlCurrentFrameNoResetCurrentTime( &pCellAnim->animCtrl, frameIdx ) )
        {
            NNS_G2dResetAnimationState( &pCellAnim->animCtrl );
        }
    }
    
    ApplyCurrentAnimResult_( pCellAnim );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTickCellAnimation

  Description:  Advances the NNSG2dCellAnimation time.
                
  Arguments:    pCellAnim:          [OUT]  cell animation instance
                frames:             [IN] time to advance (in frames)
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dTickCellAnimation( NNSG2dCellAnimation* pCellAnim, fx32 frames )
{
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pCellAnim->animCtrl.pAnimSequence );
    NNS_G2D_ASSERTMSG( NNS_G2dGetAnimSequenceAnimType( pCellAnim->animCtrl.pAnimSequence ) 
        == NNS_G2D_ANIMATIONTYPE_CELL, "A cell-Animation's Data is expected");
    
    if( NNS_G2dTickAnimCtrl( &pCellAnim->animCtrl, frames ) )
    {
        //
        // incorporates the animation results when a frame update occurs
        //
        ApplyCurrentAnimResult_( pCellAnim );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetCellAnimationCurrentFrame

  Description:  Sets the animation frame to the cell animation.
                Nothing occurs if an invalid frame number is specified.
                
  Arguments:    pCellAnim:           [OUT]  cell animation instance
                frameIndex:          [IN]  animation frame number
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetCellAnimationCurrentFrame
( 
    NNSG2dCellAnimation*    pCellAnim, 
    u16                     frameIndex 
)
{
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pCellAnim->animCtrl.pAnimSequence );
    
    if( NNS_G2dSetAnimCtrlCurrentFrame( &pCellAnim->animCtrl, frameIndex ) )
    {
        //
        // incorporates the animation results when a frame update occurs
        //
        ApplyCurrentAnimResult_( pCellAnim );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRestartCellAnimation

  Description:  Restarts cell animation playback from the beginning.
                When playing in reverse, playback restarts from the end of the sequence.
                
  Arguments:    pCellAnim:           [OUT]  cell animation instance
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRestartCellAnimation
( 
    NNSG2dCellAnimation*        pCellAnim
)
{
    NNSG2dAnimController* pAnmCtrl = NULL;
    NNS_G2D_NULL_ASSERT( pCellAnim );   
    
    // Sets the playback animation frame to the beginning.
    pAnmCtrl = NNS_G2dGetCellAnimationAnimCtrl( pCellAnim );
    NNS_G2dResetAnimCtrlState( pAnmCtrl );
    
    // Starts the playback.
    NNS_G2dStartAnimCtrl( pAnmCtrl );
    
    // Shows the animation results.
    ApplyCurrentAnimResult_( pCellAnim );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetCellAnimationSpeed

  Description:  Sets the playback speed of the cell animation.
                
  Arguments:    pCellAnim:      [OUT]  cell animation instance
                speed:          [IN]  playback speed
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetCellAnimationSpeed
(
    NNSG2dCellAnimation*     pCellAnim,
    fx32                     speed  
)
{
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pCellAnim->animCtrl.pAnimSequence );
    
    NNS_G2dSetAnimCtrlSpeed( &pCellAnim->animCtrl, speed );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMakeCellToOams

  Description:  Converts a cell to an Oam column.
                
  Arguments:    pDstOams:           [OUT] buffer for storing Oam
                numDstOam:          [IN] buffer length
                pCell:              [IN] conversion source cell
                pMtxSR:             [IN] conversion to set in the cell (optional)
                pBaseTrans:         [IN] parallel translation to set in the cell (optional)
                bDoubleAffine:      [IN] whether DoubleAffine mode
                
  Returns:      number of Oam attributes when actually converting
  
 *---------------------------------------------------------------------------*/
u16 NNS_G2dMakeCellToOams
( 
    GXOamAttr*              pDstOams,
    u16                     numDstOam, 
    const NNSG2dCellData*   pCell, 
    const MtxFx22*          pMtxSR, 
    const NNSG2dFVec2*      pBaseTrans,
    u16                     affineIndex,
    BOOL                    bDoubleAffine 
)
{
    u16             i = 0;
    NNSG2dFVec2     objTrans;
    GXOamAttr*      pDstOam = NULL;
    u16       		numOBJ = 0;
    
    if( numDstOam < pCell->numOAMAttrs )
    {
		numOBJ = numDstOam;
	}
	else
	{
		numOBJ = pCell->numOAMAttrs;
	}
    
    for( i = 0; i < numOBJ; i++ )
    {
        pDstOam = &pDstOams[i];
        
        NNS_G2dCopyCellAsOamAttr( pCell, i, pDstOam );
        
        //
        // If it's necessary to change the object's position...
        //
        if( pMtxSR != NULL || pBaseTrans != NULL )
        {
            //
            // gets the position
            //
            NNS_G2dGetOamTransFx32( pDstOam, &objTrans );
                    
            //
            // if affine transform is specified...
            // 
            if( pMtxSR != NULL )
            {   
                //
                // For objects configured with double affine, the interpolation value attached to NITRO-CHARACTER is temporarily removed.
                // 
                //
                NNSi_G2dRemovePositionAdjustmentFromDoubleAffineOBJ( pDstOam, 
                                                                     &objTrans );               
                {
                    // overwrite
                    const GXOamEffect effectTypeAfter = ( bDoubleAffine ) ? 
                                             GX_OAM_EFFECT_AFFINE_DOUBLE : GX_OAM_EFFECT_AFFINE;                
                    const BOOL bShouldAdjust = ( effectTypeAfter  == GX_OAM_EFFECT_AFFINE_DOUBLE );
        
                    MulMtx22( pMtxSR, &objTrans, &objTrans );
                    
                    // set affine Index
                    G2_SetOBJEffect( pDstOam, effectTypeAfter, affineIndex );
                    
                    NNSi_G2dAdjustDifferenceOfRotateOrientation( pDstOam, 
                                                                 pMtxSR, 
                                                                 &objTrans, 
                                                                 bShouldAdjust );
                }
            }
            //
            // if parallel translation portion is specified...
            // 
            if( pBaseTrans != NULL )
            {
                objTrans.x += pBaseTrans->x;
                objTrans.y += pBaseTrans->y;
            }
            
            //
            // write back
            //
            // 0x800 => intention to round off
            G2_SetOBJPosition( pDstOam, 
                               ( objTrans.x + 0x800 )>> FX32_SHIFT, 
                               ( objTrans.y + 0x800 )>> FX32_SHIFT );
        }
    }
    return numOBJ;
}

