/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_MultiCellAnimation.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_G2D_MULTICELLANIMATION_H_
#define NNS_G2D_MULTICELLANIMATION_H_

#include <nitro.h>
#include <nnsys/g2d/fmt/g2d_MultiCell_data.h>

#include <nnsys/g2d/g2d_Node.h>
#include <nnsys/g2d/g2d_Animation.h>
#include <nnsys/g2d/g2d_CellAnimation.h>

#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
// Exactly the same format, but defined with separate name for ease of understanding.
typedef NNSG2dAnimSequence            NNSG2dMultiCellAnimSequence;
typedef NNSG2dAnimBankData            NNSG2dMultiCellAnimBankData;


//
// This function is called while scanning all cell animations in a multicell.
// If the callback invocation is cancelled, returns FALSE.
//
typedef BOOL (*NNSG2dMCTraverseCellAnimCallBack)
( 
    u32                   userParamater,
    NNSG2dCellAnimation*  pCellAnim, 
    u16                   cellAnimIdx 
);    
//
// This function is called while scanning all cell animations in a multicell.
// If the callback invocation is cancelled, returns FALSE.
//
typedef BOOL (*NNSG2dMCTraverseNodeCallBack)
( 
    u32                                 userParamater,
    const NNSG2dMultiCellHierarchyData* pNodeData,
    NNSG2dCellAnimation*                pCellAnim, 
    u16                                 nodeIdx 
);    


//
// Aliases of functions with names changed
// Previous functions declared as aliases to preserve compatibility.
// 
#define NNS_G2dInitializeMCAnimation       NNS_G2dInitMCAnimation
#define NNS_G2dInitializeMCInstance        NNS_G2dInitMCInstance
#define NNS_G2dSetMCAnimSpeed              NNS_G2dSetMCAnimationSpeed

typedef enum 
{
    NNS_G2D_MCTYPE_DONOT_SHARE_CELLANIM,
    NNS_G2D_MCTYPE_SHARE_CELLANIM
    
}NNSG2dMCType;

typedef struct NNSG2dMCNodeArray
{
    NNSG2dNode*                         pNodeArray;         // Node array
    u16                                 numNode;            // Number of Nodes
    u16                                 pad16_;             // Padding
    
}NNSG2dMCNodeArray;

typedef struct NNSG2dMCCellAnimation
{
    NNSG2dCellAnimation     cellAnim;
    BOOL                    bInited;
    
}NNSG2dMCCellAnimation;


typedef struct NNSG2dMCNodeCellAnimArray
{
    NNSG2dMCCellAnimation*       cellAnimArray;
    
}NNSG2dMCNodeCellAnimArray;

//------------------------------------------------------------------------------
// Note: pCurrentMultiCell must satisfy ( numNode > pCurrentMultiCell->numNodes ).
//       
//       However, in most cases, numNode == pCurrentMultiCell->numNodes.
//
//       Alternate concept of the previous version of ComposedObj.
typedef struct NNSG2dMultiCellInstance 
{
    const NNSG2dMultiCellData*          pCurrentMultiCell;  // Multicell data
    
    const NNSG2dCellAnimBankData*       pAnimDataBank;      // Make MultiCell. 
                                                            // NNSG2dCellAnimation is defined. 
                                                            // NNSG2dAnimBankData
    
    //
    // The structure of the internal data instance is different depending on the initialization condition.
    // It is either NNSG2dMCNodeArray or NNSG2dMCNodeCellAnimArray
    //
    //
    NNSG2dMCType                        mcType;
    void*                               pCellAnimInstasnces;
    
    /*
    NNSG2dNode*                         pNodeArray;         // Node array
    u16                                 numNode;            // Number of nodes
    u16                                 pad16_;             // Padding
    */
    
}NNSG2dMultiCellInstance;



//------------------------------------------------------------------------------
// animCtrl results hold information about the index to the NNSG2dMultiCellData.
// The library gets the pointer to NNSG2dMultiCellData from the database with that index as the key.
// After that, sets the obtained NNSG2dMultiCellData in NNSG2dMultiCellInstance.
typedef struct NNSG2dMultiCellAnimation
{
    NNSG2dAnimController              animCtrl;             // Animation controller
    
    u16                               totalVideoFrame;      // Total video frames
    u16                               pad16;                // Padding
    
    NNSG2dMultiCellInstance           multiCellInstance;    // Multicell instance
    
    const NNSG2dMultiCellDataBank*    pMultiCellDataBank;   // Multicell data bank
    
    NNSG2dSRTControl                  srtCtrl;              // SRT animation results
    
    
    // TODO: Bounding volume
}NNSG2dMultiCellAnimation;




//------------------------------------------------------------------------------
// initialization related
//------------------------------------------------------------------------------


//------------------ NEW ------------------------------
void NNS_G2dInitMCAnimationInstance
( 
    NNSG2dMultiCellAnimation*          pMultiCellAnim,  
    void*                              pWork, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank,
    NNSG2dMCType                       mcType
);




//------------------------------------------------------------------------------
void NNS_G2dSetAnimSequenceToMCAnimation
( 
    NNSG2dMultiCellAnimation*             pMultiCellAnim, 
    const NNSG2dMultiCellAnimSequence*    pAnimSeq 
);
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
u16 NNS_G2dGetMCNumNodesRequired
( 
    const NNSG2dMultiCellAnimSequence*    pMultiCellSeq, 
    const NNSG2dMultiCellDataBank*        pMultiCellDataBank 
);
u16 NNS_G2dGetMCBankNumNodesRequired
( 
    const NNSG2dMultiCellDataBank*       pMultiCellDataBank 
);
u32 NNS_G2dGetMCWorkAreaSize
(
    const NNSG2dMultiCellDataBank*       pMultiCellDataBank,
    NNSG2dMCType                         mcType
);

//------------------------------------------------------------------------------
// Animation related
//------------------------------------------------------------------------------
void NNS_G2dTickMCInstance( NNSG2dMultiCellInstance* pMultiCellAnim, fx32 frames );
void NNS_G2dTickMCAnimation( NNSG2dMultiCellAnimation* pMultiCellAnim, fx32 frames );
//------------------------------------------------------------------------------
void NNS_G2dSetMCAnimationCurrentFrame
( 
    NNSG2dMultiCellAnimation*   pMultiCellAnim, 
    u16                         frameIndex 
);

//------------------------------------------------------------------------------
// Sets the animation frame of cell animations in multicell
void NNS_G2dSetMCAnimationCellAnimFrame
( 
    NNSG2dMultiCellAnimation*   pMultiCellAnim, 
    u16                         caFrameIndex     
);

void NNS_G2dStartMCCellAnimationAll
(
    NNSG2dMultiCellInstance*    pMCInst 
);

void NNS_G2dRestartMCAnimation
(
    NNSG2dMultiCellAnimation*   pMultiCellAnim
);

//------------------------------------------------------------------------------
void NNS_G2dSetMCAnimationSpeed
(
    NNSG2dMultiCellAnimation*   pMultiCellAnim, 
    fx32                        speed  
);

void NNS_G2dResetMCCellAnimationAll
(
    NNSG2dMultiCellInstance*    pMCInst 
);

//------------------------------------------------------------------------------
// Create OAM information
u16 NNS_G2dMakeSimpleMultiCellToOams
( 
    GXOamAttr*                      pDstOams, 
    u16                             numDstOams,
    const NNSG2dMultiCellInstance*  pMCellInst, 
    const MtxFx22*                  pMtxSR, 
    const NNSG2dFVec2*              pBaseTrans,
    u16                             affineIndex,
    BOOL                            bDoubleAffine 
);

//------------------------------------------------------------------------------
void NNS_G2dTraverseMCCellAnims
( 
    NNSG2dMultiCellInstance*         pMCellInst,
    NNSG2dMCTraverseCellAnimCallBack pCBFunc,
    u32                              userParamater
);
//------------------------------------------------------------------------------
void NNS_G2dTraverseMCNodes
( 
    NNSG2dMultiCellInstance*        pMCellInst,
    NNSG2dMCTraverseNodeCallBack    pCBFunc,
    u32                             userParamater
);


//------------------ OLD ------------------------------
// Old API
// It was kept to maintain compatibility with older versions.
void NNS_G2dInitMCAnimation( 
    NNSG2dMultiCellAnimation*          pMultiCellAnim, 
    NNSG2dNode*                        pNodeArray, 
    NNSG2dCellAnimation*               pCellAnim, 
    u16                                numNode, 
    const NNSG2dCellAnimBankData*      pAnimBank,  
    const NNSG2dCellDataBank*          pCellDataBank,
    const NNSG2dMultiCellDataBank*     pMultiCellDataBank 
);
void NNS_G2dInitMCInstance
( 
    NNSG2dMultiCellInstance*      pMultiCell, 
    NNSG2dNode*                   pNodeArray, 
    NNSG2dCellAnimation*          pCellAnim, 
    u16                           numNode, 
    const NNSG2dCellAnimBankData* pAnimBank,  
    const NNSG2dCellDataBank*     pCellDataBank 
);
BOOL NNS_G2dSetMCDataToMCInstance( 
    NNSG2dMultiCellInstance*       pMCInst, 
    const NNSG2dMultiCellData*     pMcData
);
//------------------ OLD ------------------------------


//------------------------------------------------------------------------------
// inline funcs.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
NNS_G2D_INLINE NNSG2dAnimController* NNS_G2dGetMCAnimAnimCtrl
( 
    NNSG2dMultiCellAnimation* pMultiCellAnim 
)
{
    NNS_G2D_NULL_ASSERT( pMultiCellAnim );
    return &pMultiCellAnim->animCtrl;
}

//
// Gets the pointer to the internal work memory
//
NNS_G2D_INLINE void* NNSi_G2dGetMCInstanceWorkMemory
(
    NNSG2dMultiCellInstance*      pMultiCell
)
{
    return pMultiCell->pCellAnimInstasnces;
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_MULTICELLANIMATION_H_

