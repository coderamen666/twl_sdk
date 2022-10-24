/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - HW
  File:     memorymap.h

  Copyright 2003-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_MEMORYMAP_H_
#define NITRO_MEMORYMAP_H_

#ifdef __cplusplus
extern "C" {
#endif
/*---------------------------------------------------------------------------*/
#ifndef SDK_TWL

#ifdef	SDK_ARM9
#include	<nitro/hw/ARM9/mmap_global.h>
#include	<nitro/hw/ARM9/mmap_main.h>
#include	<nitro/hw/ARM9/mmap_tcm.h>
#include	<nitro/hw/ARM9/mmap_vram.h>
#include	<nitro/hw/common/mmap_shared.h>
#include	<nitro/hw/ARM9/ioreg.h>

#else  //SDK_ARM7
#include	<nitro/hw/ARM7/mmap_global.h>
#include	<nitro/hw/ARM7/mmap_main.h>
#include	<nitro/hw/ARM7/mmap_wram.h>
#include	<nitro/hw/common/mmap_shared.h>
#include	<nitro/hw/ARM7/ioreg.h>
#endif

#endif //SDK_NITRO
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
} /* extern "C" */
#endif
/* NITRO_MEMORYMAP_H_ */
#endif
