/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_config.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/
#ifndef NNS_G2D_CONFIG_H_
#define NNS_G2D_CONFIG_H_

#include <nnsys/inline.h>
#define NNS_G2D_INLINE NNS_INLINE


#ifndef NNS_FROM_TOOL
#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif



// Changed NNS_G2D_DMA_NO from 3 to GX_GetDefaultDMA().
// #define NNS_G2D_DMA_NO 3 
#define NNS_G2D_DMA_NO      GX_GetDefaultDMA() 


#define NNS_G2D_ASSERTMSG               SDK_ASSERTMSG
#define NNS_G2D_ASSERT                  SDK_ASSERT
#define NNS_G2D_NULL_ASSERT             SDK_NULL_ASSERT
#define NNS_G2D_MINMAX_ASSERT           SDK_MINMAX_ASSERT
#define NNS_G2D_MIN_ASSERT              SDK_MIN_ASSERT
#define NNS_G2D_MAX_ASSERT              SDK_MAX_ASSERT
#define NNS_G2D_WARNING                 SDK_WARNING
#define NNS_G2D_NON_ZERO_ASSERT( val )  SDK_ASSERTMSG( (val) != 0, "Non zero value is expexted in the context." )


// Enable the cell size limits of past versions
//#define NNS_G2D_LIMIT_CELL_X_128

// Use font resource from old version
#define NNS_G2D_FONT_USE_OLD_RESOURCE

// Enable support for vertical writing/vertical holding.
#define NNS_G2D_FONT_ENABLE_DIRECTION_SUPPORT

// When a double affine object is included in the input data, do we assume that that double affine object position interpolation will occur?
// 
#define NNS_G2D_ASSUME_DOUBLEAFFINE_OBJPOS_ADJUSTED


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif  // ifndef NNS_FROM_TOOL

#endif // NNS_G2D_CONFIG_H_
