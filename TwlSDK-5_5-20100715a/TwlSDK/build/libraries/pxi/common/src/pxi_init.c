/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PXI - libraries
  File:     pxi_init.c

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
#include  <nitro/pxi.h>

#include  <nitro/misc.h>               // For filling in .version

/*
 * If not SDK_FINALROM, fill .version with a string
 */
#ifndef SDK_FINALROM
#ifdef SDK_ARM9
#include <nitro/version_begin.h>
SDK_DEFINE_MIDDLEWARE(checkString, "NINTENDO", "DEBUG");
#include <nitro/version_end.h>
#endif
#endif

/*---------------------------------------------------------------------------*
  Name:         PXI_Init

  Description:  Initialize the PXI system.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PXI_Init(void)
{
#ifndef SDK_FINALROM
#ifdef SDK_ARM9
    SDK_USING_MIDDLEWARE(checkString);
#endif
#endif
    PXI_InitFifo();
}
