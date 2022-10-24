/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WVR - demos - with_mb
  File:     gmain.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef MB_DEMO_GMAIN_H_
#define MB_DEMO_GMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>


typedef struct
{
    u32     count;                     // Frame information
    u16     key;                       // Key information
    u16     padding;
}
GShareData;


//============================================================================
//  Function Declarations
//============================================================================

void    GInitDataShare(void);
void    GStepDataShare(u32 frame);
void    GMain(void);


#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif  /* MB_DEMO_GMAIN_H_ */
