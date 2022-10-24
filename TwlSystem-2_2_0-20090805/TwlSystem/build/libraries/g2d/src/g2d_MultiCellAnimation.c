/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_MultiCellAnimation.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/g2d/g2d_MultiCellAnimation.h>
#include <nnsys/g2d/g2d_SRTControl.h>

#include <nnsys/g2d/load/g2d_NMC_load.h>
#include <nnsys/g2d/load/g2d_NAN_load.h>


//------------------------------------------------------------------------------
// Sets node information in a cell animation.
static NNS_G2D_INLINE void SetNodeDataToCellAnim_( 
    NNSG2dCellAnimation*                  pCellAnim,
    const NNSG2dMultiCellHierarchyData*   pNodeData, 
    const NNSG2dAnimBankData*             pAnimBank,
    u16                                   mcTotalFrame 
)
{
    const NNSG2dAnimSequenceData*  pAnimSeq     = NULL;
    
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_NULL_ASSERT( pNodeData );
    NNS_G2D_NULL_ASSERT( pAnimBank );
    
    
    // Gets the animation instance.
    pAnimSeq = NNS_G2dGetAnimSequenceByIdx( pAnimBank, pNodeData->animSequenceIdx );
    
    //
    // Reflects the NNSG2dNode setup data.
    // The currently displayed animation frame is reset to the sequence start.
    //
    NNS_G2dSetCellAnimationSequence( pCellAnim, pAnimSeq );            
    // Starts the playback.
    NNS_G2dStartAnimCtrl( NNS_G2dGetCellAnimationAnimCtrl( pCellAnim ) );
    
                
    //
    // Is cell animation playback set to continuous playback?
    // 
    //
    if( NNSi_G2dGetMultiCellNodePlayMode( pNodeData ) == NNS_G2D_MCANIM_PLAYMODE_CONTINUE )
    {
        // When the NNS_G2dSetCellAnimationSequence function is run, the cell animation's playback location is tentatively reset to the start of the animation sequence.
        // 
        // 
        // At this point, updates the playback position of the cell animation to the proper position.
        const u32 animSeqLength = NNS_G2dCalcAnimSequenceTotalVideoFrames( pAnimSeq );
        
        // Was looping playback specified?
        // When not specified, it is necessary to stop the animation at its end, the first time only, after it was played back.
        // 
        // 
        if( NNSi_G2dIsAnimCtrlLoopAnim( NNS_G2dGetCellAnimationAnimCtrl( pCellAnim ) ) )
        {
           const u32 frameToMove   = ( mcTotalFrame % animSeqLength );
           NNS_G2dTickCellAnimation( pCellAnim, (fx32)( frameToMove  << FX32_SHIFT ) );                            
        }else{
           const u32 frameToMove   = ( mcTotalFrame >= animSeqLength ) ? animSeqLength : mcTotalFrame;
           NNS_G2dTickCellAnimation( pCellAnim, (fx32)( frameToMove  << FX32_SHIFT ) );        
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetMCDataToMCInstanceImpl_

  Description:  Sets multicell data in a multicell instance.
                This needs to be called after initialization.
                Enough nodes must be maintained for the multicell instance to reflect the multicell data.
                
                
                Caution:
                - In previous versions, specification indicated that a FALSE would be returned and processing halted when conditions were not met after comparing pMCInst->numNode and pMcData->numNodes.
                 
                 However, it was determined that such a specification was nearly meaningless in practical use. Changes were made to continue processing for multicell animation instances, assuming that they always meet the above conditions.
                 
                 
                 Therefore the user must assure that conditions are met.
                
                - The internal structure of a multicell instance will vary greatly depending on the NNSG2dMultiCellInstance.mcType member specified at initialization.
                 
                 Accordingly, processing even in this function can be divided into two main sections.
                
                
  Arguments:    pMultiCellAnim:        multicell animation instance
                pMcData:        Multicell data
                mcTotalFrame:        animation playback total video frame length  
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
 
//
// This function has been changed to a private function.
// The old API is left here for compatibility with previous versions.
//
static void SetMCDataToMCInstanceImpl_
( 
    NNSG2dMultiCellInstance*       pMCInst, 
    const NNSG2dMultiCellData*     pMcData,
    u16                            mcTotalFrame 
)
{
    u16 i;
    
    NNS_G2D_NULL_ASSERT( pMCInst );
    NNS_G2D_NULL_ASSERT( pMCInst->pAnimDataBank );
    NNS_G2D_NULL_ASSERT( pMcData );
    
    pMCInst->pCurrentMultiCell = pMcData;
    
    //
    // Multicell instance type?
    //  The internal structure of a multicell instance will vary greatly depending on the NNSG2dMultiCellInstance.mcType member specified at initialization.
    //  
    //  In particular, the method used to get pCellAnim is quite different.
    //
    if( pMCInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
        
        NNSG2dMCCellAnimation* pCellAnimArray = (NNSG2dMCCellAnimation*)pMCInst->pCellAnimInstasnces;
        
        //
        // Resets the initialized flag.
        //
        for( i = 0; i < pMcData->numCellAnim; i++ )
        {
            pCellAnimArray[i].bInited = FALSE;
        }
        
        // for each NNSG2dNode...
        for( i = 0; i < pMcData->numNodes; i++ )
        {
            // 
            // TODO: Get from nodeAttr. Use converter to insert data.
            //
            const NNSG2dMultiCellHierarchyData* pNodeData    = &pMcData->pHierDataArray[i];
            const u16                           cellAnimIdx  = NNSi_G2dGetMC2NodeCellAinmIdx( pNodeData );
            
            // Initialized?
            // When there is a cell animation shared by multiple nodes, enabled, with conditional determination.
            // 
            if( !pCellAnimArray[cellAnimIdx].bInited )
            {
                NNSG2dCellAnimation*          pCellAnim = &pCellAnimArray[cellAnimIdx].cellAnim;
                
                SetNodeDataToCellAnim_( pCellAnim, 
                                        pNodeData, 
                                        pMCInst->pAnimDataBank,
                                        mcTotalFrame );
                  
                pCellAnimArray[cellAnimIdx].bInited = TRUE;
            }
        }
        
    }else{
       
       NNSG2dNode*   pNodeArray   = (NNSG2dNode*)(pMCInst->pCellAnimInstasnces);
       
       
       // for each NNSG2dNode...
       for( i = 0; i < pMcData->numNodes; i++ )
       {
           const NNSG2dMultiCellHierarchyData* pNodeData    = &pMcData->pHierDataArray[i];
           NNSG2dCellAnimation*  pCellAnim    = (NNSG2dCellAnimation*)pNodeArray[i].pContent;
                  
           SetNodeDataToCellAnim_( pCellAnim, 
                                   pNodeData, 
                                   pMCInst->pAnimDataBank,
                                   mcTotalFrame );
               
           pNodeArray[i].bVisible = TRUE;                          
           NNSi_G2dSrtcSetTrans( &pNodeArray[i].srtCtrl, pNodeData->posX, pNodeData->posY );             
       }
    }
}


//------------------------------------------------------------------------------
static NNS_G2D_INLINE void ApplyCurrentAnimResult_( NNSG2dMultiCellAnimation* pMultiCellAnim )
{
    //
    // This is a bit sloppy, but regardless of the animation format, it will be accessed by NNSG2dAnimDataSRT.
    // (If the animation format is NNS_G2D_ANIMELEMENT_INDEX, it will cause incorrect data to be put into the SRT section.)
    // 
    const NNSG2dAnimDataSRT*      pAnimResult  = NULL;
    const NNSG2dMultiCellData*    pData        = NULL;
    
    
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    NNS_G2D_NULL_ASSERT( pMultiCellAnim->pMultiCellDataBank );
    
    //
    // Changed so that animation frames with 0 display time will not update.
    //
    if( pMultiCellAnim->animCtrl.pActiveCurrent->frames == 0 )
    {
        return; 
    }
    pAnimResult = (const NNSG2dAnimDataSRT*)NNS_G2dGetAnimCtrlCurrentElement( &pMultiCellAnim->animCtrl );
    NNS_G2D_NULL_ASSERT( pAnimResult );

    
    pData  
        = NNS_G2dGetMultiCellDataByIdx( pMultiCellAnim->pMultiCellDataBank, 
                                        pAnimResult->index );
    NNS_G2D_NULL_ASSERT( pData );
    
    //
    // reflect SRT 
    //
    {
       const NNSG2dAnimationElement elemType 
           = NNSi_G2dGetAnimSequenceElementType( pMultiCellAnim->animCtrl.pAnimSequence->animType );
       NNSi_G2dSrtcInitControl  ( &pMultiCellAnim->srtCtrl, NNS_G2D_SRTCONTROLTYPE_SRT );
       
       if( elemType != NNS_G2D_ANIMELEMENT_INDEX )
       {
           if( elemType == NNS_G2D_ANIMELEMENT_INDEX_T )
           {
              const NNSG2dAnimDataT*  pAnmResT = (const NNSG2dAnimDataT*)pAnimResult;
              NNSi_G2dSrtcSetTrans     ( &pMultiCellAnim->srtCtrl, pAnmResT->px, pAnmResT->py );// T
           }else{
              NNSi_G2dSrtcSetSRTScale  ( &pMultiCellAnim->srtCtrl, pAnimResult->sx, pAnimResult->sy );// S    
              NNSi_G2dSrtcSetSRTRotZ   ( &pMultiCellAnim->srtCtrl, pAnimResult->rotZ );// R    
              NNSi_G2dSrtcSetTrans     ( &pMultiCellAnim->srtCtrl, pAnimResult->px, pAnimResult->py );// T    
           }
       }
       
    }
    
    //
    // multicell settings
    //    
    SetMCDataToMCInstanceImpl_( &pMultiCellAnim->multiCellInstance, 
                                pData, 
                                pMultiCellAnim->totalVideoFrame );
    
    NNS_G2D_NULL_ASSERT( pMultiCellAnim->multiCellInstance.pCurrentMultiCell );
}



//------------------------------------------------------------------------------
static NNS_G2D_INLINE void FVecToSVec( const NNSG2dFVec2* pvSrc, NNSG2dSVec2* pvDst )
{
    pvDst->x = (s16)(pvSrc->x >> FX32_SHIFT);
    pvDst->y = (s16)(pvSrc->y >> FX32_SHIFT);
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void SVecToFVec( const NNSG2dSVec2* pvSrc, NNSG2dFVec2* pvDst )
{
    pvDst->x = pvSrc->x << FX32_SHIFT;
    pvDst->y = pvSrc->y << FX32_SHIFT;
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void AddSVec_( const NNSG2dSVec2* pv1, const NNSG2dSVec2* pv2, NNSG2dSVec2* pvDst )
{
    pvDst->x = (s16)(pv1->x + pv2->x);
    pvDst->y = (s16)(pv1->y + pv2->y);
}

//------------------------------------------------------------------------------
//
static u16 GetMCBankNumCellAnimRequired_
( 
    const NNSG2dMultiCellDataBank*       pMultiCellDataBank 
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellDataBank );    
    {   
        const NNSG2dMultiCellData*  pMCell    = NULL;
        u16                         maxNum    = 0;
        u16                         i;
        
        for( i = 0; i < pMultiCellDataBank->numMultiCellData; i++ )
        {
            pMCell = NNS_G2dGetMultiCellDataByIdx( pMultiCellDataBank, i );
            NNS_G2D_NULL_ASSERT( pMCell );
            
            if( pMCell->numCellAnim > maxNum )
            {
                maxNum = pMCell->numCellAnim;
            }
        }
        return maxNum;
    }
}

//------------------------------------------------------------------------------
static u16 MakeCellAnimToOams_
(
    GXOamAttr*                   pDstOams, 
    u16                          numDstOams,
    const NNSG2dCellAnimation*   pCellAnim,
    const NNSG2dSVec2*           pNodeTrans,
    const MtxFx22*               pMtxSR, 
    const NNSG2dFVec2*           pBaseTrans,
    u16                          affineIndex,
    BOOL                         bDoubleAffine
)
{
    NNSG2dFVec2                       vTransF;
    NNSG2dSVec2                       vTransS;
    const NNSG2dSRTControl*           pContentsSRT;
    
    pContentsSRT = &pCellAnim->srtCtrl;
   
    // trans
    {
        // vTransS = pContentsSRT->srtData.trans + pNode->srtCtrl.srtData.trans
        AddSVec_( &pContentsSRT->srtData.trans, 
                   pNodeTrans, 
                  &vTransS );
        
        
        // If pMtxSR exists, transforms Trans for Node.
        if( pMtxSR != NULL )
        {
            vTransF.x = pMtxSR->_00 * vTransS.x + pMtxSR->_10 * vTransS.y;
            vTransF.y = pMtxSR->_01 * vTransS.x + pMtxSR->_11 * vTransS.y;  
            
        }else{
            SVecToFVec( &vTransS, &vTransF );
        }
        
        // Adds to vNodeTrans if pBaseTrans exists.
        if( pBaseTrans != NULL )
        {
            vTransF.x += pBaseTrans->x;
            vTransF.y += pBaseTrans->y;
        }
    }
    
    //
    // Do the constituent elements have an affine conversion?
    // 
    // TODO: Make this into NNS_G2D_WARNING.
    if( NNSi_G2dSrtcIsAffineEnable_SR( pContentsSRT ) )
    {
        // Warning!
        OS_Warning("invalid affine transformation is found in NNS_G2dInitMCAnimation()");
    }
    
    //
    // converts Cell to Oam
    // 
    return NNS_G2dMakeCellToOams( pDstOams,
                                  numDstOams,
                                  NNS_G2dGetCellAnimationCurrentCell( pCellAnim ), 
                                  pMtxSR, 
                                  &vTransF, 
                                  affineIndex,
                                  bDoubleAffine );
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE u16 MakeSimpleMultiCellToOams_
( 
    GXOamAttr*                      pDstOams, 
    u16                             numDstOams,
    const NNSG2dMultiCellInstance*  pMCellInst, 
    const MtxFx22*                  pMtxSR, 
    const NNSG2dFVec2*              pBaseTrans,
    u16                             affineIndex,
    BOOL                            bDoubleAffine 
)
{
    u16     i           = 0;
    u16     numOamUsed  = 0;
    int     numRestOams = numDstOams;
    NNSG2dNode* pNode   = NULL;
    
    NNS_G2D_NULL_ASSERT( pDstOams );
    NNS_G2D_NULL_ASSERT( pMCellInst );
    NNS_G2D_NULL_ASSERT( pMCellInst->pCurrentMultiCell );
    NNS_G2D_ASSERT( pMCellInst->mcType == NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM );
    
    
    //
    // For each node...
    //
    pNode = (NNSG2dNode*)pMCellInst->pCellAnimInstasnces;
    for( i = 0; i < pMCellInst->pCurrentMultiCell->numNodes; i++ )
    {
        // if there is sufficient buffer capacity...
        if( numRestOams > 0 )
        {
            //
            // transform Node to Oam Attrs
            //
            
            //
            // Caution:
            // It is assumed that pNode->type is NNS_G2D_NODETYPE_CELL
            // Does not support rendering multicells whose node SR parameters have changed.
            //
            NNS_G2D_WARNING( pNode->type == NNS_G2D_NODETYPE_CELL &&
                            !NNSi_G2dSrtcIsAffineEnable_SR( &pNode->srtCtrl ),
                            "A SR-Transformation of a multicell-node was ignored." );
                            
            numOamUsed = MakeCellAnimToOams_( pDstOams,
                                              (u16)numRestOams,
                                              pNode[i].pContent,
                                              &pNode[i].srtCtrl.srtData.trans,
                                              pMtxSR,
                                              pBaseTrans,
                                              affineIndex,
                                              bDoubleAffine );
            
            // Updates the write buffer position.
            numRestOams -= numOamUsed;
            pDstOams    += numOamUsed;    
        }else{
            // End of loop.
            break;
        }
    }
    
    NNS_G2D_ASSERT( numDstOams >= numRestOams );
    return (u16)( numDstOams - numRestOams );
}

// 
// sequence of events leading to NNS_G2dMakeSimpleMultiCellToOams()
// 
// Problem
// 
// When the Node or NodeContents of a Multicell has an affine conversion, it is possible for multiple affine conversion matrices to be generated during the conversion process.
// 
// 
// 1. While configuring those matrices in hardware,
// 2, You need to configure affineIndex for the results from 1, for Oam Attrs.
// 
// In the case of both (1) and (2) it is expected that people will want to customize the implementation with client code.
// Normally these cases would use a function pointer and the processing for 1 and 2 would be handed over to external functions. But we then lose sight of the objective of this function, which is to provide a simple, straightforward rendering interface.
// 
// 



//------------------------------------------------------------------------------
static NNS_G2D_INLINE u16 MakeSimpleMultiCellToOams_ShareCellAnims_
( 
    GXOamAttr*                      pDstOams, 
    u16                             numDstOams,
    const NNSG2dMultiCellInstance*  pMCellInst, 
    const MtxFx22*                  pMtxSR, 
    const NNSG2dFVec2*              pBaseTrans,
    u16                             affineIndex,
    BOOL                            bDoubleAffine 
)
{
    u16     i           = 0;
    u16     numOamUsed  = 0;
    int     numRestOams = numDstOams;
    const NNSG2dMCCellAnimation* pMCCellAnimArray = NULL;
    NNSG2dSVec2       nodeTrans;
    
    NNS_G2D_NULL_ASSERT( pDstOams );
    NNS_G2D_NULL_ASSERT( pMCellInst );
    NNS_G2D_NULL_ASSERT( pMCellInst->pCurrentMultiCell );
    NNS_G2D_ASSERT( pMCellInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM );
    
    pMCCellAnimArray = (const NNSG2dMCCellAnimation*)pMCellInst->pCellAnimInstasnces;
    
    //
    // For each node...
    //
    for( i = 0; i < pMCellInst->pCurrentMultiCell->numNodes; i++ )
    {
        const NNSG2dMultiCellHierarchyData* pNode 
           = &pMCellInst->pCurrentMultiCell->pHierDataArray[i];
        
        //
        // TODO:
        //
        // pNode->animSequenceIdx
        // Cell animation numbers associated with a node are stored in pNode->nodeAttr.  
        //
        //
        const u16 cellAnimIdx  = NNSi_G2dGetMC2NodeCellAinmIdx( pNode );
        
        
           
           
        // if there is sufficient buffer capacity...
        if( numRestOams > 0 )
        {
            //
            // transform Node to Oam Attrs
            //
            nodeTrans.x = pNode->posX;
            nodeTrans.y = pNode->posY;
            
            numOamUsed = MakeCellAnimToOams_( pDstOams,
                                              (u16)numRestOams,
                                              &pMCCellAnimArray[cellAnimIdx].cellAnim,
                                              &nodeTrans,
                                              pMtxSR,
                                              pBaseTrans,
                                              affineIndex,
                                              bDoubleAffine );
            
            // Updates the write buffer position.
            numRestOams -= numOamUsed;
            pDstOams    += numOamUsed;   
        }else{
            // End of loop.
            break;
        }
    }
    
    NNS_G2D_ASSERT( numDstOams >= numRestOams );
    return (u16)( numDstOams - numRestOams );
}

/*---------------------------------------------------------------------------*
  Name:         InitMCInstance_

  Description:  Initializes multicell instance.
                Left here for compatibility with previous versions.
                
                
  Arguments:    pMultiCell:         Multicell instance
                pNodeArray:         node instance array ( x numNode )
                pCellAnim:         cell animation instance array ( x numNode )
                numNode:         number of nodes
                pAnimBank:         cell animation definition bank
                pCellDataBank:         Cell data bank
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
static NNS_G2D_INLINE void InitMCInstance_
( 
    NNSG2dMultiCellInstance*           pMultiCell, 
    NNSG2dNode*                        pNodeArray, 
    NNSG2dCellAnimation*               pCellAnim, 
    u16                                numNode, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank 
)
{
    NNS_G2D_NULL_ASSERT( pMultiCell );
    NNS_G2D_NULL_ASSERT( pNodeArray );
    NNS_G2D_NULL_ASSERT( pAnimBank );
    NNS_G2D_ASSERTMSG( numNode != 0, "Non zero value is expected." );
    
    
    //
    // When using this function, a NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM-type instance will be initialized. 
    // 
    // The pCellAnimInstansces member in the NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM-type instance will indicate the first entry in the node array.
    // 
    //
    pMultiCell->mcType              = NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM;
    pMultiCell->pCurrentMultiCell   = NULL;
    pMultiCell->pAnimDataBank       = pAnimBank;
    pMultiCell->pCellAnimInstasnces = (void*)pNodeArray;
    
    {
        u16 i;
        for( i = 0; i < numNode; i++ ) 
        {
            NNSi_G2dInitializeNode( &pNodeArray[i], NNS_G2D_NODETYPE_CELL );
            // TODO: Make bind into a function.
            // bind
            pNodeArray[i].pContent  = &pCellAnim[i];
            
            
            NNS_G2dInitCellAnimation( 
                &pCellAnim[i], 
                NNS_G2dGetAnimSequenceByIdx( pAnimBank, 0 ) , 
                pCellDataBank );
        }
    }
}

//------------------------------------------------------------------------------
// Initializes multicell animation.
// This will be run when NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM is set as the mcType.
//
// If you specify NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM as an mcType, the instance will initialized with the same data structure as for past versions, and will be processed the same way.
// 
// As long as the programmer does not access the nodes that comprise a multicell or attempt to overwrite data, etc., initializing an instance with a NNS_G2D_MCTYPE_SHARE_CELLANIM specification is better in terms of memory consumption and processing efficiency.
// 
//
static NNS_G2D_INLINE void InitMCAnimation_( 
    NNSG2dMultiCellAnimation*          pMultiCellAnim, 
    NNSG2dNode*                        pNodeArray, 
    NNSG2dCellAnimation*               pCellAnim, 
    u16                                numNode, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank 
)
{
#pragma unused( pMultiCellDataBank )
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    
    NNS_G2D_NULL_ASSERT( pNodeArray );
    NNS_G2D_NULL_ASSERT( pCellAnim );
    NNS_G2D_ASSERTMSG( numNode != 0, "TODO: msg" );
    
    NNS_G2D_NULL_ASSERT( pAnimBank );
    NNS_G2D_NULL_ASSERT( pCellDataBank );
    
    NNS_G2D_NULL_ASSERT( pMultiCellDataBank );
    
    //
    // initializes the multicell instance
    //
    InitMCInstance_( &pMultiCellAnim->multiCellInstance, 
                            pNodeArray, 
                            pCellAnim, 
                            numNode, 
                            pAnimBank,  
                            pCellDataBank );
    
    NNS_G2dInitAnimCtrl( &pMultiCellAnim->animCtrl );
    pMultiCellAnim->pMultiCellDataBank = pMultiCellDataBank;
    NNSi_G2dSrtcInitControl( &pMultiCellAnim->srtCtrl, NNS_G2D_SRTCONTROLTYPE_SRT );
    pMultiCellAnim->totalVideoFrame = 0;
}

//------------------------------------------------------------------------------
// Initializes multicell animation.
// This will be run when NNS_G2D_MCTYPE_SHARE_CELLANIM is set as mcType.
// 
// When NNS_G2D_MCTYPE_SHARE_CELLANIM has been specified as an mcType, and when data has been entered that would play the same animation sequence in multiple nodes in the multicell, cell animation instances are shared among multiple nodes.
// 
//  This will decrease memory consumption and animation update processing overhead.
//
// If NNS_G2D_MCTYPE_SHARE_CELLANIM is specified, the multicell runtime instance will not hold information that corresponds to nodes.
// Therefore it will not support position changes or affine transformations at node level.
// However, since it is assured that, as a result of the above restrictions, the nodes playing the same cell animation will always refer to the same affine parameters, it will be possible to conserve hardware resources (i.e., the affine parameters used), compared to the results when NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM is specified, particularly when rendering using the 2D graphics engine. 
// 
// 
//
static NNS_G2D_INLINE void InitMCAnimation_SharingCellAnim_( 
    NNSG2dMultiCellAnimation*          pMultiCellAnim,  
    void*                              pWork, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank 
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    NNS_G2D_NULL_ASSERT( pWork );    
    NNS_G2D_NULL_ASSERT( pAnimBank );
    NNS_G2D_NULL_ASSERT( pCellDataBank );
    NNS_G2D_NULL_ASSERT( pMultiCellDataBank );
    
    //
    // initializes the multicell instance
    //
    pMultiCellAnim->multiCellInstance.mcType = NNS_G2D_MCTYPE_SHARE_CELLANIM;
    {
       NNSG2dMultiCellInstance* pMCInst = &pMultiCellAnim->multiCellInstance;
       
       pMCInst->pAnimDataBank = pAnimBank;
       // Initializes work memory.
       // Uses work memory as NNSG2dMCCellAnimation.
       pMCInst->pCellAnimInstasnces = (void*)pWork;

       {
           u16 i;
           const u16 numCellAnim 
              = GetMCBankNumCellAnimRequired_( pMultiCellDataBank );
           NNSG2dMCCellAnimation* pMCCellAnim 
              = (NNSG2dMCCellAnimation*)pMCInst->pCellAnimInstasnces;
            
           //
           // Tentatively allocate appropriate animations for all cell animations.
           // 
           //   
           for( i = 0; i < numCellAnim; i++ ) 
           {
              NNS_G2dInitCellAnimation( 
                  &pMCCellAnim[i].cellAnim, 
                  NNS_G2dGetAnimSequenceByIdx( pAnimBank, 0 ) , 
                  pCellDataBank );
                    
              pMCCellAnim[i].bInited = TRUE;
           }
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetAnimSequenceToMCAnimation

  Description:  Sets the playback animation to the multicell animation instance.
                This needs to be called after initialization.
                
  Arguments:    pMultiCellAnim:         [OUT] cell animation instance
                pAnimSeq:         [IN] playback animation
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
// TODO: Function group for checking whether each type of resource is in the correct state; run these in asserts.
void NNS_G2dSetAnimSequenceToMCAnimation
( 
    NNSG2dMultiCellAnimation*         pMultiCellAnim, 
    const NNSG2dAnimSequence*         pAnimSeq
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    NNS_G2D_NULL_ASSERT( pMultiCellAnim->pMultiCellDataBank );
    
    NNS_G2D_NULL_ASSERT( pAnimSeq );
    NNS_G2D_ASSERTMSG( NNS_G2dGetAnimSequenceAnimType( pAnimSeq ) 
                   == NNS_G2D_ANIMATIONTYPE_MULTICELLLOCATION, 
                    "NNSG2dAnimationType must be MultiCellLocation." );
    
    NNS_G2dBindAnimCtrl( &pMultiCellAnim->animCtrl, pAnimSeq );
    
    // Resets the multicell playback total frame time.    
    pMultiCellAnim->totalVideoFrame = 0;
    
    ApplyCurrentAnimResult_( pMultiCellAnim );
}




/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitMCAnimationInstance

  Description:  Initializes the NNSG2dMC2Animation instance.
  
                This function is available as a substitute for the NNS_G2dInitMCAnimation, NNS_G2dInitMCInstance, and NNS_G2dSetMCDataToMCInstance functions.
                         
                         
                These old functions remain with separate names to preserve compatibility with previous functions.

                Please use this function for new usages. 
                
  Arguments:    pMultiCellAnim      [OUT] multicell animation 
                pWork               [IN] work space used by a multicell instance
                pAnimBank           [IN] animation bank that defines the cell animations that constitute a multicell 
                pCellDataBank       [IN] cell data bank 
                pMultiCellDataBank  [IN] multicell data bank 
                mcType              [IN] multicell instance type (NNSG2dMCType type)

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitMCAnimationInstance
( 
    NNSG2dMultiCellAnimation*          pMultiCellAnim,  
    void*                              pWork, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank,
    NNSG2dMCType                       mcType
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    NNS_G2D_NULL_ASSERT( pWork );    
    NNS_G2D_NULL_ASSERT( pAnimBank );
    NNS_G2D_NULL_ASSERT( pCellDataBank );
    NNS_G2D_NULL_ASSERT( pMultiCellDataBank );
    
    
    //
    // initialization by instance type
    //
    if( mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
       InitMCAnimation_SharingCellAnim_( pMultiCellAnim,
                                         pWork,
                                         pAnimBank,
                                         pCellDataBank,
                                         pMultiCellDataBank );
    }else{
       const u16 numNode 
           = NNS_G2dGetMCBankNumNodesRequired( pMultiCellDataBank );
       NNSG2dNode* pNodeArray 
           = pWork;
       NNSG2dCellAnimation* pCellAnimArray 
           = (NNSG2dCellAnimation*)(pNodeArray + numNode);
                 
       InitMCAnimation_( pMultiCellAnim, 
                         pNodeArray, 
                         pCellAnimArray, 
                         numNode, 
                         pAnimBank,  
                         pCellDataBank,
                         pMultiCellDataBank );
    }
    
    //
    // initialize common area
    //
    NNS_G2dInitAnimCtrl( &pMultiCellAnim->animCtrl );
    pMultiCellAnim->pMultiCellDataBank = pMultiCellDataBank;
    NNSi_G2dSrtcInitControl( &pMultiCellAnim->srtCtrl, NNS_G2D_SRTCONTROLTYPE_SRT );
    pMultiCellAnim->totalVideoFrame = 0;
    
}






/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetMCNumNodesRequired

  Description:  Gets the maximum NNSG2dNode number needed in NNSG2dMultiCellAnimSequence.
                
                Currently, there is no need to use this function, as the new initialization function, NNS_G2dInitMCAnimationInstance, has been added.
                
                
  Arguments:    pMultiCellSeq:          [IN] multicell animation
                pMultiCellDataBank:     [IN] multicell databank
                
                
  Returns:      maximum NNSG2dNode number needed in the multicell animation
  
 *---------------------------------------------------------------------------*/
u16 NNS_G2dGetMCNumNodesRequired
( 
    const NNSG2dMultiCellAnimSequence*   pMultiCellSeq, 
    const NNSG2dMultiCellDataBank*       pMultiCellDataBank 
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellSeq );
    NNS_G2D_NULL_ASSERT( pMultiCellDataBank );
    NNS_G2D_ASSERTMSG( NNS_G2dGetAnimSequenceAnimType( pMultiCellSeq )
                   == NNS_G2D_ANIMATIONTYPE_MULTICELLLOCATION, 
                    "NNSG2dAnimationType must be MultiCellLocation." );
    
    {   
        const NNSG2dMultiCellData*  pMCell        = NULL;
        u16                         maxNumNode    = 0;
        
        //
        // Searches for the maximum NNSG2dNode number in the multicell series constituting the animation.
        // 
        u16     i;
        const NNSG2dAnimDataSRT*    pAnmFrm;
        for( i = 0; i < pMultiCellSeq->numFrames; i++ )
        {
            pAnmFrm = (const NNSG2dAnimDataSRT*)
               pMultiCellSeq->pAnmFrameArray[i].pContent;
            
            
            pMCell = NNS_G2dGetMultiCellDataByIdx( pMultiCellDataBank, 
                                                   pAnmFrm->index );
            NNS_G2D_NULL_ASSERT( pMCell );
            
            
            if( pMCell->numNodes > maxNumNode )
            {
                maxNumNode = pMCell->numNodes;
            }
        }
        return maxNumNode;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetMCWorkAreaSize

  Description:  Gets the size of the work memory that is required to initialize an instance of a multicell.
                NNS_G2dGetMCNumNodesRequired
                NNS_G2dGetMCBankNumNodesRequired
                NNS_G2dGetMCBankNumCellAnimsRequired
                In place of these functions, it gets the required work size in bytes.
                
  Arguments:    pMultiCellDataBank:     [IN] multicell databank
                mcType:                 [IN] type of multicell instance
                
  Returns:      maximum number of NNSG2dNode required in a multicell data bank
  
 *---------------------------------------------------------------------------*/
u32 NNS_G2dGetMCWorkAreaSize
(
    const NNSG2dMultiCellDataBank*       pMultiCellDataBank,
    NNSG2dMCType                         mcType
)
{
    if( mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
       return sizeof( NNSG2dMCCellAnimation ) 
           * GetMCBankNumCellAnimRequired_( pMultiCellDataBank );
    }else{
       return ( sizeof( NNSG2dNode ) + sizeof( NNSG2dCellAnimation ) ) 
           * NNS_G2dGetMCBankNumNodesRequired( pMultiCellDataBank );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetMCBankNumNodesRequired

  Description:  Gets the maximum number of NNSG2dNode needed in NNSG2dMultiCellDataBank.
                
                Currently, there is no need to use this function, as the new initialization function, NNS_G2dInitMCAnimationInstance, has been added.
                
                
  Arguments:    pMultiCellDataBank:     [IN] multicell databank
                
                
  Returns:      maximum number of NNSG2dNode required in a multicell data bank
  
 *---------------------------------------------------------------------------*/
u16 NNS_G2dGetMCBankNumNodesRequired
( 
    const NNSG2dMultiCellDataBank*       pMultiCellDataBank 
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellDataBank );    
    {   
        const NNSG2dMultiCellData*  pMCell        = NULL;
        u16                         maxNumNode    = 0;
        u16                         i;
        
        for( i = 0; i < pMultiCellDataBank->numMultiCellData; i++ )
        {
            pMCell = NNS_G2dGetMultiCellDataByIdx( pMultiCellDataBank, i );
            NNS_G2D_NULL_ASSERT( pMCell );
            
            if( pMCell->numNodes > maxNumNode )
            {
                maxNumNode = pMCell->numNodes;
            }
        }
        return maxNumNode;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTickMCInstance

  Description:  Advances the time for the elements (such as cell animation) constituting the multicell.
                
  Arguments:    pMultiCellAnim:     [OUT] multicell instance
                frames:             [IN] time to advance (in frames)
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dTickMCInstance( NNSG2dMultiCellInstance* pMCellInst, fx32 frames )
{

    if( pMCellInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
       u16 i;
       NNSG2dMCCellAnimation* pCellAnimArray = pMCellInst->pCellAnimInstasnces;
       
       for( i = 0; i < pMCellInst->pCurrentMultiCell->numCellAnim; i++ )
       {
           NNS_G2dTickCellAnimation( &pCellAnimArray[i].cellAnim, frames );
       }
    }else{    
       u16 i;
       NNSG2dNode*   pNodeArray = NULL;
        
       NNS_G2D_NULL_ASSERT( pMCellInst );
       NNS_G2D_NULL_ASSERT( pMCellInst->pCurrentMultiCell );
        
       pNodeArray = (NNSG2dNode*)(pMCellInst->pCellAnimInstasnces);
        
       for( i = 0; i < pMCellInst->pCurrentMultiCell->numNodes; i++ )
       {
            
           // Currently only NNS_G2D_NODETYPE_CELL is supported.
           NNS_G2D_ASSERT( pNodeArray[i].type == NNS_G2D_NODETYPE_CELL );
           NNS_G2dTickCellAnimation( (NNSG2dCellAnimation*)pNodeArray[i].pContent, frames );
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTickMCAnimation

  Description:  Advances the time of NNSG2dMultiCellAnimation.
                
  Arguments:    pMultiCellAnim:     [OUT] multicell instance
                frames:             [IN] time to advance (in frames)
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dTickMCAnimation( NNSG2dMultiCellAnimation* pMultiCellAnim, fx32 frames )
{
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    
    {
        const u16 currentAnimFrameFrames = pMultiCellAnim->animCtrl.pCurrent->frames;
        
        if( NNS_G2dTickAnimCtrl( &pMultiCellAnim->animCtrl, frames ) )
        {
           pMultiCellAnim->totalVideoFrame += currentAnimFrameFrames;
           //
           // Reflects the animation results if there has been a Track update.
           //
           ApplyCurrentAnimResult_( pMultiCellAnim );
        }else{
           // Updates the cell animation in the multicell.
           // Note that this does not run in the frame that results in the multicell animation frame switching.
           // 
           NNS_G2dTickMCInstance( &pMultiCellAnim->multiCellInstance, frames );
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetMCAnimationCurrentFrame

  Description:  Sets the playback animation frame for NNS_G2dSetMCAnimation.
                
  Arguments:    pMultiCellAnim:     [OUT] multicell instance
                frameIndex:         [IN] setting frame number
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetMCAnimationCurrentFrame
( 
    NNSG2dMultiCellAnimation*   pMultiCellAnim, 
    u16                         frameIndex 
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    
    if( NNS_G2dSetAnimCtrlCurrentFrame( &pMultiCellAnim->animCtrl, frameIndex ) )
    {
        //
        // Reflects the animation results if there has been a Track update.
        //
        ApplyCurrentAnimResult_( pMultiCellAnim );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetMCAnimationCellAnimFrame

  Description:  

                Configures animation frames for each of the cell animations that comprise a multicell.
                
                
                
  Arguments:    pMultiCellAnim:         [OUT] multicell instance
                caFrameIndex:           [IN] cell animation frame number    
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetMCAnimationCellAnimFrame
( 
    NNSG2dMultiCellAnimation*   pMultiCellAnim, 
    u16                         caFrameIndex     
)
{
    u16 i;
    NNSG2dMultiCellInstance* pMCInst = NULL;
        
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );            
    //
    // Sets the animation frames for all cell animations.       
    // 
    pMCInst = &pMultiCellAnim->multiCellInstance;
    
    // The data structure will differ depending on the parameters at the time of initialization.
    if( pMCInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
        NNSG2dMCCellAnimation* pCellAnimArray 
           = (NNSG2dMCCellAnimation*)pMCInst->pCellAnimInstasnces;
        const int numCellAnm = pMCInst->pCurrentMultiCell->numCellAnim;
                      
        for( i = 0; i < numCellAnm; i++ )
        {
           NNSG2dCellAnimation* pCell 
           = &pCellAnimArray[i].cellAnim;
           NNS_G2dSetCellAnimationCurrentFrame( pCell, caFrameIndex );
        }
    }else{
        NNSG2dNode*   pNodeArray   
           = (NNSG2dNode*)(pMCInst->pCellAnimInstasnces);
        const int numCellAnm = pMCInst->pCurrentMultiCell->numNodes;
           
        for( i = 0; i < numCellAnm; i++ )
        {
           NNSG2dCellAnimation* pCell 
           = (NNSG2dCellAnimation*)pNodeArray[i].pContent;
           NNS_G2dSetCellAnimationCurrentFrame( pCell, caFrameIndex );
        }
    }
}




/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetMCAnimationSpeed

  Description:  Sets the playback animation speed for NNS_G2dSetMCAnimation.
                
  Arguments:    pMultiCellAnim:     [OUT] multicell instance
                speed:              [IN] animation speed
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetMCAnimationSpeed
(
    NNSG2dMultiCellAnimation*   pMultiCellAnim, 
    fx32                        speed  
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    {
       NNSG2dMultiCellInstance* pMCellInst     = &pMultiCellAnim->multiCellInstance;
        
       if( pMCellInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
       {
           u16 i;
           
           NNSG2dMCCellAnimation*     pCellAnimArray = pMCellInst->pCellAnimInstasnces;
           NNS_G2dSetAnimCtrlSpeed( &pMultiCellAnim->animCtrl, speed );
           
           for( i = 0; i < pMCellInst->pCurrentMultiCell->numCellAnim; i++ )
           {
              NNS_G2dSetCellAnimationSpeed( &pCellAnimArray[i].cellAnim, speed );
           }
           
       }else{    
           u16 i = 0;
           NNSG2dNode*              pNode      = (NNSG2dNode*)pMultiCellAnim->multiCellInstance.pCellAnimInstasnces;
           
           NNS_G2D_NULL_ASSERT( pMultiCellAnim );
            
            
           NNS_G2dSetAnimCtrlSpeed( &pMultiCellAnim->animCtrl, speed );
            
           // Cell animation speed also changes.
           for( i = 0; i < pMCellInst->pCurrentMultiCell->numNodes; i++ )
           {
              // Currently only NNS_G2D_NODETYPE_CELL is supported.
              NNS_G2D_ASSERT( pNode[i].type == NNS_G2D_NODETYPE_CELL );
              NNS_G2dSetCellAnimationSpeed( (NNSG2dCellAnimation*)pNode[i].pContent, speed );
           }
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dResetMCCellAnimationAll

  Description:  Sets the cell animation play sequence in a multicell animation to zero.
                This will not re-commence playback of cell animation for which playback is stopped.
                
                
  Arguments:    pMultiCellAnim:      [OUT] multicell instance
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dResetMCCellAnimationAll
(
    NNSG2dMultiCellInstance*    pMCInst 
)
{
    u16 i = 0;
    NNS_G2D_NULL_ASSERT( pMCInst );
    {
       if( pMCInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
       {
           NNSG2dMCCellAnimation*     pCellAnimArray = pMCInst->pCellAnimInstasnces;
           
           for( i = 0; i < pMCInst->pCurrentMultiCell->numCellAnim; i++ )
           {
              NNS_G2dSetCellAnimationCurrentFrame( &pCellAnimArray[i].cellAnim, 0 );
           }
           
       }else{    
           
           NNSG2dNode*              pNode      = (NNSG2dNode*)pMCInst->pCellAnimInstasnces;
                 
           
           for( i = 0; i < pMCInst->pCurrentMultiCell->numNodes; i++ )
           {
              // Currently only NNS_G2D_NODETYPE_CELL is supported.
              NNS_G2D_ASSERT( pNode[i].type == NNS_G2D_NODETYPE_CELL );
              NNS_G2dSetCellAnimationCurrentFrame( (NNSG2dCellAnimation*)pNode[i].pContent, 0 );
           }
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dStartMCCellAnimationAll

  Description:  Starts all of the cell animations in a multicell animation.
                
                
  Arguments:    pMultiCellAnim:      [OUT] multicell instance
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dStartMCCellAnimationAll
(
    NNSG2dMultiCellInstance*    pMCInst 
)
{
    u16 i = 0;
    NNSG2dAnimController*    pAnmCtrl   = NULL;
    NNS_G2D_NULL_ASSERT( pMCInst );
    {
       if( pMCInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
       {
           NNSG2dMCCellAnimation*     pCellAnimArray = pMCInst->pCellAnimInstasnces;
           
           for( i = 0; i < pMCInst->pCurrentMultiCell->numCellAnim; i++ )
           {
              pAnmCtrl = NNS_G2dGetCellAnimationAnimCtrl( &pCellAnimArray[i].cellAnim );
              
              NNS_G2dStartAnimCtrl( pAnmCtrl );
           }
           
       }else{    
           
           NNSG2dNode*              pNode      = (NNSG2dNode*)pMCInst->pCellAnimInstasnces;
                 
           
           for( i = 0; i < pMCInst->pCurrentMultiCell->numNodes; i++ )
           {
              // Currently only NNS_G2D_NODETYPE_CELL is supported.
              NNS_G2D_ASSERT( pNode[i].type == NNS_G2D_NODETYPE_CELL );
              pAnmCtrl = NNS_G2dGetCellAnimationAnimCtrl( (NNSG2dCellAnimation*)pNode[i].pContent );
              
              NNS_G2dStartAnimCtrl( pAnmCtrl );
           }
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRestartMCAnimation

  Description:  Restarts the cell animation playbacks within a multicell animation from the start of the sequence.
                 
                 When playing in reverse, playback commences from the sequence end.
                
  Arguments:    pMCInst:      [OUT] multicell instance
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRestartMCAnimation
(
    NNSG2dMultiCellAnimation*   pMultiCellAnim
)
{
    u16                     i             = 0;
    NNSG2dMultiCellInstance* pMCInst      = &pMultiCellAnim->multiCellInstance;
    NNSG2dAnimController*    pMCAnmCtrl   = NNS_G2dGetMCAnimAnimCtrl( pMultiCellAnim );
    
    
    // Resets the video frame total.
    pMultiCellAnim->totalVideoFrame = 0;
    
    // Restarts the animation controller of a multicell animation.
    NNS_G2dSetAnimSequenceToMCAnimation( pMultiCellAnim, 
                                         NNS_G2dGetAnimCtrlCurrentAnimSequence( pMCAnmCtrl ) );    
    NNS_G2dStartAnimCtrl( pMCAnmCtrl );
    
    
    // Restarts the cell animation that makes up a multicell.
    NNS_G2D_NULL_ASSERT( pMCInst );
    {
       if( pMCInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
       {
           NNSG2dMCCellAnimation*     pCellAnimArray = pMCInst->pCellAnimInstasnces;
           
           for( i = 0; i < pMCInst->pCurrentMultiCell->numCellAnim; i++ )
           {
              // Restarts the playback.
              NNS_G2dRestartCellAnimation( &pCellAnimArray[i].cellAnim );
           }
           
       }else{    
           
           NNSG2dNode*              pNode      = (NNSG2dNode*)pMCInst->pCellAnimInstasnces;
                 
           for( i = 0; i < pMCInst->pCurrentMultiCell->numNodes; i++ )
           {
              // Currently only NNS_G2D_NODETYPE_CELL is supported.
              NNS_G2D_ASSERT( pNode[i].type == NNS_G2D_NODETYPE_CELL );
              // Restarts the playback.
              NNS_G2dRestartCellAnimation( (NNSG2dCellAnimation*)pNode[i].pContent );
           }
       }
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMakeSimpleMultiCellToOams

  Description:  Generates OAM information.
                This can only be run for simple multicell instances, whose component CellAnimation (MultiCellAnimation) elements do not include affine conversion.
                
  
                A warning is displayed when an invalid affine conversion is discovered within a multicell, and processing continues, ignoring the conversion.
                
                (Therefore, rendering results will not reflect it.)
                 
                The entire MultiCell can be affine-converted.              
                If that is done, though, OBJs with flip effects will not be drawn correctly.  
                
                
                
  Arguments:    pDstOams:             [IN] result buffer
                numDstOams:             [IN] result buffer length
                pMCellInst:             [OUT] multicell instance
                pMtxSR:             [IN] affine matrix (3D format)
                pBaseTrans:             [IN] translation component
                affineIndex:             [IN] affine Index referenced by the multicell constituent Oam
                bDoubleAffine:            [IN] Double affine?
                
                
  Returns:      number of result buffers actually used
  
 *---------------------------------------------------------------------------*/
 
u16 NNS_G2dMakeSimpleMultiCellToOams
( 
    GXOamAttr*                      pDstOams, 
    u16                             numDstOams,
    const NNSG2dMultiCellInstance*  pMCellInst, 
    const MtxFx22*                  pMtxSR, 
    const NNSG2dFVec2*              pBaseTrans,
    u16                             affineIndex,
    BOOL                            bDoubleAffine 
)
{
    if( pMCellInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
        return MakeSimpleMultiCellToOams_ShareCellAnims_( pDstOams, 
                              numDstOams, 
                              pMCellInst, 
                              pMtxSR, 
                              pBaseTrans, 
                              affineIndex, 
                              bDoubleAffine );
    }else{
        return MakeSimpleMultiCellToOams_( pDstOams, 
                                    numDstOams, 
                                    pMCellInst, 
                                    pMtxSR, 
                                    pBaseTrans, 
                                    affineIndex, 
                                    bDoubleAffine );
    }    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dMakeSimpleMultiCellToOams

  Description:  While traversing the cell animations that comprise a multicell, each cell animation is used as an argument when calling a callback function.
                
                
                
                
  Arguments:    pMCellInst:             [IN] multicell instance
                pCBFunc:             [IN} Callback function called for each cell animation within the multicell
                                               
                userParamater:           [IN] First argument passed to the callback function; to be used as the user desires.  
                                               
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
 
void NNS_G2dTraverseMCCellAnims
( 
    NNSG2dMultiCellInstance*         pMCellInst,
    NNSG2dMCTraverseCellAnimCallBack pCBFunc,
    u32                              userParamater
)
{
    BOOL                         bContinue = TRUE;
    if( pMCellInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
       u16 i;
       NNSG2dMCCellAnimation* pCellAnimArray = pMCellInst->pCellAnimInstasnces;
       
       for( i = 0; i < pMCellInst->pCurrentMultiCell->numCellAnim; i++ )
       {
           bContinue = (*pCBFunc)( userParamater, &pCellAnimArray[i].cellAnim, i );
           
           if( !bContinue )
           {
              break;
           }
       }
    }else{    
       u16 i;
       NNSG2dNode*   pNodeArray = NULL;
        
       NNS_G2D_NULL_ASSERT( pMCellInst );
       NNS_G2D_NULL_ASSERT( pMCellInst->pCurrentMultiCell );
        
       pNodeArray = (NNSG2dNode*)(pMCellInst->pCellAnimInstasnces);
        
       for( i = 0; i < pMCellInst->pCurrentMultiCell->numNodes; i++ )
       { 
           // Currently only NNS_G2D_NODETYPE_CELL is supported.
           NNS_G2D_ASSERT( pNodeArray[i].type == NNS_G2D_NODETYPE_CELL );
           bContinue = (*pCBFunc)( userParamater, (NNSG2dCellAnimation*)pNodeArray[i].pContent, i );           
           
           if( !bContinue )
           {
              break;
           }
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTraverseMCNodes

  Description:  While traversing the nodes that comprise a multicell, each node, cell animation and node number are used as arguments when calling a callback function.
                
                
                
                
  Arguments:    pMCellInst:             [IN] multicell instance
                pCBFunc:             [IN} Callback function called for each cell animation within the multicell
                                                 
                userParamater:             [IN] First argument passed to the callback function; to be used as the user desires.  
                                                 
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
 
void NNS_G2dTraverseMCNodes
( 
    NNSG2dMultiCellInstance*        pMCellInst,
    NNSG2dMCTraverseNodeCallBack    pCBFunc,
    u32                             userParamater
)
{
    NNS_G2D_NULL_ASSERT( pMCellInst );
    NNS_G2D_NULL_ASSERT( pMCellInst->pCurrentMultiCell );    
    {
        u16 i;
        const NNSG2dMultiCellData*   pMCData = pMCellInst->pCurrentMultiCell;
        const u16                    numNode = pMCData->numNodes;
        BOOL                         bContinue = TRUE;
        
        if( pMCellInst->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
        {
           NNSG2dMCCellAnimation* pMCCellAnimArray
               = (NNSG2dMCCellAnimation*)pMCellInst->pCellAnimInstasnces;
           //
           // while traversing each Node...
           //
           for( i = 0; i < numNode; i++ )
           {
               const NNSG2dMultiCellHierarchyData* pNodeData = &pMCData->pHierDataArray[i];
               // Cell animation numbers associated with a node are stored in pNode->nodeAttr.  
               const u16 cellAnimIdx  = NNSi_G2dGetMC2NodeCellAinmIdx( pNodeData );
                
               // callback call
               bContinue = (*pCBFunc)( userParamater,
                                       pNodeData,                                 // node data
                                       &pMCCellAnimArray[cellAnimIdx].cellAnim,   // Cell animation
                                       i );                                       // Node number                 
               if( !bContinue )
               {
                  break;
               }
           }
        }else{ 
           const NNSG2dNode* pNode 
               = (const NNSG2dNode*)pMCellInst->pCellAnimInstasnces;
           //
           // while traversing each node...
           //
           for( i = 0; i < numNode; i++ )
           {
               const NNSG2dMultiCellHierarchyData* pNodeData = &pMCData->pHierDataArray[i];
               // pNode->animSequenceIdx
               // Cell animation numbers associated with a node are stored in pNode->nodeAttr.  
               //
               //
               // callback call
               bContinue = (*pCBFunc)( userParamater,
                                       pNodeData,           // node data
                                       pNode[i].pContent,   // Cell animation
                                       i );                 // Node number            
               
               if( !bContinue )
               {
                  break;
               }
           }
        }
    }
}
//------------------------------------------------------------------------------
//
// This function has been changed to a private function.
// The old API is left here for compatibility with previous versions.
//
void NNS_G2dInitMCAnimation( 
    NNSG2dMultiCellAnimation*          pMultiCellAnim, 
    NNSG2dNode*                        pNodeArray, 
    NNSG2dCellAnimation*               pCellAnim, 
    u16                                numNode, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank 
)
{
    InitMCAnimation_( pMultiCellAnim, 
                      pNodeArray, 
                      pCellAnim, 
                      numNode, 
                      pAnimBank, 
                      pCellDataBank, 
                      pMultiCellDataBank );
}
//
// This function has been changed to a private function.
// The old API is left here for compatibility with previous versions.
//
void NNS_G2dInitMCInstance
( 
    NNSG2dMultiCellInstance*           pMultiCell, 
    NNSG2dNode*                        pNodeArray, 
    NNSG2dCellAnimation*               pCellAnim, 
    u16                                numNode, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank 
)
{
    InitMCInstance_
    ( 
       pMultiCell, 
       pNodeArray, 
       pCellAnim, 
       numNode, 
       pAnimBank,  
       pCellDataBank 
    );
}


//------------------------------------------------------------------------------
//
// This function has been changed to a private function.
// The old API is left here for compatibility with previous versions.
//
BOOL NNS_G2dSetMCDataToMCInstance
( 
    NNSG2dMultiCellInstance*       pMCInst, 
    const NNSG2dMultiCellData*     pMcData 
)
{
    SetMCDataToMCInstanceImpl_( pMCInst, pMcData, 0 );

    // Kept to maintain API compatibility; no practical meaning.   
    //  
    return TRUE;
}



//------------------------------------------------------------------------------


