/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - libraries
  File:     wm_common.c

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

#include "wm_common.h"

/*---------------------------------------------------------------------------*
  Name:         WM_CheckInitialized

  Description:  Checks whether the WM library is in use.

  Arguments:    None.

  Returns:      int: Returns WM_ERRCODE_* type processing results.
 *---------------------------------------------------------------------------*/
WMErrCode WM_CheckInitialized(void)
{
		return WMi_CheckInitialized();
}

/*---------------------------------------------------------------------------*
  Name:         NWM_CheckInitialized

  Description:  Checks whether the new WM library is in use.

  Arguments:    None.

  Returns:      int: Returns an NWM_ERRCODE_* value for the processing results.
 *---------------------------------------------------------------------------*/
NWMRetCode NWM_CheckInitialized(void)
{
#ifdef SDK_TWL
	if(OS_IsRunOnTwl()) //Check the old wireless library state only when running on a TWL system
	{
		return NWMi_CheckInitialized();
	}
#endif
    //Simply return NWM_RETCODE_SUCCESS on a NITRO system because it does not have the new wireless chip
	return NWM_RETCODE_ILLEGAL_STATE;
}
