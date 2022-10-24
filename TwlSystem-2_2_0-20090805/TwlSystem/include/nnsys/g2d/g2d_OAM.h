/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_OAM.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_G2D_OAM_H_
#define NNS_G2D_OAM_H_

#include <nitro.h>
#include <nnsys/g2d/fmt/g2d_Vec_data.h>
#include <nnsys/g2d/g2d_OAM_Types.h>
#include <nnsys/g2d/fmt/g2d_Oam_data.h>
#include <nnsys/g2d/fmt/g2d_Character_data.h>

#include <nnsys/g2d/g2d_Image.h> // TODO: Is this dependency OK? Need to investigate.

#ifdef __cplusplus
extern "C" {
#endif

//
// Aliases of functions with names changed
// Previous functions declared as aliases to preserve compatibility.
// 
#define NNS_G2dInitializeOamManager         NNS_G2dInitOamManagerModule
#define NNS_G2dEntryNewOam                  NNS_G2dEntryOamManagerOam
#define NNS_G2dEntryNewOamWithAffineIdx     NNS_G2dEntryOamManagerOamWithAffineIdx
#define NNS_G2dEntryNewOamAffine            NNS_G2dEntryOamManagerAffine
#define NNS_G2dEntryOamAffine               NNS_G2dSetOamManagerAffine
#define NNS_G2dApplyToHW                    NNS_G2dApplyOamManagerToHW
#define NNS_G2dApplyToHWSoftEmu             NNS_G2dApplyOamManagerToHWSprite               
#define NNS_G2dResetBuffer                  NNS_G2dResetOamManagerBuffer
#define NNS_G2dApplyToHWAndReset            NNS_G2dApplyAndResetOamManagerBuffer
#define NNS_G2dGetOamAttrCapacity           NNS_G2dGetOamManagerOamCapacity
#define NNS_G2dGetOamAffineCapacity         NNS_G2dGetOamManagerAffineCapacity



/*---------------------------------------------------------------------------*
  Name:         NNSG2dOamType

  Description:  Type of NNSG2dOamType
                
 *---------------------------------------------------------------------------*/
typedef enum NNSG2dOamType
{
    NNS_G2D_OAMTYPE_MAIN = 0,               // OAM for the main 2D graphics engine
    NNS_G2D_OAMTYPE_SUB,                    // OAM for the sub 2D graphics engine
    NNS_G2D_OAMTYPE_SOFTWAREEMULATION,      // Emulation using 3D Graphics Engine
    NNS_G2D_OAMTYPE_INVALID,                // Invalid type
    NNS_G2D_OAMTYPE_MAX         
    
}NNSG2dOamType;

#define ASSERT_OAMTYPE( x )                                      \
    NNS_G2D_ASSERT( (x) == NNS_G2D_OAMTYPE_INVALID                || \
                (x) == NNS_G2D_OAMTYPE_MAIN                   || \
                (x) == NNS_G2D_OAMTYPE_SUB                    || \
                (x) == NNS_G2D_OAMTYPE_SOFTWAREEMULATION      || \
                (x) == NNS_G2D_OAMTYPE_MAX       )
                
#define ASSERT_OAMTYPE_VALID( x )                  \
    NNS_G2D_ASSERT( (x) == NNS_G2D_OAMTYPE_MAIN     || \
                (x) == NNS_G2D_OAMTYPE_SUB      || \
                (x) == NNS_G2D_OAMTYPE_SOFTWAREEMULATION )



/*---------------------------------------------------------------------------*
  Name:         NNSG2dOAMManageArea

  Description:  Manage area
                
 *---------------------------------------------------------------------------*/
typedef struct NNSG2dOAMManageArea
{
    u16     fromIdx;                // Manage area  (start number)
    u16     toIdx;                  // Manage area  (end number)
    u16     currentIdx;             // Currently registered location number
}NNSG2dOAMManageArea;

/*---------------------------------------------------------------------------*
  Name:         NNSG2dOamManagerInstance

  Description:  OamManager instance
                
 *---------------------------------------------------------------------------*/
typedef struct NNSG2dOamManagerInstance
{
    NNSG2dOamType         type;                // Oam type
    
    NNSG2dOAMManageArea   managedAttrArea;     // Manager management area (Oam Attribute)
    NNSG2dOAMManageArea   managedAffineArea;   // Manager management area (Affine Parameter)
    
    u16                   GUID;                // ID of NNSG2dOamManagerInstance
    u16                   pad16_;              // Padding
    BOOL                  bFastTransferEnable; // Set during initialization. (User should not manipulate this.)
    fx32                  spriteZoffsetStep;   // The value of the automatic offset used when the sprite is rendered
    
}NNSG2dOamManagerInstance;




//------------------------------------------------------------------------------
// Library Internal Release
//------------------------------------------------------------------------------
void* NNSi_G2dGetOamManagerInternalBufferForDebug( NNSG2dOamType type );




//------------------------------------------------------------------------------
// External Release
//------------------------------------------------------------------------------

void NNS_G2dInitOamManagerModule();

BOOL NNS_G2dGetNewManagerInstance( NNSG2dOamManagerInstance* pMan, u16 from, u16 to, NNSG2dOamType type );
BOOL NNS_G2dInitManagerInstanceAffine( NNSG2dOamManagerInstance* pMan, u16 from, u16 to );


BOOL NNS_G2dGetNewOamManagerInstance
( 
    NNSG2dOamManagerInstance*   pMan, 
    u16                         fromOBJ, 
    u16                         numOBJ, 
    u16                         fromAffine, 
    u16                         numAffine, 
    NNSG2dOamType               type 
);

BOOL NNS_G2dGetNewOamManagerInstanceAsFastTransferMode
( 
    NNSG2dOamManagerInstance*   pMan, 
    u16                         fromOBJ, 
    u16                         numOBJ,
    NNSG2dOamType               type 
);


BOOL NNS_G2dEntryOamManagerOam
( 
    NNSG2dOamManagerInstance*   pMan, 
    const GXOamAttr*            pOam, 
    u16                         num 
);

BOOL NNS_G2dEntryOamManagerOamWithAffineIdx
( 
    NNSG2dOamManagerInstance*   pMan, 
    const GXOamAttr*            pOam, 
    u16                         affineIdx 
);

void NNS_G2dSetOamManagerAffine
( 
    NNSG2dOamType               type, 
    const MtxFx22*              mtx, 
    u16                         idx 
);

u16  NNS_G2dEntryOamManagerAffine( NNSG2dOamManagerInstance* pMan, const MtxFx22* mtx );

                        
void NNS_G2dApplyOamManagerToHW( NNSG2dOamManagerInstance* pMan );

void NNS_G2dApplyOamManagerToHWSprite
( 
    NNSG2dOamManagerInstance*       pMan, 
    const NNSG2dImageAttr*          pTexImageAttr,
    u32                             texBaseAddr,
    u32                             pltBaseAddr
);
                            
void NNS_G2dResetOamManagerBuffer( NNSG2dOamManagerInstance* pMan );

                         
void NNS_G2dApplyAndResetOamManagerBuffer( NNSG2dOamManagerInstance* pMan );
u16  NNS_G2dGetOamManagerOamCapacity( NNSG2dOamManagerInstance* pMan );
u16  NNS_G2dGetOamManagerAffineCapacity( NNSG2dOamManagerInstance* pMan );

GXOamAttr* NNS_G2dGetOamBuffer( NNSG2dOamType type );

//------------------------------------------------------------------------------
// inline 
//------------------------------------------------------------------------------
NNS_G2D_INLINE void NNS_G2dSetOamManagerSpriteZoffsetStep
( 
    NNSG2dOamManagerInstance*   pMan, 
    fx32                        zStep 
)
{
    NNS_G2D_NULL_ASSERT( pMan );
    NNS_G2D_WARNING( zStep <=  0, "AutoZOffsetStep should be smaller than zero." );
    NNS_G2D_WARNING( pMan->type == NNS_G2D_OAMTYPE_SOFTWAREEMULATION, 
       "This function works only for the manager initialized as NNS_G2D_OAMTYPE_SOFTWAREEMULATION type ");
    
    pMan->spriteZoffsetStep = zStep;
}   

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_OAM_H_
