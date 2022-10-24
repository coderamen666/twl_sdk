/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - demos - relayStation-2
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


/*****************************************************************************/
/* Constants */

/*
 * GGID used for testing.
 * This sample functions as a relay station for simple-1.
 */
#define SDK_MAKEGGID_SYSTEM(num)              (0x003FFF00 | (num))
#define GGID_ORG_1                            SDK_MAKEGGID_SYSTEM(0x51)

/* Send/receive data size for testing */
#define DATA_SIZE (1024 * 20)


/*****************************************************************************/
/* Variables */

/* If the relay station is in relay mode: TRUE */
extern BOOL station_is_relay;

/* Currently registered send data */
extern u8 send_data[DATA_SIZE];

/* The original owner of the above send data */
extern u8 send_data_owner[6];


/*****************************************************************************/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         User_Init

  Description:  User-side initialization operation for WXC library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    User_Init(void);



#endif /* NITRO_BUILD_DEMOS_INCLUDE_USER_H_ */
