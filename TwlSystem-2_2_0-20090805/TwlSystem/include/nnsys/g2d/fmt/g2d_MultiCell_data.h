/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d - fmt
  File:     g2d_MultiCell_data.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_G2D_MULTICELL_DATA_H_
#define NNS_G2D_MULTICELL_DATA_H_

#include <nitro/types.h>
#include <nnsys/g2d/fmt/g2d_Common_data.h>
#include <nnsys/g2d/fmt/g2d_Anim_data.h>
#include <nnsys/g2d/fmt/g2d_Cell_data.h>
#include <nnsys/g2d/g2d_config.h>


#ifdef __cplusplus
extern "C" {
#endif

#define NNS_G2D_BINFILE_EXT_MULTICELL     "NMCR"

#define NNS_G2D_BINFILE_SIG_MULTICELL          (u32)'NMCR'
#define NNS_G2D_BLKSIG_MULTICELLBANK           (u32)'MCBK'

//
// Version information
// Ver         Changes
// -------------------------------------
// 1.0         Initial version
//
#define NNS_G2D_NMCR_MAJOR_VER             (u8)1
#define NNS_G2D_NMCR_MINOR_VER             (u8)0



#define NNS_G2D_MCNODE_PLAYMODE_MASK        0x0F
#define NNS_G2D_MCNODE_PLAYMODE_SHIFT       0
#define NNS_G2D_MCNODE_VISIBILITY_SHIFT     5
#define NNS_G2D_MCNODE_CELLANIMIDX_SHIFT    8
#define NNS_G2D_MCNODE_CELLANIMIDX_MASK     0xFF00

typedef enum NNSG2dMCAnimationPlayMode
{
    NNS_G2D_MCANIM_PLAYMODE_RESET = 0,
    NNS_G2D_MCANIM_PLAYMODE_CONTINUE  = 1,
    NNS_G2D_MCANIM_PLAYMODE_PAUSE = 2,
    NNS_G2D_MCANIM_PLAYMODE_MAX
    
}NNSG2dMCAnimationPlayMode;

//------------------------------------------------------------------------------
typedef struct NNSG2dMultiCellHierarchyData
{
    u16         animSequenceIdx;        // Sequence number played by NNSG2dCellAnimation
    s16         posX;                   // NNSG2dCellAnimation location in MultiCell local system
    s16         posY;                   // NNSG2dCellAnimation location in MultiCell local system
                                        
                                        
    u16         nodeAttr;               // Node attribute ( NNSG2dMCAnimationPlayMode, etc.) 
                                        // 16 ............ 8 ... 5 .....4 .........................0
                                        //   Cell animation number    Reserved    Visibility    NNSG2dMCAnimationPlayMode
                                        //
    
    
}NNSG2dMultiCellHierarchyData;


//------------------------------------------------------------------------------
// NNSG2dMultiCellData definition data
// Data stored in NNSG2dMultiCellAnimation definition file
// referenced by NNSG2dMultiCellInstance
//
typedef struct NNSG2dMultiCellData
{
    u16                             numNodes;
//  u16                             numTotalOams;     // total number of Oams needed to render MultiCell (not implemented: unusable)
    u16                             numCellAnim;      // Number of cell animation instances needed
    NNSG2dMultiCellHierarchyData*   pHierDataArray;   // Playback sequence, location and such (for numNodes)

}NNSG2dMultiCellData;


//------------------------------------------------------------------------------
typedef struct NNSG2dMultiCellDataBank
{
    u16                             numMultiCellData;
    u16                             pad16;
    NNSG2dMultiCellData*            pMultiCellDataArray;
    NNSG2dMultiCellHierarchyData*   pHierarchyDataArray;
    void*                           pStringBank;
    void*                           pExtendedData;        // offset addr (if it exists)
    // NNSG2dAnimBankData is defined by NNSG2dCellAnimation making up MultiCell.
    // const NNSG2dAnimBankData*     pAnimDataBank; // related AnimBank (set during unpack at runtime)
    //
    // Change: Decided to maintain this data in NNSG2dMultiCellInstance (put data in an instance).
    // 
    // 
    // If we consider using the runtime instance, NNSG2dMultiCellInstance, the pAnimDataBank information would normally be maintained in NNSG2dMultiCellData. If we do that, however:
    //  - The increase in data volume is large. (The number of instances of NNSG2dMultiCellData may become very large.)
    //  - pAnimDataBank data will often be redundant.
    // 
    // If so, hold the data using NNSG2dMultiCellInstance which holds NNSG2dMultiCellData.
    // The above problem cannot be completely avoided. Only the portion associated during runtime is highly flexible.
    // (There is a high degree of freedom, particularly in the case where multiple NNSG2dMultiCellInstance elements share NNSG2dMultiCellData.)
    // 
    
    
}NNSG2dMultiCellDataBank;

//------------------------------------------------------------------------------
typedef struct NNSG2dMultiCellDataBankBlock
{
    NNSG2dBinaryBlockHeader     blockHeader;
    NNSG2dMultiCellDataBank     multiCellDataBank;
    
}NNSG2dMultiCellDataBankBlock;


//------------------------------------------------------------------------------
typedef struct NNSG2dUserExMultiCellAttr
{
    u32*           pAttr;
    
}NNSG2dUserExMultiCellAttr;

typedef struct NNSG2dUserExMultiCellAttrBank
{
    u16                          numMultiCells; // 
    u16                          numAttribute;  // Attribute count: Currently fixed at 1
    NNSG2dUserExMultiCellAttr*   pMCAttrArray;   
    
}NNSG2dUserExMultiCellAttrBank;



//------------------------------------------------------------------------------
// Inline functions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Sets multicell node attributes.
// This function is used by converter.
// 
NNS_G2D_INLINE void 
NNSi_G2dSetMultiCellNodeAttribute
( 
    NNSG2dMCAnimationPlayMode    mode, 
    int                          bVisibility,
    u16*                         pDstAttr
)
{
    *pDstAttr = (u16)( ( ( mode & NNS_G2D_MCNODE_PLAYMODE_MASK ) << NNS_G2D_MCNODE_PLAYMODE_SHIFT ) | 
                       ( ( bVisibility & 0x1 ) << NNS_G2D_MCNODE_VISIBILITY_SHIFT ) );
}

//------------------------------------------------------------------------------
// Gets visibility state of node.
NNS_G2D_INLINE BOOL 
NNSi_G2dIsMultiCellNodeVisible
( 
    const NNSG2dMultiCellHierarchyData* pNode 
)
{
    return (BOOL)(( pNode->nodeAttr >> NNS_G2D_MCNODE_VISIBILITY_SHIFT ) & 0x1);
}

//------------------------------------------------------------------------------
// Gets the cell animation playback method that is bound to the node.
NNS_G2D_INLINE NNSG2dMCAnimationPlayMode 
NNSi_G2dGetMultiCellNodePlayMode
( 
    const NNSG2dMultiCellHierarchyData* pNode 
)
{
    const NNSG2dMCAnimationPlayMode mode 
        = (NNSG2dMCAnimationPlayMode) ( ( pNode->nodeAttr >> NNS_G2D_MCNODE_PLAYMODE_SHIFT ) & 
                                          NNS_G2D_MCNODE_PLAYMODE_MASK );
    // TODO: ASSERT
                                                
    return mode;
}
//------------------------------------------------------------------------------
// Sets the number of cell animation which the node references.
// Function used in the converter.
NNS_G2D_INLINE void NNSi_G2dSetMC2NodeCellAinmIdx
( 
    NNSG2dMultiCellHierarchyData*  pNodeData, 
    u8                             idx 
)
{
    pNodeData->nodeAttr &= ~NNS_G2D_MCNODE_CELLANIMIDX_MASK;
    pNodeData->nodeAttr |= idx << NNS_G2D_MCNODE_CELLANIMIDX_SHIFT;
}

//------------------------------------------------------------------------------
// Gets the number of the cell animation which the node references.
NNS_G2D_INLINE u16 
NNSi_G2dGetMC2NodeCellAinmIdx
( 
    const NNSG2dMultiCellHierarchyData*  pNodeData
)
{
    return (u16)((NNS_G2D_MCNODE_CELLANIMIDX_MASK & pNodeData->nodeAttr ) >> NNS_G2D_MCNODE_CELLANIMIDX_SHIFT);
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetUserExCellAttrBankFromMCBank

  Description:  Gets a cell extended attribute bank from a multicell bank.
                
                
  Arguments:    pMCBank:           Multicell bank
                
                
  Returns:      Cell extended attribute bank
  
 *---------------------------------------------------------------------------*/
NNS_G2D_INLINE const NNSG2dUserExCellAttrBank* 
NNS_G2dGetUserExCellAttrBankFromMCBank( const NNSG2dMultiCellDataBank* pMCBank )
{
    // Gets the block
    const NNSG2dUserExDataBlock* pBlk 
        = NNSi_G2dGetUserExDataBlkByID( pMCBank->pExtendedData,
                                        NNS_G2D_USEREXBLK_CELLATTR );
    // If successful in getting the block...
    if( pBlk != NULL )
    {
        return (const NNSG2dUserExCellAttrBank*)(pBlk + 1);
    }else{
        return NULL;                                
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_MULTICELL_DATA_H_

