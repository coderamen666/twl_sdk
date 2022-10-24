/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS
  File:     os_resource.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-04-24#$
  $Rev: 5735 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
//#include <nitro/os.h>
#include <nitro/os/common/resource.h>
#include <nitro/os/common/system.h>

//================================================================================
//   for DEBUG
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         OS_GetAllResources

  Description:  store system resources to specified pointer

  Arguments:    resource       pointer to store system resources

  Returns:      TRUE  ... success
                FALSE ... fail
 *---------------------------------------------------------------------------*/
BOOL OS_GetAllResources(OSResource *resource)
{
    OSIntrMode enabled = OS_DisableInterrupts();
	BOOL flag = TRUE;
	SDK_ASSERT(resource != NULL);

	//---- thread resource
	if (!(resource->threadResourceFlag = OS_GetThreadResource(&resource->threadResource)))
	{
		flag = FALSE;
	}

	//---- alarm resource
	if (!(resource->alarmResourceFlag = OS_GetAlarmResource(&resource->alarmResource)))
	{
		flag = FALSE;
	}

	//---- valarm resource
	if (!(resource->valarmResourceFlag = OS_GetVAlarmResource(&resource->valarmResource)))
	{
		flag = FALSE;
	}

	//---- arena resource
	if (!(resource->arenaResourceFlag = OS_GetArenaResource(&resource->arenaResource)))
	{
		flag = FALSE;
	}

    (void)OS_RestoreInterrupts(enabled);
	return flag;
}
