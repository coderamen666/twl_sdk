/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_wm.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
 
#ifndef _MB_WM_H_
#define _MB_WM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb.h>
#include <nitro/wm.h>

/*
  MB_CALLBACK_CONNECT_FAILED
  MB_CALLBACK_CONNECTED_TO_PARENT
  MB_CALLBACK_DISCONNECTED_FROM_PARENT
  MB_CALLBACK_MP_SEND_ENABLE,
  MB_CALLBACK_MP_CHILD_RECV,
  MB_CALLBACK_MP_CHILD_SENT,
  MB_CALLBACK_MP_CHILD_SENT_TIMEOUT,
  MB_CALLBACK_MP_CHILD_SENT_ERR,
  MB_CALLBACK_API_ERROR,
  MB_CALLBACK_ERROR
*/
/* Format of functions used for MB callbacks */
typedef void (*MBWMCallbackFunc) (u16 type, void *arg);



typedef struct
{
    MBWMCallbackFunc mpCallback;
    u32    *sendBuf;
    u32    *recvBuf;
    BOOL    start_mp_busy;
    u16     sendBufSize;               // Size of send buffer
    u16     recvBufSize;               // Size of receive buffer
    u16     pSendLen;                  // Child receive size in one MP
    u16     pRecvLen;                  // Parent receive size in one MP
    u16     blockSizeMax;              // 
    u16     mpStarted;                 // Flag indicating that MP has started
    u16     endReq;                    // End request
    u16     child_bitmap;              // Child connection state
    u16     mpBusy;                    // MP sending (busy) flag
    u8      _padding[2];
}
MBWMWork;

void    MBi_WMSetBuffer(void *buf);
void    MBi_WMSetCallback(MBWMCallbackFunc callback);
void    MBi_WMStartConnect(WMBssDesc *bssDesc);
void    MBi_ChildStartMP(u16 *sendBuf, u16 *recvBuf);
WMErrCode MBi_MPSendToParent(u32 body_len, u16 pollbmp, u32 *sendbuf);
void    MBi_WMDisconnect(void);
void    MBi_WMReset(void);
void    MBi_WMClearCallback(void);


#ifdef __cplusplus
}
#endif

#endif /*  _MB_WM_H_    */
