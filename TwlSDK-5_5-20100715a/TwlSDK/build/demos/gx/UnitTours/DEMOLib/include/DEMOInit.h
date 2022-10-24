/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOInit.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-09-24#$
  $Rev: 11063 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef DEMO_INIT_H_
#define DEMO_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void VBlankIntr(void);

void    DEMOInitCommon(void);
void    DEMOInitVRAM(void);
void    DEMOInitDisplayBG0Only(void);
void    DEMOInitDisplaySubBG0Only(void);
void    DEMOInitDisplayBG2Only(void);
void    DEMOInitDisplaySubBG2Only(void);
void    DEMOInitDisplayOBJOnly(void);
void    DEMOInitDisplaySubOBJOnly(void);
void    DEMOInitDisplay3D(void);
void    DEMOInit(void);

void    DEMOStartDisplay(void);

#ifdef SDK_TWL
void    DEMOCheckRunOnTWL(void);
#endif

#ifdef __cplusplus
}/* extern "C" */
#endif

/* DEMO_INIT_H_ */
#endif
