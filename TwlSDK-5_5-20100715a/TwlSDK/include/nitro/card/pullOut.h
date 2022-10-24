/*---------------------------------------------------------------------------*
  Project:  TwlSDK - CARD - include
  File:     pullOut.h

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
#ifndef NITRO_CARD_PULLOUT_H_
#define NITRO_CARD_PULLOUT_H_


#include <nitro/types.h>


#ifdef __cplusplus
extern  "C"
{
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

//---- Callback type for card pulled out
typedef BOOL (*CARDPulledOutCallback) (void);


/*---------------------------------------------------------------------------*/
/* Functions */

/*---------------------------------------------------------------------------*
  Name:         CARD_InitPulledOutCallback

  Description:  Sets system callback for card being pulled out.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_InitPulledOutCallback(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_IsPulledOut

  Description:  Returns whether card is pulled out.

  Arguments:    None.

  Returns:      TRUE if pull out detected
 *---------------------------------------------------------------------------*/
BOOL    CARD_IsPulledOut(void);

#ifdef SDK_ARM9

/*---------------------------------------------------------------------------*
  Name:         CARD_SetPulledOutCallback

  Description:  Sets user callback for card being pulled out.

  Arguments:    callback: callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_SetPulledOutCallback(CARDPulledOutCallback callback);

/*---------------------------------------------------------------------------*
  Name:         CARD_TerminateForPulledOut

  Description:  Terminates for card being pulled out.
                Sends message for ARM7 to perform termination.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_TerminateForPulledOut(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_PulledOutCallbackProc

  Description:  Callback for card pulled out.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_PulledOutCallbackProc(void);

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckPulledOut

  Description:  Get whether system has detected pulled-out card
                by comparing IPL cardID with current cardID
                (notice that once the card has been pulled out, IDs will definitely be different.)

  Arguments:    None.

  Returns:      TRUE if current cardID is equal to IPL cardID
 *---------------------------------------------------------------------------*/
void    CARD_CheckPulledOut(void);

#endif

#ifdef SDK_ARM7

/*---------------------------------------------------------------------------*
  Name:         CARD_CheckPullOut_Polling

  Description:  Polling to see if card has been pulled out.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARD_CheckPullOut_Polling(void);

#endif


/*---------------------------------------------------------------------------*
 * Internal Functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         CARDi_ResetSlotStatus

  Description:  Initializes the card removal state when a card is reinserted into the slot.
                This is an internal function used for system development. Note that the slot's hardware state will not be initialized if this function is simply called by a normal application.
                
                

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    CARDi_ResetSlotStatus(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetSlotResetCount

  Description:  Gets the number of times that a card was reinserted into the slot.
                This simply returns the number of times that the CARDi_ResetSlotStatus function was called.

  Arguments:    None.

  Returns:      The number of times that a card was reinserted into the slot. This is initially 0.
 *---------------------------------------------------------------------------*/
u32     CARDi_GetSlotResetCount(void);

/*---------------------------------------------------------------------------*
  Name:         CARDi_IsPulledOutEx

  Description:  Determines if a card has been removed from the slot.

  Arguments:    count: The number of times that a card had been reinserted into the slot when last you checked
                             This can be obtained with the CARDi_GetSlotResetCount function.

  Returns:      TRUE if the specified slot reinsertion count has not changed and we are not currently in the slot removal state.
                
 *---------------------------------------------------------------------------*/
BOOL    CARDi_IsPulledOutEx(u32 count);




#ifdef __cplusplus
} // extern "C"
#endif


#endif // NITRO_CARD_PULLOUT_H_
