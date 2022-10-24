/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_CellTransferManager.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/g2d/g2d_CellTransferManager.h>
#include <nnsys/gfd/VramTransferMan/gfd_VramTransferManager.h>

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
static NNSG2dCellTransferState*                s_pCellStateArray = NULL;
static u32                                     s_numCellState = 0;

VramTransferTaskRegisterFuncPtr                s_pTaskRegisterFunc = NULL;


//------------------------------------------------------------------------------
// Is the state of the manager a valid state?
static NNS_G2D_INLINE BOOL IsCellTransferStateManagerValid_()
{
    return (BOOL)( ( s_pCellStateArray != NULL) && 
                   ( s_numCellState != 0 ) && 
                   ( s_pTaskRegisterFunc != NULL ) ) ;
}

//------------------------------------------------------------------------------
// Is the handle valid?
static NNS_G2D_INLINE BOOL IsValidHandle_( u32 handle )
{
    NNS_G2D_ASSERT( IsCellTransferStateManagerValid_() );
    
    if( handle != NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE )
    {    
        if( handle < s_numCellState )
        {
            if( s_pCellStateArray[handle].bActive )
            {
                return TRUE; 
            }
        }
    }
    return FALSE;
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE NNSG2dCellTransferState*
GetValidCellTransferState_( u32 validHandle )
{
    NNS_G2D_ASSERT( IsValidHandle_( validHandle ) );
    return &s_pCellStateArray[validHandle];
}



//------------------------------------------------------------------------------
// Does this need to be registered as a VRAM transfer task?
static NNS_G2D_INLINE BOOL ShouldRegisterAsVramTransferTask_
( 
    const NNSG2dCellTransferState*    pState,
    NNS_G2D_VRAM_TYPE                   type
)
{
    NNS_G2D_NULL_ASSERT( pState );
    
    return (BOOL)(  NNSi_G2dGetCellTransferStateRequestFlag( pState, type ) &&
                    NNSi_G2dGetCellTransferStateCellDrawnFlag( pState, type ) );
}

//------------------------------------------------------------------------------
// resets rendering attributes
static NNS_G2D_INLINE void ResetCellTransferStateDrawnFlag_
( 
    NNSG2dCellTransferState*  pState 
)
{
    pState->bDrawn = 0x0;
}



//------------------------------------------------------------------------------
// get the transfer source data
static NNS_G2D_INLINE const void* GetVramTransferSrc_
( 
    const NNSG2dCellTransferState*    pState,
    NNS_G2D_VRAM_TYPE                   type
)
{
    NNS_G2D_NULL_ASSERT( pState );
    
    if( type == NNS_G2D_VRAM_TYPE_3DMAIN )
    {
        return pState->pSrcNCBR;
    }else{
        return pState->pSrcNCGR;
    }
}

//------------------------------------------------------------------------------
// Is the transfer source data valid data?
static NNS_G2D_INLINE BOOL IsVramTransferSrcDataValid_
( 
    const NNSG2dCellTransferState*    pState,
    NNS_G2D_VRAM_TYPE                   type
)
{
    return (BOOL)( GetVramTransferSrc_( pState, type ) != NULL );
}



//------------------------------------------------------------------------------
// Are preparations for transfer ready? 
static NNS_G2D_INLINE BOOL IsCellTransferStateValid_
( 
    const NNSG2dCellTransferState*  pState, 
    NNS_G2D_VRAM_TYPE               type
)
{
    // transfer source data is set
    // transfer source region is set
    return (BOOL)( NNSi_G2dIsVramLocationReadyToUse( &pState->dstVramLocation, type ) && 
                   IsVramTransferSrcDataValid_( pState, type ) );
}

//------------------------------------------------------------------------------
// conversion of enumerator
// at present time, simply a cast
static NNS_G2D_INLINE NNS_GFD_DST_TYPE 
ConvertVramType_( NNS_G2D_VRAM_TYPE type )
{
    NNS_G2D_MINMAX_ASSERT( type, NNS_G2D_VRAM_TYPE_3DMAIN, NNS_G2D_VRAM_TYPE_2DSUB );
    {
        const static NNS_GFD_DST_TYPE cvtTbl []=
        {
            NNS_GFD_DST_3D_TEX_VRAM,      // NNS_G2D_VRAM_TYPE_3DMAIN
            NNS_GFD_DST_2D_OBJ_CHAR_MAIN, // NNS_G2D_VRAM_TYPE_2DMAIN
            NNS_GFD_DST_2D_OBJ_CHAR_SUB   // NNS_G2D_VRAM_TYPE_2DSUB
        };
        return cvtTbl[type];
    }
}

//------------------------------------------------------------------------------
// 
// creation of task
// Input NNSG2dCellTransferState to create a task to register to manager.
//
static NNS_G2D_INLINE BOOL MakeVramTransferTask_
(
    const NNSG2dCellTransferState*    pState, 
    NNS_G2D_VRAM_TYPE                   type
)
{    
    NNS_G2D_NULL_ASSERT( pState );
    
    NNS_G2D_ASSERT( IsCellTransferStateValid_( pState, type ) );
    
    //
    // Assign registration of VRAM transfer task to external module.
    //
    return (*s_pTaskRegisterFunc )( ConvertVramType_( type ),
                                    NNSi_G2dGetVramLocation( &pState->dstVramLocation, type ),
                                    (u8*)GetVramTransferSrc_( pState, type ) + pState->srcOffset,
                                    pState->szByte );
}
//------------------------------------------------------------------------------
// NNSG2dCellTransferState reset
static NNS_G2D_INLINE void ResetCellTransferState_( NNSG2dCellTransferState* pState )
{
    NNS_G2D_NULL_ASSERT( pState );
    {
        NNSi_G2dInitializeVRamLocation( &pState->dstVramLocation );
        pState->szDst = 0;
        pState->pSrcNCGR = NULL;
        pState->pSrcNCBR = NULL;
        pState->szSrcData = 0;
        pState->bActive = FALSE;
        
        
        pState->bDrawn              = 0x0;
        pState->bTransferRequested  = 0x0;
        pState->srcOffset           = 0x0;
        pState->szByte              = 0x0;
    }
}
//------------------------------------------------------------------------------
// Library Internal Release
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
NNSG2dCellTransferState* 
NNSi_G2dGetCellTransferState
( 
    u32 handle 
)
{
    return GetValidCellTransferState_( handle );
}

//------------------------------------------------------------------------------
// Sets the transfer request to the transfer state management object of the specified handle.
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
)
{
    NNS_G2D_ASSERT( IsValidHandle_( handle ) );
    NNS_G2D_NON_ZERO_ASSERT( szDst );
    NNS_G2D_NON_ZERO_ASSERT( szSrcData );
    NNS_G2D_NULL_ASSERT( szDst );
    
    {
        NNSG2dCellTransferState* pState 
            = GetValidCellTransferState_( handle );
        
        NNS_G2D_NULL_ASSERT( pState );
        
        // one of them should be valid
        NNS_G2D_ASSERT( dstAddr3D       != NNS_G2D_VRAM_ADDR_NONE   ||
                        dstAddr2DMain   != NNS_G2D_VRAM_ADDR_NONE   || 
                        dstAddr2DSub    != NNS_G2D_VRAM_ADDR_NONE    );
        {
            NNSG2dVRamLocation* pImg = &pState->dstVramLocation;
            
            NNSi_G2dInitializeVRamLocation( pImg );
            
            if( dstAddr3D != NNS_G2D_VRAM_ADDR_NONE )
            {
                NNSi_G2dSetVramLocation( pImg, NNS_G2D_VRAM_TYPE_3DMAIN, dstAddr3D );
            }
            
            if( dstAddr2DMain != NNS_G2D_VRAM_ADDR_NONE )
            {
                NNSi_G2dSetVramLocation( pImg, NNS_G2D_VRAM_TYPE_2DMAIN, dstAddr2DMain );
            }
            
            if( dstAddr2DSub != NNS_G2D_VRAM_ADDR_NONE )
            {
                NNSi_G2dSetVramLocation( pImg, NNS_G2D_VRAM_TYPE_2DSUB , dstAddr2DSub );
            }
        }
        
        // both NULL clearly means invalid
        NNS_G2D_ASSERT( pSrcNCGR != NULL || pSrcNCBR != NULL );
        
        pState->szDst     = szDst;
        pState->pSrcNCGR  = pSrcNCGR;
        pState->pSrcNCBR  = pSrcNCBR;
        pState->szSrcData = szSrcData;
    }
}




//------------------------------------------------------------------------------
// initialization related
//------------------------------------------------------------------------------
/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitCellTransferStateManager

  Description:  Initializes the cell VRAM transfer state object manager.
                Cell VRAM transfer state object buffer is passed as argument.
                pTaskRegisterFunc is the pointer to the function that requests registration of VRAM transfer task.
                
                
  Arguments:    pCellStateArray          [OUT] cell VRAM transfer state object buffer 
                numCellState             [IN]  cell VRAM transfer state object buffer length 
                pTaskRegisterFunc        [IN]  pointer to VRAM transfer register function 
                    
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void 
NNS_G2dInitCellTransferStateManager
( 
    NNSG2dCellTransferState*            pCellStateArray, 
    u32                                 numCellState,
    VramTransferTaskRegisterFuncPtr     pTaskRegisterFunc
)
{
    NNS_G2D_NULL_ASSERT( pCellStateArray );
    NNS_G2D_NON_ZERO_ASSERT( numCellState );
    NNS_G2D_NULL_ASSERT( pTaskRegisterFunc );
    
    
    s_pTaskRegisterFunc     = pTaskRegisterFunc;
    
    s_pCellStateArray       = pCellStateArray;
    s_numCellState          = numCellState;
    
    //
    // reset all NNSG2dCellTransferState
    //
    {
        u32     i;
        for( i = 0; i < numCellState; i++ )
        {
            ResetCellTransferState_( &pCellStateArray[i] );
        }
    }
}


//------------------------------------------------------------------------------
// processes relating to transfers
//------------------------------------------------------------------------------

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dUpdateCellTransferStateManager

  Description:  Updates the transfer state manager.
                
                Call this function at every frame animation update, after the rendering request has been completed and before VRAM transfers.
                (The position of the call is critical.)
                
                Determines whether VRAM transfer generation is needed, and, when needed, generates a task for all transfer state objects internally registered.
                
                
                
                
  Arguments:    None.
                    
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dUpdateCellTransferStateManager()
{
    u32 i;
    // for all VRAM transfer objects being used....
    for( i = 0; i < s_numCellState; i++ )
    {
        NNS_G2D_VRAM_TYPE           type;
        NNSG2dCellTransferState*  pState                = &s_pCellStateArray[i];
        
        // Active?
        if( pState->bActive )
        {
            //
            // for all types of drawing HW....
            //
            for( type = NNS_G2D_VRAM_TYPE_3DMAIN; type < NNS_G2D_VRAM_TYPE_MAX; type++ )
            {
                //
                // if transfer is necessary....
                //
                
                if( ShouldRegisterAsVramTransferTask_( pState, type ) )
                {
                    
                    // get from manager                   
                    // Create                    
                    if( MakeVramTransferTask_( pState, type ) )
                    {
                        // OS_Printf( "VRAM transfer task is registered ! surface_Id = %d \n", type );
                        // Reset registration request state (only for VRAM types that have been transferred)
                        NNSi_G2dSetCellTransferStateRequestFlag( pState, type, FALSE );                    
                    }else{
                        // registration failed
                        // TODO : Warning message? assert
                    }
                }
            }
            
            //
            // reset drawing state (for all VRAM types)
            //
            ResetCellTransferStateDrawnFlag_( pState );
        }
    }
}





//------------------------------------------------------------------------------
// Processes related to Registration
//------------------------------------------------------------------------------
/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetCellTransferStateRequested

  Description:  Sets the transfer request to the transfer state management object of the specified handle.
                
                
  Arguments:    handle:         [IN] handle of transfer state management object
             srcOffset:         [IN] value of offset from start of transfer source data
                szByte:         [IN]  transfer size
                    
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetCellTransferStateRequested
( 
    u32                                 handle,
    u32                                 srcOffset,
    u32                                 szByte
)
{
    NNS_G2D_ASSERT( IsValidHandle_( handle ) );
    
    {
        NNSG2dCellTransferState* pState 
            = NNSi_G2dGetCellTransferState( handle );
        
        NNS_G2D_NULL_ASSERT( pState );
        NNS_G2D_ASSERT( szByte <= pState->szDst );
        //
        // transition to transfer request state
        //
        pState->bTransferRequested    = 0xFFFFFFFF;// memo: when turning on, everything must be turned on together
        pState->srcOffset             = srcOffset;
        pState->szByte                = szByte;       
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetNewCellTransferStateHandle

  Description:  Gets the handle to the cell VRAM transfer state object.
                This handle is used to control the cell VRAM transfer state object.
                The handle is stored as a member of the cell animation instance.
                If getting the handle fails, NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE is returned.
                Internally, the function performs a linear search for the cell VRAM transfer state object.
                (Avoid calling this function in performance-critical places.)

                
                
  Arguments:    None.
                    
  Returns:      cell VRAM transfer state object handle
                If getting the handle fails, NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE is returned.
  
 *---------------------------------------------------------------------------*/
u32 
NNS_G2dGetNewCellTransferStateHandle()
{
    NNS_G2D_ASSERT( IsCellTransferStateManagerValid_() );
    
    
    //
    // Search from the start of the array of transfer state objects not being used.
    // 
    {
        u32 i = 0;
        for( i = 0;i < s_numCellState; i++ )
        {
            if( s_pCellStateArray[ i ].bActive != TRUE )
            {
                s_pCellStateArray[ i ].bActive = TRUE;
                return i;
            }
        }
    }    
    
    // could not be found
    return NNS_G2D_INVALID_CELL_TRANSFER_STATE_HANDLE;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dFreeCellTransferStateHandle

  Description:  Returns the handle of the cell VRAM transfer state object.
                Execute this function for handles no longer being used.


                
                
  Arguments:    handle              [IN] handle of the cell VRAM transfer state object
                    
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void
NNS_G2dFreeCellTransferStateHandle( u32 handle )
{
    NNS_G2D_ASSERT( IsValidHandle_( handle ) );
    NNS_G2D_ASSERT( IsCellTransferStateManagerValid_() );
    
    ResetCellTransferState_( GetValidCellTransferState_( handle ) );    
}

   
    
