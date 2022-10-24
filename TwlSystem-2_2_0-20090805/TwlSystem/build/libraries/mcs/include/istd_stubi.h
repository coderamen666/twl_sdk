/*---------------------------------------------------------------------------*
  Project:  TWL-System - libraries - mcs
  File:     istd_stubi.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1488 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_MCS_ISTD_STUBI_H_
#define NNS_MCS_ISTD_STUBI_H_

#ifdef __cplusplus
extern "C" {
#endif

#if ! defined(NNS_FINALROM)

#ifdef SDK_LINK_ISTD

    #include <istdbglib.h>

#else

    void                ISTDHIOInit(void);
    u32                 ISTDHIOGetDevMask(void);
    BOOL                ISTDHIOOpen(u32 fDevMask);
    BOOL                ISTDHIOClose(void);
    BOOL                ISTDSIOSend(u16 chn, const void *pSrc, u32 nSize);

    typedef void        (*ISTDSIORecvCbFunc)(void *pUser, u16 chn, const void *pBuf, u32 nSize);
    void                ISTDSIOSetRecvCallback(ISTDSIORecvCbFunc cbRecv, void *pUser);

    enum {              ISTDSIO_MAX_PAYLOAD_SIZE    = 16384 };

#endif

/* #if ! defined(NNS_FINALROM) */
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_MCS_ISTD_STUBI_H_ */
#endif
