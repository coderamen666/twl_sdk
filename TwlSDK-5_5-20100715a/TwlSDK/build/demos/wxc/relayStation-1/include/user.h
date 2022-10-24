/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - relayStation-1
  File:     user.h

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

#ifndef NITRO_BUILD_DEMOS_INCLUDE_USER_H_
#define NITRO_BUILD_DEMOS_INCLUDE_USER_H_

/* GGID used for testing */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x53)
#define GGID_ORG_2                            SDK_MAKEGGID_SYSTEM(0x54)

/* Number of selection items on the menu screen */
#define MENU_MIN 1
#define MENU_MAX 3

extern BOOL passby_ggid[MENU_MAX];
extern BOOL keep_flg;
extern int passby_endflg;

void    User_Init(void);

#endif /* NITRO_BUILD_DEMOS_INCLUDE_USER_H_ */
