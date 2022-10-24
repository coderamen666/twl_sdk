/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_Renderer.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/g2d/g2d_Renderer.h>

#include "g2d_Internal.h"
#include "g2di_RendererMtxStack.hpp"    // for MatrixStack 
#include "g2di_RendererMtxState.h"      // For MatrixCache & State


#include <nnsys/g2d/fmt/g2d_Oam_data.h>
#include <nnsys/g2d/g2d_OamSoftwareSpriteDraw.h> // auto Z offset 
#include <nnsys/g2d/g2d_SRTControl.h>








//------------------------------------------------------------------------------
#define NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pRnd ) \
    NNS_G2D_ASSERTMSG( (pRnd) != NULL, "Please call this method between Begin - End Rendering" ) \


//------------------------------------------------------------------------------
// current renderer instance
// Set when calling Begin-End Rendering().
static NNSG2dRendererInstance*      pCurrentInstance_   = NULL; 


//------------------------------------------------------------------------------
// additional variable: added for Vram transfer animation
static u32      currenVramTransferHandle_ = NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE;




typedef struct MCRenderState
{
    u16                         currentCellAnimIdx;
    u16                         pad16_;
    NNSG2dRndCore2DMtxCache*    cellAnimMtxCacheTbl[256];
    BOOL                        bDrawMC;

}MCRenderState;

static MCRenderState        mcRenderState_;







//------------------------------------------------------------------------------
// Checks whether NNSG2dRenderSurface list is circular reference.
// Called in assert macro during initialization when registration of NNSG2dRenderSurface and other processes occur.
// 
static BOOL IsNotCircularLinked_
( 
    const NNSG2dRenderSurface*      pList, 
    const NNSG2dRenderSurface*      pNew 
)
{
    const NNSG2dRenderSurface*  pCursor = pList;
    
    while( pCursor != NULL )
    {
        // found the same pointer
        if( pCursor == pNew ) 
        {
            // No good 
            return FALSE;
        }    
        pCursor = (const NNSG2dRenderSurface*)pCursor->pNextSurface;
    }
    
    // circular reference is not generated
    return TRUE;
}




//------------------------------------------------------------------------------
// process the palette number write in the OAMAttribute of the OBJ
// Executed inside DrawCellToSurface2D_() and DrawCellToSurface3D_() immediately before the render command is actually issued.
// 
// 
// 
static NNS_G2D_INLINE void OBJPaletteChangeHandling_( GXOamAttr* pOam )
{
    const NNSG2dPaletteSwapTable* pTbl 
        = NNS_G2dGetRendererPaletteTbl( pCurrentInstance_ );
    
    NNS_G2D_NULL_ASSERT( pOam );
    
    if( pTbl != NULL )
    {
        // rewrite the palette number
        const u16 newIdx 
           = NNS_G2dGetPaletteTableValue( pTbl, NNSi_G2dGetOamColorParam( pOam ) );
        pOam->cParam = newIdx;
    }
}

//------------------------------------------------------------------------------
// Determines the flip condition of the renderer, then configures the translation.
static NNS_G2D_INLINE void FlipTranslate_( int x, int y )
{
    const int x_ = NNS_G2dIsRndCoreFlipH( &pCurrentInstance_->rendererCore ) ? -x : x;    
    const int y_ = NNS_G2dIsRndCoreFlipV( &pCurrentInstance_->rendererCore ) ? -y : y;
                            
    NNS_G2dTranslate( x_ << FX32_SHIFT , y_ << FX32_SHIFT, 0 );
}

//------------------------------------------------------------------------------
// Multiply the affine conversion information held by NNSG2dSRTControl by the current matrix.
// The NNSG2dSRTControl type must be NNS_G2D_SRTCONTROLTYPE_SRT.
//
//
static NNS_G2D_INLINE void SetSrtControlToMtxStack_( const NNSG2dSRTControl* pSrtCtrl )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pSrtCtrl );
    NNS_G2D_ASSERTMSG( pSrtCtrl->type == NNS_G2D_SRTCONTROLTYPE_SRT, "TODO: show msg " );
    
    // T
    if( NNSi_G2dSrtcIsAffineEnable( pSrtCtrl, NNS_G2D_AFFINEENABLE_TRANS ) )
    {
        //
        // Reflect results of flip, and update translation parameters.
        //
        FlipTranslate_( pSrtCtrl->srtData.trans.x, pSrtCtrl->srtData.trans.y );                 
    }
    
    // R 
    if( NNSi_G2dSrtcIsAffineEnable( pSrtCtrl, NNS_G2D_AFFINEENABLE_ROTATE ) )
    {
        NNS_G2dRotZ( FX_SinIdx( pSrtCtrl->srtData.rotZ ), FX_CosIdx( pSrtCtrl->srtData.rotZ ) );
    }
    
    // S
    if( NNSi_G2dSrtcIsAffineEnable( pSrtCtrl, NNS_G2D_AFFINEENABLE_SCALE ) )
    {
        NNS_G2dScale( pSrtCtrl->srtData.scale.x, pSrtCtrl->srtData.scale.y, FX32_ONE );
    }

}


//------------------------------------------------------------------------------
// Call before and after the rendering of the cell that uses the VRAM transmission.
static NNS_G2D_INLINE void BeginDrawVramTransferedCell_( u32 cellVramTransferHandle )
{
    NNS_G2D_ASSERT( currenVramTransferHandle_ == NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE );
    NNS_G2D_ASSERT( cellVramTransferHandle != NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE );
    
    currenVramTransferHandle_ = cellVramTransferHandle;
}

//------------------------------------------------------------------------------
// Call before and after the rendering of the cell that uses the VRAM transmission.
static NNS_G2D_INLINE void EndDrawVramTransferedCell_( )
{
    NNS_G2D_ASSERT( currenVramTransferHandle_ != NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE );    
    currenVramTransferHandle_ = NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE;
}

//------------------------------------------------------------------------------
// Is the renderer currently rendering the VRAM transmission animation cell?
static NNS_G2D_INLINE BOOL IsRendererDrawingVramTransferedCell_( )
{
    return (BOOL)(currenVramTransferHandle_ != NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE);
}

//------------------------------------------------------------------------------
// Gets the handle of the current VRAM transfer animation cell.
static NNS_G2D_INLINE u32 GetCurrentVramTransfereHandle_()
{
    return currenVramTransferHandle_;
}



//------------------------------------------------------------------------------
// Registers the renderer core module to the callback, and customizes the operation.
// The renderer core module implements operation that is the same as existing modules through customization.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// callback before cell rendering
// * culling process
// * loading of current matrix to 2D matrix cache
// * callback call before cell rendering of renderer module
//
static void RndCoreCBFuncBeforeCell_
(
    struct NNSG2dRndCoreInstance*   pRend,
    const NNSG2dCellData*           pCell
)    
{   
    NNS_G2D_NULL_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCurrentInstance_->pCurrentSurface );
    {
       NNSG2dRenderSurface* pCurrentSurface = pCurrentInstance_->pCurrentSurface;
        
       //
       // culling process
       //
       if( pCurrentSurface->pFuncVisibilityCulling != NULL )
       { 
            
           if( !(*pCurrentSurface->pFuncVisibilityCulling)( pCell, 
                                                     NNSi_G2dGetCurrentMtx() , 
                                                     &pCurrentSurface->coreSurface.viewRect ) )
           {
              // 
              // configure to skip render in pRend
              // 
              pRend->bDrawEnable = FALSE;
              return;
           }else{
              pRend->bDrawEnable = TRUE;
           }
       }
       
       //
       // existing callback call
       //
       if( *pCurrentSurface->pBeforeDrawCellBackFunc )
       {   
           (*pCurrentSurface->pBeforeDrawCellBackFunc)( pCurrentInstance_,
                                                 pCurrentSurface,
                                                 pCell,
                                                 NNSi_G2dGetCurrentMtx() );
       }
    }
}
//------------------------------------------------------------------------------
// callback after cell rendering
//
// Registered as callback function in render core module.
static void RndCoreCBFuncAfterCell_
(
    struct NNSG2dRndCoreInstance*   pRend,
    const NNSG2dCellData*           pCell
)    
{
#pragma unused( pRend )
    NNS_G2D_NULL_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCurrentInstance_->pCurrentSurface );
    {
       NNSG2dRenderSurface* pCurrentSurface = pCurrentInstance_->pCurrentSurface;
    
       if( *pCurrentSurface->pAfterDrawCellBackFunc )
       {
           (*pCurrentSurface->pAfterDrawCellBackFunc)( pCurrentInstance_,
                                                 pCurrentSurface,
                                                 pCell,
                                                 NNSi_G2dGetCurrentMtx() );
       }
    }
}
//------------------------------------------------------------------------------
// callback before OBJ rendering
//
// Registered as callback function in render core module.
//
// Overwrites Oam parameters.
// 
// In the current implementation, parameters are overwritten before calling pBeforeDrawOamBackFunc. Therefore, when attempting to cull at the object level within pBeforeDrawOamBackFunc, it is possible parameter overwrites for objects not rendered will occur.
// 
// 
//
// There should be few OBJs for which individual OBJ culling needs to be carried out, and the current implementation is based on a decision to not worry about efficient processing in such cases.
// 
//
// 
static void RndCoreCBFuncBeforeOBJ_
(
    struct NNSG2dRndCoreInstance*   pRend,
    const NNSG2dCellData*           pCell,
    u16                             oamIdx
)
{
    GXOamAttr*    pTempOam = &pRend->currentOam;
    
    NNS_G2D_NULL_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCurrentInstance_->pCurrentSurface );
    
    //
    // write parameter
    //
    // palette conversion table
    OBJPaletteChangeHandling_( pTempOam );
    
    if( pCurrentInstance_->overwriteEnableFlag != NNS_G2D_RND_OVERWRITE_NONE )
    {
       // rendering priority
       if( NNS_G2dIsRendererOverwriteEnable( pCurrentInstance_, NNS_G2D_RND_OVERWRITE_PRIORITY ) )
       {
           pTempOam->priority = pCurrentInstance_->overwritePriority;
       }
        
       // palette number
       // Note: This takes priority. (Overwrites palette conversion table results.)
       if( NNS_G2dIsRendererOverwriteEnable( pCurrentInstance_, NNS_G2D_RND_OVERWRITE_PLTTNO ) )
       {
           pTempOam->cParam = pCurrentInstance_->overwritePlttNo;
       }
       
       // palette number (add offset)
       if( NNS_G2dIsRendererOverwriteEnable( pCurrentInstance_, NNS_G2D_RND_OVERWRITE_PLTTNO_OFFS ) )
       {
           pTempOam->cParam = 0xF & ( pTempOam->cParam + pCurrentInstance_->overwritePlttNoOffset );
       }
       
       // Mosaic
       if( NNS_G2dIsRendererOverwriteEnable( pCurrentInstance_, NNS_G2D_RND_OVERWRITE_MOSAIC ) )
       {
           G2_OBJMosaic( pTempOam, pCurrentInstance_->overwriteMosaicFlag );
       }
        
       // OBJ Mode
       if( NNS_G2dIsRendererOverwriteEnable( pCurrentInstance_, NNS_G2D_RND_OVERWRITE_OBJMODE ) )
       {
           G2_SetOBJMode( pTempOam, pCurrentInstance_->overwriteObjMode, G2_GetOBJColorParam(pTempOam));
       }
    }
    
    //
    // callback call
    //
    {
       NNSG2dRenderSurface* pCurrentSurface = pCurrentInstance_->pCurrentSurface;
       if( *pCurrentSurface->pBeforeDrawOamBackFunc )
       {
           (*pCurrentSurface->pBeforeDrawOamBackFunc)( pCurrentInstance_,
                                                 pCurrentSurface,
                                                 pCell,
                                                 oamIdx,
                                                 NNSi_G2dGetCurrentMtx() );
       }
    }
}
//------------------------------------------------------------------------------
// callback after rendering OBJ
//
// Registered as callback function in render core module.
static void RndCoreCBFuncAfterOBJ_
(
    struct NNSG2dRndCoreInstance*   pRend,
    const NNSG2dCellData*           pCell,
    u16                             oamIdx
)
{
#pragma unused( pRend )
    NNS_G2D_NULL_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCurrentInstance_->pCurrentSurface );
    {
       NNSG2dRenderSurface* pCurrentSurface = pCurrentInstance_->pCurrentSurface;
       if( *pCurrentSurface->pAfterDrawOamBackFunc )
       {
           (*pCurrentSurface->pAfterDrawOamBackFunc)( pCurrentInstance_,
                                                 pCurrentSurface,
                                                 pCell,
                                                 oamIdx,
                                                 NNSi_G2dGetCurrentMtx() );
       }
    }
}



//------------------------------------------------------------------------------
// Start rendering for render core 2D surface.
static NNS_G2D_INLINE void BeginRndCoreRendering2D_
(
    NNSG2dRendererInstance*  pRnd,
    NNSG2dRenderSurface*     pSurface 
)
{
    NNS_G2D_NULL_ASSERT( pRnd );
    NNS_G2D_ASSERT( pRnd->pCurrentSurface == NULL ); 
    NNS_G2D_ASSERT( pSurface->type != NNS_G2D_SURFACETYPE_MAIN3D );
    NNS_G2D_NULL_ASSERT( pSurface );
    
    //
    // Perform surface settings in render core
    //
    pRnd->pCurrentSurface = pSurface;
    NNS_G2dSetRndCoreSurface( &pRnd->rendererCore, &pSurface->coreSurface );
       
    //
    // Settings of registration function for 2D rendering
    //
    {
        {
            NNS_G2dSetRndCoreOamRegisterFunc( &pRnd->rendererCore,
                                              pSurface->pFuncOamRegister,
                                              pSurface->pFuncOamAffineRegister );
        }
    }
    
    NNS_G2dRndCoreBeginRendering( &pRnd->rendererCore );
}

//------------------------------------------------------------------------------
// Starts rendering for render core 3D surface.
static NNS_G2D_INLINE void BeginRndCoreRendering3D_
(
    NNSG2dRendererInstance*  pRnd, 
    NNSG2dRenderSurface*     pSurface 
)
{
    NNS_G2D_NULL_ASSERT( pRnd );
    NNS_G2D_ASSERT( pRnd->pCurrentSurface == NULL ); 
    NNS_G2D_ASSERT( pSurface->type == NNS_G2D_SURFACETYPE_MAIN3D );
    NNS_G2D_NULL_ASSERT( pSurface );
    
    //
    // Perform surface settings in render core
    //
    pRnd->pCurrentSurface = pSurface;
    NNS_G2dSetRndCoreSurface( &pRnd->rendererCore, &pSurface->coreSurface );
       
    
    NNS_G2dRndCoreBeginRendering( &pRnd->rendererCore );
}

//------------------------------------------------------------------------------
// Ends rendering of render core.
static NNS_G2D_INLINE void EndRndCoreRendering_( void )
{
    NNS_G2D_NULL_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCurrentInstance_->pCurrentSurface );
    
    pCurrentInstance_->pCurrentSurface = NULL;
    NNS_G2dRndCoreEndRendering( );
}


//------------------------------------------------------------------------------
// render core module 2D rendering
// Made a separate function to improve the readability of the DrawCellImpl_ function.
static NNS_G2D_INLINE void DoRenderByRndCore2D_( 
    const NNSG2dCellData*    pCell ,
    NNSG2dRndCoreInstance*   pRndCore
)
{
#pragma unused( pRndCore )
    NNS_G2D_NULL_ASSERT( pCell );
    NNS_G2D_NULL_ASSERT( pRndCore );
    //
    // Sets matrix cache in renderer.
    // (z value must be set in advance.)
    //
    {
       NNSG2dRndCore2DMtxCache* pMtx2D = NULL;

       //
       // If affine converting, sets MtxCache for 2D affine conversion.
       //
       if( NNSi_G2dIsRndCurrentMtxSRTransformed() )
       {
           //
           // if rendering a multicell...
           //
           if( mcRenderState_.bDrawMC )
           {
               //
               // Checks table to see whether it was previously loaded.
               // Identical cell animations in a multicell must reference the same affine parameters.
               // The renderer uses affine transformation, and stores the rendered cell animation matrix cache in a table.
               //
               pMtx2D 
                  = mcRenderState_.cellAnimMtxCacheTbl[mcRenderState_.currentCellAnimIdx];
               //
               // for cell animations that have never been rendered...
               //
               if( pMtx2D == NULL )
               {
                  //
                  // Loads the current matrix into matrix cache.
                  //
                  NNSi_G2dMCMStoreCurrentMtxToMtxCache();
                  //
                  // Gets matrix cache of loaded matrix.
                  //
                  pMtx2D = NNSi_G2dMCMGetCurrentMtxCache();
                  //
                  // Stores the matrix cache in a table.
                  //
                  mcRenderState_.cellAnimMtxCacheTbl[mcRenderState_.currentCellAnimIdx] = pMtx2D;
              }
           }else{
               //
               // Loads the current matrix into matrix cache.
               //
               NNSi_G2dMCMStoreCurrentMtxToMtxCache();
               //
               // Gets matrix cache of loaded matrix.
               //
               pMtx2D = NNSi_G2dMCMGetCurrentMtxCache();
           }
       }                                                   
       //
       // sets matrix cache for affine conversion
       //
       NNS_G2dSetRndCoreCurrentMtx2D( NNSi_G2dGetCurrentMtx(), pMtx2D );
    }
    
    
    //
    // passes render core to render process
    //
    if( IsRendererDrawingVramTransferedCell_( ) )
    {
        NNS_G2dRndCoreDrawCellVramTransfer( pCell, GetCurrentVramTransfereHandle_() );
    }else{
        NNS_G2dRndCoreDrawCell( pCell );
    }
}
              
//------------------------------------------------------------------------------
// render core module 3D rendering
// Made a separate function to improve the readability of the DrawCellImpl_ function.
static NNS_G2D_INLINE void DoRenderByRndCore3D_
( 
    const NNSG2dCellData*    pCell ,
    NNSG2dRndCoreInstance*   pRndCore
)
{
    NNS_G2D_NULL_ASSERT( pCell );
    NNS_G2D_NULL_ASSERT( pRndCore );
    
    //
    // Sets the z value for 3D.
    //
    NNS_G2dSetRndCore3DSoftSpriteZvalue( pRndCore, NNSi_G2dGetCurrentZ() );
    //
    // Set current matrix in renderer core.
    // (z value must be set in advance.)
    //
    NNS_G2dSetRndCoreCurrentMtx3D( NNSi_G2dGetCurrentMtx() );
    //
    // passes render core to render process
    //
    if( IsRendererDrawingVramTransferedCell_( ) )
    {
        NNS_G2dRndCoreDrawCellVramTransfer( pCell, GetCurrentVramTransfereHandle_() );
    }else{
        NNS_G2dRndCoreDrawCell( pCell );
    }
}

//------------------------------------------------------------------------------
// Renders cell.
static void DrawCellImpl_( const NNSG2dCellData* pCell )
{
    NNSG2dRndCoreInstance*   pRndCore   = NULL;
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCell );

    pRndCore = &pCurrentInstance_->rendererCore;
    
    {
        NNSG2dRenderSurface*      pSurface = pCurrentInstance_->pTargetSurfaceList;
        //
        // When only one surface is used, the configuring of surface parameters in the render core can be avoided for each cell rendering.
        // 
        //
        if( pCurrentInstance_->opzHint & NNS_G2D_RDR_OPZHINT_LOCK_PARAMS )
        {
           // 
           // Calls to the BeginRndCoreRenderingXX_ and EndRndCoreRendering_ functions for each rendering function are not made. 
           // This speeds up the operation.
           // 
           //
           // if rendering for a 2D surface...
           //       
           if( pSurface->type != NNS_G2D_SURFACETYPE_MAIN3D )
           {
              //
              // actual rendering process (2D)
              //                    
              DoRenderByRndCore2D_( pCell, pRndCore );    
           }else{
              //
              // actual render process (3D)
              //                    
              DoRenderByRndCore3D_( pCell, pRndCore );
           }
        }else{
           // for each render surface...     
           while( pSurface )
           {
              if( pSurface->bActive )
              {                
                  //
                  // if rendering for a 2D surface...
                  //
                  if( pSurface->type != NNS_G2D_SURFACETYPE_MAIN3D )
                  {
                     //
                     // actual rendering process (2D)
                     //                    
                     BeginRndCoreRendering2D_( pCurrentInstance_, pSurface );
                         DoRenderByRndCore2D_( pCell, pRndCore );
                     EndRndCoreRendering_();
                  }else{
                     //
                     // actual render process (3D)
                     //                    
                     BeginRndCoreRendering3D_( pCurrentInstance_, pSurface );
                         DoRenderByRndCore3D_( pCell, pRndCore );
                     EndRndCoreRendering_();
                  }
              }
              // next surface ... 
              pSurface = pSurface->pNextSurface;
           }
        }
    }
}



//------------------------------------------------------------------------------
static void DrawCellAnimationImpl_( const NNSG2dCellAnimation* pCellAnim )
{
    NNSG2dCellData*      pCell = NULL;
    NNS_G2D_NULL_ASSERT( pCellAnim );
    
    pCell = (NNSG2dCellData*)NNS_G2dGetCellAnimationCurrentCell( pCellAnim );
    NNS_G2D_NULL_ASSERT( pCell );
           
    //
    // When SRT animation is not used, avoids useless PushPop.
    //
    if( pCellAnim->srtCtrl.srtData.SRT_EnableFlag == NNS_G2D_AFFINEENABLE_NONE )
    {
           //
           // if this is a cell that uses the VRAM transmission animation...
           //
           if( NNSi_G2dIsCellAnimVramTransferHandleValid( pCellAnim ) )
           {
                
              BeginDrawVramTransferedCell_( NNSi_G2dGetCellAnimVramTransferHandle( pCellAnim ) );
                  DrawCellImpl_( pCell );
              EndDrawVramTransferedCell_();
                
           }else{
                  DrawCellImpl_( pCell );
           }
    }else{
       NNS_G2dPushMtx(); 
           SetSrtControlToMtxStack_( &pCellAnim->srtCtrl );        
           //
           // if this is a cell that uses the VRAM transmission animation...
           //
           if( NNSi_G2dIsCellAnimVramTransferHandleValid( pCellAnim ) )
           {
                
              BeginDrawVramTransferedCell_( NNSi_G2dGetCellAnimVramTransferHandle( pCellAnim ) );
                  DrawCellImpl_( pCell );
              EndDrawVramTransferedCell_();
                
           }else{
                  DrawCellImpl_( pCell );
           }     
       NNS_G2dPopMtx(1);
    }
}


//------------------------------------------------------------------------------
// Renders Node.
// This has been changed from externally released function to internal function.
static void DrawNode_( const NNSG2dNode* pNode )
{
    NNS_G2D_NULL_ASSERT( pNode );
    NNS_G2D_ASSERTMSG( pNode->type == NNS_G2D_NODETYPE_CELL, "NNS_G2D_NODETYPE_CELL is expected." );
    
    if( pNode->bVisible )
    {
        //
        // TODO:
        // code below this line should be "pNode->type" dependent
        // for now, we expect that "pNode->type" is always "NNS_G2D_NODETYPE_CELL"
        //        
        NNSG2dCellAnimation*    pCellAnim = (NNSG2dCellAnimation*)pNode->pContent;
        NNS_G2D_NULL_ASSERT( pCellAnim );
        
        //
        // Since there should be few cases where pNode->srtCtrl.srtData.SRT_EnableFlag == NNS_G2D_AFFINEENABLE_NONE, there won't be any avoiding of push and pop through conditional branching as seen elsewhere.
        // 
        // 
        // 
        NNS_G2dPushMtx();
            SetSrtControlToMtxStack_( &pNode->srtCtrl );    
            {                       
                DrawCellAnimationImpl_( pCellAnim );
            }
        NNS_G2dPopMtx(1);
    }
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void DrawNode2_
( 
    const NNSG2dMultiCellInstance*         pMC, 
    const NNSG2dMultiCellHierarchyData*    pNodeData 
)
{
    const u16 cellAnimIdx          = NNSi_G2dGetMC2NodeCellAinmIdx( pNodeData );
    const NNSG2dMCCellAnimation*   cellAnimArray = pMC->pCellAnimInstasnces;
    NNS_G2D_MINMAX_ASSERT( cellAnimIdx, 0, pMC->pCurrentMultiCell->numCellAnim );
    mcRenderState_.currentCellAnimIdx = cellAnimIdx;
    
    NNS_G2dPushMtx();
       FlipTranslate_( pNodeData->posX, pNodeData->posY );
       {                       
           DrawCellAnimationImpl_( &cellAnimArray[cellAnimIdx].cellAnim );
       }
    NNS_G2dPopMtx(1);
}

//------------------------------------------------------------------------------
// Is the automatic z value offset valid?
static NNS_G2D_INLINE BOOL IsAutoZoffsetEnable_( void )
{
    NNS_G2D_NULL_ASSERT( pCurrentInstance_ );
    return (BOOL)pCurrentInstance_->spriteZoffsetStep;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitRenderer

  Description:  Initializes the renderer instance.
                
                
  Arguments:    pRend:      [OUT] renderer instance
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitRenderer( NNSG2dRendererInstance* pRend )
{
    NNS_G2D_NULL_ASSERT( pRend );
    
    NNS_G2dInitRndCore( &pRend->rendererCore );
    
    pRend->pTargetSurfaceList   = NULL;
    pRend->pCurrentSurface      = NULL;
    pRend->pPaletteSwapTbl      = NULL;
    
    //
    // optimization hint is invalid as default value (optimization not performed)
    //
    pRend->opzHint = NNS_G2D_RDR_OPZHINT_NONE;
    
    pRend->spriteZoffsetStep = 0;
    
    // The overwriteEnableFlag is NNSG2dRendererOverwriteParam.
    pRend->overwriteEnableFlag = NNS_G2D_RND_OVERWRITE_NONE; 
    pRend->overwritePriority   = 0;
    pRend->overwritePlttNo     = 0;  
    pRend->overwriteObjMode    = GX_OAM_MODE_NORMAL;
    pRend->overwriteMosaicFlag = FALSE;
    pRend->overwritePlttNoOffset = 0;
    
    //
    // initialization of matrix cache
    //
    NNSi_G2dMCMInitMtxCache();
    //
    // Sets use mode of matrix stack module.
    //
    NNSi_G2dSetRndMtxStackSRTransformEnableFlag( TRUE );
    
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dAddRendererTargetSurface

  Description:  Adds the NNSG2dRenderSurface to the renderer.
                
                
  Arguments:    pRend:      [OUT] renderer instance
                pNew:       [IN] NNSG2dRenderSurface to add
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dAddRendererTargetSurface( NNSG2dRendererInstance* pRend, NNSG2dRenderSurface* pNew )
{
    NNS_G2D_NULL_ASSERT( pRend );
    NNS_G2D_NULL_ASSERT( pNew );
    NNS_G2D_ASSERTMSG( IsNotCircularLinked_( pRend->pTargetSurfaceList, pNew ),
        "Circular linked lists is detected in NNS_G2dAddRendererTargetSurface()" );
    
    // add_front
    pNew->pNextSurface          = pRend->pTargetSurfaceList;
    pRend->pTargetSurfaceList   = pNew;
    
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitRenderSurface

  Description:  Initializes the render surface.
                Callback function is registered internally in renderer core module.
                
                
  Arguments:    pSurface:      [OUT] render surface instance
                
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitRenderSurface( NNSG2dRenderSurface* pSurface )
{
    NNS_G2D_NULL_ASSERT( pSurface );
    
    MI_CpuFill16( pSurface, 0x0, sizeof( NNSG2dRenderSurface ) );
    
    pSurface->coreSurface.bActive = TRUE;
    
    pSurface->coreSurface.type = NNS_G2D_SURFACETYPE_MAX;
    
    //
    // Registers callback function in renderer core module.
    //
    {
        NNSG2dRndCoreSurface* pS = &pSurface->coreSurface;
        
        pS->pBeforeDrawCellBackFunc   = RndCoreCBFuncBeforeCell_;
        pS->pAfterDrawCellBackFunc    = RndCoreCBFuncAfterCell_;
        pS->pBeforeDrawOamBackFunc    = RndCoreCBFuncBeforeOBJ_;
        pS->pAfterDrawOamBackFunc     = RndCoreCBFuncAfterOBJ_;
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBeginRendering

  Description:  Makes settings necessary to begin rendering with renderer.
                Call before calling the rendering method of the renderer.
                Do not call from within Begin End Rendering.
                
                Also, the NNS_G2dBeginRenderingEx() function is provided, which can specify hint flags for rendering optimization, and fulfill a role similar to this function.
                
                
                
  Arguments:    pRendererInstance:      [IN]  renderer instance
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dBeginRendering( NNSG2dRendererInstance* pRendererInstance )
{
    NNS_G2D_NULL_ASSERT( pRendererInstance );
    NNS_G2D_ASSERTMSG( pCurrentInstance_ == NULL, 
        "Must be NULL, Make sure calling Begin - End correctly." );
    
    pCurrentInstance_ = pRendererInstance;
    
    NNSi_G2dMCMCleanupMtxCache();
    
    G3_PushMtx();            
    
    G3_Identity();
    NNSi_G2dIdentity();
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dBeginRenderingEx

  Description:  Makes settings necessary to begin rendering with renderer.
                Call before calling the rendering method of the renderer.
                Do not call from within Begin End Rendering.
                For this function, hint flags can be specified to optimize rendering.
                
                The hint is created with a logical OR of the NNSG2dRendererOptimizeHint enumerated type.
                
                
                Optimization hint flags are reset with NNS_G2dEndRendering().
                I.e., the optimized hint flag is only valid within the renderer's Begin and End Rendering block.
                
                
                This function normally calls the NNS_G2dBeginRendering function after completing the pre-processing for optimization.
                
                
                
  Arguments:    pRendererInstance       [IN]  renderer 
                opzHint                 [IN]  optimization hint
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dBeginRenderingEx 
( 
    NNSG2dRendererInstance* pRendererInstance, 
    u32                     opzHint 
)
{
    NNS_G2D_NULL_ASSERT( pRendererInstance );
    NNS_G2D_ASSERT( pRendererInstance->opzHint == NNS_G2D_RDR_OPZHINT_NONE );
    
    pRendererInstance->opzHint = opzHint;
    //
    // preprocessing for rendering optimization
    //
    {
       if( opzHint & NNS_G2D_RDR_OPZHINT_NOT_SR )
       {
           NNSi_G2dSetRndMtxStackSRTransformEnableFlag( FALSE );
       }
       
       //
       // When only one surface is used, the configuring of surface parameters in the render core can occur only once at the Begin&#8212;End timing to lessen the processing load for each cell rendering.
       // 
       //
       if( opzHint & NNS_G2D_RDR_OPZHINT_LOCK_PARAMS )
       {
           NNSG2dRndCoreInstance*  pRndCore = &pRendererInstance->rendererCore;
           NNSG2dRenderSurface*    pSurface = pRendererInstance->pTargetSurfaceList;
            
           NNS_G2D_ASSERTMSG( pSurface->pNextSurface == NULL,
              "The number of target surface must be ONE. when you spesified the NNS_G2D_RDR_OPZHINT_LOCK_PARAMS flag." );  
              
           if( pSurface->bActive )
           {                
              //
              // if rendering for a 2D surface...
              //
              if( pSurface->type != NNS_G2D_SURFACETYPE_MAIN3D )
              {
                  BeginRndCoreRendering2D_( pRendererInstance, pSurface );
              }else{
                  BeginRndCoreRendering3D_( pRendererInstance, pSurface );
              }
           }
       }
    }

    
    NNS_G2dBeginRendering( pRendererInstance );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dEndRendering

  Description:  Performs settings after render.
                The modified internal state is restored to its original status after rendering.
                
                When an optimized hint is specified (and the NNS_G2dBeginRenderingEx function is used), optimization is post-processing.
                
                
                Optimization hint flags are reset by this function.
                I.e., the optimized hint flag is only valid within the renderer's Begin and End Rendering block.
                
                
                
  Arguments:    None.
  
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dEndRendering()
{
    // check that push pop are called correctly
    NNS_G2D_NULL_ASSERT( pCurrentInstance_ );
    
    
    G3_PopMtx(1);
    
    {
        //
        // If optimization flag is set, performs that process.
        //
        const u32 opzHint = pCurrentInstance_->opzHint;
        if( opzHint != NNS_G2D_RDR_OPZHINT_NONE )
        {
            //
            // Restores SR conversion settings of stack to the original settings.
            //
            if( opzHint & NNS_G2D_RDR_OPZHINT_NOT_SR )
            {
                NNSi_G2dSetRndMtxStackSRTransformEnableFlag( TRUE );
            }
            
            //
            // When only one surface is used, the configuring of surface parameters in the render core can occur only once at the Begin&#8212;End Rendering timing to lessen the processing load for each cell rendering.
            // 
            //
            if( opzHint & NNS_G2D_RDR_OPZHINT_LOCK_PARAMS )
            {
                EndRndCoreRendering_();
            }
            //
            // Resets optimization flag.
            // (In other words, optimization flag is only valid in Begin - End.)
            //
            pCurrentInstance_->opzHint = NNS_G2D_RDR_OPZHINT_NONE;
        }    
    }
    
    pCurrentInstance_ = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dDrawCell

  Description:  Renders the cell.
                
                
  Arguments:    pCell: [IN] cell instance to render             
                                  
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dDrawCell( const NNSG2dCellData* pCell )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCell );
    
    if( IsAutoZoffsetEnable_() )
    {
        const fx32 offset = NNSi_G2dGetOamSoftEmuAutoZOffsetStep();
        NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( TRUE );
        NNSi_G2dSetOamSoftEmuAutoZOffsetStep( pCurrentInstance_->spriteZoffsetStep );
        
        DrawCellImpl_( pCell );
        
        NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( FALSE );
        NNSi_G2dSetOamSoftEmuAutoZOffsetStep( offset );
        NNSi_G2dResetOamSoftEmuAutoZOffset();
    }else{
        DrawCellImpl_( pCell );
    }
}
   


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dDrawCellAnimation

  Description:  Renders the cell animation.
                
                
  Arguments:    pMC: [IN] cell animation to render        
                                  
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dDrawCellAnimation( const NNSG2dCellAnimation* pCellAnim )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pCellAnim );
    
    if( IsAutoZoffsetEnable_() )
    {
        const fx32 offset = NNSi_G2dGetOamSoftEmuAutoZOffsetStep();
        NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( TRUE );
        NNSi_G2dSetOamSoftEmuAutoZOffsetStep( pCurrentInstance_->spriteZoffsetStep );
        
        DrawCellAnimationImpl_( pCellAnim );
        
        NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( FALSE );
        NNSi_G2dSetOamSoftEmuAutoZOffsetStep( offset );
        NNSi_G2dResetOamSoftEmuAutoZOffset();
    }else{
        DrawCellAnimationImpl_( pCellAnim );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dDrawMultiCell

  Description:  Renders the multicell.
                
                
  Arguments:    pMC: [IN]  multicell instance to render             
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dDrawMultiCell
( 
    const NNSG2dMultiCellInstance*      pMC 
)
{
    u16 i;
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pMC );
    if( pMC->mcType == NNS_G2D_MCTYPE_SHARE_CELLANIM )
    {
       //
       // Initializes only the number of cell animations that use the multicell 2D rendering matrix cache table.
       //
       for( i = 0; i < pMC->pCurrentMultiCell->numCellAnim; i++ )
       {
           mcRenderState_.cellAnimMtxCacheTbl[i] = NULL;
       }
       mcRenderState_.bDrawMC = TRUE;
        
       if( IsAutoZoffsetEnable_() )
       {
           const fx32 offset = NNSi_G2dGetOamSoftEmuAutoZOffsetStep();
           NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( TRUE );
           NNSi_G2dSetOamSoftEmuAutoZOffsetStep( pCurrentInstance_->spriteZoffsetStep );
            
           for( i = 0; i < pMC->pCurrentMultiCell->numNodes; i++ )
           {
              DrawNode2_( pMC, &pMC->pCurrentMultiCell->pHierDataArray[i] );
           }
            
           NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( FALSE );
           NNSi_G2dSetOamSoftEmuAutoZOffsetStep( offset );
           NNSi_G2dResetOamSoftEmuAutoZOffset();
       }else{
           for( i = 0; i < pMC->pCurrentMultiCell->numNodes; i++ )
           {
              DrawNode2_( pMC, &pMC->pCurrentMultiCell->pHierDataArray[i] );
           }
       }
        
       mcRenderState_.bDrawMC = FALSE;
    
    }else{
       const NNSG2dNode* pNode = pMC->pCellAnimInstasnces;
       if( IsAutoZoffsetEnable_() )
       {
           const fx32 offset = NNSi_G2dGetOamSoftEmuAutoZOffsetStep();
           
           NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( TRUE );
           NNSi_G2dSetOamSoftEmuAutoZOffsetStep( pCurrentInstance_->spriteZoffsetStep );
            
           for( i = 0; i < pMC->pCurrentMultiCell->numNodes; i++ )
           {
              DrawNode_( &pNode[i] );
           }
            
           NNSi_G2dSetOamSoftEmuAutoZOffsetFlag( FALSE );
           NNSi_G2dSetOamSoftEmuAutoZOffsetStep( offset );
           NNSi_G2dResetOamSoftEmuAutoZOffset();
       }else{
           for( i = 0; i < pMC->pCurrentMultiCell->numNodes; i++ )
           {
              DrawNode_( &pNode[i] );
           }
       }
    }
}






/*---------------------------------------------------------------------------*
  Name:         NNS_G2dDrawMultiCellAnimation

  Description:  Renders multicell animation.
                Calls NNS_G2dDrawMultiCell() internally.
                
                The consideration of the multicell animation's SRT animation results is what makes this different from the NNS_G2dDrawMultiCell function.
                
                
                
  Arguments:    pMCAnim: [IN]  multicell animation to render
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dDrawMultiCellAnimation
( 
    const NNSG2dMultiCellAnimation*     pMCAnim 
)
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pMCAnim );
    
    if( pMCAnim->srtCtrl.srtData.SRT_EnableFlag == NNS_G2D_AFFINEENABLE_NONE )
    {
        NNS_G2dDrawMultiCell( &pMCAnim->multiCellInstance );    
    }else{
        NNS_G2dPushMtx();
            SetSrtControlToMtxStack_( &pMCAnim->srtCtrl );
            NNS_G2dDrawMultiCell( &pMCAnim->multiCellInstance );    
        NNS_G2dPopMtx(1);
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dDrawEntity

  Description:  Renders the entity.
                When the palette conversion table has the entity data, rendering uses the palette conversion table.
                
                
                
  Arguments:    pEntity: [IN]  entity instance to render             
                               
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dDrawEntity( NNSG2dEntity* pEntity )
{
    BOOL bAffined = FALSE;
    
    BOOL bPaletteChange                 = FALSE;
    const NNSG2dPaletteSwapTable* pTbl  = NULL;
    
    
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_ASSERT_ENTITY_VALID( pEntity );
    
    //
    // configure the palette
    //
    bPaletteChange = NNS_G2dIsEntityPaletteTblEnable( pEntity );
    if( bPaletteChange )
    {
        pTbl = NNS_G2dGetRendererPaletteTbl( pCurrentInstance_ );
        NNS_G2dSetRendererPaletteTbl( pCurrentInstance_, pEntity->pPaletteTbl );
    }
    
    //
    // Renders according to the type of the entity.
    //
    {       
        switch( pEntity->pEntityData->type )
        {
        case NNS_G2D_ENTITYTYPE_CELL:
            {
                NNSG2dCellAnimation*  
                  pCellAnim = (NNSG2dCellAnimation*)pEntity->pDrawStuff;
                NNS_G2D_NULL_ASSERT( pCellAnim );
                NNS_G2dDrawCellAnimation( pCellAnim );
            }
            break;
        case NNS_G2D_ENTITYTYPE_MULTICELL:
            {
                NNSG2dMultiCellAnimation*  
                  pMCAnim = (NNSG2dMultiCellAnimation*)pEntity->pDrawStuff;
                NNS_G2D_NULL_ASSERT( pMCAnim );
                NNS_G2dDrawMultiCellAnimation( pMCAnim );
            }
            break;
        default:
            NNS_G2D_ASSERTMSG( FALSE, "TODO: msg ");
        }    
    }
    
    //
    // Restore the palette to its original state.
    //
    if( bPaletteChange )
    {
        if( pTbl != NULL )
        {
            NNS_G2dSetRendererPaletteTbl( pCurrentInstance_, pTbl );
        }else{
            NNS_G2dResetRendererPaletteTbl( pCurrentInstance_ );
        }
    }
}




/*---------------------------------------------------------------------------*
  Name:         NNS_G2dPushMtx 

  Description:  Operates the matrix stack internal to the renderer.
                The matrix stack is processed by the CPU.
                
                Because the parsing of the scale parameter differs in the 2D and 3D graphics engines, internally there are two matrix stacks, one for 2D and one for 3D (NNS_G2dScale).
                
                
                
  Arguments:    None.
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dPushMtx()
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    //
    // For the render module, matrix calculations now occur in the CPU, regardless of the load on the graphics engine, since it seems to benefit performance.
    // 
    //
    if( !(pCurrentInstance_->opzHint & NNS_G2D_RDR_OPZHINT_NOT_SR) )
    {
        const u16 lastPos = NNSi_G2dGetMtxStackPos();
        NNSi_G2dMtxPush();
        {
            const u16 newPos = NNSi_G2dGetMtxStackPos();
            //
            // updates the state of the current matrix
            //
            NNSi_G2dMCMSetMtxStackPushed( newPos, lastPos );
        }
    }else{
        NNSi_G2dMtxPush();
    }    
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dPopMtx 

  Description:  Pops the matrix stack inside the renderer.
                
  Arguments:    None.
                 
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dPopMtx()
{ 
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNSi_G2dMtxPop();
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTranslate 

  Description:  Multiplies the current matrix by the translation matrix in the renderer.
                
  Arguments:    x:      [IN]  trans x
                y:      [IN]  trans y
                z:      [IN]  trans z
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dTranslate(fx32 x, fx32 y, fx32 z )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNSi_G2dTranslate( x, y, z );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTranslate 

  Description:  Sets translation component of current matrix in renderer.
                
  Arguments:    x:      [IN]  trans x
                y:      [IN]  trans y
                z:      [IN]  trans z
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetTrans(fx32 x, fx32 y, fx32 z )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNSi_G2dSetTrans( x, y, z );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dScale 

  Description:  Multiplies the current matrix by the scale matrix in the renderer.
                
  Arguments:    x:      [IN]  scale x
                y:      [IN]  scale y
                z:      [IN]  scale z
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dScale(fx32 x, fx32 y, fx32 z )
{
#pragma unused( z )
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNSi_G2dScale( x, y );
    // 
    // When including the affine transform, the Mtx must be applied to HW. (2D Graphics Engine)
    // If the current SR transform is not included, the status is set to newly include the SR transform.
    //      
    //
    if( !NNSi_G2dIsRndCurrentMtxSRTransformed() )
    {
        NNS_G2D_WARNING( pCurrentInstance_->rendererCore.flipFlag == NNS_G2D_RENDERERFLIP_NONE, 
                    "You can't use affine transformation using flip function." );
        NNSi_G2dSetRndMtxStackSRTransformed();
    }
    
    NNSi_G2dMCMSetCurrentMtxSRChanged();
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRotZ 

  Description:  Multiplies the current matrix by the rotation matrix in the renderer.
                
  Arguments:    sin:      [IN]  sine value
                cos:      [IN]  cosine value
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRotZ( fx32 sin, fx32 cos )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT( pCurrentInstance_ );
    NNSi_G2dRotate( sin, cos );
    
    // 
    // When including the SR transform, the Mtx must be applied to HW. (2D Graphics Engine)
    // If the current SR transform is not included, the status is set to newly include the SR transform.
    //      
    //
    if( !NNSi_G2dIsRndCurrentMtxSRTransformed() )
    {
        NNS_G2D_WARNING( pCurrentInstance_->rendererCore.flipFlag == NNS_G2D_RENDERERFLIP_NONE, 
                    "You can't use affine transformation using flip function." );
        NNSi_G2dSetRndMtxStackSRTransformed();
    }
    
    NNSi_G2dMCMSetCurrentMtxSRChanged();   
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRendererFlipMode 

  Description:  Configures the flip rendering settings of the renderer.
                Caution:
                    The affine conversion features cannot be used if the flip rendering is in effect.
                
                Can be called either inside or outside of the Begin - End rendering block.
                
                
  Arguments:    pRend:       [OUT] renderer instance
                bFlipH:      [IN]  Using H flip?
                bFlipV:      [IN]  Using V flip?
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRendererFlipMode
( 
    NNSG2dRendererInstance* pRend, 
    BOOL bFlipH, 
    BOOL bFlipV 
)
{    
    NNS_G2D_WARNING( !NNSi_G2dIsRndCurrentMtxSRTransformed(), 
       "You can't use the flip function using affine transformation." );
    NNS_G2D_NULL_ASSERT( pRend );
    
    NNS_G2dSetRndCoreFlipMode( &pRend->rendererCore, bFlipH, bFlipV );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRendererPaletteTbl 

  Description:  Configures the palette conversion table settings in the renderer instance.
  
                Can be called either inside or outside of the Begin - End rendering block.
                
  Arguments:    pRend:      [OUT] renderer instance
                pTbl:       [IN] palette conversion table
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRendererPaletteTbl
( 
    NNSG2dRendererInstance*         pRend, 
    const NNSG2dPaletteSwapTable*   pTbl 
)
{
    NNS_G2D_NULL_ASSERT( pRend );
    NNS_G2D_NULL_ASSERT( pTbl );
    
    pRend->pPaletteSwapTbl = pTbl;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetRendererPaletteTbl TODO:

  Description:  Gets the color palette conversion table settings of the renderer instance.
                
                Can be called either inside or outside of the Begin - End rendering block.
                
  Arguments:    pRend:      [OUT] renderer instance
                
  Returns:      color palette conversion table
  
 *---------------------------------------------------------------------------*/
const NNSG2dPaletteSwapTable* 
NNS_G2dGetRendererPaletteTbl( NNSG2dRendererInstance* pRend )
{
    NNS_G2D_NULL_ASSERT( pRend );
    
    return pRend->pPaletteSwapTbl;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dResetRendererPaletteTbl 

  Description:  Resets the color palette conversion table settings in the renderer instance.
  
                Can be called either inside or outside of the Begin - End rendering block.
                
  Arguments:    pRend:      [OUT] renderer instance
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dResetRendererPaletteTbl( NNSG2dRendererInstance* pRend )
{
    NNS_G2D_NULL_ASSERT( pRend );
    pRend->pPaletteSwapTbl = NULL;
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRendererImageProxy 

  Description:  Configures the image information and the palette information in the renderer instance.
                Execute this function before using the renderer instance.
                
                
                This function can also be used inside the Begin End Rendering() block.
                
                Caution:
                When rendering occurs with the NNS_G2D_RDR_OPZHINT_LOCK_PARAMS optimization flag specified, use is prohibited within the Begin&#8212;End Rendering block.
                
                
                This is because the render core module's Begin&#8212;End Rendering call occurs at the same time as that of the renderer module.
                
                
                
                In the renderer core module, it is forbidden to change the image proxy settings inside Begin-End Rendering().
                You cannot switch to image proxy settings within the Begin&#8212;End Rendering block.
                (This is to perform preprocessing of ImageProxy-related parameters in the Begin Rendering function.)
                
                
                
                
                
  Arguments:    pRend:      [OUT] renderer instance
                pImgProxy:  [IN]  image information configured in the renderer
                pPltProxy:  [IN]  palette information configured in the renderer
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRendererImageProxy
( 
    NNSG2dRendererInstance*             pRend, 
    const NNSG2dImageProxy*             pImgProxy, 
    const NNSG2dImagePaletteProxy*      pPltProxy
)
{
    NNS_G2D_NULL_ASSERT( pRend );
    NNS_G2D_NULL_ASSERT( pImgProxy );
    NNS_G2D_NULL_ASSERT( pPltProxy );
    
    SDK_WARNING( pImgProxy->attr.bExtendedPlt == pPltProxy->bExtendedPlt, 
        "Palette type mismatching was detected.\n Make sure that you use the correct palette." );
    
    NNS_G2D_WARNING( !(pRend->opzHint & NNS_G2D_RDR_OPZHINT_LOCK_PARAMS),
        "Avoid calling this function, when you specified the optimize flag NNS_G2D_RDR_OPZHINT_LOCK_PARAMS." );
    
    NNS_G2dSetRndCoreImageProxy( &pRend->rendererCore, pImgProxy, pPltProxy );
}

