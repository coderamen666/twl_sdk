/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_CellTransferManager.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
 
#ifndef NNS_G2D_CELL_TRANSFER_MANAGER_H_
#define NNS_G2D_CELL_TRANSFER_MANAGER_H_

#include <nitro.h>
#include <nnsys/g2d/g2d_config.h>
#include <nnsys/g2d/g2d_Image.h>

#ifdef __cplusplus
extern "C" {
#endif

// This might not be needed later...
#include <nnsys/gfd/VramTransferMan/gfd_VramTransferManager.h>

#define NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE    (u32)0xFFFFFFFF


/*---------------------------------------------------------------------------*
  Name:         VramTransferTaskRegisterFuncPtr

  Description:  The pointer to the function that registers the Vram transfer task.
                
 *---------------------------------------------------------------------------*/
typedef BOOL (*VramTransferTaskRegisterFuncPtr)( NNS_GFD_DST_TYPE  type, 
                                              u32               dstAddr, 
                                              void*             pSrc, 
                                              u32               szByte );

//------------------------------------------------------------------------------
//
// NNSG2dCellTransferState 
//
//------------------------------------------------------------------------------
typedef struct NNSG2dCellTransferState
{
    //
    // Member set in initialization phase
    //
    NNSG2dVRamLocation    dstVramLocation;      // Transfer destination of image address (be sure each area size is at least szDst)
    u32                   szDst;                // Transfer destination area size
    
    const void*          pSrcNCGR;             // Transfer source data (character method)
    const void*          pSrcNCBR;             // Transfer source data (bitmap method)
    u32                   szSrcData;            // Transfer source data size (same)
    BOOL                  bActive;              // Active state?
    
    //
    // Member renewed every frame
    //
    u32                   bDrawn;               // Rendered?
                                                // Stores whether or not the image was rendered for each graphics engine.
                                                // This is used to avoid Vram transfer of non-rendered cells.
                                                // configured by the rendering module
                                                // refreshed every frame by the manager module   
                                                // When the user builds his or her own render module, this member must be set appropriately.
                                            
    u32                   bTransferRequested;   // Transfer requested?
                                                // Stores the state of the transfer request for each graphics engine.
                                                // The cell animation control module is set.
                                                // Reset when the transfer task registration is ended by the control module.
    
    u32                   srcOffset;            // source offset (details of transfer request)
    u32                   szByte;               // transfer size (details of transfer request)
    
}NNSG2dCellTransferState;



//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void 
NNS_G2dInitCellTransferStateManager
( 
    NNSG2dCellTransferState*            pCellStateArray, 
    u32                                 numCellState,
    VramTransferTaskRegisterFuncPtr     pTaskRegisterFunc
);

//------------------------------------------------------------------------------
// get and return handle
u32 
NNS_G2dGetNewCellTransferStateHandle();
void
NNS_G2dFreeCellTransferStateHandle( u32 handle );

//------------------------------------------------------------------------------
void NNS_G2dUpdateCellTransferStateManager();

//------------------------------------------------------------------------------
// transfer request related
// Called from the cell animation entity
// 
void NNS_G2dSetCellTransferStateRequested
( 
    u32                                 handle, 
    u32                                 srcOffset,
    u32                                 szByte
);







//------------------------------------------------------------------------------
// Internal functions 
//------------------------------------------------------------------------------
NNSG2dCellTransferState* 
NNSi_G2dGetCellTransferState
( 
    u32 handle 
);

//------------------------------------------------------------------------------
void NNSi_G2dInitCellTransferState
( 
    u32                   handle,
    
    u32                   dstAddr3D,
    u32                   dstAddr2DMain,
    u32                   dstAddr2DSub,
    u32                   szDst,
    
    const void*          pSrcNCGR,
    const void*          pSrcNCBR,
    u32                   szSrcData
);


//------------------------------------------------------------------------------
// Inline functions 
//------------------------------------------------------------------------------
// Access, such as flag operation access: internally released functions.
//------------------------------------------------------------------------------
NNS_G2D_INLINE void 
NNSi_G2dSetCellTransferStateRequestFlag( NNSG2dCellTransferState* pState, NNS_G2D_VRAM_TYPE type, BOOL flag )
{
    pState->bTransferRequested = ( pState->bTransferRequested & ~( 0x1 << type ) ) | ( flag << type );
}

NNS_G2D_INLINE void 
NNSi_G2dSetVramTransferRequestFlag( u32 handle, NNS_G2D_VRAM_TYPE type, BOOL flag )
{
    NNSG2dCellTransferState* pState = NNSi_G2dGetCellTransferState( handle );
    
    NNSi_G2dSetCellTransferStateRequestFlag( pState, type, flag );
}


//------------------------------------------------------------------------------
NNS_G2D_INLINE BOOL 
NNSi_G2dGetCellTransferStateRequestFlag( const NNSG2dCellTransferState* pState, NNS_G2D_VRAM_TYPE type )
{
    return (BOOL)( pState->bTransferRequested & ( 0x1 << type ) );
}

NNS_G2D_INLINE BOOL 
NNSi_G2dGetVramTransferRequestFlag( u32 handle, NNS_G2D_VRAM_TYPE type )
{
    const NNSG2dCellTransferState* pState = NNSi_G2dGetCellTransferState( handle );
    return NNSi_G2dGetCellTransferStateRequestFlag( pState, type );
}

//------------------------------------------------------------------------------
NNS_G2D_INLINE void 
NNSi_G2dSetCellTransferStateCellDrawnFlag( NNSG2dCellTransferState* pState, NNS_G2D_VRAM_TYPE type, BOOL flag )
{
    pState->bDrawn = ( pState->bDrawn & ~( 0x1 << type ) ) | ( flag << type );
}

NNS_G2D_INLINE void 
NNSi_G2dSetVramTransferCellDrawnFlag( u32 handle, NNS_G2D_VRAM_TYPE type, BOOL flag )
{
    NNSG2dCellTransferState* pState = NNSi_G2dGetCellTransferState( handle );
    NNSi_G2dSetCellTransferStateCellDrawnFlag( pState, type, flag );
}

//------------------------------------------------------------------------------
NNS_G2D_INLINE BOOL 
NNSi_G2dGetCellTransferStateCellDrawnFlag( const NNSG2dCellTransferState* pState, NNS_G2D_VRAM_TYPE type )
{
    return (BOOL)( pState->bDrawn & ( 0x1 << type ) );
}

NNS_G2D_INLINE BOOL 
NNSi_G2dGetVramTransferCellDrawnFlag( u32 handle, NNS_G2D_VRAM_TYPE type )
{
    const NNSG2dCellTransferState* pState = NNSi_G2dGetCellTransferState( handle );
    return NNSi_G2dGetCellTransferStateCellDrawnFlag( pState, type );
}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_CELL_TRANSFER_MANAGER_H_
