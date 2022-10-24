/*---------------------------------------------------------------------------*
  Project:  TwlSDK - -include - PXI
  File:     init.h

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
#ifndef NITRO_PXI_COMMON_INIT_H_
#define NITRO_PXI_COMMON_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         PXI_Init

  Description:  initialize PXI system

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    PXI_Init(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_PXI_COMMON_INIT_H_ */
