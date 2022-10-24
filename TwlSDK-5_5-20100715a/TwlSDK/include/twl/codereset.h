/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include
  File:     codereset.h

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
#ifdef	SDK_CODE_ARM                   // switch by default codegen
#include <nitro/code32.h>
#else  //SDK_CODE_THUMB
#include <nitro/code16.h>
#endif
