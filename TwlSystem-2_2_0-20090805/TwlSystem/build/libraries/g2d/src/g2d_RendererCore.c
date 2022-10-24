/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_RendererCore.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#include <nnsys/g2d/g2d_RendererCore.h>
#include "g2di_RendererMtxCache.h"      // For MatrixCache & State


#include <nnsys/g2d/fmt/g2d_Cell_data.h>
#include <nnsys/g2d/g2d_OamSoftwareSpriteDraw.h>
#include <nnsys/g2d/g2d_CellTransferManager.h>




#define NNSi_G2D_RNDCORE_DUMMY_FLAG                FALSE
#define NNSi_G2D_RNDCORE_OAMENTORY_SUCCESSFUL      TRUE



static NNSG2dRndCoreInstance*     pTheInstance_ = NULL;
static const MtxFx32              mtxIdentity_ = { FX32_ONE,        0, 
                                                          0, FX32_ONE, 
                                                          0,        0 };
// if the function is called in the Begin - End Rendering block 
#define NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT(  ) \
       NNS_G2D_ASSERTMSG( pTheInstance_ != NULL, "Please call this method between Begin - End Rendering" ) 

// if the function is called outside the Begin - End Rendering block 
#define NNS_G2D_RND_OUTSIDE_BEGINEND_ASSERT(  ) \
       NNS_G2D_ASSERTMSG( pTheInstance_ == NULL, "Please call this method outside Begin - End Rendering" ) 



//------------------------------------------------------------------------------
// MtxCache affine Index accessor
// 
//    NNS_G2D_SURFACETYPE_MAIN3D = 0x00,  // use 3D Graphics Engine
//    NNS_G2D_SURFACETYPE_MAIN2D = 0x01,  // use 2D Graphics Engine A
//    NNS_G2D_SURFACETYPE_SUB2D  = 0x02,  // use 2D Graphics Engine B
//    NNS_G2D_SURFACETYPE_MAX    = 0x03
//
// Note: Processing is closely dependent on the enum values. 
//
static NNS_G2D_INLINE u16 GetMtxCacheAffineIndex_
( 
    const NNSG2dRndCore2DMtxCache*     pMtxCache,
    NNSG2dSurfaceType                  surfaceType,
    OAM_FLIP                           flipType
)
{
    return pMtxCache->affineIndex[flipType][surfaceType - NNS_G2D_SURFACETYPE_MAIN2D];
}

//------------------------------------------------------------------------------
// 
//
static NNS_G2D_INLINE void SetMtxCacheAffineIndex_
( 
    NNSG2dRndCore2DMtxCache*    pMtxCache,
    NNSG2dSurfaceType           surfaceType,
    OAM_FLIP                    flipType,
    u16                         affineIndex
)
{
    pMtxCache->affineIndex[flipType][surfaceType - NNS_G2D_SURFACETYPE_MAIN2D] = affineIndex;
}

//------------------------------------------------------------------------------
// Checks if it has been registered previously before registering it. This prevents duplicate registration.
// 
static NNS_G2D_INLINE BOOL IsMtxCacheRegisteredAsAffineParams_
(
    const u16 affineIdx
)
{
    return ( affineIdx != MtxCache_NOT_AVAILABLE ) ? TRUE : FALSE;
}



//------------------------------------------------------------------------------
static NNS_G2D_INLINE const MtxFx32* RndCoreGetCurrentMtx_()
{
    if( pTheInstance_->pCurrentMxt )
    { 
        //if( pTheInstance_->flipFlag != NNS_G2D_RENDERERFLIP_NONE )
        // TODO: Display a warning.
        // matrix that was set
        return pTheInstance_->pCurrentMxt;
    }else{
        //
        // unit matrix
        //    
        return &mtxIdentity_;
    }
}

//------------------------------------------------------------------------------
// Is affine transformation (2D) currently applied to the renderer?
static NNS_G2D_INLINE BOOL IsRndCore2DAffineTransformed_
(
    const NNSG2dRndCoreInstance* pRnd 
)
{
    return (BOOL)( pRnd->flipFlag == NNS_G2D_RENDERERFLIP_NONE && 
                   pRnd->pCurrentMtxCacheFor2D != NULL );
}

//------------------------------------------------------------------------------
// Calculate the shift value that converts the byte size into the number of characters from the mapping type.
// 
static NNS_G2D_INLINE int GetShiftToConvByteTo2DChar_( GXOBJVRamModeChar mappingType )
{
    int shift = ( REG_GX_DISPCNT_EXOBJ_MASK & mappingType ) >> REG_GX_DISPCNT_EXOBJ_SHIFT;
    
    return shift;
}

//------------------------------------------------------------------------------
// Determine whether the shift value is aligned to the shift width boundary.
static NNS_G2D_INLINE BOOL IsAlignedShiftValueBoundary_( u32 sizeByte, int shiftBit )
{
    const int mask = (0x1 << shiftBit) - 1;
    return (BOOL)( (mask & sizeByte) == 0);
}

//------------------------------------------------------------------------------
// The number of characters is calculated from the number of bytes, the mapping type, etc.
// 
static NNS_G2D_INLINE u32 GetNum2DCharacter_( u32 sizeByte, GXOBJVRamModeChar mappingType )
{
   
    const int shiftBit  = 5 + GetShiftToConvByteTo2DChar_( mappingType ); // 5 means  /= 32 
    u32       numChar   = sizeByte >> shiftBit;
    
    NNS_G2D_ASSERT( IsAlignedShiftValueBoundary_( sizeByte, shiftBit ) ); 
    
    return numChar;
}

//------------------------------------------------------------------------------
// Get the texture setting of the 3D Graphics Engine.
// 
static NNS_G2D_INLINE u32 GetTexBaseAddr3D_( const NNSG2dImageProxy* pImgProxy )
{
    NNS_G2D_NULL_ASSERT( pImgProxy );
    if( NNS_G2dIsImageReadyToUse( pImgProxy, NNS_G2D_VRAM_TYPE_3DMAIN ) )
    {
        return NNS_G2dGetImageLocation( pImgProxy, NNS_G2D_VRAM_TYPE_3DMAIN );
    }else{
        // TODO: warning
        return 0;
    }
}

//------------------------------------------------------------------------------
// perform palette configuration for the 3D Graphics Engine
//
static NNS_G2D_INLINE u32 GetPltBaseAddr3D_( const NNSG2dImagePaletteProxy* pPltProxy )
{
    NNS_G2D_NULL_ASSERT( pPltProxy );
    if( NNS_G2dIsImagePaletteReadyToUse( pPltProxy, NNS_G2D_VRAM_TYPE_3DMAIN ) )
    {
        return NNS_G2dGetImagePaletteLocation( pPltProxy, NNS_G2D_VRAM_TYPE_3DMAIN );
    }else{
        // TODO: warning
        return 0;
    }
}


//------------------------------------------------------------------------------
// currently, this only does the cast
static NNS_G2D_INLINE NNS_G2D_VRAM_TYPE SurfaceTypeToVramType_( NNSG2dSurfaceType   surfaceType )
{
    return (NNS_G2D_VRAM_TYPE)surfaceType;
}

//------------------------------------------------------------------------------
// get the character offset of the 2D Graphics Engine
// 
static NNS_G2D_INLINE u32 GetCharacterBase2D_
( 
    const NNSG2dImageProxy* pImgProxy, 
    NNSG2dSurfaceType       type 
)
{
    
    NNS_G2D_NULL_ASSERT( pImgProxy );
    NNS_G2D_ASSERT( type == NNS_G2D_SURFACETYPE_MAIN2D || 
                    type == NNS_G2D_SURFACETYPE_SUB2D );

    
    {   
        const NNS_G2D_VRAM_TYPE       vramType = SurfaceTypeToVramType_( type );
   
        if( NNS_G2dIsImageReadyToUse( pImgProxy, vramType ) )
        {
            u32 baseAddr = NNS_G2dGetImageLocation( pImgProxy, vramType );
            
            return GetNum2DCharacter_( baseAddr, pImgProxy->attr.mappingType );     
            
        }else{
            // TODO: warning
            return 0;
        }
    }
}



//------------------------------------------------------------------------------
static NNS_G2D_INLINE BOOL IsOamAttrDoubleAffineMode_( const GXOamAttr* pAttr )
{
    return (BOOL)(pAttr->rsMode == 0x3);
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void HandleCellCallBackFunc_
( 
    NNSG2dRndCoreDrawCellCallBack   pFunc,
    const NNSG2dCellData*           pCell 
) 
{
    //
    // call the callback function
    //
    // We have determined not to use the matrix as an argument, since theInstance_ is passed as well...
    if( pFunc )
    {
        (*pFunc)( pTheInstance_, pCell ); 
    } 
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void HandleCellOamBackFunc_
( 
    NNSG2dRndCoreDrawOamCallBack    pFunc,
    const NNSG2dCellData*           pCell,
    u16                             oamIdx 
) 
{
    //
    // call the callback function
    //
    // We have determined not to use the matrix as an argument, since theInstance_ is passed as well...
    // 
    if( pFunc )
    {
        (*pFunc)( pTheInstance_, pCell, oamIdx ); 
    } 
}

//------------------------------------------------------------------------------
// create the flip matrix
// The operation such as the one created immediately before loading has been changed.
static NNS_G2D_INLINE void MakeFlipMtx_
( 
    const MtxFx22*    pMtxSrc, 
    MtxFx22*          pMtxDst, 
    OAM_FLIP          type 
)
{
    NNS_G2D_NULL_ASSERT( pMtxSrc );
    NNS_G2D_NULL_ASSERT( pMtxDst );
    
    {
        *pMtxDst = *pMtxSrc;
        
        if( type & OAM_FLIP_H )
        {
            pMtxDst->_00 = -pMtxDst->_00;
            pMtxDst->_01 = -pMtxDst->_01;
        }
        
        if( type & OAM_FLIP_V )
        {
            pMtxDst->_10 = -pMtxDst->_10;
            pMtxDst->_11 = -pMtxDst->_11;
        }
    }

}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE u16 LoadMtxCacheAsAffineParams_
( 
    NNSG2dRndCore2DMtxCache*               pMtxCache,
    const NNSG2dRndCoreSurface*            pSurface, 
    OAM_FLIP                               type  
)
{
    u16    affineIdx  = GetMtxCacheAffineIndex_( pMtxCache, pSurface->type, type );
    
    //
    // if this is a matrix not previously registered in the lower module as an affine parameter...
    //
    if( !IsMtxCacheRegisteredAsAffineParams_( affineIdx ) )
    {
        NNS_G2D_NULL_ASSERT( pTheInstance_->pFuncOamAffineRegister );        
        
        //
        // perform the registration, and store that result in the cache 
        //
        {
            if( type == OAM_FLIP_NONE )
            {
                affineIdx = (*pTheInstance_->pFuncOamAffineRegister)( &pMtxCache->m22 );
            }else{
                MtxFx22     mtxTemp;
                MakeFlipMtx_( &pMtxCache->m22, &mtxTemp, type );
                affineIdx = (*pTheInstance_->pFuncOamAffineRegister)( &mtxTemp );
            }
                
            SetMtxCacheAffineIndex_( pMtxCache, pSurface->type, type, affineIdx );
        }
    }
    
    return affineIdx;
}


//------------------------------------------------------------------------------
static NNS_G2D_INLINE NNS_G2D_VRAM_TYPE 
ConvertSurfaceTypeToVramType_( NNSG2dSurfaceType surfaceType )
{
    return (NNS_G2D_VRAM_TYPE)(surfaceType);
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE u16 GetFlipedOBJPosX_( GXOamAttr* pOam, const GXOamShape shape )
{
    if( NNS_G2dIsRndCoreFlipH( pTheInstance_ ) )
    {
		return (u16)( -NNS_G2dRepeatXinCellSpace(  (s16)pOam->x ) - NNS_G2dGetOamSizeX( &shape ) );
	}
    
    return (u16)(NNS_G2dRepeatXinCellSpace( (s16)pOam->x ));
}
//------------------------------------------------------------------------------
static NNS_G2D_INLINE u16 GetFlipedOBJPosY_( GXOamAttr* pOam, const GXOamShape shape )
{
	if( NNS_G2dIsRndCoreFlipV( pTheInstance_ ) )
	{
		return (u16)( -NNS_G2dRepeatYinCellSpace( (s16)pOam->y ) - NNS_G2dGetOamSizeY( &shape ) );
	}
	
	return (u16)NNS_G2dRepeatYinCellSpace( (s16)pOam->y );
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void OverwriteOamFlipFlag_( GXOamAttr* pOam )
{
    NNS_G2D_NULL_ASSERT( pOam );
    {
        //
        // manipulate the flip information
        //        
        const BOOL bFlipH = 
           NNS_G2dIsRndCoreFlipH( pTheInstance_ ) ^ NNSi_G2dGetOamFlipH( pOam );
        const BOOL bFlipV = 
           NNS_G2dIsRndCoreFlipV( pTheInstance_ ) ^ NNSi_G2dGetOamFlipV( pOam );
           
        pOam->attr01 &= ~GX_OAM_ATTR01_HF_MASK;
        pOam->attr01 |= bFlipH << GX_OAM_ATTR01_HF_SHIFT;
           
        pOam->attr01 &= ~GX_OAM_ATTR01_VF_MASK;
        pOam->attr01 |= bFlipV << GX_OAM_ATTR01_VF_SHIFT;
    }
}


//------------------------------------------------------------------------------
static void DoAffineTransforme_
( 
    const MtxFx22*    pMtxSR, 
    GXOamAttr*        pOam, 
    NNSG2dFVec2*      pBaseTrans
)
{
    NNSG2dFVec2          objTrans;
    GXOamEffect          effectTypeAfter;
    
    // Get oam's position data in OBJ local coordinates, and convert it to world coordinates.
    NNS_G2dGetOamTransFx32( pOam, &objTrans );
    
    //
    // When an object is configured for double affine, a position adjustment value is appended to the normal object display position. As a result, the adjustment value is temporarily removed.
    // 
    // 
    //
    NNSi_G2dRemovePositionAdjustmentFromDoubleAffineOBJ( pOam, &objTrans );               
                        
    // do affine transform
    MulMtx22( pMtxSR, &objTrans, &objTrans );
    
    //
    // If affine transform was used, specify affine mode (double size or normal).
    // (Note: In previous versions, this processing was done in the low-level module.)
    // 
    // We want to set only affine mode, so set the proper value in the affine parameter number.
    // The GX library does not contain the proper API.
    // We plan to remedy this.
    //
    if( pTheInstance_->affineOverwriteMode != NNS_G2D_RND_AFFINE_OVERWRITE_NONE )
    {
        if( pTheInstance_->affineOverwriteMode == NNS_G2D_RND_AFFINE_OVERWRITE_DOUBLE  ) 
        {
           effectTypeAfter =  GX_OAM_EFFECT_AFFINE_DOUBLE;
        }else{
           effectTypeAfter =  GX_OAM_EFFECT_AFFINE;
        }
                            
        G2_SetOBJEffect( pOam, effectTypeAfter, 0 );
    }
    
    //
    // Because the display position for double affine mode is different than that for normal affine conversion mode, objects for which double affine mode conversion is specified are adjusted by this function.
    // 
    // 
    // 
    {
        // If double-size affine, use position correction for double-size affine OBJs.
        const BOOL bShouldAdjust = G2_GetOBJEffect( pOam ) == GX_OAM_EFFECT_AFFINE_DOUBLE;
                                 
        NNSi_G2dAdjustDifferenceOfRotateOrientation( pOam, 
                                                     pMtxSR, 
                                                     &objTrans, 
                                                     bShouldAdjust );
    }
    
    // add base offset( left top position of Obj )
    objTrans.x += pBaseTrans->x;
    objTrans.y += pBaseTrans->y;
    G2_SetOBJPosition( pOam, objTrans.x >> FX32_SHIFT, objTrans.y >> FX32_SHIFT  );
}

//------------------------------------------------------------------------------                                                             
static NNS_G2D_INLINE void DoFlipTransforme_( GXOamAttr* pOam, NNSG2dFVec2* pBaseTrans )
{    
    //
    // flip process
    //
    if( pTheInstance_->flipFlag != NNS_G2D_RENDERERFLIP_NONE )
    {    
        
        
        const GXOamShape shape = NNS_G2dGetOAMSize( pOam ); 
                                
        OverwriteOamFlipFlag_( pOam );
        
        if( NNS_G2dIsRndCoreFlipH( pTheInstance_ ) )
        {
            pOam->x = -pOam->x - NNS_G2dGetOamSizeX( &shape );
        }
        
        if( NNS_G2dIsRndCoreFlipV( pTheInstance_ ) )
        {
            pOam->y = (u8)(-pOam->y - NNS_G2dGetOamSizeY( &shape ));
        }
    }
    
    // add base offset( left top position of Obj )
    pOam->x += pBaseTrans->x >> FX32_SHIFT;
    pOam->y += pBaseTrans->y >> FX32_SHIFT;
}


//------------------------------------------------------------------------------
// Investigates if the renderer is ready to start rendering.
// Used only in the assert statements at debug build.
//
static BOOL IsRndCoreReadyForBeginRendering_( NNSG2dRndCoreInstance* pRnd )
{
    NNS_G2D_NULL_ASSERT( pRnd );
    {
       //
       // Is the image proxy set?
       //
       if( !(pRnd->pImgProxy && pRnd->pPltProxy) )
       {
           OS_Warning("RendererCore:ImageProxy isn't ready.");
           return FALSE;
       }
       
       //
       // Is the rendering target surface set?
       //
       if( pRnd->pCurrentTargetSurface == NULL )
       {
           OS_Warning("RendererCore:TragetSurface isn't ready.");
           return FALSE;
       }else{
           // Is surface active?
           if( !pRnd->pCurrentTargetSurface->bActive )
           {
               OS_Warning("RendererCore:The current tragetSurface isn't active. Is it OK?");
               // Warning only. Does not make it FALSE.
           }
       }
       
       //
       // For 2D rendering, is OAM registration function set properly?       
       //
       if( pRnd->pCurrentTargetSurface->type == NNS_G2D_SURFACETYPE_MAIN2D ||
           pRnd->pCurrentTargetSurface->type == NNS_G2D_SURFACETYPE_SUB2D  )
       {
           if
           ( 
              ( pRnd->pFuncOamAffineRegister == NULL && 
                  pRnd->pFuncOamRegister == NULL      )            
           )
           {
               OS_Warning( "RendererCore:OAM-RegisterFunction For 2D Graphics Engine rendering isn't ready." );
               return FALSE;
           }
       }
    }
    // passed test
    return TRUE;
}

//------------------------------------------------------------------------------
// Render the cell using the 2D Graphics Engine.
//
// This is a process exclusive to 2D graphics engine rendering.
//
static void DrawCellToSurface2D_
( 
    const NNSG2dRndCoreSurface*    pSurface, 
    const NNSG2dCellData*          pCell
)
{
    NNSG2dFVec2          baseTrans;
    MtxFx22              mtxSR;
    const MtxFx32*       pCurrMtx = RndCoreGetCurrentMtx_();
    
    NNS_G2D_NULL_ASSERT( pSurface );
    NNS_G2D_NULL_ASSERT( pCell );
    
    
    // get current Mtx for affine transformation
    NNSi_G2dGetMtxTrans( pCurrMtx, &baseTrans );  
    //
    // conversion to view local type
    //                  
    baseTrans.x -= pSurface->viewRect.posTopLeft.x;
    baseTrans.y -= pSurface->viewRect.posTopLeft.y;
    
    {
        const u32   baseCharOffset  = pTheInstance_->base2DCharOffset;
        const BOOL  bAffined        = IsRndCore2DAffineTransformed_( pTheInstance_ );
        
        u16          i;
        GXOamAttr*   pTempOam = &pTheInstance_->currentOam;
        u16          oamAffinIdx = NNS_G2D_OAM_AFFINE_IDX_NONE;
                        
        for( i = 0; i < pCell->numOAMAttrs; i++ )
        {
            //
            // Oam attribute pre-render callback
            //
            pTheInstance_->bDrawEnable = TRUE;
            NNS_G2dCopyCellAsOamAttr( pCell, i, pTempOam );
            
            HandleCellOamBackFunc_( pSurface->pBeforeDrawOamBackFunc, pCell, i );
            
            //
            // render skip decision
            //
            if( !pTheInstance_->bDrawEnable )
            {
                continue;
            }
            
            
            
            //
            // calculation of the character number offset
            //
            pTempOam->charNo += baseCharOffset;
                
            
            //
            // coordinate transformation: the flip process or the affine transformation process
            //
            if( bAffined )
            {       
                //
                // Function determines whether it is necessary to load the current matrix stack, loads it and returns the index.
                // 
                // Checks the object's flip flag and creates the appropriate affine conversion matrix. It must therefore be run every time within the loop for each object.
                // 
                //
                oamAffinIdx = 
                LoadMtxCacheAsAffineParams_( pTheInstance_->pCurrentMtxCacheFor2D, 
                                             pSurface, 
                                             TO_OAM_FLIP( NNSi_G2dGetOamFlipH( pTempOam ), 
                                                          NNSi_G2dGetOamFlipV( pTempOam ) ));
                NNSi_G2dGetMtxRS( pCurrMtx, &mtxSR );
                DoAffineTransforme_( &mtxSR, pTempOam, &baseTrans );
            }else{
                oamAffinIdx = NNS_G2D_OAM_AFFINE_IDX_NONE;
                DoFlipTransforme_( pTempOam, &baseTrans );
            }

            //
            // rendering registration function call
            // Third argument is not used. Pass a dummy argument.
            //
            NNS_G2D_NULL_ASSERT( pTheInstance_->pFuncOamRegister );
            
            if( NNSi_G2D_RNDCORE_OAMENTORY_SUCCESSFUL 
                != (*pTheInstance_->pFuncOamRegister)( pTempOam, 
                                                      oamAffinIdx, 
                                                      NNSi_G2D_RNDCORE_DUMMY_FLAG ) )
            {
                // we have no capacity for new oam data
                return;
            }
             
            //
            // callback after Oam attribute rendering
            //
            HandleCellOamBackFunc_( pSurface->pAfterDrawOamBackFunc, pCell, i );
        }
    }
}

//------------------------------------------------------------------------------
static void DrawOamToSurface3D_
( 
    GXOamAttr*                     pOam
)
{
    // matrix setting
    G3_LoadMtx43( &pTheInstance_->mtxFor3DGE );
        
    if( pTheInstance_->flipFlag != NNS_G2D_RENDERERFLIP_NONE )
    {   
        const GXOamShape shape = NNS_G2dGetOAMSize( pOam );
    
        //
        // gets the render position of the object
        //
    	const s16 posX = (s16)GetFlipedOBJPosX_( pOam, shape );
        const s16 posY = (s16)GetFlipedOBJPosY_( pOam, shape );
        const s16 posZ = -1;      
        
        //
        // manipulate the flip information
        //        
        OverwriteOamFlipFlag_( pOam );
        
        
        NNS_G2dDrawOneOam3DDirectWithPosFast( posX, posY, posZ, 
                                      pOam, 
                                      &pTheInstance_->pImgProxy->attr, 
                                       pTheInstance_->baseTexAddr3D, 
                                       pTheInstance_->basePltAddr3D );        
    }else{
        const s16 posX = (s16)NNS_G2dRepeatXinCellSpace( (s16)pOam->x );
        const s16 posY = (s16)NNS_G2dRepeatYinCellSpace( (s16)pOam->y );
        const s16 posZ = -1;
        
        NNS_G2dDrawOneOam3DDirectWithPosFast( posX, posY, posZ, 
                                      pOam, 
                                      &pTheInstance_->pImgProxy->attr, 
                                       pTheInstance_->baseTexAddr3D, 
                                       pTheInstance_->basePltAddr3D );    
    }
    
}
//------------------------------------------------------------------------------
// Renders the cell using the 3D Graphics Engine.
// 
static void DrawCellToSurface3D_
( 
    const NNSG2dRndCoreSurface*    pSurface, 
    const NNSG2dCellData*          pCell
)
{
    u16 i = 0;
    
    GXOamAttr*            pTempOam = &pTheInstance_->currentOam;
    
    
    NNS_G2D_NULL_ASSERT( pSurface );
    NNS_G2D_NULL_ASSERT( pCell );
    
    
    // Draw All Objects
    for( i = 0; i < pCell->numOAMAttrs; i++ )
    {
        //
        // Oam attribute pre-render callback
        //
        pTheInstance_->bDrawEnable = TRUE;
        NNS_G2dCopyCellAsOamAttr( pCell, i, pTempOam );
        
        HandleCellOamBackFunc_( pSurface->pBeforeDrawOamBackFunc, pCell, i );
        if( pTheInstance_->bDrawEnable )            
        {            
            // Render
            DrawOamToSurface3D_( pTempOam );
        }
        //
        // Oam attribute pre-render callback
        //
        HandleCellOamBackFunc_( pSurface->pAfterDrawOamBackFunc, pCell, i );
    }
}



//------------------------------------------------------------------------------
static NNS_G2D_INLINE void DrawCellImpl_
( 
    const NNSG2dCellData*   pCell, 
    u32                     cellVramTransferHandle 
)
{
    const NNSG2dRndCoreSurface*   pSurface = pTheInstance_->pCurrentTargetSurface;
    
    //
    // If not active, render process does nothing
    //
    if( !pSurface->bActive )
    {
        return;
    }
    
    // the culling and matrix load are also included in the callback
    pTheInstance_->bDrawEnable = TRUE;
    HandleCellCallBackFunc_( pSurface->pBeforeDrawCellBackFunc, pCell );
    if( pTheInstance_->bDrawEnable )
    {        
        //
        // If this is a VRAM transmission cell, record in the VRAM transmission state object that the cell was rendered.
        //
        if( cellVramTransferHandle != NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE )
        {
            NNSi_G2dSetVramTransferCellDrawnFlag
            ( 
                cellVramTransferHandle, 
                ConvertSurfaceTypeToVramType_( pSurface->type ),
                TRUE 
            );
        }
        
        //
        // for each type of surface...
        //
        switch( pSurface->type )
        {
        case NNS_G2D_SURFACETYPE_MAIN3D:
            DrawCellToSurface3D_( pSurface, pCell ); 
            break;
        case NNS_G2D_SURFACETYPE_MAIN2D:
        case NNS_G2D_SURFACETYPE_SUB2D:
            DrawCellToSurface2D_( pSurface, pCell );
            break;
        case NNS_G2D_SURFACETYPE_MAX:
        default:
            NNS_G2D_ASSERT(FALSE);
        }
    }
    HandleCellCallBackFunc_( pSurface->pAfterDrawCellBackFunc, pCell );
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitRndCore

  Description:  Initializes the render core module.
                Make sure to execute this before using NNSG2dRndCoreInstance.
                
                
  Arguments:    pRnd        [OUT] renderer core
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitRndCore( NNSG2dRndCoreInstance* pRnd )
{
    MI_CpuFill16( pRnd, 0x0, sizeof( NNSG2dRndCoreInstance ) );
    
    pRnd->pCurrentTargetSurface = NULL;
    
    pRnd->affineOverwriteMode  = NNS_G2D_RND_AFFINE_OVERWRITE_DOUBLE;
    
    pRnd->pImgProxy = NULL;
    pRnd->pPltProxy = NULL;
    
    pRnd->flipFlag = NNS_G2D_RENDERERFLIP_NONE;
    
    pRnd->bDrawEnable = TRUE;
    
    pRnd->pCurrentMtxCacheFor2D = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitRndCoreSurface

  Description:  Initializes the renderer core drawing target surface.
                Make sure to execute this function before using NNSG2dRndCoreSurface.
                
                
  Arguments:    pSurface          [OUT] renderer core drawing target surface
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitRndCoreSurface( NNSG2dRndCoreSurface* pSurface )
{
    MI_CpuFill16( pSurface, 0x0, sizeof( NNSG2dRndCoreSurface ) );
    
    pSurface->bActive   = TRUE;
    pSurface->type      = NNS_G2D_SURFACETYPE_MAX;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreImageProxy

  Description:  Sets the image proxy which the renderer core references for rendering.
                Image proxies must not be modified within the Begin&#8212;End Rendering block. This differs from the renderer module.
                
                This is because the renderer core module calculates the parameter at the timing of BeginRendering.
                
                
                
                This function needs to be called outside the Begin-EndRendering block.
                                
  Arguments:    pRnd        [OUT] renderer core
                pImgProxy                 [IN]  image proxy
                pPltProxy                 [IN]  image palette proxy
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCoreImageProxy
( 
    NNSG2dRndCoreInstance*              pRnd,
    const NNSG2dImageProxy*             pImgProxy, 
    const NNSG2dImagePaletteProxy*      pPltProxy
)
{
    NNS_G2D_RND_OUTSIDE_BEGINEND_ASSERT();
    NNS_G2D_NULL_ASSERT( pRnd );
    NNS_G2D_NULL_ASSERT( pImgProxy );
    NNS_G2D_NULL_ASSERT( pPltProxy );
    
    pRnd->pImgProxy = pImgProxy;
    pRnd->pPltProxy = pPltProxy;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreOamRegisterFunc

  Description:  Configures the set of function pointers used when registering the OAMs that indicate objects rendered by the renderer core module to external modules.
                
                
                The OAM registration function must be configures when the set surface is to be rendered with the 2D graphics engine.
                
                
                
                In addition to this function, there is also the NNS_G2dSetRndCoreOamRegisterFuncEx function, which configures the block transfer registration function as an OAM registration function.
                
                
                Note: When this function is called, the block transfer registration function is reset.
                
                
                This function needs to be called outside the Begin-EndRendering block.
                                
  Arguments:    pRnd        [OUT] renderer core
                pFuncOamRegister                 [IN]  OAM registration function
                pFuncOamAffineRegister           [IN]  affine parameter registration function
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCoreOamRegisterFunc
(
    NNSG2dRndCoreInstance*          pRnd, 
    NNSG2dOamRegisterFunction       pFuncOamRegister,
    NNSG2dAffineRegisterFunction    pFuncOamAffineRegister
)
{
    NNS_G2D_RND_OUTSIDE_BEGINEND_ASSERT();
    NNS_G2D_NULL_ASSERT( pRnd );
    NNS_G2D_NULL_ASSERT( pFuncOamRegister );
    NNS_G2D_NULL_ASSERT( pFuncOamAffineRegister );
    
    pRnd->pFuncOamRegister       = pFuncOamRegister;
    pRnd->pFuncOamAffineRegister = pFuncOamAffineRegister;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreAffineOverwriteMode

  Description:  Sets the overwriting operation in affine transformation mode.
                
                The following is the definition of the enumerator that represents the overwriting procedure.
                
                typedef enum NNSG2dRendererAffineTypeOverwiteMode
                {
                    NNS_G2D_RND_AFFINE_OVERWRITE_NONE,  // Does not overwrite
                    NNS_G2D_RND_AFFINE_OVERWRITE_NORMAL,// Sets to normal affine transformation method
                    NNS_G2D_RND_AFFINE_OVERWRITE_DOUBLE // Sets to double-size affine transformation method
                  
                }NNSG2dRendererAffineTypeOverwiteMode;
                
                
                This function can be called in or outside the Begin-EndRendering block.
                
                
  Arguments:    pCurrentMxt         [IN]  affine transform matrix
                mode                [IN]  enumerator representing the overwriting operation in affine transformation mode
                                         (NNSG2dRendererAffineTypeOverwiteMode)
                                    
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCoreAffineOverwriteMode
( 
    NNSG2dRndCoreInstance*                  pRnd, 
    NNSG2dRendererAffineTypeOverwiteMode    mode 
)
{
    NNS_G2D_NULL_ASSERT( pRnd );
    
    pRnd->affineOverwriteMode = mode;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreCurrentMtx3D

  Description:  Sets the affine transformation matrix.
                The affine conversion matrix is used by the current matrix configured by the geometry engine during 3D graphics engine rendering.
                 
                
                
                The transformation matrix set here is valid in the Begin-EndRendering block.
                Settings are reset when the NNS_G2dRndCoreEndRendering function is called. As such, when new rendering occurs, this function has to be called again to re-do the settings.
                
                
                
                Internally, a 4x3 matrix to be set to the current matrix of Geometry Engine is calculated.
                This function is used only for rendering with 3D Graphics Engine.
                A warning message will be output when this function is called during 2D graphics engine rendering.
                
                
                This function needs to be called in the Begin-EndRendering block.
                
                
  Arguments:    pCurrentMxt         [IN]  current transform matrix
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCoreCurrentMtx3D
( 
    const MtxFx32*              pCurrentMxt
)
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT();
    NNS_G2D_NULL_ASSERT( pCurrentMxt );
    NNS_G2D_WARNING( pTheInstance_->pCurrentTargetSurface->type == NNS_G2D_SURFACETYPE_MAIN3D,
       "This method works only for 3D graphics engine rendering." );
       
    {
       NNSG2dRndCoreInstance*      pRnd = pTheInstance_;
         
       pRnd->pCurrentMxt            = pCurrentMxt;
        
       //
       // When rendering with the 3D graphics engine, calculate a 4x3 matrix to configure as the geometry engine's current matrix.
       // 
       // When rendering with the 2D graphics engine, pRnd->pCurrentMxt are only used for cell affine conversion, so a 3x2 matrix is sufficient.
       // 
       //
       {
           //
           // generate a 4x3 matrix to load to Geometry Engine
           //
           {
              pRnd->mtxFor3DGE._00 = pCurrentMxt->_00;
              pRnd->mtxFor3DGE._01 = pCurrentMxt->_01;
              pRnd->mtxFor3DGE._02 = 0;

              pRnd->mtxFor3DGE._10 = pCurrentMxt->_10;
              pRnd->mtxFor3DGE._11 = pCurrentMxt->_11;
              pRnd->mtxFor3DGE._12 = 0;

              pRnd->mtxFor3DGE._20 = 0;
              pRnd->mtxFor3DGE._21 = 0;
              pRnd->mtxFor3DGE._22 = FX32_ONE;

              pRnd->mtxFor3DGE._30 = pCurrentMxt->_20;
              pRnd->mtxFor3DGE._31 = pCurrentMxt->_21;
              pRnd->mtxFor3DGE._32 = pRnd->zFor3DSoftwareSprite;
           }
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreCurrentMtx2D

  Description:  Configures the affine conversion matrix and affine conversion parameters applied to objects when rendering with the 2D graphics engine.
                
                The affine conversion matrix is used for object position coordinate conversion in the CPU during 2D graphics engine rendering.
                 
                
                
                Affine parameter is set by using matrix cache.
                If NULL is specified as a pointer to the matrix cache, it is assumed that the affine transformation will not be performed.
                
                
                Affine parameter set here is valid in the Begin-EndRendering block.
                Settings are reset when the NNS_G2dRndCoreEndRendering function is called. As such, this function has to be called again to re-do the settings.
                
                
                
                This function is used only when rendering with 2D Graphics Engine.
                A warning message will be output when this function is called during 3D graphics engine rendering.
                
                
                This function needs to be called in the Begin-EndRendering block.
                
                
  Arguments:    pMtx                           [IN]  current transform matrix
                pCurrentMtxCacheFor2D         [IN]  matrix cache that stores affine parameters
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
static BOOL CheckMtx2DParamsValid
(
    const MtxFx32*                 pMtx,
    NNSG2dRndCore2DMtxCache*       pCurrentMtxCacheFor2D
)
{
    if( !( pMtx->_00 == FX32_ONE && pMtx->_01 == 0 &&
           pMtx->_10 == 0        && pMtx->_11 == FX32_ONE ) )
    {
       if( pCurrentMtxCacheFor2D == NULL )
       {
           NNS_G2D_WARNING( FALSE,
              "Make sure that you have to specified the affine-mtx for the 2D graphics engine when you use affine transformation." );
           return FALSE;
       }
    }else{
       //
       // No warning is given for the time being.
       //
       /*
       if( pCurrentMtxCacheFor2D != NULL )
       {
           NNS_G2D_WARNING( FALSE,
              "The affine mtx setting is useless when you don't use affine transformation." );
           return FALSE;
       }
       */
    }
    return TRUE;
}
//------------------------------------------------------------------------------
void NNS_G2dSetRndCoreCurrentMtx2D
( 
    const MtxFx32*                 pMtx,
    NNSG2dRndCore2DMtxCache*       pCurrentMtxCacheFor2D
)
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT();
    NNS_G2D_NULL_ASSERT( pMtx );
    
    NNS_G2D_WARNING( pTheInstance_->pCurrentTargetSurface->type != NNS_G2D_SURFACETYPE_MAIN3D,
       "This method works only for 2D graphics engine rendering." );
    NNS_G2D_ASSERT( CheckMtx2DParamsValid( pMtx, pCurrentMtxCacheFor2D ) );
    
           
    {
       NNSG2dRndCoreInstance*      pRnd = pTheInstance_;
       
       pRnd->pCurrentMxt            = pMtx;
       pRnd->pCurrentMtxCacheFor2D  = pCurrentMtxCacheFor2D;
    }
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCore3DSoftSpriteZvalue

  Description:  Sets the z value of sprite to be used when rendering software sprites.
                
  Arguments:    pRnd        [OUT] renderer core
                z           [IN]  z value used for rendering software sprites
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCore3DSoftSpriteZvalue( NNSG2dRndCoreInstance* pRnd, fx32 z )
{
    pRnd->zFor3DSoftwareSprite = z;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreSurface

  Description:  Sets the rendering target surface to renderer core.
  
                It needs to be called outside the Begin-EndRendering block.
                
  Arguments:    pRnd        [OUT] renderer core
                pSurface    [IN]  rendering target surface
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCoreSurface
( 
    NNSG2dRndCoreInstance*      pRnd, 
    NNSG2dRndCoreSurface*       pSurface 
)
{
    NNS_G2D_RND_OUTSIDE_BEGINEND_ASSERT();
    NNS_G2D_NULL_ASSERT( pRnd );
    NNS_G2D_NULL_ASSERT( pSurface );
    //
    // TODO: Checks if it is active, and displays a warning message
    //
    pRnd->pCurrentTargetSurface = pSurface;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsRndCoreFlipH

  Description:  Gets the rendering flip status of renderer core.
                
  Arguments:    pRnd        [IN]  renderer core
                
  Returns:      TRUE when horizontal direction flip status is ON
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dIsRndCoreFlipH( const NNSG2dRndCoreInstance* pRnd )
{
    NNS_G2D_NULL_ASSERT( pRnd );
    return (BOOL)( (pRnd->flipFlag & NNS_G2D_RENDERERFLIP_H) != 0 );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsRndCoreFlipV

  Description:  Gets the rendering flip status of renderer core.
                
  Arguments:    pRnd        [IN]  renderer core
                
  Returns:      TRUE when vertical direction flip status is ON
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dIsRndCoreFlipV( const NNSG2dRndCoreInstance* pRnd )
{
    NNS_G2D_NULL_ASSERT( pRnd );
    return (BOOL)( (pRnd->flipFlag & NNS_G2D_RENDERERFLIP_V) != 0 );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreFlipMode

  Description:  Sets the rendering flip status for the renderer core.
                It is possible to be called in or outside the Begin-EndRendering block.
                Calling this function when using affine transformation feature is prohibited, and will cause an assert failure.
                
  Arguments:    pRnd        [OUT] renderer core
                bFlipH      [IN]  horizontal direction flip
                bFlipV      [IN]  vertical direction flip
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCoreFlipMode
( 
    NNSG2dRndCoreInstance* pRnd, 
    BOOL bFlipH, 
    BOOL bFlipV 
)
{
    //
    // checks if it is currently SR transformed
    // 
    NNS_G2D_NULL_ASSERT( pRnd );
    NNS_G2D_WARNING( pRnd->pCurrentMtxCacheFor2D == NULL, 
       "You can't use the flip function using affine transformation." );

    if( bFlipH )
    {
        pRnd->flipFlag |= NNS_G2D_RENDERERFLIP_H;
    }else{
        pRnd->flipFlag &= ~NNS_G2D_RENDERERFLIP_H;
    }
    
    if( bFlipV )
    {
        pRnd->flipFlag |= NNS_G2D_RENDERERFLIP_V;
    }else{
        pRnd->flipFlag &= ~NNS_G2D_RENDERERFLIP_V;
    }
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRndCoreBeginRendering

  Description:  Starts rendering with renderer core.
                Performs various preprocesses to start rendering.
                
                After calling this function, note that neither the image proxy nor the parameters for the target render surface can be modified.
                
                After calling this function, always call NNS_G2dRndCoreEndRendering() after the rendering is finished.
                
                The Begin&#8212;End rendering functions cannot be called within the Begin&#8212;End rendering block (i.e., no nested calls). If this does happen, an assert failure will result.
                
                
  Arguments:    pRnd        [IN]  renderer core
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRndCoreBeginRendering( NNSG2dRndCoreInstance* pRnd )
{
    NNS_G2D_NULL_ASSERT( pRnd );
    NNS_G2D_ASSERT( pTheInstance_ == NULL );
    NNS_G2D_ASSERTMSG( IsRndCoreReadyForBeginRendering_( pRnd ), 
       "NNSG2dRndCoreInstance isn't ready to begin rendering." );
    
    pTheInstance_ = pRnd;
    
    //
    // preprocesses specific to 3D rendering
    //
    if( pRnd->pCurrentTargetSurface->type == NNS_G2D_SURFACETYPE_MAIN3D )
    {
        // camera settings    
        G3_MtxMode( GX_MTXMODE_PROJECTION );
        G3_PushMtx();
        G3_Translate( -pRnd->pCurrentTargetSurface->viewRect.posTopLeft.x, 
                      -pRnd->pCurrentTargetSurface->viewRect.posTopLeft.y, 0 );
        G3_MtxMode( GX_MTXMODE_POSITION );
        
        //
        // calculation of base address
        //
        pRnd->baseTexAddr3D = GetTexBaseAddr3D_( pTheInstance_->pImgProxy );
        pRnd->basePltAddr3D = GetPltBaseAddr3D_( pTheInstance_->pPltProxy );
        
    }else{
    //
    // preprocesses specific to 2D rendering
    //
        pTheInstance_->base2DCharOffset 
           = GetCharacterBase2D_( pTheInstance_->pImgProxy, 
                                  pTheInstance_->pCurrentTargetSurface->type );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRndCoreEndRendering

  Description:  Ends rendering with renderer core.
                Performs various post-processes to end rendering.
                
                When an OAM block transfer function is configured as an OAM registration function, a block transfer function is used within this function to copy the content of the internal OAM buffer to an external module.
                
                
                Transfer is expected to always succeed, but if the transfer failed, an assert failure will result.
                
                (When the transfer fails, it is expected that there is an error with buffer size setting of internal OAM buffer.)
                
                
  Arguments:    None.
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRndCoreEndRendering( void )
{
    NNS_G2D_NULL_ASSERT( pTheInstance_ );
     
    //
    // post-processes specific to 3D rendering
    //
    if( pTheInstance_->pCurrentTargetSurface->type == NNS_G2D_SURFACETYPE_MAIN3D )
    {
        // camera settings (restore)
        G3_MtxMode( GX_MTXMODE_PROJECTION );
        G3_PopMtx(1);
        G3_MtxMode( GX_MTXMODE_POSITION );
        
    }else{
    //
    // post-processes specific to 2D rendering
    //    
        pTheInstance_->base2DCharOffset = 0;
        //
        // reset affine conversion setting
        //
        pTheInstance_->pCurrentMxt = NULL;
        pTheInstance_->pCurrentMtxCacheFor2D = NULL;
    }    
    pTheInstance_ = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRndCoreDrawCell

  Description:  Renders cell.
                It needs to be called in the Begin-EndRendering block.
                
  Arguments:    pCell:                    [IN]  cell data
  
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRndCoreDrawCell( const NNSG2dCellData* pCell )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT();
    NNS_G2D_NULL_ASSERT( pCell );
    
    DrawCellImpl_( pCell, NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE );   
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRndCoreDrawCellVramTransfer

  Description:  Draws the VRAM transfer cell.
                It needs to be called in the Begin-EndRendering block.
                
  Arguments:    pCell:                    [IN]  cell data
                cellVramTransferHandle:   [IN] handle of the cell VRAM transfer state object   
  
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRndCoreDrawCellVramTransfer( const NNSG2dCellData* pCell, u32 cellVramTransferHandle )
{
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT();
    NNS_G2D_NULL_ASSERT( pCell );
    
    NNS_G2D_ASSERT( cellVramTransferHandle != NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE );
    DrawCellImpl_( pCell, cellVramTransferHandle );
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRndCoreDrawCellFast2D

  Description:  Renders the cell in high speed by using the 2D Graphics Engine (OBJ feature).
                
                This function calls a callback function. It does not support cell affine conversion.
                When this function is called when a 2D affine conversion matrix (pCurrentMtxCacheFor2D) is set, it will result in an assert failure.
                
                
                It needs to be called in the Begin-EndRendering block.
                
  Arguments:    pCell:      [IN]  cell data
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRndCoreDrawCellFast2D( const NNSG2dCellData* pCell )
{
    const NNSG2dRndCoreSurface*   pSurface = pTheInstance_->pCurrentTargetSurface;    
    NNSG2dFVec2          baseTrans;
    const MtxFx32*       pCurrMtx = RndCoreGetCurrentMtx_();
    
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT();
    
    
    
    NNS_G2D_ASSERTMSG( pSurface->type == NNS_G2D_SURFACETYPE_MAIN2D || 
                      pSurface->type == NNS_G2D_SURFACETYPE_SUB2D,
                      "This method can work only for the 2D Graphics Engine." );
       
    NNS_G2D_ASSERTMSG( pTheInstance_->pCurrentMtxCacheFor2D == NULL,
                      "You can't use this method using affine transfomation." );
    NNS_G2D_NULL_ASSERT( pCell );   
    //------------------------------------------------------------------------------
    // Render the cell using the 2D Graphics Engine.
    // No support for callback invocation etc., but this version operates at high speed.
    // 

    // get current Mtx for affine transformation
    NNSi_G2dGetMtxTrans( pCurrMtx, &baseTrans );  
       
    //
    // conversion to view local type
    //                  
    baseTrans.x -= pSurface->viewRect.posTopLeft.x;
    baseTrans.y -= pSurface->viewRect.posTopLeft.y;
    //
    // Does virtually the same processing as NNS_G2dMakeCellToOams().
    // (However, it supports flip.)
    //
    //
    // version that does not use block transfer
    // Each time an OBJ is registered, overhead is generated for calling the render registration function.
    // When compared to the version that uses block transfers, there are some performance disadvantages. Originally, however, this function's rendering registration only occupies a small percent of a single frame (even when registering 128x2 objects), so this method is used when it is deemed to be non-problematic.
    // 
    // 
    // 
    {
       u16          i;
       GXOamAttr*   pTempOam;
       const u32    baseCharOffset = pTheInstance_->base2DCharOffset;

       for( i = 0; i < pCell->numOAMAttrs; i++ )
       {
           pTempOam = &pTheInstance_->currentOam;
                        
           NNS_G2dCopyCellAsOamAttr( pCell, i, pTempOam );
           
           DoFlipTransforme_( pTempOam, &baseTrans );
                        
           pTempOam->charNo += baseCharOffset;
                        
           //
           // rendering registration function call
           //
           NNS_G2D_NULL_ASSERT( pTheInstance_->pFuncOamRegister );
           if( FALSE == (*pTheInstance_->pFuncOamRegister)( pTempOam, NNS_G2D_OAM_AFFINE_IDX_NONE, FALSE ) )
           {
              // we have no capacity for new oam data
              return;
           }
       }
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetRndCoreCellCloneSource3D

  Description:  Calculates in advance the UV parameters referenced by the NNS_G2dRndCoreDrawCellClone3D function, and saves them as a UV parameter cache.
                
                It is anticipated that the input cells will be comprised of objects with the same image data, so UV values are calculated for the index 0 object and saved as a cache.
                
                
                Note: The UV parameter of a flipped sprite is different from a normal UV parameter.
                Flipped sprites therefore cannot be treated as having the same image data.
                This process is exclusively for 3D surfaces.
                Of the current surface types, an assert failure will result unless it is the NNS_G2D_SURFACETYPE_MAIN3D type.
                
                It needs to be called in the Begin-EndRendering block.
                
  Arguments:    pCell:      [IN]  cell data
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetRndCoreCellCloneSource3D( const NNSG2dCellData* pCell )
{
    
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT();
    NNS_G2D_ASSERTMSG( pTheInstance_->pCurrentTargetSurface->type == NNS_G2D_SURFACETYPE_MAIN3D,
                          "This method can work only for the 3D Graphics Engine." );
    NNS_G2D_NULL_ASSERT( pCell );         
    
    {   
        GXOamAttr*            pOam = &pTheInstance_->currentOam;
        // Using the top OBJ as representation...
        NNS_G2dCopyCellAsOamAttr( pCell, 0, pOam );
        // to set as parameter cache
        // This value is referenced by texture parameters for subsequent rendering.
        // 
        
        
        //
        // reflect the flip setting of renderer to OAM attribute
        //        
        OverwriteOamFlipFlag_( pOam );
               
               
        NNS_G2dSetOamSoftEmuSpriteParamCache
        (
           pOam, 
           &pTheInstance_->pImgProxy->attr, 
           pTheInstance_->baseTexAddr3D, 
           pTheInstance_->basePltAddr3D 
        );    
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dRndCoreDrawCellClone3D

  Description:  Cells comprised of the objects having the same image data will render faster using software sprites.
                
                
                When rendering objects having the same image data as software sprites (quads with textures), the objects will have identical UV values.
                
                
                As long as the OBJs have the same image data, either single or multiple OBJs can be contained in the cell.
                This function performs rendering by using the UV parameter cache that was calculated in advance.
                Because the UV value calculation needed for normal software sprite rendering can be omitted, it works at high speed.
                
                To configure the UV parameter cache, it is necessary to complete the parameter initialization by calling the NNS_G2dSetRndCoreCellCloneSource3D function in advance of this.
                
                
                
                Note: The UV parameter of a flipped sprite is different from a normal UV parameter.
                Flipped sprites therefore cannot be treated as having the same image data.
                
                
                This function supports affine transformation. Does not support calling of various callbacks.
                
                This process is exclusively for 3D surfaces.
                Of the current surface types, an assert failure will result unless it is the NNS_G2D_SURFACETYPE_MAIN3D type.
                
                It needs to be called in the Begin-EndRendering block.
                
  Arguments:    pCell:      [IN]  cell data
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dRndCoreDrawCellClone3D( const NNSG2dCellData* pCell )
{
    u16 i = 0;
    
    GXOamAttr*            pOam = &pTheInstance_->currentOam;
    
    NNS_G2D_RND_BETWEEN_BEGINEND_ASSERT();
    NNS_G2D_ASSERTMSG( pTheInstance_->pCurrentTargetSurface->type == NNS_G2D_SURFACETYPE_MAIN3D,
                          "This method can work only for the 3D Graphics Engine." );
    NNS_G2D_NULL_ASSERT( pTheInstance_->pCurrentTargetSurface );
    NNS_G2D_NULL_ASSERT( pCell );
    
    if( pTheInstance_->flipFlag != NNS_G2D_RENDERERFLIP_NONE )
    {
        //
        // flipped rendering
        //
        for( i = 0; i < pCell->numOAMAttrs; i++ )
        {
           NNS_G2dCopyCellAsOamAttr( pCell, i, pOam );
           
           // matrix setting
           G3_LoadMtx43( &pTheInstance_->mtxFor3DGE );
    
           // Render
           {               
               const GXOamShape shape = NNS_G2dGetOAMSize( pOam );
                
               //
               // gets the render position of the object
               //
               const s16 posX = (s16)GetFlipedOBJPosX_( pOam, shape );
               const s16 posY = (s16)GetFlipedOBJPosY_( pOam, shape );
               const s16 posZ = -1;      
                
               //
               // reflect the flip setting of renderer to OAM attribute
               //        
               OverwriteOamFlipFlag_( pOam );
               
                
               NNS_G2dDrawOneOam3DDirectUsingParamCacheFast( posX, posY, posZ, pOam );    
           }
        }
    }else{
        //
        // rendering without flip
        //
        for( i = 0; i < pCell->numOAMAttrs; i++ )
        {
           NNS_G2dCopyCellAsOamAttr( pCell, i, pOam );
           
           // matrix setting
           G3_LoadMtx43( &pTheInstance_->mtxFor3DGE );
    
           {
               const s16 posX = (s16)NNS_G2dRepeatXinCellSpace( (s16)pOam->x );
               const s16 posY = (s16)NNS_G2dRepeatYinCellSpace( (s16)pOam->y );
               const s16 posZ = -1;
                               
               NNS_G2dDrawOneOam3DDirectUsingParamCacheFast( posX, posY, posZ, pOam );
           }
        }
    }
}
