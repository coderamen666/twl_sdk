 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_block.h

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

#ifndef _MB_BLOCK_H_
#define _MB_BLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>
#include <nitro/mb.h>
#include <nitro/wm.h>
#include "mb_common.h"


/* ----------------------------------------------------------------------- *
 *  Structure Definitions
 * ----------------------------------------------------------------------- */

/*
 * Transfer block header (header added to each block)
   Actually sent and received in a form that is packed by Padding amount.
 */
typedef struct
{
    u8      type;                      // Data type:                             1B
    u8      pad1[1];
    u16     fid;                       // File number value:                            4B
    u16     seqno;                     // Parent use:                                  6B
}
MBCommParentBlockHeader;


typedef struct
{
    u8      type;                      // Data type:                             1B
    u8      pad1[1];                   //                                          1B
    union                              //                              union total: 16B
    {
        struct
        {
            u16     req;               // Block number request value:                       2B
            u8      reserved[MB_COMM_CALC_REQ_DATA_PIECE_SIZE(MB_COMM_P_RECVLEN_MAX)];  // 14B
        }
        data;

        struct
        {
            u8      piece;             //  Fragmented data number:                1B
            //  Buffer that stores fragmented data
            u8      data[MB_COMM_CALC_REQ_DATA_PIECE_SIZE(MB_COMM_P_RECVLEN_MAX)];      //  14B
            u8      pad2[1];           //  1B
        }
        req_data;
    };
    // Total: 18B
}
MBCommChildBlockHeader;


// Receive buffer for a divided request
typedef struct
{
    u32     data_buf[MB_MAX_CHILD][MB_COMM_CALC_REQ_DATA_BUF_SIZE(MB_COMM_P_RECVLEN_MAX) /
                                   sizeof(u32) + 1];
    u32     data_bmp[MB_MAX_CHILD];
}
MbRequestPieceBuf;

/* ----------------------------------------------------------------------- *
 *  Function Declarations
 * ----------------------------------------------------------------------- */

/* Set child MP transmission size */
void    MBi_SetChildMPMaxSize(u16 childMaxSize);
/* Set parent buffer for receiving divided requests */
void    MBi_SetParentPieceBuffer(MbRequestPieceBuf * buf);
/* Clear the buffer for receiving divided requests */
void    MBi_ClearParentPieceBuffer(u16 child_aid);

/* From the parent's send header information, build the actual data to be sent */
u8     *MBi_MakeParentSendBuffer(const MBCommParentBlockHeader * hdr, u8 *sendbuf);
/* Build a structure from the data buffer received from a child */
u8     *MBi_SetRecvBufferFromChild(const u8 *recvbuf, MBCommChildBlockHeader * hdr, u16 child_id);

/* From the child's send header information, build the actual data to be sent */
u8     *MBi_MakeChildSendBuffer(const MBCommChildBlockHeader * hdr, u8 *sendbuf);
/* Divide request data to send from a child */
u8      MBi_SendRequestDataPiece(u8 *pData, const MBCommRequestData *pReq);
/* Get the header portion from a packet received from the parent and return a pointer to the data portion */
u8     *MBi_SetRecvBufferFromParent(MBCommParentBlockHeader * hdr, const u8 *recvbuf);


#ifdef __cplusplus
}
#endif

#endif /*  _MB_BLOCK_H_    */
