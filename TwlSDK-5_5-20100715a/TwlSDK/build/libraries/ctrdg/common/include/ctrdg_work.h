/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CTRDG - include
  File:     ctrdg_work.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_LIBRARY_CTRDG_WORK_H_
#define NITRO_LIBRARY_CTRDG_WORK_H_


#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------
// for ctrdg common work
//
typedef struct CTRDGWork
{
    vu16    subpInitialized;
    u16     lockID;

}
CTRDGWork;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_LIBRARY_CTRDG_WORK_H_ */
#endif
