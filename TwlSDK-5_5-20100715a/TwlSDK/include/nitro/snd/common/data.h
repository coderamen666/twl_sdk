/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - snd - common
  File:     data.h

  Copyright 2004-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_SND_COMMON_DATA_H_
#define NITRO_SND_COMMON_DATA_H_

#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
	structure definition
 ******************************************************************************/

typedef struct SNDBinaryFileHeader
{
    char    signature[4];
    u16     byteOrder;
    u16     version;
    u32     fileSize;
    u16     headerSize;
    u16     dataBlocks;
}
SNDBinaryFileHeader;

typedef struct SNDBinaryBlockHeader
{
    u32     kind;
    u32     size;
}
SNDBinaryBlockHeader;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NITRO_SND_COMMON_DATA_H_ */
