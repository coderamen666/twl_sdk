/*---------------------------------------------------------------------------*
  Project:  TwlSDK - wireless_shared - demos - wh
  File:     wh_config.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef _DEFAULT_WH_CONFIG_H__
#define _DEFAULT_WH_CONFIG_H__


// DMA number used for wireless communications
#define WH_DMA_NO                 2

// Maximum number of children (this number does not include the parent)
#define WH_CHILD_MAX              15

// Maximum size of data that can be shared
#define WH_DS_DATA_SIZE           12

// Maximum size of data sent in one transmission.
// If you are performing normal communication in addition to data sharing, increase this value only by the amount that those normal communications use.
// You must then add enough for the additional headers and footers that accompany multi-packet transmissions.
// 
// For more details, see docs/TechnicalNotes/WirelessManager.doc.
// GUIDELINE: Guideline compliance point (6.3.2)
// We recommend a value of 5600 microseconds or less for the time required by a single MP transmission, as calculated in the Function Reference Manual under Wireless Manager (WM) > Figures and Information > Wireless communication time calculation sheet.
// 
#define WH_PARENT_MAX_SIZE      (WH_DS_DATA_SIZE * (1 + WH_CHILD_MAX) + WM_SIZE_DS_PARENT_HEADER)
#define WH_CHILD_MAX_SIZE       (WH_DS_DATA_SIZE)

// Upper limit on the number of MP communications for one picture frame
// When multiple protocols are used in parallel (such as data sharing and block transfer), this value must be set greater than 1 (or to 0, specifying no limit).
//  
// Without that, the only protocol that will be allowed to run MP communication will be the one with the highest priority.
//  
#define WH_MP_FREQUENCY           1

// Port to use for normal MP communication
#define WH_DATA_PORT              14

// Priority to use for normal MP communication
#define WH_DATA_PRIO              WM_PRIORITY_NORMAL

// Port to use for data sharing
#define WH_DS_PORT                13



#endif // _DEFAULT_WH_CONFIG_H__
