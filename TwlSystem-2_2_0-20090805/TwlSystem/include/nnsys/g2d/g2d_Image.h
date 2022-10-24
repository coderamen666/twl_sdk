/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_Image.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_G2D_IMAGE_H_
#define NNS_G2D_IMAGE_H_

#include <nitro.h>
#include <nnsys/g2d/g2d_config.h>
#include <nnsys/g2d/fmt/g2d_Character_data.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// Aliases of functions with names changed
// Previous functions declared as aliases to preserve compatibility.
// 
#define NNS_G2dInitializeImageProxy         NNS_G2dInitImageProxy
#define NNS_G2dInitializeImagePaletteProxy  NNS_G2dInitImagePaletteProxy 

/*---------------------------------------------------------------------------*
  Name:         NNS_G2D_VRAM_TYPE

  Description:  VRAM type
 *---------------------------------------------------------------------------*/
typedef enum NNS_G2D_VRAM_TYPE
{
    NNS_G2D_VRAM_TYPE_3DMAIN = 0,
    NNS_G2D_VRAM_TYPE_2DMAIN = 1,
    NNS_G2D_VRAM_TYPE_2DSUB  = 2,
    NNS_G2D_VRAM_TYPE_MAX    = 3

}NNS_G2D_VRAM_TYPE;

#define NNS_G2D_VRAM_TYPE_VALID( val )                   \
    NNS_G2D_ASSERTMSG( (val) == NNS_G2D_VRAM_TYPE_3DMAIN ||  \
                   (val) == NNS_G2D_VRAM_TYPE_2DMAIN ||  \
                   (val) == NNS_G2D_VRAM_TYPE_2DSUB  ,   \
                    "Invalid NNS_G2D_VRAM_TYPE type" )   \

#define NNS_G2D_VRAM_ADDR_NOT_INITIALIZED       0xFFFFFFFF
#define NNS_G2D_VRAM_ADDR_NONE                  NNS_G2D_VRAM_ADDR_NOT_INITIALIZED

/*---------------------------------------------------------------------------*
  Name:         NNSG2dImageAttr

  Description:  Image Attributes
                NNSG2dImageProxy has this function.
                
 *---------------------------------------------------------------------------*/
typedef struct NNSG2dImageAttr
{
    GXTexSizeS                      sizeS;          // Image size (Invalid data is set during 1D mapping.)
    GXTexSizeT                      sizeT;          // Image size (Invalid data is set during 1D mapping.)
    
    GXTexFmt                        fmt;            // Image format
    BOOL                            bExtendedPlt;   // Whether or not to use the extended palette
    
    GXTexPlttColor0                 plttUse;        // How to use palette number 0
    GXOBJVRamModeChar               mappingType;    // Mapping mode
}
NNSG2dImageAttr;

/*---------------------------------------------------------------------------*
  Name:         NNSG2dVRamLocation

  Description:  Actual address in VRAM.
                Contains the VRAM type actual address.
                This is located in NNSG2dImageProxy.
                
 *---------------------------------------------------------------------------*/
typedef struct NNSG2dVRamLocation
{
    u32                 baseAddrOfVram[ NNS_G2D_VRAM_TYPE_MAX ];
}
NNSG2dVRamLocation;

/*---------------------------------------------------------------------------*
  Name:         NNSG2dImageProxy

  Description:  Structure that represents an image.
                
 *---------------------------------------------------------------------------*/
typedef struct NNSG2dImageProxy
{
    NNSG2dVRamLocation        vramLocation;           
    NNSG2dImageAttr           attr;
}
NNSG2dImageProxy;

/*---------------------------------------------------------------------------*
  Name:         NNSG2dImagePaletteProxy

  Description:  Structure that represents the image palette.
 *---------------------------------------------------------------------------*/
typedef struct NNSG2dImagePaletteProxy
{
    
    GXTexFmt                  fmt;          // palette format
                                            // (Obtainable values are limited to GX_TEXFMT_PLTT16 and GX_TEXFMT_PLTT256)
                                            
    BOOL                      bExtendedPlt; // Use expanded palette?
    
    NNSG2dVRamLocation        vramLocation;           
}
NNSG2dImagePaletteProxy;


// For Image

void NNS_G2dInitImageProxy   ( NNSG2dImageProxy* pImg );


void NNS_G2dSetImageLocation       ( NNSG2dImageProxy* pImg, NNS_G2D_VRAM_TYPE type, u32 addr );



u32  NNS_G2dGetImageLocation       ( const NNSG2dImageProxy* pImg, NNS_G2D_VRAM_TYPE type );
BOOL NNS_G2dIsImageReadyToUse      ( const NNSG2dImageProxy* pImg, NNS_G2D_VRAM_TYPE type );


void NNS_G2dLoadImage1DMapping
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy 
);

void NNS_G2dLoadImage2DMapping
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy 
);


void NNS_G2dLoadImageVramTransfer
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy 
);

void NNS_G2dLoadPalette
(
    const NNSG2dPaletteData*    pSrcData,
    u32                         addr,
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImagePaletteProxy*    pPltProxy
);

void NNS_G2dLoadPaletteEx
(
    const NNSG2dPaletteData*            pSrcData,
    const NNSG2dPaletteCompressInfo*    pCmpInfo,
    u32                                 addr,
    NNS_G2D_VRAM_TYPE                   type, 
    NNSG2dImagePaletteProxy*            pPltProxy
);

// For ImagePalette
void           NNS_G2dInitImagePaletteProxy( NNSG2dImagePaletteProxy* pImg );
void           NNS_G2dSetImagePaletteLocation( NNSG2dImagePaletteProxy* pImg, NNS_G2D_VRAM_TYPE type, u32 addr );
u32            NNS_G2dGetImagePaletteLocation( const NNSG2dImagePaletteProxy* pImg, NNS_G2D_VRAM_TYPE type );
BOOL           NNS_G2dIsImagePaletteReadyToUse( const NNSG2dImagePaletteProxy* pImg, NNS_G2D_VRAM_TYPE type );
static BOOL    NNS_G2dIsPaletteImageFmt( const NNSG2dImageAttr* imgAttr );


//------------------------------------------------------------------------------
void NNSi_G2dInitializeVRamLocation( NNSG2dVRamLocation* pVramLocation );
void NNSi_G2dSetVramLocation( NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type, u32 addr );
u32  NNSi_G2dGetVramLocation( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type );
BOOL NNSi_G2dIsVramLocationReadyToUse( const NNSG2dVRamLocation* pVramLocation, NNS_G2D_VRAM_TYPE type );

void NNSi_G2dDoImageLoadingToVram
( 
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr,
    NNS_G2D_VRAM_TYPE           type
);
void NNSi_G2dSetupImageProxyPrams
(
    const NNSG2dCharacterData*  pSrcData, 
    u32                         baseAddr, 
    NNS_G2D_VRAM_TYPE           type, 
    NNSG2dImageProxy*           pImgProxy
);

void NNSi_G2dDoLoadingPaletteToVram
(
    const NNSG2dPaletteData*    pSrcData,
    u32                         addr,
    NNS_G2D_VRAM_TYPE           type
);

void NNSi_G2dDoLoadingPaletteToVramEx
(
    const NNSG2dPaletteData*            pSrcData,
    const NNSG2dPaletteCompressInfo*    pCmpInfo,
    u32                                 addr,
    NNS_G2D_VRAM_TYPE                   type   
);

void NNSi_G2dSetupPaletteProxyPrams
(
    const NNSG2dPaletteData*            pSrcData,
    u32                                 addr,
    NNS_G2D_VRAM_TYPE                   type, 
    NNSG2dImagePaletteProxy*            pPltProxy
);

//------------------------------------------------------------------------------
// Inline Functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

NNS_G2D_INLINE BOOL NNS_G2dIsPaletteImageFmt( const NNSG2dImageAttr* imgAttr )
{
    NNS_G2D_NULL_ASSERT( imgAttr );
    return ( imgAttr->fmt == GX_TEXFMT_PLTT4   ||
             imgAttr->fmt == GX_TEXFMT_PLTT16  ||
             imgAttr->fmt == GX_TEXFMT_PLTT256 ) ? TRUE : FALSE;
}

//------------------------------------------------------------------------------
NNS_G2D_INLINE void NNS_G2dSetImageExtPaletteFlag( NNSG2dImageProxy* pImgProxy, BOOL bUseExtPlt )
{
    NNS_G2D_NULL_ASSERT( pImgProxy );
    pImgProxy->attr.bExtendedPlt = bUseExtPlt;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // NNS_G2D_IMAGE_H_
