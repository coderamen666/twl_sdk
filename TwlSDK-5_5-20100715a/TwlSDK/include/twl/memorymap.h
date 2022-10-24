/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - twl
  File:     memorymap.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_MEMORYMAP_H_
#define TWL_MEMORYMAP_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/
#ifdef SDK_TWL

#ifdef  SDK_ARM9
#include    <twl/hw/ARM9/mmap_global.h>
#include    <twl/hw/ARM9/mmap_main.h>
#include    <twl/hw/ARM9/mmap_tcm.h>
#include    <twl/hw/ARM9/mmap_vram.h>
#include    <twl/hw/common/mmap_shared.h>
#include    <twl/hw/common/mmap_parameter.h>
#include    <twl/hw/ARM9/ioreg.h>

#else   /* SDK_ARM7 */
#include    <twl/hw/ARM7/mmap_global.h>
#include    <twl/hw/ARM7/mmap_main.h>
#include    <twl/hw/ARM7/mmap_wram.h>
#include    <twl/hw/common/mmap_shared.h>
#include    <twl/hw/common/mmap_parameter.h>
#include    <twl/hw/ARM7/ioreg.h>
#endif

#endif //SDK_TWL
/*---------------------------------------------------------------------------*/
#ifdef  __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_MEMORYMAP_H_ */
