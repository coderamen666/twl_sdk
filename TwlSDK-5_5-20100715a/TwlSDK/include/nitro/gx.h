/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX
  File:     gx.h

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

#ifndef NITRO_GX_H_
#define NITRO_GX_H_

#ifdef __cplusplus
extern "C" {
#endif


#ifdef  SDK_ARM9

#include <nitro/gx/g3.h>
#include <nitro/gx/g3x.h>
#include <nitro/gx/g2.h>
#include <nitro/gx/gx.h>
#include <nitro/gx/gx_vramcnt.h>
#include <nitro/gx/gx_bgcnt.h>
#include <nitro/gx/gx_capture.h>
#include <nitro/gx/g2_oam.h>
#include <nitro/gx/struct_2d.h>
#include <nitro/gx/g3b.h>
#include <nitro/gx/g3c.h>

/* if include from Other Environment for exsample VC or BCB, */
/* please define SDK_FROM_TOOL                               */
#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))

#include <nitro/gx/g3imm.h>
#include <nitro/gx/g3_util.h>
#include <nitro/gx/gx_load.h>

#endif // SDK_FROM_TOOL


#else  //SDK_ARM7

/* if include from Other Environment for exsample VC or BCB, */
/* please define SDK_FROM_TOOL                               */
#if !(defined(SDK_WIN32) || defined(SDK_FROM_TOOL))

#include <nitro/gx/gx_sp.h>

#endif // SDK_FROM_TOOL

#endif




#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_GX_H_ */
#endif
