/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_utility.h

  Copyright 2007-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/
#ifndef NITRO_LIBRARIES_CARD_UTILITY_H__
#define NITRO_LIBRARIES_CARD_UTILITY_H__


#include <nitro/types.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// Command interface for switching between the old and new DMA.
typedef struct CARDDmaInterface
{
    void  (*Recv)(u32 channel, const void *src, void *dst, u32 len);
    void  (*Stop)(u32 channel);
}
CARDDmaInterface;


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetDmaInterface

  Description:  Gets either the new or old DMA command interface from a DMA channel after determining which is appropriate.
                

  Arguments:    channel   DMA channel.
                          The range starts at 0 and ends at MI_DMA_MAX_NUM. If MI_DMA_USING_NEW is enabled, it indicates the new DMA.
                          

  Returns:      The appropriate DMA interface (either old or new).
 *---------------------------------------------------------------------------*/
const CARDDmaInterface* CARDi_GetDmaInterface(u32 channel);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ICInvalidateSmart

  Description:  Chooses the IC_InvalidateAll or IC_InvalidateRange function based on a threshold value.

  Arguments:    buffer     The buffer that should be invalidated
                length     The size that should be invalidated
                threshold  The threshold value to switch processing

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ICInvalidateSmart(void *buffer, u32 length, u32 threshold);

/*---------------------------------------------------------------------------*
  Name:         CARDi_DCInvalidateSmart

  Description:  Chooses the DC_FlushAll or DC_InvalidateRange function based on a threshold value.
                If the buffer is not aligned to a cache line, a Store operation will also be performed automatically at only the beginning and end.
                

  Arguments:    buffer     The buffer that should be invalidated
                length     The size that should be invalidated
                threshold  The threshold value to switch processing

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_DCInvalidateSmart(void *buffer, u32 length, u32 threshold);


#ifdef __cplusplus
} // extern "C"
#endif



#endif // NITRO_LIBRARIES_CARD_UTILITY_H__
