/*---------------------------------------------------------------------------*
  Project:  NitroSDK - WM - demos - wireless-all
  File:     wh_config.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: wh_config.h,v $
  Revision 1.1  2006/04/10 13:07:32  yosizaki
  Initial upload.

  $NoKeywords$
 *---------------------------------------------------------------------------*/

#ifndef WM_DEMO_WIRELESS_ALL_WH_CONFIG_H_
#define WM_DEMO_WIRELESS_ALL_WH_CONFIG_H_


/* The GGID used in this demo */
#define WH_GGID                   SDK_MAKEGGID_SYSTEM(0x15)

// DMA number used for wireless communications
#define WH_DMA_NO                 2

#define WH_PLAYER_MAX             4

// Maximum number of children (this number does not include the parent)
#define WH_CHILD_MAX              (WH_PLAYER_MAX - 1)

// Maximum size of data that can be shared
#define WH_DS_DATA_SIZE           32

#define WH_PARENT_MAX_SIZE        512
#define WH_CHILD_MAX_SIZE       (WH_DS_DATA_SIZE)

// Port to use for normal MP communications
#define WH_DATA_PORT              14

// Priority to use for normal MP communications
#define WH_DATA_PRIO              WM_PRIORITY_NORMAL

// Continuous transfer mode to use WFS and data sharing simultaneously
#define WH_MP_FREQUENCY           0

// Port to use for data sharing
#define WH_DS_PORT                13

// Port to use for WFS
#define PORT_WFS                  4

#endif /* WM_DEMO_WIRELESS_ALL_WH_CONFIG_H_ */
