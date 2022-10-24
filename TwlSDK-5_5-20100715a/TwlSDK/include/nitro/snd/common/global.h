/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - snd - common
  File:     global.h

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

#ifndef NITRO_SND_COMMON_GLOBAL_H_
#define NITRO_SND_COMMON_GLOBAL_H_

#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
	macro definition
 ******************************************************************************/

#define SND_MASTER_VOLUME_MAX        0x7f

/******************************************************************************
	enum definition
 ******************************************************************************/

typedef enum SNDChannelOut
{
    SND_CHANNEL_OUT_MIXER,
    SND_CHANNEL_OUT_BYPASS
}
SNDChannelOut;

typedef enum SNDOutput
{
    SND_OUTPUT_MIXER,
    SND_OUTPUT_CHANNEL1,
    SND_OUTPUT_CHANNEL3,
    SND_OUTPUT_CHANNEL1_3
}
SNDOutput;

/******************************************************************************
	public function declaration
 ******************************************************************************/

#ifdef SDK_ARM7

void    SND_Enable(void);
void    SND_Disable(void);

void    SND_Shutdown(void);

void    SND_BeginSleep(void);
void    SND_EndSleep(void);

void    SND_SetMasterVolume(int volume);

void    SND_SetOutputSelector(SNDOutput left,
                              SNDOutput right, SNDChannelOut channel1, SNDChannelOut channel3);

#endif /* SDK_ARM7 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SND_COMMON_GLOBAL_H_ */

