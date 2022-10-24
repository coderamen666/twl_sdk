/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - libraries
  File:     mb_block.c

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-09#$
  $Rev: 10718 $
  $Author: okajima_manabu $
 *---------------------------------------------------------------------------*/

#include <nitro.h>
#include "mb_common.h"
#include "mb_block.h"
#include "mb_private.h"


//============================================================================
//  Function Prototype Declarations
//============================================================================
static u8 *MBi_ReceiveRequestDataPiece(const MBCommChildBlockHeader * hdr, u16 child);
static BOOL IsGetAllRequestData(u16 child);


//============================================================================
//  Variable Declarations
//============================================================================

// The size and number of a divided request's fragments
static struct
{
    int     size;
    int     num;
    int     bufSize;
}
req_data_piece;

// For child
// Divided request send index.
static u8 req_data_piece_idx = 0;

// For parent
// Receive buffer for a divided request
static MbRequestPieceBuf *req_buf;


//============================================================================
//  Function definitions
//============================================================================

/*---------------------------------------------------------------------------*
  Name:         MBi_SetChildMPMaxSize
  
  Description:  Sets the transmission data size for a child and calculates the divided request size.
                
  
  Arguments:    childMaxSize: Child transmission size
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_SetChildMPMaxSize(u16 childMaxSize)
{
    req_data_piece.size = MB_COMM_CALC_REQ_DATA_PIECE_SIZE(childMaxSize);
    req_data_piece.num = MB_COMM_CALC_REQ_DATA_PIECE_NUM(childMaxSize);
    req_data_piece.bufSize = MB_COMM_CALC_REQ_DATA_BUF_SIZE(childMaxSize);
}


/*---------------------------------------------------------------------------*
  Name:         MBi_SetParentPieceBuffer

  Description:  Sets the receive buffer for a parent's divided request.
  
  Arguments:    buf: Pointer to the receive buffer
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_SetParentPieceBuffer(MbRequestPieceBuf * buf)
{
    req_buf = buf;
    MI_CpuClear8(req_buf, sizeof(MbRequestPieceBuf));
}


/*---------------------------------------------------------------------------*
  Name:         MBi_ClearParentPieceBuffer

  Description:  Clears the buffer for receiving divided requests.
  
  Arguments:    child_aid: Child AID of the buffer to clear
  
  Returns:      None.
 *---------------------------------------------------------------------------*/
void MBi_ClearParentPieceBuffer(u16 child_aid)
{
    // child_aid range check
    SDK_ASSERT(child_aid != 0 && child_aid <= MB_MAX_CHILD);

    if (req_buf == NULL)
    {
        return;
    }
    
    MI_CpuClear8(req_buf->data_buf[child_aid - 1],
                 MB_COMM_CALC_REQ_DATA_BUF_SIZE(req_data_piece.bufSize));
    req_buf->data_bmp[child_aid - 1] = 0;
}


/*---------------------------------------------------------------------------*
  Name:         MBi_MakeParentSendBuffer

  Description:  Builds the actual data to send, using a parent's send header information.

  Arguments:    hdr: Pointer to the parent send header
                sendbuf: Pointer to the buffer that creates the send data

  Returns:      Pointer to send buffer.
 *---------------------------------------------------------------------------*/
u8     *MBi_MakeParentSendBuffer(const MBCommParentBlockHeader * hdr, u8 *sendbuf)
{
    u8     *ptr = sendbuf;

    *ptr++ = hdr->type;

    switch (hdr->type)
    {
    case MB_COMM_TYPE_PARENT_SENDSTART:        //  1
        break;
    case MB_COMM_TYPE_PARENT_KICKREQ: //  2
        break;
    case MB_COMM_TYPE_PARENT_DL_FILEINFO:      //  3
        break;
    case MB_COMM_TYPE_PARENT_DATA:    //  4
        *ptr++ = (u8)(0x00ff & hdr->fid);       // Lo
        *ptr++ = (u8)((0xff00 & hdr->fid) >> 8);        // Hi
        *ptr++ = (u8)(0x00ff & hdr->seqno);     // Lo
        *ptr++ = (u8)((0xff00 & hdr->seqno) >> 8);      // Hi
        break;
    case MB_COMM_TYPE_PARENT_BOOTREQ: //  5
        break;
    case MB_COMM_TYPE_PARENT_MEMBER_FULL:      //  6
        break;
    default:
        return NULL;
    }

    return ptr;
}


/*---------------------------------------------------------------------------*
  Name:         MBi_SetRecvBufferFromChild

  Description:  Get the header portion from a packet received from the child, and return a pointer to the data portion.
                

  Arguments:    hdr: Pointer to a variable for getting the header
                recvbuf: Pointer to the receive buffer

  Returns:      Pointer to the data portion.
 *---------------------------------------------------------------------------*/
u8     *MBi_SetRecvBufferFromChild(const u8 *recvbuf, MBCommChildBlockHeader * hdr, u16 child_id)
{
    u8     *ptr = (u8 *)recvbuf;
    
    SDK_ASSERT(child_id != 0 && child_id <= MB_MAX_CHILD);

    hdr->type = *ptr++;

    switch (hdr->type)
    {
    case MB_COMM_TYPE_CHILD_FILEREQ:
        // Restore fragmented data
        if (IsGetAllRequestData(child_id))
        {
            return (u8 *)req_buf->data_buf[child_id - 1];
        }

        hdr->req_data.piece = *ptr++;
        if (hdr->req_data.piece > req_data_piece.num)
        {
            return NULL;               // Error
        }
        MI_CpuCopy8(ptr, hdr->req_data.data, (u32)req_data_piece.size);
        ptr = MBi_ReceiveRequestDataPiece(hdr, child_id);
        break;
    case MB_COMM_TYPE_CHILD_ACCEPT_FILEINFO:
        hdr->data.req = (u16)(0x00ff & (*ptr++));
        hdr->data.req |= (((u16)(*ptr++) << 8) & 0xff00);
        break;
    case MB_COMM_TYPE_CHILD_CONTINUE:
        hdr->data.req = (u16)(0x00ff & (*ptr++));
        hdr->data.req |= (((u16)(*ptr++) << 8) & 0xff00);
        MI_CpuCopy8(ptr, hdr->data.reserved, (u32)req_data_piece.size);
        ptr += req_data_piece.size;
        break;
    default:
        return NULL;
    }

    return ptr;
}


/*---------------------------------------------------------------------------*
  Name:         MBi_ReceiveRequestDataPiece

  Description:  Builds data in its entirety from the divided request data that was received from the child.

  Arguments:    hdr: Pointer to a variable for getting the header
                recvbuf: Pointer to the receive buffer

  Returns:      Pointer to the data when the divided request data is all complete.
                NULL when not it is not complete.
 *---------------------------------------------------------------------------*/
static u8 *MBi_ReceiveRequestDataPiece(const MBCommChildBlockHeader * hdr, u16 child)
{
    u8      piece;
    u8     *ptr;

    SDK_ASSERT(child != 0 && child <= MB_MAX_CHILD);
    
    if (req_buf == NULL)
    {
        return NULL;
    }

    piece = hdr->req_data.piece;

    if (piece > req_data_piece.num)
    {
        return NULL;
    }
    
    ptr = ((u8 *)req_buf->data_buf[child - 1]) + (piece * req_data_piece.size);

    MI_CpuCopy8(&hdr->req_data.data[0], ptr, (u32)req_data_piece.size);

    req_buf->data_bmp[child - 1] |= (1 << piece);

    MB_DEBUG_OUTPUT(" %02x %02x %02x %02x %02x %02x\n", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4],
                    ptr[5]);

    if (IsGetAllRequestData(child))
    {
        return (u8 *)req_buf->data_buf[child - 1];
    }

    return NULL;
}


/*---------------------------------------------------------------------------*
  Name:         

  Description:  Builds data in its entirety from the divided request data that was received from the child.

  Arguments:    hdr: Pointer to a variable for getting the header
                recvbuf: Pointer to the receive buffer

  Returns:      TRUE if the divided request data is all complete.
                FALSE if it is not complete.
 *---------------------------------------------------------------------------*/
static BOOL IsGetAllRequestData(u16 child)
{
    u16     i;
    
    SDK_ASSERT(child != 0 && child <= MB_MAX_CHILD);

    /* Determine whether Pieces were collected */
    for (i = 0; i < req_data_piece.num; i++)
    {
        if ((req_buf->data_bmp[child - 1] & (1 << i)) == 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}



/*---------------------------------------------------------------------------*
  Name:         MBi_MakeChildSendBuffer

  Description:  Builds the actual data to send, using a child's send header information.

  Arguments:    hdr: Pointer to the child send header
                sendbuf: Pointer to the buffer that creates the send data

  Returns:      Pointer to send buffer.
 *---------------------------------------------------------------------------*/
u8     *MBi_MakeChildSendBuffer(const MBCommChildBlockHeader * hdr, u8 *sendbuf)
{
    u8     *pbuf = sendbuf;

    *pbuf++ = hdr->type;
    switch (hdr->type)
    {
    case MB_COMM_TYPE_CHILD_FILEREQ:
        *pbuf++ = hdr->req_data.piece;
        if (hdr->req_data.piece > req_data_piece.num)
        {
            return NULL;               // Error
        }

        MI_CpuCopy8((void *)(hdr->req_data.data), (void *)pbuf, (u32)req_data_piece.size);
        pbuf += req_data_piece.size;
        break;
    case MB_COMM_TYPE_CHILD_ACCEPT_FILEINFO:
        break;
    case MB_COMM_TYPE_CHILD_CONTINUE:
        *pbuf++ = (u8)(0x00ff & hdr->data.req); // Lo
        *pbuf++ = (u8)((0xff00 & hdr->data.req) >> 8);  // Hi
        MI_CpuCopy8((void *)(hdr->data.reserved), (void *)pbuf, (u32)req_data_piece.size);
        pbuf += req_data_piece.size;
        break;
    case MB_COMM_TYPE_CHILD_STOPREQ:
        break;
    case MB_COMM_TYPE_CHILD_BOOTREQ_ACCEPTED:
        break;
    default:
        return NULL;
    }

    return pbuf;
}


/*---------------------------------------------------------------------------*
  Name:         MBi_SendRequestDataPiece

  Description:  Divides request data to send from a child.

  Arguments:    pData: Pointer that gets divided data
                pReq: Request data to divide

  Returns:      Index value of the divided data that was obtained.
 *---------------------------------------------------------------------------*/
u8 MBi_SendRequestDataPiece(u8 *pData, const MBCommRequestData *pReq)
{
    const u8 *ptr = (u8 *)pReq;

    /* Decide upon the send piece */
    req_data_piece_idx = (u8)((req_data_piece_idx + 1) % req_data_piece.num);
    MB_DEBUG_OUTPUT("req_data piece : %d\n", req_data_piece_idx);

    // Copy data to the send buffer
    MI_CpuCopy8((void *)&ptr[req_data_piece_idx * req_data_piece.size],
                pData, (u32)req_data_piece.size);

    MB_DEBUG_OUTPUT(" %02x %02x %02x %02x %02x %02x\n", pData[0], pData[1], pData[2], pData[3],
                    pData[4], pData[5]);

    return req_data_piece_idx;
}



/*---------------------------------------------------------------------------*
  Name:         MBi_SetRecvBufferFromParent

  Description:  Get the header portion from a packet received from the parent, and return a pointer to the data portion.
                

  Arguments:    hdr: Pointer to a variable for getting the header
                recvbuf: Pointer to the receive buffer

  Returns:      Pointer to the data portion.
 *---------------------------------------------------------------------------*/
u8     *MBi_SetRecvBufferFromParent(MBCommParentBlockHeader * hdr, const u8 *recvbuf)
{

    hdr->type = *recvbuf++;

    switch (hdr->type)
    {
    case MB_COMM_TYPE_PARENT_SENDSTART:
        break;
    case MB_COMM_TYPE_PARENT_KICKREQ:
        break;
    case MB_COMM_TYPE_PARENT_DL_FILEINFO:
        break;
    case MB_COMM_TYPE_PARENT_DATA:
        hdr->fid = (u16)(*recvbuf++);  // Lo
        hdr->fid |= ((u16)(*recvbuf++) << 8);   // Hi
        hdr->seqno = (u16)(*recvbuf++); // Lo
        hdr->seqno |= ((u16)(*recvbuf++) << 8); // Hi
        break;
    case MB_COMM_TYPE_PARENT_BOOTREQ:
        break;
    case MB_COMM_TYPE_PARENT_MEMBER_FULL:
        break;
    default:
        return NULL;
    }
    return (u8 *)recvbuf;
}
