/*---------------------------------------------------------------------------*
  Project:  TwlSDK - SPI
  File:     spi.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_SPI_H_
#define NITRO_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <nitro/spi/common/config.h>
#include <nitro/spi/common/type.h>

#ifdef  SDK_ARM9

#include <nitro/spi/ARM9/tp.h>
#include <nitro/spi/ARM9/mic.h>
#include <nitro/spi/ARM9/pm.h>

#else  // SDK_ARM7

#include <nitro/spi/ARM7/spi.h>
#include <nitro/spi/ARM7/pm.h>

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_SPI_H_ */
#endif
