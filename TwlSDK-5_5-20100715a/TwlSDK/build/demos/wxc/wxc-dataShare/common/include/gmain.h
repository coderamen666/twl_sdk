/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - wxc-dataShare
  File:     gmain.h

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 2414 $
  $Author: hatamoto_minoru $
 *---------------------------------------------------------------------------*/
#ifndef MB_DEMO_GMAIN_H_
#define MB_DEMO_GMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>


typedef struct
{
    u32 count;                         // Frame info
    u16 key;                           // Key information
    u16 padding;
}
GShareData;


//============================================================================
//  Function Declarations
//============================================================================

void GInitDataShare(void);
BOOL GStepDataShare(s32 frame);
void GMain(void);


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // MB_DEMO_GMAIN_H_
