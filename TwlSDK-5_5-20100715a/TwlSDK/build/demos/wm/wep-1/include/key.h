/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WM - demos - wep-1
  File:     key.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef __KEY_H__
#define __KEY_H__

#define KEY_REPEAT_START 25            // Number of frames until key repeat starts
#define KEY_REPEAT_SPAN  10            // Number of frames between key repeats

typedef struct KeyInfo
{
    u16     cnt;                       // Unprocessed input value
    u16     trg;                       // Press trigger input
    u16     up;                        // Release trigger input
    u16     rep;                       // Press and hold repeat input
}
KeyInfo;

extern void updateKeys(void);
extern KeyInfo *getKeyInfo(void);

#endif
