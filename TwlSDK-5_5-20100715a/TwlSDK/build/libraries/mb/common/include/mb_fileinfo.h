/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - include
  File:     mb_fileinfo.h

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

#ifndef	__MB_FILEINFO_H__
#define	__MB_FILEINFO_H__

#include <nitro.h>
#include <nitro/mb.h>

/* Macro definition -------------------------------------------------------- */

#define MB_DL_SEGMENT_NUM				(3)     //      Three segments: RomHeader, ARM9Static, and ARM7Static
#define MB_DOWNLOAD_FILEINFO_SIZE		(sizeof(MBDownloadFileInfo))

#define MB_AUTHCODE_SIZE				(136)   /* Authentication code size */

#define ROM_HEADER_SIZE_FULL			(0x160)

/* Type structure definition --------------------------------------------------------- */

//---------------------------------------------------------
// Download file information for multiboot, to be sent from a parent device to a child device
//---------------------------------------------------------

/*
 * Segment type definitions
 */
typedef enum MbSegmentType
{
    MB_SEG_ARM9STATIC = 0,
    MB_SEG_ARM7STATIC,
    MB_SEG_ROMHEADER
}
MbSegmentType;

/*
 * The multiboot header structure for initial boot segment information
 */
typedef struct MbDownloadFileInfoHeader
{
    u32     arm9EntryAddr;             /* ARM9 entry address */
    u32     arm7EntryAddr;             /* ARM7 entry address */
    u32     padding;
}
MbDownloadFileInfoHeader;

/*
 * Segment composition information
 */
typedef struct MbSegmentInfo
{
    u32     recv_addr;                 // Temporary storage address at receive time
    u32     load_addr;                 // Load address  (execution address)
    u32     size;                      // Segment size
    u32     target:1;                  // Target (MI_PROCESSOR_ARM9 or _ARM7. A file without attributes is ARM9)
    u32     rsv:31;                    // Reserved
}
MbSegmentInfo;                         // 16 bytes


/*
 * Structure for the entire initial boot segment
 */
typedef struct
{
    MbDownloadFileInfoHeader header;   // Header information (the entry address is stored here)
    MbSegmentInfo seg[MB_DL_SEGMENT_NUM];       // Segment information
    u32     auth_code[MB_AUTHCODE_SIZE / sizeof(u32)];  // Authentication code
    u32     reserved[32 / sizeof(u32)]; // Reserved region
}
MBDownloadFileInfo;


/* Table for referencing block information */
typedef struct
{
    // Offset from the start of the image in each segment
    u32     seg_src_offset[MB_DL_SEGMENT_NUM];
    u16     seg_head_blockno[MB_DL_SEGMENT_NUM];        // Starting block number of the segment
    u16     block_num;                 /* Total number of blocks */
}
MB_BlockInfoTable;


/* Block transfer-related data */
typedef struct
{
    u32     child_address;             // Storage address of the child device
    u32     size;                      // Block size
    u32     offset;                    // Set so that it is held in the offset value from the start of the image
    u8      segment_no;
    u8      pad[3];
}
MB_BlockInfo;

/* Const variables -------------------------------------------------------- */

extern const MbSegmentType MBi_defaultLoadSegList[MB_DL_SEGMENT_NUM];

/* Functions --------------------------------------------------------------- */

/*
 * Gets segment information from a pointer to MbDownloadFileInfoHeader
 * 
 */
static inline MbSegmentInfo *MBi_GetSegmentInfo(MbDownloadFileInfoHeader * mdfi, int i)
{
    return ((MbSegmentInfo *) (mdfi + 1)) + i;
}

BOOL    MBi_MakeBlockInfoTable(MB_BlockInfoTable * table, MbDownloadFileInfoHeader * mdfi);
BOOL    MBi_get_blockinfo(MB_BlockInfo * bi, MB_BlockInfoTable * table, u32 block,
                          MbDownloadFileInfoHeader * mdfi);
u16     MBi_get_blocknum(MB_BlockInfoTable * table);
BOOL    MBi_IsAbleToRecv(u8 segment_no, u32 address, u32 size);

#endif /* __MB_FILEINFO_H__ */
