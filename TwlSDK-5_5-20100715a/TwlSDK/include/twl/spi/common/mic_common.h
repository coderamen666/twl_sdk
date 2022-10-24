/*---------------------------------------------------------------------------*
  Project:  TwlSDK - spi - include
  File:     mic_common.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef TWL_SPI_COMMON_MIC_COMMON_H_
#define TWL_SPI_COMMON_MIC_COMMON_H_
#ifdef  __cplusplus
extern  "C" {
#endif
/*---------------------------------------------------------------------------*/

#include    <nitro/hw/common/armArch.h>

enum
{
    MIC_SAMPLING_RATE_32730 =   (HW_CPU_CLOCK_ARM7 / 32730),
    MIC_SAMPLING_RATE_16360 =   (HW_CPU_CLOCK_ARM7 / 16360),
    MIC_SAMPLING_RATE_10910 =   (HW_CPU_CLOCK_ARM7 / 10910),
    MIC_SAMPLING_RATE_8180  =   (HW_CPU_CLOCK_ARM7 / 8180),
    MIC_SAMPLING_RATE_47610 =   (HW_CPU_CLOCK_ARM7 / 47610),
    MIC_SAMPLING_RATE_23810 =   (HW_CPU_CLOCK_ARM7 / 23810),
    MIC_SAMPLING_RATE_15870 =   (HW_CPU_CLOCK_ARM7 / 15870),
    MIC_SAMPLING_RATE_11900 =   (HW_CPU_CLOCK_ARM7 / 11900)
};

/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
}   /* extern "C" */
#endif
#endif  /* TWL_SPI_COMMON_MIC_COMMON_H_ */
