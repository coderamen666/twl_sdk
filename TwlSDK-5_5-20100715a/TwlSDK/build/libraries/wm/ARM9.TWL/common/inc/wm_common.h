/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_common.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef LIBRARIES_WM_ARM9_WM_COMMON_H__
#define LIBRARIES_WM_ARM9_WM_COMMON_H__

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef SDK_TWL
#include "nwm_arm9_private.h"
#endif
#include "wm_arm9_private.h"
#include <nitro/os.h>

/*---------------------------------------------------------------------------*
    Function definitions
 *---------------------------------------------------------------------------*/
// Prototype for a function that determines whether wireless functionality has been initialized
extern WMErrCode WM_CheckInitialized();
#ifdef SDK_TWL
extern NWMRetCode NWM_CheckInitialized();
#endif
/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* LIBRARIES_WM_ARM9_WM_COMMON_H__ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
