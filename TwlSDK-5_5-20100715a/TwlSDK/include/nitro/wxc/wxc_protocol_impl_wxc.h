/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     wxc_protocol_impl_wxc.h

  Copyright 2005-2009 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef	NITRO_WXC_PROTOCOL_IMPL_WXC_H_
#define	NITRO_WXC_PROTOCOL_IMPL_WXC_H_

#include <nitro.h>


/*****************************************************************************/
/* Constants */


#ifdef  __cplusplus
extern "C" {
#endif


// ImplWork structure for wxc
// This used to be in WXCProtocolContext of protocol.h
typedef struct
{
    WXCBlockInfo req;                  /* Request from self */
    WXCBlockInfo ack;                  /* Response to the target */

    u32     recv_total;                /* Total received packet */
    u32     recv_rest;                 /* Remaining receive packet */

    /* Data receive management information */
    u32     recv_bitmap_buf[WXC_RECV_BITSET_SIZE];

    u16     send_unit;                 /* Unit size of send packet */
    u16     recv_unit;                 /* Unit size of receive packet */

    /* Data send management information */
    u16     recent_index[WXC_RECENT_SENT_LIST_MAX];

    BOOL    executing;
} WXCImplWorkWxc;

extern WXCProtocolImpl wxc_protocol_impl_wxc;


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_PROTOCOL_IMPL_WXC_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
