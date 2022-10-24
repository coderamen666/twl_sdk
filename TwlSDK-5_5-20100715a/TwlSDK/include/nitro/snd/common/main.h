/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - snd - common
  File:     main.h

  Copyright 2004-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_SND_COMMON_MAIN_H_
#define NITRO_SND_COMMON_MAIN_H_

#include <nitro/types.h>
#include <nitro/os.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
	macro definition
 ******************************************************************************/

#define SND_PROC_INTERVAL 0xAA8        // = 5.2095 msec

#define SND_MESSAGE_PERIODIC        1
#define SND_MESSAGE_WAKEUP_THREAD   2

/******************************************************************************
	public function declaration
 ******************************************************************************/

#ifdef SDK_ARM9

void    SND_Init(void);

#else  /* SDK_ARM7 */

void    SND_Init(u32 threadPrio);

void    SND_CreateThread(u32 threadPrio);
BOOL    SND_SetThreadPriority(u32 prio);

void    SND_InitIntervalTimer(void);
void    SND_StartIntervalTimer(void);
void    SND_StopIntervalTimer(void);
OSMessage SND_WaitForIntervalTimer(void);
BOOL    SND_SendWakeupMessage(void);

#endif /* SDK_ARM7 */

/******************************************************************************
	private function declaration
 ******************************************************************************/

void    SNDi_LockMutex(void);
void    SNDi_UnlockMutex(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SND_COMMON_MAIN_H_ */
