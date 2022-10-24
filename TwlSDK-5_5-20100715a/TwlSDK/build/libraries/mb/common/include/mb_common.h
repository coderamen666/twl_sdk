 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_common.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef __MB_COMMON_H__
#define __MB_COMMON_H__

/* Macro definition -------------------------------------------------------- */

// For debugging
#ifdef	PRINT_DEBUG
#define MB_COMM_TYPE_OUTPUT             MBi_comm_type_output
#define MB_COMM_WMEVENT_OUTPUT          MBi_comm_wmevent_output
#else
#define MB_COMM_TYPE_OUTPUT( ... )      ((void)0)
#define MB_COMM_WMEVENT_OUTPUT( ... )   ((void)0)
#endif

/* Send/receive size definitions */
#define MB_COMM_P_SENDLEN_DEFAULT       (256)
#define MB_COMM_P_RECVLEN_DEFAULT       (8)

#define MB_COMM_P_SENDLEN_MAX           MB_COMM_PARENT_SEND_MAX
#define MB_COMM_P_RECVLEN_MAX           MB_COMM_PARENT_RECV_MAX

#define MB_COMM_P_SENDLEN_MIN           (sizeof(MBDownloadFileInfo))    // Minimum size
#define MB_COMM_P_RECVLEN_MIN           MB_COMM_PARENT_RECV_MIN

#define MB_COMM_REQ_DATA_SIZE           (29)

/* Calculation macros relating to fragmented request data. */
#define MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ )       (( __P_RECVLEN__ ) - 2)
#define MB_COMM_CALC_REQ_DATA_PIECE_NUM( __P_RECVLEN__ )        \
                                ( (MB_COMM_REQ_DATA_SIZE + 1) / (MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ )) )
#define MB_COMM_CALC_REQ_DATA_BUF_SIZE( __P_RECVLEN__ )     (MB_COMM_REQ_DATA_SIZE + 1)
/*
 * There are logical errors above, but they cannot be corrected because the IPL runs on these specifications.
 *    Properly speaking,
 * #define MB_COMM_CALC_REQ_DATA_PICE_NUM( __P_RECV_LEN__)                                                            \
 *                                  ( (MB_COMM_REQ_DATA_SIZE + MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ ) - 1) \
 *                                    / MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ ) )
 * #define MB_COMM_CALC_REQ_DATA_BUF_SIZE( __P_RECV_LEN__ )                                 \
 *                                  ( MB_COMM_CALC_REQ_DATA_PIECE_SIZE( __P_RECVLEN__ )     \
 *                                    * MB_COMM_CALC_REQ_DATA_PIECE_NUM( __P_RECVLEN__ ) )
 */


/* Block header size */
#define MB_COMM_PARENT_HEADER_SIZE      (6)     // MB_CommParentBlockHeader size (without padding)
#define MB_COMM_CHILD_HEADER_SIZE       (8)     // MB_CommChildBlockHeader size (without padding)

#define MB_COMM_CALC_BLOCK_SIZE( __P_SENDLEN__ )                (( __P_SENDLEN__ ) - MB_COMM_PARENT_HEADER_SIZE)

/* Definition of an error that is returned by data transmission functions.
   Defined for values that do not overlap with WM ERRCODE. */
#define MB_SENDFUNC_STATE_ERR           (WM_ERRCODE_MAX + 1)

/* Block transfer data types */
typedef enum MBCommType
{
    MB_COMM_TYPE_DUMMY = 0,            //  0

    MB_COMM_TYPE_PARENT_SENDSTART,     //  1
    MB_COMM_TYPE_PARENT_KICKREQ,       //  2
    MB_COMM_TYPE_PARENT_DL_FILEINFO,   //  3
    MB_COMM_TYPE_PARENT_DATA,          //  4
    MB_COMM_TYPE_PARENT_BOOTREQ,       //  5
    MB_COMM_TYPE_PARENT_MEMBER_FULL,   //  6

    MB_COMM_TYPE_CHILD_FILEREQ,        //  7
    MB_COMM_TYPE_CHILD_ACCEPT_FILEINFO, //  8
    MB_COMM_TYPE_CHILD_CONTINUE,       //  9
    MB_COMM_TYPE_CHILD_STOPREQ,        //  10
    MB_COMM_TYPE_CHILD_BOOTREQ_ACCEPTED //  11
}
MBCommType;

/* Request type from the user */
typedef enum MBCommUserReq
{
    MB_COMM_USER_REQ_NONE = 0,         //  0
    MB_COMM_USER_REQ_DL_START,         //  1
    MB_COMM_USER_REQ_SEND_START,       //  2
    MB_COMM_USER_REQ_ACCEPT,           //  3
    MB_COMM_USER_REQ_KICK,             //  4
    MB_COMM_USER_REQ_BOOT              //  5
}
MBCommUserReq;

/* Structure definition ---------------------------------------------------- */

/*
 * Error notification callback
 */
typedef struct
{
    u16     apiid;                     // API code
    u16     errcode;                   // Error Codes
}
MBErrorCallback;


/* Functions --------------------------------------------------------------- */

/*
 * Initializes the block header, and configures only the type.
 * After this, small fields will be packed in until they are sent with the MBi_BlockHeaderEnd function.
 * 
 * If there are no arguments, it may be left unchanged.
 */
void    MBi_BlockHeaderBegin(u8 type, u32 *sendbuf);

/*
 * Finishes configuring the block header and calculates the checksum.
 * After that, it will actually be sent to the other party specified by pollbmp.
 */
int     MBi_BlockHeaderEnd(int body_len, u16 pollbmp, u32 *sendbuf);


/*
 * Checksum calculation
 */
u16     MBi_calc_cksum(const u16 *buf, int length);

// --- For debugging
void    MBi_DebugPrint(const char *file, int line, const char *func, const char *fmt, ...);
void    MBi_comm_type_output(u16 type);
void    MBi_comm_wmevent_output(u16 type, void *arg);

#endif /* __MB_COMMON_H__ */
