/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - include
  File:     wvr.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_WVR_H_
#define NITRO_WVR_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

#ifdef  SDK_ARM9

#include    <nitro/wvr/common/wvr_common.h>
#include    <nitro/wvr/ARM9/wvr.h>

#else  /* SDK_ARM7 */

#include    <nitro/wvr/common/wvr_common.h>
#include    <nitro/wvr/ARM7/wvr_sp.h>

#endif

/*===========================================================================*/

#ifdef  __cplusplus
}       /* extern "C" */
#endif

#endif /* NITRO_WVR_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
