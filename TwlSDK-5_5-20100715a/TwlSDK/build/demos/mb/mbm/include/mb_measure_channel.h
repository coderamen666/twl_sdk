/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - mbm
  File:     mb_measure_channel.h

  Copyright 2006-2008 Nintendo. All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law. They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef _MBM_MEASURE_CHANNEL_H_
#define _MBM_MEASURE_CHANNEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/types.h>

/* Constant Definitions */

typedef enum
{
    MBM_MEASURE_SUCCESS = 0,           // Get the channel
    MBM_MEASURE_ERROR_INIT_API = -1,   // 
    MBM_MEASURE_ERROR_INIT_CALLBACK = -2,       // 
    MBM_MEASURE_ERROR_MEASURECHANNEL_API = -3,  // 
    MBM_MEASURE_ERROR_MEASURECHANNEL_CALLBACK = -4,     // 
    MBM_MEASURE_ERROR_RESET_API = -5,  // 
    MBM_MEASURE_ERROR_RESET_CALLBACK = -6,      // 
    MBM_MEASURE_ERROR_END_API = -7,    // 
    MBM_MEASURE_ERROR_END_CALLBACK = -8,        // 
    MBM_MEASURE_ERROR_NO_ALLOWED_CHANNEL = -9,  // 
    MBM_MEASURE_ERROR_ILLEGAL_STATE = -10       // 
}
MBMErrCode;


typedef struct
{
    s16     errcode;                   // Error code (MBMErrCode)
    u16     channel;                   // Channel with the lowest usage rate
}
MBMCallback;

/* Type Definitions */
typedef void (*MBMCallbackFunc) (MBMCallback *);


/* Function definitions */

// The function to search for the optimal channel, which is called before wireless initialization
void    MBM_MeasureChannel(u8 *wm_buffer, MBMCallbackFunc callback);
// The function to search for the optimal channel, which is called in the IDLE state
void    MBM_MeasureChannelInIdle(MBMCallbackFunc callback);



#ifdef __cplusplus
}
#endif

#endif /* _MBM_MEASURE_CHANNEL_H_ */
