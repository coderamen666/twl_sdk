/*---------------------------------------------------------------------------*
  Project:  TwlSDK - DSP - include
  File:     dsp.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-11-26#$
  $Rev: 9412 $
  $Author: kitase_hirotake $
 *---------------------------------------------------------------------------*/

#ifndef TWL_DSP_H_
#define TWL_DSP_H_


#ifdef SDK_ARM7
#else

#include <twl/dsp/ARM9/dsp_if.h>

#include <twl/dsp/common/byteaccess.h>
#include <twl/dsp/common/pipe.h>

#include <twl/dsp/ARM9/dsp_util.h>
#include <twl/dsp/ARM9/process.h>

#include <twl/dsp/common/audio.h>

#include <twl/dsp/common/g711.h>

#include <twl/dsp/common/graphics.h>

#endif

/* TWL_DSP_H_ */
#endif
