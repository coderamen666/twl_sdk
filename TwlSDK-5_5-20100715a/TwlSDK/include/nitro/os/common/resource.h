/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     resource.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-02-21#$
  $Rev: 4282 $
  $Author: yada $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_OS_RESOURCE_H_
#define NITRO_OS_RESOURCE_H_

#ifndef SDK_ASM
#include  <nitro/types.h>

#include  <nitro/os/common/thread.h>
#include  <nitro/os/common/alarm.h>
#include  <nitro/os/common/arena.h>
#include  <nitro/os/common/valarm.h>

#ifdef __cplusplus
extern "C" {
#endif

//---- OS resource (for debug)
typedef struct
{
	BOOL               threadResourceFlag;
	BOOL               alarmResourceFlag;
	BOOL               valarmResourceFlag;
	BOOL               arenaResourceFlag;

	OSThreadResource   threadResource;
	OSAlarmResource    alarmResource;
	OSVAlarmResource   valarmResource;
	OSArenaResource    arenaResource;
}
OSResource;

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
extern BOOL OS_GetAllResources(OSResource *resource);


#endif /* SDK_ASM */

#ifdef __cplusplus
} /* extern "C" */
#endif

/*NITRO_OS_RESOURCE_H_*/
#endif
