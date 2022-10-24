/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - camera
  File:     camera_intr.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef TWL_CAMERA_INTR_H_
#define TWL_CAMERA_INTR_H_

#include <twl/camera.h>

#ifdef __cplusplus
extern "C" {
#endif


void CAMERA_SetMasterInterruptCore( BOOL enabled );
BOOL CAMERA_GetMasterInterruptCore( void );
void CAMERA_SetVsyncInterruptCore( CAMERAIntrVsync type );
CAMERAIntrVsync CAMERA_GetVsyncInterruptCore( void );
void CAMERA_SetBufferErrorInterruptCore( BOOL enabled );
BOOL CAMERA_GetBufferErrorInterruptCore( void );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TWL_CAMERA_INTR_H_ */
