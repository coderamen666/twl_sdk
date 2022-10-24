/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WBT - demos - wbt-1
  File:     bt.h

  Copyright 2005-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_BUILD_DEMOS_WBT_WBT1_INCLUDE_BT_H_
#define	NITRO_BUILD_DEMOS_WBT_WBT1_INCLUDE_BT_H_

#include	<nitro/wbt.h>

/* Number of block data */
#define NUM_OF_BT_LIST 3

/* Size of 1 data block */
#define BT_DATA_SIZE 0x10000

//Parent packet size
#define BT_PARENT_PACKET_SIZE WC_PARENT_DATA_SIZE_MAX

//Child packet size
#define BT_CHILD_PACKET_SIZE  WC_CHILD_DATA_SIZE_MAX

#define     SDK_MAKEGGID_SYSTEM(num)    (0x003FFF00 | (num))
#define     GGID_WBT_1                  SDK_MAKEGGID_SYSTEM(0x30)
#define     WC_PARENT_DATA_SIZE_MAX     128
#define     WC_CHILD_DATA_SIZE_MAX      16


/* Block registration function for parent */
extern void bt_register_blocks(void);

/* Callback function for block transmission */
extern void bt_callback(void *arg);

/* Block transmission start function for child */
extern void bt_start(void);

/* Block transmission end function for child */
extern void bt_stop(void);

#endif /* NITRO_BUILD_DEMOS_WBT_WBT1_INCLUDE_BT_H_ */
