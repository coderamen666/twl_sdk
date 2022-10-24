/*---------------------------------------------------------------------------*
  Project:  TWLSDK - demos - FS - overlay-staticinit
  File:     mode.h

  Copyright 2007 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-06 #$
  $Rev: 2135 $
  $Author: yosizaki $
 *---------------------------------------------------------------------------*/
#if	!defined(NITRO_DEMOS_FS_OVERLAY_STATICINIT_MODE_H_)
#define NITRO_DEMOS_FS_OVERLAY_STATICINIT_MODE_H_


#include <nitro.h>


// Defines the overlay interface for each mode.
// These are self-configured by the overlay's static initializer.


#ifdef __cplusplus
extern  "C" {
#endif


/*---------------------------------------------------------------------------*/
/* Declarations */

// Input information structure
typedef struct InputData
{
    TPData  tp;
    u16     push_bits;              // The key bit at the moment of the press
    u16     hold_bits;              // The key bit of the state being pressed
    u16     release_bits;           // The key bit at the moment of the release
    u16     reserved;
}
InputData;


/*---------------------------------------------------------------------------*/
/* Constants */

// Proprietary expanded bit that displays the stylus touch state
#define PAD_BUTTON_TP   0x4000


/*---------------------------------------------------------------------------*/
/* functions */

// The state determination macro for the designated key
#define	IS_INPUT_(input, key, action)	\
	(((input).action ## _bits & (key)) != 0)


/*---------------------------------------------------------------------------*
  Name:         UpdateFrame

  Description:  Updates the internal state by only one frame in the current mode.

  Arguments:    frame_count      Frame count for the current operation.
                input            Input information array.
                player_count     Current number of players (number of valid input elements)
                own_player_id    This system's player number

  Returns:      Returns FALSE if the current mode ends this frame and TRUE otherwise.
                
 *---------------------------------------------------------------------------*/
extern BOOL (*UpdateFrame) (int frame_count,
                            const InputData * input, int player_count, int own_player_id);

/*---------------------------------------------------------------------------*
  Name:         DrawFrame

  Description:  Updates rendering based on the internal state in the current mode.

  Arguments:    frame_count      Frame count for the current operation.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void (*DrawFrame) (int frame_count);

/*---------------------------------------------------------------------------*
  Name:         EndFrame

  Description:  Ends the current mode.

  Arguments:    p_next_mode      When the next mode will be specified explicitly, the ID will overwrite the value indicated by this pointer.
                                 
                                 If no mode is specified, the mode that called the current mode will be selected.
                                 

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern void (*EndFrame) (FSOverlayID *p_next_mode);


#ifdef __cplusplus
}   // extern "C"
#endif


#endif  // NITRO_DEMOS_FS_OVERLAY_STATICINIT_MODE_H_
