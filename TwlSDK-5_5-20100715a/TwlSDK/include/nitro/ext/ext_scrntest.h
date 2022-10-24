/*---------------------------------------------------------------------------*
  Project:  TwlSDK - screenshot test - Ext
  File:     ext_scrntest.h

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
#ifndef EXT_SCRNTEST_H_
#define EXT_SCRNTEST_H_

#include <nitro/types.h>
#include <nitro/gx/gx_vramcnt.h>

#ifdef __cplusplus
extern "C" {
#endif

void    EXT_Printf(const char *fmt, ...);
void    EXT_CompPrint(const char *src1, const char *src2_fmt, ...);
void    EXT_TestScreenShot(u32 testFrames, u32 checkSum);
void    EXT_TestTickCounter();
void    EXT_TestResetCounter();
void    EXT_TestSetVRAMForScreenShot(GXVRamLCDC vram);

#ifdef __cplusplus
}/* extern "C" */
#endif

#endif /* EXT_SCRNTEST_H_ */
