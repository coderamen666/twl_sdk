/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - include
  File:     prc.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef NITRO_PRC_H_
#define NITRO_PRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>

/*===========================================================================*
  Constant Definitions
 *===========================================================================*/

/*===========================================================================*
  Type Definitions
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  Algorithm Dependency Type Declarations
 *---------------------------------------------------------------------------*/

#include <nitro/prc/algo_standard.h>
#include <nitro/prc/algo_light.h>
#include <nitro/prc/algo_fine.h>
#include <nitro/prc/algo_superfine.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_PRC_H_ */
#endif
