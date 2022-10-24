/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_Entity.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
 
#ifndef NNS_G2D_ENTITY2_H_
#define NNS_G2D_ENTITY2_H_

#include <nitro.h>
#include <nnsys/g2d/g2d_config.h>
#include <nnsys/g2d/g2d_Animation.h>
#include <nnsys/g2d/g2d_PaletteTable.h>


#ifdef __cplusplus
extern "C" {
#endif

//
// Aliases of functions with names changed
// Previous functions declared as aliases to preserve compatibility.
// 
#define NNS_G2dInitializeEntity        NNS_G2dInitEntity
#define NNS_G2dSetCurrentAnimation     NNS_G2dSetEntityCurrentAnimation



#define NNS_G2D_ASSERT_ENTITY_VALID( entity )           \
           NNS_G2D_ASSERTMSG( NNS_G2dIsEntityValid( ( entity ) ),"A Invalid Entity instance was detected." );


/*---------------------------------------------------------------------------*
  Name:         NNSG2dEntity

  Description:  Concept of grouping rendering and animation data.
                Basic structure of game characters.
                
 *---------------------------------------------------------------------------*/
 
typedef struct NNSG2dEntity
{
    void*                         pDrawStuff;                 // rendering data
    const NNSG2dEntityData*       pEntityData;                // reference to static data
    const NNSG2dAnimBankData*     pAnimDataBank;              // associated bank
    u16                           currentSequenceIdx;         // number of the sequence currently being played back
    u16                           pad16_;                     // Padding
    //
    // If NULL, regarded as bPaletteChangeEnable = FALSE.
    // Size of table type is large and is probably shared by multiple characters.
    // Therefore, this was made so the pointer is maintained.
    // 
    NNSG2dPaletteSwapTable*         pPaletteTbl;
    
}NNSG2dEntity;



//------------------------------------------------------------------------------
void    NNS_G2dInitEntity      
( 
    NNSG2dEntity*               pEntity, 
    void*                       pDrawStuff, 
    const NNSG2dEntityData*     pEntityData, 
    const NNSG2dAnimBankData*   pAnimDataBank 
);
void    NNS_G2dSetEntityCurrentAnimation  ( NNSG2dEntity* pEntity, u16 idx );



//------------------------------------------------------------------------------
void NNS_G2dSetEntityPaletteTable   ( NNSG2dEntity* pEntity, NNSG2dPaletteSwapTable* pPlttTbl );
void NNS_G2dResetEntityPaletteTable ( NNSG2dEntity* pEntity );
BOOL NNS_G2dIsEntityPaletteTblEnable ( const NNSG2dEntity* pEntity );



//------------------------------------------------------------------------------
void NNS_G2dTickEntity( NNSG2dEntity*    pEntity, fx32 dt );
void NNS_G2dSetEntityCurrentFrame
( 
    NNSG2dEntity*    pEntity,
    u16              frameIndex  
);

//------------------------------------------------------------------------------
void NNS_G2dSetEntitySpeed
(
    NNSG2dEntity*     pEntity,
    fx32              speed 
);

//------------------------------------------------------------------------------
BOOL            NNS_G2dIsEntityValid( NNSG2dEntity*    pEntity );

//------------------------------------------------------------------------------
NNSG2dAnimController* NNS_G2dGetEntityAnimCtrl( NNSG2dEntity*    pEntity );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_ENTITY_H_
