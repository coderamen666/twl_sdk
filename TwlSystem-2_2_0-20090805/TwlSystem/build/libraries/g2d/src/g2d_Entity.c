/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - g2d
  File:     g2d_Entity.c

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#include <nnsys/g2d/g2d_Entity.h>
#include <nnsys/g2d/g2d_CellAnimation.h>
#include <nnsys/g2d/g2d_MultiCellAnimation.h>
#include <nnsys/g2d/load/g2d_NAN_load.h>
#include "g2d_Internal.h"

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void ResetPaletteTbl_( NNSG2dEntity* pEntity )
{
    
    NNS_G2D_ASSERT_ENTITY_VALID( pEntity );
    
    
    NNS_G2D_NULL_ASSERT( pEntity );
    
    // reset the color palette conversion table
    NNS_G2dResetEntityPaletteTable( pEntity );
}

//------------------------------------------------------------------------------
static NNS_G2D_INLINE void SetCurrentAnimation_( NNSG2dEntity* pEntity )
{
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_ASSERT_ENTITY_VALID( pEntity );
    
    {
        const NNSG2dAnimSequence* pAnimSeq = NNS_G2dGetAnimSequenceByIdx( pEntity->pAnimDataBank,
                                                             pEntity->currentSequenceIdx );
        if( pAnimSeq )
        {
            switch( pEntity->pEntityData->type )
            {
            case NNS_G2D_ENTITYTYPE_CELL:
                NNS_G2dSetCellAnimationSequence( (NNSG2dCellAnimation*)pEntity->pDrawStuff, pAnimSeq );
                break;
            case NNS_G2D_ENTITYTYPE_MULTICELL:
                NNS_G2dSetAnimSequenceToMCAnimation( (NNSG2dMultiCellAnimation*)pEntity->pDrawStuff, pAnimSeq );
                break;
            default:
                NNS_G2D_ASSERT( FALSE );
            }
        }else{
            NNS_G2D_ASSERT( FALSE );
        }
    }   
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dInitEntity

  Description:  Initializes Entity itself.
                
                
  Arguments:    pEntity         [OUT] entity 
                pDrawStuff      [IN]  data for rendering (NNSG2dCellAnimation or NNSG2dMultiCellAnimation) 
                pEntityData     [IN]  entity definition data 
                pAnimDataBank   [IN]  animation data bank 

  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dInitEntity
( 
    NNSG2dEntity*               pEntity, 
    void*                       pDrawStuff, 
    const NNSG2dEntityData*     pEntityData, 
    const NNSG2dAnimBankData*   pAnimDataBank 
)
{
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_NULL_ASSERT( pDrawStuff );
    NNS_G2D_NULL_ASSERT( pEntityData );
    NNS_G2D_NULL_ASSERT( pAnimDataBank );
    
    
    // pEntityData->type
    pEntity->pDrawStuff             = pDrawStuff;
    pEntity->pAnimDataBank          = pAnimDataBank;
    pEntity->pEntityData            = pEntityData;
    pEntity->currentSequenceIdx     = 0;
     
    // Initialization of the color palette conversion table
    ResetPaletteTbl_( pEntity );
}



/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetEntityCurrentAnimation

  Description:  Sets the current playback animation for NNSG2dEntity.
                
                
  Arguments:    pEntity:            [OUT] instance of NNSG2dEntity
                idx:                [IN]  AnimationSequence Index inside NNSG2dEntity
                
                Animations must be loaded.
                Nothing happens unless animations have been loaded.
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetEntityCurrentAnimation( NNSG2dEntity* pEntity, u16 idx )
{
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_NULL_ASSERT( pEntity->pAnimDataBank );
    NNS_G2D_NULL_ASSERT( pEntity->pEntityData );
    
    if( pEntity->pEntityData->animData.numAnimSequence > idx )
    {
        pEntity->currentSequenceIdx = pEntity->pEntityData->animData.pAnimSequenceIdxArray[idx];
        // TODO: This process should be performed at time of initialization on all sequence number array elements!
        NNS_G2D_ASSERT( pEntity->pAnimDataBank->numSequences > pEntity->currentSequenceIdx );
        //
        // process for animation switching
        //
        SetCurrentAnimation_( pEntity );
    }else{
        NNSI_G2D_DEBUGMSG0( FALSE, "Failure of finding animation data in NNS_G2dSetEntityCurrentAnimation()" );
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetColorPaletteTable

  Description:  Sets the color palette conversion table in NNSG2dEntity.
                
                
  Arguments:    pEntity:            [OUT] instance of NNSG2dEntity
                pPlttTbl:           [IN] color palette conversion table
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetEntityPaletteTable
( 
    NNSG2dEntity*           pEntity, 
    NNSG2dPaletteSwapTable* pPlttTbl 
)
{
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_NULL_ASSERT( pPlttTbl );
    
    pEntity->pPaletteTbl = pPlttTbl;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dResetEntityPaletteTable

  Description:  Sets the color palette conversion table in NNSG2dEntity to invalid.
                
                
  Arguments:    pEntity:            [OUT] instance of NNSG2dEntity
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dResetEntityPaletteTable
( 
    NNSG2dEntity*           pEntity
)
{
    NNS_G2D_NULL_ASSERT( pEntity );
    pEntity->pPaletteTbl = NULL;
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsEntityPaletteTblEnable

  Description:  Checks whether the NNSG2dEntity color palette is valid.
                
                
  Arguments:    pEntity:            [IN]  instance of NNSG2dEntity
                
                
                
  Returns:      TRUE if color palette is valid
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dIsEntityPaletteTblEnable( const NNSG2dEntity* pEntity )
{
    NNS_G2D_NULL_ASSERT( pEntity );
    return ( pEntity->pPaletteTbl != NULL ) ? TRUE : FALSE;
}


/*---------------------------------------------------------------------------*
  Name:         NNS_G2dTickEntity

  Description:  Updates NNSG2dEntity.
                
                
  Arguments:    pEntity:            [OUT] instance of NNSG2dEntity
                dt:                 [IN] time to advance (in frames)
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void  NNS_G2dTickEntity( NNSG2dEntity*    pEntity, fx32 dt )
{
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_ASSERT_ENTITY_VALID( pEntity );
    
    switch( pEntity->pEntityData->type )
    {
    case NNS_G2D_ENTITYTYPE_CELL:
        NNS_G2dTickCellAnimation( (NNSG2dCellAnimation*)pEntity->pDrawStuff, dt );
        break;
    case NNS_G2D_ENTITYTYPE_MULTICELL:
        NNS_G2dTickMCAnimation( (NNSG2dMultiCellAnimation*)pEntity->pDrawStuff, dt );
        break;
    default:
        NNS_G2D_ASSERT(FALSE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetEntityCurrentFrame

  Description:  Sets the playback animation frame for NNSG2dEntity.
                
                
  Arguments:    pEntity:            [OUT] instance of NNSG2dEntity
                frameIndex:         [IN]  animation frame number
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void  NNS_G2dSetEntityCurrentFrame
( 
    NNSG2dEntity*    pEntity,
    u16              frameIndex  
)
{
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_ASSERT_ENTITY_VALID( pEntity );
    
    switch( pEntity->pEntityData->type )
    {
    case NNS_G2D_ENTITYTYPE_CELL:
        NNS_G2dSetCellAnimationCurrentFrame( (NNSG2dCellAnimation*)pEntity->pDrawStuff, 
                                             frameIndex );
        break;
    case NNS_G2D_ENTITYTYPE_MULTICELL:
        NNS_G2dSetMCAnimationCurrentFrame( (NNSG2dMultiCellAnimation*)pEntity->pDrawStuff,
                                             frameIndex );
        break;
    default:
        NNS_G2D_ASSERT(FALSE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dSetEntitySpeed

  Description:  Sets the animation speed for NNSG2dEntity.
                
                
  Arguments:    pEntity:            [OUT] instance of NNSG2dEntity
                speed:              [IN]  animation speed
                
                
  Returns:      None.
  
 *---------------------------------------------------------------------------*/
void NNS_G2dSetEntitySpeed
(
    NNSG2dEntity*     pEntity,
    fx32              speed  
)
{
    NNS_G2D_NULL_ASSERT( pEntity );
    NNS_G2D_ASSERT_ENTITY_VALID( pEntity );
    
    switch( pEntity->pEntityData->type )
    {
    case NNS_G2D_ENTITYTYPE_CELL:
        NNS_G2dSetCellAnimationSpeed( (NNSG2dCellAnimation*)pEntity->pDrawStuff, 
                                             speed );
        break;
    case NNS_G2D_ENTITYTYPE_MULTICELL:
        NNS_G2dSetMCAnimationSpeed( (NNSG2dMultiCellAnimation*)pEntity->pDrawStuff,
                                             speed );
        break;
    default:
        NNS_G2D_ASSERT(FALSE);
    }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsEntityValid

  Description:  Checks whether NNSG2dEntity is in the valid state.
                
                
  Arguments:    pEntity:            [IN]  instance of NNSG2dEntity
                
                
  Returns:      TRUE if state is valid
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dIsEntityValid( NNSG2dEntity*    pEntity )
{
    NNS_G2D_NULL_ASSERT( pEntity );
    
    if( ( pEntity->pEntityData     != NULL ) &&
        ( pEntity->pDrawStuff      != NULL ) &&
        ( pEntity->pAnimDataBank   != NULL )  )
        {
            return TRUE;
        }else{
            return FALSE;
        }
}

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dGetEntityAnimCtrl

  Description:  Gets the entity's animation controller.
                
                
  Arguments:    pEntity:            [IN]  instance of NNSG2dEntity
                
                
  Returns:      Animation controller
  
 *---------------------------------------------------------------------------*/
NNSG2dAnimController* NNS_G2dGetEntityAnimCtrl( NNSG2dEntity*    pEntity )
{
    NNS_G2D_NULL_ASSERT( pEntity );
    switch( pEntity->pEntityData->type )
    {
    case NNS_G2D_ENTITYTYPE_CELL:
        return NNS_G2dGetCellAnimationAnimCtrl( (NNSG2dCellAnimation*)pEntity->pDrawStuff );
    case NNS_G2D_ENTITYTYPE_MULTICELL:
        return NNS_G2dGetMCAnimAnimCtrl( (NNSG2dMultiCellAnimation*)pEntity->pDrawStuff );
    default:
        NNS_G2D_ASSERT(FALSE);
        return NULL;
    }
}



