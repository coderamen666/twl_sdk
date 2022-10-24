/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - g2d
  File:     g2d_CullingUtility.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1155 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_G2D_CULLINGUTILITY_H_
#define NNS_G2D_CULLINGUTILITY_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <nitro.h>
#include <nnsys/g2d/fmt/g2d_Vec_data.h>

/*---------------------------------------------------------------------------*
  Name:         NNS_G2dIsInViewCircle

  Description:  Determines if in visible area (circle)
                
                
  Arguments:    pvUL:            Visible area rectangle (upper left)
                pvSize:          Visible area rectangle (size)
                pos:             Target object position
                boundingR:      Boundary circle radius      
                
                
  Returns:      In visible area?
  
 *---------------------------------------------------------------------------*/
BOOL NNS_G2dIsInViewCircle( const NNSG2dFVec2* pvUL, const NNSG2dFVec2* pvSize, const NNSG2dFVec2* pos, fx32 boundingR );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif// NNS_G2D_CULLINGUTILITY_H_
