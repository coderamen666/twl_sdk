/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - libraries
  File:     card_rom.h

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
#ifndef NITRO_LIBRARIES_CARD_ROM_H__
#define NITRO_LIBRARIES_CARD_ROM_H__


#include <nitro.h>
#include "../include/card_common.h"


// #define SDK_ARM7_READROM_SUPPORT


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

typedef struct CARDTransferInfo
{
    u32     command;
    void  (*callback)(void *userdata);
    void   *userdata;
    u32     src;
    u32     dst;
    u32     len;
    u32     work;
}
CARDTransferInfo;

typedef void (*CARDTransferCallbackFunction)(void *userdata);


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithCPU

  Description:  Uses the CPU for a ROM transfer.
                Even though you do not need to consider caching or per-page restrictions, note that this function will block until the transfer is complete.
                

  Arguments:    userdata          (Dummy code for use as another callback)
                buffer            Buffer to transfer to
                offset            ROM offset to transfer from
                length            The transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
int CARDi_ReadRomWithCPU(void *userdata, void *buffer, u32 offset, u32 length);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithDMA

  Description:  Uses DMA for a ROM transfer.
                The function will return immediately if a transfer operation is started for the first page.

  Arguments:    info              Transfer information

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_ReadRomWithDMA(CARDTransferInfo *info);

/*---------------------------------------------------------------------------*
  Name:         CARDi_InitRom

  Description:  Initializes information for managing ROM access.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_InitRom(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_CheckPulledOutCore

  Description:  Main processing for functions that detect a card removal.
                The card bus must be locked.

  Arguments:    id            ROM-ID read from the card

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_CheckPulledOutCore(u32 id);

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomIDCore

  Description:  Read the Card ID

  Arguments:    None.

  Returns:      Card ID
 *---------------------------------------------------------------------------*/
u32     CARDi_ReadRomIDCore(void);

#ifdef SDK_ARM7_READROM_SUPPORT

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomCore

  Description:  Accesses the card from the ARM7.

  Arguments:    src        Transfer source offset
                src        Transfer source memory address
                src        Transfer size

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ReadRomCore(const void *src, void *dst, u32 len);

#endif // SDK_ARM7_READROM_SUPPORT

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomStatusCore

  Description:  Loads the card status.
                This is issued only when detecting a ROM that supports refreshing.
                This is necessary for NITRO applications with a supported ROM as well.

  Arguments:    None.

  Returns:      Card status
 *---------------------------------------------------------------------------*/
u32 CARDi_ReadRomStatusCore(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_RefreshRomCore

  Description:  Refreshes bad blocks on a card's ROM.
                This is necessary for NITRO applications with a relevant ROM, as well.
                This only issues a command to the card and therefore ignores latency settings.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void CARDi_RefreshRomCore(void);


#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_LIBRARIES_CARD_ROM_H__
