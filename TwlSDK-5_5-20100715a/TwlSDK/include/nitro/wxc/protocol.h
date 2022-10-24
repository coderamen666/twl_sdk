/*---------------------------------------------------------------------------*
  Project:  TwlSDK - WXC - include -
  File:     protocol.h

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

#ifndef	NITRO_WXC_PROTOCOL_H_
#define	NITRO_WXC_PROTOCOL_H_


#include <nitro.h>
#include <nitro/wxc/common.h>


/*****************************************************************************/
/* Constants */

/* Command type */
#define WXC_BLOCK_COMMAND_IDLE      0  /* State with request or response */
#define WXC_BLOCK_COMMAND_INIT      1  /* Send/receive data size and checksum */
#define WXC_BLOCK_COMMAND_SEND      2  /* Send/receive data block */
#define WXC_BLOCK_COMMAND_STOP      3  /* Send/receive cancel */
#define WXC_BLOCK_COMMAND_DONE      4  /* Send/receive completion */
#define WXC_BLOCK_COMMAND_QUIT      5  /* Close communication request (disconnect) */

/* MP package size for single data exchange */
#define WXC_PACKET_BUFFRER_MIN      (int)(WXC_PACKET_SIZE_MIN - sizeof(WXCBlockHeader))
/* Maximum data length (32 KB) used for data exchange */
#define WXC_MAX_DATA_SIZE           (32 * 1024)
#define WXC_RECV_BITSET_SIZE        ((((WXC_MAX_DATA_SIZE + WXC_PACKET_BUFFRER_MIN - 1) / WXC_PACKET_BUFFRER_MIN) + 31) / 32)

/* Index history for prevention of duplicate data during block data response */
#define WXC_RECENT_SENT_LIST_MAX    2

/* Maximum number of data blocks that can be registered */
#define WXC_REGISTER_DATA_MAX       16


/*****************************************************************************/
/* Declaration */

/* Block data exchange information */
typedef struct WXCBlockInfo
{
    u8      phase;                     /* Block data exchange phase */
    u8      command;                   /* Command in the current phase */
    u16     index;                     /* Required sequence number */
}
WXCBlockInfo;

/* Block data exchange packet header */
typedef struct WXCBlockHeader
{
    WXCBlockInfo req;                  /* Request from transmission source */
    WXCBlockInfo ack;                  /* Response to the data destination */
}
WXCBlockHeader;

SDK_STATIC_ASSERT(sizeof(WXCBlockHeader) == 8);


/* Block data information structure */
typedef struct WXCBlockDataFormat
{
    void   *buffer;                    /* Pointer to a buffer */
    u32     length;                    /* Data size */
    u32     buffer_max;                /* Buffer size (match with 'length' for send data) */
    u16     checksum;                  /* Checksum (use MATH_CalcChecksum8()) */
    u8      padding[2];
}
WXCBlockDataFormat;


struct WXCProtocolContext;


/* Structure regulating the communication protocol interface that can be handled by WXC */
typedef struct WXCProtocolImpl
{
    /* String indicating the protocol types */
    const char *name;
    /* Beacon send/receive hook function */
    void    (*BeaconSend) (struct WXCProtocolContext *, WMParentParam *);
    BOOL    (*BeaconRecv) (struct WXCProtocolContext *, const WMBssDesc *);
    /* Hook function called prior to connection. */
    void    (*PreConnectHook) (struct WXCProtocolContext *, const WMBssDesc *, u8 ssid[WM_SIZE_CHILD_SSID]);
    /* MP packet send/receive hook function */
    void    (*PacketSend) (struct WXCProtocolContext *, WXCPacketInfo *);
    BOOL    (*PacketRecv) (struct WXCProtocolContext *, const WXCPacketInfo *);
    /* Initialization function when connecting */
    void    (*Init) (struct WXCProtocolContext *, u16, u16);
    /* Data block registration function */
    BOOL    (*AddData) (struct WXCProtocolContext *, const void *, u32, void *, u32);
    /* Function determining whether data is currently being exchanged */
    BOOL    (*IsExecuting) (struct WXCProtocolContext *);
    /* Pointer to the work area used by each protocols as needed */
    void   *impl_work;
    /* Pointer to manage as a singly linked list */
    struct WXCProtocolImpl *next;
}
WXCProtocolImpl;

/* Structure regulating the registered protocol operation */
typedef struct WXCProtocolRegistry
{
    /* Pointer for managing the singly linked lists in the future */
    struct WXCProtocolRegistry *next;
    /* Protocol information */
    u32     ggid;
    WXCCallback callback;
    WXCProtocolImpl *impl;
    /* Initial automatic send/receive data */
    WXCBlockDataFormat send;           /* Send buffer */
    WXCBlockDataFormat recv;           /* Receive buffer */
}
WXCProtocolRegistry;


/* WXC library communication PCI (protocol control information) */
typedef struct WXCProtocolContext
{
    WXCBlockDataFormat send;           /* Send buffer */
    WXCBlockDataFormat recv;           /* Receive buffer */
    /*
     * Information on send and receive data to use for exchanges.
     * This will ultimately be made into a list instead of an array.
     */
    WXCProtocolRegistry *current_block;
    WXCProtocolRegistry data_array[WXC_REGISTER_DATA_MAX];
    u32     target_ggid;
}
WXCProtocolContext;


/*****************************************************************************/
/* Functions */

#ifdef  __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*
  Name:         WXC_InitProtocol

  Description:  Initializes the WXC library protocol.

  Arguments:    protocol: WXCProtocolContext structure

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_InitProtocol(WXCProtocolContext * protocol);

/*---------------------------------------------------------------------------*
  Name:         WXC_InstallProtocolImpl

  Description:  Adds a new selectable protocol.

  Arguments:    impl: Pointer to the WXCProtocolImpl structure.
                      This structure is used internally until the library is closed.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_InstallProtocolImpl(WXCProtocolImpl * impl);

/*---------------------------------------------------------------------------*
  Name:         WXC_FindProtocolImpl

  Description:  Searches for a protocol with the specified name.

  Arguments:    name: Protocol name

  Returns:      If a protocol with the specified name is found, its pointer is returned.
 *---------------------------------------------------------------------------*/
WXCProtocolImpl *WXC_FindProtocolImpl(const char *name);

/*---------------------------------------------------------------------------*
  Name:         WXC_ResetSequence

  Description:  Reinitializes the WXC library protocol.

  Arguments:    protocol: WXCProtocolContext structure
                send_max: Maximum send packet size
                recv_max: Maximum receive packet size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_ResetSequence(WXCProtocolContext * protocol, u16 send_max, u16 recv_max);

/*---------------------------------------------------------------------------*
  Name:         WXC_AddBlockSequence

  Description:  Sets block data exchange.

  Arguments:    protocol: WXCProtocolContext structure
                send_buf: Send buffer
                send_size: Send buffer size
                recv_buf: Receive buffer
                recv_max: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_AddBlockSequence(WXCProtocolContext * protocol,
                             const void *send_buf, u32 send_size, void *recv_buf, u32 recv_max);

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentBlock

  Description:  Gets the currently selected block data.

  Arguments:    protocol: WXCProtocolContext structure

  Returns:      Currently selected block data.
 *---------------------------------------------------------------------------*/
static inline WXCProtocolRegistry *WXC_GetCurrentBlock(WXCProtocolContext * protocol)
{
    return protocol->current_block;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_GetCurrentBlockImpl

  Description:  Gets the currently selected block data's protocol interface.

  Arguments:    protocol: WXCProtocolContext structure

  Returns:      Currently selected block data.
 *---------------------------------------------------------------------------*/
static inline WXCProtocolImpl *WXC_GetCurrentBlockImpl(WXCProtocolContext * protocol)
{
    return protocol->current_block->impl;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_SetCurrentBlock

  Description:  Selects the specified block data.

  Arguments:    protocol: WXCProtocolContext structure
                target: Data block to be selected

  Returns:      None.
 *---------------------------------------------------------------------------*/
static inline void WXC_SetCurrentBlock(WXCProtocolContext * protocol, WXCProtocolRegistry * target)
{
    protocol->current_block = target;
}

/*---------------------------------------------------------------------------*
  Name:         WXC_FindNextBlock

  Description:  Specifies a GGID and searches for block data.

  Arguments:    protocol: WXCProtocolContext structure
                from: Starting search location.
                      If NULL, search from the beginning. Otherwise, start searching from the block after the specified one.
                              
                ggid: GGID associated with the search block data
                match: Search condition.
                       If TRUE, data with matching GGID will be search target,
                       If FALSE, those with non-matching GGID will be search target.

  Returns:      Returns a pointer to the corresponding block data if it exists and NULL otherwise.
                
 *---------------------------------------------------------------------------*/
WXCProtocolRegistry *WXC_FindNextBlock(WXCProtocolContext * protocol,
                                       const WXCProtocolRegistry * from, u32 ggid, BOOL match);

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconSendHook

  Description:  Hook called at beacon update.

  Arguments:    protocol: WXCProtocolContext structure
                p_param: WMParentParam structure used for next beacon
                         Change inside the function as necessary.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_BeaconSendHook(WXCProtocolContext * protocol, WMParentParam *p_param);

/*---------------------------------------------------------------------------*
  Name:         WXC_BeaconRecvHook

  Description:  Hook called for individual scanned beacons.

  Arguments:    protocol: WXCProtocolContext structure
                p_desc: Scanned WMBssDesc structure

  Returns:      If it is seen as connection target, return TRUE. Otherwise, return FALSE.
 *---------------------------------------------------------------------------*/
BOOL    WXC_BeaconRecvHook(WXCProtocolContext * protocol, const WMBssDesc *p_desc);

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketSendHook

  Description:  Hook called at MP packet transmission.

  Arguments:    protocol: WXCProtocolContext structure
                packet: WXCPacketInfo pointer configuring transmission packet information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_PacketSendHook(WXCProtocolContext * protocol, WXCPacketInfo * packet);

/*---------------------------------------------------------------------------*
  Name:         WXC_PacketRecvHook

  Description:  Hook called at MP packet reception.

  Arguments:    protocol: WXCProtocolContext structure
                packet: WXCPacketInfo pointer configuring reception packet information

  Returns:      TRUE if the other party is disconnected on the protocol side.
 *---------------------------------------------------------------------------*/
BOOL    WXC_PacketRecvHook(WXCProtocolContext * protocol, const WXCPacketInfo * packet);

/*---------------------------------------------------------------------------*
  Name:         WXC_ConnectHook

  Description:  Hook called at connection detection.

  Arguments:    protocol: WXCProtocolContext structure
                bitmap: AID bitmap of the connected target

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_ConnectHook(WXCProtocolContext * protocol, u16 bitmap);

/*---------------------------------------------------------------------------*
  Name:         WXC_DisconnectHook

  Description:  Hook called at disconnection detection.

  Arguments:    protocol: WXCProtocolContext structure
                bitmap: AID bitmap of the disconnected target

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_DisconnectHook(WXCProtocolContext * protocol, u16 bitmap);

/*---------------------------------------------------------------------------*
  Name:         WXC_CallPreConnectHook

  Description:  Hook called before connecting to the communication target.

  Arguments:    protocol: WXCProtocolContext structure
                p_desc: Connection target WMBssDesc structure
                ssid: Buffer that stores the SSID for configuration

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_CallPreConnectHook(WXCProtocolContext * protocol, WMBssDesc *p_desc, u8 *ssid);

/*---------------------------------------------------------------------------*
  Name:         WXC_InitProtocolRegistry

  Description:  Associates GGID and callback to the specified registry structure.

  Arguments:    p_data: WXCProtocolRegistry structure used for registration
                ggid: GGID to be set
                callback: Callback function to the user
                              (Cancel setting if NULL)
                impl: Communication protocol to be adopted

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_InitProtocolRegistry(WXCProtocolRegistry * p_data, u32 ggid, WXCCallback callback,
                                 WXCProtocolImpl * impl);

/*---------------------------------------------------------------------------*
  Name:         WXC_SetInitialExchangeBuffers

  Description:  Sets automatically used data for the initial data exchange.

  Arguments:    p_data: WXCProtocolRegistry structure used for registration
                send_ptr: Pointer to registered data
                send_size: Size of registered data
                recv_ptr: Pointer to the receive buffer
                recv_size: Receive buffer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    WXC_SetInitialExchangeBuffers(WXCProtocolRegistry * p_data, u8 *send_ptr, u32 send_size,
                                      u8 *recv_ptr, u32 recv_size);

WXCProtocolImpl* WXCi_GetProtocolImplCommon(void);
WXCProtocolImpl* WXCi_GetProtocolImplWPB(void);
WXCProtocolImpl* WXCi_GetProtocolImplWXC(void);


#ifdef  __cplusplus
}       /* extern "C" */
#endif


#endif /* NITRO_WXC_PROTOCOL_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
