/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_OAMEx.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include <nnsys/g2d/g2d_OAMEx.h>
#include <nnsys/g2d/fmt/g2d_Cell_data.h>

#include "g2d_Internal.h"





static GXOamAttr       defaultOam_ = { 193, 193, 193 };







//------------------------------------------------------------------------------
// check the appropriateness of the various registration functions
static NNS_G2D_INLINE BOOL IsOamEntryFuncsValid_
( 
    const NNSG2dOamManagerInstanceEx*  pMan,
    const NNSG2dOamExEntryFunctions*   pF 
)
{
    const BOOL bValid = (BOOL)(( pF                       != NULL ) &&
                               ( pF->getOamCapacity       != NULL ) &&    
                               ( pF->entryNewOam          != NULL ) );
                      
    // when managing affine parameters...         
    if( pMan->pAffineBuffer != NULL || pMan->lengthAffineBuffer != 0 )
    {
        // The associated callback must be set correctly.
        return (BOOL)( bValid &&
                      ( pF->getAffineCapacity    != NULL ) &&
                      ( pF->entryNewAffine       != NULL ) );
    }else{
        return bValid;  
    }
    
}

//------------------------------------------------------------------------------
// Get new NNSG2dOamChunk from shared NNSG2dOamChunk.
// Shared NNSG2dOamChunk is set during initialization.
//
static NNS_G2D_INLINE NNSG2dOamChunk* GetNewOamChunk_( NNSG2dOamManagerInstanceEx* pMan, const GXOamAttr* pOam )
{
    NNSG2dOamChunk*   pRet = NULL;
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_NULL_ASSERT( pOam );
    
    if( pMan->numUsedOam < pMan->numPooledOam )
    {
        
        pRet = &pMan->pPoolOamChunks[pMan->numUsedOam];
        pMan->numUsedOam++;
        
        pRet->oam = *pOam;
        return pRet;
    }else{
        NNSI_G2D_DEBUGMSG0("We have no capacity for a new Oam.");
        return NULL;
    }
}

//------------------------------------------------------------------------------
// Add NNSG2dOamChunk to start of NNSG2dOamChunkList.
static NNS_G2D_INLINE void AddFront_( NNSG2dOamChunkList* pOamList, NNSG2dOamChunk* pChunk )
{
    pChunk->pNext       = pOamList->pChunks;
    pOamList->pChunks   = pChunk;
    
    pOamList->numChunks++;
}

//------------------------------------------------------------------------------
// Add NNSG2dOamChunk to end of NNSG2dOamChunkList.
static NNS_G2D_INLINE void AddBack_( NNSG2dOamChunkList* pOamList, NNSG2dOamChunk* pChunk )
{
    pChunk->pNext               = NULL;
    
    if( pOamList->pLastChunk != NULL )
    {
        // after the first time
        pOamList->pLastChunk->pNext = pChunk;
    }else{
        // first time
        pOamList->pChunks    = pChunk;
    }
    
    // update the end of the list
    pOamList->pLastChunk = pChunk;
    
    pOamList->numChunks++;
}

//------------------------------------------------------------------------------
// register Oam to lower module
static NNS_G2D_INLINE void EntryOamToToBaseModule_( NNSG2dOamManagerInstanceEx* pMan, const GXOamAttr* pOam, u16 totalOam )
{
    (void)(*pMan->oamEntryFuncs.entryNewOam)( pOam, totalOam );
}


//------------------------------------------------------------------------------
// determine whether affineProxy is valid
static NNS_G2D_INLINE BOOL IsAffineProxyValid_( NNSG2dOamManagerInstanceEx* pMan )
{
    return ( pMan->pAffineBuffer != NULL && pMan->lengthAffineBuffer != 0 ) ? 
        TRUE : FALSE;
}
//------------------------------------------------------------------------------
// Check whether enough space remains for new registration.
static NNS_G2D_INLINE BOOL HasEnoughCapacity_( NNSG2dOamManagerInstanceEx* pMan )
{
    NNS_G2D_NULL_ASSERT( pMan );
    return ( pMan->numAffineBufferUsed < pMan->lengthAffineBuffer ) ? TRUE : FALSE;
}
//------------------------------------------------------------------------------
// Perform conversion affineProxyIdx => affineHWIndex.
static NNS_G2D_INLINE u16 ConvertAffineIndex_( NNSG2dOamManagerInstanceEx* pMan, u16 affineProxyIdx )
{
    NNS_G2D_ASSERT( IsAffineProxyValid_( pMan ) );
    NNS_G2D_ASSERT( affineProxyIdx < pMan->lengthAffineBuffer );
        
    return pMan->pAffineBuffer[affineProxyIdx].affineHWIndex;            
}

//------------------------------------------------------------------------------
// Change the NNSG2dOamChunk reference affine parameter data for the entire NNSG2dOamChunkList to what was actually configured in hardware.
// 
//
// We changed the rendering order of affine-transformed OAMs so that it matches that of regular OAMs.
// Previously, affine-transformed OAMs were rendered in the reverse order they were saved in.
// 
// Also note that by rebuilding the library with NNS_G2D_OAMEX_USE_OLD_REINDEXOAMCHUNKLIST_ defined, it is possible to return to the same behavior as past implementations.
// 
//
//#define NNS_G2D_OAMEX_USE_OLD_REINDEXOAMCHUNKLIST_ 1

static NNS_G2D_INLINE void ReindexOamChunkList_
( 
    NNSG2dOamManagerInstanceEx*     pMan, 
    NNSG2dOamChunkList*             pChunkList
)
{
#ifdef NNS_G2D_OAMEX_USE_OLD_REINDEXOAMCHUNKLIST_
    //
    // previous implementation
    //
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_NULL_ASSERT( pChunkList );
    {
        NNSG2dOamChunk*    pChunk     = pChunkList->pAffinedChunks;
        NNSG2dOamChunk*    pNextChunk = NULL;
        
        //
        // to the end of the list...
        //
        while( pChunk != NULL )
        {
            //
            // convert index from AffineProxyIndex into actual HW Index
            //
            const u16 index = ConvertAffineIndex_( pMan, pChunk->affineProxyIdx );
            
            //OS_Printf("AffProxy_Idx = %d, => HW_Idx = %d \n", pChunk->affineProxyIdx, index );
            
            pNextChunk = pChunk->pNext;
            
            // if index has valid value.....
            if( index != NNS_G2D_OAMEX_HW_ID_NOT_INIT )
            {
                // replace the OAM Index
                // G2_SetOBJEffect( &pChunk->oam, NNS_G2dGetOamManExDoubleAffineFlag( pMan ), index );
                pChunk->oam.rsParam = index;
                
                //
                // move to normal OamChunkList
                // The registration order will be the reverse of that used with a normal OamChunkList.
                //
                AddFront_( pChunkList, pChunk );
            }
            pChunk     = pNextChunk;
        }
    }
#else // NNS_G2D_OAMEX_USE_OLD_REINDEXOAMCHUNKLIST_
    //
    // current implementation
    //
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_NULL_ASSERT( pChunkList );
    {
        NNSG2dOamChunk*    pChunk     = pChunkList->pAffinedChunks;
        NNSG2dOamChunk*    pHeadChunk = NULL;
        NNSG2dOamChunk*    pPrevChunk = NULL;
        int                numAffinedOam = 0;
        
        
        //
        // continue to the end of the affine-transformed OAM list...
        //
        while( pChunk != NULL )
        {
            //
            // convert index from AffineProxyIndex into actual HW Index
            //
            const u16 index = ConvertAffineIndex_( pMan, pChunk->affineProxyIdx );
            
            // if index has valid value.....
            if( index != NNS_G2D_OAMEX_HW_ID_NOT_INIT )
            {
                // store the top of the (valid) affine-transformed OAM list
                if( pHeadChunk == NULL )
                {
                    pHeadChunk = pChunk;
                }
                
                //
                // replace the OAM Index
                //
                pChunk->oam.rsParam = index;
                
                
                // go to the next chunk
                pPrevChunk = pChunk;
                pChunk     = pChunk->pNext;
                numAffinedOam++;
                
            }else{
                // remove from list
                if( pPrevChunk != NULL )
                {
                    pPrevChunk->pNext = pChunk->pNext;
                }
                pMan->numUsedOam--;
                // go to the next chunk
                pChunk     = pChunk->pNext;
            }
        }
        
        //
        // pPrevChunk->pNext => end of affine-transformed OAM list
        // Collect the affine-transformed OAM list and insert it at the top of the normal chunk list.
        // There is no change to the order of the affine-transformed OAM list.
        // 
        if( numAffinedOam != 0 ) 
        {
           pPrevChunk->pNext     = pChunkList->pChunks;
           pChunkList->pChunks   = pHeadChunk;
           pChunkList->numChunks += numAffinedOam;
           pChunkList->pAffinedChunks = NULL;
        }
    }
#endif // NNS_G2D_OAMEX_USE_OLD_REINDEXOAMCHUNKLIST_
}

//------------------------------------------------------------------------------
// Process affine-transformed NNSG2dOamChunk so that it can be rendered.
//
// The affine-transformed NNSG2dOamChunk is stored on a different list than normal NNSG2dOamChunk. 
// It is necessary to replace the normal affine parameter index value because NNSG2dOamChunk internally references affine parameters using the affineProxy array's index.
// 
// 
// 
static NNS_G2D_INLINE void ReindexAffinedOams_( NNSG2dOamManagerInstanceEx* pMan )
{
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_ASSERT( pMan->numAffineBufferUsed != 0 );
    NNS_G2D_ASSERT( IsAffineProxyValid_( pMan ) );
    
    {
       u16 i;
       //
       // For all NNSG2dOamChunkList instances...
       //
       for( i = 0; i < pMan->lengthOfOrderingTbl; i++ )
       {
           ReindexOamChunkList_( pMan, &pMan->pOamOrderingTbl[i] );
       }
    }
}




//------------------------------------------------------------------------------
// Check whether the specified rendering priority is appropriate.
// The value pMan->lengthOfOrderingTbl is freely set by the user at the time the manager is initialized.
static NNS_G2D_INLINE BOOL IsPriorityValid_( NNSG2dOamManagerInstanceEx* pMan, u8 priority )
{
    return ( priority < pMan->lengthOfOrderingTbl ) ? TRUE : FALSE;
}

//------------------------------------------------------------------------------
// Specify AffineIndex in NNSG2dOamChunkList and register OAMAttribute.
//
// The new NNSG2dOamChunk is stored in the list dedicated to affine-transformed NNSG2dOamChunk.
// The NNSG2dOamChunk-specific list is re-indexed with the ReindexAffinedOams_ function, and the normal NNSG2dOamChunk list is inserted.
// 
//
static NNS_G2D_INLINE BOOL EntryNewOamWithAffine_
( 
    NNSG2dOamChunkList*       pOamList, 
    NNSG2dOamChunk*           pOamChunk, 
    u16                       affineIdx, 
    NNSG2dOamExDrawOrder      drawOrderType
)
{
    NNS_G2D_NULL_ASSERT( pOamList );
    
    if( pOamChunk != NULL )
    {
        //
        // do not use affine
        //
        if( affineIdx == NNS_G2D_OAM_AFFINE_IDX_NONE )
        {
           if( drawOrderType == NNSG2D_OAMEX_DRAWORDER_BACKWARD )
           {
               //
               // move to normal OamChunkList
               //
               AddFront_( pOamList, pOamChunk );
           }else{
               AddBack_( pOamList, pOamChunk );
           }
        }else{
            
            pOamChunk->affineProxyIdx = affineIdx;
            if( drawOrderType == NNSG2D_OAMEX_DRAWORDER_BACKWARD )
            {
               //
               // put in the affine list
               // add to top
               //
               pOamChunk->pNext            = pOamList->pAffinedChunks;
               pOamList->pAffinedChunks    = pOamChunk;
            }else{
               //
               // put in the affine list
               // add to end
               //
               pOamChunk->pNext = NULL;
               if( pOamList->pLastAffinedChunk != NULL )
               {
                  pOamList->pLastAffinedChunk->pNext = pOamChunk;
               }else{
                  pOamList->pAffinedChunks = pOamChunk;
               }
               pOamList->pLastAffinedChunk = pOamChunk;
            }
        }
        return TRUE;
    }else{
        return FALSE;
    }
}



//------------------------------------------------------------------------------
// advance the list's pointer by "num" amount only
static NNS_G2D_INLINE NNSG2dOamChunk* AdvancePointer_( NNSG2dOamChunk* pChunk, u16 num )
{
    NNS_G2D_NULL_ASSERT( pChunk );
    while( num > 0 )
    {
        pChunk = pChunk->pNext;
        num--;
        NNS_G2D_NULL_ASSERT( pChunk );
    }
    
    return pChunk;
}

//------------------------------------------------------------------------------
// render OamChunkList
// Collect together all OamChunk with same rendering priority to form OamChunkList.
//
static NNS_G2D_INLINE u16 DrawOamChunks_
( 
    NNSG2dOamManagerInstanceEx*    pMan, 
    NNSG2dOamChunkList*             pOamList, 
    NNSG2dOamChunk*                 pChunk, 
    u16                             numOam, 
    u16                             capacityOfHW, 
    u16                             numTotalOamDrawn 
)
{
    //
    // until all OBJ have been drawn or there is no more free space
    // 
    while( numOam > 0 && (capacityOfHW - numTotalOamDrawn) > 0 )
    {
        //
        // copy value
        //
        EntryOamToToBaseModule_( pMan, &pChunk->oam, numTotalOamDrawn );
        
        
        pChunk = pChunk->pNext;
        // 
        // If end of list has been reached, start again from top of list.
        //
        if( pChunk == NULL )
        {
            pChunk = pOamList->pChunks;
        }
        
        
        numOam--;
        numTotalOamDrawn++;
    }
    return numTotalOamDrawn;
} 



//------------------------------------------------------------------------------
// Reflect AffineProxy in lower module.
// (Normally, the lower module propagates the reflection to HW.)
static NNS_G2D_INLINE void LoadAffineProxyToBaseModule_( NNSG2dOamManagerInstanceEx* pMan )
{
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_ASSERT( pMan->numAffineBufferUsed != 0 );
    
    {    
       u16        count     = 0x0;
       u16        i         = pMan->lastFrameAffineIdx;
       const u16  numAffine = pMan->numAffineBufferUsed;
       const u16  capacity  = (*pMan->oamEntryFuncs.getAffineCapacity)();
           
       NNSG2dAffineParamProxy*   pAff = NULL;
       //
       // Aim of process is to register in continuation from previous frame.
       //
       while( ( count < numAffine ) && 
              ( count < capacity ) )
       {
           if( i >= numAffine )
           {
              i = 0;
           }
           pAff = &pMan->pAffineBuffer[ i ];
           
           // store HW affine index
           pAff->affineHWIndex 
              = (*pMan->oamEntryFuncs.entryNewAffine)( &pAff->mtxAffine, count );
           
           // pMan->lastFrameAffineIdx = i;
           
           i++;
           count++;    
       }
       
       pMan->lastFrameAffineIdx = i;
    }
}

//------------------------------------------------------------------------------
//
// Calculate the range for render registration.
//
// procedure:
//
// 1. Set all lists as lists that are not rendered.
// 2. Scan the lists and, while counting the render OAM chunks, put information for rendering in lists.
// 3. End process when there is no more free space or when all chunks have been checked.
//
static NNS_G2D_INLINE void 
CalcDrawnListArea_( NNSG2dOamManagerInstanceEx* pMan )
{
    NNSG2dOamChunk*       pChunk    = NULL;
    NNSG2dOamChunkList*   pOamList  = NULL;
    u16       numTotalOamDrawn      = 0;
    const u16 capacityOfHW          = ( *(pMan->oamEntryFuncs.getOamCapacity) )();
    
    u16       i;
    
    
    //
    // Reset the rendering flag in all chunk lists.
    //
    for( i = 0; i < pMan->lengthOfOrderingTbl; i++ )
    {
       pMan->pOamOrderingTbl[i].bDrawn = FALSE;
    }
    
    
    //
    // Scan data, starting from the position where the previous frame ended.
    //
    i = (u16)(pMan->lastRenderedOrderingTblIdx);
    
    //
    // until all chunks in the chunk list have been inspected...
    //
    while( numTotalOamDrawn < pMan->numUsedOam )
    {
       // If end of ordering table has been reached, return to start of table.
       if( i >= pMan->lengthOfOrderingTbl )
       {
           i = 0;
       }
       
       pOamList   = &pMan->pOamOrderingTbl[i];
       //
       // if chunks are registered in chunk list....
       //
       if( pOamList->numChunks != 0 )
       {
           const u16 currentCapacity = (u16)(capacityOfHW - numTotalOamDrawn);
           
           //
           // set as chunk list that should be rendered
           // also, update the number of the last rendered chunk list
           //
           pOamList->bDrawn                 = TRUE;
           pMan->lastRenderedOrderingTblIdx = i;
           
           //
           // if all chunks in list can be registered
           //
           if( pOamList->numChunks <= currentCapacity )
           {
              pOamList->numLastFrameDrawn = 0;
              pOamList->numDrawn          = pOamList->numChunks;   
              
           }else{
              // If rendered to the end of the list, in the next frame, rendering occurs from the list after this chunk list.
              // 
              if( (pOamList->numDrawn + pOamList->numLastFrameDrawn) / pOamList->numChunks > 0 )
              {
                  pMan->lastRenderedOrderingTblIdx = (u16)(i+1);
              }
              
              //
              // Calculates the offset number given to the current frame before rendering from the number of rendered chunks in the prior frame.
              // 
              //
              pOamList->numLastFrameDrawn = (u16)((pOamList->numDrawn + 
                                            pOamList->numLastFrameDrawn ) % pOamList->numChunks);
              
              pOamList->numDrawn          = currentCapacity;// render only what can be rendered    
              //
              // Quit
              //
              break;
           }
           numTotalOamDrawn += pOamList->numDrawn;
       }
       i++;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dResetOamManExBuffer

  Description:  Resets the manager.
                                
  Arguments:    pMan:   [OUT] manager instance
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dResetOamManExBuffer( NNSG2dOamManagerInstanceEx* pOam )
{
    NNS_G2D_NULL_ASSERT( pOam );
    pOam->numUsedOam = 0;
    pOam->numAffineBufferUsed = 0;
    
    // reset the ordering table
    {
        u16 i = 0;
        for( i = 0;i < pOam->lengthOfOrderingTbl; i++ )
        {
            pOam->pOamOrderingTbl[i].numChunks      = 0;
            pOam->pOamOrderingTbl[i].pChunks        = NULL;
            pOam->pOamOrderingTbl[i].pAffinedChunks = NULL;
            pOam->pOamOrderingTbl[i].pLastChunk        = NULL;
            pOam->pOamOrderingTbl[i].pLastAffinedChunk = NULL;
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetOamManExInstance

  Description:  Creates the manager instance.


                
                
                
  Arguments:    pOam                    [OUT] expanded OAM manager instance 
                pOamOrderingTbl         [IN]  starting address of ordering table 
                lengthOfOrderingTbl     [IN]  length of ordering table 
                numPooledOam            [IN]  number of OBJChunk 
                pPooledOam              [IN]  pointer to OBJChunk array 
                lengthAffineBuffer      [IN]  length of affine parameter buffer array 
                pAffineBuffer           [IN]   pointer to affine parameter buffer array 

                
  Returns:      success or failure of initialization (at present, always succeeds)
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dGetOamManExInstance
( 
    NNSG2dOamManagerInstanceEx*     pOam, 
    NNSG2dOamChunkList*             pOamOrderingTbl, 
    u8                              lengthOfOrderingTbl,
    u16                             numPooledOam,
    NNSG2dOamChunk*                 pPooledOam,
    u16                             lengthAffineBuffer,
    NNSG2dAffineParamProxy*         pAffineBuffer 
)
{
    NNS_G2D_NULL_ASSERT( pOam );
    NNS_G2D_ASSERT( lengthOfOrderingTbl != 0 );
    NNS_G2D_NULL_ASSERT( pPooledOam );
    NNS_G2D_ASSERT( numPooledOam != 0 );
    
    // Rendering order type
    // (To maintain backward compatibility, the default order is reversed)
    pOam->drawOrderType = NNSG2D_OAMEX_DRAWORDER_BACKWARD;
    
    //
    // zero-clear chunk list elements
    //
    MI_CpuClear32( pOamOrderingTbl, lengthOfOrderingTbl * sizeof( NNSG2dOamChunkList ) );
    
    
    pOam->pOamOrderingTbl           = pOamOrderingTbl;
    pOam->lengthOfOrderingTbl       = lengthOfOrderingTbl;
    
    pOam->numPooledOam              = numPooledOam;
    pOam->pPoolOamChunks            = pPooledOam;
    
    pOam->lengthAffineBuffer    = lengthAffineBuffer;
    pOam->pAffineBuffer         = pAffineBuffer;
    
    
    
    //
    // initialization related to registration functions
    //
    {
        NNSG2dOamExEntryFunctions*  pFuncs = &pOam->oamEntryFuncs;
        
        pFuncs->getOamCapacity       = NULL;
        pFuncs->getAffineCapacity    = NULL;
        pFuncs->entryNewOam          = NULL;
        pFuncs->entryNewAffine       = NULL;
    }
    
    
    NNS_G2dResetOamManExBuffer( pOam );
    
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         NNSG2d_SetOamManExDrawOrderType

  Description:  Configures the manager's OAM rendering order type.
                Note that the specified OAM rendering order value is NNSG2D_OAMEX_DRAWORDER_BACKWARD.
                
                This is to maintain compatibility with previous versions.

  Arguments:    pOam:   [OUT] manager instance
                drawOrderType:   [IN]  rendering order type
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNSG2d_SetOamManExDrawOrderType
( 
    NNSG2dOamManagerInstanceEx*    pOam, 
    NNSG2dOamExDrawOrder           drawOrderType
)
{
    NNS_G2D_NULL_ASSERT( pOam );
    
    pOam->drawOrderType = drawOrderType;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetOamManExEntryFunctions

  Description:  Sets various registration functions in the manager.
                You must call this function and set the registration functions before using the manager.

                
                
                
  Arguments:    pMan:   [OUT] manager instance
                pSrc:   [IN]  registration functions
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetOamManExEntryFunctions
( 
    NNSG2dOamManagerInstanceEx*        pMan, 
    const NNSG2dOamExEntryFunctions*   pSrc 
)
{
    NNS_G2D_NULL_ASSERT( pMan );
    {
        NNSG2dOamExEntryFunctions*  pDst = &pMan->oamEntryFuncs;    
        
        NNS_G2D_NULL_ASSERT( pSrc );

        if( pSrc->getOamCapacity != NULL )
        {
            pDst->getOamCapacity = pSrc->getOamCapacity;
        }
        
        if( pSrc->getAffineCapacity != NULL )
        {
            pDst->getAffineCapacity = pSrc->getAffineCapacity;
        }
        
        if( pSrc->entryNewOam != NULL )
        {
            pDst->entryNewOam = pSrc->entryNewOam;
        }
        
        if( pSrc->entryNewAffine != NULL )
        {
            pDst->entryNewAffine = pSrc->entryNewAffine;
        }
        
        NNS_G2D_ASSERT( IsOamEntryFuncsValid_( pMan, pDst ) );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dEntryOamManExOam

  Description:  Specifies and registers OBJ in extended OAM in the order of registration at time of rendering.
                When actually applying to the hardware, the specified order is used.

                
                If there is enough space, registration is performed and TRUE is returned.
                
                
                
  Arguments:    pMan:   [OUT] manager instance
                pOam:   [IN]  OAMAttribute 
                priority:   [IN]  rendering priority
                affineIdx:   [IN]  affine index
                
  Returns:      registration success or failure
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dEntryOamManExOam
( 
    NNSG2dOamManagerInstanceEx*    pMan, 
    const GXOamAttr*               pOam, 
    u8                             priority 
)
{
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_NULL_ASSERT( pOam );
    NNS_G2D_ASSERT( IsPriorityValid_( pMan, priority ) );
    
    {
        NNSG2dOamChunkList*   pOamList    = &pMan->pOamOrderingTbl[ priority ];
        NNSG2dOamChunk*       pOamChunk   = GetNewOamChunk_( pMan, pOam );
         
        return EntryNewOamWithAffine_( pOamList, 
                                       pOamChunk, 
                                       NNS_G2D_OAM_AFFINE_IDX_NONE, 
                                       pMan->drawOrderType );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dEntryOamManExOamWithAffineIdx

  Description:  Specifies AffineIndex and registers OAMAttribute in manager.
                Specify the return value for NNS_G2dEntryOamManExAffine().
                
                If there is enough space, registration is performed and TRUE is returned.
                
                
                
  Arguments:    pMan:   [OUT] manager instance
                pOam:   [IN]  OAMAttribute 
                priority:   [IN]  rendering priority
                affineIdx:   [IN]  affine index
                
  Returns:      registration success or failure
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dEntryOamManExOamWithAffineIdx
( 
    NNSG2dOamManagerInstanceEx*    pMan, 
    const GXOamAttr*               pOam, 
    u8                             priority, 
    u16                            affineIdx 
)
{
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_NULL_ASSERT( pOam );
    // Is affine transformation enabled for specified OAM attribute?
    NNS_G2D_ASSERT( pOam->rsMode & 0x1 );
    NNS_G2D_ASSERT( IsPriorityValid_( pMan, priority ) );
    
    {
        NNSG2dOamChunkList*   pOamList    = &pMan->pOamOrderingTbl[ priority ];
        NNSG2dOamChunk*       pOamChunk   = GetNewOamChunk_( pMan, pOam );
         
        return EntryNewOamWithAffine_( pOamList, 
                                       pOamChunk, 
                                       affineIdx, 
                                       pMan->drawOrderType );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dEntryOamManExAffine

  Description:  Registers affine parameters to the manager.
                If the manager has sufficient space the affine conversion matrix is stored in NNSG2dAffineParamProxy, and the index is returned.
                
                If a buffer array for the affine parameters has not been set up, the function fails on ASSERT.
                
                Objects reference affine parameters using the NNSG2dAffineParamProxy index within the manager.
                
                
                
                
  Arguments:    pMan:      [OUT] manager instance
                mtx:      [IN]  affine transform matrix
  
  Returns:      Index for the internal NNSG2dAffineParamProxy
  
 *---------------------------------------------------------------------------*/
u16 NNS_G2dEntryOamManExAffine
( 
    NNSG2dOamManagerInstanceEx*    pMan, 
    const MtxFx22*                 mtx 
)
{
    NNS_G2D_NULL_ASSERT( pMan ); 
    NNS_G2D_NULL_ASSERT( mtx );
    NNS_G2D_ASSERT( IsAffineProxyValid_( pMan ) );
    
    if( HasEnoughCapacity_( pMan ) )
    {
        NNSG2dAffineParamProxy* pAffineProxy 
           = &pMan->pAffineBuffer[pMan->numAffineBufferUsed];
           
        pAffineProxy->mtxAffine     = *mtx;
        pAffineProxy->affineHWIndex = NNS_G2D_OAMEX_HW_ID_NOT_INIT;
        
        return pMan->numAffineBufferUsed++;
    }   
    
    return NNS_G2D_OAM_AFFINE_IDX_NONE;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dApplyOamManExToBaseModule 

  Description:  Reflects the contents of the manager in HW.
                In actuality, the contents are not reflected in HW, but in the lower module.
                The contents will not be reflected in the rendering until the lower module reflects the contents in HW.
                
                
                
  Arguments:    pMan:      [OUT] manager instance
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dApplyOamManExToBaseModule( NNSG2dOamManagerInstanceEx* pMan )
{
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_NULL_ASSERT( pMan->oamEntryFuncs.getOamCapacity );
    {
        u16       numTotalOamDrawn = 0;
        const u16 capacityOfHW     = ( *(pMan->oamEntryFuncs.getOamCapacity) )();
        
        if( pMan->numUsedOam != 0 )
        {
            //
            // prepare affine-transformed Oam
            // 
            if( pMan->numAffineBufferUsed != 0 )
            {
                // apply contents of Affine Proxy in lower module and determine Affine Index
                LoadAffineProxyToBaseModule_( pMan );
                // re-index
                ReindexAffinedOams_( pMan );
            }
            
            //
            // calculate the chunk list registration range
            //
            CalcDrawnListArea_( pMan );
            
            //
            // perform actual rendering registration for external module
            //
            {
                u16 i = 0;
                NNSG2dOamChunk*       pChunk    = NULL;
                NNSG2dOamChunkList*   pOamList  = NULL;
        
                for( i = 0; i < pMan->lengthOfOrderingTbl; i++ )
                {
                    pOamList  = &pMan->pOamOrderingTbl[i];
                    //
                    // if chunk list needs to be rendered...
                    //  
                    if( pOamList->bDrawn )
                    {
                        NNS_G2D_ASSERT( pOamList->numLastFrameDrawn < pOamList->numChunks );
                        
                        pChunk = AdvancePointer_( pOamList->pChunks, pOamList->numLastFrameDrawn );               
                         
                        numTotalOamDrawn = DrawOamChunks_( pMan, 
                                                 pOamList, 
                                                 pChunk, 
                                                 pOamList->numDrawn, 
                                                 capacityOfHW, 
                                                 numTotalOamDrawn );
                    }
                }
            }
        }
        
        //
        // clear remaining space with default value
        //
        while( capacityOfHW > numTotalOamDrawn )
        {
            // clear with default value
            EntryOamToToBaseModule_( pMan, &defaultOam_, numTotalOamDrawn );
            numTotalOamDrawn++;
        }
    }
}




