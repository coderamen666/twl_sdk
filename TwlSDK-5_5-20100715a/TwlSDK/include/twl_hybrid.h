/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     twl_hybrid.h

  Copyright 2008-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_HYBRID_H_
#define TWL_HYBRID_H_

#ifdef  SDK_ASM
//-------------------------------- SDK_ASM
#ifndef  SDK_TWL
//---- NITRO
#include <nitro/spec.h>
#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/hw/common/armArch.h>
#else
//---- TWL
#include <twl/spec.h>
#include <twl/types.h>
#include <twl/memorymap.h>
#include <twl/hw/common/armArch.h>
#endif //SDK_NITRO

#else  //SDK_ASM
//-------------------------------- not SDK_ASM
//
// Workaround for no macro warnings on CW:
//  - __embedded_cplusplus in MSL_C\MSL_ARM\Include\ansi_prefix.ARM.h
//  - __VEC__              in MSL_C\MSL_Common\Include\stdlib.h
//  - _MSL_USE_INLINE      in MSL_C\MSL_ARM\Include\math.ARM.h, 
//
#ifdef        __MWERKS__
#  ifdef      __cplusplus
#    ifndef   __embedded_cplusplus
#      define __embedded_cplusplus 0
#    endif
#    ifndef   __VEC__
#      define __VEC__			0
#    endif
#  endif
#  if         !defined(_MSL_USE_INLINE) && __option(dont_inline)
#  define     _MSL_USE_INLINE		0
#  endif
#endif

#ifndef SDK_TWL
//---------------- for NITRO
#include <nitro/spec.h>
#include <nitro/misc.h>
#include <nitro/init/crt0.h>

#include <nitro/types.h>
#include <nitro/memorymap.h>
#include <nitro/hw/common/armArch.h>
#include <nitro/hw/common/lcd.h>

#include <nitro/os.h>
#include <nitro/mi.h>
#else
//---------------- for TWL
#include <twl/spec.h>
#include <twl/misc.h>
#include <twl/init/crt0.h>

#include <twl/types.h>
#include <twl/memorymap.h>
#include <twl/hw/common/armArch.h>
#include <nitro/hw/common/lcd.h>

#include <twl/os.h>
#include <twl/mi.h>
#endif // SDK_NITRO

//---------------- for common
#include <nitro/pxi.h>
#include <nitro/pad.h>

#ifndef SDK_TWL
//---------------- for NITRO
#include <nitro/spi.h>
#include <nitro/rtc.h>
#include <nitro/snd.h>
#else
//---------------- for TWL
#include <twl/spi.h>
#include <twl/rtc.h>
#include <twl/snd.h>
#endif

//---------------- for common
#include <nitro/card.h>
#include <nitro/fs.h>
#include <nitro/gx.h>
#include <nitro/wm.h>
#include <nitro/wvr.h>
#include <nitro/ctrdg.h>
#include <nitro/math.h>
//#include <nitro/utest.h>
#include <nitro/std.h>

#ifdef SDK_TWL
//---------------- for TWL
#include <twl/nwm.h>
#include <twl/scfg.h>
#include <twl/camera.h>
#include <twl/dsp.h>
#endif

//---------------- for common
#ifdef  SDK_ARM9
//---- only for ARM9
#include <nitro/fx/fx.h>
#include <nitro/fx/fx_const.h>
#include <nitro/fx/fx_trig.h>
#include <nitro/fx/fx_cp.h>
#include <nitro/fx/fx_vec.h>
#include <nitro/fx/fx_mtx.h>
#include <nitro/cp.h>
#include <nitro/ext.h>
#include <nitro/mb.h>
#include <nitro/wbt.h>
#include <nitro/wfs.h>
#include <nitro/env.h>
#include <nitro/vib.h>

#else  //SDK_ARM7
//---- only for ARM7
#include <nitro/exi.h>

#endif //SDK_ARM7

//------------------------------------------------
#endif //SDK_ASM

/* TWL_HYBRID_H_ */
#endif
