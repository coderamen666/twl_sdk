/*---------------------------------------------------------------------------*
  Project:  TwlSDK - include - nitro - vib
  File:     vib_system.h

  Copyright 2003-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $

 *---------------------------------------------------------------------------*/

#ifndef PULSE_VIB_H
#define PULSE_VIB_H

#include <nitro.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*
                    Types and Constants
 *-----------------------------------------------------------------------*/

/*!
    Maximum number of pulses in the pulse set. Rebuild the library if changes are made.
*/
#define VIB_PULSE_NUM_MAX   6

/*!
    Maximum value of on_time in one cycle (0.1-ms units)
*/
#define VIB_ON_TIME_MAX 15

/*!
    Minimum value of rest_time (0.1-ms units)
*/
#define VIB_REST_TIME_MIN   15

/*!
    Displays the status of the pulse rumble.
    
    The standard rumble is a pulse that is 1.5 ms ON, 1.5 ms OFF, and 1.5 ms ON. This allows generation of the strongest vibration.
    
    
    The hardware specifications demand that the value of VIBPulseState must obey the rules below.
    
    @li A single on_time must not exceed 1.5ms.
    @li off_time[n] must be set equal or longer to the value of the previous on_time[n].
    @li rest_time must be equal to or longer than 1.5ms.
    
    Note that these conditions are checked when the VIB_StartPulse function is called.
    
    @image html pulse_vib.jpg "Pulse rumble example (when pulse count is three)"
*/
typedef struct
{
    u32     pulse_num;                  /*! How many pulses to generate in a single pulse set. This must be at least 1 and no more than VIB_PULSE_NUM_MAX. */
    u32     rest_time;                  /*! Length of pause during pulse-set period. 1=0.1 millisecond. */
    u32     on_time[VIB_PULSE_NUM_MAX]; /*! Length of the activation time. Set a value larger than 0. 1=0.1 millisecond. */
    u32     off_time[VIB_PULSE_NUM_MAX];/*! Length of the stop time. Set a value larger than 0. 1=0.1 millisecond. */
    u32     repeat_num;                 /*! Number of times to repeat pulse set. When 0, repeats endlessly. */
}
VIBPulseState;

/*! This is a Rumble Pak-removal callback type */
typedef void (*VIBCartridgePulloutCallback) (void);

/*-----------------------------------------------------------------------*
                    External Function Declarations
 *-----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         VIB_Init

  Description:  Initializes Rumble Pak library.
                If this function is called again after it has been called once, it is equivalent to the VIB_IsCartridgeEnabled function.
                
                
                This function internally calls the PM_AppendPreSleepCallback function and registers the callback to end the rumble before entering sleep mode.
                
                
                This function also internally registers the callback that stops rumbling when cartridge removal is detected. Accordingly, after calling this function, if you set the cartridge removal callback with the CTRDG_SetPulledOutCallback function, the cartridge removal detection callback set with VIB_Init is overwritten.
                
                
                In that case, you will need to stop the rumbling inside the cartridge removal callback that you set.
                If you want to perform processing inside the cartridge removal callback that goes beyond stopping rumbling, register the callback using the VIB_SetCartridgePulloutCallback function, and make it so the processing is performed inside that callback.
                
                 

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern BOOL VIB_Init(void);

/*---------------------------------------------------------------------------*
  Name:         VIB_End

  Description:  Ends usage of the Rumble Pak library.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void VIB_End(void);

/*---------------------------------------------------------------------------*
  Name:         VIB_StartPulse

  Description:  Starts the pulse rumble.
                If the previous pulse rumble has not been completed, this lets it complete and then starts.
                Because the status is copied by the library, there is no need to allocate memory.

  Arguments:    state: Pulse rumble status

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void VIB_StartPulse(const VIBPulseState * state);

/*---------------------------------------------------------------------------*
  Name:         VIB_StopPulse

  Description:  Stops pulse rumble.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void VIB_StopPulse(void);

/*---------------------------------------------------------------------------*
  Name:         VIB_IsExecuting

  Description:  Determines whether pulse rumble is in progress.
                Returns TRUE from the time when rumble is turned on using the VIB_StartPulse function until the time when rumble is turned off using the VIB_StopPulse function.

  Arguments:    None.

  Returns:      TRUE: State in which pulse rumble is in progress
                FALSE: State in which pulse rumble is not in progress
 *---------------------------------------------------------------------------*/
extern BOOL VIB_IsExecuting(void);

/*---------------------------------------------------------------------------*
  Name:         VIB_SetCartridgePulloutCallback

  Description:  Registers the Game Pak removal callback.
                When the Game Pak is pulled out, the library immediately stops the pulse rumble.
                If a callback was registered using this function, it will be called afterwards. 

  Arguments:    func: Cartridge removal callback

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void VIB_SetCartridgePulloutCallback(VIBCartridgePulloutCallback func);

/*---------------------------------------------------------------------------*
  Name:         VIB_IsCartridgeEnabled

  Description:  Determines whether a Rumble Pak is inserted.

  Arguments:    None.

  Returns:      TRUE: State in which rumble pack was inserted at startup 
                FALSE: State in which rumble pack was not inserted at startup
 *---------------------------------------------------------------------------*/
extern BOOL VIB_IsCartridgeEnabled(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* PULSE_VIB_H */
