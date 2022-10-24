/*---------------------------------------------------------------------------*
  Project:  TwlSDK - init - include
  File:     crt0.h

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

#ifndef NITRO_INIT_CRT0_H_
#define NITRO_INIT_CRT0_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef	SDK_ARM9
void    NitroMain(void);
void    NitroStartUp(void);
#else
void    NitroSpMain(void);
void    NitroSpStartUp(void);
#define NitroMain(x)  NitroSpMain(x)
#define NitroStartUp(x)  NitroSpStartUp(x)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_INIT_CRT0_H_ */
#endif
