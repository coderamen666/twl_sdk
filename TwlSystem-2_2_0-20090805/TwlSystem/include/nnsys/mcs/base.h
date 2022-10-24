/*---------------------------------------------------------------------------*
  Project:  TWL-System - include - nnsys - mcs
  File:     base.h

  Copyright 2004-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Revision: 1496 $
 *---------------------------------------------------------------------------*/

#ifndef NNS_MCS_BASE_H_
#define NNS_MCS_BASE_H_

#include <nitro/types.h>
#include <nnsys/fnd/list.h>
#include <isdbglib.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================
    Constant Definitions
   ======================================================================== */

#define NNS_MCS_WORKMEM_SIZE        sizeof(NNSiMcsWork)


/* =======================================================================
    Type Definitions
   ======================================================================== */


// Device Information
typedef struct NNSMcsDeviceCaps NNSMcsDeviceCaps;
struct NNSMcsDeviceCaps
{
    u32         deviceID;           // Device recognition ID
    u32         maskResource;       // Resources needed to operate this device
};


// Callback function type when receiving
typedef void (*NNSMcsRecvCallback)(const void* recv, u32 recvSize, u32 userData, u32 offset, u32 totalSize);


// Callback function information when receiving
typedef struct NNSMcsRecvCBInfo NNSMcsRecvCBInfo;
struct NNSMcsRecvCBInfo
{
    u32                 channel;
    NNSMcsRecvCallback  cbFunc;
    u32                 userData;
    NNSFndLink          link;
};

// MCS working memory
typedef struct NNSiMcsWork NNSiMcsWork;
struct NNSiMcsWork
{
    u8                  bProtocolError;                     // mcs library-level conflict
    u8                  bLengthEnable;                      // TRUE if length is obtained and it is a non-zero value
    u8                  bHostDataRecived;                   // TRUE if the data was received from host at first
    u8                  padding;

    OSMutex             mutex;
    NNSFndList          recvCBInfoList;                     // List of callback functions when receiving data

    NNSiMcsMsg          writeBuf;                           // Write buffer
};


/* =======================================================================
    Function Prototypes
   ======================================================================== */

#if defined(NNS_FINALROM)

    #define             NNS_McsInit(workMem)    ( (void)(workMem) )

    #define             NNS_McsFinalize()       ((void)0)

    #define             NNS_McsGetMaxCaps()     (0)

    #define             NNS_McsOpen(pCaps)      ( (void)(pCaps), FALSE )

    #define             NNS_McsClose()          (FALSE)

    #define             NNS_McsRegisterRecvCallback(pInfo, channel, cbFunc, userData)   ( (void)( (void)( (void)( (void)( (void)(pInfo), (channel) ), (cbFunc) ), (userData) ), 0 ) )

    #define             NNS_McsRegisterStreamRecvBuffer(channel, buf, bufSize)          ( (void)( (void)( (void)( (void)(channel), (buf) ), (bufSize) ), 0 ) )

    #define             NNS_McsUnregisterRecvResource(channel)      ( (void)(channel) )

    #define             NNS_McsGetStreamReadableSize(channel)       ( (void)(channel), 0U )

    #define             NNS_McsGetTotalStreamReadableSize(channel)  ( (void)(channel), 0U )

    #define             NNS_McsReadStream(channel, data, size, pReadSize)   ( (void)( (void)( (void)( (void)(channel), (data) ), (size) ), (pReadSize) ), FALSE )

    #define             NNS_McsGetStreamWritableLength(pLength)             ( (void)(pLength), FALSE )

    #define             NNS_McsWriteStream(channel, data, size)             ( (void)( (void)( (void)(channel), (data) ), (size) ), FALSE )

    #define             NNS_McsClearBuffer()            ((void)0)

    #define             NNS_McsIsServerConnect()        (FALSE)

    #define             NNS_McsPollingIdle()            ((void)0)

    #define             NNS_McsVBlankInterrupt()        ((void)0)

    #define             NNS_McsCartridgeInterrupt()     ((void)0)

/* #if defined(NNS_FINALROM) */
#else

    void                NNS_McsInit(void* workMem);

    void                NNS_McsFinalize();

    int                 NNS_McsGetMaxCaps(void);

    BOOL                NNS_McsOpen(
                            NNSMcsDeviceCaps* pCaps);

    BOOL                NNS_McsClose(void);

    void                NNS_McsRegisterRecvCallback(
                            NNSMcsRecvCBInfo*   pInfo,
                            u16                 channel,
                            NNSMcsRecvCallback  cbFunc,
                            u32                 userData);

    void                NNS_McsRegisterStreamRecvBuffer(
                            u16     channel,
                            void*   buf,
                            u32     bufSize);

    void                NNS_McsUnregisterRecvResource(
                            u16     channel);

    u32                 NNS_McsGetStreamReadableSize(
                            u16     channel);

    u32                 NNS_McsGetTotalStreamReadableSize(
                            u16     channel);

    BOOL                NNS_McsReadStream(
                            u16         channel,
                            void*       data,
                            u32         size,
                            u32*        pReadSize);

    BOOL                NNS_McsGetStreamWritableLength(
                            u32*    pLength);

    BOOL                NNS_McsWriteStream(
                            u16         channel,
                            const void* data,
                            u32         size);

    void                NNS_McsClearBuffer(void);

    BOOL                NNS_McsIsServerConnect(void);

    void                NNS_McsPollingIdle(void);

    void                NNS_McsVBlankInterrupt(void);

    void                NNS_McsCartridgeInterrupt(void);


/* #if defined(NNS_FINALROM) */
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NNS_MCS_BASE_H_ */
#endif
